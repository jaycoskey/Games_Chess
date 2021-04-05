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

#include <vector>

#include "util.h"
#include "player.h"
#include "board.h"
#include "move.h"
#include "game_state.h"


class Game {
public:
    static void printConciseMatchSummary(std::vector<GameState>& gss);
    static void printVerboseMatchSummary(const std::vector<GameState>& gss);

    Game();

    const GameState gameLoop();
    void play( Short autoReplayCount=0
             , PlayerType wPlayer=PlayerType::Computer_Random
             , PlayerType bPlayer=PlayerType::Computer_Random
             );

private:
    // ---------- Private read methods
    void _announceGameEnd(const GameState& gs) const;
    void _printGameStats() const;

    // ---------- Private write methods
    void _initPlayers();
    void _reset();

    Board _board;
    Pos2Moves _validPlayerMovesCache{};
};
