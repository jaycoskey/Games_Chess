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
