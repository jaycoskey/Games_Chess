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


Color opponent(Color color)
{
    return color == Color::Black ? Color::White : Color::Black;
}

string to_string(Color c)
{
    return c == Color::Black ? "Black" : "White";
}

ostream& operator<<(ostream& os, Color c) {
    if (c == Color::Black) { os << 'B'; }
    else { os << 'W'; }
    return os;
}

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
