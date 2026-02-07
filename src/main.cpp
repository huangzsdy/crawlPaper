//
// Created by huang on 2026/2/8.
//
#include <iostream>
#include <memory>
#include <string>
#include "config/CrawlerConfig.h"
#include "scheduler/Scheduler.h"
#include "storage/DataStorage.h"

int main(int argc, char* argv[]) {
    try {
        // Load configuration
        auto& config = CrawlerConfig::getInstance();
        if (!config.loadConfig("config/config.toml")) {
            std::cerr << "Failed to load configuration file" << std::endl;
            return 1;
        }

        // Initialize storage
        auto storage = std::make_unique<DataStorage>();

        // Create scheduler based on configuration
        auto scheduler = Scheduler::create(config.getCrawlerSettings().mode);

        // Set paper processing callback
        scheduler->set_paper_callback([&storage](const Paper& paper) {
            storage->save_paper(paper);
        });

        // Schedule crawling tasks for each source
        const auto& keywords = config.getKeywords();

        // Schedule arXiv crawling
        scheduler->schedule_crawl("arxiv", keywords.at("physics"));
        scheduler->schedule_crawl("arxiv", keywords.at("materials"));
        scheduler->schedule_crawl("arxiv", keywords.at("chemistry"));

        // Schedule bioRxiv crawling
        scheduler->schedule_crawl("biorxiv", keywords.at("biology"));

        // Schedule ChemRxiv crawling
        scheduler->schedule_crawl("chemrxiv", keywords.at("chemistry"));

        // Electrical engineering
        scheduler->schedule_crawl("arxiv", keywords.at("electrical_engineering"));

        // Wait for completion
        scheduler->wait_completion();
        std::cout << "Crawling completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}