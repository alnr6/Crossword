#pragma once
#include <string>
#include <vector>
#include <memory>

class Trie {
public:
    Trie();
    void insert(const std::string& word);
    bool contains(const std::string& word) const;
    bool startsWith(const std::string& prefix) const;
    bool loadFromFile(const std::string& path);
    std::vector<std::string> collectWithPattern(const std::string& pattern) const; // pattern: letters or '_' for unknown

private:
    struct Node {
        bool end = false;
        std::unique_ptr<Node> next[26];
    };

    std::unique_ptr<Node> root;

    void collectRec(Node* node, std::string& cur, const std::string& pattern, std::vector<std::string>& out, size_t idx) const;
};
