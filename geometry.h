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
using std::ostream;
using std::set;

struct Dir;
using Dirs = set<Dir>;


const Col BOARD_COLS = 8;
const Row BOARD_ROWS = 8;
const Short BOARD_SPACES = BOARD_COLS * BOARD_ROWS;

const Col BOARD_KING_COL = 4;
const Row BOARD_PAWN_PROMOTION_ROW = BOARD_ROWS - 1;
const Row BOARD_EN_PASSANT_FROM_ROW = 4;

// ---------- Board index inversion

Row homeRow(Color c) { return c == Color::Black ? BOARD_ROWS - 1 : 0; }
Short invertIndex(Short index) { return BOARD_SPACES - 1 - index; }
Short invertRow(Short index) {
    return BOARD_SPACES - 1 - index
        + 2 * (index % BOARD_COLS) - (BOARD_COLS - 1);
}

// ========================================
// Direction

struct Dir {
    Dir(Col x, Row y) : x{x}, y{y} {}

    Col x;
    Row y;

    bool isDir(Col dx, Row dy) { return dx == x && dy == y; }
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

// ---------- Direction-valued constants

Dirs orthoDirs   = dirSignedPerms(Dir(1,0));
Dirs diagDirs    = dirSigns(Dir(1,1));
Dirs allDirs     = getUnion(orthoDirs, diagDirs);

Dirs knightDirs  = dirSignedPerms(Dir(1,2));

// ========================================
// Position

struct Pos {
    Pos(Col x, Row y) : x{x}, y{y} {}
    Pos(Short index) : Pos(div(index, BOARD_COLS)) {}
    Pos(div_t d) : x(d.rem), y(d.quot) {}
    Pos(const Pos& pos) : x(pos.x), y(pos.y) {}
    Pos(const string& posStr);  // For testing

    Col x;
    Row y;

    // ---------- Pos read methods
    // Convert abs<-->rel
    Pos fromRel(Color c) const { return Pos(toRelCol(c), toRelRow(c)); }
    Col toRelCol(Color c) const { return c == Color::White ? x : BOARD_COLS - 1 - x; }
    Row toRelRow(Color c) const { return c == Color::White ? y : BOARD_ROWS - 1 - y; }

    // Shift left/right
    Pos posLeft(Col col)  const { return Pos(x - col, y); }
    Pos posRight(Col col) const { return Pos(x + col, y); }

    const string algNotation() const;
    Short index() const { return x + BOARD_COLS * y; }
    bool  isAt(Col col, Row row) const { return col == x && row == y; }
    bool  isOnBoard() const { return x >= 0 && y >= 0 && x < BOARD_COLS && y < BOARD_ROWS; }
    bool  isPawnInitialPosition(Color color) const { return toRelRow(color) == 1; }
    bool  isPawnPromotionRow(Color color) const {
        return toRelRow(color) == BOARD_PAWN_PROMOTION_ROW;
    }
    Color squareColor() const { return (x + y) % 2 == 0 ? Color::Black : Color::White; }
    Col   xdiff(const Pos& other) const { return x - other.x; }
    Row   ydiff(const Pos& other) const { return y - other.y; }

    // ---------- Pos write methods
    void moveTo(const Pos& other) { x = other.x; y = other.y; }

    // ---------- Pos operators
    Pos operator+(const Dir& d) const { return Pos(x + d.x, y + d.y); }
    bool operator<(const Pos& other) const;
    bool operator==(const Pos& other) const { return x == other.x && y == other.y; }

    friend ostream& operator<<(ostream& os, const Pos& pos);
};

// ---------- Pos constructor
Pos::Pos(const string& posStr)
    : x{static_cast<Short>(tolower(posStr[0]) - 'a')}
    , y{static_cast<Short>(stoi(posStr.substr(1)) - 1)}
{}  // For testing

// ---------- Pos read methods
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

// ---------- Pos operators
bool Pos::operator<(const Pos& other) const
{
    return x < other.x || (x == other.x && y < other.y);
}

ostream& operator<<(ostream& os, const Pos& pos)
{
    os << pos.algNotation();
    return os;
}
