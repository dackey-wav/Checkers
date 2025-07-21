#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <optional>
#include <chrono>
#include "../include/Board.hpp"
#include "../include/AI.hpp"

const int BOARD_SIZE = 8;
const int SPRITE_SIZE = 16; 
const int CELL_SIZE = 16;   
const int BOARD_OFFSET_X = 7; 
const int BOARD_OFFSET_Y = 7;
const int WINDOW_SIZE = 800; 
const int BOARD_PIXEL_SIZE = 8 * CELL_SIZE; 

std::vector<Move> getPossibleMoves(int startCol, int startRow, Board& board, Piececolor color) {
    std::vector<Move> allMoves = board.getAllValidMoves(color);
    std::vector<Move> filtered;
    for (const auto& move : allMoves) {
        if (move.getFrom().row == startRow && move.getFrom().col == startCol) {
            filtered.push_back(move);
        }
    } 
    return filtered;
}

bool isGameOver(Board& board, Piececolor player) {
    return board.getAllValidMoves(player).empty();
}

enum class ScreenState { Start, Game, GameOver, Options };
ScreenState screenState = ScreenState::Start;

struct CheckerAnimation {
    bool active = false;
    sf::Vector2i from, to;
    float progress = 0.f; // od 0 do 1
    float duration = 0.2f; // 
    Piececolor color;
    Piecetype type;
};
CheckerAnimation checkerAnim;
sf::Clock animClock;

int aiDepth = 2;
int gameMode = 2; // 1 — PvP, 2 — PvE

