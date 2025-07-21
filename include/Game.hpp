#ifndef GAME_H
#define GAME_H

#include "Board.hpp"

class Game {
private:
    Board board;
    Piececolor currentPlayer;
    bool gameOver;

public:
    void start();
    void nextTurn();
    bool isGameOver();
    std::optional<Piececolor> getWinner();
};

#endif