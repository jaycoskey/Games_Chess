// Copyright 2021, by Jay M. Coskey

#include <iostream>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "game.h"


int main() {
    logger.setLogLevel(LogDebug);
    Game game{};
    game.play();
}
