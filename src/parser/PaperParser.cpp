#include "parser/PaperParser.h"
#include "parser/ArxivParser.h"
#include "parser/BiorxivParser.h"
#include "parser/ChemRxivParser.h"
#include <algorithm>
#include <regex>

std::unique_ptr<PaperParser> PaperParser::create(const std::string& source) {
    if (source == "arxiv") {
        return std::make_unique<ArxivParser>();
    } else if (source == "biorxiv") {
        return std::make_unique<BiorxivParser>();
    } else if (source == "chemrxiv") {
        return std::make_unique<ChemRxivParser>();
    }
    throw std::invalid_argument("Unknown paper source: " + source);
}

std::string PaperParser::extract_text_between(const std::string& content,
                                              const std::string& start,
                                              const std::string& end) {
    size_t start_pos = content.find(start);
    if (start_pos == std::string::npos) return "";

    start_pos += start.length();
    size_t end_pos = content.find(end, start_pos);
    if (end_pos == std::string::npos) return "";

    return content.substr(start_pos, end_pos - start_pos);
}

std::vector<std::string> PaperParser::extract_links(const std::string& content) {
    std::vector<std::string> links;
    std::regex link_regex(R"(href="([^"]*)")", std::regex::icase);

    std::sregex_iterator it(content.begin(), content.end(), link_regex);
    std::sregex_iterator end;

    while (it != end) {
        std::smatch match = *it;
        if (match.size() > 1) {
            links.push_back(match[1].str());
        }
        ++it;
    }

    return links;
}

std::string PaperParser::clean_html(const std::string& html) {
    std::string result = html;

    // Remove HTML tags
    std::regex tag_regex("<[^>]*>");
    result = std::regex_replace(result, tag_regex, "");

    // Replace HTML entities
    std::regex entity_regex("&[^;]*;");
    result = std::regex_replace(result, entity_regex, " ");

    // Remove extra whitespace
    std::