#include "pch.h"
#include "NewsFlashItem.h"
#include "MessageListDlg.h"
#include "OptionsDlg.h"
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

CNewsFlashItem::CNewsFlashItem()
    : m_line1_height(0), m_line2_height(0)
{
    m_item_name = L"News Flash";
    UpdateDisplayText();
}

CNewsFlashItem::~CNewsFlashItem()
{
}

const wchar_t* CNewsFlashItem::GetItemName() const
{
    return m_item_name.c_str();
}

const wchar_t* CNewsFlashItem::GetItemId() const
{
    return L"news_flash";
}

const wchar_t* CNewsFlashItem::GetItemLableText() const
{
    return L"";  // 不使用标签，使用自定义绘制
}

const wchar_t* CNewsFlashItem::GetItemValueText() const
{
    return L"";  // 不使用值文本，使用自定义绘制
}

const wchar_t* CNewsFlashItem::GetItemValueSampleText() const
{
    return L"[News] Sample message for width calculation...";
}

bool CNewsFlashItem::IsCustomDraw() const
{
    return true;  // 使用自定义绘制
}

int CNewsFlashItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    
    // Fixed width 200 pixels
    return 200;
}

void CNewsFlashItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CRect rect(x, y, x + w, y + h);
    
    // Set colors - background is now transparent (no fill)
    // 正文颜色：优先使用主程序传来的颜色，否则根据深浅模式
    COLORREF text_color;
    if (g_data.m_value_text_color != 0)
        text_color = g_data.m_value_text_color;
    else
        text_color = dark_mode ? RGB(220, 220, 220) : RGB(0, 0, 0);

    // 前缀颜色：保持之前的蓝色，不跟随主程序
    COLORREF prefix_color = dark_mode ? RGB(100, 200, 255) : RGB(0, 100, 200);
    
    // 未读徽标颜色：红色强调
    COLORREF unread_color = dark_mode ? RGB(255, 100, 100) : RGB(200, 0, 0);
    
    // Set transparent text background
    pDC->SetBkMode(TRANSPARENT);
    
    // Set default font
    CFont* pOldFont = nullptr;
    CFont font;
    LOGFONT lf = {0};
    lf.lfHeight = -12;  // Font size
    lf.lfWeight = FW_NORMAL;
    lstrcpy(lf.lfFaceName, _T("Segoe UI"));
    font.CreateFontIndirect(&lf);
    pOldFont = pDC->SelectObject(&font);
    
    // Get unread count and draw badge
    auto unread_news = g_data.GetUnreadNews();
    int unread_count = (int)unread_news.size();
    int right_limit = x + w - 5; // 文字区域右边界，预留给未读徽标
    if (unread_count > 0)
    {
        CString badge;
        badge.Format(_T("(%d)"), unread_count);
        pDC->SetTextColor(unread_color);
        CSize badge_size = pDC->GetTextExtent(badge);

        // 徽标区域靠右，上下留少量间距
        int badge_padding = 3;
        CRect badge_rect(right_limit - badge_size.cx - badge_padding,
                         y + 2,
                         right_limit,
                         y + badge_size.cy + 2);
        pDC->DrawText(badge, badge_rect, DT_RIGHT | DT_TOP | DT_SINGLELINE);

        // 为避免文字与徽标重叠，整体内容区域右边界向左收缩徽标宽度
        right_limit -= (badge_size.cx + badge_padding * 2);
        if (right_limit < x + 50) // 保底，防止极端情况下文字区域为负
            right_limit = x + 50;
    }
    
    // 获取最新2条消息
    auto latest_news = g_data.GetLatestNews(2);
    
    int line_height = h / 2;
    int current_y = y;
    
    m_line1_height = m_line2_height = 0;
    m_line1_news.reset();
    m_line2_news.reset();
    
    for (size_t i = 0; i < latest_news.size() && i < 2; i++)
    {
        auto& news = latest_news[i];
        
        // 绘制前缀
        CRect prefix_rect(x + 5, current_y, right_limit, current_y + line_height);
        pDC->SetTextColor(prefix_color);
        CString prefix_str(news->source_prefix.c_str());
        pDC->DrawText(prefix_str, prefix_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        // 计算前缀宽度
        CSize prefix_size = pDC->GetTextExtent(CString(news->source_prefix.c_str()));
        
        // 绘制消息内容
        CRect content_rect(x + 10 + prefix_size.cx, current_y, right_limit, current_y + line_height);
        pDC->SetTextColor(text_color);
        
        // 截断过长的消息
        std::wstring display_text = news->title.empty() ? news->content : news->title;
        if (display_text.length() > 50)
        {
            display_text = display_text.substr(0, 47) + L"...";
        }
        
        CString content_str(display_text.c_str());
        pDC->DrawText(content_str, content_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        // 保存点击区域信息
        if (i == 0)
        {
            m_line1_height = line_height;
            m_line1_news = news;
        }
        else if (i == 1)
        {
            m_line2_height = line_height;
            m_line2_news = news;
        }
        
        current_y += line_height;
    }
    
    // If no news, draw placeholder text
    if (latest_news.empty())
    {
        pDC->SetTextColor(text_color);
        CString placeholder = _T("Loading news...");
        pDC->DrawText(placeholder, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    // Restore old font
    if (pOldFont)
        pDC->SelectObject(pOldFont);
}

int CNewsFlashItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag)
{
    if (type == MT_LCLICKED)
    {
        // 确定点击了哪一行
        std::shared_ptr<NewsItem> clicked_news;
        if (y < m_line1_height && m_line1_news)
        {
            clicked_news = m_line1_news;
        }
        else if (m_line2_news)
        {
            clicked_news = m_line2_news;
        }
        
        if (clicked_news)
        {
            // 打开浏览器
            ShellExecuteW(NULL, L"open", clicked_news->url.c_str(), NULL, NULL, SW_SHOWNORMAL);
            
            // 标记为已读
            g_data.MarkAsRead(clicked_news->id);
            
            return 1;  // 已处理
        }
    }
    else if (type == MT_RCLICKED)
    {
        // Show context menu
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        CMenu menu;
        menu.CreatePopupMenu();
        
        menu.AppendMenu(MF_STRING, (UINT_PTR)1, _T("View All Messages"));
        menu.AppendMenu(MF_SEPARATOR, 0);
        
        // Determine which line was clicked
        std::shared_ptr<NewsItem> clicked_news;
        if (y < m_line1_height && m_line1_news)
            clicked_news = m_line1_news;
        else if (m_line2_news)
            clicked_news = m_line2_news;
        
        if (clicked_news && !clicked_news->is_read)
        {
            menu.AppendMenu(MF_STRING, (UINT_PTR)2, _T("Mark as Read"));
        }
        menu.AppendMenu(MF_STRING, (UINT_PTR)3, _T("Mark All as Read"));
        menu.AppendMenu(MF_SEPARATOR, 0);
        menu.AppendMenu(MF_STRING, (UINT_PTR)4, _T("Settings..."));
        
        // Show menu
        POINT pt;
        GetCursorPos(&pt);
        int cmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, CWnd::FromHandle((HWND)hWnd));
        
        if (cmd == 1)
        {
            // Show message list dialog
            CMessageListDlg dlg(CWnd::FromHandle((HWND)hWnd));
            dlg.DoModal();
        }
        else if (cmd == 2 && clicked_news)
        {
            // Mark current message as read
            g_data.MarkAsRead(clicked_news->id);
        }
        else if (cmd == 3)
        {
            // Mark all as read
            g_data.MarkAllAsRead();
        }
        else if (cmd == 4)
        {
            // Show settings dialog
            COptionsDlg dlg(CWnd::FromHandle((HWND)hWnd));
            if (dlg.DoModal() == IDOK)
            {
                g_data.SaveConfig();
            }
        }
        
        return 1;  // Handled
    }
    
    return 0;
}

void CNewsFlashItem::UpdateDisplayText()
{
    // 更新显示的消息
    m_display_news = g_data.GetLatestNews(2);
    
    // 更新 tooltip
    m_tooltip_text.clear();
    for (const auto& news : m_display_news)
    {
        if (!m_tooltip_text.empty())
            m_tooltip_text += L"\n\n";
        m_tooltip_text += news->source_prefix + L" " + (news->title.empty() ? news->content : news->title);
    }
}
