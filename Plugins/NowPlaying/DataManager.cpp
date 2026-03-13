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

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    m_config_path = config_dir + L"NowPlaying.ini";

    utilities::CIniHelper ini(m_config_path);
    m_setting_data.show_artist = ini.GetBool(L"settings", L"show_artist", true);
    m_setting_data.show_title = ini.GetBool(L"settings", L"show_title", true);
    m_setting_data.max_length = ini.GetInt(L"settings", L"max_length", 50);
    m_setting_data.cover_art_size = ini.GetInt(L"settings", L"cover_art_size", 48);
    m_setting_data.show_cover_art = ini.GetBool(L"settings", L"show_cover_art", true);
}

void CDataManager::SaveConfig() const
{
    utilities::CIniHelper ini(m_config_path);
    ini.WriteBool(L"settings", L"show_artist", m_setting_data.show_artist);
    ini.WriteBool(L"settings", L"show_title", m_setting_data.show_title);
    ini.WriteInt(L"settings", L"max_length", m_setting_data.max_length);
    ini.WriteInt(L"settings", L"cover_art_size", m_setting_data.cover_art_size);
    ini.WriteBool(L"settings", L"show_cover_art", m_setting_data.show_cover_art);
    ini.Save();
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
