#include "pch.h"
#include "OutlookCalendar.h"
#include "DataManager.h"
#include "OptionsDlg.h"

COutlookCalendar COutlookCalendar::m_instance;

COutlookCalendar::COutlookCalendar()
{
}

COutlookCalendar& COutlookCalendar::Instance()
{
    return m_instance;
}

IPluginItem* COutlookCalendar::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* COutlookCalendar::GetTooltipInfo()
{
    return g_data.GetTooltipInfo().c_str();
}

void COutlookCalendar::DataRequired()
{
    g_data.UpdateData();
}

ITMPlugin::OptionReturn COutlookCalendar::ShowOptionsDialog(void* hParent)
{
    return ITMPlugin::OR_OPTION_NOT_PROVIDED;
}

const wchar_t* COutlookCalendar::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return g_data.StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION:
        return g_data.StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR:
        return L"Leon Tegeler";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Leon Tegeler 2026";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/zhongyang219/TrafficMonitorPlugins";
        break;
    case TMI_VERSION:
        return L"1.00";
    default:
        break;
    }
    return L"";
}

void COutlookCalendar::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
}

int COutlookCalendar::GetCommandCount()
{
    return 1;
}

const wchar_t* COutlookCalendar::GetCommandName(int command_index)
{
    if (command_index == 0)
        return L"Refresh Outlook Data";
    return nullptr;
}

void COutlookCalendar::OnPluginCommand(int command_index, void* hWnd, void* para)
{
    if (command_index == 0)
    {
        g_data.ForceUpdate();
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &COutlookCalendar::Instance();
}
