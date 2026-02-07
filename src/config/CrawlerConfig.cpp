#include "config/CrawlerConfig.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

using namespace std::filesystem;

bool CrawlerConfig::loadConfig(const std::string& config_path) {
    try {
        // 检查配置文件是否存在
        if (!exists(config_path)) {
            std::cerr << "Config file not found: " << config_path << std::endl;
            return false;
        }

        // 解析TOML文件
        auto config = toml::parse_file(config_path);

        // 解析爬虫基本配置
        parseCrawlerConfig(config);

        // 解析存储配置
        parseStorageConfig(config);

        // 解析各API配置
        parseApiConfigs(config);

        // 解析关键词映射
        parseKeywords(config);

        // 解析数据库配置（如果存在）
        parseDatabaseConfig(config);

        // 解析监控配置（如果存在）
        parseMonitoringConfig(config);

        std::cout << "Configuration loaded successfully from: " << config_path << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        return false;
    }
}

void CrawlerConfig::parseCrawlerConfig(const toml::table& config) {
    auto crawler_tbl = config["crawler"];
    if (!crawler_tbl) {
        std::cout << "Using default crawler settings" << std::endl;
        return;
    }

    // 解析爬虫模式
    std::string mode_str = crawler_tbl["mode"].value_or("thread");
    crawler_settings_.mode = stringToMode(mode_str);

    // 解析其他爬虫参数
    crawler_settings_.max_connections = crawler_tbl["max_connections"].value_or(10);
    crawler_settings_.request_timeout = crawler_tbl["request_timeout"].value_or(30);
    crawler_settings_.retry_attempts = crawler_tbl["retry_attempts"].value_or(3);
    crawler_settings_.delay_between_requests =
            crawler_tbl["delay_between_requests"].value_or(1.0);
    crawler_settings_.user_agent =
            crawler_tbl["user_agent"].value_or("AcademicCrawler/1.0");
    crawler_settings_.log_level = crawler_tbl["log_level"].value_or("info");
}

void CrawlerConfig::parseStorageConfig(const toml::table& config) {
    auto storage_tbl = config["storage"];
    if (!storage_tbl) {
        std::cout << "Using default storage settings" << std::endl;
        return;
    }

    storage_settings_.output_dir = storage_tbl["output_dir"].value_or("./data");
    storage_settings_.output_format = storage_tbl["output_format"].value_or("json");
    storage_settings_.max_file_size_mb = storage_tbl["max_file_size_mb"].value_or(100);
    storage_settings_.batch_size = storage_tbl["batch_size"].value_or(100);

    // 创建输出目录（如果不存在）
    create_directories(storage_settings_.output_dir);
}

void CrawlerConfig::parseApiConfigs(const toml::table& config) {
    // 解析arXiv配置
    parseArxivConfig(config);

    // 解析bioRxiv配置
    parseBiorxivConfig(config);

    // 解析ChemRxiv配置
    parseChemRxivConfig(config);
}

void CrawlerConfig::parseArxivConfig(const toml::table& config) {
    auto arxiv_tbl = config["arxiv"];
    if (!arxiv_tbl) {
        std::cout << "Using default arXiv settings" << std::endl;
        return;
    }

    arxiv_settings_.base_url =
            arxiv_tbl["base_url"].value_or("https://export.arxiv.org/api/query");

    // 解析分类列表
    if (arxiv_tbl["categories"].is_array()) {
        arxiv_settings_.categories.clear();
        for (const auto& category : *arxiv_tbl["categories"].as_array()) {
            if (category.is_string()) {
                arxiv_settings_.categories.push_back(category.value_or(""));
            }
        }
    }

    arxiv_settings_.max_results = arxiv_tbl["max_results"].value_or(1000);
    arxiv_settings_.update_interval_hours =
            arxiv_tbl["update_interval_hours"].value_or(24);
    arxiv_settings_.batch_size = arxiv_tbl["batch_size"].value_or(50);
}

void CrawlerConfig::parseBiorxivConfig(const toml::table& config) {
    auto biorxiv_tbl = config["biorxiv"];
    if (!biorxiv_tbl) {
        std::cout << "Using default bioRxiv settings" << std::endl;
        return;
    }

    biorxiv_settings_.base_url =
            biorxiv_tbl["base_url"].value_or("https://api.biorxiv.org/details/");

    if (biorxiv_tbl["categories"].is_array()) {
        biorxiv_settings_.categories.clear();
        for (const auto& category : *biorxiv_tbl["categories"].as_array()) {
            if (category.is_string()) {
                biorxiv_settings_.categories.push_back(category.value_or(""));
            }
        }
    }

    biorxiv_settings_.max_results = biorxiv_tbl["max_results"].value_or(500);
    biorxiv_settings_.update_interval_hours =
            biorxiv_tbl["update_interval_hours"].value_or(24);
}

