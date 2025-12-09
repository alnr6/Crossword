#pragma once
#include <vector>
#include <variant>

struct SetLetterAction {
    int r, c;
    char before;
    char after;
};

class History {
public:
    void pushSetLetter(int r, int c, char before, char after);
    bool canUndo() const;
    bool canRedo() const;
    SetLetterAction undo();
    SetLetterAction redo();
    void clear();

private:
    std::vector<SetLetterAction> past;
    std::vector<SetLetterAction> future;
};
