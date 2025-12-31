#include "pch.h"
#include "MessageListDlg.h"
#include "resource.h"
#include <shellapi.h>

IMPLEMENT_DYNAMIC(CMessageListDlg, CDialogEx)

CMessageListDlg::CMessageListDlg(CWnd* pParent)
    : CDialogEx(IDD_MESSAGE_LIST_DIALOG, pParent)
{
}

CMessageListDlg::~CMessageListDlg()
{
}

void CMessageListDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_UNREAD_LIST, m_unread_list);
    DDX_Control(pDX, IDC_READ_LIST, m_read_list);
}

BEGIN_MESSAGE_MAP(CMessageListDlg, CDialogEx)
    ON_NOTIFY(NM_DBLCLK, IDC_UNREAD_LIST, &CMessageListDlg::OnNMDblclkUnreadList)
    ON_NOTIFY(NM_DBLCLK, IDC_READ_LIST, &CMessageListDlg::OnNMDblclkReadList)
    ON_BN_CLICKED(IDC_MARK_READ, &CMessageListDlg::OnBnClickedMarkRead)
    ON_BN_CLICKED(IDC_CLEAR_ALL, &CMessageListDlg::OnBnClickedClearAll)
END_MESSAGE_MAP()

BOOL CMessageListDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Setup unread list
    m_unread_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_unread_list.InsertColumn(0, L"Source", LVCFMT_LEFT, 80);
    m_unread_list.InsertColumn(1, L"Time", LVCFMT_LEFT, 100);
    m_unread_list.InsertColumn(2, L"Content", LVCFMT_LEFT, 300);

    // Setup read list
    m_read_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_read_list.InsertColumn(0, L"Source", LVCFMT_LEFT, 80);
    m_read_list.InsertColumn(1, L"Time", LVCFMT_LEFT, 100);
    m_read_list.InsertColumn(2, L"Content", LVCFMT_LEFT, 300);

    // Load messages
    LoadUnreadMessages();
    LoadReadMessages();

    return TRUE;
}

void CMessageListDlg::LoadUnreadMessages()
{
    m_unread_list.DeleteAllItems();
    m_unread_news = g_data.GetUnreadNews();
    
    int index = 0;
    for (const auto& news : m_unread_news)
    {
        m_unread_list.InsertItem(index, news->source_prefix.c_str());
        m_unread_list.SetItemText(index, 1, news->time_str.c_str());
        
        std::wstring content = news->title.empty() ? news->content : news->title;
        if (content.length() > 50)
            content = content.substr(0, 47) + L"...";
        m_unread_list.SetItemText(index, 2, content.c_str());
        
        m_unread_list.SetItemData(index, (DWORD_PTR)news.get());
        index++;
    }
}

void CMessageListDlg::LoadReadMessages()
{
    m_read_list.DeleteAllItems();
    m_read_news = g_data.GetReadNews();
    
    int index = 0;
    for (const auto& news : m_read_news)
    {
        m_read_list.InsertItem(index, news->source_prefix.c_str());
        m_read_list.SetItemText(index, 1, news->time_str.c_str());
        
        std::wstring content = news->title.empty() ? news->content : news->title;
        if (content.length() > 50)
            content = content.substr(0, 47) + L"...";
        m_read_list.SetItemText(index, 2, content.c_str());
        
        m_read_list.SetItemData(index, (DWORD_PTR)news.get());
        index++;
    }
}

void CMessageListDlg::OpenNewsUrl(const std::wstring& news_id)
{
    auto news = g_data.GetNewsById(news_id);
    if (news)
    {
        ShellExecuteW(NULL, L"open", news->url.c_str(), NULL, NULL, SW_SHOWNORMAL);
        
        if (!news->is_read)
        {
            g_data.MarkAsRead(news_id);
            LoadUnreadMessages();
            LoadReadMessages();
        }
    }
}

void CMessageListDlg::OnNMDblclkUnreadList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    
    int sel = pNMItemActivate->iItem;
    if (sel >= 0 && sel < (int)m_unread_news.size())
    {
        OpenNewsUrl(m_unread_news[sel]->id);
    }
    
    *pResult = 0;
}

void CMessageListDlg::OnNMDblclkReadList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    
    int sel = pNMItemActivate->iItem;
    if (sel >= 0 && sel < (int)m_read_news.size())
    {
        ShellExecuteW(NULL, L"open", m_read_news[sel]->url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    
    *pResult = 0;
}

void CMessageListDlg::OnBnClickedMarkRead()
{
    POSITION pos = m_unread_list.GetFirstSelectedItemPosition();
    if (pos != NULL)
    {
        int sel = m_unread_list.GetNextSelectedItem(pos);
        if (sel >= 0 && sel < (int)m_unread_news.size())
        {
            g_data.MarkAsRead(m_unread_news[sel]->id);
            LoadUnreadMessages();
            LoadReadMessages();
        }
    }
}

void CMessageListDlg::OnBnClickedClearAll()
{
    if (AfxMessageBox(L"Mark all messages as read?", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        g_data.MarkAllAsRead();
        LoadUnreadMessages();
        LoadReadMessages();
    }
}
