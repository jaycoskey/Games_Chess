CC=g++

CPPFLAGS = -std=c++17
CPPFLAGS += -Wall -Wextra -Wunused
CPPFLAGS += -g
# CPPFLAGS += -w

LDFLAGS = -lgtest

PROG = chess

.PHONY: all
all: build run

TEST_PROGS = test_util
OBJECTS=chess.o

build: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) -o $(PROG) $(LDFLAGS)

E:
	$(CC) $(CPPFLAGS) -E chess.cpp

run:
	./$(PROG)

test:
	g++ $(CPPFLAGS) -o test_board  test_board.cpp
	g++ $(CPPFLAGS) -o test_logger test_logger.cpp
	g++ $(CPPFLAGS) -o test_util   test_util.cpp
	./test_board
	./test_logger
	./test_util

clean:
	rm -rf $(PROG) *.o *.dSYM
	rm -rf test_board test_logger test_util
	rm -rf test_logger_*
