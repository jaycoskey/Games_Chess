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

#include <map>
#include <string>

#include "util.h"
#include "geometry.h"

using Color2Dir = std::map<Color, Dir>;
using Color2Name = std::map<Color, std::string>;


enum class PlayerType
    { Human
    , Computer_Random
    , Computer_RandomCapture
    };

using Color2PlayerType = std::map<Color, PlayerType>;


class Player {
public:
    // ---------- Static read methods
    static const Dir& backward(Color c);
    static const Dir& forward(Color c);

    static std::string playerName(Color c) { return _color2PlayerName.at(c); }
    static PlayerType playerType(Color c) { return _color2PlayerType.at(c); }

    static bool offerBool(std::optional<Color> oc, const std::string& offerMsg);

    // ---------- Static write methods
    static void setPlayerName(Color c, std::string name) { _color2PlayerName[c] = name; }
    static void setPlayerType(Color c, PlayerType pt) { _color2PlayerType[c] = pt; }

private:
    static Color2Name       _color2PlayerName;
    static Color2PlayerType _color2PlayerType;
};
