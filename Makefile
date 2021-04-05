
.PHONY: all build clean objs run test
all: build run

# ---------------------------------------- 
CPP=g++
# ---------------------------------------- 
CPPFLAGS = -std=c++17
CPPFLAGS += -Wall -Wextra -Wunused
CPPFLAGS += -g
# CPPFLAGS += -v

LDFLAGS = -std=c++17 -lgtest

SRC_DIR := .
MAIN_SRC := chess.cpp
OTHER_SRCS := board.cpp game.cpp game_state.cpp geometry.cpp logger.cpp move.cpp piece.cpp player.cpp util.cpp
HDRS := board.h game.h game_state.h geometry.h logger.h move.h piece.h player.h util.h

OBJ_DIR := .
MAIN_OBJ := $(MAIN_SRC:.cpp=.o)
OTHER_OBJS := $(OTHER_SRCS:.cpp=.o)

PROG := chess
$(PROG): $(MAIN_OBJ) $(OTHER_OBJS)
	$(CPP) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

# ---------------------------------------- 
TEST_CPP := $(CPP)
TEST_CPPFLAGS := $(CPPFLAGS)
TEST_LDFLAGS := $(LDFLAGS)

TEST_SRC_DIR := .
TEST_SRCS := test_chess.cpp

# TODO: Add tests for Game, GameState, Dir, Pos, Piece, Player
TEST_HDRS := test_board.h test_common.h test_game_state.h test_logger.h test_move.h test_util.h

TEST_OBJ_DIR := .

TEST_OBJS := test_chess.o

TEST_PROG := test_chess

$(TEST_PROG): $(TEST_OBJS) $(OTHER_OBJS)
	$(TEST_CPP) $(TEST_LDFLAGS) -o $@ $^
 
$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp $(HDRS) $(TEST_HRDS)
	$(TEST_CPP) $(TEST_CPPFLAGS) -c -o $@ $<

# ---------------------------------------- 
objs: $(MAIN_OBJS) $(OTHER_OBJS)

build: $(PROG)

run:
	$(OBJ_DIR)/$(PROG)

test: $(TEST_PROG)
	$(TEST_OBJ_DIR)/$(TEST_PROG)

clean:
	rm -rf $(PROG) $(MAIN_OBJ) $(OTHER_OBJS)
	rm -rf $(TEST_PROG) $(TEST_OBJS)
	rm -f *.dSYM *.E
	rm -rf test_logger_*
