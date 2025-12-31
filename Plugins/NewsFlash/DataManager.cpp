#include "pch.h"
#include "DataManager.h"
#include <algorithm>
#include <winhttp.h>
#include "../../utilities/yyjson/yyjson.h"

#pragma comment(lib, "winhttp.lib")

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
    : m_fetch_thread(nullptr), m_stop_fetching(false)
{
}

CDataManager::~CDataManager()
{
    StopFetching();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    m_config_path = config_dir;
    
    // 加载配置文件
    std::wstring config_file = config_dir + L"\\NewsFlash.ini";
    
    // 简单的INI文件读取（后续可优化）
    // 这里先使用默认配置
    
    // 加载消息源
    if (m_setting_data.sources.empty())
    {
        LoadDefaultSources();
    }
    
    // 清空当前消息列表，准备从订阅源拉取真实数据
    {
        std::lock_guard<std::mutex> lock(m_news_mutex);
        m_all_news.clear();
    }
    
    // 加载消息缓存
    LoadNewsCache();
    
    // 启动后台获取线程
    StartFetching();
}

void CDataManager::SaveConfig() const
{
    // 简化版本，后续可优化为INI文件存储
    // SaveNewsCache();  // 暂时注释，因为const函数问题
}

void CDataManager::LoadDefaultSources()
{
    // 当前版本不再内置默认消息源，保留为空，完全由用户在 Setting 界面自行添加
}

// Add some test data for immediate display
void CDataManager::AddTestData()
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    
    time_t now = time(nullptr);
    
    // Test news 1 - System
    auto news1 = std::make_shared<NewsItem>();
    news1->id = L"test_001";
    news1->title = L"Welcome to News Flash Plugin";
    news1->content = L"Welcome to News Flash Plugin";
    news1->source_name = L"System";
    news1->source_prefix = L"[Info]";
    news1->url = L"https://github.com";
    news1->time_str = L"Just now";
    news1->timestamp = now;
    news1->is_read = false;
    m_all_news.push_back(news1);
    
    // Test news 2 - System
    auto news2 = std::make_shared<NewsItem>();
    news2->id = L"test_002";
    news2->title = L"Plugin is working correctly";
    news2->content = L"Plugin is working correctly";
    news2->source_name = L"System";
    news2->source_prefix = L"[OK]";
    news2->url = L"https://github.com";
    news2->time_str = L"1 min ago";
    news2->timestamp = now - 60;
    news2->is_read = false;
    m_all_news.push_back(news2);
    
    // Test news 3 - Finance
    auto news3 = std::make_shared<NewsItem>();
    news3->id = L"test_003";
    news3->title = L"Markets open higher today";
    news3->content = L"Markets open higher today";
    news3->source_name = L"Finance";
    news3->source_prefix = L"[Market]";
    news3->url = L"https://finance.example.com";
    news3->time_str = L"5 min ago";
    news3->timestamp = now - 300;
    news3->is_read = false;
    m_all_news.push_back(news3);
    
    // Test news 4 - Tech
    auto news4 = std::make_shared<NewsItem>();
    news4->id = L"test_004";
    news4->title = L"New technology breakthrough announced";
    news4->content = L"New technology breakthrough announced";
    news4->source_name = L"Tech News";
    news4->source_prefix = L"[Tech]";
    news4->url = L"https://tech.example.com";
    news4->time_str = L"10 min ago";
    news4->timestamp = now - 600;
    news4->is_read = false;
    m_all_news.push_back(news4);
    
    // Test news 5 - Finance (already read)
    auto news5 = std::make_shared<NewsItem>();
    news5->id = L"test_005";
    news5->title = L"Stock market analysis report";
    news5->content = L"Stock market analysis report";
    news5->source_name = L"Finance";
    news5->source_prefix = L"[Market]";
    news5->url = L"https://finance.example.com";
    news5->time_str = L"30 min ago";
    news5->timestamp = now - 1800;
    news5->is_read = true;
    m_all_news.push_back(news5);
    
    // Test news 6 - Tech (already read)
    auto news6 = std::make_shared<NewsItem>();
    news6->id = L"test_006";
    news6->title = L"Software update available";
    news6->content = L"Software update available";
    news6->source_name = L"Tech News";
    news6->source_prefix = L"[Tech]";
    news6->url = L"https://tech.example.com";
    news6->time_str = L"1 hour ago";
    news6->timestamp = now - 3600;
    news6->is_read = true;
    m_all_news.push_back(news6);
}

