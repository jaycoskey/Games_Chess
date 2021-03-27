// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>
#include <numeric>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"

#include "logger.h"

class Move;

using std::string;
using std::cout, std::ostream, std::ostringstream;
using std::array, std::map, std::set, std::vector;

using Players = vector<Player>;

// ---------- Piece-related using aliases
using PieceP    = std::shared_ptr<Piece>;
using OptPieceP = std::optional<PieceP>;
using PieceData = std::tuple<Color, PieceType, Short>;
using PiecePs   = set<PieceP>;

// ---------- Move-related using aliases
using Moves   = vector<Move>;
using IsAttackingRule = std::function<bool(const Board& b, const Piece& attacker, const Pos& tgtPos, const Color c)>;
using MoveRule = std::function<Moves(const Board&, Color, const Pos&)>;

// ---------- Board-related using aliases
using Color2KingP   = map<Color, PieceP>;
using Color2PiecePs = map<Color, PiecePs>;
using Pos2Moves     = map<Pos, Moves>;
using Pos2PieceP    = map<Pos, PieceP>;

using PieceType2IsAttackingRule = map<PieceType, IsAttackingRule>;
using PieceType2MoveRule    = map<PieceType, MoveRule>;

using Color2ZIndex     = map<Color, int>;
using PieceType2ZIndex = map<PieceType, int>;
using ZTable           = vector<vector<Hash>>;


// TODO: Consider including storage of captured pieces for Undo.
// TODO: Make color2PiecePs map private, replace in public API w/ piece iterator.
// TODO: On AI implementation, add a sensible valuation function.
// TODO: Before initializing Board, assert that it is uninitialized.
class Board {
public:
    static bool containsPos(Pos pos);

    static Pos kInitPos(Color c)     { return Pos(4, 0).fromRel(c); }
    static Pos kRookInitPos(Color c) { return Pos(7, 0).fromRel(c); }
    static Pos qRookInitPos(Color c) { return Pos(0, 0).fromRel(c); }

    Hash zobristBitstring(int cInd, int ptInd) { return Board::_zobristTable[cInd][ptInd]; }

    // ---------- Constructors
    Board();
    Board(const Board& other);
    Board(const vector<PieceData>& layout);  // For testing. Might later support loading saved game.

    // ---------- Piece data - read
    OptPieceP pieceAt(const Pos& pos) const;
    OptPieceP pieceAt(Col col, Row row) const;
    OptPieceP pieceAt(Short index) const;

    bool isEmpty(Pos pos) const { return pieceAt(pos) == std::nullopt; }
    const Piece& king(Color c) const { return *_color2KingP.at(c); }
    const PiecePs& piecesWithColor(Color c) const;

    // ---------- Piece data - write
    // void addPiecePTo(PieceP pieceP, const Pos& to);
    void addPieceTo(Color c, PieceType pt, Short index);
    void addPiecePair(PieceType pt, Short index, bool preserveCol=false);
    void movePiece(const Pos& from, const Pos& to);
    PieceP removePieceFrom(const Pos& pos);

    // ---------- Board data - read
    float boardValue() const;
    Short currentMoveIndex() const { return _currentMoveIndex; }
    Short movesSinceLastPawnMoveOrCapture() const;

    // ---------- Board data - write
    void currentMoveIndex_decr() { _currentMoveIndex--; }
    void currentMoveIndex_incr() { _currentMoveIndex++; }

    Color2PiecePs color2PiecePs;  // TODO: Make private & add iterator to public API.

    // ---------- Debugging
    void reportStatusAt(const Pos& pos) const;
    void listPieces() const;

private:
    static Short _getZIndex(Color c) { return Board::_color2ZIndex.at(c); }
    static Short _getZIndex(PieceType pt) { return Board::_pieceType2ZIndex.at(pt); }

    static Color2ZIndex     _color2ZIndex;
    static PieceType2ZIndex _pieceType2ZIndex;
    static ZTable           _zobristTable;

