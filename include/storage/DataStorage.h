//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_DATASTORAGE_H
#define CRAWLPAPER_DATASTORAGE_H

#endif //CRAWLPAPER_DATASTORAGE_H
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>
#include "Paper.h"

class DataStorage {
public:
    DataStorage();
    ~DataStorage();

    bool initialize(const std::string& output_dir, const std::string& format = "json");

    // Paper storage methods
    bool save_paper(const Paper& paper);
    bool save_papers(const std::vector<Paper>& papers);
    std::vector<Paper> load_papers(const std::string& source = "");
    std::vector<Paper> load_papers_by_category(const std::string& category);

    // File management
    bool rotate_file_if_needed();
    std::string get_current_filename() const;

    // Statistics
    struct StorageStats {
        size_t total_papers = 0;
        size_t papers_by_source;
        size_t papers_by_category;
        std::chrono::system_clock::time_point last_updated;
    };

    StorageStats get_stats() const;

private:
    std::string output_dir_;
    std::string format_;
    size_t max_file_size_;
    mutable std::ofstream current_file_;
    std::string current_filename_;
    size_t current_size_ = 0;

    std::string generate_filename() const;
    bool open_new_file();
    bool close_current_file();

    // Format-specific writers
    bool write_json(const Paper& paper);
    bool write_csv(const Paper& paper);
    bool write_xml(const Paper& paper);

    // Format-specific readers
    std::vector<Paper> read_json_file(const std::string& filename) const;
    std::vector<Paper> read_csv_file(const std::string& filename) const;
    std::vector<Paper> read_xml_file(const std::string& filename) const;
};