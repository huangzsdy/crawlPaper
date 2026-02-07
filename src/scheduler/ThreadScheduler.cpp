#include "scheduler/ThreadScheduler.h"
#include "network/HttpClient.h"
#include "parser/PaperParser.h"
#include <iostream>
#include <chrono>

ThreadScheduler::ThreadScheduler(size_t thread_count)
        : stop_(false), active_tasks_(0) {
    // 创建工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_thread(); });
    }
}

ThreadScheduler::~ThreadScheduler() {
    stop();
}

void ThreadScheduler::schedule_crawl(const std::string& source,
                                     const std::vector<std::string>& categories) {
    // 将爬取任务包装成函数对象添加到任务队列
    auto task = [this, source, categories]() {
        try {
            // 创建解析器和HTTP客户端
            auto parser = PaperParser::create(source);
            auto http_client = std::make_unique<HttpClient>();

            // 构建查询URL
            auto query = parser->build_query(categories, 0, 100);
            auto full_url = source == "arxiv" ?
                            "https://export.arxiv.org/api/query?" + query :
                            parser->get_source_name() + query;

            // 发送请求
            auto content = http_client->get(full_url);
            if (!content) {
                notify_error(source, "Failed to fetch data from " + source);
                return;
            }

            // 解析论文
            auto papers = parser->parse_papers(*content);
            for (auto& paper : papers) {
                notify_paper(paper);
            }

            // 更新进度
            notify_progress(1, 1, "Completed crawling " + source);
        } catch (const std::exception& e) {
            notify_error(source, "Exception occurred: " + std::string(e.what()));
        }
    };

    // 将任务加入队列
    {
        std::lock_guard lock(queue_mutex_);
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}

void ThreadScheduler::worker_thread() {
    while (true) {
        std::function<void()> task;

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

        task();
        active_tasks_--;
    }
}

void ThreadScheduler::wait_completion() {
    while (active_tasks_.load() > 0 || !tasks_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ThreadScheduler::stop() {
    stop_.store(true);
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

bool ThreadScheduler::is_running() const {
    return !stop_.load() && (active_tasks_.load() > 0 || !tasks_.empty());
}

size_t ThreadScheduler::get_completed_count() const {
    // 简化的实现，实际中可能需要更复杂的统计
    return 0;
}

size_t ThreadScheduler::get_failed_count() const {
    // 简化的实现，实际中可能需要更复杂的统计
    return 0;
}

size_t ThreadScheduler::get_queued_count() const {
    std::lock_guard lock(queue_mutex_);
    return tasks_.size();
}