// Copyright 2021, by Jay M. Coskey

#pragma once

#include <regex>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"

using std::string, std::ostream;
using std::cin, std::cout, std::ostringstream;


enum class CheckType {
    NotCheck,
    Check,
    CheckMate
};

// Types of moves/move steps/actions in chess:
//   - Move primary piece
//   - Move secondary piece (when castling)
//   - Remove captured piece from board (destination of move, except in en passant)
//   - Exchange piece with another (when promoting a Pawn)

enum class MoveType {
    Simple,
    CastleK,
    CastleQ,
    EnPassant,
    Promotion
};

// TODO: Expand to include MoveSteps, as described below
// TODO: Privacy
class Move {
public:
    Move(Color color, PieceType pieceType, Pos from, Pos to)
        : _color{color}, _pieceType{pieceType}, _from{from}, _to{to}
        // TODO: _checkType, _captured, _isPromotion
        {}
    Color      color()      const { return _color;       }
    PieceType  pieceType()  const { return _pieceType;   }
    const Pos& from()       const { return _from;        }
    const Pos& to()         const { return _to;          }
    CheckType  checkType()  const { return _checkType;   }

    const PieceP captured() const { return _captured;    }
    bool isPromotion()      const { return _isPromotion; }

    bool operator==(const Move& other) const;

private:
    Color     _color;
    PieceType _pieceType;
    Pos       _from;
    Pos       _to;
    CheckType _checkType;

    PieceP    _captured;
    bool      _isPromotion;

    bool operator<(const Move& other) const;

    friend ostream& operator<<(ostream& os, const Move& move);
};

ostream& operator<<(ostream& os, const Move& move) {
    os << "(from=" << move._from << ", to=" << move._to << ')';
    return os;
}

bool Move::operator==(const Move& other) const
{
    return _color == other._color
        && _pieceType == other._pieceType
        && _from == other._from
        && _to == other._to;
}

// ---------- private Move members

bool Move::operator<(const Move& other) const
{
    if (  _from < other._from
       || (_from.x == other._from.x && _from.y == other._from.y && _to < other._to)
       )  { return true; }
    return false;
}

// ---------- Move non-member functions

const Move parseMoveInAlgNotation(const Board& board, const string& input);

const Move getPlayerMove(
    const Board& board
    , const Player& player
    , const Pos2Moves& validPlayerMoves)
{
    const string& help   = "\tEnter moves as a pair of board positions in algebraic notation. E.g.: e2 e4";
    const string& prompt = "Enter move (? for help): ";
    cout << player.name() << "'s turn\n";

    while (true) {
        cout << prompt;
        try {
            string input;
            getline(cin, input);
            if (input[0] == '?') {
                cout << help << "\n";
                continue;
            }
            if (input == "list") {
                cout << "List of valid moves\n";
                for (const auto& [from, moves] : validPlayerMoves) {
                    PieceType pt = board.pieceAt(from)->get()->pieceType();
                    cout << "  Moves of " << showPieceType(pt)
                         << '@' <<  from << " (count=" << moves.size() << "):\n";
                    for (const Move& move : moves) {
                        cout << "    " << move << "\n";
                    }
                }
                continue;
            }
            Move move = parseMoveInAlgNotation(board, input);
            cout << "Getting valid piece moves: from=" << move.from() << "\n";
            Moves validPieceMoves = validPlayerMoves.at(move.from());
            const auto& beg = validPieceMoves.begin();
            const auto& end = validPieceMoves.end();
            if (find(beg, end, move) == end) {
                cout << "That is not a legal move." << "\n";
                continue;
            }
            // TODO: Check to see if player is putting self in check.
            return move;
        }
        catch (const std::invalid_argument& ex) {
            cout << ex.what();
        }
    }
}

const Pos2Moves getValidPlayerMoves(
    const PieceType2MoveRule& pieceType2MoveRule
    , const Board& board
    , const Color& color
    )
{
    Pos2Moves result;
    for (const PieceP& pieceP : board.piecesWithColor(color)) {
        PieceType pt = pieceP->pieceType();
        MoveRule moveRule = pieceType2MoveRule.at(pt);
        // cout << "Finding legal moves for " << *pieceP << "\n";
        for (Move move : moveRule(board, pieceP->color(), pieceP->pos())) {
            auto& moves = result[pieceP->pos()];
            moves.insert(moves.end(), move);
        }
    }
    return result;
}

