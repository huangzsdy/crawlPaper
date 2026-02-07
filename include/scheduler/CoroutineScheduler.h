//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_COROUTINESCHEDULER_H
#define CRAWLPAPER_COROUTINESCHEDULER_H

#endif //CRAWLPAPER_COROUTINESCHEDULER_H


#pragma once
#include "Scheduler.h"
#include <coroutine>
#include <atomic>

struct CrawlTask {
    struct promise_type {
        CrawlTask get_return_object() {
            return CrawlTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;
};

class CoroutineScheduler : public Scheduler {
public:
    CoroutineScheduler(size_t max_concurrent = 100);
    ~CoroutineScheduler();

    void schedule_crawl(const std::string& source,
                        const std::vector<std::string>& categories) override;
    void wait_completion() override;
    void stop() override;
    bool is_running() const override;

    size_t get_completed_count() const override;
    size_t get_failed_count() const override;
    size_t get_queued_count() const override;

private:
    std::atomic<size_t> active_coroutines_{0};
    std::atomic<size_t> max_concurrent_;
    std::atomic<bool> stop_{false};
};};