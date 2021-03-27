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

    void     setPlayerName(Color color, const string& name) { _color2PlayerName[color] = name; }
    IsAttackingRule getIsAttackingRule(PieceType pieceType);
    MoveRule getMoveRule(PieceType pieceType);
    void     execMove(Board& b, const Move& move);
    void     unexecMove(Board& b, const Move& move);

    void     play();

private:
    GameState getGameState(Color colorCompletingMove) const;
    void initPlayers();

    Board _board;
    Players _players;

    map<Hash, int> _boardLayoutRepetitionCount;
    int _movesSinceCapture = 0;

    const map<Color, Dir> color2Forward
        { {Color::Black, Dir{0, -1}}
        , {Color::White, Dir{0,  1}}
        };
    map<Color, string> _color2PlayerName;
};

// ---------- Public members

Game::Game()
{
    // Init board not needed---use default layout
    initPlayers();
}

void announceGameEnd(const GameState gameState);

void Game::play()
{
    bool isGameInPlay = true;
    int  move_num = 0;

    while (isGameInPlay) {
        move_num++;
        for (auto& color : colors) {
            cout << _board;

            const Pos2Moves& validPlayerMoves = getValidPlayerMoves(_board, color);
            Move move = queryPlayerMove(_board, color, validPlayerMoves);
            move.apply(_board);
            GameState gameState = getGameState(color);
            if (gameState == GameState::InPlay) {
                continue;
            }
            announceGameEnd(gameState);
            isGameInPlay = false;
            break;
        }
    }
}

// ---------- private Game members

// TODO: Test for Draw.
GameState Game::getGameState(Color colorPlayed) const
{

    if (_board.movesSinceLastPawnMoveOrCapture() >= 50) {
        return GameState::Draw_Move50Rule;
    }

    // Test for checkmate
    Color opponentColor = opponent(colorPlayed);
    const Piece& king = _board.king(opponentColor);
    if (!isAttacked(_board, king.pos(), king.color())) {
        return GameState::InPlay;
    }
    Pos2Moves validPlayerMoves = getValidPlayerMoves(_board, colorPlayed);
    for (auto& [from, moves] : validPlayerMoves) {
        for (Move& move : moves) {
            move.apply(const_cast<Board&>(_board));  // Temp board change
            bool canEscape = !isInCheck(_board, colorPlayed);
            move.applyUndo(const_cast<Board&>(_board));  // Undo temp board change
            if (canEscape) {
                return GameState::InPlay;
            }
        }
    }
    return colorPlayed == Color::Black
                              ? GameState::WinBlack_Checkmate
                              : GameState::WinWhite_Checkmate;
}

void Game::initPlayers()
{
}

void announceGameEnd(const GameState gameState)
{
    if (gameState != GameState::InPlay) {
        cout << "Game is over: ";
        switch (gameState)
        {
            case GameState::Draw_Move50Rule:
                cout << "Draw! (50 Move Rule)\n";
                break;
            case GameState::Draw_Move75Rule:
                cout << "Draw! (75 Move Rule)\n";
                break;
            case GameState::Draw_Stalemate:
                cout << "Draw! (Stalemate)\n";
                break;
            case GameState::Draw_ThreefoldRepetition:
                cout << "Draw! (3-fold repetition)\n";
                break;
            case GameState::WinBlack_Checkmate:
                cout << "Black Wins! (Checkmate)";
                break;
            case GameState::WinWhite_Checkmate:
                cout << "White Wins! (Checkmate)";
                break;
            default:
                throw new std::invalid_argument("Unrecognized GameState");
        }
    }
}
