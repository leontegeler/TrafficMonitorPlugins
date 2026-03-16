#include "pch.h"
#include "NowPlayingItem.h"
#include "DataManager.h"
#include <algorithm>

CNowPlayingItem::CNowPlayingItem()
    : m_scroll_offset(0)
    , m_last_scroll_time(0)
{
}

CNowPlayingItem::~CNowPlayingItem()
{
}

const wchar_t* CNowPlayingItem::GetItemName() const
{
    return L"Now playing";
}

const wchar_t* CNowPlayingItem::GetItemId() const
{
    return L"now_playing_v6";
}

const wchar_t* CNowPlayingItem::GetItemLableText() const
{
    if (m_playback_status == PlaybackStatus::Playing)
        return L"\u23F5 ";
    if (m_playback_status == PlaybackStatus::Paused)
        return L"\u23F8 ";
    return L"";
}

const wchar_t* CNowPlayingItem::GetItemValueText() const
{
    if (m_playback_status == PlaybackStatus::Stopped || (m_artist.empty() && m_title.empty()))
    {
        return L"";
    }

    m_full_text.clear();
    if (!m_artist.empty())
    {
        m_full_text = m_artist;
    }
    
    if (!m_title.empty())
    {
        if (!m_full_text.empty())
            m_full_text += L" - ";
        m_full_text += m_title;
    }

    return m_full_text.c_str();
}

const wchar_t* CNowPlayingItem::GetItemValueSampleText() const
{
    return L"Artist Name - Song Title";
}

bool CNowPlayingItem::IsCustomDraw() const
{
    return true;
}

int CNowPlayingItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    if (pDC == nullptr)
        return 0;

    std::wstring label = GetItemLableText();
    std::wstring text = GetItemValueText();
    if (text.empty() && label.empty()) return 0;

    // Calculate sizes
    int label_width = 0;
    if (!label.empty())
    {
        CFont font;
        LOGFONT lf;
        pDC->GetCurrentFont()->GetLogFont(&lf);
        wcscpy_s(lf.lfFaceName, L"Segoe UI Symbol");
        font.CreateFontIndirect(&lf);
        CFont* pOldFont = pDC->SelectObject(&font);
        label_width = pDC->GetTextExtent(label.c_str(), static_cast<int>(label.length())).cx;
        pDC->SelectObject(pOldFont);
    }
    int text_width = pDC->GetTextExtent(text.c_str(), static_cast<int>(text.length())).cx;

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int content_width = label_width + text_width + 8; // 8px padding

    // To utilize available space on the left, we request a large width.
    // Since we right-align in DrawItem, this effectively claims space to the left.
    // We use a generous minimum width (25% of screen) to fulfill the request 
    // of using available taskbar space.
    int min_width = screen_width / 4;
    return (std::max)(content_width, min_width);
}

void CNowPlayingItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Set colors
    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    std::wstring label = GetItemLableText();
    std::wstring text = GetItemValueText();
    if (text.empty() && label.empty()) return;

    // Calculate sizes
    CSize label_size = { 0, 0 };
    CFont icon_font;
    bool has_label = !label.empty();
    if (has_label)
    {
        LOGFONT lf;
        pDC->GetCurrentFont()->GetLogFont(&lf);
        wcscpy_s(lf.lfFaceName, L"Segoe UI Symbol");
        icon_font.CreateFontIndirect(&lf);
        CFont* pOldFont = pDC->SelectObject(&icon_font);
        label_size = pDC->GetTextExtent(label.c_str(), static_cast<int>(label.length()));
        pDC->SelectObject(pOldFont);
    }
    CSize text_size = pDC->GetTextExtent(text.c_str(), static_cast<int>(text.length()));

    int total_content_width = label_size.cx + text_size.cx;

    // Right align everything as a group within the allocated width 'w'.
    // start_x is where the icon/label will begin.
    int start_x = x + w - total_content_width;
    
    // If w is somehow smaller than our content (capped by TrafficMonitor),
    // we must not draw outside the left boundary (x).
    if (start_x < x) start_x = x;
    
    // 1. Draw label (icon)
    if (has_label)
    {
        CFont* pOldFont = pDC->SelectObject(&icon_font);
        CRect label_rect(start_x, y, start_x + label_size.cx, y + h);
        pDC->DrawText(label.c_str(), -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        pDC->SelectObject(pOldFont);
    }
    
    // 2. Draw text
    // The text starts immediately after the label.
    CRect value_rect(start_x + label_size.cx, y, x + w, y + h);
    pDC->DrawText(text.c_str(), -1, &value_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    
    // Reset scrolling state
    m_scroll_offset = 0;
    m_scroll_reverse = false;
    m_pause_end_time = 0;
}

void CNowPlayingItem::SetMediaInfo(const std::wstring& artist, const std::wstring& title, PlaybackStatus status)
{
    if (m_artist != artist || m_title != title || m_playback_status != status)
    {
        m_scroll_offset = 0;
    }
    m_artist = artist;
    m_title = title;
    m_playback_status = status;
}
