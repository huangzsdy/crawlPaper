#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <toml.hpp>
#include <optional>

enum class CrawlerMode {
    THREAD,
    COROUTINE,
    PROCESS
};

struct CrawlerSettings {
    CrawlerMode mode = CrawlerMode::THREAD;
    size_t max_connections = 10;
    int request_timeout = 30;
    int retry_attempts = 3;
    double delay_between_requests = 1.0;
    std::string user_agent = "AcademicCrawler/1.0";
    std::string log_level = "info";
};

struct StorageSettings {
    std::string output_dir = "./data";
    std::string output_format = "json";
    size_t max_file_size_mb = 100;
    size_t batch_size = 100;
};

struct ApiSettings {
    std::string base_url;
    std::vector<std::string> categories;
    size_t max_results = 100;
    int update_interval_hours = 24;
    size_t batch_size = 50;
};

struct DatabaseSettings {
    bool enabled = false;
    std::string host = "localhost";
    int port = 5432;
    std::string name = "papers";
    std::string user = "crawler";
    std::string password = "password";
};

struct MonitoringSettings {
    bool enable_metrics = true;
    int metrics_port = 9090;
    int log_interval_seconds = 60;
};

class CrawlerConfig {
public:
    static CrawlerConfig& getInstance() {
        static CrawlerConfig instance;
        return instance;
    }

    bool loadConfig(const std::string& config_path);
    bool saveConfig(const std::string& config_path);

    // Getters
    const CrawlerSettings& getCrawlerSettings() const { return crawler_settings_; }
    const StorageSettings& getStorageSettings() const { return storage_settings_; }
    const ApiSettings& getArxivSettings() const { return arxiv_settings_; }
    const ApiSettings& getBiorxivSettings() const { return biorxiv_settings_; }
    const ApiSettings& getChemRxivSettings() const { return chemrxiv_settings_; }
    const DatabaseSettings& getDatabaseSettings() const { return database_settings_; }
    const MonitoringSettings& getMonitoringSettings() const { return monitoring_settings_; }

    const std::unordered_map<std::string, std::vector<std::string>>& getKeywords() const {
        return keywords_;
    }

    // Setters for dynamic configuration
    void setCrawlerSettings(const CrawlerSettings& settings) { crawler_settings_ = settings; }
    void updateCategory(const std::string& source, const std::vector<std::string>& categories);

private:
    CrawlerConfig() = default;
    ~CrawlerConfig() = default;

    CrawlerConfig(const CrawlerConfig&) = delete;
    CrawlerConfig& operator=(const CrawlerConfig&) = delete;

    CrawlerSettings crawler_settings_;
    StorageSettings storage_settings_;
    ApiSettings arxiv_settings_;
    ApiSettings biorxiv_settings_;
    ApiSettings chemrxiv_settings_;
    DatabaseSettings database_settings_;
    MonitoringSettings monitoring_settings_;
    std::unordered_map<std::string, std::vector<std::string>> keywords_;

    // Helper methods
    CrawlerMode stringToMode(const std::string& mode_str);
    std::string modeToString(CrawlerMode mode);
};