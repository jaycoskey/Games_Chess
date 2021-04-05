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

#include <vector>

#include "util.h"
// #include "geometry.h"
#include "player.h"
// #include "piece.h"
#include "board.h"
#include "move.h"
#include "game.h"

using std::cout, std::ostringstream;
using std::vector;


void Game::printConciseMatchSummary(vector<GameState>& gss)
{
    std::map<GameState, Short> gsHistogram{};
    for (GameState& gs : gss) {
        if (gsHistogram.find(gs) == gsHistogram.end()) {
            gsHistogram[gs] = 1;
        } else {
            gsHistogram[gs] += 1;
        }
    }
    for (auto [gs, count] : gsHistogram) {
        cout << "\t"
             << std::setw(5) << std::left << std::setfill(' ')
             << count << "instances: " << gs;
    }
}

void Game::printVerboseMatchSummary(const vector<GameState>& gss)
{
    for (Short k = 0; (unsigned long) k < gss.size(); ++k) {
        cout << "\tGame #" << k + 1 << ". " << gss[k];
    }
}

// ---------- Constructor
Game::Game()
{
    // Init board not needed---use default layout
    _initPlayers();
}

// ---------- Public write methods
const GameState Game::gameLoop()
{
    _reset();
    int  move_num = 0;
    Color c = Color::Black;
    GameState result;

    while (true) {
        move_num++;
        c = opponent(c);

        cout << "Turn #" << _board.currentMoveIndex() << " (" << to_string(c) << "):\n";
        cout << _board;

        _board.updateBoardHashHistory(c);
        const Pos2Moves& validPlayerMoves
            = _validPlayerMovesCache.size() > 0
                  ? _validPlayerMovesCache  // Cached from end of prev turn
                  : Move::getValidPlayerMoves(_board, c)
                  ;

        ExtMove extMove
            = Move::getPlayerMove(Player::playerType(c), _board, c, validPlayerMoves);

        if (extMove.optMove == std::nullopt) {
            if (extMove.isDrawClaim) {
                // Pre-verified Draw condition is claimed
                DrawFlags drawFlags = Draw_None;

                if (_board.maxBoardRepetitionCount(c) >= 3) {
                    drawFlags |= Draw_Claimed_3xRepetition;
                } else if (_board.movesSinceLastPmoc() >= 50) {
                    drawFlags |= Draw_Claimed_50MoveRule;
                }
                result = GameState{ GameEnd::Draw
                                  , WinType::None
                                  , drawFlags
                                  };
                break;
            } else {
                // Game ended by agreement
                GameEnd agreedGameEnd = extMove.agreedGameEnd;
                WinType winType = WinType::None;
                if (  (agreedGameEnd == GameEnd::WinBlack && c == Color::White)
                   || (agreedGameEnd == GameEnd::WinWhite && c == Color::Black)
                   )
                {
                    winType = WinType::Conceding;
                } else {
                    assert(  (agreedGameEnd == GameEnd::WinBlack && c == Color::Black)
                          || (agreedGameEnd == GameEnd::WinWhite && c == Color::White)
                          );
                    winType = WinType::Agreement;
                }
                result = GameState{ agreedGameEnd
                                  , winType
                                  , agreedGameEnd == GameEnd::Draw
                                      ? Draw_Agreement : Draw_None
                                  };
                break;
            }
        } else {
            // A Piece was moved
            const Move& move = *extMove.optMove;
            ostringstream oss;
            oss << _board.currentMoveIndex() << '.';
            cout << std::setw(4) << std::left << std::setfill(' ')
                 << oss.str() << " Moved: " << move << "\n";
            cout << "-------------------------\n";
            move.apply(_board);

            // Determine GameState from board
            _validPlayerMovesCache  // Cached for beginning of next turn
                = Move::getValidPlayerMoves(_board, opponent(c));
            result = GameState{_board, c, extMove.isDrawClaim, _validPlayerMovesCache};

            Move::prevMove().setCheck(result.isCheck());
            Move::prevMove().setCheckmate(result.isCheckmate());
            if (result.gameEnd() == GameEnd::InPlay) {
                continue;
            }
            break;
        }
    }
    _announceGameEnd(result);
    return result;
}

void Game::play(Short autoReplayCount, PlayerType wPlayer, PlayerType bPlayer)
{
    vector<GameState> gss;
    Player::setPlayerType(Color::White, wPlayer);
    Player::setPlayerType(Color::Black, bPlayer);

    for ( Short gameNum = 1
        ; autoReplayCount == 0 || gameNum <= autoReplayCount
        ; ++gameNum
        )
    {
        cout << "=========================\n";
        ostringstream oss;
        oss << gameNum << '.';
        cout << "Game #" << std::setw(5) << std::left << std::setfill(' ')
             << oss.str() << "\n";
        gss.push_back(gameLoop());
        _printGameStats();
        if (autoReplayCount == 0) {
            if (!Player::offerBool(std::nullopt, "Play again (y/n)? ")) {
                cout << "Thanks for playing. Bye!\n";
                break;
            }
        }
    }

    cout << "Match records (" << gss.size() << " games):\n";
    // Game::printVerboseMatchSummary(gss);
    Game::printConciseMatchSummary(gss);
}

// ---------- Private read members
void Game::_announceGameEnd(const GameState& gs) const
{
    assert(gs.gameEnd() != GameEnd::InPlay);
    cout << "Final board layout:\n";
    cout << _board;  // Board inserts final newline.
    cout << "Game over: " << gs << "\n";
}

void Game::_printGameStats() const
{
    // Print game stats after game end
    cout << "Move history (custom):\n\t" << Move::getMoveHistory() << "\n";
    cout << "Move history (verbose input PGN):\n\t" << Move::history_to_pgn() << "\n";
    _board.printBoardHashRepetitions();
    cout << "Moves since last Pawn move or capture:\n\t"
         << _board.movesSinceLastPmoc() << "\n";
}

// ---------- Private write members
void Game::_initPlayers()
{
}

void Game::_reset()
{
    _board = Board{true};
    _validPlayerMovesCache.clear();
    Move::reset();
}
