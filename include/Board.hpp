#ifndef BOARD_H
#define BOARD_H

#include "Tile.hpp"
#include "Move.hpp"

/*
Board - plansza
co wie: 64 pola (2D tablica Tile)
co umie:
    * inicjalizuje lokalizacje początkowę
    * zwraca komórkę według współrzędnych
    * zastosować ruch
    * zwraca wszystkie możliwe ruchy dla gracza
    * sprawdza czy ruch jest wykonalny
*/
class Board {
private:
    static const int SIZE = 8;
    Tile tiles[SIZE][SIZE];
    Piececolor currentPlayer = Piececolor::White; // Начинаем с белых

public:
    Board();
    ~Board();
    void initialize();
    Tile& getTile(int row, int col);
    const Tile& getTile(int row, int col) const;
    bool isValidMove(const Move& move, Piececolor playerColor) const;
    std::vector<Move> getAllValidMoves(Piececolor playerColor) const;
    void applyMove(const Move& move);
    void findMultiCaptures(Position from,
                               std::vector<Position> captured,
                               std::vector<Move>& result,
                               Piececolor color,
                               Piecetype type,
                               const Move& currentMove) const;
    Piececolor getCurrentPlayer() const { return currentPlayer; }
    //bool isInsideBoard(int row, int col) const;
    //std::vector<Move> getAllPossibleMoves(Piececolor playerColor) const;
};

#endif