const CString& CDataManager::StringRes(UINT id)
{
    if (m_string_table.find(id) != m_string_table.end())
        return m_string_table[id];

    CString str;
    str.LoadString(id);
    m_string_table[id] = str;
    return m_string_table[id];
}

void CDataManager::DPIFromWindow(CWnd* pWnd)
{
    if (pWnd != nullptr && pWnd->GetSafeHwnd() != NULL)
    {
        HDC hDC = ::GetDC(pWnd->GetSafeHwnd());
        m_dpi = GetDeviceCaps(hDC, LOGPIXELSX);
        ::ReleaseDC(pWnd->GetSafeHwnd(), hDC);
    }
}

int CDataManager::DPI(int pixel)
{
    return pixel * m_dpi / 96;
}

float CDataManager::DPIF(float pixel)
{
    return pixel * m_dpi / 96.0f;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

HICON CDataManager::GetIcon(UINT id)
{
    if (m_icons.find(id) != m_icons.end())
        return m_icons[id];

    HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    m_icons[id] = hIcon;
    return hIcon;
}

// 启动后台获取线程
void CDataManager::StartFetching()
{
    if (m_fetch_thread == nullptr)
    {
        m_stop_fetching = false;
        m_fetch_thread = AfxBeginThread(FetchThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL);
    }
}

// 停止后台获取线程
void CDataManager::StopFetching()
{
    m_stop_fetching = true;
    if (m_fetch_thread != nullptr)
    {
        WaitForSingleObject(m_fetch_thread->m_hThread, 5000);
        m_fetch_thread = nullptr;
    }
}

// 后台获取线程
UINT CDataManager::FetchThreadProc(LPVOID pParam)
{
    CDataManager* pThis = (CDataManager*)pParam;
    
    while (!pThis->m_stop_fetching)
    {
        pThis->FetchAllSources();
        
        // 等待一段时间再继续
        for (int i = 0; i < 10 && !pThis->m_stop_fetching; i++)
        {
            Sleep(1000);  // 每秒检查一次是否需要停止
        }
    }
    
    return 0;
}

// 获取所有消息源
void CDataManager::FetchAllSources()
{
    time_t now = time(nullptr);
    
    for (const auto& source : m_setting_data.sources)
    {
        if (!source.enabled)
            continue;
            
        // 检查是否到了更新时间
        if (now - source.last_update >= source.interval)
        {
            FetchSource(source);
            const_cast<NewsSource&>(source).last_update = now;
        }
    }
}

// 获取单个消息源
void CDataManager::FetchSource(const NewsSource& source)
{
    std::wstring response;
    if (!HttpGet(source.url, response))
        return;
        
    std::vector<std::shared_ptr<NewsItem>> new_news;
    bool ok = false;

    // 根据类型选择解析方式
    if (source.type == L"rss")
    {
        ok = ParseRssNews(response, source, new_news);
    }
    else
    {
        ok = ParseJsonNews(response, source, new_news);
    }

    if (ok)
    {
        std::lock_guard<std::mutex> lock(m_news_mutex);
        
        // 添加新消息到列表
        for (auto& news : new_news)
        {
            // 检查是否已存在
            bool exists = false;
            for (const auto& existing : m_all_news)
            {
                if (existing->id == news->id)
                {
                    exists = true;
                    break;
                }
            }
            
            if (!exists)
            {
                m_all_news.insert(m_all_news.begin(), news);
            }
        }
        
        // 限制历史记录数量
        if (m_all_news.size() > (size_t)m_setting_data.max_history_count)
        {
            m_all_news.resize(m_setting_data.max_history_count);
        }
        
        // 按时间排序
        std::sort(m_all_news.begin(), m_all_news.end(), 
            [](const std::shared_ptr<NewsItem>& a, const std::shared_ptr<NewsItem>& b) {
                return a->timestamp > b->timestamp;
            });
    }
}

// 获取最新消息
std::vector<std::shared_ptr<NewsItem>> CDataManager::GetLatestNews(int count)
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    std::vector<std::shared_ptr<NewsItem>> result;
    
    int added = 0;
    for (const auto& news : m_all_news)
    {
        if (added >= count)
            break;
        result.push_back(news);
        added++;
    }
    
    return result;
}

