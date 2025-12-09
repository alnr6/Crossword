#pragma once
#include <vector>
#include <string>
#include "CrosswordGrid.hpp"
#include "trie.hpp"

class Generator {
public:
    Generator(Trie* dict=nullptr);
    // attempt to fill slots in grid using words from provided list; returns true if success
    bool generate(CrosswordGrid& grid, const std::vector<std::string>& wordList);

private:
    Trie* dict = nullptr;
    bool backtrackPlace(CrosswordGrid& grid, std::vector<int>& slotOrder, size_t idx, const std::vector<std::string>& pool);
};
