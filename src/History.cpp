#include "History.hpp"

void History::pushSetLetter(int r, int c, char before, char after) {
    past.push_back({r,c,before,after});
    future.clear();
}

bool History::canUndo() const { return !past.empty(); }
bool History::canRedo() const { return !future.empty(); }

SetLetterAction History::undo() {
    SetLetterAction act = past.back();
    past.pop_back();
    future.push_back(act);
    // Return the reverse action (apply before -> after?) We'll return the reverse application so caller can set grid to 'before'
    return act;
}

SetLetterAction History::redo() {
    SetLetterAction act = future.back();
    future.pop_back();
    past.push_back(act);
    return act;
}

void History::clear() {
    past.clear(); future.clear();
}
