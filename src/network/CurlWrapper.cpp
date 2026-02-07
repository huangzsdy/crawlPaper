#include "network/CurlWrapper.h"
#include <iostream>
#include <fstream>

CurlWrapper::CurlWrapper() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
}

CurlWrapper::~CurlWrapper() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

bool CurlWrapper::download_file(const std::string& url, const std::string& output_path) {
    if (!curl_) {
        return false;
    }

    FILE* file = fopen(output_path.c_str(), "wb");
    if (!file) {
        std::cerr << "Failed to open file: " << output_path << std::endl;
        return false;
    }

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "ScientificCrawler/1.0");
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl_);
    fclose(file);

    if (res != CURLE_OK) {
        std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;
        remove(output_path.c_str());
        return false;
    }

    // 检查文件大小
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        remove(output_path.c_str());
        return false;
    }

    return true;
}

std::string CurlWrapper::download_string(const std::string& url) {
    if (!curl_) {
        return "";
    }

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_string);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "ScientificCrawler/1.0");
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        std::cerr << "Download string failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        return "";
    }

    return response;
}

std::optional<nlohmann::json> CurlWrapper::download_json(const std::string& url) {
    auto content = download_string(url);
    if (content.empty()) {
        return std::nullopt;
    }

    try {
        return nlohmann::json::parse(content);
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool CurlWrapper::set_proxy(const std::string& proxy) {
    if (!curl_) return false;

    CURLcode res = curl_easy_setopt(curl_, CURLOPT_PROXY, proxy.c_str());
    return res == CURLE_OK;
}

bool CurlWrapper::set_timeout(int timeout_seconds) {
    if (!curl_) return false;

    CURLcode res = curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout_seconds);
    return res == CURLE_OK;
}

bool CurlWrapper::set_connect_timeout(int timeout_seconds) {
    if (!curl_) return false;

    CURLcode res = curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeout_seconds);
    return res == CURLE_OK;
}

void CurlWrapper::add_header(const std::string& header) {
    if (!curl_) return;

    headers_ = curl_slist_append(headers_, header.c_str());
}

void CurlWrapper::clear_headers() {
    if (headers_) {
        curl_slist_free_all(headers_);
        headers_ = nullptr;
    }
}

std::string CurlWrapper::get_last_error() const {
    if (!curl_) return "CURL not initialized";
    return curl_easy_strerror(last_error_);
}

long CurlWrapper::get_response_code() const {
    if (!curl_) return -1;

    long response_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
    return response_code;
}

double CurlWrapper::get_total_time() const {
    if (!curl_) return -1;

    double total_time = 0;
    curl_easy_getinfo(curl_, CURLINFO_TOTAL_TIME, &total_time);
    return total_time;
}

size_t CurlWrapper::write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

size_t CurlWrapper::write_string(void* ptr, size_t size, size_t nmemb, std::string* stream) {
    size_t total_size = size * nmemb;
    stream->append(static_cast<char*>(ptr), total_size);
    return total_size;
}

bool CurlWrapper::perform_request() {
    if (!curl_) return false;

    // 设置头部
    if (headers_) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
    }

    last_error_ = curl_easy_perform(curl_);
    return last_error_ == CURLE_OK;
}