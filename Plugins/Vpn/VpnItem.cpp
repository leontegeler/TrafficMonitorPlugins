#include "pch.h"
#include "VpnItem.h"
#include "DataManager.h"

CVpnItem::CVpnItem()
{
}

const wchar_t* CVpnItem::GetItemName() const
{
    return L"VPN Connection";
}

const wchar_t* CVpnItem::GetItemId() const
{
    return L"VpnConnection_v2";
}

const wchar_t* CVpnItem::GetItemLableText() const
{
    return L"VPN:";
}

const wchar_t* CVpnItem::GetItemValueText() const
{
    return m_value.c_str();
}

const wchar_t* CVpnItem::GetItemValueSampleText() const
{
    return L"My VPN Name";
}

bool CVpnItem::IsCustomDraw() const
{
    return true;
}

int CVpnItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CSize label_size = pDC->GetTextExtent(L"VPN: ", 4);
    return label_size.cx + 150;
}

void CVpnItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Set colors
    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    // 1. Draw static label
    std::wstring label = L"VPN: ";
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

void CVpnItem::SetText(const wchar_t* text)
{
    if (m_value != text)
    {
        m_scroll_offset = 0;
    }
    m_value = text;
}
