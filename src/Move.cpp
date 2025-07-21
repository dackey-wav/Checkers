#include "C:\Visual Studio\PiAA_proj3\include\Move.hpp"

Move::Move(Position from, Position to) : from(from), to(to) {}

Position Move::getFrom() const {
    return from;
}

Position Move::getTo() const {
    return to;
}

void Move::addCaptured(Position pos) {
    captured.push_back(pos);
}

const std::vector<Position>& Move::getCaptured() const {
    return captured;
}

bool Move::isCapture() const {
    return !captured.empty();
}