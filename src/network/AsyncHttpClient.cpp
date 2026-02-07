#include "network/AsyncHttpClient.h"
#include <iostream>
#include <chrono>

AsyncHttpClient::AsyncHttpClient(size_t thread_count)
        : stop_(false), active_tasks_(0) {

    // 创建工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_thread(); });
    }
}

AsyncHttpClient::~AsyncHttpClient() {
    stop();
}

void AsyncHttpClient::worker_thread() {
    while (!stop_.load() || !tasks_.empty()) {
        Task task;

        {
            std::unique_lock lock(queue_mutex_);
            condition_.wait(lock, [this] {
                return stop_.load() || !tasks_.empty();
            });

            if (stop_.load() && tasks_.empty()) {
                break;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
            active_tasks_++;
        }

        // 执行任务
        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "AsyncHttpClient task error: " << e.what() << std::endl;
            }
        }

        active_tasks_--;
    }
}

void AsyncHttpClient::get(const std::string& url, HttpClient::ResponseCallback callback) {
    add_task([this, url, callback]() {
        http_client_.async_get(url, callback);
    });
}

void AsyncHttpClient::post(const std::string& url, const std::string& data,
                           HttpClient::ResponseCallback callback) {
    add_task([this, url, data, callback]() {
        auto response = http_client_.post(url, data);
        if (callback) {
            callback(response.value_or(""), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    });
}

void AsyncHttpClient::get_json(const std::string& url, HttpClient::JsonCallback callback) {
    add_task([this, url, callback]() {
        auto response = http_client_.getJson(url);
        if (callback) {
            callback(response.value_or(nlohmann::json()), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    });
}

void HttpClient::async_get(const std::string& url, ResponseCallback callback) {
    // 使用线程池或异步机制实现真正的异步操作
    std::thread([this, url, callback]() {
        auto response = get(url);
        if (callback) {
            callback(response.value_or(""), response ? CURLE_OK : CURLE_HTTP_RETURNED_ERROR);
        }
    }).detach();
}

void AsyncHttpClient::batch_get(const std::vector<std::string>& urls,
                                std::function<void(const std::vector<std::string>&)> callback) {
    add_task([this, urls, callback]() {
        std::vector<std::string> results;
        results.reserve(urls.size());

        for (const auto& url : urls) {
            auto response = http_client_.get(url);
            results.push_back(response.value_or(""));

            // 添加延迟以避免过于频繁的请求
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (callback) {
            callback(results);
        }
    });
}

void AsyncHttpClient::add_task(Task task) {
    {
        std::lock_guard lock(queue_mutex_);
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}

void AsyncHttpClient::wait_completion() {
    while (active_tasks_.load() > 0 || !tasks_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void AsyncHttpClient::stop() {
    stop_.store(true);
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}