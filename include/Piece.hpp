#ifndef PIECE_H
#define PIECE_H

enum class Piecetype {Man, King};
enum class Piececolor {White, Black};

/*
Piece - jeden pion
co wie: kolor i typ
co umie: stać się damką; powiedzieć czy jest damką czy nie
*/
class Piece
{
private:
    Piecetype type;
    Piececolor color;

public:
    Piece(Piececolor color, Piecetype type = Piecetype::Man);
    bool isKing() const;
    void makeKing();
    Piecetype getType() const;
    Piececolor getColor() const;
};

#endif