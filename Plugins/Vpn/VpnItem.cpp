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
    if (m_value.empty())
        return L"";
    return L"\u2191\u2193 ";
}

const wchar_t* CVpnItem::GetItemValueText() const
{
    if (m_value.empty())
        return L"";
    m_display_text = L"Connected to " + m_value;
    return m_display_text.c_str();
}

const wchar_t* CVpnItem::GetItemValueSampleText() const
{
    return L"Connected to My VPN Name";
}

bool CVpnItem::IsCustomDraw() const
{
    return true;
}

int CVpnItem::GetItemWidthEx(void* hDC) const
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

void CVpnItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Set colors
    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);

    // 1. Draw static label
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
        // Ping-pong scrolling logic at ~60 FPS
        DWORD now = GetTickCount();
        if (m_last_scroll_time == 0) m_last_scroll_time = now;

        if (now >= m_pause_end_time)
        {
            if (now - m_last_scroll_time > 16)
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

void CVpnItem::SetText(const wchar_t* text)
{
    if (m_value != text)
    {
        m_scroll_offset = 0;
    }
    m_value = text;
}
