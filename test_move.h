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

#include <string>

#include <gtest/gtest.h>

#include "geometry.h"
#include "util.h"
// #include "player.h"
#include "board.h"
#include "game_state.h"
#include "move.h"
#include "piece.h"

#include "logger.h"
#include "test_common.h"

void _test_move_checkmate(const Board &bRef, Board &b, Move move) {
    ScopedTracer(__func__);
    move.apply(b);
    const Pos2Moves &oppMoves =
        Move::getValidPlayerMoves(b, opponent(move.color()));
    GameState gs{b, move.color(), false, oppMoves};
    ASSERT_TRUE(
        gs.winType() == WinType::Checkmate
        && gs.gameEnd() == (move.color() == Color::Black
                            ? GameEnd::WinBlack
                            : GameEnd::WinWhite
                            )
        );
    move.applyUndo(b);
    ASSERT_EQ(b, bRef);
}

// Black can castle Kingside & Queenside. White cannot castle on either side.
void _test_move_pieceType(PieceType pt, int validMoveCount,
                          const std::string &posStr = "d4")
{
    ScopedTracer(__func__);
    Board b{false};

    add_bk_to(b, "e8");
    add_wk_to(b, "e1");

    b.addPieceTo(Color::Black, pt, posStr);
    const Pos &from{posStr};

    const Piece &piece = *b.pieceAt(from).get();
    MoveRule mRule = Move::getMoveRule(piece.pieceType());
    const Moves &moves = mRule(b, piece.color(), from);
    assert(moves.size() == (unsigned long)validMoveCount);
    for (const Move &move : moves) {
        move.apply(b);
        assert(!b.pieceAt(from));
        move.applyUndo(b);
    }
    ASSERT_TRUE(b.pieceAt(from));
}

void _test_move_undo(const Board &bRef, Board &b, Move move) {
    ScopedTracer(__func__);
    move.apply(b);
    move.applyUndo(b);
    ASSERT_EQ(b, bRef);
}

// On this board: Black can castle Kingside & Queenside.
//                White cannot castle on either side.
void test_move_castling() {
    ScopedTracer(__func__);
    Board b = mkCastlingBoard();
    Board bRef = mkCastlingBoard();
    std::cout << b << "\n";

    Moves blackMoves = concatMap(Move::getValidPlayerMoves(b, Color::Black));
    const Move &bksCastle =
        Move(Color::Black, PieceType::King, Pos{"e8"}, Pos{"g8"});
    const Move &bqsCastle =
        Move(Color::Black, PieceType::King, Pos{"e8"}, Pos{"c8"});
    ASSERT_TRUE(doesContain(blackMoves, bksCastle));
    ASSERT_TRUE(doesContain(blackMoves, bqsCastle));

    Moves whiteMoves = concatMap(Move::getValidPlayerMoves(b, Color::White));
    const Move &wksCastle =
        Move(Color::White, PieceType::King, Pos{"e1"}, Pos{"g1"});
    const Move &wqsCastle =
        Move(Color::White, PieceType::King, Pos{"e1"}, Pos{"c1"});

    bool canWhiteKCastle = doesContain(whiteMoves, wksCastle);
    ASSERT_FALSE(canWhiteKCastle);

    bool canWhiteQCastle = doesContain(whiteMoves, wqsCastle);
    ASSERT_FALSE(canWhiteQCastle);
}

void test_move_checkmate() {
    ScopedTracer(__func__);
    Board b = mkCheckmatesBoard();
    Board bRef = mkCheckmatesBoard();
    std::cout << b << "\n";

    Moves blackMoves = concatMap(Move::getValidPlayerMoves(b, Color::Black));
    const Move &bqxpMate = Move(Color::Black, PieceType::Queen, Pos{"h3"},
                                Pos{"h2"}, b.pieceAt(Pos{"h2"})
                                );
    const Move &bqnMate =
        Move(Color::Black, PieceType::Queen, Pos{"h3"}, Pos{"g2"});
    const Move &bqMate =
        Move(Color::Black, PieceType::Queen, Pos{"h3"}, Pos{"f1"});

    _test_move_checkmate(bRef, b, bqxpMate);
    _test_move_checkmate(bRef, b, bqnMate);
    _test_move_checkmate(bRef, b, bqMate);

    Moves whiteMoves = concatMap(Move::getValidPlayerMoves(b, Color::White));
    const Move &wpxpMate = Move(Color::White, PieceType::Pawn, Pos{"c6"},
                                Pos{"b7"}, b.pieceAt(Pos{"b7"}));
    _test_move_checkmate(bRef, b, wpxpMate);
}

