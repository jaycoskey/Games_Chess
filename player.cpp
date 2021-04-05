// Copyright 2021, by Jay M. Coskey

#include <iostream>
#include <string>

#include "util.h"
#include "geometry.h"
#include "player.h"

using std::string;
using std::cin, std::cout;


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
