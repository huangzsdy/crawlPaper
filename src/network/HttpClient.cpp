#include "network/HttpClient.h"
#include <iostream>
#include <sstream>
#include <chrono>

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (curl_) {
        setup_curl_options();
    }
}

HttpClient::~HttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

void HttpClient::setup_curl_options() {
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout_);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent_.c_str());
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
}

size_t HttpClient::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    size_t total_size = size * nmemb;
    response->append(ptr, total_size);
    return total_size;
}

size_t HttpClient::header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    return size * nitems;
}

std::optional<std::string> HttpClient::get(const std::string& url) {
    if (!curl_) {
        return std::nullopt;
    }

    std::string response_data;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

    // 设置自定义头部
    if (headers_) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
    }

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        std::cerr << "HTTP GET failed: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200) {
        std::cerr << "HTTP GET returned code: " << http_code << std::endl;
        return std::nullopt;
    }

    return response_data;
}

std::optional<std::string> HttpClient::post(const std::string& url, const std::string& data) {
    if (!curl_) {
        return std::nullopt;
    }

    std::string response_data;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());

    if (headers_) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
    }

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        std::cerr << "HTTP POST failed: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200) {
        std::cerr << "HTTP POST returned code: " << http_code << std::endl;
        return std::nullopt;
    }

    return response_data;
}

std::optional<nlohmann::json> HttpClient::getJson(const std::string& url) {
    auto response = get(url);
    if (!response) {
        return std::nullopt;
    }

    try {
        return nlohmann::json::parse(*response);
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

void HttpClient::async_get(const std::string& url, ResponseCallback callback) {
    // 在实际实现中，这里应该使用线程池
    std::thread([this, url, callback]() {
        auto response = get(url);
        if (callback) {
            callback(response.value_or(""), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    }).detach();
}

void HttpClient::async_post(const std::string& url, const std::string& data, ResponseCallback callback) {
    std::thread([this, url, data, callback]() {
        auto response = post(url, data);
        if (callback) {
            callback(response.value_or(""), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    }).detach();
}

void HttpClient::async_get_json(const std::string& url, JsonCallback callback) {
    std::thread([this, url, callback]() {
        auto response = getJson(url);
        if (callback) {
            callback(response.value_or(nlohmann::json()), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    }).detach();
}

void HttpClient::add_header(const std::string& header) {
    headers_ = curl_slist_append(headers_, header.c_str());
}

void HttpClient::clear_headers() {
    if (headers_) {
        curl_slist_free_all(headers_);
        headers_ = nullptr;
    }
}