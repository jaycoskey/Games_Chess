// Copyright 2021, by Jay M. Coskey

#pragma once

#include <algorithm>
#include <functional>
#include <tuple>
#include <vector>

#include "util.h"
#include "player.h"
#include "piece.h"
#include "board.h"
#include "move.h"

using std::cout;
using std::map;

// Note: The FIDE laws for the 50 Move Rule mention "50 moves by each player",
//       by the law is interpreted as meaning 50 moves total.
enum class GameState {
    InPlay,
    // Draw_Agreement,
    // Draw_Clock,             // Both players exceed their time allotment.
    // Draw_DeadPosition,      // Insufficient material to lead to checkmate (e.g., two Kings).
    Draw_Move50Rule,           // 50 moves w/ no captures or Pawn moves. Must be claimed by player on turn.
    Draw_Move75Rule,           // 75 moves w/ no captures or Pawn moves. Automatic.
    Draw_Stalemate,            // No legal moves available.
    Draw_ThreefoldRepetition,  // 3x repetition of Board w/ same Player to move.

    // WinBlack_Agreement,
    WinBlack_Checkmate,
    // WinBlack_Clock,

    // WinWhite_Agreement,
    WinWhite_Checkmate
    // WinWhite_Clock,
};

// TODO: Get Player names at startup.
// TODO: Implement check, checkmate, and draw detection.
// TODO: Announce winner at end of Game.
// TODO: Any user-selectable rule variations (e.g., Checker-Pawn Chess)?
class Game {
public:
    Game();

    void        setPlayerName(Color color, const string& name) { color2PlayerName[color] = name; }
    CaptureRule getCaptureRule(PieceType pieceType);
    MoveRule    getMoveRule(PieceType pieceType);
    void        execMove(Board& board, const Move& move);
    void        unexecMove(Board& board, const Move& move);

    void     play();

private:
    std::tuple<bool, GameState> getGameState(Color colorCompletingMove);
    void initPlayers();

    Board board;
    Players players;

    map<Hash, int> boardLayoutRepetitionCount;
    int moveCount = 0;
    int movesSinceCapture = 0;
    Moves moveHistory;

    const map<Color, Dir> color2Forward
        { {Color::Black, Dir{0, -1}}
        , {Color::White, Dir{0,  1}}
        };
    map<Color, string> color2PlayerName;
};

// ---------- Public members

Game::Game()
{
    // Init board not needed---use default layout
    initPlayers();
}

void Game::play()
{
    bool     isGameOver = false;
    GameState endState;

    for (int move_num = 1; ;++move_num) {
        for (auto& color : colors) {
            cout << board;

            const Pos2Moves& validPlayerMoves = getValidPlayerMoves(board, color);
            Move move = getPlayerMove(board, color, validPlayerMoves);
            move.apply(board);
            std::tie(isGameOver, endState) = getGameState(color);
            if (endState != GameState::InPlay) { break; }
        }
        if (endState != GameState::InPlay) { break; }
    }
}

// ---------- private Game members

std::tuple<bool, GameState> Game::getGameState(Color color)
{
    bool      isGameOver = false;
    GameState gameState  = GameState::InPlay;
    return std::make_tuple(isGameOver, gameState);
}

void Game::initPlayers()
{
}
