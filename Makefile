
.PHONY: all build clean objs run test
all: build run

# ---------------------------------------- 
CPP=g++
# ---------------------------------------- 
CPPFLAGS = -std=c++17
CPPFLAGS += -Wall -Wextra -Wunused
CPPFLAGS += -g
# CPPFLAGS += -v

LDFLAGS = -lgtest

SRC_DIR := .
SRCS := chess.cpp
HDRS := board.h game.h game_state.h geometry.h logger.h move.h piece.h player.h util.h

OBJ_DIR := .
OBJS := chess.o

PROG := chess
$(PROG): $(OBJS)
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

# ---------------------------------------- 
TEST_CPP := $(CPP)
TEST_CPPFLAGS := $(CPPFLAGS)
TEST_LDFLAGS := $(LDFLAGS)

TEST_SRC_DIR := .
TEST_SRCS := test_chess.cpp

# TODO: Add tests for Game, GameState, Dir, Pos, Piece, Player
TEST_HDRS := test_board.h test_logger.h test_move.h test_util.h

TEST_OBJ_DIR := .

TEST_OBJS := test_chess.o

TEST_PROG := test_chess

$(TEST_PROG): $(TEST_OBJS)
	$(TEST_CPP) $(TEST_LDFLAGS) -o $@ $^
 
$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp $(HDRS)
	$(TEST_CPP) $(TEST_CPPFLAGS) -c -o $@ $<

# ---------------------------------------- 
objs: $(OBJS)

build: $(PROG)

run:
	$(OBJ_DIR)/$(PROG)

test: $(TEST_PROG)
	$(TEST_OBJ_DIR)/$(TEST_PROG)

clean:
	rm -rf $(PROG) $(OBJS)
	rm -rf $(TEST_PROG) $(TEST_OBJS)
	rm -f *.dSYM *.E
	rm -rf test_board test_board_state test_logger test_move test_util
	rm -rf test_logger_*
