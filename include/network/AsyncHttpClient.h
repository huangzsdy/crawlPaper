//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_ASYNCHTTPCLIENT_H
#define CRAWLPAPER_ASYNCHTTPCLIENT_H

#endif //CRAWLPAPER_ASYNCHTTPCLIENT_H
#pragma once
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <curl/curl.h>
#include "HttpClient.h"

class AsyncHttpClient {
public:
    using Task = std::function<void()>;

    AsyncHttpClient(size_t thread_count = std::thread::hardware_concurrency());
    ~AsyncHttpClient();

    // Async operations
    void get(const std::string& url, HttpClient::ResponseCallback callback);
    void post(const std::string& url, const std::string& data, HttpClient::ResponseCallback callback);
    void get_json(const std::string& url, HttpClient::JsonCallback callback);

    // Batch operations
    void batch_get(const std::vector<std::string>& urls,
                   std::function<void(const std::vector<std::string>&)> callback);

    // Control methods
    void wait_completion();
    void stop();

private:
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_tasks_{0};

    void worker_thread();
    void add_task(Task task);

    // HTTP client instance
    HttpClient http_client_;
};