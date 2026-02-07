//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_BIORXIVPARSER_H
#define CRAWLPAPER_BIORXIVPARSER_H

#endif //CRAWLPAPER_BIORXIVPARSER_H
#pragma once
#include "PaperParser.h"

class BiorxivParser : public PaperParser {
public:
    std::vector<Paper> parse_papers(const std::string& content) override;
    std::string build_query(const std::vector<std::string>& categories,
                            size_t start_index, size_t max_results) override;
    std::string get_source_name() const override { return "biorxiv"; }

private:
    Paper parse_paper_item(const nlohmann::json& item);
    std::vector<Author> parse_authors(const std::string& authors_str);
};