CPPFLAGS = -std=c++17
CPPFLAGS += -Wall -Wextra -Wunused
CPPFLAGS += -g
CPPFLAGS += -Qunused-arguments
# CPPFLAGS += -w

PROG = chess

TEST_PROGS = test_util

all: build run

build:
	g++ $(CPPFLAGS) -o $(PROG) chess.cpp

E:
	g++ $(CPPFLAGS) -E chess.cpp

run:
	./$(PROG)

test:
	g++ $(CPPFLAGS) -o test_util test_util.cpp
	./test_util

clean:
	rm -rf $(PROG) *.dSYM test_util