Moves move_leaps(const Board& board, Color color, const Pos& pos, PieceType pieceType, const Dirs& dirs)
{
    bool verbose = false;

    Moves result{};
    for (const Dir& dir : dirs) {
        Pos dest(pos + dir);
        verbose && cout << "leaps(" << showPieceType(pieceType) << "): "
                        << "Dest test: " << dest << "\n";
        if (!board.containsPos(dest)) {
            verbose && cout << "leaps(" << showPieceType(pieceType) << "): "
                            << "Dest rejected: " << dest << " Not on board\n";
            continue;
        }
        auto oPieceAtDestP = board.pieceAt(dest);
        if (oPieceAtDestP != std::nullopt) {
            if (oPieceAtDestP->get()->color() == color) {
                verbose && cout << "leaps(" << showPieceType(pieceType) << "): "
                                << "Dest rejected: " << dest << " Piece belongs to same player\n";
                continue;  // Cannot capture one's own piece
            }
        }
        verbose && cout << "leaps: Dest is legal: " << dest << "\n";
        result.emplace(result.end(), color, pieceType, pos, pos+dir);
    }
    return result;
}

Moves move_slides( const Board& board, Color color, Pos pos, PieceType pieceType, Dirs dirs
            , Short maxSteps=100)
{
    Moves result{};
    for (Dir dir : dirs) {
        Pos dest{pos};
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            if (!board.containsPos(dest)) {
                break;  // Done stepping in this direction
            }
            auto oPieceAtDestP = board.pieceAt(dest);
            if (oPieceAtDestP != std::nullopt) {
                if (oPieceAtDestP->get()->color() == color) {
                    break;  // Cannot capture one's own piece
                }
                // cout << "INFO: slides: Found capture @ " << dest << " by " << showPieceType(pieceType) << "\n";
                result.emplace(result.end(), color, pieceType, pos, dest);  // Capture
                break;  // Cannot move past opponent's piece
            } else {
                // cout << "INFO: slides: Found non-capture @ " << dest << " by " showPieceType(pieceType) << "\n";
                result.emplace(result.end(), color, pieceType, pos, dest);  // No capture
            }
        }
    }
    return result;
}

Moves move_steps(const Board& board, Color color, Pos pos, PieceType pieceType, Dirs dirs)
{
    return move_leaps(board, color, pos, pieceType, dirs);
}

const Move parseMoveInAlgNotation(const Board& board, const string& input) noexcept(false)
{
    std::regex halvesRegex{"^\\s*(\\S+)\\s+(\\S+)$"};
    std::regex algNotationRegex{"^\\s*([a-z]+)([1-9][0-9]*)$"};

    std::smatch halvesMatch;
    Pos *fromP;
    Pos *toP;
    if (std::regex_match(input, halvesMatch, halvesRegex)) {
        if (halvesMatch.size() != 3) {
            throw new std::invalid_argument("Input has the wrong format");
        }
        for (unsigned int k = 1; k < halvesMatch.size(); ++k) {
            string half = halvesMatch[k];
            string halfName = k == 1 ? "From" : "To";
            std::smatch algNotationMatch;
            if (std::regex_match(half, algNotationMatch, algNotationRegex)) {
                string colStr = algNotationMatch[1];
                string rowStr = algNotationMatch[2];

                int col = tolower(colStr[0]) - 'a';
                if (col < 0 or col >= BOARD_COLS) {
                    ostringstream oss;
                    oss << halfName << " position has an illegal column number: " << col;
                    throw new std::invalid_argument(oss.str());
                }

                int row = stoi(rowStr);
                if (row < 1 or row > BOARD_ROWS) {
                    ostringstream oss;
                    oss << halfName << " position has an illegal row number: " << row;
                    throw new std::invalid_argument(oss.str());
                }
                row -= 1;  // Adjust for difference between alg chess notation and C++ indexing
                if (k == 1) { fromP = new Pos(col, row); }
                else { toP = new Pos(col, row); }
            } else {
                ostringstream oss;
                oss << halfName << " position has the wrong format";
                throw new std::invalid_argument(oss.str());
            }
        }
    } else {
        throw new std::invalid_argument("Input has the wrong format");
    }
    const Piece& fromPiece = *(board.pieceAt(*fromP)->get());
    return Move{fromPiece.color(), fromPiece.pieceType(), *fromP, *toP};
}
