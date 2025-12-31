#include "pch.h"
#include "EditSourceDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CEditSourceDlg, CDialogEx)

CEditSourceDlg::CEditSourceDlg(NewsSource& source, CWnd* pParent)
    : CDialogEx(IDD_EDIT_SOURCE_DIALOG, pParent)
    , m_source(source)
{
}

CEditSourceDlg::~CEditSourceDlg()
{
}

void CEditSourceDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEditSourceDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CEditSourceDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 初始化类型下拉框
    InitTypeCombo();

    // 名称
    SetDlgItemText(IDC_SOURCE_NAME, m_source.name.c_str());
    // 前缀
    SetDlgItemText(IDC_SOURCE_PREFIX, m_source.prefix.c_str());
    // URL
    SetDlgItemText(IDC_SOURCE_URL, m_source.url.c_str());
    // 轮询间隔
    if (m_source.interval <= 0)
        m_source.interval = 60;
    SetDlgItemInt(IDC_SOURCE_INTERVAL, m_source.interval, FALSE);
    // 启用
    CheckDlgButton(IDC_SOURCE_ENABLED, m_source.enabled ? BST_CHECKED : BST_UNCHECKED);

    // JSON 映射
    SetDlgItemText(IDC_JSON_DATA_PATH, m_source.json_data_path.c_str());
    SetDlgItemText(IDC_JSON_ID_FIELD, m_source.json_id_field.c_str());
    SetDlgItemText(IDC_JSON_TITLE_FIELD, m_source.json_title_field.c_str());
    SetDlgItemText(IDC_JSON_TIME_FIELD, m_source.json_time_field.c_str());
    SetDlgItemText(IDC_JSON_URL_FIELD, m_source.json_url_field.c_str());

    return TRUE;
}

void CEditSourceDlg::InitTypeCombo()
{
    CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_SOURCE_TYPE);
    if (pCombo == nullptr)
        return;

    // 下拉项文本为中文，内部类型使用固定字符串
    int indexHttpJson = pCombo->AddString(L"HTTP JSON \u63a5\u53e3");
    pCombo->SetItemDataPtr(indexHttpJson, (void*)L"http_json");

    int indexRss = pCombo->AddString(L"RSS \u8ba2\u9605\u6e90");
    pCombo->SetItemDataPtr(indexRss, (void*)L"rss");

    int indexWs = pCombo->AddString(L"WebSocket \u63a8\u9001");
    pCombo->SetItemDataPtr(indexWs, (void*)L"websocket");

    // 根据当前 source.type 选择
    std::wstring type = m_source.type;
    if (type.empty())
        type = L"http_json";

    int count = pCombo->GetCount();
    for (int i = 0; i < count; ++i)
    {
        const wchar_t* pType = (const wchar_t*)pCombo->GetItemDataPtr(i);
        if (pType != nullptr && type == pType)
        {
            pCombo->SetCurSel(i);
            return;
        }
    }

    // 默认选择 HTTP JSON
    pCombo->SetCurSel(indexHttpJson);
}

void CEditSourceDlg::OnOK()
{
    CString text;

    // 名称
    GetDlgItemText(IDC_SOURCE_NAME, text);
    m_source.name = (LPCTSTR)text;

    // 前缀
    GetDlgItemText(IDC_SOURCE_PREFIX, text);
    m_source.prefix = (LPCTSTR)text;

    // URL
    GetDlgItemText(IDC_SOURCE_URL, text);
    m_source.url = (LPCTSTR)text;

    // 轮询间隔
    BOOL bTrans = FALSE;
    UINT interval = GetDlgItemInt(IDC_SOURCE_INTERVAL, &bTrans, FALSE);
    if (!bTrans || interval == 0)
        interval = 60;
    m_source.interval = (int)interval;

    // 启用
    m_source.enabled = (IsDlgButtonChecked(IDC_SOURCE_ENABLED) == BST_CHECKED);

    // 类型
    CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_SOURCE_TYPE);
    if (pCombo != nullptr)
    {
        int sel = pCombo->GetCurSel();
        if (sel >= 0)
        {
            const wchar_t* pType = (const wchar_t*)pCombo->GetItemDataPtr(sel);
            if (pType != nullptr)
                m_source.type = pType;
        }
    }
    if (m_source.type.empty())
        m_source.type = L"http_json";

    // JSON 映射
    GetDlgItemText(IDC_JSON_DATA_PATH, text);
    m_source.json_data_path = (LPCTSTR)text;

    GetDlgItemText(IDC_JSON_ID_FIELD, text);
    m_source.json_id_field = (LPCTSTR)text;

    GetDlgItemText(IDC_JSON_TITLE_FIELD, text);
    m_source.json_title_field = (LPCTSTR)text;

    GetDlgItemText(IDC_JSON_TIME_FIELD, text);
    m_source.json_time_field = (LPCTSTR)text;

    GetDlgItemText(IDC_JSON_URL_FIELD, text);
    m_source.json_url_field = (LPCTSTR)text;

    CDialogEx::OnOK();
}
