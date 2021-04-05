// Copyright 2021, by Jay M. Coskey

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