    void   _initPieces();
    bool   _isAtPromotionPos(const Piece&) const;

    Short       _lastPawnMoveOrCaptureMoveIndex=1;
    Short       _currentMoveIndex;
    Pos2PieceP  _pos2PieceP;
    Color2KingP _color2KingP;

    string _show() const;  // Used by operator<<
    friend struct std::hash<Board>;
    friend ostream& operator<<(ostream& os, const Board& board);
};

// ---------- Initialization of static data
Color2ZIndex Board::_color2ZIndex
    = { {Color::Black, 0}, {Color::White, 1} };

PieceType2ZIndex Board::_pieceType2ZIndex
    = { {PieceType::King,   0}
      , {PieceType::Queen,  1}
      , {PieceType::Rook,   2}
      , {PieceType::Bishop, 3}
      , {PieceType::Knight, 4}
      , {PieceType::Pawn,   5}
      };

ZTable Board::_zobristTable{ [] ()
    {
        ZTable zt;
        zt.reserve(colors.size());
        for (Color color : colors) {
            Short indClr = _color2ZIndex.at(color);
            zt[indClr].reserve(pieceTypes.size());
            for (PieceType pt : pieceTypes) {
                Short indPt = _pieceType2ZIndex.at(pt);
                zt[indClr][indPt] = random_bitstring();
            }
        }
        return zt;
    }()
};

// ---------- Board - public static methods

bool Board::containsPos(Pos pos)
{
    return pos.x >= 0 && pos.x < BOARD_COLS
        && pos.y >= 0 && pos.y < BOARD_ROWS;
}

// ---------- Board - Constructors

Board::Board()
    :_currentMoveIndex{1}
{
    _initPieces();
}

Board::Board(const Board& other)
    : color2PiecePs{other.color2PiecePs}
    , _currentMoveIndex{1}
    , _pos2PieceP{other._pos2PieceP}
{}

// For custom board layouts, including testing.
Board::Board(const vector<PieceData>& layout)
    : _currentMoveIndex{1}
{
    for (const PieceData& pd : layout) {
        addPieceTo(std::get<0>(pd), std::get<1>(pd), std::get<2>(pd));
    }
}

// ---------- Piece data - read

OptPieceP Board::pieceAt(const Pos& pos) const
{
    if (_pos2PieceP.find(pos) == _pos2PieceP.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(_pos2PieceP.at(pos));
    }
}

OptPieceP Board::pieceAt(Col col, Row row) const {
    return pieceAt(Pos(col, row));
}

OptPieceP Board::pieceAt(Short index) const {
    return pieceAt(Pos(index % BOARD_COLS, index / BOARD_COLS));
}

const PiecePs& Board::piecesWithColor(Color c) const
{
    return color2PiecePs.at(c);
}

// ---------- Piece data - write

void Board::addPieceTo(Color color, PieceType pt, Short index)
{
    auto pieceP = std::make_shared<Piece>(color, pt, index);
    color2PiecePs[color].insert(pieceP);
    Pos pos{index};
    _pos2PieceP[pos] = pieceP;
    if (pt == PieceType::King) { _color2KingP[color] = pieceP; }
}

void Board::addPiecePair(PieceType pt, Short index, bool preserveCol /*=false*/)
{
    addPieceTo(Color::White, pt, index);
    Short blackIndex = preserveCol ? invertRow(index) : invertIndex(index);
    addPieceTo(Color::Black, pt, blackIndex);
}

void Board::movePiece(const Pos& from, const Pos& to)
{
    OptPieceP oPieceP = pieceAt(from);
    assert (oPieceP != std::nullopt);
    PieceP pieceP = *pieceAt(from);
    pieceP->moveTo(to);
    _pos2PieceP[to] = pieceP;
    _pos2PieceP.erase(from);
}

