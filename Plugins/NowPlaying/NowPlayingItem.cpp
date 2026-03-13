#include "pch.h"
#include "NowPlayingItem.h"
#include "DataManager.h"

CNowPlayingItem::CNowPlayingItem()
    : m_thumbnail(nullptr)
    , m_thumbnail_size(0)
    , m_scroll_offset(0)
    , m_last_scroll_time(0)
{
}

CNowPlayingItem::~CNowPlayingItem()
{
    if (m_thumbnail)
    {
        DeleteObject(m_thumbnail);
        m_thumbnail = nullptr;
    }
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
    return L"Now playing:";
}

const wchar_t* CNowPlayingItem::GetItemValueText() const
{
    if (m_artist.empty() && m_title.empty())
    {
        return L"No media";
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
    CSize label_size = pDC->GetTextExtent(L"Now playing: ", 13);
    return label_size.cx + 150;
}

void CNowPlayingItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Set colors
    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    // 1. Draw static label
    std::wstring label = L"Now playing: ";
    CSize label_size = pDC->GetTextExtent(label.c_str(), static_cast<int>(label.length()));
    CRect label_rect(x, y, x + label_size.cx, y + h);
    pDC->DrawText(label.c_str(), -1, &label_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

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
    }
    else
    {
        // Marquee scrolling logic
        DWORD now = GetTickCount();
        if (m_last_scroll_time == 0) m_last_scroll_time = now;

        if (now - m_last_scroll_time > 16) // 60 FPS
        {
            m_scroll_offset += 1;
            if (m_scroll_offset > text_size.cx - value_w + 30)
            {
                m_scroll_offset = -30;
            }
            m_last_scroll_time = now;
        }

        int draw_x = value_x - (m_scroll_offset < 0 ? 0 : m_scroll_offset);

        // Clip the drawing area to the value part only
        int save_dc = pDC->SaveDC();
        CRgn clipRgn;
        clipRgn.CreateRectRgn(value_rect.left, value_rect.top, value_rect.right, value_rect.bottom);
        pDC->SelectClipRgn(&clipRgn);

        pDC->TextOut(draw_x, y + (h - text_size.cy) / 2, text.c_str(), static_cast<int>(text.length()));

        pDC->RestoreDC(save_dc);
    }
}

void CNowPlayingItem::SetMediaInfo(const std::wstring& artist, const std::wstring& title, HBITMAP hThumbnail)
{
    if (m_artist != artist || m_title != title)
    {
        m_scroll_offset = 0;
    }
    m_artist = artist;
    m_title = title;
    
    if (m_thumbnail)
    {
        DeleteObject(m_thumbnail);
        m_thumbnail = nullptr;
    }
    m_thumbnail = hThumbnail;
}