void test_move_pawn_movement() {
    ScopedTracer(__func__);
    Board b = mkCheckmatesBoard();
    Board bRef = mkCheckmatesBoard();
    std::cout << b << "\n";

    Moves blackMoves = concatMap(Move::getValidPlayerMoves(b, Color::Black));

    // Single step
    const Move &pawnStep1 = Move(Color::Black, PieceType::Pawn, Pos{"b7"},
                                 Pos{"b6"}, nullptr, true
                                 );
    ASSERT_TRUE(doesContain(blackMoves, pawnStep1));
    _test_move_undo(bRef, b, pawnStep1);

    // Capture step
    const Move &pawnCapture = Move(Color::Black, PieceType::Pawn, Pos{"b7"},
                                   Pos{"c6"}, b.pieceAt(Pos{"c6"}), true
                                   );
    ASSERT_TRUE(doesContain(blackMoves, pawnCapture));
    _test_move_undo(bRef, b, pawnCapture);

    // Double step
    const Move &pawnStep2 = Move(Color::Black, PieceType::Pawn, Pos{"b7"},
                                 Pos{"b5"}, nullptr, true
                                 );
    ASSERT_TRUE(doesContain(blackMoves, pawnStep2));
    _test_move_undo(bRef, b, pawnStep2);

    pawnStep2.apply(b);
    std::cout << "test_pawn_movement: After applying 'pawnStep2', there is a "
                 "valid en passant move:\n";
    std::cout << b << "\n";
    Moves postStep2Moves =
        concatMap(Move::getValidPlayerMoves(b, Color::White));
    const Move &enPassantMove = Move(Color::White, PieceType::Pawn, Pos{"a5"},
                                     Pos{"b6"}, b.pieceAt(Pos{"b6"}), true
                                     );
    ASSERT_TRUE(doesContain(postStep2Moves, enPassantMove));
    pawnStep2.applyUndo(b);
    std::cout << "_test_pawn_movement: After applyUndo():\n" << b << "\n";
    ASSERT_EQ(b, bRef);

    // Promotion
    const Move &pawnPromotion =
        Move(Color::Black, PieceType::Pawn, Pos{"e2"}, Pos{"e1"}, nullptr,
             false, false, std::make_optional<PieceType>(PieceType::Queen)
             );
    ASSERT_TRUE(doesContain(blackMoves, pawnPromotion));
    _test_move_checkmate(bRef, b, pawnPromotion);

    // Blocked moves
    Moves whiteMoves = concatMap(Move::getValidPlayerMoves(b, Color::White));
    const Move &blockedStep1 = Move(Color::White, PieceType::Pawn, Pos{"h1"},
                                    Pos{"h2"}, nullptr, true
                                    );
    ASSERT_FALSE(doesContain(whiteMoves, blockedStep1));
    const Move &blockedStep2 = Move(Color::White, PieceType::Pawn, Pos{"f1"},
                                    Pos{"f3"}, nullptr, true
                                    );
    ASSERT_FALSE(doesContain(whiteMoves, blockedStep2));

    std::cout << "test_pawn_movement: Exiting\n";
}

void test_move_pieceTypes() {
    ScopedTracer(__func__);
    _test_move_pieceType(PieceType::King, 8);
    _test_move_pieceType(PieceType::Queen, 27);
    _test_move_pieceType(PieceType::Rook, 14);
    _test_move_pieceType(PieceType::Bishop, 13);
    _test_move_pieceType(PieceType::Knight, 8);
    _test_move_pieceType(PieceType::Pawn, 1);
}
