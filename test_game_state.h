// Copyright 2021, by Jay M. Coskey

#pragma once

#include <gtest/gtest.h>

#include "move.h"
#include "game_state.h"


TEST(BoardStateTest, NearMate)
{
    ScopedTracer(__func__);
    Board b = Board{false};
    add_bk_to(b, "f4");
    add_br_to(b, "d7");
    add_bp_to(b, "b5");

    add_wk_to(b, "d4");

    // White is in check, but not mate.
    const Pos2Moves& p2m1 = Move::getValidPlayerMoves(b, Color::White);
    GameState gs1{b, Color::Black, false, p2m1};
    ASSERT_EQ(gs1.gameEnd(), GameEnd::InPlay);

    // White is in checkmate.
    add_bn_to(b, "a4");
    const Pos2Moves& p2m2 = Move::getValidPlayerMoves(b, Color::White);
    GameState gs2{b, Color::Black, false, p2m2};
    ASSERT_EQ(gs2.gameEnd(), GameEnd::WinBlack);

    b.removePieceAt(Pos("d7"));
    add_bp_to(b, "d6");

    add_wp_to(b, "d3");
    add_wp_to(b, "d5");

    // White is in stalemate.
    const Pos2Moves& p2m3 = Move::getValidPlayerMoves(b, Color::White);
    GameState gs3{b, Color::Black, false, p2m3};
    ASSERT_TRUE(  gs3.gameEnd() == GameEnd::Draw
               && (gs3.drawFlags() & Draw_Stalemate) != Draw_None
               );
}

TEST(BoardStateTest, Draw_kr_kb)
{
    ScopedTracer(__func__);
    Board b{false};

    add_bk_to(b, "a8");
    add_br_to(b, "a7");

    add_wk_to(b, "h1");
    add_wb_to(b, "h2");

    // InsufficientResources
    const Pos2Moves& p2m = Move::getValidPlayerMoves(b, Color::White);
    GameState gs{b, Color::Black, false, p2m};
    ASSERT_TRUE(  gs.gameEnd() == GameEnd::Draw
               && (gs.drawFlags() & Draw_InsufficientResources) != Draw_None
               );
}

TEST(BoardStateTest, Draw_kr_kn)
{
    ScopedTracer(__func__);
    Board b{false};

    add_bk_to(b, "a8");
    add_br_to(b, "a7");

    add_wk_to(b, "h1");
    add_wn_to(b, "h2");

    // InsufficientResources
    const Pos2Moves& p2m = Move::getValidPlayerMoves(b, Color::White);
    GameState gs{b, Color::Black, false, p2m};
    ASSERT_TRUE(  gs.gameEnd() == GameEnd::Draw
               && (gs.drawFlags() & Draw_InsufficientResources) != Draw_None
               );
}

TEST(BoardStateTest, Draw_kb_kb)
{
    ScopedTracer(__func__);
    Board b{false};
   
    add_bk_to(b, "a8");
    add_bb_to(b, "a7");

    add_wk_to(b, "h1");
    add_wb_to(b, "h2");

    // InsufficientResources
    const Pos2Moves& p2m = Move::getValidPlayerMoves(b, Color::White);
    GameState gs{b, Color::Black, false, p2m};
    ASSERT_TRUE(  gs.gameEnd() == GameEnd::Draw
               && (gs.drawFlags() & Draw_InsufficientResources) != Draw_None
               );
}
