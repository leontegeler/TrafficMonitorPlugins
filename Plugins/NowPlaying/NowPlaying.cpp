#include "pch.h"
#include "NowPlaying.h"
#include "DataManager.h"
#include <winrt/Windows.Foundation.Collections.h>

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

void CNowPlaying::UpdateMediaInfo()
{
    if (!m_initialized)
    {
        InitializeMediaSessionManager();
        if (!m_initialized)
        {
            m_item.SetMediaInfo(L"", L"", CNowPlayingItem::PlaybackStatus::Stopped);
            m_tooltip_info.clear();
            return;
        }
    }

    try
    {
        auto current_session = m_session_manager.GetCurrentSession();

        if (current_session == nullptr)
        {
            m_item.SetMediaInfo(L"", L"", CNowPlayingItem::PlaybackStatus::Stopped);
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
            m_item.SetMediaInfo(L"", L"", status);
            m_tooltip_info.clear();
            return;
        }

        std::wstring artist = media_properties.Artist().c_str();
        std::wstring title = media_properties.Title().c_str();
        std::wstring album = media_properties.AlbumTitle().c_str();

        m_item.SetMediaInfo(artist, title, status);

        // Build tooltip with more details
        m_tooltip_info.clear();
    }
    catch (...)
    {
        m_item.SetMediaInfo(L"", L"", CNowPlayingItem::PlaybackStatus::Stopped);
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
