// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>
#include <numeric>

#include "util.h"
#include "geometry.h"
#include "piece.h"

class Move;
class Game;

using std::string;
using std::cout, std::ostream, std::ostringstream;
using std::map, std::set, std::vector;

using Moves   = vector<Move>;

using MoveRule = std::function<Moves(const Board&, Color, const Pos&)>;
using OptPiece = std::optional<std::shared_ptr<Piece>>;
using PieceP   = std::shared_ptr<Piece>;

using PiecePs = set<PieceP>;
using Players = vector<Player>;

using Color2PiecePs      = map<Color, PiecePs>;
using PieceType2MoveRule = map<PieceType, MoveRule>;
using Pos2Moves          = map<Pos, Moves>;
using Pos2PieceP         = map<Pos, PieceP>;


// TODO: Consider including storage of captured pieces for Undo
// TODO: Privacy
class Board {
public:
    Board();

    bool containsPos(Pos pos) const;

    void addPiecePair(PieceType pieceType, Short index);
    void addPieceTo(Color color, PieceType pieceType, Short index);
    void addPieceTo(PieceP pieceP, Pos to);
    PieceP removePieceFrom(Pos pos);

    OptPiece pieceAt(Pos pos) const;
    OptPiece pieceAt(Col col, Row row) const;
    const PiecePs& piecesWithColor(Color color) const;
    Hash zobristHash() const;

private:
    void   initPieces();
    bool   isAtLastRow(Piece) const;
    string show() const;
    float  valuation() const;

    Color2PiecePs color2PiecePs;
    Pos2PieceP    pos2PieceP;

    friend struct std::hash<Board>;
    friend ostream& operator<<(ostream& os, const Board& board);
};

ostream& operator<<(ostream& os, const Board& board)
{
    os << board.show();
    return os;
}

// ---------- public Board members

Board::Board() {
    initPieces();
}

bool Board::containsPos(Pos pos) const
{
    return pos.x >= 0 && pos.x < BOARD_COLS
        && pos.y >= 0 && pos.y < BOARD_ROWS;
}

void Board::addPiecePair(PieceType pieceType, Short index)
{
    addPieceTo(Color::White, pieceType, index);
    addPieceTo(Color::Black, pieceType, invertIndex(index));
}

void Board::addPieceTo(Color color, PieceType pieceType, Short index)
{
    auto pieceP = std::make_shared<Piece>(color, pieceType, index);
    color2PiecePs[color].insert(pieceP);
    Pos pos{index};
    pos2PieceP[pos] = pieceP;
}

void Board::addPieceTo(PieceP pieceP, Pos to)
{
    pieceP->setPos(to);
    color2PiecePs[pieceP->color()].insert(pieceP);
    pos2PieceP[to] = pieceP;
}

PieceP Board::removePieceFrom(Pos pos)
{
    const PieceP pieceP = pos2PieceP.at(pos);
    color2PiecePs[pieceP->color()].erase(pieceP);
    pos2PieceP.erase(pos);
    return pieceP;
}

OptPiece Board::pieceAt(Col col, Row row) const {
    return pieceAt(Pos(col, row));
}

OptPiece Board::pieceAt(Pos pos) const
{
    if (pos2PieceP.find(pos) == pos2PieceP.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(pos2PieceP.at(pos));
    }
}

const PiecePs& Board::piecesWithColor(Color color) const
{
    return color2PiecePs.at(color);
}

// TODO: Implement Zobrist hash function, below. See Wikipedia.
Hash Board::zobristHash() const
{
    return Hash();  // TODO: std::hash<Board>{}(*this);
}

// TODO
float Board::valuation() const {
    return 0.0;
}

// ---------- private Board members

void Board::initPieces() {
    // TODO: assert that board is uninitialized
    for (Color color: {Color::Black, Color::White}) {
        cout << "JMC: Initializing pieces for Color: " << showColor(color) << "\n";

        color2PiecePs[color] = set<PieceP>();
        int king_indexes[]   = {4};
        int queen_indexes[]  = {3};
        int rook_indexes[]   = {0, 7};
        int bishop_indexes[] = {2, 5};
        int knight_indexes[] = {1, 6};
        int pawn_indexes[]   = {8, 9, 10, 11, 12, 13, 14, 15};

        for (Short index : king_indexes)   { addPiecePair(PieceType::King,   index); }
        for (Short index : queen_indexes)  { addPiecePair(PieceType::Queen,  index); }
        for (Short index : rook_indexes)   { addPiecePair(PieceType::Rook,   index); }
        for (Short index : bishop_indexes) { addPiecePair(PieceType::Bishop, index); }
        for (Short index : knight_indexes) { addPiecePair(PieceType::Knight, index); }
        // TODO: for (Short index : pawn_indexes)   { addPiecePair(PieceType::Pawn,   index); }
    }
}

string Board::show() const
{
    ostringstream oss;

    string hRule{1, '+'};
    hRule += repeatString("--+", BOARD_COLS);

    oss << hRule << "\n";
    for (Row row = BOARD_ROWS - 1; row >= 0; --row) {
        oss << '|';
        for (Col col = 0; col < BOARD_COLS; ++col) {
            OptPiece op = pieceAt(col, row);
            if (op == std::nullopt) {
                oss << "  ";
            } else {
                Color color = op->get()->color();
                PieceType pieceType = op->get()->pieceType();
                oss << showColor(color) << showPieceType(pieceType);
            }
            oss << '|';
        }
        oss << "\n" << hRule << "\n";
    }
    return oss.str();
}

// ---------- Board specialization of std::hash

namespace std {
    template<> struct std::hash<Board> {
        // TODO: Replace with implementation of Zobrist hashing. Current impl does not guarantee uniqueness.
        Hash operator()(const Board& board) const noexcept
        {
            static int primes[] = {
                  2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
                 73,  79,  83,  89,  97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
                179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
                283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
                419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541
            };

            Hash result = 0;
            for (auto const& [color, piecePs] : board.color2PiecePs) {
                for (auto const& pieceP : piecePs) {
                    auto color_id      = static_cast<Hash>(to_underlying(color));
                    auto piece_type_id = static_cast<Hash>(to_underlying(pieceP->pieceType()));
                    auto pos_index     = static_cast<Hash>(pieceP->pos().index());

                    result += primes[pos_index + 10] * primes[color_id + 80] * primes[piece_type_id + 90];
                }
            }
            return result;
        }
    };
}
