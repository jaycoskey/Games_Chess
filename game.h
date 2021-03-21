// Copyright 2021, by Jay M. Coskey

#pragma once

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


enum class GameState {
    InPlay,
    Tie,
    WinBlack,
    WinWhite
};

// TODO: Privacy
class Game {
public:
    Game();

    void execMove(Board& board, const Move& move);
    MoveRule getMoveRule(PieceType pieceType);
    void play();

private:
    std::tuple<bool, GameState> getGameState(const Player& playerCompletingMove);
    void initMoveRules();
    void initPlayers();

    Board board;
    Players players;

    map<Hash, int> boardLayoutRepetitionCount;
    int moveCount = 0;
    int movesSinceCapture = 0;
    Moves moveHistory;
    PieceType2MoveRule pieceType2MoveRule;
};

// ---------- Public members

Game::Game()
{
    // Init board not needed---use default
    initMoveRules();
    initPlayers();
}

void Game::execMove(Board& board, const Move& move)
{
    // TODO: Support complex moves (en passant, castling, Pawn promotion, etc.
    PieceP movedPieceP = board.removePieceFrom(move.from());
    board.addPieceTo(movedPieceP, move.to());
}

MoveRule Game::getMoveRule(PieceType pieceType)
{
    return pieceType2MoveRule.at(pieceType);
}

void Game::play()
{
    bool     isGameOver = false;
    GameState endState;

    for (int round_num = 0; ;++round_num) {
        for (auto& player : players) {
            cout << board << "\n";

            const Pos2Moves& validPlayerMoves = getValidPlayerMoves(pieceType2MoveRule, board, player.color());
            Move move = getPlayerMove(board, player, validPlayerMoves);
            execMove(board, move);
            std::tie(isGameOver, endState) = getGameState(player);
            if (endState != GameState::InPlay) { break; }
        }
        if (endState != GameState::InPlay) { break; }
    }
    // TODO: Announce winner
}

// ---------- private Game members

std::tuple<bool, GameState> Game::getGameState(const Player& playerCompletingMove)
{
    // TODO:  isCheckmate
    // TODO:  isStalemate [multiple forms]
    bool      isGameOver = false;   // TODO
    GameState gameState  = GameState::InPlay;  // TODO
    return std::make_tuple(isGameOver, gameState);
}

void Game::initMoveRules()
{
    // TODO: Assert the map has not been initialized
    pieceType2MoveRule[PieceType::King]   = [&](const auto& b, const auto c, const auto& pos)
                                              { return move_steps( b, c, pos, PieceType::King,   allDirs); };
    pieceType2MoveRule[PieceType::Queen]  = [&](const auto& b, const auto c, const auto& pos)
                                              { return move_slides(b, c, pos, PieceType::Queen,  allDirs); };
    pieceType2MoveRule[PieceType::Rook]   = [&](const auto& b, const auto c, const auto& pos)
                                              { return move_slides(b, c, pos, PieceType::Rook,   orthoDirs); };
    pieceType2MoveRule[PieceType::Bishop] = [&](const auto& b, const auto c, const auto& pos)
                                              { return move_slides(b, c, pos, PieceType::Bishop, diagDirs); };
    pieceType2MoveRule[PieceType::Knight] = [&](const auto& b, const auto c, const auto& pos)
                                              { return move_leaps( b, c, pos, PieceType::Knight, knightDirs); };
    // TODO: pieceType2MoveRule[PieceType::Pawn]   = pawnMoveRule;
}

void Game::initPlayers()
{
    players.emplace(players.end(), Color::White, Dir(0,  1), "Amy");
    players.emplace(players.end(), Color::Black, Dir(0, -1), "Bob");
}

// ---------- Game non-member functions

const Moves& pawnMoveRule(const Board& b, const Player& p, const Pos& pos)
{
    Moves result{};
    // TODO: if (relRow(pos) < BOARD_ROWS - 1) { result.insert(step(p.getForward())); }  // Advance 1 space
    // TODO: if (relRow(pos) == 2) { result.insert(step(p.getForward(), 2)); }           // Advance 2 spaces
    // TODO: if (piece at FL) { result.insert(move + capture); }                // Capture FL
    // TODO: if (piece at FR) { result.insert(move + capture); }                // Capture FL
    // TODO: if (is en passant valid) { capture en passant }                // Capture FL
    // TODO: if (relRow(pos) == BOARD_ROWS) { append pawn promotion to each item in result } // Pawn promotion
    return result;
}