void CrawlerConfig::parseChemRxivConfig(const toml::table& config) {
    auto chemrxiv_tbl = config["chemrxiv"];
    if (!chemrxiv_tbl) {
        std::cout << "Using default ChemRxiv settings" << std::endl;
        return;
    }

    chemrxiv_settings_.base_url =
            chemrxiv_tbl["base_url"].value_or(
                    "https://chemrxiv.org/engage/chemrxiv/public-api/v1/items");

    if (chemrxiv_tbl["categories"].is_array()) {
        chemrxiv_settings_.categories.clear();
        for (const auto& category : *chemrxiv_tbl["categories"].as_array()) {
            if (category.is_string()) {
                chemrxiv_settings_.categories.push_back(category.value_or(""));
            }
        }
    }

    chemrxiv_settings_.max_results = chemrxiv_tbl["max_results"].value_or(300);
    chemrxiv_settings_.update_interval_hours =
            chemrxiv_tbl["update_interval_hours"].value_or(24);
}

void CrawlerConfig::parseKeywords(const toml::table& config) {
    auto keywords_tbl = config["keywords"];
    if (!keywords_tbl) {
        std::cout << "Using default keyword mappings" << std::endl;
        return;
    }

    for (const auto& [key, value] : *keywords_tbl.as_table()) {
        if (value.is_array()) {
            keywords_[std::string(key)].clear();
            for (const auto& item : *value.as_array()) {
                if (item.is_string()) {
                    keywords_[std::string(key)].push_back(item.value_or(""));
                }
            }
        }
    }
}

void CrawlerConfig::parseDatabaseConfig(const toml::table& config) {
    auto database_tbl = config["database"];
    if (!database_tbl) {
        database_settings_.enabled = false;
        return;
    }

    database_settings_.enabled = database_tbl["enabled"].value_or(false);
    database_settings_.host = database_tbl["host"].value_or("localhost");
    database_settings_.port = database_tbl["port"].value_or(5432);
    database_settings_.name = database_tbl["name"].value_or("papers");
    database_settings_.user = database_tbl["user"].value_or("crawler");
    database_settings_.password = database_tbl["password"].value_or("password");
}

void CrawlerConfig::parseMonitoringConfig(const toml::table& config) {
    auto monitoring_tbl = config["monitoring"];
    if (!monitoring_tbl) {
        monitoring_settings_.enable_metrics = true; // 默认启用
        return;
    }

    monitoring_settings_.enable_metrics =
            monitoring_tbl["enable_metrics"].value_or(true);
    monitoring_settings_.metrics_port = monitoring_tbl["metrics_port"].value_or(9090);
    monitoring_settings_.log_interval_seconds =
            monitoring_tbl["log_interval_seconds"].value_or(60);
}

CrawlerMode CrawlerConfig::stringToMode(const std::string& mode_str) {
    if (mode_str == "thread") {
        return CrawlerMode::THREAD;
    } else if (mode_str == "coroutine") {
        return CrawlerMode::COROUTINE;
    } else if (mode_str == "process") {
        return CrawlerMode::PROCESS;
    } else {
        std::cerr << "Unknown crawler mode: " << mode_str
                  << ", using default: thread" << std::endl;
        return CrawlerMode::THREAD;
    }
}

std::string CrawlerConfig::modeToString(CrawlerMode mode) {
    switch (mode) {
        case CrawlerMode::THREAD: return "thread";
        case CrawlerMode::COROUTINE: return "coroutine";
        case CrawlerMode::PROCESS: return "process";
        default: return "thread";
    }
}

bool CrawlerConfig::saveConfig(const std::string& config_path) {
    try {
        toml::table config;

        // 保存爬虫配置
        config.insert("crawler", toml::table{
                {"mode", modeToString(crawler_settings_.mode)},
                {"max_connections", crawler_settings_.max_connections},
                {"request_timeout", crawler_settings_.request_timeout},
                {"retry_attempts", crawler_settings_.retry_attempts},
                {"delay_between_requests", crawler_settings_.delay_between_requests},
                {"user_agent", crawler_settings_.user_agent},
                {"log_level", crawler_settings_.log_level}
        });

        // 保存存储配置
        config.insert("storage", toml::table{
                {"output_dir", storage_settings_.output_dir},
                {"output_format", storage_settings_.output_format},
                {"max_file_size_mb", storage_settings_.max_file_size_mb},
                {"batch_size", storage_settings_.batch_size}
        });

        // 保存arXiv配置
        config.insert("arxiv", toml::table{
                {"base_url", arxiv_settings_.base_url},
                {"categories", arxiv_settings_.categories},
                {"max_results", arxiv_settings_.max_results},
                {"update_interval_hours", arxiv_settings_.update_interval_hours},
                {"batch_size", arxiv_settings_.batch_size}
        });

        // 保存bioRxiv配置
        config.insert("biorxiv", toml::table{
                {"base_url", biorxiv_settings_.base_url},
                {"categories", biorxiv_settings_.categories},
                {"max_results", biorxiv_settings_.max_results},
                {"update_interval_hours", biorxiv_settings_.update_interval_hours}
        });

        // 保存ChemRxiv配置
        config.insert("chemrxiv", toml::table{
                {"base_url", chemrxiv_settings_.base_url},
                {"categories", chemrxiv_settings_.categories},
                {"max_results", chemrxiv_settings_.max_results},
                {"update_interval_hours", chemrxiv_settings_.update_interval_hours}
        });

        // 保存数据库配置
        config.insert("database", toml::table{
                {"enabled", database_settings_.enabled},
                {"host", database_settings_.host},
                {"port", database_settings_.port},
                {"name", database_settings_.name},
                {"user", database_settings_.user},
                {"password", database_settings_.password}
        });

        // 保存监控配置
        config.insert("monitoring", toml::table{
                {"enable_metrics", monitoring_settings_.enable_metrics},
                {"metrics_port", monitoring_settings_.metrics_port},
                {"log_interval_seconds", monitoring_settings_.log_interval_seconds}
        });

        // 保存关键词映射
        toml::table keywords_tbl;
        for (const auto& [key, values] : keywords_) {
            keywords_tbl.insert(key, values);
        }
        config.insert("keywords", keywords_tbl);

        // 写入文件
        std::ofstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << config_path << std::endl;
            return false;
        }

        file << config;
        file.close();

        std::cout << "Configuration saved successfully to: " << config_path << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving config file: " << e.what() << std::endl;
        return false;
    }
}

