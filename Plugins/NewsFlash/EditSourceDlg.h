#pragma once
#include "afxdialogex.h"
#include "DataManager.h"

// 编辑订阅源对话框
class CEditSourceDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CEditSourceDlg)

public:
    CEditSourceDlg(NewsSource& source, CWnd* pParent = nullptr);
    virtual ~CEditSourceDlg();

    enum { IDD = IDD_EDIT_SOURCE_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

private:
    void InitTypeCombo();

private:
    NewsSource& m_source;
};
