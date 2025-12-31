#pragma once
#include "../../include/PluginInterface.h"
#include "NewsFlashItem.h"
#include <string>

class CNewsFlash : public ITMPlugin
{
private:
    CNewsFlash();

public:
    static CNewsFlash& Instance();

    // ITMPlugin接口实现
    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    virtual void OnInitialize(ITrafficMonitor* pApp) override;

private:
    static CNewsFlash m_instance;
    CNewsFlashItem m_item;
    std::wstring m_tooltip_info;
    ITrafficMonitor* m_pApp;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
