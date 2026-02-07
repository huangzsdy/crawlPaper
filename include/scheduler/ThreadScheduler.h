//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_THREADSCHEDULER_H
#define CRAWLPAPER_THREADSCHEDULER_H

#endif //CRAWLPAPER_THREADSCHEDULER_H

#pragma once
#include "Scheduler.h"
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

class ThreadScheduler : public Scheduler {
public:
    ThreadScheduler(size_t thread_count = std::thread::hardware_concurrency());
    ~ThreadScheduler();

    void schedule_crawl(const std::string& source,
                        const std::vector<std::string>& categories) override;
    void wait_completion() override;
    void stop() override;
    bool is_running() const override;

    size_t get_completed_count() const override;
    size_t get_failed_count() const override;
    size_t get_queued_count() const override;

private:
    void worker_thread();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_tasks_{0};
};