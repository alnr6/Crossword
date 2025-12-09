#pragma once
#include <vector>
#include <string>
#include "Cell.hpp"

class CrosswordGrid {
public:
    int rows, cols;
    std::vector<std::vector<Cell>> grid;
    std::vector<std::vector<int>> clueNumbers; // 0 means no number

    CrosswordGrid(int r, int c);

    void setLetter(int r, int c, char letter);
    char getLetter(int r, int c) const;
    bool isBlocked(int r, int c) const;

    // Helpers
    bool inBounds(int r, int c) const;

    // Load/save simple puzzle format (rows cols, then rows lines with '.' for blocked,
    // uppercase letters or ' ' for empty)
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

    // Get the clue number for a cell (0 if none). Numbers are assigned according to
    // standard crossword rules: a cell gets a number if it's not blocked and (cell above
    // is blocked or cell to the left is blocked).
    int getClueNumber(int r, int c) const;

    // Randomize blocked cells with optional density (0.0 to 1.0, default 0.2 = 20%)
    // Also resets all letters to empty
    void randomizeBlockedCells(double blockDensity = 0.2);

    // Recompute clue numbers after grid changes
    void recomputeClueNumbers();
};
