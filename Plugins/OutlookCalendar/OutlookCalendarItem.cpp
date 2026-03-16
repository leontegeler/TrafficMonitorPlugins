#include "pch.h"
#include "OutlookCalendarItem.h"
#include "DataManager.h"
#include <algorithm>

COutlookCalendarItem::COutlookCalendarItem()
{
}

const wchar_t* COutlookCalendarItem::GetItemName() const
{
    return g_data.StringRes(IDS_PLUGIN_NAME);
}

const wchar_t* COutlookCalendarItem::GetItemId() const
{
    return L"outlook_calendar_item_v3";
}

const wchar_t* COutlookCalendarItem::GetItemLableText() const
{
    return L"\xD83D\xDCC5 ";
}

const wchar_t* COutlookCalendarItem::GetItemValueText() const
{
    return g_data.GetNextEventDisplayString();
}

const wchar_t* COutlookCalendarItem::GetItemValueSampleText() const
{
    return L"Meeting (1h 30m)";
}

bool COutlookCalendarItem::IsCustomDraw() const
{
    return true;
}

int COutlookCalendarItem::GetItemWidthEx(void* hDC) const
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

    // Use generous minimum width to utilize available taskbar space
    int min_width = screen_width / 4;
    return (std::max)(content_width, min_width);
}

void COutlookCalendarItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
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

    // Right align everything as a group within the allocated width 'w'
    int start_x = x + w - total_content_width;
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
    CRect value_rect(start_x + label_size.cx, y, x + w, y + h);
    pDC->DrawText(text.c_str(), -1, &value_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    
    // Reset scrolling state
    m_scroll_offset = 0;
    m_scroll_reverse = false;
    m_pause_end_time = 0;
}
