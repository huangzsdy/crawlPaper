#include "storage/DataStorage.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <iomanip>

DataStorage::DataStorage() : current_size_(0) {
}

DataStorage::~DataStorage() {
    close_current_file();
}

bool DataStorage::initialize(const std::string& output_dir, const std::string& format) {
    output_dir_ = output_dir;
    format_ = format;

    // Create output directory if it doesn't exist
    std::filesystem::create_directories(output_dir_);

    // Generate initial filename
    current_filename_ = generate_filename();

    return open_new_file();
}

bool DataStorage::save_paper(const Paper& paper) {
    if (!current_file_.is_open()) {
        if (!open_new_file()) return false;
    }

    // Check if we need to rotate file
    if (current_size_ > max_file_size_ * 1024 * 1024) {
        if (!rotate_file_if_needed()) return false;
    }

    try {
        bool success = false;
        if (format_ == "json") {
            success = write_json(paper);
        } else if (format_ == "csv") {
            success = write_csv(paper);
        } else if (format_ == "xml") {
            success = write_xml(paper);
        }

        if (success) {
            current_size_ += paper.to_json().dump().size();
            current_file_ << std::endl; // Add newline after each paper
            current_file_.flush();
        }

        return success;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DataStorage::save_papers(const std::vector<Paper>& papers) {
    bool all_success = true;
    for (const auto& paper : papers) {
        if (!save_paper(paper)) {
            all_success = false;
        }
    }
    return all_success;
}

std::vector<Paper> DataStorage::load_papers(const std::string& source) {
    std::vector<Paper> papers;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(output_dir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                auto file_papers = read_json_file(entry.path().string());
                for (auto& paper : file_papers) {
                    if (source.empty() || paper.source == source) {
                        papers.push_back(std::move(paper));
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        // Handle file reading errors
    }

    return papers;
}

std::vector<Paper> DataStorage::load_papers_by_category(const std::string& category) {
    auto all_papers = load_papers();
    std::vector<Paper> filtered_papers;

    for (const auto& paper : all_papers) {
        if (std::find(paper.categories.begin(), paper.categories.end(), category)
            != paper.categories.end()) {
            filtered_papers.push_back(paper);
        }
    }

    return filtered_papers;
}

bool DataStorage::rotate_file_if_needed() {
    if (current_size_ < max_file_size_ * 1024 * 1024) {
        return true;
    }

    if (!close_current_file()) {
        return false;
    }

    current_filename_ = generate_filename();
    return open_new_file();
}

std::string DataStorage::get_current_filename() const {
    return current_filename_;
}

std::string DataStorage::generate_filename() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::gmtime(&time_t);

    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);

    return output_dir_ + "/papers_" + std::string(buffer) + "." + format_;
}

bool DataStorage::open_new_file() {
    try {
        current_file_.open(current_filename_, std::ios::out | std::ios::app);
        if (!current_file_.is_open()) {
            return false;
        }

        // Write file header based on format
        if (format_ == "json") {
            current_file_ << "[" << std::endl;
        } else if (format_ == "csv") {
            current_file_ << "id,title,authors,doi,source,categories,published_date" << std::endl;
        } else if (format_ == "xml") {
            current_file_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
            current_file_ << "<papers>" << std::endl;
        }

        current_size_ = 0;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DataStorage::close_current_file() {
    if (!current_file_.is_open()) {
        return true;
    }

    try {
        // Write file footer based on format
        if (format_ == "json") {
            current_file_ << "]" << std::endl;
        } else if (format_ == "xml") {
            current_file_ << "</papers>" << std::endl;
        }

        current_file_.close();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DataStorage::write_json(const Paper& paper) {
    static bool first_paper = true;

    if (!first_paper) {
        current_file_ << "," << std::endl;
    }

    current_file_ << paper.to_json().dump(4);
    first_paper = false;

    return true;
}

bool DataStorage::write_csv(const Paper& paper) {
    // Simple CSV writing - in real implementation, you'd want proper CSV escaping
    current_file_ << "\"" << paper.id << "\","
                  << "\"" << paper.title << "\","
                  << "\"" << paper.authors.size() << "\","
                  << "\"" << paper.doi << "\","
                  << "\"" << paper.source << "\","
                  << "\"" << paper.categories.size() << "\","
                  << "\"" << std::chrono::system_clock::to_time_t(paper.published_date) << "\""
                  << std::endl;

    return true;
}

bool DataStorage::write_xml(const Paper& paper) {
    current_file_ << "  <paper>" << std::endl;
    current_file_ << "    <id>" << paper.id << "</id>" << std::endl;
    current_file_ << "    <title>" << paper.title << "</title>" << std::endl;
    current_file_ << "    <source>" << paper.source << "</source>" << std::endl;
    current_file_ << "  </paper>" << std::endl;

    return true;
}

std::vector<Paper> DataStorage::read_json_file(const std::string& filename) const {
    std::vector<Paper> papers;

    try {
        std::ifstream file(filename);
        if (!file.is_open()) return papers;

        nlohmann::json json_data;
        file >> json_data;

        if (json_data.is_array()) {
            for (const auto& item : json_data) {
                papers.push_back(Paper::from_json(item));
            }
        }
    } catch (const std::exception& e) {
        // Handle parsing errors
    }

    return papers;
}

// Other format readers would be implemented similarly...