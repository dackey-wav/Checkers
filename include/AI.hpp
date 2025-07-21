#pragma once
#include "Board.hpp"
#include <iostream>
#include <algorithm>

// wagi
const int PIECE_VALUE = 100;
const int KING_VALUE = 300;
const int MOBILITY_WEIGHT = 5;
const int ADVANCEMENT_WEIGHT = 3;
const int CENTER_CONTROL_WEIGHT = 2;
const int EDGE_PENALTY = -10;
const int BACK_ROW_BONUS = 5;

// tabela wag (w środku większe)
const int POSITION_TABLE[8][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 2, 0, 2, 0, 2, 0},
    {0, 2, 0, 3, 0, 3, 0, 2},
    {1, 0, 3, 0, 4, 0, 3, 0},
    {0, 3, 0, 4, 0, 3, 0, 1},
    {2, 0, 3, 0, 3, 0, 2, 0},
    {0, 2, 0, 2, 0, 2, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0}
};

// Funkcja oceniająca aktualny stan planszy
inline int evaluateBoard(const Board& board) {
    int score = 0;
    int blackPieces = 0, whitePieces = 0;
    int blackKings = 0, whiteKings = 0;
    
    // Liczenie punktów za pionki, damki, pozycję itd.
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Tile& tile = board.getTile(row, col);
            if (tile.hasPiece()) {
                const Piece* piece = tile.getPiece();
                bool isBlack = (piece->getColor() == Piececolor::Black);
                bool isKing = (piece->getType() == Piecetype::King);
                
                int pieceValue = isKing ? KING_VALUE : PIECE_VALUE;
                
                // Premia za pozycję na planszy
                int positionBonus = POSITION_TABLE[row][col] * CENTER_CONTROL_WEIGHT;
                
                // Premia za przesunięcie pionka do przodu
                int advancementBonus = 0;
                if (!isKing) {
                    if (isBlack) {
                        advancementBonus = (7 - row) * ADVANCEMENT_WEIGHT;
                    } else {
                        advancementBonus = row * ADVANCEMENT_WEIGHT;
                    }
                }
                
                // Kara za pionki na krawędzi (jeśli nie są damkami)
                int edgePenalty = 0;
                if (!isKing && (col == 0 || col == 7)) {
                    edgePenalty = EDGE_PENALTY;
                }
                
                // Premia za ochronę ostatniego rzędu
                int backRowBonus = 0;
                if (!isKing) {
                    if ((isBlack && row == 7) || (!isBlack && row == 0)) {
                        backRowBonus = BACK_ROW_BONUS;
                    }
                }
                
                int totalValue = pieceValue + positionBonus + advancementBonus + edgePenalty + backRowBonus;
                
                if (isBlack) {
                    score += totalValue;
                    blackPieces++;
                    if (isKing) blackKings++;
                } else {
                    score -= totalValue;
                    whitePieces++;
                    if (isKing) whiteKings++;
                }
            }
        }
    }
    
    // Ocena mobilności (liczba możliwych ruchów)
    std::vector<Move> blackMoves = board.getAllValidMoves(Piececolor::Black);
    std::vector<Move> whiteMoves = board.getAllValidMoves(Piececolor::White);
    
    score += (blackMoves.size() - whiteMoves.size()) * MOBILITY_WEIGHT;
    
    // Premia za przewagę liczebną w końcówce
    int totalPieces = blackPieces + whitePieces;
    if (totalPieces <= 8) {
        int pieceDifference = blackPieces - whitePieces;
        score += pieceDifference * 50; // Większa waga pionków w końcówce
    }
    
    // Sprawdzenie zwycięstwa/przegranej
    if (blackPieces == 0) return -10000;
    if (whitePieces == 0) return 10000;
    if (blackMoves.empty()) return -10000;
    if (whiteMoves.empty()) return 10000;
    
    return score;
}

// Ulepszony minimax z alfa-beta pruning (opcjonalnie z tabelą transpozycji)
inline int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    Piececolor player = maximizingPlayer ? Piececolor::Black : Piececolor::White;
    std::vector<Move> moves = board.getAllValidMoves(player);

    // Koniec gry lub osiągnięta maksymalna głębokość
    if (depth == 0 || moves.empty()) {
        return evaluateBoard(board);
    }

    // Sortowanie ruchów dla lepszego cięcia alfa-beta
    // Można dodać prostą эвристику сортировки
    
    if (maximizingPlayer) {
        int maxEval = -100000;
        for (const auto& move : moves) {
            Board temp = board;
            temp.applyMove(move);
            int eval = minimax(temp, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; // a-b pruning
        }
        return maxEval;
    } else {
        int minEval = 100000;
        for (const auto& move : moves) {
            Board temp = board;
            temp.applyMove(move);
            int eval = minimax(temp, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break; 
        }
        return minEval;
    }
}

\
inline Move findBestMove(Board& board, int depth) {
    std::vector<Move> moves = board.getAllValidMoves(Piececolor::Black);
    if (moves.empty()) throw std::runtime_error("No moves for AI");

    int bestValue = -100000;
    Move bestMove = moves.front();
    
    for (const auto& move : moves) {
        Board temp = board;
        temp.applyMove(move);
        int value = minimax(temp, depth - 1, -100000, 100000, false);
        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
        }
    }
    return bestMove;
}