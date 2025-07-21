#include "C:\Visual Studio\PiAA_proj3\include\Tile.hpp"

Tile::Tile() : row(0), col(0), highlighted(false), piece(std::nullopt) {}

Tile::Tile(int row, int col)
    : row(row), col(col), highlighted(false), piece(std::nullopt) {}


int Tile::getRow() const {return row;}
int Tile::getCol() const {return col;}

bool Tile::isHighlighted() const {return highlighted;}
void Tile::setHighlighted(bool value) {highlighted = value;}

bool Tile::hasPiece() const {return piece.has_value();}

Piece* Tile::getPiece() {
    return piece.has_value() ? &piece.value() : nullptr;
}

const Piece* Tile::getPiece() const {
    return piece.has_value() ? &piece.value() : nullptr;
}

void Tile::setPiece(const Piece& p) {piece = p;}

void Tile::removePiece() {piece.reset();}