#include "pch.h"
#include "NowPlayingItem.h"
#include "DataManager.h"

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
    std::wstring label = GetItemLableText();
    std::wstring text = GetItemValueText();
    if (label.empty() && text.empty())
        return 0;

    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CSize label_size = { 0, 0 };
    if (!label.empty())
    {
        CFont font;
        LOGFONT lf;
        pDC->GetCurrentFont()->GetLogFont(&lf);
        wcscpy_s(lf.lfFaceName, L"Segoe UI Symbol");
        font.CreateFontIndirect(&lf);
        CFont* pOldFont = pDC->SelectObject(&font);
        label_size = pDC->GetTextExtent(label.c_str(), static_cast<int>(label.length()));
        pDC->SelectObject(pOldFont);
    }
    return label_size.cx + (text.empty() ? 0 : 150);
}

void CNowPlayingItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Set colors
    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    // 1. Draw static label (icon)
    std::wstring label = GetItemLableText();
    CSize label_size = { 0, 0 };
    if (!label.empty())
    {
        CFont font;
        LOGFONT lf;
        pDC->GetCurrentFont()->GetLogFont(&lf);
        wcscpy_s(lf.lfFaceName, L"Segoe UI Symbol");
        font.CreateFontIndirect(&lf);
        CFont* pOldFont = pDC->SelectObject(&font);

        label_size = pDC->GetTextExtent(label.c_str(), static_cast<int>(label.length()));
        CRect label_rect(x, y, x + label_size.cx, y + h);
        pDC->DrawText(label.c_str(), -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        pDC->SelectObject(pOldFont);
    }

    // 2. Draw scrolling value
    std::wstring text = GetItemValueText();
    if (text.empty()) return;

    int value_x = x + label_size.cx;
    int value_w = w - label_size.cx;
    CRect value_rect(value_x, y, x + w, y + h);

    CSize text_size = pDC->GetTextExtent(text.c_str(), static_cast<int>(text.length()));

    if (text_size.cx <= value_w)
    {
        // No scrolling needed
        pDC->DrawText(text.c_str(), -1, &value_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        m_scroll_offset = 0;
        m_scroll_reverse = false;
        m_pause_end_time = 0;
    }
    else
    {
        // Ping-pong scrolling logic
        DWORD now = GetTickCount();
        if (m_last_scroll_time == 0) m_last_scroll_time = now;

        if (now >= m_pause_end_time)
        {
            if (now - m_last_scroll_time > 16) // ~60 FPS
            {
                int max_scroll = text_size.cx - value_w;
                const DWORD pause_duration = 1500; // 1.5s pause

                if (!m_scroll_reverse)
                {
                    m_scroll_offset++;
                    if (m_scroll_offset >= max_scroll)
                    {
                        m_scroll_offset = max_scroll;
                        m_scroll_reverse = true;
                        m_pause_end_time = now + pause_duration;
                    }
                }
                else
                {
                    m_scroll_offset--;
                    if (m_scroll_offset <= 0)
                    {
                        m_scroll_offset = 0;
                        m_scroll_reverse = false;
                        m_pause_end_time = now + pause_duration;
                    }
                }
                m_last_scroll_time = now;
            }
        }
        else
        {
            // During pause, just update last scroll time to avoid sudden jump after pause
            m_last_scroll_time = now;
        }

        int draw_x = value_x - m_scroll_offset;

        // Clip the drawing area to the value part only
        int save_dc = pDC->SaveDC();
        CRgn clipRgn;
        clipRgn.CreateRectRgn(value_rect.left, value_rect.top, value_rect.right, value_rect.bottom);
        pDC->SelectClipRgn(&clipRgn);

        pDC->TextOut(draw_x, y + (h - text_size.cy) / 2, text.c_str(), static_cast<int>(text.length()));

        pDC->RestoreDC(save_dc);
    }
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
