#include "Generator.hpp"
#include <algorithm>
#include <random>
#include <ctime>

Generator::Generator(Trie* dict) : dict(dict) {}

bool Generator::generate(CrosswordGrid& grid, const std::vector<std::string>& wordList) {
    grid.recomputeClueNumbers();
    grid.buildSlots();
    std::vector<int> slotOrder(grid.getSlots().size());
    for (size_t i = 0; i < slotOrder.size(); ++i) slotOrder[i] = (int)i;
    // Sort by descending length to reduce branching
    std::sort(slotOrder.begin(), slotOrder.end(), [&](int a, int b){ return grid.getSlots()[a].length > grid.getSlots()[b].length; });

    std::vector<std::string> pool = wordList;
    // Uppercase pool
    for (auto &w : pool) {
        std::string tmp; tmp.reserve(w.size());
        for (char ch : w) if (std::isalpha(static_cast<unsigned char>(ch))) tmp.push_back(std::toupper(static_cast<unsigned char>(ch)));
        w = tmp;
    }

    // try backtracking
    return backtrackPlace(grid, slotOrder, 0, pool);
}

bool Generator::backtrackPlace(CrosswordGrid& grid, std::vector<int>& slotOrder, size_t idx, const std::vector<std::string>& pool) {
    if (idx >= slotOrder.size()) return true;
    int slotIdx = slotOrder[idx];
    const auto& slots = grid.getSlots();
    auto len = slots[slotIdx].length;

    // collect candidate words of matching length that also fit existing letters
    std::vector<std::string> candidates;
    for (const auto& w : pool) {
        if ((int)w.size() != len) continue;
        bool ok = true;
        for (int k = 0; k < (int)len; ++k) {
            int r = slots[slotIdx].r + (slots[slotIdx].across ? 0 : k);
            int c = slots[slotIdx].c + (slots[slotIdx].across ? k : 0);
            char ch = grid.getLetter(r, c);
            if (ch != ' ' && ch != w[k]) { ok = false; break; }
        }
        if (ok) candidates.push_back(w);
    }

    // shuffle candidates for variability
    std::mt19937 gen(static_cast<unsigned>(std::time(nullptr)) + (int)idx);
    std::shuffle(candidates.begin(), candidates.end(), gen);

    // try each
    for (auto &cand : candidates) {
        // snapshot letters to restore on backtrack
        std::vector<char> snap;
        snap.reserve(slots[slotIdx].length);
        for (int k = 0; k < slots[slotIdx].length; ++k) {
            int r = slots[slotIdx].r + (slots[slotIdx].across ? 0 : k);
            int c = slots[slotIdx].c + (slots[slotIdx].across ? k : 0);
            snap.push_back(grid.getLetter(r,c));
        }

        if (!grid.placeWordInSlot(slotIdx, cand)) continue;
        if (backtrackPlace(grid, slotOrder, idx + 1, pool)) return true;

        // undo placement
        for (int k = 0; k < slots[slotIdx].length; ++k) {
            int r = slots[slotIdx].r + (slots[slotIdx].across ? 0 : k);
            int c = slots[slotIdx].c + (slots[slotIdx].across ? k : 0);
            grid.grid[r][c].letter = snap[k];
            grid.grid[r][c].isEditable = (snap[k] == ' ');
        }
        grid.slots[slotIdx].word.clear();
    }

    return false;
}
