#include "parser/ChemRxivParser.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include <algorithm>

std::vector<Paper> ChemRxivParser::parse_papers(const std::string& content) {
    std::vector<Paper> papers;

    try {
        auto json = nlohmann::json::parse(content);

        if (json.contains("itemHits") && json["itemHits"].is_array()) {
            for (const auto& hit : json["itemHits"]) {
                if (hit.contains("item") && hit["item"].is_object()) {
                    try {
                        Paper paper = parse_paper_item(hit["item"]);
                        papers.push_back(std::move(paper));
                    } catch (const std::exception& e) {
                        continue;
                    }
                }
            }
        }
    } catch (const nlohmann::json::exception& e) {
        // Handle JSON parsing errors
    }

    return papers;
}

Paper ChemRxivParser::parse_paper_item(const nlohmann::json& item) {
    Paper paper;
    paper.source = "chemrxiv";

    // Parse basic fields
    paper.id = item.value("id", "");
    paper.title = item.value("title", "");
    paper.doi = item.value("doi", "");

    // Parse abstract
    if (item.contains("description") && item["description"].is_string()) {
        paper.abstract = item["description"];
    }

    // Parse authors
    if (item.contains("authors") && item["authors"].is_array()) {
        for (const auto& author_json : item["authors"]) {
            Author author;
            author.name = author_json.value("firstName", "") + " " +
                          author_json.value("lastName", "");
            author.affiliation = author_json.value("affiliation", "");
            author.orcid = author_json.value("orcid", "");
            paper.authors.push_back(author);
        }
    }

    // Parse categories
    if (item.contains("categories") && item["categories"].is_array()) {
        for (const auto& category : item["categories"]) {
            paper.categories.push_back(category);
        }
    }

    // Parse dates
    auto parse_chemrxiv_date = [](const std::string& date_str) {
        std::tm tm = {};
        std::istringstream ss(date_str);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S.%fZ");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    };

    paper.published_date = parse_chemrxiv_date(item.value("publishedDate", ""));
    paper.updated_date = parse_chemrxiv_date(item.value("updatedDate", ""));

    // Parse PDF URL
    if (item.contains("pdfUrl") && item["pdfUrl"].is_string()) {
        paper.pdf_url = item["pdfUrl"];
    }

    // Parse version
    paper.version = item.value("version", 1);

    return paper;
}

std::string Chem