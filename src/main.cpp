#include "CrosswordGrid.hpp"
#include "GUI.hpp"

int main() {
    CrosswordGrid grid(10, 10);

    // Randomize blocked cells with 25% density
    grid.randomizeBlockedCells(0.25);

    GUI gui(grid);
    gui.run();

    return 0;
}
