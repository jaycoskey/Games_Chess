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

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "util.h"
#include "geometry.h"


// ---------- Board index inversion

Row homeRow(Color c) { return c == Color::Black ? BOARD_ROWS - 1 : 0; }
Short invertIndex(Short index) { return BOARD_SPACES - 1 - index; }
Short invertRow(Short index) {
    return BOARD_SPACES - 1 - index
        + 2 * (index % BOARD_COLS) - (BOARD_COLS - 1);
}

// ========================================
// Direction

const Dirs Dir::orthoDirs() {
    static const Dirs& result = dirSignedPerms(Dir(1,0));
    return result;
}

const Dirs Dir::diagDirs() {
    static const Dirs& result = dirSigns(Dir(1,1));
    return result;
}

const Dirs Dir::allDirs() {
    static Dirs result = getUnion(Dir::orthoDirs(), Dir::diagDirs());
    return result;
}

const Dirs Dir::knightDirs() {
    static const Dirs& result = dirSignedPerms(Dir(1,2));
    return result;
}

std::ostream& operator<<(std::ostream& os, const Dir& dir)
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

// ========================================
// Position

// ---------- Pos constructor
Pos::Pos(const std::string& posStr)
    : x{static_cast<Short>(tolower(posStr[0]) - 'a')}
    , y{static_cast<Short>(stoi(posStr.substr(1)) - 1)}
{}  // For testing

// ---------- Pos read methods
const std::string Pos::algNotation() const
{
    std::ostringstream oss;

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

std::ostream& operator<<(std::ostream& os, const Pos& pos)
{
    os << pos.algNotation();
    return os;
}
