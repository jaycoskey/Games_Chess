// Copyright 2021, by Jay M. Coskey

#pragma once

#include <vector>

#include "util.h"
#include "piece.h"


// Types of moves/move steps/actions in chess:
//   - Move primary piece
//   - Move secondary piece (when castling)
//   - Remove captured piece from board (destination of move, except in en passant)
//   - Exchange piece with another (when promoting a Pawn)

// TODO: Expand to include MoveSteps, as described below
// TODO: Privacy
class Move {
public:
    Move(Color color, PieceType type, Pos from, Pos to) : color{color}, type{type}, from{from}, to{to} {}
    Color color;
    PieceType type;
    Pos from;
    Pos to;
};

std::vector<Move> steps(const Board& board, Color color, PieceType type, Pos pos, std::set<Dir> dirs) {
     std::vector<Move> result{};
     for (Dir dir : dirs) {
         Pos dest = pos + dir;
         if (board.containsPos(dest)) {
             result.emplace(result.end(), color, type, pos, pos+dir);
         }
     }
     return result;
}
