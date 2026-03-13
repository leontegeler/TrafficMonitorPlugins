#pragma once
#include "PluginInterface.h"
#include "VpnItem.h"
#include <string>

class CVpn : public ITMPlugin
{
private:
    CVpn();

public:
    static CVpn& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

private:

private:
    static CVpn m_instance;
    CVpnItem m_item;
    std::wstring m_tooltip_info;
    std::wstring m_last_vpn_name;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
