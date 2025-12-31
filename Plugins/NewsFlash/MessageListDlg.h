#pragma once
#include "afxdialogex.h"
#include "DataManager.h"

// CMessageListDlg dialog
class CMessageListDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMessageListDlg)

public:
    CMessageListDlg(CWnd* pParent = nullptr);
    virtual ~CMessageListDlg();

    enum { IDD = IDD_MESSAGE_LIST_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnNMDblclkUnreadList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkReadList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedMarkRead();
    afx_msg void OnBnClickedClearAll();

private:
    void LoadUnreadMessages();
    void LoadReadMessages();
    void OpenNewsUrl(const std::wstring& news_id);

private:
    CListCtrl m_unread_list;
    CListCtrl m_read_list;
    std::vector<std::shared_ptr<NewsItem>> m_unread_news;
    std::vector<std::shared_ptr<NewsItem>> m_read_news;
};
