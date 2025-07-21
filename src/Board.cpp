#include "../include/Board.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

Board::Board() {}
Board::~Board() {}

void Board::initialize() {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            tiles[row][col].removePiece();
        }
    }

    
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if ((row + col) % 2 == 1) {
                tiles[row][col].setPiece(Piececolor::Black);
            }
        }
    }
    for (int row = SIZE - 3; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if ((row + col) % 2 == 1) {
                tiles[row][col].setPiece(Piececolor::White);
            }
        }
    }
}

Tile& Board::getTile(int row, int col) {
    return tiles[row][col];
}

const Tile& Board::getTile(int row, int col) const {
    return tiles[row][col];
}


bool Board::isValidMove(const Move& move, Piececolor playerColor) const {
    Position from = move.getFrom();
    Position to = move.getTo();

    //std::cout << "isValidMove: from (" << from.row << ", " << from.col << ") to (" << to.row << ", " << to.col << ")" << std::endl;

    if (from.row < 0 || from.row >= SIZE || from.col < 0 || from.col >= SIZE ||
        to.row < 0 || to.row >= SIZE || to.col < 0 || to.col >= SIZE) {
      //  std::cout << "isValidMove: out of bounds" << std::endl;
        return false;
    }

    const Tile& fromTile = getTile(from.row, from.col);
    const Tile& toTile = getTile(to.row, to.col);

    if (!(fromTile.hasPiece()) || toTile.hasPiece()) {
        //std::cout << "isValidMove: no piece or target has piece" << std::endl;
        return false;
    }

    const Piece* piece = fromTile.getPiece();
    if (piece->getColor() != playerColor) {
        //std::cout << "isValidMove: wrong color" << std::endl;
        return false;
    }

    int rowDiff = to.row - from.row;
    int colDiff = to.col - from.col;

    if (piece->getType() == Piecetype::Man) {
        int dir = (playerColor == Piececolor::White) ? -1 : 1;
        if (move.getCaptured().empty()) {
            // zwykły ruch
            if (rowDiff == dir && std::abs(colDiff) == 1) {
          //      std::cout << "isValidMove: normal move" << std::endl;
                return true;
            }
        } else {
            // bicie
            if (rowDiff == 2 * dir && std::abs(colDiff) == 2) {
                int midRow = from.row + dir;
                int midCol = (from.col + to.col) / 2;
                const Tile& midTile = getTile(midRow, midCol);
                if (midTile.hasPiece() && midTile.getPiece()->getColor() != playerColor) {
            //        std::cout << "isValidMove: capture" << std::endl;
                    return true;
                }
            }
        }
    } else if (piece->getType() == Piecetype::King) {
        int rowStep = (rowDiff > 0) ? 1 : -1;
        int colStep = (colDiff > 0) ? 1 : -1;

        if (std::abs(rowDiff) != std::abs(colDiff)) return false;

        int enemyCount = 0;
        // przejście po przekątnej
        int r = from.row + rowStep; // następny wiersz po from[row, ...]
        int c = from.col + colStep; // następna kolumna po from[..., col]

        while (r != to.row && c != to.col) {
            const Tile& t = getTile(r, c);
            if (t.hasPiece()){
                const Piece* midPiece = t.getPiece();
                if (midPiece->getColor() == playerColor) return false;
                enemyCount++;
                if (enemyCount > 1) return false;
            }
            r += rowStep;
            c += colStep;
        }

        if (move.getCaptured().empty()) {
            return enemyCount == 0;
        } else {
            return enemyCount == 1;
        }
    }
    //std::cout << "isValidMove: false" << std::endl;
    return false;
}

std::vector<Move> Board::getAllValidMoves(Piececolor playercolor) const {
    std::vector<Move> captureMoves;
    std::vector<Move> normalMoves;
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            const Tile& tile = getTile(row, col);
            if (!tile.hasPiece()) continue;

            const Piece* piece = tile.getPiece();
            if (piece->getColor() != playercolor) continue;

            Position from = {row, col};

            
            std::vector<Move> multiCaptures;
            Move baseMove(from, from);
            findMultiCaptures(from, {}, multiCaptures, playercolor, piece->getType(), baseMove);

            captureMoves.insert(captureMoves.end(), multiCaptures.begin(), multiCaptures.end());

            if (piece->getType() == Piecetype::Man) {
                int dir = (playercolor == Piececolor::White) ? -1 : 1;
                for (int dc : {-1, 1}) {
                    Position to = {row + dir, col + dc};
                    Move m(from, to);
                    if (isValidMove(m, playercolor)) normalMoves.push_back(m);
                }
            } else if (piece->getType() == Piecetype::King) {
                for (auto [dr, dc] : directions) {
                    for (int step = 1; step < 8; ++step) {
                        int newRow = row + step * dr;
                        int newCol = col + step * dc;
                        if (newRow < 0 || newRow >= SIZE || newCol < 0 || newCol >= SIZE) break;
                        Position to = {newRow, newCol};
                        Move m(from, to);
                        if (isValidMove(m, playercolor)) normalMoves.push_back(m);
                        if (getTile(newRow, newCol).hasPiece()) break;
                    }
                }
            }
        }
    }

    return !captureMoves.empty() ? captureMoves : normalMoves;
}

