#pragma once
#include "../../include/PluginInterface.h"
#include "NowPlayingItem.h"
#include <string>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>

class CNowPlaying : public ITMPlugin
{
private:
    CNowPlaying();

public:
    static CNowPlaying& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    virtual void OnInitialize(ITrafficMonitor* pApp) override;

private:
    void InitializeMediaSessionManager();
    void UpdateMediaInfo();
    HBITMAP LoadThumbnailFromStream(winrt::Windows::Storage::Streams::IRandomAccessStreamReference thumbnail);
    HBITMAP CreateDefaultThumbnail(int size);

private:
    static CNowPlaying m_instance;
    CNowPlayingItem m_item;
    std::wstring m_tooltip_info;
    winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager m_session_manager{ nullptr };
    bool m_initialized{ false };
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
