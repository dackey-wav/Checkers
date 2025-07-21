#include "C:\Visual Studio\PiAA_proj3\include\Piece.hpp"

Piece::Piece(Piececolor color, Piecetype type) : color(color), type(type) {}

bool Piece::isKing() const {
    return type == Piecetype::King;
}

void Piece::makeKing() {
    type = Piecetype::King;
}

Piecetype Piece::getType() const {
    return type;
}

Piececolor Piece::getColor() const {
    return color;
}