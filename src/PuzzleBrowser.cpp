#include "PuzzleBrowser.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

PuzzleBrowser::PuzzleBrowser(const std::string& dir) : puzzleDir(dir) {
    refresh();
}

void PuzzleBrowser::refresh() {
    puzzles.clear();
    if (!fs::exists(puzzleDir)) {
        return;
    }
    for (const auto& entry : fs::directory_iterator(puzzleDir)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            if (name.size() > 4 && name.substr(name.size() - 4) == ".txt") {
                puzzles.push_back(name);
            }
        }
    }
    std::sort(puzzles.begin(), puzzles.end());
}

std::string PuzzleBrowser::getFullPath(int index) const {
    if (index < 0 || index >= (int)puzzles.size()) return "";
    return puzzleDir + "/" + puzzles[index];
}
