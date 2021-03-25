// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>

#include "util.h"
#include "geometry.h"
// #include "player.h"

using std::string;
using std::map, std::vector;


enum class PieceType {
    None,
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};
const vector<PieceType> pieceTypes{
    PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };

const string& showPieceType(PieceType pieceType) {
    static const map<PieceType, string> pieceType2cstr {
        { PieceType::King,   string{"K"} },
        { PieceType::Queen,  string{"Q"} },
        { PieceType::Rook,   string{"R"} },
        { PieceType::Bishop, string{"B"} },
        { PieceType::Knight, string{"N"} },
        { PieceType::Pawn,   string{"P"} },
        { PieceType::None,   string{" "} }
    };
    return pieceType2cstr.find(pieceType)->second;
}

class Board;

class Piece {
public:
    Piece(Color color, PieceType pieceType, Short index)
        : _color{color}, _pieceType{pieceType}, _pos{index},
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

    void moveTo(const Pos& pos)
    {
        _pos.setPos(pos);
    }
    void setPieceType(PieceType pieceType) { _pieceType = pieceType; }

    bool operator<(const Piece& other) { return _pos < other.pos(); }

    friend ostream& operator<<(ostream& os, const Piece& piece);

private:
    Color     _color;
    PieceType _pieceType;

    Pos       _pos;
    Short     _lastMoveIndex;  // Convention: Zero before first move

    friend struct std::hash<Board>;
};

ostream& operator<<(ostream& os, const Piece& piece) {
    os << showColor(piece._color) << showPieceType(piece._pieceType)
       << "_@_" << piece._pos << "=index#" << piece.index();
    return os;
}
