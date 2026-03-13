#include "pch.h"
#include "Vpn.h"
#include "DataManager.h"
#include <ras.h>        // Required for RasEnumConnections
#include <raserror.h>   // Required for RAS errors
#pragma comment(lib, "rasapi32.lib") // Link with rasapi32.lib

CVpn CVpn::m_instance;

CVpn::CVpn()
{
}

CVpn& CVpn::Instance()
{
    return m_instance;
}

IPluginItem* CVpn::GetItem(int index)
{
    if (index == 0)
    {
        return &m_item;
    }
    return nullptr;
}

const wchar_t* CVpn::GetTooltipInfo()
{
    return m_tooltip_info.c_str();
}

void CVpn::DataRequired()
{
    DWORD dwCb = 0;
    DWORD dwConnections = 0;

    // First, call RasEnumConnections to get the required buffer size.
    DWORD dwRet = RasEnumConnections(NULL, &dwCb, &dwConnections);

    if (dwRet == ERROR_BUFFER_TOO_SMALL)
    {
        // If the buffer is too small, it means there are active connections.
        // Allocate the required memory.
        RASCONN* pRasConn = (RASCONN*)GlobalAlloc(GPTR, dwCb);
        if (pRasConn)
        {
            pRasConn->dwSize = sizeof(RASCONN);

            // Call RasEnumConnections again to get the actual connection data.
            if (RasEnumConnections(pRasConn, &dwCb, &dwConnections) == ERROR_SUCCESS)
            {
                if (dwConnections > 0)
                {
                    // For simplicity, we use the first connection found.
                    m_item.SetText(pRasConn[0].szEntryName);
                }
                else
                {
                    // This case should ideally not be reached if the first call indicated a buffer was needed,
                    // but we handle it just in case.
                    m_item.SetText(L"Disonnected");
                }
            }
            else
            {
                // The second call to RasEnumConnections failed.
                m_item.SetText(L"Error: Enum failed");
            }

            // Free the allocated memory.
            GlobalFree(pRasConn);
        }
        else
        {
            // Memory allocation failed.
            m_item.SetText(L"Error: Mem alloc");
        }
    }
    else if (dwRet == ERROR_SUCCESS)
    {
        // If the function succeeds on the first call, it means there are no active connections.
        m_item.SetText(L"Disonnected");
    }
    else
    {
        // An unexpected error occurred.
        m_item.SetText((L"Error: " + std::to_wstring(dwRet)).c_str());
    }
}

ITMPlugin::OptionReturn CVpn::ShowOptionsDialog(void* hParent)
{
    return ITMPlugin::OR_OPTION_NOT_PROVIDED;
}

const wchar_t* CVpn::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return L"VPN";
    case TMI_DESCRIPTION:
        return L"Displays the name of the active VPN connection.";
    case TMI_AUTHOR:
        return L"Leon Tegeler";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Leon Tegeler 2026";
    case ITMPlugin::TMI_URL:
        return L"";
    case TMI_VERSION:
        return L"1.00";
    default:
        break;
    }
    return L"";
}

void CVpn::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));
        break;
    default:
        break;
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CVpn::Instance();
}
