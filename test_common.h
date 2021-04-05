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

#include <iostream>
#include <string>

#include "util.h"
#include "geometry.h"
// #include "player.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "game_state.h"
#include "game.h"


class ScopedTracer {
public:
    ScopedTracer(const char* func_name, bool verbose=false)
        : _func_name{func_name}, _verbose{verbose}
    {
        if (_verbose) {
            std::cout << "Entering " << _func_name << "\n";
        }
    }
    ~ScopedTracer() {
        if (_verbose) {
            std::cout << "Exiting " << _func_name << "\n";
        }
    }
private:
    const char* _func_name;
    bool _verbose;
};

// bool equal_eps(float a, float b) {
//     float eps = 10e-5;  // std::numeric_limits<float>::epsilon();
//     return std::fabs(a - b) < eps;
// }

void add_bk_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::King,   pos, lmi);
}
void add_bq_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::Queen,  pos, lmi);
}
void add_br_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::Rook,   pos, lmi);
}
void add_bb_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::Bishop, pos, lmi);
}
void add_bn_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::Knight, pos, lmi);
}
void add_bp_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::Black, PieceType::Pawn,   pos, lmi);
}

void add_wk_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::King,   pos, lmi);
}
void add_wq_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::Queen,  pos, lmi);
}
void add_wr_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::Rook,   pos, lmi);
}
void add_wb_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::Bishop, pos, lmi);
}
void add_wn_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::Knight, pos, lmi);
}
void add_wp_to(Board& b, const std::string& pos, Short lmi=0) {
    b.addPieceTo(Color::White, PieceType::Pawn,   pos, lmi);
}

// Black can castle Kingside & Queenside. White cannot castle on either side.
Board mkCastlingBoard() {
    Board b{false};

    // Without specifying lastMoveIndex, pieces are treated as though they've never moved.
    add_bk_to(b, "e8");
    add_br_to(b, "a8");
    add_br_to(b, "h8");
    add_bb_to(b, "a6");
    add_bb_to(b, "f6");

    add_wk_to(b, "e1");
    add_wr_to(b, "a1");
    add_wr_to(b, "h1");
    add_wp_to(b, "h2");

    return b;
}

Board mkCheckmatesBoard() {
    Board b{false};

    // Piece constructor called via make_shared ignores lastMoveIndex argument.
    // Workaround by updating MoveIndex history in a separate method call.
    add_bk_to(b, "a8", 2); b.pieceAt(Pos("a8"))->updateMoveIndexHistory(2);
    add_bq_to(b, "h3");
    add_br_to(b, "h5");
    add_bn_to(b, "f4");

    add_bp_to(b, "b7");
    add_bp_to(b, "e2");

    add_wk_to(b, "h1", 2); b.pieceAt(Pos("h1"))->updateMoveIndexHistory(2);
    add_wb_to(b, "e3");
    add_wb_to(b, "e5");
    add_wn_to(b, "d8");

    add_wp_to(b, "a5");
    add_wp_to(b, "c6");
    add_wp_to(b, "f2");
    add_wp_to(b, "g3");
    add_wp_to(b, "h2");

    return b;
}

void printHRule() {
    std::cout << "----------------------------------------\n";
}
