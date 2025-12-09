#include "GUI.hpp"
#include <iostream>
#include <cctype>

GUI::GUI(CrosswordGrid& g)
    : window(sf::VideoMode(sf::Vector2u(600, 800)), "Crossword Puzzle"), grid(g)
{
    // Disable key repeat so holding a key doesn't flood input
    window.setKeyRepeatEnabled(false);

    // Try to load font and record result
    // SFML3 uses openFromFile
    // Font path is relative to the working directory (build folder where exe is)
    fontLoaded = font.openFromFile("assets/fonts/OpenSans.ttf");
    if (!fontLoaded) {
        std::cerr << "Warning: failed to load font 'assets/fonts/OpenSans.ttf'. Text will not be displayed." << std::endl;
    }

    if (fontLoaded) {
        numberTextPrototype.emplace(font, "", 12u);
        numberTextPrototype->setFillColor(sf::Color::Black);
    }

    // Initialize menu buttons
    startButton.setSize(sf::Vector2f(200.f, 50.f));
    startButton.setPosition(sf::Vector2f(300.f, 300.f));
    startButton.setFillColor(sf::Color::Green);

    quitButton.setSize(sf::Vector2f(200.f, 50.f));
    quitButton.setPosition(sf::Vector2f(300.f, 400.f));
    quitButton.setFillColor(sf::Color::Red);
}

void GUI::run() {
    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) {
            const sf::Event& event = *eventOpt;

            if (event.is<sf::Event::Closed>()) {
                window.close();
            }

            if (gameState == GameState::Menu) {
                if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                    handleMenuClick(mb->position.x, mb->position.y);
                }
            } else if (gameState == GameState::Playing) {
                if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                    handleMouseClick(mb->position.x, mb->position.y);
                }

                if (auto te = event.getIf<sf::Event::TextEntered>()) {
                    if (selectedRow != -1 && selectedCol != -1) {
                        char32_t unicode = te->unicode;
                        if (unicode < 128) {
                            char c = static_cast<char>(unicode);
                            if (std::isalpha(static_cast<unsigned char>(c))) {
                                grid.setLetter(selectedRow, selectedCol, static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
                            }
                        }
                    }
                }

                if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
                    handleKeyPress(*kp);
                }
            }
        }

        window.clear(sf::Color::White);

        if (gameState == GameState::Menu) {
            drawMenu();
        } else if (gameState == GameState::Playing) {
            drawGrid();
        } else if (gameState == GameState::Quit) {
            window.close();
        }

        window.display();
    }
}

void GUI::drawGrid() {
    for (int r = 0; r < grid.rows; r++) {
        for (int c = 0; c < grid.cols; c++) {

            sf::RectangleShape cell(sf::Vector2f(static_cast<float>(cellSize), static_cast<float>(cellSize)));
            cell.setPosition(sf::Vector2f(static_cast<float>(c * cellSize), static_cast<float>(r * cellSize)));

            if (grid.isBlocked(r, c))
                cell.setFillColor(sf::Color::Black);
            else
                cell.setFillColor(sf::Color(230, 230, 230));

            if (r == selectedRow && c == selectedCol)
                cell.setFillColor(sf::Color(180, 220, 255));

            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color::Black);

            window.draw(cell);

            if (!grid.isBlocked(r, c)) {
                char letter = grid.getLetter(r, c);
                if (letter != ' ' && fontLoaded) {
                    sf::Text text(font, std::string(1, letter), 24u);
                    text.setFillColor(sf::Color::Black);
                    text.setPosition(sf::Vector2f(static_cast<float>(c * cellSize + 10), static_cast<float>(r * cellSize + 5)));
                    window.draw(text);
                }

                // draw clue number if present
                if (showClueNumbers && fontLoaded && numberTextPrototype) {
                    int num = grid.getClueNumber(r, c);
                    if (num > 0) {
                        sf::Text ntext = *numberTextPrototype;
                        ntext.setString(std::to_string(num));
                        ntext.setPosition(sf::Vector2f(static_cast<float>(c * cellSize + 3), static_cast<float>(r * cellSize + 1)));
                        window.draw(ntext);
                    }
                }
            }
        }
    }
}

void GUI::handleMouseClick(int x, int y) {
    int col = x / cellSize;
    int row = y / cellSize;

    if (row >= 0 && row < grid.rows && col >= 0 && col < grid.cols) {
        if (!grid.isBlocked(row, col)) {
            selectedRow = row;
            selectedCol = col;
        }
    }
}

void GUI::handleKeyPress(const sf::Event::KeyPressed& key) {
    if (selectedRow == -1 || selectedCol == -1)
        return;

    switch (key.code) {
    case sf::Keyboard::Key::Backspace:
        grid.setLetter(selectedRow, selectedCol, ' ');
        break;
    case sf::Keyboard::Key::Left: {
        int c = selectedCol - 1;
        while (c >= 0 && grid.isBlocked(selectedRow, c)) --c;
        if (c >= 0) selectedCol = c;
        break;
    }
    case sf::Keyboard::Key::Right: {
        int c = selectedCol + 1;
        while (c < grid.cols && grid.isBlocked(selectedRow, c)) ++c;
        if (c < grid.cols) selectedCol = c;
        break;
    }
    case sf::Keyboard::Key::Up: {
        int r = selectedRow - 1;
        while (r >= 0 && grid.isBlocked(r, selectedCol)) --r;
        if (r >= 0) selectedRow = r;
        break;
    }
    case sf::Keyboard::Key::Down: {
        int r = selectedRow + 1;
        while (r < grid.rows && grid.isBlocked(r, selectedCol)) ++r;
        if (r < grid.rows) selectedRow = r;
        break;
    }
    default:
        break;
    }
}

void GUI::drawMenu() {
    // Draw title
    if (fontLoaded) {
        sf::Text titleText(font, "Crossword Puzzle", 48u);
        titleText.setFillColor(sf::Color::Black);
        titleText.setPosition(sf::Vector2f(120.f, 80.f));
        window.draw(titleText);
    }

    // Draw start button
    window.draw(startButton);
    if (fontLoaded) {
        sf::Text startText(font, "Start Game", 20u);
        startText.setFillColor(sf::Color::White);
        startText.setPosition(sf::Vector2f(250.f, 310.f));
        window.draw(startText);
    }

    // Draw quit button
    window.draw(quitButton);
    if (fontLoaded) {
        sf::Text quitText(font, "Quit", 20u);
        quitText.setFillColor(sf::Color::White);
        quitText.setPosition(sf::Vector2f(365.f, 410.f));
        window.draw(quitText);
    }
}

void GUI::handleMenuClick(int x, int y) {
    // Check if Start Game button was clicked
    if (x >= startButton.getPosition().x && x <= startButton.getPosition().x + startButton.getSize().x &&
        y >= startButton.getPosition().y && y <= startButton.getPosition().y + startButton.getSize().y) {
        gameState = GameState::Playing;
    }
    // Check if Quit button was clicked
    else if (x >= quitButton.getPosition().x && x <= quitButton.getPosition().x + quitButton.getSize().x &&
             y >= quitButton.getPosition().y && y <= quitButton.getPosition().y + quitButton.getSize().y) {
        gameState = GameState::Quit;
    }
}
