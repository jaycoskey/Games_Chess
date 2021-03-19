// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"


enum PieceType {
    NoPieceType,
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

class Board;

// TODO: Determine which location (pos or index) to treat as primary.
//       Initially, provide both as read functions.
class Piece {
public:
    Piece(Color color, PieceType type, Short index)
        : _color{color}, _type{type}, _index{index},
          _pos{index},
          _lastMoveIndex{0}
        {}

    Color color() const { return _color; }
    bool  isBlack() const { return _color == Black; }
    bool  isWhite() const { return _color == White; }

    PieceType type() const { return _type; }

    Short index() const { return _index; }
    Pos   pos() const { return _pos; }
    Col   col() const { return _pos.x; }
    Row   row() const { return _pos.y; }
    Col   relCol()  const { return isWhite() ? _pos.x : BOARD_COLS - 1 - _pos.x; }
    Col   relRow()  const { return isWhite() ? _pos.y : BOARD_ROWS - 1 - _pos.y; }


private:
    Color     _color;
    PieceType _type;
    Short     _index;

    Pos       _pos;
    Short     _lastMoveIndex;  // Convention: Zero before first move

    friend struct std::hash<Board>;
};
