#include "pch.h"
#include "OptionsDlg.h"
#include "resource.h"
#include "EditSourceDlg.h"

IMPLEMENT_DYNAMIC(COptionsDlg, CDialogEx)

COptionsDlg::COptionsDlg(CWnd* pParent)
    : CDialogEx(IDD_OPTIONS_DIALOG, pParent)
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SOURCE_LIST, m_source_list);
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialogEx)
    ON_BN_CLICKED(IDC_ADD_SOURCE, &COptionsDlg::OnBnClickedAddSource)
    ON_BN_CLICKED(IDC_EDIT_SOURCE, &COptionsDlg::OnBnClickedEditSource)
    ON_BN_CLICKED(IDC_DELETE_SOURCE, &COptionsDlg::OnBnClickedDeleteSource)
    ON_LBN_SELCHANGE(IDC_SOURCE_LIST, &COptionsDlg::OnLbnSelchangeSourceList)
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Load current sources
    m_sources = g_data.m_setting_data.sources;
    LoadSourceList();
    UpdateButtons();

    return TRUE;
}

void COptionsDlg::LoadSourceList()
{
    m_source_list.ResetContent();
    
    for (const auto& source : m_sources)
    {
        CString display_text;
        display_text.Format(_T("%s - %s (%s)"), 
            source.prefix.c_str(),
            source.name.c_str(),
            source.enabled ? _T("Enabled") : _T("Disabled"));
        m_source_list.AddString(display_text);
    }
}

void COptionsDlg::UpdateButtons()
{
    int sel = m_source_list.GetCurSel();
    GetDlgItem(IDC_EDIT_SOURCE)->EnableWindow(sel != LB_ERR);
    GetDlgItem(IDC_DELETE_SOURCE)->EnableWindow(sel != LB_ERR);
}

void COptionsDlg::OnBnClickedAddSource()
{
    // 新增订阅源
    NewsSource new_source;
    new_source.id = L"custom_" + std::to_wstring(time(nullptr));
    new_source.type = L"http_json";
    new_source.interval = 60;
    new_source.enabled = true;

    CEditSourceDlg dlg(new_source, this);
    if (dlg.DoModal() == IDOK)
    {
        m_sources.push_back(new_source);
        LoadSourceList();
        UpdateButtons();
    }
}

void COptionsDlg::OnBnClickedEditSource()
{
    int sel = m_source_list.GetCurSel();
    if (sel == LB_ERR)
        return;
    if (sel < 0 || sel >= (int)m_sources.size())
        return;

    NewsSource& src = m_sources[sel];
    CEditSourceDlg dlg(src, this);
    if (dlg.DoModal() == IDOK)
    {
        LoadSourceList();
        UpdateButtons();
    }
}

void COptionsDlg::OnBnClickedDeleteSource()
{
    int sel = m_source_list.GetCurSel();
    if (sel == LB_ERR)
        return;
    
    if (AfxMessageBox(_T("Are you sure to delete this source?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        if (sel >= 0 && sel < (int)m_sources.size())
        {
            m_sources.erase(m_sources.begin() + sel);
            LoadSourceList();
            UpdateButtons();
        }
    }
}

void COptionsDlg::OnLbnSelchangeSourceList()
{
    UpdateButtons();
}

void COptionsDlg::OnOK()
{
    // Save sources to global data
    g_data.m_setting_data.sources = m_sources;
    
    CDialogEx::OnOK();
}
