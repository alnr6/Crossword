#include "trie.hpp"
#include <fstream>
#include <algorithm>

Trie::Trie() : root(std::make_unique<Node>()) {}

void Trie::insert(const std::string& word) {
    Node* cur = root.get();
    for (char ch : word) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) continue;
        int i = std::toupper(static_cast<unsigned char>(ch)) - 'A';
        if (!cur->next[i]) cur->next[i] = std::make_unique<Node>();
        cur = cur->next[i].get();
    }
    cur->end = true;
}

bool Trie::contains(const std::string& word) const {
    Node* cur = root.get();
    for (char ch : word) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) return false;
        int i = std::toupper(static_cast<unsigned char>(ch)) - 'A';
        if (!cur->next[i]) return false;
        cur = cur->next[i].get();
    }
    return cur->end;
}

bool Trie::startsWith(const std::string& prefix) const {
    Node* cur = root.get();
    for (char ch : prefix) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) return false;
        int i = std::toupper(static_cast<unsigned char>(ch)) - 'A';
        if (!cur->next[i]) return false;
        cur = cur->next[i].get();
    }
    return true;
}

bool Trie::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string w;
    while (in >> w) {
        // normalize to uppercase
        std::string tmp;
        tmp.reserve(w.size());
        for (char ch : w) if (std::isalpha(static_cast<unsigned char>(ch))) tmp.push_back(std::toupper(static_cast<unsigned char>(ch)));
        if (!tmp.empty()) insert(tmp);
    }
    return true;
}

std::vector<std::string> Trie::collectWithPattern(const std::string& pattern) const {
    std::vector<std::string> out;
    std::string cur;
    cur.reserve(pattern.size());
    collectRec(root.get(), cur, pattern, out, 0);
    return out;
}

void Trie::collectRec(Node* node, std::string& cur, const std::string& pattern, std::vector<std::string>& out, size_t idx) const {
    if (!node) return;
    if (idx == pattern.size()) {
        if (node->end) out.push_back(cur);
        return;
    }
    char p = pattern[idx];
    if (p == '_' || p == '?' ) {
        for (int i = 0; i < 26; ++i) {
            if (node->next[i]) {
                cur.push_back('A' + i);
                collectRec(node->next[i].get(), cur, pattern, out, idx + 1);
                cur.pop_back();
            }
        }
    } else if (std::isalpha(static_cast<unsigned char>(p))) {
        int i = std::toupper(static_cast<unsigned char>(p)) - 'A';
        if (i >=0 && i < 26 && node->next[i]) {
            cur.push_back('A' + i);
            collectRec(node->next[i].get(), cur, pattern, out, idx + 1);
            cur.pop_back();
        }
    }
}
