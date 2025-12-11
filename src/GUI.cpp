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
    startButton.setPosition(sf::Vector2f(200.f, 300.f));
    startButton.setFillColor(sf::Color::Green);

    quitButton.setSize(sf::Vector2f(200.f, 50.f));
    quitButton.setPosition(sf::Vector2f(200.f, 400.f));
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
            } else if (gameState == GameState::PuzzleSelect) {
                if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                    handlePuzzleSelectClick(mb->position.x, mb->position.y);
                }
                if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Up) {
                        if (puzzleSelectedIndex > 0) puzzleSelectedIndex--;
                    } else if (kp->code == sf::Keyboard::Key::Down) {
                        if (puzzleSelectedIndex < puzzleBrowser.count() - 1) puzzleSelectedIndex++;
                    } else if (kp->code == sf::Keyboard::Key::Escape) {
                        gameState = GameState::Menu;
                    } else if (kp->code == sf::Keyboard::Key::Enter) {
                        loadPuzzleByIndex(puzzleSelectedIndex);
                        gameState = GameState::Playing;
                    }
                }
            } else if (gameState == GameState::Playing) {
                if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                    handleMouseClick(mb->position.x, mb->position.y);
                }

                if (auto te = event.getIf<sf::Event::TextEntered>()) {
                    handleTextEntered(te->unicode);
                }

                if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
                    handleKeyPress(*kp);
                }
            }
        }

        window.clear(sf::Color::White);

        if (gameState == GameState::Menu) {
            drawMenu();
        } else if (gameState == GameState::PuzzleSelect) {
            drawPuzzleSelect();
        } else if (gameState == GameState::Playing) {
            drawGrid();
            drawCluePanel();
            if (showHelp) drawHelpPanel();
        } else if (gameState == GameState::Quit) {
            window.close();
        }

        window.display();
    }
}