void CrawlerConfig::updateCategory(const std::string& source,
                                   const std::vector<std::string>& categories) {
    if (source == "arxiv") {
        arxiv_settings_.categories = categories;
    } else if (source == "biorxiv") {
        biorxiv_settings_.categories = categories;
    } else if (source == "chemrxiv") {
        chemrxiv_settings_.categories = categories;
    } else {
        std::cerr << "Unknown source for category update: " << source << std::endl;
    }
}

// 验证配置的完整性
bool CrawlerConfig::validate() const {
    if (crawler_settings_.max_connections == 0) {
        std::cerr << "Validation error: max_connections must be greater than 0" << std::endl;
        return false;
    }

    if (crawler_settings_.request_timeout <= 0) {
        std::cerr << "Validation error: request_timeout must be positive" << std::endl;
        return false;
    }

    if (storage_settings_.output_dir.empty()) {
        std::cerr << "Validation error: output_dir cannot be empty" << std::endl;
        return false;
    }

    // 检查必要的API配置
    if (arxiv_settings_.base_url.empty() ||
        biorxiv_settings_.base_url.empty() ||
        chemrxiv_settings_.base_url.empty()) {
        std::cerr << "Validation error: API base URLs cannot be empty" << std::endl;
        return false;
    }

    // 检查关键词映射
    if (keywords_.empty()) {
        std::cerr << "Warning: No keyword mappings found" << std::endl;
    }

    return true;
}

// 获取默认配置（用于初始化）
CrawlerConfig CrawlerConfig::getDefaultConfig() {
    CrawlerConfig config;

    // 设置默认爬虫参数
    config.crawler_settings_.mode = CrawlerMode::THREAD;
    config.crawler_settings_.max_connections = 10;
    config.crawler_settings_.request_timeout = 30;
    config.crawler_settings_.retry_attempts = 3;
    config.crawler_settings_.delay_between_requests = 1.0;
    config.crawler_settings_.user_agent = "AcademicCrawler/1.0";
    config.crawler_settings_.log_level = "info";

    // 设置默认存储参数
    config.storage_settings_.output_dir = "./data";
    config.storage_settings_.output_format = "json";
    config.storage_settings_.max_file_size_mb = 100;
    config.storage_settings_.batch_size = 100;

    // 设置默认API参数
    config.arxiv_settings_.base_url = "https://export.arxiv.org/api/query";
    config.arxiv_settings_.categories = {
            "cond-mat", "hep-th", "hep-ph", "hep-ex", "hep-lat",
            "quant-ph", "physics", "cond-mat.mtrl-sci", "physics.chem-ph"
    };
    config.arxiv_settings_.max_results = 1000;
    config.arxiv_settings_.update_interval_hours = 24;
    config.arxiv_settings_.batch_size = 50;

    config.biorxiv_settings_.base_url = "https://api.biorxiv.org/details/";
    config.biorxiv_settings_.categories = {"q-bio"};
    config.biorxiv_settings_.max_results = 500;
    config.biorxiv_settings_.update_interval_hours = 24;

    config.chemrxiv_settings_.base_url =
            "https://chemrxiv.org/engage/chemrxiv/public-api/v1/items";
    config.chemrxiv_settings_.categories = {"physics.chem-ph"};
    config.chemrxiv_settings_.max_results = 300;
    config.chemrxiv_settings_.update_interval_hours = 24;

    // 设置默认关键词映射
    config.keywords_ = {
            {"physics", {"cond-mat", "hep-", "quant-ph", "physics"}},
            {"materials", {"cond-mat.mtrl-sci"}},
            {"chemistry", {"physics.chem-ph"}},
            {"biology", {"q-bio"}},
            {"electrical_engineering", {"eess.SP", "eess.SY"}}
    };

    return config;
}