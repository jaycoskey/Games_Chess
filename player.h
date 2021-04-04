// Copyright 2021, by Jay M. Coskey

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "util.h"
#include "geometry.h"

using std::string;
using std::cin, std::cout;
using std::map;

using Color2Dir = map<Color, Dir>;
using Color2Name = map<Color, string>;

enum class PlayerType
    { Human
    , Computer_Random
    , Computer_RandomCapture
    };

using Color2PlayerType = map<Color, PlayerType>;


class Player {
public:
    // ---------- Static read methods
    static const Dir& backward(Color c);
    static const Dir& forward(Color c);

    static string playerName(Color c) { return _color2PlayerName.at(c); }
    static PlayerType playerType(Color c) { return _color2PlayerType.at(c); }

    static bool offerBool(std::optional<Color> oc, const string& offerMsg);

    // ---------- Static write methods
    static void setPlayerName(Color c, string name) { _color2PlayerName[c] = name; }
    static void setPlayerType(Color c, PlayerType pt) { _color2PlayerType[c] = pt; }

private:
    static Color2Name       _color2PlayerName;
    static Color2PlayerType _color2PlayerType;
};

// ---------- Static data initialization
Color2Name Player::_color2PlayerName
    { {Color::Black, "Basho"}
    , {Color::White, "Wilma"}
    };

Color2PlayerType Player::_color2PlayerType
    { {Color::Black, PlayerType::Computer_Random}
    , {Color::White, PlayerType::Computer_Random}
    };

// ---------- Static read methods
const Dir& Player::backward(Color c)
{
    static const Color2Dir c2b
        { {Color::Black, Dir{0,  1}}
        , {Color::White, Dir{0, -1}}
        };
    return c2b.at(c);
}

const Dir& Player::forward(Color c)
{
    static const Color2Dir c2f
        { {Color::Black, Dir{0, -1}}
        , {Color::White, Dir{0,  1}}
        };
    return c2f.at(c);
}

bool Player::offerBool(std::optional<Color> oc, const string& offerMsg)
{
    if (oc != std::nullopt) {
        cout << Player::playerName(*oc) << ": ";
    }
    cout << offerMsg;
    while (true) {
        string input;
        getline(cin, input);
        if (input.size() != 1) {
            cout << "Please enter simply 'y' or 'n'.\n";
            continue;
        }
        if (input != "y" && input != "n") {
            continue;
        }
        return input == "y";
    }
}
