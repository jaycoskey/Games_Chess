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

#include "util.h"
// #include "geometry.h"
// #include "player.h"
// #include "piece.h"
#include "board.h"
#include "move.h"

// State of Game (Win, Draw, or still in play) determined after each move.
class GameState {
  public:
    GameState();
    GameState(GameEnd gameEnd, WinType winType, DrawFlags drawFlags);
    GameState(const Board &b, Color c, bool isDrawClaim,
              const Pos2Moves &validOppPos2Moves
              );

    GameEnd gameEnd() const { return _gameEnd; };
    WinType winType() const { return _winType; };
    DrawFlags drawFlags() const { return _drawFlags; }
    bool isCheck() const { return _isCheck; }
    bool isCheckmate() const { return _isCheckmate; }

  private:
    GameEnd _gameEnd;
    WinType _winType;
    DrawFlags _drawFlags;

    // isCheck and isCheckmate are copied into the last Move instance.
    bool _isCheck;
    bool _isCheckmate;

    friend bool operator<(const GameState &gs1, const GameState &gs2);
    friend std::ostream &operator<<(std::ostream &os, const GameState &gs);
};