// 获取未读消息
std::vector<std::shared_ptr<NewsItem>> CDataManager::GetUnreadNews()
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    std::vector<std::shared_ptr<NewsItem>> result;
    
    for (const auto& news : m_all_news)
    {
        if (!news->is_read)
        {
            result.push_back(news);
        }
    }
    
    return result;
}

// 获取已读消息
std::vector<std::shared_ptr<NewsItem>> CDataManager::GetReadNews()
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    std::vector<std::shared_ptr<NewsItem>> result;
    
    for (const auto& news : m_all_news)
    {
        if (news->is_read)
        {
            result.push_back(news);
        }
    }
    
    return result;
}

// 标记为已读
void CDataManager::MarkAsRead(const std::wstring& news_id)
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    
    for (auto& news : m_all_news)
    {
        if (news->id == news_id)
        {
            news->is_read = true;
            break;
        }
    }
    
    SaveNewsCache();
}

// 标记全部为已读
void CDataManager::MarkAllAsRead()
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    
    for (auto& news : m_all_news)
    {
        news->is_read = true;
    }
    
    SaveNewsCache();
}

// 根据ID获取消息
std::shared_ptr<NewsItem> CDataManager::GetNewsById(const std::wstring& id)
{
    std::lock_guard<std::mutex> lock(m_news_mutex);
    
    for (const auto& news : m_all_news)
    {
        if (news->id == id)
        {
            return news;
        }
    }
    
    return nullptr;
}

// HTTP GET请求
bool CDataManager::HttpGet(const std::wstring& url, std::wstring& response)
{
    // Parse URL
    URL_COMPONENTS urlComp = { 0 };
    urlComp.dwStructSize = sizeof(urlComp);
    WCHAR szHostName[256] = { 0 };
    WCHAR szUrlPath[1024] = { 0 };
    urlComp.lpszHostName = szHostName;
    urlComp.dwHostNameLength = _countof(szHostName);
    urlComp.lpszUrlPath = szUrlPath;
    urlComp.dwUrlPathLength = _countof(szUrlPath);

    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp))
        return false;

    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(L"NewsFlash/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession)
        return false;

    // Connect to server
    HINTERNET hConnect = WinHttpConnect(hSession, szHostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Create request
    DWORD dwFlags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", szUrlPath,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        dwFlags);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Set timeout (10 seconds)
    DWORD dwTimeout = 10000;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));
    WinHttpSetOption(hRequest, WINHTTP_OPTION_RECEIVE_TIMEOUT, &dwTimeout, sizeof(dwTimeout));

    // Send request
    BOOL bResult = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResult)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Receive response
    bResult = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResult)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Read data
    response.clear();
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BYTE* pszOutBuffer = nullptr;

    do
    {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
            break;

        if (dwSize == 0)
            break;

        pszOutBuffer = new BYTE[dwSize + 1];
        ZeroMemory(pszOutBuffer, dwSize + 1);

        if (!WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded))
        {
            delete[] pszOutBuffer;
            break;
        }

        // Convert UTF-8 to wstring
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)pszOutBuffer, dwDownloaded, NULL, 0);
        if (len > 0)
        {
            WCHAR* wbuf = new WCHAR[len + 1];
            MultiByteToWideChar(CP_UTF8, 0, (LPCCH)pszOutBuffer, dwDownloaded, wbuf, len);
            wbuf[len] = 0;
            response += wbuf;
            delete[] wbuf;
        }

        delete[] pszOutBuffer;

    } while (dwSize > 0);

    // Cleanup
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return !response.empty();
}

