//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_CURLWRAPPER_H
#define CRAWLPAPER_CURLWRAPPER_H

#endif //CRAWLPAPER_CURLWRAPPER_H
#pragma once
#include <string>
#include <curl/curl.h>

class CurlWrapper {
public:
    CurlWrapper();
    ~CurlWrapper();

    bool download_file(const std::string& url, const std::string& output_path);
    std::string download_string(const std::string& url);

private:
    CURL* curl_;

    static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream);
    static size_t write_string(void* ptr, size_t size, size_t nmemb, std::string* stream);

    CurlWrapper(const CurlWrapper&) = delete;
    CurlWrapper& operator=(const CurlWrapper&) = delete;
};