#pragma once
#include "afxdialogex.h"
#include "DataManager.h"

// COptionsDlg dialog
class COptionsDlg : public CDialogEx
{
    DECLARE_DYNAMIC(COptionsDlg)

public:
    COptionsDlg(CWnd* pParent = nullptr);
    virtual ~COptionsDlg();

    enum { IDD = IDD_OPTIONS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedAddSource();
    afx_msg void OnBnClickedEditSource();
    afx_msg void OnBnClickedDeleteSource();
    afx_msg void OnLbnSelchangeSourceList();

private:
    void LoadSourceList();
    void UpdateButtons();

private:
    CListBox m_source_list;
    std::vector<NewsSource> m_sources;
};
