// Copyright 2021, by Jay M. Coskey

#include <iostream>

#include "test_common.h"

#include "test_board.h"
#include "test_game_state.h"
#include "test_logger.h"
#include "test_move.h"
#include "test_util.h"

using std::cout;


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    cout << "========================================\n";
    test_logger();  // Check output

    return result;
}
