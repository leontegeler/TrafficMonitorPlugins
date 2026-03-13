#pragma once
#include <string>
#include <map>
#include <ctime>
#include <thread>
#include <mutex>
#include <atomic>
#include "resource.h"

#define g_data CDataManager::Instance()

struct AppointmentInfo
{
    std::wstring subject;
    std::time_t start_time{ 0 };
    std::time_t end_time{ 0 };
    bool has_event{ false };
};

struct SettingData
{
    int refresh_interval{ 1 }; // minutes (default to 1 for better responsiveness)
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
    const CString& StringRes(UINT id);      //根据资源id获取一个字符串资源
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);
    HICON GetIcon(UINT id);

    const wchar_t* GetNextEventDisplayString();
    const std::wstring& GetTooltipInfo() const { return m_tooltip_info; }
    void UpdateData();
    void ForceUpdate();

    SettingData m_setting_data;

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    int m_dpi{ 96 };

    AppointmentInfo m_next_appointment;
    std::wstring m_display_string;
    std::wstring m_tooltip_info;
    int m_fetch_count{ 0 };
    std::mutex m_data_mutex;
    std::thread m_update_thread;
    std::atomic<bool> m_exit_thread{ false };
    std::time_t m_last_update_time{ 0 };

    void UpdateThreadFunc();
    void FetchFromOutlook();
};
