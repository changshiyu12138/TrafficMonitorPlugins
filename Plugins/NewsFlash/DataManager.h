#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <ctime>
#include "resource.h"

#define g_data CDataManager::Instance()

// 消息项结构
struct NewsItem
{
    std::wstring id;            // 消息唯一ID
    std::wstring title;         // 消息标题
    std::wstring content;       // 消息内容
    std::wstring source_name;   // 消息源名称
    std::wstring source_prefix; // 消息源前缀（如 [金十]）
    std::wstring url;           // 消息链接
    std::wstring time_str;      // 时间字符串
    time_t timestamp;           // 时间戳
    bool is_read;               // 是否已读

    NewsItem() : timestamp(0), is_read(false) {}
};

// 消息源配置
struct NewsSource
{
    std::wstring id;            // 消息源唯一ID
    std::wstring name;          // 消息源名称
    std::wstring prefix;        // 显示前缀（如 [金十]）
    std::wstring url;           // API URL
    std::wstring type;          // 类型：http_json, rss等
    int interval;               // 轮询间隔（秒）
    bool enabled;               // 是否启用
    time_t last_update;         // 上次更新时间
    
    // JSON解析配置
    std::wstring json_data_path;    // 数据路径，如 "data.items"
    std::wstring json_id_field;     // ID字段名
    std::wstring json_title_field;  // 标题字段名
    std::wstring json_time_field;   // 时间字段名
    std::wstring json_url_field;    // URL字段名

    NewsSource() : interval(10), enabled(true), last_update(0) {}
};

// 设置数据
struct SettingData
{
    std::vector<NewsSource> sources;        // 消息源列表
    int max_display_length;                 // 最大显示长度
    int max_history_count;                  // 最大历史记录数
    bool show_time;                         // 是否显示时间
    
    SettingData() : max_display_length(100), max_history_count(100), show_time(true) {}
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    const CString& StringRes(UINT id);
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);
    HICON GetIcon(UINT id);

    // 消息管理
    void StartFetching();
    void StopFetching();
    void FetchAllSources();
    void FetchSource(const NewsSource& source);
    
    // 获取最新消息
    std::vector<std::shared_ptr<NewsItem>> GetLatestNews(int count = 2);
    std::vector<std::shared_ptr<NewsItem>> GetUnreadNews();
    std::vector<std::shared_ptr<NewsItem>> GetReadNews();
    
    // 标记已读
    void MarkAsRead(const std::wstring& news_id);
    void MarkAllAsRead();
    
    // 获取消息详情
    std::shared_ptr<NewsItem> GetNewsById(const std::wstring& id);

    // HTTP请求（简单实现）
    bool HttpGet(const std::wstring& url, std::wstring& response);
    
    // JSON解析辅助（简单实现）
    bool ParseJsonNews(const std::wstring& json, const NewsSource& source, 
                       std::vector<std::shared_ptr<NewsItem>>& news_list);
    // RSS解析辅助（简单实现）
    bool ParseRssNews(const std::wstring& xml, const NewsSource& source,
                      std::vector<std::shared_ptr<NewsItem>>& news_list);

    SettingData m_setting_data;

    // 文本颜色（由主程序通过扩展信息传递），0 表示未设置
    COLORREF m_label_text_color{ 0 };
    COLORREF m_value_text_color{ 0 };

private:
    void LoadDefaultSources();
    void AddTestData();
    void SaveNewsCache();
    void LoadNewsCache();
    static UINT FetchThreadProc(LPVOID pParam);

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    std::map<UINT, HICON> m_icons;
    int m_dpi{ 96 };
    
    // 消息数据
    std::vector<std::shared_ptr<NewsItem>> m_all_news;
    std::mutex m_news_mutex;
    
    // 后台线程
    CWinThread* m_fetch_thread;
    bool m_stop_fetching;
    std::map<std::wstring, time_t> m_last_news_id;  // 记录每个源的最后一条消息ID
};
