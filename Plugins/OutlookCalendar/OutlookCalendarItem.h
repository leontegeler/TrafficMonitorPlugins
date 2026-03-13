#pragma once
#include "PluginInterface.h"

class COutlookCalendarItem : public IPluginItem
{
public:
    COutlookCalendarItem();
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;

private:
    mutable int m_scroll_offset{ 0 };
    mutable DWORD m_last_scroll_time{ 0 };
};
