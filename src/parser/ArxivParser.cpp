#include "parser/ArxivParser.h"
#include <sstream>
#include <pugixml.hpp>
#include <algorithm>
#include <iomanip>
#include <ctime>

std::vector<Paper> ArxivParser::parse_papers(const std::string& content) {
    std::vector<Paper> papers;
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_string(content.c_str());
    if (!result) {
        return papers;
    }

    pugi::xpath_node_set entries = doc.select_nodes("//entry");
    for (pugi::xpath_node node : entries) {
        try {
            Paper paper = parse_paper_entry(node.node());
            papers.push_back(std::move(paper));
        } catch (const std::exception& e) {
            // Log parsing error but continue with other entries
            continue;
        }
    }

    return papers;
}

Paper ArxivParser::parse_paper_entry(const pugi::xml_node& entry) {
    Paper paper;
    paper.source = "arxiv";

    // Parse ID
    paper.id = parse_arxiv_id(entry.child("id").text().as_string());

    // Parse title
    paper.title = entry.child("title").text().as_string();

    // Parse abstract/summary
    paper.abstract = entry.child("summary").text().as_string();

    // Parse authors
    pugi::xpath_node_set authors = entry.select_nodes("author/name");
    for (pugi::xpath_node author_node : authors) {
        Author author;
        author.name = author_node.node().text().as_string();
        paper.authors.push_back(author);
    }

    // Parse DOI
    paper.doi = parse_doi(entry);

    // Parse categories
    std::string categories_str = entry.child("arxiv:categories").text().as_string();
    paper.categories = parse_categories(categories_str);

    // Parse dates
    auto parse_arxiv_date = [](const std::string& date_str) {
        std::tm tm = {};
        std::istringstream ss(date_str);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    };

    paper.published_date = parse_arxiv_date(entry.child("published").text().as_string());
    paper.updated_date = parse_arxiv_date(entry.child("updated").text().as_string());

    // Parse journal reference and comment
    paper.journal_ref = parse_journal_ref(entry);
    paper.comment = parse_comment(entry);

    // Construct PDF URL
    paper.pdf_url = "https://arxiv.org/pdf/" + paper.id + ".pdf";

    return paper;
}

std::string ArxivParser::build_query(const std::vector<std::string>& categories,
                                     size_t start_index, size_t max_results) {
    std::stringstream query;
    query << "search_query=cat:";

    bool first = true;
    for (const auto& category : categories) {
        if (!first) query << "+OR+cat:";
        query << category;
        first = false;
    }

    query << "&start=" << start_index
          << "&max_results=" << max_results
          << "&sortBy=submittedDate&sortOrder=descending";

    return query.str();
}

std::string ArxivParser::parse_arxiv_id(const std::string& id_text) {
    // Extract arXiv ID from full URL
    std::regex id_regex(R"(arxiv\.org/abs/([^/]+)$)");
    std::smatch match;

    if (std::regex_search(id_text, match, id_regex) && match.size() > 1) {
        return match[1];
    }

    // If no match, return the original text
    return id_text;
}

std::string ArxivParser::parse_doi(const pugi::xml_node& entry) {
    pugi::xpath_node_set links = entry.select_nodes("link[@title='doi']");
    for (pugi::xpath_node link : links) {
        std::string href = link.node().attribute("href").as_string();
        std::regex doi_regex(R"(10\.\d{4,9}/[-._;()/:A-Z0-9]+)", std::regex::icase);
        std::smatch match;

        if (std::regex_search(href, match, doi_regex)) {
            return match[0];
        }
    }
    return "";
}

std::string ArxivParser::parse_journal_ref(const pugi::xml_node& entry) {
    return entry.child("arxiv:journal_ref").text().as_string();
}

std::string ArxivParser::parse_comment(const pugi::xml_node& entry) {
    return entry.child("arxiv:comment").text().as_string();
}