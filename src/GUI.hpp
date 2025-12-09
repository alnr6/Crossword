#pragma once

#include <SFML/Graphics.hpp>
#include "CrosswordGrid.hpp"
#include <optional>

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
    enum class GameState { Menu, Playing, Quit };
    GameState gameState = GameState::Menu;

    int cellSize = 40;
    int selectedRow = -1;
    int selectedCol = -1;
    bool showClueNumbers = true;
    std::optional<sf::Text> numberTextPrototype;

    // Menu buttons (approximate clickable areas)
    sf::RectangleShape startButton;
    sf::RectangleShape quitButton;

    void drawMenu();
    void drawGrid();
    void handleMenuClick(int x, int y);
    void handleMouseClick(int x, int y);
    void handleKeyPress(const sf::Event::KeyPressed& key);
};

