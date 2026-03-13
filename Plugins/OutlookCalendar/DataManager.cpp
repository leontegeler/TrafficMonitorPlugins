#include "pch.h"
#include "DataManager.h"
#include <iomanip>
#include <sstream>

// Import the Office shared library
#import "libid:2DF8D04C-5BFA-101B-BDE5-00AA0044DE52" \
    rename("RGB", "MSORGB") \
    rename("DocumentProperties", "MSODocumentProperties")

// Import the Outlook Type Library
#import "libid:00062FFF-0000-0000-C000-000000000046" \
    rename_namespace("Outlook") \
    rename("CopyFile", "OutlookCopyFile") \
    rename("ReplaceText", "OutlookReplaceText") \
    rename("GetOrganizer", "OutlookGetOrganizer") \
    rename("GetOnlineMeetingProvider", "OutlookGetOnlineMeetingProvider")

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    //初始化DPI
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);

    m_update_thread = std::thread(&CDataManager::UpdateThreadFunc, this);
}

CDataManager::~CDataManager()
{
    m_exit_thread = true;
    if (m_update_thread.joinable())
        m_update_thread.join();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

const CString& CDataManager::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_table[id].LoadString(id);
        return m_string_table[id];
    }
}

void CDataManager::DPIFromWindow(CWnd* pWnd)
{
    CWindowDC dc(pWnd);
    HDC hDC = dc.GetSafeHdc();
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}

int CDataManager::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel)
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

HICON CDataManager::GetIcon(UINT id)
{
    auto iter = m_icons.find(id);
    if (iter != m_icons.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, DPI(16), DPI(16), 0);
        m_icons[id] = hIcon;
        return hIcon;
    }
}

const wchar_t* CDataManager::GetNextEventDisplayString()
{
    std::lock_guard<std::mutex> lock(m_data_mutex);
    return m_display_string.c_str();
}

void CDataManager::UpdateData()
{
    std::lock_guard<std::mutex> lock(m_data_mutex);
    if (!m_next_appointment.has_event)
    {
        m_display_string = L"No upcoming events";
        return;
    }

    std::time_t now = std::time(nullptr);
    int diff_minutes = static_cast<int>(std::difftime(m_next_appointment.start_time, now) / 60);

    std::wstringstream ss;
    std::wstring subject = m_next_appointment.subject;
    ss << subject << L" ";

    if (diff_minutes <= 0)
    {
        int end_diff = static_cast<int>(std::difftime(m_next_appointment.end_time, now) / 60);
        if (end_diff > 0)
            ss << L"(ends in " << end_diff << L"m)";
        else
        {
            m_display_string = L"No upcoming events";
            return;
        }
    }
    else if (diff_minutes < 60)
    {
        ss << L"(in " << diff_minutes << L"m)";
    }
    else if (diff_minutes < 1440) // Less than 24 hours
    {
        int hours = diff_minutes / 60;
        int mins = diff_minutes % 60;
        ss << L"(in " << hours << L"h " << mins << L"m)";
    }
    else // More than 24 hours
    {
        int days = diff_minutes / 1440;
        int hours = (diff_minutes % 1440) / 60;
        ss << L"(in " << days << L"d " << hours << L"h)";
    }

    m_display_string = ss.str();
}

void CDataManager::ForceUpdate()
{
    m_last_update_time = 0;
}

