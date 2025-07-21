#ifndef TILE_H
#define TILE_H

#include "Piece.hpp"
#include <optional>

/*
Tile - jedno pole na planszy
co wie: współrzędne; czy jest na nim pion; higlighted - podświetlenie
co umie: umieścić/usunąć pion; zwraca pion
*/
class Tile {
private:
    int row;
    int col;
    std::optional<Piece> piece;
    bool highlighted;

public:
    Tile();
    Tile(int row, int col);

    int getRow() const;
    int getCol() const;
    bool isHighlighted() const;
    void setHighlighted(bool value);
    bool hasPiece() const;
    Piece* getPiece();
    const Piece* getPiece() const;
    void setPiece(const Piece& p);
    void removePiece();
};

#endif