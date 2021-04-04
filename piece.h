// Copyright 2021, by Jay M. Coskey

#pragma once

#include <algorithm>
#include <map>

#include "util.h"
#include "geometry.h"
// #include "player.h"

#include "logger.h"

using std::ostream;
using std::map, std::vector;

using MoveIndex         = Short;
using MoveIndexHistory  = vector<bool>;
using PieceValue        = float;

const PieceValue KING_VALUE = 1'000.0;

// ========================================
enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

vector<PieceType> allPieceTypes
    { PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };
const Short PIECE_TYPES_COUNT = 6;

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

using OptPieceType = std::optional<PieceType>;

const vector<PieceType> pieceTypes {
    PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };

// ========================================
// Piece

class Board;

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

    Short index() const { return _pos.x + _pos.y * BOARD_COLS; }
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
    MoveIndexHistory _moveIndexHistory;  // First move=1. Never moved=0.

    friend ostream& operator<<(ostream& os, const Piece& piece);
};

// ---------- Static public method
PieceValue Piece::pieceValue(PieceType pt)
{
    static map<PieceType, PieceValue> pt2pv{
        {PieceType::King,     KING_VALUE}
        , {PieceType::Queen,  9.0}
        , {PieceType::Rook,   5.0}
        , {PieceType::Bishop, 3.5}
        , {PieceType::Knight, 3.0}
        , {PieceType::Pawn,   1.0}
    };
    if (  pt != PieceType::King   && pt != PieceType::Queen
       && pt != PieceType::Rook   && pt != PieceType::Bishop
       && pt != PieceType::Knight && pt != PieceType::Pawn
       )
    {
        logger.error("Piece::pieceValue: Called with unrecognized PieceType.");
        return 0.0;  // TODO: Diagnose & remove workaround.
    }
    return pt2pv.at(pt);
}

// ---------- Constructors
Piece::Piece(Color color, PieceType pt, Short index, MoveIndex lastMoveIndex /* =0 */)
    : _color{color}, _pieceType{pt}, _pos{index},
      _moveIndexHistory{}
{
    _moveIndexHistory.reserve(lastMoveIndex + VECTOR_CAPACITY_INCR);
    assert(_moveIndexHistory.capacity() - 1 >= (unsigned long) lastMoveIndex);
    if (lastMoveIndex == 0) {
        _moveIndexHistory.push_back(true);
    } else {
        _moveIndexHistory[lastMoveIndex] = true;
    }
}

// ---------- Public read methods
MoveIndex Piece::lastMoveIndex() const
{
    for (int k = _moveIndexHistory.size(); k >= 0; --k) {
        if (_moveIndexHistory[k]) { return k; }
    }
    return 0;
}

// ---------- Public write methods
void Piece::rollBackLastMoveIndex(MoveIndex mi)
{
    const MoveIndexHistory::iterator& beg = _moveIndexHistory.begin() + mi;
    const MoveIndexHistory::iterator& end = _moveIndexHistory.end();
    _moveIndexHistory.erase(beg, end);
}

void Piece::updateMoveIndexHistory(MoveIndex mi)
{
    if ((_moveIndexHistory.capacity()) <= (unsigned long) mi + 1)
    {
        _moveIndexHistory.reserve(mi + VECTOR_CAPACITY_INCR);
    }
    assert(_moveIndexHistory.capacity() - 1 >= (unsigned long) mi);
    _moveIndexHistory[mi] = true;
}

// ---------- Operator
ostream& operator<<(ostream& os, const Piece& piece)
{
    os << piece._color << piece._pieceType
       << "_@_" << piece._pos << "=index#" << piece.index();
    return os;
}