bool CDataManager::ParseJsonNews(const std::wstring& json, const NewsSource& source, 
                                std::vector<std::shared_ptr<NewsItem>>& news_list)
{
    // Convert wstring to UTF-8 string
    int len = WideCharToMultiByte(CP_UTF8, 0, json.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0)
        return false;
    
    char* utf8_json = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, json.c_str(), -1, utf8_json, len, NULL, NULL);
    
    // Parse JSON
    yyjson_doc* doc = yyjson_read(utf8_json, strlen(utf8_json), 0);
    delete[] utf8_json;
    
    if (!doc)
        return false;
    
    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!root)
    {
        yyjson_doc_free(doc);
        return false;
    }
    
    // Get data array based on json_data_path
    std::string data_path_utf8;
    int path_len = WideCharToMultiByte(CP_UTF8, 0, source.json_data_path.c_str(), -1, NULL, 0, NULL, NULL);
    if (path_len > 0)
    {
        char* path_buf = new char[path_len];
        WideCharToMultiByte(CP_UTF8, 0, source.json_data_path.c_str(), -1, path_buf, path_len, NULL, NULL);
        data_path_utf8 = path_buf;
        delete[] path_buf;
    }
    
    yyjson_val* data_array = yyjson_obj_get(root, data_path_utf8.c_str());
    if (!data_array || !yyjson_is_arr(data_array))
    {
        yyjson_doc_free(doc);
        return false;
    }
    
    // Convert field names to UTF-8
    std::string id_field, title_field, time_field, url_field;
    auto wstr_to_utf8 = [](const std::wstring& wstr) -> std::string {
        int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (len <= 0) return "";
        char* buf = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf, len, NULL, NULL);
        std::string result = buf;
        delete[] buf;
        return result;
    };
    
    id_field = wstr_to_utf8(source.json_id_field);
    title_field = wstr_to_utf8(source.json_title_field);
    time_field = wstr_to_utf8(source.json_time_field);
    url_field = wstr_to_utf8(source.json_url_field);
    
    // Iterate through array
    yyjson_val* item;
    yyjson_arr_iter iter;
    yyjson_arr_iter_init(data_array, &iter);
    
    while ((item = yyjson_arr_iter_next(&iter)))
    {
        auto news = std::make_shared<NewsItem>();
        
        // Parse ID
        yyjson_val* id_val = yyjson_obj_get(item, id_field.c_str());
        if (id_val && yyjson_is_str(id_val))
        {
            const char* id_str = yyjson_get_str(id_val);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, id_str, -1, NULL, 0);
            if (wlen > 0)
            {
                WCHAR* wbuf = new WCHAR[wlen];
                MultiByteToWideChar(CP_UTF8, 0, id_str, -1, wbuf, wlen);
                news->id = wbuf;
                delete[] wbuf;
            }
        }
        
        // Parse title/content
        yyjson_val* title_val = yyjson_obj_get(item, title_field.c_str());
        if (title_val && yyjson_is_str(title_val))
        {
            const char* title_str = yyjson_get_str(title_val);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, title_str, -1, NULL, 0);
            if (wlen > 0)
            {
                WCHAR* wbuf = new WCHAR[wlen];
                MultiByteToWideChar(CP_UTF8, 0, title_str, -1, wbuf, wlen);
                news->title = wbuf;
                news->content = wbuf;  // Use same content
                delete[] wbuf;
            }
        }
        
        // Parse time
        yyjson_val* time_val = yyjson_obj_get(item, time_field.c_str());
        if (time_val && yyjson_is_str(time_val))
        {
            const char* time_str = yyjson_get_str(time_val);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, time_str, -1, NULL, 0);
            if (wlen > 0)
            {
                WCHAR* wbuf = new WCHAR[wlen];
                MultiByteToWideChar(CP_UTF8, 0, time_str, -1, wbuf, wlen);
                news->time_str = wbuf;
                delete[] wbuf;
            }
        }
        
        // Parse URL
        yyjson_val* url_val = yyjson_obj_get(item, url_field.c_str());
        if (url_val && yyjson_is_str(url_val))
        {
            const char* url_str = yyjson_get_str(url_val);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, url_str, -1, NULL, 0);
            if (wlen > 0)
            {
                WCHAR* wbuf = new WCHAR[wlen];
                MultiByteToWideChar(CP_UTF8, 0, url_str, -1, wbuf, wlen);
                news->url = wbuf;
                delete[] wbuf;
            }
        }
        
        // Set source info
        news->source_name = source.name;
        news->source_prefix = source.prefix;
        news->timestamp = time(nullptr);
        news->is_read = false;
        
        // Add to list if has valid ID
        if (!news->id.empty())
        {
            news_list.push_back(news);
        }
    }
    
    yyjson_doc_free(doc);
    return !news_list.empty();
}

