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

#pragma once

#include <cctype>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "util.h"


struct Dir;
using Dirs = std::set<Dir>;


constexpr Col BOARD_COLS = 8;
constexpr Row BOARD_ROWS = 8;
constexpr Short BOARD_SPACES = BOARD_COLS * BOARD_ROWS;

constexpr Col BOARD_KING_COL = 4;
constexpr Row BOARD_PAWN_PROMOTION_ROW = BOARD_ROWS - 1;
constexpr Row BOARD_EN_PASSANT_FROM_ROW = 4;

// ---------- Board index inversion

Row homeRow(Color c);
Short invertIndex(Short index);
Short invertRow(Short index);

// ========================================
// Direction

struct Dir;

struct Dir {
    static const Dirs orthoDirs();
    static const Dirs diagDirs();
    static const Dirs allDirs();
    static const Dirs knightDirs();

    Dir(Col x, Row y) : x{x}, y{y} {}

    Col x;
    Row y;

    bool isDir(Col dx, Row dy) { return dx == x && dy == y; }
    Dir operator+(const Dir& d) const { return Dir(x + d.x, y + d.y); }
    bool operator<(const Dir& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Dir& dir);
};

std::ostream& operator<<(std::ostream& os, const Dir& dir);

// ---------- Direction non-member functions

Dir negx(const Dir& dir);
Dir negy(const Dir& dir);
Dir negxy(const Dir& dir);

Dirs dirPerms(const Dir& dir);
Dirs dirSigns(const Dir& dir);
Dirs dirSignedPerms(const Dir& dir);

// ========================================
// Position

struct Pos
{
    Pos(Col x, Row y) : x{x}, y{y} {}
    Pos(Short index) : Pos(div(index, BOARD_COLS)) {}
    Pos(div_t d) : x(d.rem), y(d.quot) {}
    Pos(const Pos& pos) : x(pos.x), y(pos.y) {}
    Pos(const std::string& posStr);  // For testing

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

    const std::string algNotation() const;
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

    friend std::ostream& operator<<(std::ostream& os, const Pos& pos);
};
