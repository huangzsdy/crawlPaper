//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_SCHEDULER_H
#define CRAWLPAPER_SCHEDULER_H

#endif //CRAWLPAPER_SCHEDULER_H

#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include "Paper.h"

class Scheduler {
public:
    using PaperCallback = std::function<void(const Paper&)>;
    using ProgressCallback = std::function<void(size_t, size_t, const std::string&)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;

    virtual ~Scheduler() = default;

    // Interface methods
    virtual void schedule_crawl(const std::string& source,
                                const std::vector<std::string>& categories) = 0;
    virtual void wait_completion() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;

    // Statistics
    virtual size_t get_completed_count() const = 0;
    virtual size_t get_failed_count() const = 0;
    virtual size_t get_queued_count() const = 0;

    // Callback setters
    void set_paper_callback(PaperCallback callback) { paper_callback_ = callback; }
    void set_progress_callback(ProgressCallback callback) { progress_callback_ = callback; }
    void set_error_callback(ErrorCallback callback) { error_callback_ = callback; }

    // Factory method
    static std::unique_ptr<Scheduler> create(const std::string& mode);

protected:
    PaperCallback paper_callback_;
    ProgressCallback progress_callback_;
    ErrorCallback error_callback_;

    // Helper methods
    void notify_paper(const Paper& paper);
    void notify_progress(size_t completed, size_t total, const std::string& message);
    void notify_error(const std::string& source, const std::string& error);
};
};