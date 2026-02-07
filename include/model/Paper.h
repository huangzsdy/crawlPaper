//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_PAPER_H
#define CRAWLPAPER_PAPER_H

#endif //CRAWLPAPER_PAPER_H

#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>

struct Author {
    std::string name;
    std::string affiliation;
    std::string orcid;

    nlohmann::json to_json() const {
        return {
                {"name", name},
                {"affiliation", affiliation},
                {"orcid", orcid}
        };
    }

    static Author from_json(const nlohmann::json& j) {
        return {
                j.value("name", ""),
                j.value("affiliation", ""),
                j.value("orcid", "")
        };
    }
};

struct Paper {
    std::string id;
    std::string title;
    std::string abstract;
    std::vector<Author> authors;
    std::string doi;
    std::string pdf_url;
    std::string source;
    std::vector<std::string> categories;
    std::chrono::system_clock::time_point published_date;
    std::chrono::system_clock::time_point updated_date;
    std::string journal_ref;
    std::string comment;
    int version = 1;

    // Modern C++ features
    Paper() = default;

    // Move semantics
    Paper(Paper&&) = default;
    Paper& operator=(Paper&&) = default;

    // Copy semantics (explicitly enabled)
    Paper(const Paper&) = default;
    Paper& operator=(const Paper&) = default;

    nlohmann::json to_json() const {
        auto time_to_string = [](const auto& time_point) {
            auto time_t = std::chrono::system_clock::to_time_t(time_point);
            std::tm tm = *std::gmtime(&time_t);
            char buffer[64];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
            return std::string(buffer);
        };

        return {
                {"id", id},
                {"title", title},
                {"abstract", abstract},
                {"authors", nlohmann::json::array()},
                {"doi", doi},
                {"pdf_url", pdf_url},
                {"source", source},
                {"categories", categories},
                {"published_date", time_to_string(published_date)},
                {"updated_date", time_to_string(updated_date)},
                {"journal_ref", journal_ref},
                {"comment", comment},
                {"version", version}
        };
    }

    static Paper from_json(const nlohmann::json& j) {
        Paper paper;
        paper.id = j.value("id", "");
        paper.title = j.value("title", "");
        paper.abstract = j.value("abstract", "");
        paper.doi = j.value("doi", "");
        paper.pdf_url = j.value("pdf_url", "");
        paper.source = j.value("source", "");
        paper.journal_ref = j.value("journal_ref", "");
        paper.comment = j.value("comment", "");
        paper.version = j.value("version", 1);

        // Parse authors
        if (j.contains("authors") && j["authors"].is_array()) {
            for (const auto& author_json : j["authors"]) {
                paper.authors.push_back(Author::from_json(author_json));
            }
        }

        // Parse categories
        if (j.contains("categories") && j["categories"].is_array()) {
            for (const auto& category : j["categories"]) {
                paper.categories.push_back(category);
            }
        }

        // Parse dates
        auto string_to_time = [](const std::string& str) {
            std::tm tm = {};
            std::istringstream ss(str);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            return std::chrono::system_clock::from_time_t(std::mktime(&tm));
        };

        if (j.contains("published_date")) {
            paper.published_date = string_to_time(j["published_date"]);
        }
        if (j.contains("updated_date")) {
            paper.updated_date = string_to_time(j["updated_date"]);
        }

        return paper;
    }
};