#include "scheduler/CoroutineScheduler.h"
#include "network/HttpClient.h"
#include "parser/PaperParser.h"
#include <iostream>
#include <chrono>

CoroutineScheduler::CoroutineScheduler(size_t max_concurrent)
        : max_concurrent_(max_concurrent), stop_(false) {
}

CoroutineScheduler::~CoroutineScheduler() {
    stop();
}

void CoroutineScheduler::schedule_crawl(const std::string& source,
                                        const std::vector<std::string>& categories) {
    // 在协程调度器中，我们启动一个协程来处理爬取任务
    // 注意：这里使用了C++20协程，需要编译器支持
    auto task = [this, source, categories]() -> CrawlTask {
        try {
            // 创建解析器和HTTP客户端
            auto parser = PaperParser::create(source);
            auto http_client = std::make_unique<HttpClient>();

            // 构建查询URL
            auto query = parser->build_query(categories, 0, 100);
            auto full_url = source == "arxiv" ?
                            "https://export.arxiv.org/api/query?" + query :
                            parser->get_source_name() + query;

            // 发送请求（这里应该是异步的，但为了简单起见，我们使用同步）
            // 在实际实现中，应该使用异步HTTP客户端
            auto content = http_client->get(full_url);
            if (!content) {
                notify_error(source, "Failed to fetch data from " + source);
                co_return;
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

        co_return;
    };

    // 启动协程
    task();
}

void CoroutineScheduler::wait_completion() {
    // 简化的实现：循环等待直到所有协程完成
    while (active_coroutines_.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CoroutineScheduler::stop() {
    stop_.store(true);
}

bool CoroutineScheduler::is_running() const {
    return !stop_.load() && active_coroutines_.load() > 0;
}

size_t CoroutineScheduler::get_completed_count() const {
    // 简化的实现
    return 0;
}

size_t CoroutineScheduler::get_failed_count() const {
    // 简化的实现
    return 0;
}

size_t CoroutineScheduler::get_queued_count() const {
    // 简化的实现
    return 0;
}