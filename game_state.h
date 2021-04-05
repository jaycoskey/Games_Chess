// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>

#include "util.h"
// #include "geometry.h"
// #include "player.h"
// #include "piece.h"
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
    friend std::ostream& operator<<(std::ostream& os, const GameState& gs);
};
