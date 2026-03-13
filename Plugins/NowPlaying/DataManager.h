#pragma once
#include <string>
#include <map>
#include "resource.h"

#define g_data CDataManager::Instance()

struct SettingData
{
    bool show_artist{ true };
    bool show_title{ true };
    int max_length{ 50 };
    int cover_art_size{ 48 };  // Size in pixels for cover art
    bool show_cover_art{ true };
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    const CString& StringRes(UINT id);
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);
    HICON GetIcon(UINT id);

    SettingData m_setting_data;

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    int m_dpi{ 96 };
};