PieceP Board::removePieceFrom(const Pos& pos)
{
    OptPieceP oPieceP = pieceAt(pos);
    assert(oPieceP != std::nullopt);
    PieceType pt = oPieceP->get()->pieceType();
    assert(pt != PieceType::King);
    const PieceP pieceP = _pos2PieceP.at(pos);
    color2PiecePs[pieceP->color()].erase(pieceP);
    _pos2PieceP.erase(pos);
    return pieceP;
}

void Board::_initPieces() {
    // color2PiecePs[color] = PiecePs();
    int king_indexes[]   = {4};
    int queen_indexes[]  = {3};
    int rook_indexes[]   = {0, 7};
    int bishop_indexes[] = {2, 5};
    int knight_indexes[] = {1, 6};
    int pawn_indexes[]   = {8, 9, 10, 11, 12, 13, 14, 15};

    for (Short index : king_indexes)   { addPiecePair(PieceType::King,   index, true); }
    for (Short index : queen_indexes)  { addPiecePair(PieceType::Queen,  index, true); }
    for (Short index : rook_indexes)   { addPiecePair(PieceType::Rook,   index); }
    for (Short index : bishop_indexes) { addPiecePair(PieceType::Bishop, index); }
    for (Short index : knight_indexes) { addPiecePair(PieceType::Knight, index); }
    for (Short index : pawn_indexes)   { addPiecePair(PieceType::Pawn,   index); }
}

// ---------- Board data - read

// TODO: Implement
float Board::boardValue() const {
    return 0.0;
}

Short Board::movesSinceLastPawnMoveOrCapture() const {
    return _currentMoveIndex - _lastPawnMoveOrCaptureMoveIndex;
}

// void Board::addPiecePTo(PieceP pieceP, const Pos& to)
// {
//     pieceP->moveTo(to);
//     color2PiecePs[pieceP->color()].insert(pieceP);
//     _pos2PieceP[to] = pieceP;
// }

// ---------- Debugging

void Board::listPieces() const {
    for (const auto& [c, piecePs] : color2PiecePs) {
        cout << "Piece with color " << c << ":\n";
        for (const PieceP& pieceP : piecePs) {
            cout << "\t" << *pieceP << "\n";
        }
    }
}

void Board::reportStatusAt(const Pos& pos) const {
    OptPieceP oPieceP = pieceAt(pos);
    if (oPieceP == std::nullopt) {
        cout << "Position " << pos << " is empty.\n";
    } else {
        cout << "Position " << pos << " contains " << *oPieceP->get() << "\n";
    }
}


// ---------- Board specialization of std::hash

namespace std
{
    template<> struct std::hash<Board>
    {
        Hash operator()(const Board& board) const noexcept
        {
            // Zobrist hashing
            Hash result = 0;
            for (Short index = 0; index < BOARD_COLS * BOARD_ROWS; ++index) {
                OptPieceP oPiece = board.pieceAt(index);
                if (oPiece != std::nullopt) {
                    int cInd  = Board::_getZIndex(oPiece->get()->color());
                    int ptInd = Board::_getZIndex(oPiece->get()->pieceType());
                    result = result ^ Board::_zobristTable[cInd][ptInd];
                }
            }
            return result;
        }
    };
}

// ---------- Custom printing

string Board::_show() const
{
    ostringstream oss;

    string hRule{1, '+'};
    hRule += repeatString("--+", BOARD_COLS);

    oss << hRule << "\n";
    for (Row row = BOARD_ROWS - 1; row >= 0; --row) {
        oss << '|';
        for (Col col = 0; col < BOARD_COLS; ++col) {
            OptPieceP op = pieceAt(col, row);
            if (op == std::nullopt) {
                oss << "  ";
            } else {
                Color c = op->get()->color();
                PieceType pt = op->get()->pieceType();
                oss << c << pt;
            }
            oss << '|';
        }
        oss << "\n" << hRule << "\n";
    }
    return oss.str();
}

ostream& operator<<(ostream& os, const Board& b)
{
    os << b._show();
    return os;
}