void Board::findMultiCaptures(Position from,
                               std::vector<Position> captured,
                               std::vector<Move>& result,
                               Piececolor color,
                               Piecetype type,
                               const Move& currentMove) const {
    bool foundExtension = false;

    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    for (auto [dr, dc] : directions) {
        if (type == Piecetype::Man) {
            int midRow = from.row + dr;
            int midCol = from.col + dc;
            int destRow = from.row + 2 * dr;
            int destCol = from.col + 2 * dc;

            if (destRow < 0 || destRow >= SIZE || destCol < 0 || destCol >= SIZE) continue;

            Position mid = {midRow, midCol};
            Position dest = {destRow, destCol};

            if (std::find(captured.begin(), captured.end(), mid) != captured.end()) continue;

            Move candidate(from, dest);
            candidate.addCaptured(mid);

            if (isValidMove(candidate, color)) {
                Move newMove(currentMove.getFrom(), dest);
                for (const Position& cap : currentMove.getCaptured()) {
                    newMove.addCaptured(cap);
                }
                newMove.addCaptured(mid);

                std::vector<Position> newCaptured = captured;
                newCaptured.push_back(mid);

                findMultiCaptures(dest, newCaptured, result, color, type, newMove);
                foundExtension = true;
            }
        }
        else if (type == Piecetype::King) {
            for (int step = 1; step < SIZE; ++step) {
                int midRow = from.row + dr * step;
                int midCol = from.col + dc * step;
                if (midRow < 0 || midRow >= SIZE || midCol < 0 || midCol >= SIZE) break;

                const Tile& midTile = getTile(midRow, midCol);
                if (!midTile.hasPiece()) continue;

                const Piece* p = midTile.getPiece();
                if (p->getColor() == color) break;

                Position mid = {midRow, midCol};
                if (std::find(captured.begin(), captured.end(), mid) != captured.end()) break;

                int nextRow = midRow + dr;
                int nextCol = midCol + dc;

                while (nextRow >= 0 && nextRow < SIZE &&
                       nextCol >= 0 && nextCol < SIZE &&
                       !getTile(nextRow, nextCol).hasPiece()) {
                    Position dest = {nextRow, nextCol};
                    Move candidate(from, dest);
                    candidate.addCaptured(mid);

                    if (isValidMove(candidate, color)) {
                        Move newMove(currentMove.getFrom(), dest);
                        for (const Position& cap : currentMove.getCaptured()) {
                            newMove.addCaptured(cap);
                        }
                        newMove.addCaptured(mid);

                        std::vector<Position> newCaptured = captured;
                        newCaptured.push_back(mid);

                        findMultiCaptures(dest, newCaptured, result, color, type, newMove);
                        foundExtension = true;
                    }

                    nextRow += dr;
                    nextCol += dc;
                }
                break;
            }
        }
    }

    if (!foundExtension && !currentMove.getCaptured().empty()) {
        result.push_back(currentMove);
    }
}


/*bool Board::isInsideBoard(int row, int col) const {
    return row >= 0 && row < SIZE && col >= 0 && col < SIZE;
}*/


void Board::applyMove(const Move& move) {
    Tile& fromTile = tiles[move.getFrom().row][move.getFrom().col];
    Tile& toTile = tiles[move.getTo().row][move.getTo().col];

    if (fromTile.hasPiece()) {
        toTile.setPiece(*(fromTile.getPiece()));
        fromTile.removePiece();
    }

    for (const Position& capturedPos : move.getCaptured()) {
        tiles[capturedPos.row][capturedPos.col].removePiece();
    }

    Piece& movedPiece = *(toTile.getPiece());
    if ((movedPiece.getColor() == Piececolor::White && move.getTo().row == 0) ||
        (movedPiece.getColor() == Piececolor::Black && move.getTo().row == 7)) {
        movedPiece.makeKing();
    }

    // zmieniamy gracza
    currentPlayer = (currentPlayer == Piececolor::White) ? Piececolor::Black : Piececolor::White;
}