#pragma once
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class HttpClient {
public:
    using ResponseCallback = std::function<void(const std::string&, CURLcode)>;
    using JsonCallback = std::function<void(const nlohmann::json&, CURLcode)>;

    HttpClient();
    ~HttpClient();

    // Synchronous methods
    std::optional<std::string> get(const std::string& url);
    std::optional<std::string> post(const std::string& url, const std::string& data);
    std::optional<nlohmann::json> getJson(const std::string& url);

    // Asynchronous methods
    void async_get(const std::string& url, ResponseCallback callback);
    void async_post(const std::string& url, const std::string& data, ResponseCallback callback);
    void async_get_json(const std::string& url, JsonCallback callback);

    // Configuration
    void set_timeout(int timeout) { timeout_ = timeout; }
    void set_user_agent(const std::string& user_agent) { user_agent_ = user_agent; }
    void set_retry_count(int retry_count) { retry_count_ = retry_count; }
    void set_proxy(const std::string& proxy) { proxy_ = proxy; }

    // Headers management
    void add_header(const std::string& header);
    void clear_headers();

private:
    CURL* curl_;
    int timeout_;
    int retry_count_;
    std::string user_agent_;
    std::string proxy_;
    struct curl_slist* headers_;

    // Callback data structure
    struct WriteCallbackData {
        std::string data;
        std::function<void(const std::string&, CURLcode)> callback;
    };

    // Static callbacks for libcurl
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata);

    // Helper methods
    void setup_curl_options(CURL* curl);
    std::optional<std::string> perform_request(CURL* curl);

    // Modern C++: disable copying
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
};