// RSS 解析：简单解析常见 RSS (<item>) 结构
bool CDataManager::ParseRssNews(const std::wstring& xml, const NewsSource& source,
                                std::vector<std::shared_ptr<NewsItem>>& news_list)
{
    std::wstring lower_xml = xml;
    // 为了简单，这里不做严格 XML 解析，只做字符串查找，适合常规 RSS 源

    auto find_tag = [](const std::wstring& text, const std::wstring& tag, size_t start) -> std::wstring {
        // 支持 <tag>value</tag> 和 <tag attr="...">value</tag> 这两种形式
        std::wstring open_prefix = L"<" + tag;
        std::wstring close = L"</" + tag + L">";

        size_t pos1 = text.find(open_prefix, start);
        if (pos1 == std::wstring::npos)
            return L"";

        // 跳过可能存在的属性，定位到 '>' 之后
        pos1 = text.find(L">", pos1);
        if (pos1 == std::wstring::npos)
            return L"";
        pos1 += 1;

        size_t pos2 = text.find(close, pos1);
        if (pos2 == std::wstring::npos)
            return L"";

        return text.substr(pos1, pos2 - pos1);
    };

    auto strip_cdata = [](std::wstring s) -> std::wstring {
        const std::wstring cdata_start = L"<![CDATA[";
        const std::wstring cdata_end = L"]]>";
        if (s.size() >= cdata_start.size() + cdata_end.size() &&
            s.find(cdata_start) == 0)
        {
            s = s.substr(cdata_start.size());
            size_t pos_end = s.rfind(cdata_end);
            if (pos_end != std::wstring::npos)
                s = s.substr(0, pos_end);
        }
        return s;
    };

    size_t pos = 0;
    int count = 0;
    const int max_items = 50;

    while (count < max_items)
    {
        size_t item_start = xml.find(L"<item", pos);
        if (item_start == std::wstring::npos)
            break;
        size_t item_body_start = xml.find(L">", item_start);
        if (item_body_start == std::wstring::npos)
            break;
        item_body_start += 1;
        size_t item_end = xml.find(L"</item>", item_body_start);
        if (item_end == std::wstring::npos)
            break;

        std::wstring item_xml = xml.substr(item_body_start, item_end - item_body_start);

        std::wstring title = strip_cdata(find_tag(item_xml, L"title", 0));
        std::wstring description = strip_cdata(find_tag(item_xml, L"description", 0));
        std::wstring link = strip_cdata(find_tag(item_xml, L"link", 0));
        std::wstring guid = strip_cdata(find_tag(item_xml, L"guid", 0));
        std::wstring pubDate = strip_cdata(find_tag(item_xml, L"pubDate", 0));

        if (title.empty() && link.empty())
        {
            pos = item_end + 7; // strlen("</item>")
            continue;
        }

        auto news = std::make_shared<NewsItem>();

        // ID 优先使用 guid，其次 link，再次 title
        if (!guid.empty())
            news->id = guid;
        else if (!link.empty())
            news->id = link;
        else
            news->id = title;

        news->title = title;
        // 内容优先使用 description，其次使用 title
        if (!description.empty())
            news->content = description;
        else
            news->content = title;
        news->url = link;
        news->time_str = pubDate;
        news->source_name = source.name;
        news->source_prefix = source.prefix;
        news->timestamp = time(nullptr);
        news->is_read = false;

        news_list.push_back(news);
        ++count;

        pos = item_end + 7;
    }

    return !news_list.empty();
}

// 保存消息缓存
void CDataManager::SaveNewsCache()
{
    // TODO: 实现消息缓存保存
}

// 加载消息缓存
void CDataManager::LoadNewsCache()
{
    // TODO: 实现消息缓存加载
}
