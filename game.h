// Copyright 2021, by Jay M. Coskey

#pragma once

#include <functional>
#include <tuple>
#include <vector>


enum CheckType {
    NotCheck,
    Check,
    CheckMate
};

enum EndState {
    NotEnd,
    Tie,
    WinBlack,
    WinWhite
};

using MoveRule = std::function<std::vector<Move>(Board, Color, Pos)>;

// Some moves (e.g., en passant, castling, Pawn promotion) have multiple "steps".
// For such moves, an undo operation must undo all components, or none at all.
// A "primary" move can have an associated "secondary" moves, making a MoveGroup.
// MoveGroups must be undone atomically, or not at all.
//
// TODO: Privacy
class Game {
    Game() {
        initMoveRules();
    }

    static std::map<PieceType, MoveRule> pieceType2moveRule;

private:
    auto checkGameEnd(const Board& board, const Player& playerCompletingMove) {
        // TODO: isCheckmate
        // TODO: isStalemate [multiple forms]
        bool     isGameOver = false;   // TODO
        EndState endState   = NotEnd;  // TODO
        return std::make_tuple(isGameOver, endState);
    }
    void initMoveRules() {
        // TODO: Assert the map has not been initialized
        pieceType2moveRule[King]         = [&](const auto b, const auto c, const auto pos)
                                               { return steps(b, c, King, pos, allDirs); };
        // TODO: pieceType2moveRule[Queen]  = [&](const auto b, const auto c, const auto pos){ return slides(... allDirs); };
        // TODO: pieceType2moveRule[Rook]   = [&](const auto b, const auto c, const auto pos){ return slides(... orthoDirs); };
        // TODO: pieceType2moveRule[Bishop] = [&](const auto b, const auto c, const auto pos){ return slides(... diagDirs); };
        // TODO: pieceType2moveRule[Knight] = [&](const auto b, const auto c, const auto pos){ return leaps(.. knightDirs); };
        // TODO: pieceType2moveRule[Pawn]   = pawnMoveRule;
    }
    std::vector<Move>
    pawnMoveRule(const Board& b, const Player& p, const Pos& pos) {
        std::vector<Move> result{};
        // TODO: if (relRow(pos) < BOARD_ROWS - 1) { result.insert(step(p.getForward())); }  // Advance 1 space
        // TODO: if (relRow(pos) == 2) { result.insert(step(p.getForward(), 2)); }           // Advance 2 spaces
        // TODO: if (piece at FL) { result.insert(move + capture); }                // Capture FL
        // TODO: if (piece at FR) { result.insert(move + capture); }                // Capture FL
        // TODO: if (is en passant available) { capture en passant }                // Capture FL
        // TODO: if (relRow(pos) == BOARD_ROWS) { append pawn promotion to each item in result } // Pawn promotion
        return result;
    }

    auto play() {
        bool     isGameOver = false;
        EndState endState;

        for (int round_num = 0; ;++round_num) {
            for (Player player : players) {
                // TODO: player.getMove()
                std::tie(isGameOver, endState) = checkGameEnd(board, player);
                if (endState != NotEnd) { break; }
            }
            if (endState != NotEnd) { break; }
        }
        // TODO: Announce winner
    }

    // ----------
    Board board;
    int moveCount = 0;
    int movesSinceCapture = 0;
    std::vector<Move> moveHistory;
    std::map<Hash, int> boardLayoutRepetitions;
    std::vector<Player> players;
};
