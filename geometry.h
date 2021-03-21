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

// ---------- Direction

struct Dir {
    Dir(Col x, Row y) : x{x}, y{y} {}

    Col x;
    Row y;

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

Dirs orthoDirs  = dirSignedPerms(Dir(1,0));
Dirs diagDirs   = dirSigns(Dir(1,1));
Dirs allDirs    = getUnion(orthoDirs, diagDirs);

Dirs knightDirs = dirSignedPerms(Dir(1,2));

// ---------- Position

struct Pos {
    Pos(Col x, Row y) : x{x}, y{y} {}
    Pos(Short index) : Pos(div(index, BOARD_COLS)) {}
    Pos(div_t d) : x(d.rem), y(d.quot) {}
    Pos(const Pos& pos) : x(pos.x), y(pos.y) {}

    Col x;
    Row y;

    Pos operator+(const Dir& d) const;
    bool operator<(const Pos& other) const;
    bool operator==(const Pos& other) const;
    void setPos(const Pos& other);
    Short index() const;

    friend ostream& operator<<(ostream& os, const Pos& pos);
};

ostream& operator<<(ostream& os, const Pos& pos)
{
    os << '(' << pos.x << ", " << pos.y << ')';
    return os;
}

// ---------- public Position methods

Pos Pos::operator+(const Dir& d) const
{
    return Pos(x + d.x, y + d.y);
}

bool Pos::operator<(const Pos& other) const
{
    if (x < other.x || (x == other.x && y < other.y)) { return true; }
    return false;
}

bool Pos::operator==(const Pos& other) const
{
    return x == other.x && y == other.y;
}

Short Pos::index() const
{
    return x + BOARD_COLS * y;
}

void Pos::setPos(const Pos& other)
{
    x = other.x;
    y = other.y;
}