void GUI::drawGrid() {
    const int gridW = grid.cols * cellSize;
    const int gridH = grid.rows * cellSize;
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

    // highlight full words for selection
    if (selectedRow != -1 && selectedCol != -1) {
        auto slotIds = grid.getSlotsForCell(selectedRow, selectedCol);
        for (int sid : slotIds) {
            const auto& s = grid.getSlots()[sid];
            sf::RectangleShape highlight(sf::Vector2f(static_cast<float>(s.across ? s.length * cellSize : cellSize), static_cast<float>(s.across ? cellSize : s.length * cellSize)));
            highlight.setPosition(sf::Vector2f(static_cast<float>(s.c * cellSize), static_cast<float>(s.r * cellSize)));
            highlight.setFillColor(sf::Color(200, 240, 200, 100));
            window.draw(highlight);
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

void GUI::handleTextEntered(char32_t unicode) {
    if (selectedRow == -1 || selectedCol == -1) return;
    if (unicode < 128) {
        char c = static_cast<char>(unicode);
        if (std::isalpha(static_cast<unsigned char>(c))) {
            char up = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            char before = grid.getLetter(selectedRow, selectedCol);
            history.pushSetLetter(selectedRow, selectedCol, before, up);
            grid.setLetter(selectedRow, selectedCol, up);
        }
    }
}

void GUI::handleKeyPress(const sf::Event::KeyPressed& key) {
    if (selectedRow == -1 || selectedCol == -1)
        ;

    // Toggle help: ? or / key
    if (key.code == sf::Keyboard::Key::Slash) { showHelp = !showHelp; return; }

    // Undo/redo: Ctrl+Z / Ctrl+Y
    if (key.control && key.code == sf::Keyboard::Key::Z) {
        if (history.canUndo()) {
            auto act = history.undo();
            grid.setLetter(act.r, act.c, act.before);
        }
        return;
    }
    if (key.control && key.code == sf::Keyboard::Key::Y) {
        if (history.canRedo()) {
            auto act = history.redo();
            grid.setLetter(act.r, act.c, act.after);
        }
        return;
    }

    // Save/load
    if (key.control && key.code == sf::Keyboard::Key::S) { saveAuto(); return; }
    if (key.control && key.code == sf::Keyboard::Key::L) { loadAuto(); return; }

    // Hint
    if (key.code == sf::Keyboard::Key::H) { provideHint(); return; }

    // Try generator (G) if dictionary loaded
    if (key.code == sf::Keyboard::Key::.) {
        std::vector<std::string> pool = {"HELLO","WORLD","CROSS","WORDS","PUZZLE","GRID","STACK","QUEUE","TREE","GRAPH","ARRAY","VECTOR"};
        grid.clearSlotAssignments();
        grid.buildSlots();
        generator.generate(grid, pool);
        return;
    }

    switch (key.code) {
    case sf::Keyboard::Key::Backspace:
        history.pushSetLetter(selectedRow, selectedCol, grid.getLetter(selectedRow, selectedCol), ' ');
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
        quitText.setPosition(sf::Vector2f(275.f, 410.f));
        window.draw(quitText);
    }
}

void GUI::handleMenuClick(int x, int y) {
    // Check if Start Game button was clicked
    if (x >= startButton.getPosition().x && x <= startButton.getPosition().x + startButton.getSize().x &&
        y >= startButton.getPosition().y && y <= startButton.getPosition().y + startButton.getSize().y) {
        // Check if puzzles exist; if so, go to puzzle select; otherwise start with random
        if (puzzleBrowser.count() > 0) {
            puzzleSelectedIndex = 0;
            gameState = GameState::PuzzleSelect;
        } else {
            gameState = GameState::Playing;
        }
    }
    // Check if Quit button was clicked
    else if (x >= quitButton.getPosition().x && x <= quitButton.getPosition().x + quitButton.getSize().x &&
             y >= quitButton.getPosition().y && y <= quitButton.getPosition().y + quitButton.getSize().y) {
        gameState = GameState::Quit;
    }
}

void GUI::drawCluePanel() {
    if (!fontLoaded) return;
    int panelX = grid.cols * cellSize + 10;
    int panelW = 250;
    sf::RectangleShape panelBg(sf::Vector2f(static_cast<float>(panelW), static_cast<float>(grid.rows * cellSize)));
    panelBg.setPosition(sf::Vector2f(static_cast<float>(panelX), 0.f));
    panelBg.setFillColor(sf::Color(245, 245, 245));
    panelBg.setOutlineThickness(1);
    panelBg.setOutlineColor(sf::Color::Black);
    window.draw(panelBg);

    float y = 10.f;
    // Title
    sf::Text title(font, "Clues", 20u);
    title.setFillColor(sf::Color::Black);
    title.setPosition(sf::Vector2f(static_cast<float>(panelX + 10), y));
    window.draw(title);
    y += 30.f;

    // list across then down
    const auto& slots = grid.getSlots();
    for (size_t i = 0; i < slots.size(); ++i) {
        const auto &s = slots[i];
        std::string dir = s.across ? "A" : "D";
        std::string display = std::to_string(s.clueNumber) + dir + ": ";
        // build current pattern
        for (int k = 0; k < s.length; ++k) {
            int rr = s.r + (s.across ? 0 : k);
            int cc = s.c + (s.across ? k : 0);
            char ch = grid.getLetter(rr, cc);
            display.push_back(ch == ' ' ? '_' : ch);
        }

        sf::Text t(font, display, 14u);
        t.setFillColor(sf::Color::Black);
        t.setPosition(sf::Vector2f(static_cast<float>(panelX + 10), y));
        window.draw(t);
        y += 18.f;
        if (y > grid.rows * cellSize - 30) break;
    }
}

void GUI::saveAuto() {
    if (grid.saveToFile("autosave.txt")) {
        std::cerr << "Saved autosave.txt" << std::endl;
    } else {
        std::cerr << "Failed to save autosave.txt" << std::endl;
    }
}

void GUI::loadAuto() {
    if (grid.loadFromFile("autosave.txt")) {
        grid.recomputeClueNumbers();
        grid.buildSlots();
        std::cerr << "Loaded autosave.txt" << std::endl;
    } else {
        std::cerr << "Failed to load autosave.txt" << std::endl;
    }
}

void GUI::provideHint() {
    if (selectedRow == -1 || selectedCol == -1) return;
    auto slotIds = grid.getSlotsForCell(selectedRow, selectedCol);
    if (slotIds.empty()) return;
    int sid = slotIds[0];
    const auto &s = grid.getSlots()[sid];
    // build pattern
    std::string pattern;
    int posInSlot = -1;
    for (int k = 0; k < s.length; ++k) {
        int rr = s.r + (s.across ? 0 : k);
        int cc = s.c + (s.across ? k : 0);
        char ch = grid.getLetter(rr, cc);
        pattern.push_back(ch == ' ' ? '_' : ch);
        if (rr == selectedRow && cc == selectedCol) posInSlot = k;
    }

    auto cand = dict.collectWithPattern(pattern);
    if (cand.empty()) {
        // no dictionary matches; reveal a letter at selected cell if any common letter in pool fallback
        // reveal nothing
        std::cerr << "No hints available (dictionary or matches not found)." << std::endl;
        return;
    }
    // if exact one candidate fill the whole slot
    if (cand.size() == 1) {
        grid.placeWordInSlot(sid, cand[0]);
    } else {
        // reveal one letter at selected position from first candidate
        if (posInSlot >= 0 && posInSlot < (int)cand[0].size()) {
            char reveal = cand[0][posInSlot];
            history.pushSetLetter(selectedRow, selectedCol, grid.getLetter(selectedRow, selectedCol), reveal);
            grid.setLetter(selectedRow, selectedCol, reveal);
        }
    }
}

void GUI::drawPuzzleSelect() {
    if (!fontLoaded) return;

    // Title
    sf::Text titleText(font, "Select a Puzzle", 48u);
    titleText.setFillColor(sf::Color::Black);
    titleText.setPosition(sf::Vector2f(100.f, 40.f));
    window.draw(titleText);

    // List of puzzles
    const auto& puzzles = puzzleBrowser.getPuzzles();
    float y = 150.f;
    const float itemHeight = 40.f;

    for (int i = 0; i < puzzleBrowser.count(); ++i) {
        if (i < puzzleScrollOffset) continue;
        if (y > 700.f) break;

        sf::RectangleShape itemBg(sf::Vector2f(400.f, itemHeight));
        itemBg.setPosition(sf::Vector2f(100.f, y));
        itemBg.setFillColor(i == puzzleSelectedIndex ? sf::Color::Cyan : sf::Color(220, 220, 220));
        itemBg.setOutlineThickness(1);
        itemBg.setOutlineColor(sf::Color::Black);
        window.draw(itemBg);

        sf::Text itemText(font, puzzles[i], 18u);
        itemText.setFillColor(sf::Color::Black);
        itemText.setPosition(sf::Vector2f(110.f, y + 10.f));
        window.draw(itemText);

        y += itemHeight + 5.f;
    }

    // Instructions
    sf::Text instrText(font, "Up/Down: Select | Enter: Load | Esc: Back", 14u);
    instrText.setFillColor(sf::Color(100, 100, 100));
    instrText.setPosition(sf::Vector2f(50.f, 750.f));
    window.draw(instrText);
}

void GUI::handlePuzzleSelectClick(int x, int y) {
    // Simple click-to-select
    const auto& puzzles = puzzleBrowser.getPuzzles();
    float itemY = 150.f;
    const float itemHeight = 40.f;
    for (int i = 0; i < puzzleBrowser.count(); ++i) {
        if (x >= 100.f && x <= 500.f && y >= itemY && y <= itemY + itemHeight) {
            puzzleSelectedIndex = i;
            loadPuzzleByIndex(i);
            gameState = GameState::Playing;
            return;
        }
        itemY += itemHeight + 5.f;
        if (itemY > 700.f) break;
    }
}

void GUI::loadPuzzleByIndex(int index) {
    std::string path = puzzleBrowser.getFullPath(index);
    if (!path.empty() && grid.loadFromFile(path)) {
        grid.recomputeClueNumbers();
        grid.buildSlots();
        selectedRow = -1;
        selectedCol = -1;
        history.clear();
        std::cerr << "Loaded puzzle: " << path << std::endl;
    } else {
        std::cerr << "Failed to load puzzle: " << path << std::endl;
    }
}

void GUI::drawHelpPanel() {
    if (!fontLoaded) return;

    // Semi-transparent background overlay
    sf::RectangleShape bgOverlay(sf::Vector2f(600.f, 800.f));
    bgOverlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(bgOverlay);

    // Help panel box
    sf::RectangleShape helpBox(sf::Vector2f(500.f, 650.f));
    helpBox.setPosition(sf::Vector2f(50.f, 75.f));
    helpBox.setFillColor(sf::Color(240, 240, 240));
    helpBox.setOutlineThickness(2);
    helpBox.setOutlineColor(sf::Color::Black);
    window.draw(helpBox);

    // Title
    sf::Text title(font, "KEYBOARD SHORTCUTS", 24u);
    title.setFillColor(sf::Color::Black);
    title.setPosition(sf::Vector2f(70.f, 90.f));
    window.draw(title);

    // Shortcuts list
    std::vector<std::pair<std::string, std::string>> shortcuts = {
        {"Ctrl+Z", "Undo last letter"},
        {"Ctrl+Y", "Redo last letter"},
        {"Ctrl+S", "Save to autosave.txt"},
        {"Ctrl+L", "Load from autosave.txt"},
        {"H", "Hint for selected cell"},
        {".", "Clear Puzzle"},
        {"Arrow Keys", "Move selection"},
        {"Backspace", "Clear cell"},
        {"Letters", "Fill cell"},
        {"/", "Toggle this help"}
    };

    float y = 140.f;
    const float lineHeight = 40.f;

    for (const auto& shortcut : shortcuts) {
        // Key label
        sf::Text keyText(font, shortcut.first, 16u);
        keyText.setFillColor(sf::Color::Blue);
        keyText.setStyle(sf::Text::Bold);
        keyText.setPosition(sf::Vector2f(80.f, y));
        window.draw(keyText);

        // Arrow separator
        sf::Text arrow(font, "->", 16u);
        arrow.setFillColor(sf::Color::Black);
        arrow.setPosition(sf::Vector2f(200.f, y));
        window.draw(arrow);

        // Description
        sf::Text descText(font, shortcut.second, 16u);
        descText.setFillColor(sf::Color::Black);
        descText.setPosition(sf::Vector2f(240.f, y));
        window.draw(descText);

        y += lineHeight;
    }

    // Footer
    sf::Text footer(font, "Press / to close", 14u);
    footer.setFillColor(sf::Color(100, 100, 100));
    footer.setPosition(sf::Vector2f(750.f, 700.f));
    window.draw(footer);
}

