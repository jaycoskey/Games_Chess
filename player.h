// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"
#include "geometry.h"

using std::string;
using std::map;


class Board;

// TODO: Add user interactivity, including asking Players for their names
class Player {
public:
    static const string& playerName(Color color) { return _color2PlayerName.at(color); }
    static void setPlayerName(Color color, const string& name) { _color2PlayerName[color] = name; }

    static const Dir& getForward(Color color) { return _color2Forward.at(color); }

private:
    static map<Color, string> _color2PlayerName;
    static const map<Color, Dir> _color2Forward;
};

map<Color, string> Player::_color2PlayerName{ {Color::Black, "Amy"}, {Color::White, "Bob"} };
const map<Color, Dir> Player::_color2Forward{ {Color::Black, Dir{0, -1}}, {Color::White, Dir{0, 1}} };
