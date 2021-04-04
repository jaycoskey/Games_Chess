// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>

#include <gtest/gtest.h>

#include "util.h"
#include "piece.h"
#include "board.h"

// using std::cout;


TEST(BoardTest, BoardKings) {
    ScopedTracer(__func__);
    Board b{true};

    const Piece& bk = b.king(Color::Black);
    EXPECT_EQ(bk.pieceType(), PieceType::King);

    const Piece& wk = b.king(Color::White);
    EXPECT_EQ(wk.pieceType(), PieceType::King);
}

TEST(BoardTest, BoardPieceCounts) {
    ScopedTracer(__func__);
    Board b{true};

    for (const auto& [c, piecePs] : b.color2PiecePs) {
        Short kingCount   = 0;
        Short queenCount  = 0;
        Short rookCount   = 0;
        Short bishopCount = 0;
        Short knightCount = 0;
        Short pawnCount   = 0;

        for (const PieceP& pieceP : piecePs) {

            switch(pieceP->pieceType()) {
              case PieceType::King:
                kingCount++;
                break;
              case PieceType::Queen:
                queenCount++;
                break;
              case PieceType::Rook:
                rookCount++;
                break;
              case PieceType::Bishop:
                bishopCount++;
                break;
              case PieceType::Knight:
                knightCount++;
                break;
              case PieceType::Pawn:
                pawnCount++;
                break;
              default:
                throw new std::invalid_argument("Invalid piece type in board data");
            }
        }
        EXPECT_EQ(kingCount,   1);
        EXPECT_EQ(queenCount,  1);
        EXPECT_EQ(rookCount,   2);
        EXPECT_EQ(bishopCount, 2);
        EXPECT_EQ(knightCount, 2);
        EXPECT_EQ(pawnCount,   8);
    }
}

TEST(BoardTest, BoardValue) {
    ScopedTracer(__func__);
    Board b = mkCheckmatesBoard();

    ASSERT_FLOAT_EQ(b.boardValue(Color::Black), KING_VALUE + 19.0);
    ASSERT_FLOAT_EQ(b.boardValue(Color::White), KING_VALUE + 15.0);
}
