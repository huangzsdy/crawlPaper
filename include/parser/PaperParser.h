#pragma once
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include "Paper.h"

class PaperParser {
public:
    virtual ~PaperParser() = default;

    // Pure virtual interface for different preprint servers
    virtual std::vector<Paper> parse_papers(const std::string& content) = 0;
    virtual std::string build_query(const std::vector<std::string>& categories,
                                    size_t start_index, size_t max_results) = 0;
    virtual std::string get_source_name() const = 0;

    // Common parsing helpers
    static std::string extract_text_between(const std::string& content,
                                            const std::string& start,
                                            const std::string& end);
    static std::vector<std::string> extract_links(const std::string& content);
    static std::string clean_html(const std::string& html);

    // Factory method
    static std::unique_ptr<PaperParser> create(const std::string& source);

protected:
    // Common utility methods
    std::string parse_date(const std::string& date_str);
    std::vector<Author> parse_authors(const std::string& authors_str);
    std::vector<std::string> parse_categories(const std::string& categories_str);
};