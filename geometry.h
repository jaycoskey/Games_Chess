// Copyright 2021, by Jay M. Coskey

#pragma once

#include <cctype>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

#include "util.h"

using std::string;
using std::cout, std::ostream;
using std::set;

struct Dir;
using Dirs = set<Dir>;


const Col BOARD_COLS = 8;
const Row BOARD_ROWS = 8;

// ---------- Board index inversion

Short invertIndex(Short index) { return BOARD_ROWS * BOARD_COLS - 1 - index; }
Short invertRow(Short index) {
    return BOARD_ROWS * BOARD_COLS - 1 - index
        + 2 * (index % BOARD_COLS) - (BOARD_COLS - 1);
}

// ---------- Direction

struct Dir {
    Dir(Col x, Row y) : x{x}, y{y} {}

    Col x;
    Row y;

    Dir operator+(const Dir& d) const { return Dir(x + d.x, y + d.y); }
    bool operator<(const Dir& other) const;

    friend ostream& operator<<(ostream& os, const Dir& dir);
};

ostream& operator<<(ostream& os, const Dir& dir)
{
    os << '(' << dir.x << ", " << dir.y << ')';
    return os;
}

bool Dir::operator<(const Dir& other) const
{
    if (x < other.x || (x == other.x && y < other.y)) { return true; }
    return false;
}

// ---------- Direction non-member functions

Dir negx(const Dir& dir)  { return Dir(-dir.x,  dir.y); }
Dir negy(const Dir& dir)  { return Dir( dir.x, -dir.y); }
Dir negxy(const Dir& dir) { return Dir(-dir.x, -dir.y); }

Dirs dirPerms(const Dir& dir)
{
    return Dirs{Dir(dir.x, dir.y), Dir(dir.y, dir.x)};
}

Dirs dirSigns(const Dir& dir)
{
    auto result = Dirs{dir, negx(dir), negy(dir), negxy(dir)};
    return result;
}

Dirs dirSignedPerms(const Dir& dir)
{
    Dirs result;
    for (Dir d : dirPerms(dir)) { result.merge(dirSigns(d)); }
    return result;
}

// ---------- Direction non-member values

Dirs orthoDirs   = dirSignedPerms(Dir(1,0));
Dirs diagDirs    = dirSigns(Dir(1,1));
Dirs allDirs     = getUnion(orthoDirs, diagDirs);

Dirs knightDirs  = dirSignedPerms(Dir(1,2));

// ---------- Position

struct Pos {
    Pos(Col x, Row y) : x{x}, y{y} {}
    Pos(Short index) : Pos(div(index, BOARD_COLS)) {}
    Pos(div_t d) : x(d.rem), y(d.quot) {}
    Pos(const Pos& pos) : x(pos.x), y(pos.y) {}

    Col x;
    Row y;

    Pos operator+(const Dir& d) const { return Pos(x + d.x, y + d.y); }
    bool operator<(const Pos& other) const;
    bool operator==(const Pos& other) const;

    Pos fromRel(Color c) const { return Pos(toRelCol(c), toRelRow(c)); }

    Col   toRelCol(Color c) const { return c == Color::White ? x : BOARD_COLS - 1 - x; }
    Row   toRelRow(Color c) const { return c == Color::White ? y : BOARD_ROWS - 1 - y; }
    Short index() const { return x + BOARD_COLS * y; }
    const string algNotation() const;

    Short xdiff(const Pos& other) const { return other.x - x; }

    Pos posLeft(Color c, Short n)  const { return Pos(x + (c == Color::White ? -n :  n), y); }
    Pos posRight(Color c, Short n) const { return Pos(x + (c == Color::White ?  n : -n), y); }

    void moveTo(const Pos& other);

    friend ostream& operator<<(ostream& os, const Pos& pos);
};

ostream& operator<<(ostream& os, const Pos& pos)
{
    os << pos.algNotation();
    return os;
}

const string Pos::algNotation() const
{
    ostringstream oss;

    if (x < 0)  { oss << 'L'; }
    else if (x > BOARD_COLS - 1) { oss << 'R'; }
    else { oss << char((unsigned char)('a') + x); }

    if (y < 0) { oss << 'B'; }
    else if (y > BOARD_ROWS - 1) { oss << 'T'; }
    else { oss << y + 1; }

    return oss.str();
}

// ---------- public Position methods

bool Pos::operator<(const Pos& other) const
{
    if (x < other.x || (x == other.x && y < other.y)) { return true; }
    return false;
}

bool Pos::operator==(const Pos& other) const
{
    return x == other.x && y == other.y;
}

void Pos::moveTo(const Pos& other)
{
    x = other.x;
    y = other.y;
}

// ---------- Non-member Position methods

bool isPawnInitialPosition(Color color, const Pos& pos) {
    return pos.toRelRow(color) == 1;
}
