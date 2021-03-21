// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>

#include "util.h"
#include "geometry.h"
// #include "player.h"

using std::string;
using std::map;


enum class PieceType {
    NoType,
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

const string& showPieceType(PieceType pieceType) {
    static const map<PieceType, string> pieceType2cstr {
        { PieceType::King,    string{"K"} },
        { PieceType::Queen,   string{"Q"} },
        { PieceType::Rook,    string{"R"} },
        { PieceType::Bishop,  string{"B"} },
        { PieceType::Knight,  string{"N"} },
        { PieceType::Pawn,    string{"P"} },
        { PieceType::NoType,  string{" "} }
    };
    return pieceType2cstr.find(pieceType)->second;
}

class Board;

class Piece {
public:
    Piece(Color color, PieceType pieceType, Short index)
        : _color{color}, _pieceType{pieceType}, _index{index},
          _pos{index},
          _lastMoveIndex{0}
        {}

    Color color() const { return _color; }
    bool  isBlack() const { return _color == Color::Black; }
    bool  isWhite() const { return _color == Color::White; }

    PieceType pieceType() const { return _pieceType; }

    Pos   pos() const { return _pos; }
    Col   col() const { return _pos.x; }
    Row   row() const { return _pos.y; }
    Short index() const { return _index; }

    Col   relCol()  const { return isWhite() ? _pos.x : BOARD_COLS - 1 - _pos.x; }
    Col   relRow()  const { return isWhite() ? _pos.y : BOARD_ROWS - 1 - _pos.y; }

    void  setPos(Pos pos) { _pos = pos; }

    bool operator<(const Piece& other) { return _pos < other.pos(); }

    friend ostream& operator<<(ostream& os, const Piece& piece);

private:
    Color     _color;
    PieceType _pieceType;
    Short     _index;

    Pos       _pos;
    Short     _lastMoveIndex;  // Convention: Zero before first move

    friend struct std::hash<Board>;
};

ostream& operator<<(ostream& os, const Piece& piece) {
    os << "Piece{"
       << showColor(piece._color) << showPieceType(piece._pieceType)
       << '@' << piece._pos << "=#" << piece._index
       << "}\n";
    return os;
}
