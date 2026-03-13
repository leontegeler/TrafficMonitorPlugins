#pragma once
#include <string>
#include <map>
#include "resource.h"

#define g_data CDataManager::Instance()

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    const CString& StringRes(UINT id);
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);
    HICON GetIcon(UINT id);

private:
    static CDataManager m_instance;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    int m_dpi{ 96 };
};
