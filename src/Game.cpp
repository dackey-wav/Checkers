#include "..\include\Game.hpp"

void Game::start() {
    board.initialize();
    currentPlayer = Piececolor::White;
    gameOver = false;
}

void Game::nextTurn() {
    std::vector<Move> moves = board.getAllValidMoves(currentPlayer);
    if (moves.empty()) {
        gameOver = true;
        // Zwycięzca — przeciwny gracz
        return;
    } else {
        const Move& chosenMove = moves[0];
        board.applyMove(chosenMove);

        if (chosenMove.isCapture()) {
            std::vector<Move> multiCaptures;
            Move baseMove(chosenMove.getTo(), chosenMove.getTo());
            board.findMultiCaptures(
                chosenMove.getTo(),
                {},
                multiCaptures,
                currentPlayer,
                board.getTile(chosenMove.getTo().row, chosenMove.getTo().col).getPiece()->getType(),
                baseMove
            );
            // jeśli są kolejne bicie — kontynuować tą samą figurą
            if (!multiCaptures.empty()) {
                return;
            }
        }
        // jeśli nie było wielokrotnego bicia — zmieniamy gracza
        currentPlayer = (currentPlayer == Piececolor::White) ? Piececolor::Black : Piececolor::White;
    }
}

bool Game::isGameOver() {
    return gameOver;
}

std::optional<Piececolor> Game::getWinner() {
    if (!gameOver) return std::nullopt;
    // Zwycięzca — ten, kto ma jeszcze ruchy
    return (currentPlayer == Piececolor::White) ? Piececolor::Black : Piececolor::White;
}