// Games_Chess
// Copyright (C) 2021, by Jay M. Coskey
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "util.h"
#include "geometry.h"

#include "logger.h"

using MoveIndex         = Short;
using MoveIndexHistory  = std::vector<bool>;
using PieceValue        = float;


constexpr PieceValue KING_VALUE = 1'000.0;

enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

constexpr Short PIECE_TYPES_COUNT = 6;

std::ostream& operator<<(std::ostream& os, PieceType pt);

using OptPieceType = std::optional<PieceType>;

const std::vector<PieceType> pieceTypes {
    PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };

// ========================================
// Piece

// class Board;

class Piece
{
public:
    // Static method
    static PieceValue pieceValue(PieceType pt);

    // Constructor
    Piece(Color color, PieceType pt, Short index, MoveIndex lastMoveIndex=0);

    // Public read methods
    Color color() const { return _color; }
    bool  isBlack() const { return _color == Color::Black; }
    bool  isWhite() const { return _color == Color::White; }

    PieceType pieceType() const { return _pieceType; }

    Col   col() const { return _pos.x; }
    Row   row() const { return _pos.y; }
    Pos   pos() const { return _pos; }
    Color squareColor() const { return _pos.squareColor(); }

    bool hasMoved() const { return lastMoveIndex() > 0; }
    MoveIndex lastMoveIndex() const;

    // Public write methods
    void moveTo(const Pos& pos) { _pos.moveTo(pos); }
    void rollBackLastMoveIndex(MoveIndex mi);
    void setPieceType(PieceType pt) { _pieceType = pt; }
    void updateMoveIndexHistory(MoveIndex mi);

    // Operator
    bool operator<(const Piece& other) { return _pos < other.pos(); }

private:
    Color     _color;
    PieceType _pieceType;
    Pos       _pos;

    // First move is at ...[1]. The value at ...[0] acts as a "reverse sentinel".
    MoveIndexHistory _moveIndexHistory;

    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
};
