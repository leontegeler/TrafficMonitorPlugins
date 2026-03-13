#include "pch.h"
#include "NowPlaying.h"
#include "DataManager.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <wincodec.h>
#include <wincodecsdk.h>

#pragma comment(lib, "windowscodecs.lib")

CNowPlaying CNowPlaying::m_instance;

CNowPlaying::CNowPlaying()
{
}

CNowPlaying& CNowPlaying::Instance()
{
    return m_instance;
}

IPluginItem* CNowPlaying::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CNowPlaying::GetTooltipInfo()
{
    return L" ";
}

void CNowPlaying::InitializeMediaSessionManager()
{
    if (m_initialized)
        return;

    try
    {
        winrt::init_apartment();

        auto async_op = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync();
        m_session_manager = async_op.get();
        m_initialized = true;
    }
    catch (...)
    {
        m_initialized = false;
    }
}

HBITMAP CNowPlaying::CreateDefaultThumbnail(int size)
{
    // Create a simple gray square as default thumbnail
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    // Fill with gray color
    RECT rect = { 0, 0, size, size };
    HBRUSH hBrush = CreateSolidBrush(RGB(128, 128, 128));
    FillRect(memDC, &rect, hBrush);
    DeleteObject(hBrush);

    // Draw music note symbol
    SetTextColor(memDC, RGB(200, 200, 200));
    SetBkMode(memDC, TRANSPARENT);

    HFONT hFont = CreateFont(size * 2 / 3, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);

    DrawText(memDC, L"♪", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(memDC, hOldFont);
    DeleteObject(hFont);

    SelectObject(memDC, hOldBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    return hBitmap;
}

HBITMAP CNowPlaying::LoadThumbnailFromStream(winrt::Windows::Storage::Streams::IRandomAccessStreamReference thumbnail)
{
    try
    {
        if (!thumbnail)
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);

        // Open the stream
        auto stream = thumbnail.OpenReadAsync().get();
        if (!stream)
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);

        // Get stream size
        uint64_t size = stream.Size();
        if (size == 0)
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);

        // Read data into buffer
        winrt::Windows::Storage::Streams::Buffer buffer(static_cast<uint32_t>(size));
        stream.ReadAsync(buffer, static_cast<uint32_t>(size), winrt::Windows::Storage::Streams::InputStreamOptions::None).get();

        // Get raw buffer pointer
        auto dataReader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
        std::vector<uint8_t> imageData(static_cast<size_t>(size));
        dataReader.ReadBytes(imageData);

        // Create WIC factory
        IWICImagingFactory* pFactory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr) || !pFactory)
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);

        // Create stream from memory
        IWICStream* pStream = nullptr;
        hr = pFactory->CreateStream(&pStream);
        if (FAILED(hr))
        {
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        hr = pStream->InitializeFromMemory(imageData.data(), static_cast<DWORD>(imageData.size()));
        if (FAILED(hr))
        {
            pStream->Release();
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        // Create decoder
        IWICBitmapDecoder* pDecoder = nullptr;
        hr = pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
        if (FAILED(hr))
        {
            pStream->Release();
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        // Get first frame
        IWICBitmapFrameDecode* pFrame = nullptr;
        hr = pDecoder->GetFrame(0, &pFrame);
        if (FAILED(hr))
        {
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        // Create scaler
        int targetSize = g_data.m_setting_data.cover_art_size;
        IWICBitmapScaler* pScaler = nullptr;
        hr = pFactory->CreateBitmapScaler(&pScaler);
        if (SUCCEEDED(hr))
        {
            hr = pScaler->Initialize(pFrame, targetSize, targetSize, WICBitmapInterpolationModeFant);
        }

        // Convert to 32bppBGRA format
        IWICFormatConverter* pConverter = nullptr;
        hr = pFactory->CreateFormatConverter(&pConverter);
        if (FAILED(hr))
        {
            if (pScaler) pScaler->Release();
            pFrame->Release();
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        hr = pConverter->Initialize(
            pScaler ? (IWICBitmapSource*)pScaler : (IWICBitmapSource*)pFrame,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.0,
            WICBitmapPaletteTypeCustom
        );

        if (FAILED(hr))
        {
            pConverter->Release();
            if (pScaler) pScaler->Release();
            pFrame->Release();
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
        }

        // Get dimensions
        UINT width, height;
        pConverter->GetSize(&width, &height);

        // Create HBITMAP
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HDC hdc = GetDC(NULL);
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        ReleaseDC(NULL, hdc);

        if (hBitmap && pBits)
        {
            // Copy pixels
            pConverter->CopyPixels(NULL, width * 4, width * height * 4, static_cast<BYTE*>(pBits));
        }

        // Cleanup
        pConverter->Release();
        if (pScaler) pScaler->Release();
        pFrame->Release();
        pDecoder->Release();
        pStream->Release();
        pFactory->Release();

        return hBitmap ? hBitmap : CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
    }
    catch (...)
    {
        return CreateDefaultThumbnail(g_data.m_setting_data.cover_art_size);
    }
}

void CNowPlaying::UpdateMediaInfo()
{
    if (!m_initialized)
    {
        InitializeMediaSessionManager();
        if (!m_initialized)
        {
            m_item.SetMediaInfo(L"", L"", nullptr, CNowPlayingItem::PlaybackStatus::Stopped);
            m_tooltip_info.clear();
            return;
        }
    }

    try
    {
        auto current_session = m_session_manager.GetCurrentSession();

        if (current_session == nullptr)
        {
            m_item.SetMediaInfo(L"", L"", nullptr, CNowPlayingItem::PlaybackStatus::Stopped);
            m_tooltip_info.clear();
            return;
        }

        // Get playback status
        CNowPlayingItem::PlaybackStatus status = CNowPlayingItem::PlaybackStatus::Stopped;
        auto playback_info = current_session.GetPlaybackInfo();
        if (playback_info != nullptr)
        {
            auto winrt_status = playback_info.PlaybackStatus();
            if (winrt_status == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing)
                status = CNowPlayingItem::PlaybackStatus::Playing;
            else if (winrt_status == winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused)
                status = CNowPlayingItem::PlaybackStatus::Paused;
        }

        auto media_properties = current_session.TryGetMediaPropertiesAsync().get();

        if (media_properties == nullptr)
        {
            m_item.SetMediaInfo(L"", L"", nullptr, status);
            m_tooltip_info.clear();
            return;
        }

        std::wstring artist = media_properties.Artist().c_str();
        std::wstring title = media_properties.Title().c_str();
        std::wstring album = media_properties.AlbumTitle().c_str();

        // Load thumbnail
        HBITMAP hThumbnail = nullptr;
        if (g_data.m_setting_data.show_cover_art)
        {
            auto thumbnail = media_properties.Thumbnail();
            hThumbnail = LoadThumbnailFromStream(thumbnail);
        }

        m_item.SetMediaInfo(artist, title, hThumbnail, status);

        // Build tooltip with more details
        m_tooltip_info.clear();
    }
    catch (...)
    {
        m_item.SetMediaInfo(L"", L"", nullptr, CNowPlayingItem::PlaybackStatus::Stopped);
        m_tooltip_info.clear();
    }
}

void CNowPlaying::DataRequired()
{
    UpdateMediaInfo();
}

ITMPlugin::OptionReturn CNowPlaying::ShowOptionsDialog(void* hParent)
{
    // No options dialog for now
    return ITMPlugin::OR_OPTION_NOT_PROVIDED;
}

const wchar_t* CNowPlaying::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"Now playing";
    case TMI_DESCRIPTION:
        return L"Display currently playing media information from Windows";
    case TMI_AUTHOR:
        return L"Leon Tegeler";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Leon Tegeler 2026";
    case ITMPlugin::TMI_URL:
        return L"";
    case TMI_VERSION:
        return L"1.02";
    default:
        break;
    }
    return L"";
}

void CNowPlaying::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        g_data.LoadConfig(std::wstring(data));
        break;
    default:
        break;
    }
}

void CNowPlaying::OnInitialize(ITrafficMonitor* pApp)
{
    // Initialize on first load
    InitializeMediaSessionManager();
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CNowPlaying::Instance();
}
