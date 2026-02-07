//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_PROCESSSCHEDULER_H
#define CRAWLPAPER_PROCESSSCHEDULER_H

#endif //CRAWLPAPER_PROCESSSCHEDULER_H

#pragma once
#include "Scheduler.h"
#include <vector>
#include <atomic>
#include <unistd.h>
#include <sys/types.h>

class ProcessScheduler : public Scheduler {
public:
    ProcessScheduler(size_t max_processes = 4);
    ~ProcessScheduler();

    void schedule_crawl(const std::string& source,
                        const std::vector<std::string>& categories) override;
    void wait_completion() override;
    void stop() override;
    bool is_running() const override;

    size_t get_completed_count() const override;
    size_t get_failed_count() const override;
    size_t get_queued_count() const override;

private:
    std::vector<pid_t> child_processes_;
    size_t max_processes_;
    std::atomic<bool> stop_{false};
};