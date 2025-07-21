#ifndef MOVE_H
#define MOVE_H

#include <vector>

// Position - zawiera współrzędne (row, col)

struct Position
{
    int row;
    int col;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
};

/* 
odpowiada za jeden ruch
co wie: skąd i dokąd; które pionki zostały zbite 
co umie: zwraca czy było bicie 
*/
class Move
{
private:
    Position from;
    Position to;
    std::vector<Position> captured; // współrzędne zbitych pionków

public:
    Move(Position from, Position to);
    Position getFrom() const;
    Position getTo() const;
    const std::vector<Position>& getCaptured() const;
    void addCaptured(Position pos); // dla > 1 bicie
    bool isCapture() const;
};

#endif