int main() {
    
    if (sf::Joystick::isConnected(0)) {
            std::cout << "Joystick 0 connected: " << sf::Joystick::getIdentification(0).name.toAnsiString() << std::endl;
    }

    sf::Texture boardTexture;
    if (!boardTexture.loadFromFile("../assets/board_plain_01.png")) {
        std::cout << "Error loading board texture" << std::endl;
        return 1;
    }

    //
    sf::Texture checkersTexture;
    if (!checkersTexture.loadFromFile("../assets/checkers_topDown.png")) {
        std::cout << "Error loading checkers texture" << std::endl;
        return 1;
    }
    checkersTexture.setSmooth(false);

    sf::Sprite boardSprite(boardTexture);

    sf::IntRect whiteRect     ({0 * SPRITE_SIZE, 0}, {SPRITE_SIZE, SPRITE_SIZE});
    sf::IntRect whiteKingRect({1 * SPRITE_SIZE, 0}, {SPRITE_SIZE, SPRITE_SIZE});
    sf::IntRect blackRect     ({2 * SPRITE_SIZE, 0}, {SPRITE_SIZE, SPRITE_SIZE});
    sf::IntRect blackKingRect({3 * SPRITE_SIZE, 0}, {SPRITE_SIZE, SPRITE_SIZE});

    sf::Sprite checkerSprite(checkersTexture);
    checkerSprite.setOrigin(sf::Vector2f(SPRITE_SIZE / 2.f, SPRITE_SIZE / 2.f));

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WINDOW_SIZE, WINDOW_SIZE)), "Checkers");

    float scale = float(WINDOW_SIZE) / (BOARD_PIXEL_SIZE + 2 * BOARD_OFFSET_X);

    float boardDrawOffset = (WINDOW_SIZE - (BOARD_PIXEL_SIZE + 2 * BOARD_OFFSET_X) * scale) / 2.f;

    std::optional<sf::Vector2i> selectedCellOpt = std::nullopt; 

    sf::RectangleShape selectionHighlight;
    selectionHighlight.setFillColor(sf::Color(191, 193, 197)); 

    sf::CircleShape possibleMoveHighlight;
    possibleMoveHighlight.setFillColor(sf::Color(165, 170, 153));

    Board board; 
    board.initialize(); 

    Piececolor currentPlayer = Piececolor::White; 

    bool gameOver = false;
    std::string gameOverText;

    sf::FloatRect easyBounds, hardBounds, pvpBounds, pveBounds, backBounds;

    sf::Vector2i joySelectedCell(0, 0); 
    bool joyHadInput = false;

    while (window.isOpen()) {
        std::optional<sf::Event> optEvent;
        while (optEvent = window.pollEvent()) {
            sf::Event event = *optEvent;
            if (event.is<sf::Event::Closed>()) {
                window.close();
            }

            if (sf::Joystick::isConnected(0)) {
                float x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
                float y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);

                static sf::Clock joyMoveClock;
                float moveDelay = 0.15f; 

                if (joyMoveClock.getElapsedTime().asSeconds() > moveDelay) {
                    bool moved = false;
                    if (x > 50 && joySelectedCell.x < BOARD_SIZE - 1) { 
                        joySelectedCell.x++; 
                        moved = true;
                    }
                    else if (x < -50 && joySelectedCell.x > 0) { 
                        joySelectedCell.x--; 
                        moved = true;
                    }
                    else if (y > 50 && joySelectedCell.y > 0) { 
                        joySelectedCell.y--; 
                        moved = true;
                    }
                    else if (y < -50 && joySelectedCell.y < BOARD_SIZE - 1) { 
                        joySelectedCell.y++; 
                        moved = true;
                    }
                    
                    if (moved) {
                        joyMoveClock.restart();
                    }
                }

                if (screenState == ScreenState::Game) {
                    if (sf::Joystick::isButtonPressed(0, 1) && !joyHadInput) {
                        joyHadInput = true;
                        if (screenState == ScreenState::Game) {
                            int clickedCol = joySelectedCell.x;
                            int clickedRow = joySelectedCell.y;
                            Tile& clickedTile = board.getTile(clickedRow, clickedCol);
                            if (selectedCellOpt.has_value()) {
                                int selCol = selectedCellOpt->x;
                                int selRow = selectedCellOpt->y;
                                if (clickedCol == selCol && clickedRow == selRow) {
                                    selectedCellOpt = std::nullopt;
                                } else {
                                    std::vector<Move> possibleMoves = getPossibleMoves(selCol, selRow, board, currentPlayer);
                                    bool moveDone = false;
                                    for (const auto& move : possibleMoves) {
                                        if (move.getTo().col == clickedCol && move.getTo().row == clickedRow) {
                                            board.applyMove(move);
                                            std::cout << "Board evaluation: " << evaluateBoard(board) << std::endl;
                                            moveDone = true;
                                            
                                            std::vector<Move> nextCaptures = getPossibleMoves(clickedCol, clickedRow, board, currentPlayer);
                                            bool hasFurtherCapture = false;
                                            for (const auto& m : nextCaptures) {
                                                if (!m.getCaptured().empty()) {
                                                    hasFurtherCapture = true;
                                                    break;
                                                }
                                            }
                                            
                                            if (!move.getCaptured().empty() && hasFurtherCapture) {
                                                selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                            } else {
                                                selectedCellOpt = std::nullopt;
                                                currentPlayer = (currentPlayer == Piececolor::White) ? Piececolor::Black : Piececolor::White;
                                                
                                                if (isGameOver(board, currentPlayer)) {
                                                    gameOver = true;
                                                    gameOverText = (currentPlayer == Piececolor::White) ? "Black wins!" : "White wins!";
                                                    screenState = ScreenState::GameOver;
                                                }
                                                
                                                if (gameMode == 2 && currentPlayer == Piececolor::Black) {
                                                    std::vector<Move> aiMoves = board.getAllValidMoves(Piececolor::Black);
                                                    if (!aiMoves.empty()) {
                                                        auto start = std::chrono::high_resolution_clock::now();
                                                        Move aiMove = findBestMove(board, aiDepth);
                                                        auto end = std::chrono::high_resolution_clock::now();
                                                        std::cout << "AI move time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
                                                        board.applyMove(aiMove);
                                                        std::cout << "Board evaluation: " << evaluateBoard(board) << std::endl;
                                                        currentPlayer = Piececolor::White;
                                                        selectedCellOpt = std::nullopt;
                                                        
                                                        if (isGameOver(board, currentPlayer)) {
                                                            gameOver = true;
                                                            gameOverText = (currentPlayer == Piececolor::White) ? "Black wins!" : "White wins!";
                                                            screenState = ScreenState::GameOver;
                                                        }
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }
                                    if (!moveDone) {
                                        if (clickedTile.hasPiece() && clickedTile.getPiece()->getColor() == currentPlayer) {
                                            selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                        } else {
                                            selectedCellOpt = std::nullopt;
                                        }
                                    }
                                }
                            } else {
                                if (clickedTile.hasPiece() && clickedTile.getPiece()->getColor() == currentPlayer) {
                                    selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                } 
                            }
                        }
                        else if (sf::Joystick::isButtonPressed(0, 1) && !joyHadInput) {
                            joyHadInput = true;
                            selectedCellOpt = std::nullopt;
                        }
                    }
                    if (!sf::Joystick::isButtonPressed(0, 0) && !sf::Joystick::isButtonPressed(0, 1)) {
                        joyHadInput = false;
                    }
                }
            }

            if (screenState == ScreenState::GameOver) {
                if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                        sf::Vector2i mousePos = mouseButtonPressed->position;
                        sf::FloatRect buttonRect(
                            sf::Vector2f(WINDOW_SIZE / 2 - 150, WINDOW_SIZE / 2 + 60 - 25),
                            sf::Vector2f(300, 50)
                        );
                        if (buttonRect.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
                            screenState = ScreenState::Start;
                        }
                    }
                }
            }

            if (screenState == ScreenState::Start) {
                if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                        sf::Vector2i mousePos = mouseButtonPressed->position;
                        // Start Game
                        if (mousePos.y > 200 && mousePos.y < 260) {
                            board.initialize();
                            currentPlayer = Piececolor::White;
                            selectedCellOpt = std::nullopt;
                            gameOver = false;
                            gameOverText.clear();
                            screenState = ScreenState::Game;
                        }
                        // Options
                        else if (mousePos.y > 270 && mousePos.y < 330) {
                            screenState = ScreenState::Options;
                        }
                    }
                }
            }

            if (screenState == ScreenState::Game) {
                if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                        if (gameMode == 2 && currentPlayer == Piececolor::Black)
                            continue;

                        sf::Vector2i mousePos = mouseButtonPressed->position;

                        //
                        float textureX = (mousePos.x - boardDrawOffset) / scale;
                        float textureY = (mousePos.y - boardDrawOffset) / scale;

                        //
                        if (textureX >= BOARD_OFFSET_X && textureY >= BOARD_OFFSET_Y) {
                            int clickedCol = static_cast<int>((textureX - BOARD_OFFSET_X) / CELL_SIZE);
                            int clickedRow = static_cast<int>((textureY - BOARD_OFFSET_Y) / CELL_SIZE);

                            if (clickedCol >= 0 && clickedCol < BOARD_SIZE && clickedRow >= 0 && clickedRow < BOARD_SIZE) {
                                Tile& clickedTile = board.getTile(clickedRow, clickedCol);
                                if (selectedCellOpt.has_value()) {
                                    int selCol = selectedCellOpt->x;
                                    int selRow = selectedCellOpt->y;

                                    if (clickedCol == selCol && clickedRow == selRow) {
                                        selectedCellOpt = std::nullopt;
                                        if (gameMode == 2 && currentPlayer == Piececolor::Black && !selectedCellOpt.has_value()) {
                                            std::vector<Move> aiMoves = board.getAllValidMoves(Piececolor::Black);
                                            if (!aiMoves.empty()) {
                                                auto start = std::chrono::high_resolution_clock::now();
                                                Move aiMove = findBestMove(board, aiDepth); 
                                                auto end = std::chrono::high_resolution_clock::now();
                                                std::cout << "AI move time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
                                                board.applyMove(aiMove);
                                                std::cout << "Board evaluation: " << evaluateBoard(board) << std::endl;
                                                currentPlayer = Piececolor::White;
                                                selectedCellOpt = std::nullopt;
                                            }
                                        }
                                    } else {
                                        bool canMove = false;
                                        Tile& selectedTile = board.getTile(selRow, selCol);
                                        Piececolor selectedColor = selectedTile.getPiece()->getColor();

                                        std::vector<Move> possibleMoves = getPossibleMoves(selCol, selRow, board, selectedColor);
                                        bool moveDone = false;
                                        for (const auto& move : possibleMoves) {
                                            if (move.getTo().col == clickedCol && move.getTo().row == clickedRow) {
                                                board.applyMove(move);
                                                std::cout << "Board evaluation: " << evaluateBoard(board) << std::endl;
                                                moveDone = true;

                                                std::vector<Move> nextCaptures = getPossibleMoves(clickedCol, clickedRow, board, currentPlayer);
                                                bool hasFurtherCapture = false;
                                                for (const auto& m : nextCaptures) {
                                                    if (!m.getCaptured().empty()) {
                                                        hasFurtherCapture = true;
                                                        break;
                                                    }
                                                }

                                                if (!move.getCaptured().empty() && hasFurtherCapture) {
                                                    selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                                } else {
                                                    selectedCellOpt = std::nullopt;
                                                    currentPlayer = (currentPlayer == Piececolor::White) ? Piececolor::Black : Piececolor::White;

                                                    if (isGameOver(board, currentPlayer)) {
                                                        gameOver = true;
                                                        gameOverText = (currentPlayer == Piececolor::White) ? "Black wins!" : "White wins!";
                                                        screenState = ScreenState::GameOver; 
                                                    }

                                                    if (gameMode == 2 && currentPlayer == Piececolor::Black) {
                                                        std::vector<Move> aiMoves = board.getAllValidMoves(Piececolor::Black);
                                                        if (!aiMoves.empty()) {
                                                            auto start = std::chrono::high_resolution_clock::now();
                                                            Move aiMove = findBestMove(board, aiDepth); // 
                                                            auto end = std::chrono::high_resolution_clock::now();
                                                            std::cout << "AI move time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
                                                            board.applyMove(aiMove);
                                                            std::cout << "Board evaluation: " << evaluateBoard(board) << std::endl;
                                                            currentPlayer = Piececolor::White;
                                                            selectedCellOpt = std::nullopt;

                                                            if (isGameOver(board, currentPlayer)) {
                                                                gameOver = true;
                                                                gameOverText = (currentPlayer == Piececolor::White) ? "Black wins!" : "White wins!";
                                                                screenState = ScreenState::GameOver;
                                                            }
                                                        }
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                        if (!moveDone) {
                                            if (clickedTile.hasPiece() && clickedTile.getPiece()->getColor() == currentPlayer) {
                                                selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                            } else {
                                                selectedCellOpt = std::nullopt;
                                            }
                                        }
                                    }
                                } else {
                                    if (clickedTile.hasPiece() && clickedTile.getPiece()->getColor() == currentPlayer) {
                                        selectedCellOpt = sf::Vector2i(clickedCol, clickedRow);
                                    } else {
                                        selectedCellOpt = std::nullopt;
                                    }
                                }
                            }
                        }
                    }
                }

                if (auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scan::Escape) {
                        screenState = ScreenState::Start;
                        continue;
                    }
                }

                window.clear();

                boardSprite.setScale(sf::Vector2f(scale, scale));
                boardSprite.setPosition(sf::Vector2f(boardDrawOffset, boardDrawOffset));
                window.draw(boardSprite);

                float scaledCellSize = CELL_SIZE * scale;

                if (selectedCellOpt.has_value()) {
                    int selCol = selectedCellOpt.value().x;
                    int selRow = selectedCellOpt.value().y;
                    Tile& selectedTile = board.getTile(selRow, selCol);

                    if (selectedTile.hasPiece() && selectedTile.getPiece()->getColor() == currentPlayer) {
                        selectionHighlight.setSize(sf::Vector2f(scaledCellSize, scaledCellSize));
                        selectionHighlight.setPosition(sf::Vector2f(
                            boardDrawOffset + scale * BOARD_OFFSET_X + selCol * scaledCellSize,
                            boardDrawOffset + scale * BOARD_OFFSET_Y + selRow * scaledCellSize
                        ));
                        window.draw(selectionHighlight);

                        Piececolor selectedColor = selectedTile.getPiece()->getColor();
                        std::vector<Move> moves = getPossibleMoves(selCol, selRow, board, selectedColor);
                        possibleMoveHighlight.setRadius(scaledCellSize * 0.25f);
                        possibleMoveHighlight.setOrigin(sf::Vector2f(possibleMoveHighlight.getRadius(), possibleMoveHighlight.getRadius()));

                        for (const auto& move : moves) {
                            possibleMoveHighlight.setPosition(sf::Vector2f(
                                boardDrawOffset + scale * BOARD_OFFSET_X + move.getTo().col * scaledCellSize + scaledCellSize / 2.f,
                                boardDrawOffset + scale * BOARD_OFFSET_Y + move.getTo().row * scaledCellSize + scaledCellSize / 2.f
                            ));
                            window.draw(possibleMoveHighlight);
                        }
                    }
                }

                for (int row = 0; row < BOARD_SIZE; ++row) {
                    for (int col = 0; col < BOARD_SIZE; ++col) {
                        Tile& tile = board.getTile(row, col);
                        if (tile.hasPiece()) {
                            Piececolor color = tile.getPiece()->getColor();
                            Piecetype type = tile.getPiece()->getType();

                            if (color == Piececolor::White) {
                                checkerSprite.setTextureRect((type == Piecetype::King) ? whiteKingRect : whiteRect);
                            } else {
                                checkerSprite.setTextureRect((type == Piecetype::King) ? blackKingRect : blackRect);
                            }

                            checkerSprite.setScale(sf::Vector2f(scale * 0.8f, scale * 0.8f)); 
                            checkerSprite.setPosition(sf::Vector2f(
                                boardDrawOffset + scale * (BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2.f),
                                boardDrawOffset + scale * (BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2.f)
                            ));
                            window.draw(checkerSprite);
                        }
                    }
                }

                if (gameOver) {
                    sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
                    overlay.setFillColor(sf::Color(0, 89, 96, 112));
                    window.draw(overlay);

                    sf::Font font;
                    if (!font.openFromFile("visitor2.ttf")) {
                        std::cout << "Error loading font\n";
                    }

                    sf::Text text(font, gameOverText + "\nGame Over", 48);
                    text.setFillColor(sf::Color(234, 240, 216));
                    text.setStyle(sf::Text::Bold);

                    auto textBounds = text.getLocalBounds();
                    text.setOrigin(sf::Vector2f(
                        textBounds.position.x + textBounds.size.x / 2,
                        textBounds.position.y + textBounds.size.y / 2
                    ));
                    text.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 - 60));
                    window.draw(text);

                    sf::Text menuText(font, "Return to menu", 24);
                    menuText.setFillColor(sf::Color(234, 240, 216));
                    auto menuBounds = menuText.getLocalBounds();
                    menuText.setOrigin(sf::Vector2f(
                        menuBounds.position.x + menuBounds.size.x / 2,
                        menuBounds.position.y + menuBounds.size.y / 2
                    ));
                    menuText.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 + 60));
                    window.draw(menuText);
                }
            }

            if (screenState == ScreenState::GameOver) {
                if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                        sf::Vector2i mousePos = mouseButtonPressed->position;
                        sf::FloatRect buttonRect(
                            sf::Vector2f(WINDOW_SIZE / 2 - 150, WINDOW_SIZE / 2 + 60 - 25), // position
                            sf::Vector2f(300, 50) // size
                        );
                        if (buttonRect.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
                            screenState = ScreenState::Start;
                            continue;
                        }
                    }
                }
                continue;
            }

            if (screenState == ScreenState::Options) {
                if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                        sf::Vector2i mousePos = mouseButtonPressed->position;
                        // Easy AI
                        if (sf::FloatRect(sf::Vector2f(WINDOW_SIZE/2 - easyBounds.size.x/2, 200 - easyBounds.size.y/2), 
                        sf::Vector2f(easyBounds.size.x, easyBounds.size.y))
                            .contains(sf::Vector2f(mousePos))) {
                            aiDepth = 3;
                        }
                        // Hard AI
                        if (sf::FloatRect(sf::Vector2f(WINDOW_SIZE/2 - hardBounds.size.x/2, 250 - hardBounds.size.y/2), 
                            sf::Vector2f(hardBounds.size.x, hardBounds.size.y))
                            .contains(sf::Vector2f(mousePos))) {
                            aiDepth = 9;
                        }
                        // PvP
                        if (sf::FloatRect(sf::Vector2f(WINDOW_SIZE/2 - pvpBounds.size.x/2, 300 - pvpBounds.size.y/2), 
                            sf::Vector2f(pvpBounds.size.x, pvpBounds.size.y))
                            .contains(sf::Vector2f(mousePos))) {
                            gameMode = 1;
                        }
                        // PvE
                        if (sf::FloatRect(sf::Vector2f(WINDOW_SIZE/2 - pveBounds.size.x/2, 350 - pveBounds.size.y/2), 
                            sf::Vector2f(pveBounds.size.x, pveBounds.size.y))
                            .contains(sf::Vector2f(mousePos))) {
                            gameMode = 2;
                        }
                        // Back
                        if (sf::FloatRect(sf::Vector2f(WINDOW_SIZE/2 - backBounds.size.x/2, 420 - backBounds.size.y/2), 
                            sf::Vector2f(backBounds.size.x, backBounds.size.y))
                            .contains(sf::Vector2f(mousePos))) {
                            screenState = ScreenState::Start;
                        }
                    }
                }
            }
        }

        window.clear();

        if (screenState == ScreenState::Start) {
            window.clear(sf::Color(89, 96, 112));
            sf::Font font;
            if (!font.openFromFile("visitor2.ttf")) {
                std::cout << "Error loading font\n";
            }
            sf::Text title(font, "Checkers", 64);
            title.setFillColor(sf::Color(221, 227, 206));
            auto titleBounds = title.getLocalBounds();
            title.setOrigin(sf::Vector2f(
                titleBounds.position.x + titleBounds.size.x / 2,
                titleBounds.position.y + titleBounds.size.y / 2
            ));
            title.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 100));
            window.draw(title);

            sf::Text startText(font, "Start Game", 36);
            startText.setFillColor(sf::Color(234, 240, 216));
            auto startBounds = startText.getLocalBounds();
            startText.setOrigin(sf::Vector2f(startBounds.position.x + startBounds.size.x / 2, startBounds.position.y + startBounds.size.y / 2));
            startText.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 230));
            window.draw(startText);

            sf::Text optionsText(font, "Options", 36);
            optionsText.setFillColor(sf::Color(234, 240, 216));
            auto optionsBounds = optionsText.getLocalBounds();
            optionsText.setOrigin(sf::Vector2f(optionsBounds.position.x + optionsBounds.size.x / 2, optionsBounds.position.y + optionsBounds.size.y / 2));
            optionsText.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 300));
            window.draw(optionsText);
        }
        
        // if (sf::Joystick::isConnected(0) && screenState == ScreenState::Game) {
        //     sf::RectangleShape joyCursor;
        //     joyCursor.setSize(sf::Vector2f(scaledCellSize, scaledCellSize));
        //     joyCursor.setFillColor(sf::Color::Transparent);
        //     joyCursor.setOutlineColor(sf::Color(255, 255, 0, 150));
        //     joyCursor.setOutlineThickness(3.0f);
        //     joyCursor.setPosition(sf::Vector2f(
        //         boardDrawOffset + scale * BOARD_OFFSET_X + joySelectedCell.x * scaledCellSize,
        //         boardDrawOffset + scale * BOARD_OFFSET_Y + joySelectedCell.y * scaledCellSize
        //     ));
        //     window.draw(joyCursor);
        // }

        if (screenState == ScreenState::Game) {
            boardSprite.setScale(sf::Vector2f(scale, scale));
            boardSprite.setPosition(sf::Vector2f(boardDrawOffset, boardDrawOffset));
            window.draw(boardSprite);

            float scaledCellSize = CELL_SIZE * scale;

            if (selectedCellOpt.has_value()) {
                int selCol = selectedCellOpt.value().x;
                int selRow = selectedCellOpt.value().y;
                Tile& selectedTile = board.getTile(selRow, selCol);

                if (selectedTile.hasPiece() && selectedTile.getPiece()->getColor() == currentPlayer) {
                    selectionHighlight.setSize(sf::Vector2f(scaledCellSize, scaledCellSize));
                    selectionHighlight.setPosition(sf::Vector2f(
                        boardDrawOffset + scale * BOARD_OFFSET_X + selCol * scaledCellSize,
                        boardDrawOffset + scale * BOARD_OFFSET_Y + selRow * scaledCellSize
                    ));
                    window.draw(selectionHighlight);

                    Piececolor selectedColor = selectedTile.getPiece()->getColor();
                    std::vector<Move> moves = getPossibleMoves(selCol, selRow, board, selectedColor);
                    possibleMoveHighlight.setRadius(scaledCellSize * 0.25f);
                    possibleMoveHighlight.setOrigin(sf::Vector2f(possibleMoveHighlight.getRadius(), possibleMoveHighlight.getRadius()));

                    for (const auto& move : moves) {
                        possibleMoveHighlight.setPosition(sf::Vector2f(
                            boardDrawOffset + scale * BOARD_OFFSET_X + move.getTo().col * scaledCellSize + scaledCellSize / 2.f,
                            boardDrawOffset + scale * BOARD_OFFSET_Y + move.getTo().row * scaledCellSize + scaledCellSize / 2.f
                        ));
                        window.draw(possibleMoveHighlight);
                    }
                }
            }

            for (int row = 0; row < BOARD_SIZE; ++row) {
                for (int col = 0; col < BOARD_SIZE; ++col) {
                    Tile& tile = board.getTile(row, col);
                    if (tile.hasPiece()) {
                        Piececolor color = tile.getPiece()->getColor();
                        Piecetype type = tile.getPiece()->getType();

                        if (color == Piececolor::White) {
                            checkerSprite.setTextureRect((type == Piecetype::King) ? whiteKingRect : whiteRect);
                        } else {
                            checkerSprite.setTextureRect((type == Piecetype::King) ? blackKingRect : blackRect);
                        }

                        checkerSprite.setScale(sf::Vector2f(scale * 0.8f, scale * 0.8f));
                        checkerSprite.setPosition(sf::Vector2f(
                            boardDrawOffset + scale * (BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2.f),
                            boardDrawOffset + scale * (BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2.f)
                        ));
                        window.draw(checkerSprite);
                    }
                }
            }

            if (sf::Joystick::isConnected(0)) {
                sf::RectangleShape joyCursor;
                joyCursor.setSize(sf::Vector2f(scaledCellSize, scaledCellSize));
                joyCursor.setFillColor(sf::Color::Transparent);
                joyCursor.setOutlineColor(sf::Color(255, 255, 128));
                joyCursor.setOutlineThickness(3.0f);
                joyCursor.setPosition(sf::Vector2f(
                    boardDrawOffset + scale * BOARD_OFFSET_X + joySelectedCell.x * scaledCellSize,
                    boardDrawOffset + scale * BOARD_OFFSET_Y + joySelectedCell.y * scaledCellSize
                ));
                window.draw(joyCursor);
            }
            
            if (gameOver) {
                sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
                overlay.setFillColor(sf::Color(0, 89, 96, 112));
                window.draw(overlay);

                sf::Font font;
                if (!font.openFromFile("visitor2.ttf")) {
                    std::cout << "Error loading font\n";
                }

                sf::Text text(font, gameOverText + "\nGame Over", 48);
                text.setFillColor(sf::Color(221, 227, 206));
                text.setStyle(sf::Text::Bold);

                auto textBounds = text.getLocalBounds();
                text.setOrigin(sf::Vector2f(
                    textBounds.position.x + textBounds.size.x / 2,
                    textBounds.position.y + textBounds.size.y / 2
                ));
                text.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 - 60));
                window.draw(text);

                sf::Text menuText(font, "Return to menu", 24);
                menuText.setFillColor(sf::Color(234, 240, 216));
                auto menuBounds = menuText.getLocalBounds();
                menuText.setOrigin(sf::Vector2f(
                    menuBounds.position.x + menuBounds.size.x / 2,
                    menuBounds.position.y + menuBounds.size.y / 2
                ));
                menuText.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 + 60));
                window.draw(menuText);
            }

            if (checkerAnim.active) {
                checkerAnim.progress = animClock.getElapsedTime().asSeconds() / checkerAnim.duration;
                if (checkerAnim.progress >= 1.f) {
                    checkerAnim.active = false;
                } else {
                    float x = checkerAnim.from.x + (checkerAnim.to.x - checkerAnim.from.x) * checkerAnim.progress;
                    float y = checkerAnim.from.y + (checkerAnim.to.y - checkerAnim.from.y) * checkerAnim.progress;
                    checkerSprite.setTextureRect((checkerAnim.type == Piecetype::King)
                        ? (checkerAnim.color == Piececolor::White ? whiteKingRect : blackKingRect)
                        : (checkerAnim.color == Piececolor::White ? whiteRect : blackRect)
                    );
                    checkerSprite.setScale(sf::Vector2f(scale * 0.8f, scale * 0.8f));
                    checkerSprite.setPosition(sf::Vector2f(
                        boardDrawOffset + scale * (BOARD_OFFSET_X + x * CELL_SIZE + CELL_SIZE / 2.f),
                        boardDrawOffset + scale * (BOARD_OFFSET_Y + y * CELL_SIZE + CELL_SIZE / 2.f)
                    ));
                    window.draw(checkerSprite);
                }
            }
        }

        if (screenState == ScreenState::GameOver) {
            sf::Font font;
            if (!font.openFromFile("visitor2.ttf")) {
                std::cout << "Error loading font\n";
            }

            // sf::RectangleShape overlay(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
            // overlay.setFillColor(sf::Color(0, 89, 96, 112));
            // window.draw(overlay);
            window.clear(sf::Color(89, 96, 112));
            sf::Text text(font, gameOverText + "\nGame Over", 48);
            text.setFillColor(sf::Color(234, 240, 216));
            //text.setStyle(sf::Text::Bold);

            auto textBounds = text.getLocalBounds();
            text.setOrigin(sf::Vector2f(
                textBounds.position.x + textBounds.size.x / 2,
                textBounds.position.y + textBounds.size.y / 2
            ));
            text.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 - 60));
            window.draw(text);

            // przycisk return to menu
            sf::RectangleShape menuButton(sf::Vector2f(300, 50));
            menuButton.setFillColor(sf::Color(165, 170, 153));
            menuButton.setOrigin(sf::Vector2f(menuButton.getSize().x / 2, menuButton.getSize().y / 2));
            menuButton.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 + 60));
            window.draw(menuButton);

            sf::Text menuText(font, "Return to menu", 24);
            menuText.setFillColor(sf::Color(234, 240, 216));
            auto menuBounds = menuText.getLocalBounds();
            menuText.setOrigin(sf::Vector2f(
                menuBounds.position.x + menuBounds.size.x / 2,
                menuBounds.position.y + menuBounds.size.y / 2
            ));
            menuText.setPosition(sf::Vector2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 + 60));
            window.draw(menuText);
        }

        if (screenState == ScreenState::Options) {
            window.clear(sf::Color(89, 96, 112));
            sf::Font font;
            if (!font.openFromFile("visitor2.ttf")) {
                std::cout << "Error loading font\n";
            }

            sf::Text title(font, "Options", 48);
            title.setFillColor(sf::Color(221, 227, 206));
            auto titleBounds = title.getLocalBounds();
            title.setOrigin(sf::Vector2f(titleBounds.position.x + titleBounds.size.x / 2, titleBounds.position.y + titleBounds.size.y / 2));
            title.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 100));
            window.draw(title);

            sf::Text easy(font, "Easy AI", 32);
            if (aiDepth == 3) easy.setFillColor(sf::Color(255, 255, 128)); 
            else easy.setFillColor(sf::Color(234, 240, 216));
            easyBounds = easy.getLocalBounds();
            easy.setOrigin(sf::Vector2f(easyBounds.position.x + easyBounds.size.x / 2, easyBounds.position.y + easyBounds.size.y / 2));
            easy.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 200));
            window.draw(easy);

            sf::Text hard(font, "Hard AI", 32);
            if (aiDepth == 9) hard.setFillColor(sf::Color(255, 255, 128));
            else hard.setFillColor(sf::Color(234, 240, 216));
            hardBounds = hard.getLocalBounds();
            hard.setOrigin(sf::Vector2f(hardBounds.position.x + hardBounds.size.x / 2, hardBounds.position.y + hardBounds.size.y / 2));
            hard.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 250));
            window.draw(hard);

            sf::Text pvp(font, "Player vs Player", 32);
            if (gameMode == 1) pvp.setFillColor(sf::Color(255, 255, 128));
            else pvp.setFillColor(sf::Color(234, 240, 216));
            pvpBounds = pvp.getLocalBounds();
            pvp.setOrigin(sf::Vector2f(pvpBounds.position.x + pvpBounds.size.x / 2, pvpBounds.position.y + pvpBounds.size.y / 2));
            pvp.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 300));
            window.draw(pvp);

            sf::Text pve(font, "Player vs Computer", 32);
            if (gameMode == 2) pve.setFillColor(sf::Color(255, 255, 128));
            else pve.setFillColor(sf::Color(234, 240, 216));
            pveBounds = pve.getLocalBounds();
            pve.setOrigin(sf::Vector2f(pveBounds.position.x + pveBounds.size.x / 2, pveBounds.position.y + pveBounds.size.y / 2));
            pve.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 350));
            window.draw(pve);

            sf::Text back(font, "Back", 28);
            backBounds = back.getLocalBounds();
            back.setOrigin(sf::Vector2f(backBounds.position.x + backBounds.size.x / 2, backBounds.position.y + backBounds.size.y / 2));
            back.setPosition(sf::Vector2f(WINDOW_SIZE / 2, 420));
            window.draw(back);
        }

        
        window.display();
    }
    return 0;
}