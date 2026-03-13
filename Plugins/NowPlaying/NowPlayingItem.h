#pragma once
#include "../../include/PluginInterface.h"
#include <memory>

class CNowPlayingItem : public IPluginItem
{
public:
    enum class PlaybackStatus
    {
        Stopped,
        Playing,
        Paused
    };

    CNowPlayingItem();
    virtual ~CNowPlayingItem();

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;

    void SetMediaInfo(const std::wstring& artist, const std::wstring& title, PlaybackStatus status);

private:
    mutable std::wstring m_display_text;
    std::wstring m_artist;
    std::wstring m_title;
    PlaybackStatus m_playback_status{ PlaybackStatus::Stopped };
    
    // Scrolling state
    mutable int m_scroll_offset{ 0 };
    mutable DWORD m_last_scroll_time{ 0 };
    mutable std::wstring m_full_text;
    mutable bool m_scroll_reverse{ false };
    mutable DWORD m_pause_end_time{ 0 };
};
