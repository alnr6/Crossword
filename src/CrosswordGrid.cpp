#include "CrosswordGrid.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <ctime>

CrosswordGrid::CrosswordGrid(int r, int c) : rows(r), cols(c) {
    grid.resize(rows, std::vector<Cell>(cols));
    clueNumbers.resize(rows, std::vector<int>(cols, 0));
}

void CrosswordGrid::setLetter(int r, int c, char letter) {
    if (!grid[r][c].isBlocked && grid[r][c].isEditable)
        grid[r][c].letter = letter;
}

char CrosswordGrid::getLetter(int r, int c) const {
    return grid[r][c].letter;
}

bool CrosswordGrid::isBlocked(int r, int c) const {
    return grid[r][c].isBlocked;
}

bool CrosswordGrid::inBounds(int r, int c) const {
    return r >= 0 && r < rows && c >= 0 && c < cols;
}

bool CrosswordGrid::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in)
        return false;

    int r, c;
    if (!(in >> r >> c))
        return false;
    std::string line;
    std::getline(in, line); // consume rest of line

    rows = r; cols = c;
    grid.clear();
    grid.resize(rows, std::vector<Cell>(cols));
    clueNumbers.clear();
    clueNumbers.resize(rows, std::vector<int>(cols, 0));

    for (int i = 0; i < rows; ++i) {
        if (!std::getline(in, line))
            return false;
        // If line shorter, pad with spaces
        if ((int)line.size() < cols) line += std::string(cols - line.size(), ' ');
        for (int j = 0; j < cols; ++j) {
            char ch = line[j];
            if (ch == '.') {
                grid[i][j].isBlocked = true;
                grid[i][j].isEditable = false;
                grid[i][j].letter = ' ';
            } else if (ch == ' ' || ch == '_') {
                grid[i][j].isBlocked = false;
                grid[i][j].isEditable = true;
                grid[i][j].letter = ' ';
            } else {
                grid[i][j].isBlocked = false;
                grid[i][j].isEditable = false;
                grid[i][j].letter = ch;
            }
        }
    }

    // compute clue numbers
    int nextNumber = 1;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            clueNumbers[i][j] = 0;
            if (grid[i][j].isBlocked) continue;
            bool startsAcross = (j == 0) || grid[i][j-1].isBlocked;
            bool startsDown = (i == 0) || grid[i-1][j].isBlocked;
            if (startsAcross || startsDown) {
                clueNumbers[i][j] = nextNumber++;
            }
        }
    }

    return true;
}

bool CrosswordGrid::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << rows << " " << cols << '\n';
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j].isBlocked) out << '.';
            else if (grid[i][j].letter != ' ') out << grid[i][j].letter;
            else out << ' ';
        }
        out << '\n';
    }
    return true;
}

int CrosswordGrid::getClueNumber(int r, int c) const {
    if (!inBounds(r, c)) return 0;
    return clueNumbers[r][c];
}

void CrosswordGrid::randomizeBlockedCells(double blockDensity) {
    // Clamp density to [0, 1]
    if (blockDensity < 0.0) blockDensity = 0.0;
    if (blockDensity > 1.0) blockDensity = 1.0;

    // Seed random number generator
    std::mt19937 gen(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // Randomize each cell
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (dis(gen) < blockDensity) {
                grid[i][j].isBlocked = true;
                grid[i][j].isEditable = false;
                grid[i][j].letter = ' ';
            } else {
                grid[i][j].isBlocked = false;
                grid[i][j].isEditable = true;
                grid[i][j].letter = ' ';
            }
        }
    }

    // Recompute clue numbers
    recomputeClueNumbers();
}

void CrosswordGrid::recomputeClueNumbers() {
    // Clear all clue numbers
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            clueNumbers[i][j] = 0;
        }
    }

    // Assign new clue numbers
    int nextNumber = 1;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j].isBlocked) continue;
            bool startsAcross = (j == 0) || grid[i][j-1].isBlocked;
            bool startsDown = (i == 0) || grid[i-1][j].isBlocked;
            if (startsAcross || startsDown) {
                clueNumbers[i][j] = nextNumber++;
            }
        }
    }
}
