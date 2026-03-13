#pragma once
#include "PluginInterface.h"
#include <string>

class CVpnItem : public IPluginItem
{
public:
    CVpnItem();
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;

    void SetText(const wchar_t* text);

public:
    std::wstring m_value;

private:
    mutable int m_scroll_offset{ 0 };
    mutable DWORD m_last_scroll_time{ 0 };
};
