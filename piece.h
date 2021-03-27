// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>

#include "util.h"
#include "geometry.h"
// #include "player.h"

using std::string;
using std::ostream;
using std::map, std::vector;

using PieceValue = float;

enum class PieceType {
    None,
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};
ostream& operator<<(ostream& os, PieceType pt)
{
    if (pt == PieceType::King)        { os << 'K'; }
    else if (pt == PieceType::Queen)  { os << 'Q'; }
    else if (pt == PieceType::Rook)   { os << 'R'; }
    else if (pt == PieceType::Bishop) { os << 'B'; }
    else if (pt == PieceType::Knight) { os << 'N'; }
    else if (pt == PieceType::Pawn)   { os << 'P'; }
    return os;
}

const vector<PieceType> pieceTypes{
    PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };

class Board;

class Piece {
public:
    static PieceValue pieceValue(PieceType pt) { return _pieceType2PieceValue.at(pt); }

    Piece(Color color, PieceType pt, Short index)
        : _color{color}, _pieceType{pt}, _pos{index},
          _lastMoveIndex{0}
        {}

    Color color() const { return _color; }
    bool  isBlack() const { return _color == Color::Black; }
    bool  isWhite() const { return _color == Color::White; }

    PieceType pieceType() const { return _pieceType; }

    Pos   pos() const { return _pos; }
    Col   col() const { return _pos.x; }
    Row   row() const { return _pos.y; }
    Short index() const { return _pos.x + _pos.y * BOARD_COLS; }
    Short lastMoveIndex() const { return _lastMoveIndex; }

    void moveTo(const Pos& pos) {
        _pos.moveTo(pos);
    }
    void setPieceType(PieceType pt) { _pieceType = pt; }

    bool operator<(const Piece& other) { return _pos < other.pos(); }

    friend ostream& operator<<(ostream& os, const Piece& piece);

private:
    static map<PieceType, PieceValue> _pieceType2PieceValue;

    Color     _color;
    PieceType _pieceType;
    Pos       _pos;
    Short     _lastMoveIndex;  // Convention: Zero before first move

    friend struct std::hash<Board>;
};

ostream& operator<<(ostream& os, const Piece& piece) {
    os << piece._color << piece._pieceType
       << "_@_" << piece._pos << "=index#" << piece.index();
    return os;
}

map<PieceType, PieceValue> Piece::_pieceType2PieceValue{
    {PieceType::King, 1'000'000.0}
    , {PieceType::Queen,  9.0}
    , {PieceType::Rook,   5.0}
    , {PieceType::Bishop, 3.5}
    , {PieceType::Knight, 3.0}
    , {PieceType::Pawn,   1.0}
    };
