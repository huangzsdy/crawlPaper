//
// Created by huang on 2026/2/8.
//

#ifndef CRAWLPAPER_ARXIVPARSER_H
#define CRAWLPAPER_ARXIVPARSER_H

#endif //CRAWLPAPER_ARXIVPARSER_H
#pragma once
#include "PaperParser.h"
#include <pugixml.hpp>

class ArxivParser : public PaperParser {
public:
    std::vector<Paper> parse_papers(const std::string& content) override;
    std::string build_query(const std::vector<std::string>& categories,
                            size_t start_index, size_t max_results) override;
    std::string get_source_name() const override { return "arxiv"; }

private:
    Paper parse_paper_entry(const pugi::xml_node& entry);
    std::string parse_arxiv_id(const std::string& id_text);
    std::string parse_doi(const pugi::xml_node& entry);
    std::string parse_journal_ref(const pugi::xml_node& entry);
    std::string parse_comment(const pugi::xml_node& entry);
};