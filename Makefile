# Copyright 2021, by Jay M. Coskey

CPPFLAGS = -std=c++17
CPPFLAGS += -Wall -Wextra -Wunused
# CPPFLAGS += -w

PROG = chess

TEST_PROGS = test_util

all: build run

build:
	g++ $(CPPFLAGS) -o $(PROG) chess.cpp

run:
	./$(PROG)

test: test
	g++ $(CPPFLAGS) -o test_util test_util.cpp
	./test_util

clean:
	rm -f $(PROG) test_util
