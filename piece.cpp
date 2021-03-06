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

#include <map>

#include "geometry.h"
#include "util.h"
// #include "player.h"
#include "piece.h"

#include "logger.h"

using std::map;
using std::ostream;

// ========================================
// PieceType

ostream &operator<<(ostream &os, PieceType pt) {
    if (pt == PieceType::King) {
        os << 'K';
    } else if (pt == PieceType::Queen) {
        os << 'Q';
    } else if (pt == PieceType::Rook) {
        os << 'R';
    } else if (pt == PieceType::Bishop) {
        os << 'B';
    } else if (pt == PieceType::Knight) {
        os << 'N';
    } else if (pt == PieceType::Pawn) {
        os << 'P';
    }
    return os;
}

// ---------- Static public method
PieceValue Piece::pieceValue(PieceType pt) {
    static map<PieceType, PieceValue> pt2pv{
        {PieceType::King, KING_VALUE}, {PieceType::Queen, 9.0},
        {PieceType::Rook, 5.0},        {PieceType::Bishop, 3.5},
        {PieceType::Knight, 3.0},      {PieceType::Pawn, 1.0}};
    if (pt != PieceType::King && pt != PieceType::Queen
        && pt != PieceType::Rook && pt != PieceType::Bishop
        && pt != PieceType::Knight && pt != PieceType::Pawn)
    {
        Logger::error("Piece::pieceValue: Called with unrecognized PieceType.");
        return 0.0; // TODO: Diagnose & remove workaround.
    }
    return pt2pv.at(pt);
}

// ---------- Constructors
Piece::Piece(Color color, PieceType pt, Short index,
             MoveIndex lastMoveIndex /* =0 */
             )
    : _color{color}, _pieceType{pt}, _pos{index}, _moveIndexHistory{}
{
    _moveIndexHistory.reserve(lastMoveIndex + VECTOR_CAPACITY_INCR);
    assert(_moveIndexHistory.capacity() - 1 >= (unsigned long)lastMoveIndex);
    if (lastMoveIndex == 0) {
        _moveIndexHistory.push_back(true);
    } else {
        _moveIndexHistory[lastMoveIndex] = true;
    }
}

// ---------- Public read methods
MoveIndex Piece::lastMoveIndex() const {
    for (int k = _moveIndexHistory.size(); k >= 0; --k) {
        if (_moveIndexHistory[k]) {
            return k;
        }
    }
    return 0;
}

// ---------- Public write methods
void Piece::rollBackLastMoveIndex(MoveIndex mi) {
    const MoveIndexHistory::iterator &beg = _moveIndexHistory.begin() + mi;
    const MoveIndexHistory::iterator &end = _moveIndexHistory.end();
    _moveIndexHistory.erase(beg, end);
}

void Piece::updateMoveIndexHistory(MoveIndex mi) {
    if ((_moveIndexHistory.capacity()) <= (unsigned long)mi + 1) {
        _moveIndexHistory.reserve(mi + VECTOR_CAPACITY_INCR);
    }
    assert(_moveIndexHistory.capacity() - 1 >= (unsigned long)mi);
    _moveIndexHistory[mi] = true;
}

// ---------- Operator
ostream &operator<<(ostream &os, const Piece &piece) {
    os << piece._color << piece._pieceType << "_@_" << piece._pos << "=index#"
       << piece.pos().index();
    return os;
}