void CDataManager::UpdateThreadFunc()
{
    while (!m_exit_thread)
    {
        std::time_t now = std::time(nullptr);
        if (m_last_update_time == 0 || std::difftime(now, m_last_update_time) >= 60)
        {
            FetchFromOutlook();
            m_last_update_time = now;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static std::time_t DateToTimeT(DATE date)
{
    SYSTEMTIME st;
    VariantTimeToSystemTime(date, &st);
    struct tm tm_date = { 0 };
    tm_date.tm_year = st.wYear - 1900;
    tm_date.tm_mon = st.wMonth - 1;
    tm_date.tm_mday = st.wDay;
    tm_date.tm_hour = st.wHour;
    tm_date.tm_min = st.wMinute;
    tm_date.tm_sec = st.wSecond;
    tm_date.tm_isdst = -1;
    return std::mktime(&tm_date);
}

void CDataManager::FetchFromOutlook()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return;

    std::wstring debug_info;
    try
    {
        Outlook::_ApplicationPtr pApp(L"Outlook.Application");
        Outlook::_NameSpacePtr pNS = pApp->GetNamespace(L"MAPI");
        Outlook::MAPIFolderPtr pCalendar = pNS->GetDefaultFolder(Outlook::olFolderCalendar);
        Outlook::_ItemsPtr pItems = pCalendar->Items;

        std::time_t t_now = std::time(nullptr);
        std::tm tm_now;
        localtime_s(&tm_now, &t_now);

        auto FormatTimeISO = [](const std::tm& tm) {
            std::wstringstream ss;
            ss << (tm.tm_year + 1900) << L"-"
               << std::setw(2) << std::setfill(L'0') << (tm.tm_mon + 1) << L"-"
               << std::setw(2) << std::setfill(L'0') << tm.tm_mday << L" "
               << std::setw(2) << std::setfill(L'0') << tm.tm_hour << L":"
               << std::setw(2) << std::setfill(L'0') << tm.tm_min;
            return ss.str();
        };

        std::wstringstream filter;
        filter << L"@SQL=(\"urn:schemas:calendar:dtend\" >= '" << FormatTimeISO(tm_now) << L"')";

        pItems->IncludeRecurrences = VARIANT_TRUE;
        pItems->Sort(L"[Start]", vtMissing);

        Outlook::_ItemsPtr pRestrictedItems = pItems->Restrict(filter.str().c_str());
        
        AppointmentInfo next_appt;
        bool found = false;
        long restricted_count = pRestrictedItems->Count;

        m_fetch_count++;
        debug_info = L"Fetch count: ";
        debug_info += std::to_wstring(m_fetch_count);
        debug_info += L"\nLast fetch: ";
        wchar_t time_buf[64];
        wcsftime(time_buf, 64, L"%H:%M:%S", &tm_now);
        debug_info += time_buf;
        debug_info += L"\nTotal in folder: ";
        debug_info += std::to_wstring(pItems->Count);
        debug_info += L"\nRestricted: ";
        debug_info += std::to_wstring(restricted_count);

        Outlook::_AppointmentItemPtr pAppt = pRestrictedItems->GetFirst();
        int checked = 0;
        
        while (pAppt != nullptr && checked < 500)
        {
            checked++;
            std::wstring subject = (wchar_t*)pAppt->Subject;
            if (subject.find(L"Abgesagt") == std::wstring::npos)
            {
                std::time_t start = DateToTimeT(pAppt->Start);
                std::time_t end = DateToTimeT(pAppt->End);

                if (end > t_now)
                {
                    next_appt.subject = subject;
                    next_appt.start_time = start;
                    next_appt.end_time = end;
                    next_appt.has_event = true;
                    found = true;
                    break;
                }
            }
            pAppt = pRestrictedItems->GetNext();
        }

        std::lock_guard<std::mutex> lock(m_data_mutex);
        m_next_appointment = next_appt;
        m_tooltip_info = debug_info;
        if (found)
        {
            m_tooltip_info += L"\nNext: ";
            m_tooltip_info += m_next_appointment.subject;
        }
        else
        {
            m_tooltip_info += L"\n(No events found)";
        }
    }
    catch (_com_error& e)
    {
        std::lock_guard<std::mutex> lock(m_data_mutex);
        m_tooltip_info = L"COM Error: ";
        m_tooltip_info += e.ErrorMessage();
        m_next_appointment.has_event = false;
    }

    CoUninitialize();
}
