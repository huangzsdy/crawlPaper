#include "scheduler/Scheduler.h"
#include "scheduler/ThreadScheduler.h"
#include "scheduler/CoroutineScheduler.h"
#include "scheduler/ProcessScheduler.h"
#include <stdexcept>

std::unique_ptr<Scheduler> Scheduler::create(const std::string& mode) {
    if (mode == "thread") {
        return std::make_unique<ThreadScheduler>();
    } else if (mode == "coroutine") {
        return std::make_unique<CoroutineScheduler>();
    } else if (mode == "process") {
        return std::make_unique<ProcessScheduler>();
    }
    throw std::invalid_argument("Unknown scheduler mode: " + mode);
}

void Scheduler::notify_paper(const Paper& paper) {
    if (paper_callback_) {
        paper_callback_(paper);
    }
}

void Scheduler::notify_progress(size_t completed, size_t total, const std::string& message) {
    if (progress_callback_) {
        progress_callback_(completed, total, message);
    }
}

void Scheduler::notify_error(const std::string& source, const std::string& error) {
    if (error_callback_) {
        error_callback_(source, error);
    }
}