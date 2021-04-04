// Copyright 2021, by Jay M. Coskey

#include <iostream>
#include <vector>

#include <libgen.h>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"
#include "move.h"
#include "game_state.h"
#include "game.h"

using std::string;
using std::cerr, std::cout;
using std::map, std::vector;


int main(int argc, char **argv) {
    string progname{basename(argv[0])};
    vector<string> args(argv + 1, argv + argc);
    PlayerType bPlayer = PlayerType::Computer_Random;
    PlayerType wPlayer = PlayerType::Computer_Random;
    Short matchGameCount = 5;  // 0 represents unlimited
    string helpMsg =
        "A chess platform that supports Human (H) vs Computer (C) play, H vs. H, or C vs. C.\n"
        "  Options:\n"
        "    -1 <player1_type>, where <player1_type> is human, random, or randomCapture\n"
        "    -2 <player2_type>, where <player2_type> is human, random, or randomCapture\n"
        "    -n <games_count>,  to set the number of games in a match\n"
        "                       (default is 5 for batch play; unlimited for interactive play)\n"
        "So, for example,\n"
        "    % chess -1 human -2 human\n"
        "plays an unlimited number of games between two humans.\n"
        "\n"
        "For further details, see the README file, or enter '?' or 'help' "
            "when playing interactively.\n"
        ;
    bool isArgParsingError = false;
    bool isMatchGameCountSpecified = false;

    map<string, PlayerType> s2pt{
        {"human", PlayerType::Human}
        , {"random", PlayerType::Computer_Random}
        , {"randomCapture", PlayerType::Computer_RandomCapture}
    };
    for (auto i = args.begin(); i != args.end(); ++i) {
        if (*i == "-1" || *i == "-w") {
            ++i;
            string& player1 = *i;
            if (s2pt.find(player1) == s2pt.end()) {
                cerr << progname << ": Unrecognized player type: " << player1 << "\n";
                isArgParsingError = true;
            } else {
                wPlayer = s2pt.at(player1);
            }
            continue;
        } else if (*i == "-2" || *i == "-bw") {
            ++i;
            string& player2 = *i;
            if (s2pt.find(player2) == s2pt.end()) {
                cerr << progname << ": Unrecognized player type: " << player2 << "\n";
                isArgParsingError = true;
            } else {
                bPlayer = s2pt.at(player2);
            }
            continue;
        } else if (*i == "-n") {
            ++i;
            try {
                matchGameCount = std::stoi(*i);
                isMatchGameCountSpecified = true;
            }
            catch (std::invalid_argument& ex) {
                cerr << progname << ": " << ex.what();
                isArgParsingError = true;
            }
            continue;
        } else {
            cerr << progname << ": Unrecognized argument: " << *i << "\n";
            isArgParsingError = true;
        }
    }
    if (isArgParsingError) {
        cout << progname << ": " << helpMsg;
        exit(1);
    }
    if (!isMatchGameCountSpecified) {
        if (bPlayer == PlayerType::Human && wPlayer == PlayerType::Human) {
            matchGameCount = 0;  // Unlimited
        } else {
            matchGameCount = 5;
        }
    }

    logger.setReportLevel(LogError);
    // logger.setReportLevel(LogTrace);
    logger.logToCout();
    // logger.logToFile("foo.txt");

    Game game{};
    game.play(matchGameCount, wPlayer, bPlayer);
}
