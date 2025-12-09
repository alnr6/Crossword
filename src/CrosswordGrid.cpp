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

void CrosswordGrid::buildSlots() {
    slots.clear();
    // across slots
    for (int i = 0; i < rows; ++i) {
        int j = 0;
        while (j < cols) {
            while (j < cols && grid[i][j].isBlocked) ++j;
            int start = j;
            while (j < cols && !grid[i][j].isBlocked) ++j;
            int len = j - start;
            if (len >= 2) {
                WordSlot ws; ws.r = i; ws.c = start; ws.length = len; ws.across = true; ws.word = std::string(len, ' ');
                ws.clueNumber = clueNumbers[i][start];
                slots.push_back(ws);
            }
        }
    }

    // down slots
    for (int j = 0; j < cols; ++j) {
        int i = 0;
        while (i < rows) {
            while (i < rows && grid[i][j].isBlocked) ++i;
            int start = i;
            while (i < rows && !grid[i][j].isBlocked) ++i;
            int len = i - start;
            if (len >= 2) {
                WordSlot ws; ws.r = start; ws.c = j; ws.length = len; ws.across = false; ws.word = std::string(len, ' ');
                ws.clueNumber = clueNumbers[start][j];
                slots.push_back(ws);
            }
        }
    }
}

std::vector<int> CrosswordGrid::getSlotsForCell(int r, int c) const {
    std::vector<int> out;
    for (size_t i = 0; i < slots.size(); ++i) {
        const WordSlot& s = slots[i];
        if (s.across) {
            if (r == s.r && c >= s.c && c < s.c + s.length) out.push_back((int)i);
        } else {
            if (c == s.c && r >= s.r && r < s.r + s.length) out.push_back((int)i);
        }
    }
    return out;
}

bool CrosswordGrid::placeWordInSlot(int slotIndex, const std::string& word) {
    if (slotIndex < 0 || slotIndex >= (int)slots.size()) return false;
    WordSlot& s = slots[slotIndex];
    if ((int)word.size() != s.length) return false;
    // check conflicts
    for (int k = 0; k < s.length; ++k) {
        int rr = s.r + (s.across ? 0 : k);
        int cc = s.c + (s.across ? k : 0);
        char existing = grid[rr][cc].letter;
        if (existing != ' ' && existing != word[k]) return false;
    }
    // place
    for (int k = 0; k < s.length; ++k) {
        int rr = s.r + (s.across ? 0 : k);
        int cc = s.c + (s.across ? k : 0);
        grid[rr][cc].letter = word[k];
        grid[rr][cc].isEditable = false;
    }
    s.word = word;
    return true;
}

void CrosswordGrid::clearSlotAssignments() {
    for (auto &s : slots) s.word.clear();
    for (int i = 0; i < rows; ++i) for (int j = 0; j < cols; ++j) if (!grid[i][j].isBlocked) { grid[i][j].letter = ' '; grid[i][j].isEditable = true; }
}
