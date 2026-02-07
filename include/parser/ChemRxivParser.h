//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_CHEMRXIVPARSER_H
#define CRAWLPAPER_CHEMRXIVPARSER_H

#endif //CRAWLPAPER_CHEMRXIVPARSER_H
#pragma once
#include "PaperParser.h"

class ChemRxivParser : public PaperParser {
public:
    std::vector<Paper> parse_papers(const std::string& content) override;
    std::string build_query(const std::vector<std::string>& categories,
                            size_t start_index, size_t max_results) override;
    std::string get_source_name() const override { return "chemrxiv"; }

private:
    Paper parse_paper_item(const nlohmann::json& item);
};