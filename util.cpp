// Copyright 2021, by Jay M. Coskey

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <type_traits>
// #include <variant>
#include <vector>

#include "util.h"

using std::string;
using std::ostream, std::ostringstream;
using std::map, std::pair, std::set, std::vector;

using Short = int;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;


string to_string(Color c)
{
    return c == Color::Black ? "Black" : "White";
}

ostream& operator<<(ostream& os, Color c) {
    if (c == Color::Black) { os << 'B'; }
    else { os << 'W'; }
    return os;
}

Color opponent(Color color)
{
    return color == Color::Black ? Color::White : Color::Black;
}

// const vector<Color> allColors{Color::White, Color::Black};
// const Short COLORS_COUNT = 2;

// ---------- Hash
string test_to_string(Hash h)
{
    ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

// ---------- PRNG
std::mt19937& prng() {
    static std::random_device rd;
    static std::mt19937 gen{rd()};
    return gen;
}

// Used for Zobrist hashing in board.h.
Hash random_bitstring() {
    static std::mt19937_64 prng;
    return prng();
}

// ---------- String
string repeatString(const string& input, int count) {
    string result;
    result.reserve(input.size() * count);
    while (count-- > 0) {
        result += input;
    }
    return result;
}
