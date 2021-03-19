// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>
#include <set>
#include <sstream>
#include <type_traits>


using Short = int16_t;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;

const Short BOARD_COLS = 8;
const Short BOARD_ROWS = 8;

enum Color {
    Black,
    White
};

// ---------- Board index inversion
Short invertIndex(Short index) { return BOARD_ROWS * BOARD_COLS - 1 - index; }

// ---------- Board direction
struct Dir {
    Dir(Short xarg, Short yarg) : x{xarg}, y{yarg} {}

    Short x;
    Short y;

    bool operator<(const Dir& other) const {
        if (x < other.x || (x == other.x && y < other.y)) { return true; }
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const Dir& dir);
};

// ---------- Board direction functions
std::ostream& operator<<(std::ostream& os, const Dir& dir) {
    os << '(' << dir.x << ", " << dir.y << ')';
    return os;
}

Dir negx(const Dir& dir)  { return Dir(-dir.x,  dir.y); }
Dir negy(const Dir& dir)  { return Dir( dir.x, -dir.y); }
Dir negxy(const Dir& dir) { return Dir(-dir.x, -dir.y); }

std::set<Dir> dirPerms(const Dir& dir) {
    return std::set<Dir>{Dir(dir.x, dir.y), Dir(dir.y, dir.x)};
}

std::set<Dir> dirSigns(const Dir& dir) {
    auto result = std::set<Dir>{dir, negx(dir), negy(dir), negxy(dir)};
    return result;
}

std::set<Dir> dirSignedPerms(const Dir& dir) {
    std::set<Dir> result;
    for (Dir d : dirPerms(dir)) { result.merge(dirSigns(d)); }
    return result;
}

// ---------- Board direction values
template <typename T> std::set<T> getUnion(const std::set<T>& a, const std::set<T>& b);

std::set<Dir> orthoDirs  = dirSignedPerms(Dir(1,0));
std::set<Dir> diagDirs   = dirSigns(Dir(1,1));
std::set<Dir> allDirs    = getUnion(orthoDirs, diagDirs);

std::set<Dir> knightDirs = dirSignedPerms(Dir(1,2));

// ---------- Board position
struct Pos {
    Pos(Short xarg, Short yarg) : x{xarg}, y{yarg} {}
    Pos(Short index) : Pos(div(index, BOARD_COLS)) {}
    Pos(div_t d) : x(d.rem), y(d.quot) {}

    Short x;
    Short y;

    Pos operator+(const Dir& d) { return Pos(x + d.x, y + d.y); }
    Short index() const { return x + BOARD_COLS * y; }
    friend std::ostream& operator<<(std::ostream& os, const Pos& pos);
};

// ---------- Board position functions
std::ostream& operator<<(std::ostream& os, const Pos& pos) {
    os << '(' << pos.x << ", " << pos.y << ')';
    return os;
}

// ---------- Set operations
template <typename T>
std::string showSet(const std::set<T>& items) {
    std::ostringstream oss;
    oss << '[';
    int i = 0;
    for (auto& item : items) {
        if (i++ > 0) { oss << ", "; }
        oss << item;
    }
    oss << ']';
    return oss.str();
}

template <typename T>
std::set<T> getUnion(const std::set<T>& a, const std::set<T>& b) {
    std::set<T> result = a;
    result.insert(b.begin(), b.end());
    return result;
}

// ---------- Prime numbers (first 100) (for hash function in board.h)
int primes[] = {
      2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
     73,  79,  83,  89,  97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541
    };

// From Scott Meyers, Effective Modern C++ (for enums)
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
