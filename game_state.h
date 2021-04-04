// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>

#include "util.h"
#include "geometry.h"
#include "piece.h"
#include "board.h"
#include "move.h"


class GameState {
public:
    GameState();
    GameState(GameEnd gameEnd, WinType winType, DrawFlags drawFlags);
    GameState( const Board& b, Color c, bool isDrawClaim
             , const Pos2Moves& validOppPos2Moves
             );

    GameEnd   gameEnd()     const { return _gameEnd; };
    WinType   winType()     const { return _winType; };
    DrawFlags drawFlags()   const { return _drawFlags; }
    bool      isCheck()     const { return _isCheck; }
    bool      isCheckmate() const { return _isCheckmate; }

private:
    GameEnd   _gameEnd;
    WinType   _winType;
    DrawFlags _drawFlags;
    bool      _isCheck;
    bool      _isCheckmate;

    friend bool operator<(const GameState& gs1, const GameState& gs2);
    friend ostream& operator<<(ostream& os, const GameState& gs);
};

// ---------- Constructors
GameState::GameState()
    : _gameEnd{GameEnd::InPlay}, _winType{WinType::None}, _drawFlags{0}
    , _isCheck{false}, _isCheckmate{false}
{}

GameState::GameState(GameEnd gameEnd, WinType winType, DrawFlags drawFlags)
    : _gameEnd{gameEnd}, _winType{winType}, _drawFlags{drawFlags}
    , _isCheck{false}, _isCheckmate{false}
{ }

GameState::GameState
    ( const Board& b, Color colorPlayed, bool isDrawClaim
    , const Pos2Moves& oppPos2Moves
    )
    : _gameEnd{GameEnd::InPlay}
    , _winType{WinType::None}
    , _drawFlags{Draw_None}
    , _isCheck{false}
    , _isCheckmate{false}
{
    // Test for checkmate
    Color oppColor = opponent(colorPlayed);
    const Piece& oppKing = b.king(oppColor);
    bool isOppKingInCheck = Move::isAttacked(b, oppKing.pos(), oppKing.color());
    bool canOppKingEscape = false;
    if (isOppKingInCheck) {
        _isCheck = true;
        // Determine whether there is checkmate
        logger.trace("GameState::GameState: Found check. Determine if it is checkmate");
        for (auto& [oppFrom, oppMoves] : oppPos2Moves) {
            for (const Move& oppMove : oppMoves) {
                assert(!oppMove.isCastling());
                oppMove.apply(const_cast<Board&>(b));  // Temp board change
                bool isOppStillInCheck = Move::isInCheck(b, oppColor);
                oppMove.applyUndo(const_cast<Board&>(b));  // Undo temp board change
                if (!isOppStillInCheck) {
                    canOppKingEscape = true;
                    break;
                }
            }
        }
    }
    if (isOppKingInCheck && !canOppKingEscape) {
        _isCheckmate = true;
        _isCheck     = false;
        _gameEnd = colorPlayed == Color::Black
                         ? GameEnd::WinBlack
                         : GameEnd::WinWhite;
        _winType = WinType::Checkmate;
        return;
    }

    // Test for automatic Draw
    bool isStalemate = oppPos2Moves.size() == 0;
    if (isStalemate) {
        _drawFlags |= Draw_Stalemate;
    }
    if (b.hasInsufficientResources()) {
        _drawFlags |= Draw_InsufficientResources;
    }
    if (b.maxBoardRepetitionCount(colorPlayed) >= 5) {
        _drawFlags |= Draw_5xRepetition;
    }
    if (b.movesSinceLastPmoc() >= 75) {
        _drawFlags |= Draw_75MoveRule;
    }

    // Test for claimed Draw
    if (isDrawClaim && b.maxBoardRepetitionCount(colorPlayed) >= 3) {
        _drawFlags |= Draw_Claimed_3xRepetition;
    }
    if (isDrawClaim && b.movesSinceLastPmoc() >= 50) {
        _drawFlags |= Draw_Claimed_50MoveRule;
    }
    if (_drawFlags != Draw_None) {
        _gameEnd = GameEnd::Draw;
    }
}

// ---------- Operator
bool operator<(const GameState& gs1, const GameState& gs2) {
    if (gs1._gameEnd < gs2._gameEnd) { return true; }
    else if (gs2._gameEnd < gs1._gameEnd) { return false; }

    if (gs1._winType < gs2._winType) { return true; }
    else if (gs2._winType < gs1._winType) { return false; }

    if (gs1._drawFlags < gs2._drawFlags) { return true; }
    else if (gs2._drawFlags < gs1._drawFlags) { return false; }

    if (gs1._isCheck < gs2._isCheck) { return true; }
    else if (gs2._isCheck < gs1._isCheck) { return false; }

    if (gs1._isCheckmate < gs2._isCheckmate) { return true; }
    else { return false; }
}

ostream& operator<<(ostream& os, const GameState& gs) {
    if (gs._gameEnd == GameEnd::InPlay) {
        os << "The game is in play.\n";
        return os;
    }
    switch (gs._gameEnd) {
        case GameEnd::Draw:
            os << "Draw (code=" << std::setw(2) << gs._drawFlags << ") - ";
            if ((gs._drawFlags & Draw_5xRepetition) != Draw_None) {
                os << "5x Repetition. ";
            }
            if ((gs._drawFlags & Draw_75MoveRule) != Draw_None) {
                os << "75 Move Rule. ";
            }
            if ((gs._drawFlags & Draw_Claimed_3xRepetition) != Draw_None) {
                os << "3x Repetition (claimed). ";
            }
            if ((gs._drawFlags & Draw_Claimed_50MoveRule) != Draw_None) {
                os << "50 Move Rule (claimed). ";
            }
            if ((gs._drawFlags & Draw_InsufficientResources) != Draw_None) {
                os << "Insufficient Resources. ";
            }
            if ((gs._drawFlags & Draw_Stalemate) != Draw_None) {
                os << "Stalemate. ";
            }
            break;
        case GameEnd::WinBlack:
        case GameEnd::WinWhite:
            os << "Winner = ";
            os << (gs._gameEnd == GameEnd::WinBlack ? "Black" : "White");
            os << ", by " << gs._winType;
            break;
        default:
            throw new std::invalid_argument("Internal Error: Unrecognized GameEnd value");
    }
    os << "\n";
    return os;
}
