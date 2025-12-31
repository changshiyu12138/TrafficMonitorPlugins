#pragma once
#include "../../include/PluginInterface.h"
#include "DataManager.h"
#include <vector>
#include <memory>

class CNewsFlashItem : public IPluginItem
{
public:
    CNewsFlashItem();
    ~CNewsFlashItem();

    // IPluginItem接口实现
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    
    // 自定义绘制
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    
    // 鼠标事件
    virtual int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;

    void UpdateDisplayText();  // 更新显示文本

private:
    std::vector<std::shared_ptr<NewsItem>> m_display_news;  // 显示的消息（最多2条）
    mutable std::wstring m_item_name;
    mutable std::wstring m_tooltip_text;
    
    // 用于点击识别
    int m_line1_height;
    int m_line2_height;
    std::shared_ptr<NewsItem> m_line1_news;
    std::shared_ptr<NewsItem> m_line2_news;
};
