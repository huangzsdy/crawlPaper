#include "scheduler/ProcessScheduler.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

ProcessScheduler::ProcessScheduler(size_t max_processes)
        : max_processes_(max_processes), stop_(false) {
}

ProcessScheduler::~ProcessScheduler() {
    stop();
}

void ProcessScheduler::schedule_crawl(const std::string& source,
                                      const std::vector<std::string>& categories) {
    // 创建子进程来执行爬取任务
    pid_t pid = fork();

    if (pid == 0) {
        // 子进程
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
                // 使用标准错误输出
                std::cerr << "Failed to fetch data from " << source << std::endl;
                exit(1);
            }

            // 解析论文
            auto papers = parser->parse_papers(*content);
            for (auto& paper : papers) {
                // 将论文输出到标准输出，父进程会读取
                std::cout << paper.to_json().dump() << std::endl;
            }

            exit(0);
        } catch (const std::exception& e) {
            std::cerr << "Exception in child process: " << e.what() << std::endl;
            exit(1);
        }
    } else if (pid > 0) {
        // 父进程
        child_processes_.push_back(pid);
    } else {
        notify_error(source, "Failed to create child process");
    }
}

void ProcessScheduler::wait_completion() {
    for (pid_t pid : child_processes_) {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // 子进程成功退出
            notify_progress(1, 1, "Child process " + std::to_string(pid) + " completed");
        } else {
            notify_error("Child process " + std::to_string(pid), "Exited with error");
        }
    }
    child_processes_.clear();
}

void ProcessScheduler::stop() {
    stop_.store(true);

    // 终止所有子进程
    for (pid_t pid : child_processes_) {
        kill(pid, SIGTERM);
    }

    // 等待子进程终止
    wait_completion();
}

bool ProcessScheduler::is_running() const {
    return !stop_.load() && !child_processes_.empty();
}

size_t ProcessScheduler::get_completed_count() const {
    // 简化的实现
    return 0;
}

size_t ProcessScheduler::get_failed_count() const {
    // 简化的实现
    return 0;
}

size_t ProcessScheduler::get_queued_count() const {
    return child_processes_.size();
}