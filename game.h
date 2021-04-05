// Copyright 2021, by Jay M. Coskey

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
