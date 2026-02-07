#include "parser/BiorxivParser.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include <algorithm>

std::vector<Paper> BiorxivParser::parse_papers(const std::string& content) {
    std::vector<Paper> papers;

    try {
        auto json = nlohmann::json::parse(content);

        if (json.contains("collection") && json["collection"].is_array()) {
            for (const auto& item : json["collection"]) {
                try {
                    Paper paper = parse_paper_item(item);
                    papers.push_back(std::move(paper));
                } catch (const std::exception& e) {
                    continue;
                }
            }
        }
    } catch (const nlohmann::json::exception& e) {
        // Handle JSON parsing errors
    }

    return papers;
}

Paper BiorxivParser::parse_paper_item(const nlohmann::json& item) {
    Paper paper;
    paper.source = "biorxiv";

    // Parse basic fields
    paper.id = item.value("doi", "");
    paper.title = item.value("title", "");
    paper.abstract = item.value("abstract", "");
    paper.doi = item.value("doi", "");

    // Parse authors
    std::string authors_str = item.value("authors", "");
    paper.authors = parse_authors(authors_str);

    // Parse categories
    std::string category = item.value("category", "");
    if (!category.empty()) {
        paper.categories.push_back(category);
    }

    // Parse dates
    auto parse_biorxiv_date = [](const std::string& date_str) {
        std::tm tm = {};
        std::istringstream ss(date_str);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    };

    paper.published_date = parse_biorxiv_date(item.value("date", ""));
    paper.updated_date = paper.published_date; // bioRxiv doesn't typically have update dates

    // Parse PDF URL
    paper.pdf_url = item.value("jatsxml", "");
    if (!paper.pdf_url.empty()) {
        // Convert to PDF URL
        std::regex pdf_regex(R"(\.xml$)");
        paper.pdf_url = std::regex_replace(paper.pdf_url, pdf_regex, ".pdf");
    }

    // Parse version
    paper.version = item.value("version", 1);

    return paper;
}

std::string BiorxivParser::build_query(const std::vector<std::string>& categories,
                                       size_t start_index, size_t max_results) {
    std::stringstream query;

    if (!categories.empty()) {
        query << "cat:";
        for (size_t i = 0; i < categories.size(); ++i) {
            if (i > 0) query << "+OR+";
            query << categories[i];
        }
    }

    query << "&limit=" << max_results
          << "&skip=" << start_index
          << "&format=json";

    return query.str();
}

std::vector<Author> BiorxivParser::parse_authors(const std::string& authors_str) {
    std::vector<Author> authors;

    // bioRxiv authors are typically in "LastName1 FirstName1, LastName2 FirstName2" format
    std::regex author_regex(R"(([^,]+)(?:,\s*|$))");
    std::sregex_iterator it(authors_str.begin(), authors_str.end(), author_regex);
    std::sregex_iterator end;

    while (it != end) {
        std::smatch match = *it;
        if (match.size() > 1) {
            Author author;
            author.name = match[1].str();
            authors.push_back(author);
        }
        ++it;
    }

    return authors;
}