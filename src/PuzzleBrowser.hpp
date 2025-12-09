#pragma once
#include <vector>
#include <string>

class PuzzleBrowser {
public:
    PuzzleBrowser(const std::string& puzzleDir = "assets/puzzles");
    void refresh();
    const std::vector<std::string>& getPuzzles() const { return puzzles; }
    std::string getFullPath(int index) const;
    int count() const { return (int)puzzles.size(); }

private:
    std::string puzzleDir;
    std::vector<std::string> puzzles; // just filenames (without directory)
};
