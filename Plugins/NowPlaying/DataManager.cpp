#include "pch.h"
#include "DataManager.h"

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
}

CDataManager::~CDataManager()
{
    for (auto& icon : m_icons)
    {
        if (icon.second != NULL)
            DestroyIcon(icon.second);
    }
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

const CString& CDataManager::StringRes(UINT id)
{
    if (m_string_table.find(id) == m_string_table.end())
    {
        CString str;
        str.LoadString(id);
        m_string_table[id] = str;
    }
    return m_string_table[id];
}

void CDataManager::DPIFromWindow(CWnd* pWnd)
{
    if (pWnd != nullptr)
    {
        CWindowDC dc(pWnd);
        m_dpi = dc.GetDeviceCaps(LOGPIXELSX);
    }
}

int CDataManager::DPI(int pixel)
{
    return pixel * m_dpi / 96;
}

float CDataManager::DPIF(float pixel)
{
    return pixel * m_dpi / 96.0f;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

HICON CDataManager::GetIcon(UINT id)
{
    if (m_icons.find(id) == m_icons.end())
    {
        m_icons[id] = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, 0, 0, 0);
    }
    return m_icons[id];
}
