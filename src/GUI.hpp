#pragma once

#include <SFML/Graphics.hpp>
#include "CrosswordGrid.hpp"
#include <optional>
#include "History.hpp"
#include "trie.hpp"
#include "Generator.hpp"
#include "PuzzleBrowser.hpp"

class GUI {
public:
    GUI(CrosswordGrid& grid);

    void run();

private:
    sf::RenderWindow window;
    CrosswordGrid& grid;
    sf::Font font;

    bool fontLoaded = false;

    // Menu state
    enum class GameState { Menu, PuzzleSelect, Playing, Quit };
    GameState gameState = GameState::Menu;

    int cellSize = 40;
    int selectedRow = -1;
    int selectedCol = -1;
    bool showHelp = true;  // toggle with ? key
    bool showClueNumbers = true;
    std::optional<sf::Text> numberTextPrototype;

    // Menu buttons (approximate clickable areas)
    sf::RectangleShape startButton;
    sf::RectangleShape quitButton;

    // Puzzle browser
    PuzzleBrowser puzzleBrowser;
    int puzzleSelectedIndex = 0;
    int puzzleScrollOffset = 0;

    void drawMenu();
    void drawPuzzleSelect();
    void drawGrid();
    void drawCluePanel();
    void drawHelpPanel();
    void handleMenuClick(int x, int y);
    void handlePuzzleSelectClick(int x, int y);
    void handleMouseClick(int x, int y);
    void handleKeyPress(const sf::Event::KeyPressed& key);
    void handleTextEntered(char32_t unicode);
    void provideHint();
    void saveAuto();
    void loadAuto();
    void loadPuzzleByIndex(int index);

    // Data & helpers for generator/undo/hints
    History history;
    Trie dict;
    Generator generator{&dict};
};

