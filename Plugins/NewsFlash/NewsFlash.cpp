#include "pch.h"
#include "NewsFlash.h"
#include "DataManager.h"
#include "OptionsDlg.h"

CNewsFlash CNewsFlash::m_instance;

CNewsFlash::CNewsFlash()
    : m_pApp(nullptr)
{
}

CNewsFlash& CNewsFlash::Instance()
{
    return m_instance;
}

IPluginItem* CNewsFlash::GetItem(int index)
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

const wchar_t* CNewsFlash::GetTooltipInfo()
{
    return m_tooltip_info.c_str();
}

void CNewsFlash::DataRequired()
{
    // 更新显示数据
    m_item.UpdateDisplayText();
    
    // Update tooltip
    auto latest_news = g_data.GetLatestNews(2);
    m_tooltip_info.clear();
    for (const auto& news : latest_news)
    {
        if (!m_tooltip_info.empty())
            m_tooltip_info += L"\n\n";
        m_tooltip_info += news->source_prefix + L" ";
        m_tooltip_info += news->title.empty() ? news->content : news->title;
        if (g_data.m_setting_data.show_time && !news->time_str.empty())
        {
            m_tooltip_info += L"\nTime: " + news->time_str;
        }
    }
}

ITMPlugin::OptionReturn CNewsFlash::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    COptionsDlg dlg(CWnd::FromHandle((HWND)hParent));
    if (dlg.DoModal() == IDOK)
    {
        g_data.SaveConfig();
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

const wchar_t* CNewsFlash::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"Financial News Flash";
    case TMI_DESCRIPTION:
        return L"Display real-time financial news from multiple sources";
    case TMI_AUTHOR:
        return L"Your Name";
    case TMI_COPYRIGHT:
        return L"Copyright (C) 2025";
    case ITMPlugin::TMI_URL:
        return L"";
    case TMI_VERSION:
        return L"1.00";
    default:
        break;
    }
    return L"";
}

void CNewsFlash::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        // 从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));
        break;
    case ITMPlugin::EI_LABEL_TEXT_COLOR:
    {
        if (data != nullptr && *data != L'\0')
        {
            wchar_t* end_ptr = nullptr;
            unsigned long value = wcstoul(data, &end_ptr, 10);
            // 如果十进制解析失败或有多余字符，再尝试按16进制解析
            if (end_ptr == data || (end_ptr != nullptr && *end_ptr != L'\0'))
                value = wcstoul(data, nullptr, 16);
            g_data.m_label_text_color = static_cast<COLORREF>(value);
        }
        break;
    }
    case ITMPlugin::EI_VALUE_TEXT_COLOR:
    {
        if (data != nullptr && *data != L'\0')
        {
            wchar_t* end_ptr = nullptr;
            unsigned long value = wcstoul(data, &end_ptr, 10);
            if (end_ptr == data || (end_ptr != nullptr && *end_ptr != L'\0'))
                value = wcstoul(data, nullptr, 16);
            g_data.m_value_text_color = static_cast<COLORREF>(value);
        }
        break;
    }
    default:
        break;
    }
}

void CNewsFlash::OnInitialize(ITrafficMonitor* pApp)
{
    m_pApp = pApp;
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CNewsFlash::Instance();
}
