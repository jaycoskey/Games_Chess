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
using std::array, std::map, std::set, std::vector;

using Moves   = vector<Move>;

using CaptureRule = std::function<bool(const Board&, const Piece& attacker, const Piece& target)>;
using MoveRule = std::function<Moves(const Board&, Color, const Pos&)>;
using OptPieceP = std::optional<std::shared_ptr<Piece>>;
using PieceP   = std::shared_ptr<Piece>;

using PiecePs = set<PieceP>;
using Players = vector<Player>;

using Color2KingP   = map<Color, PieceP>;
using Color2PiecePs = map<Color, PiecePs>;
using Pos2Moves     = map<Pos, Moves>;
using Pos2PieceP    = map<Pos, PieceP>;

using PieceType2CaptureRule = map<PieceType, CaptureRule>;
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
    Board();
    Board(const Board& other);

    bool containsPos(Pos pos) const;

    void addPiecePair(PieceType pieceType, Short index);
    void addPieceTo(Color color, PieceType pieceType, Short index);
    void addPiecePTo(PieceP pieceP, Pos to);
    PieceP removePieceFrom(Pos pos);

    OptPieceP pieceAt(Pos pos) const;
    OptPieceP pieceAt(Col col, Row row) const;
    OptPieceP pieceAt(Short index) const;
    const PiecePs& piecesWithColor(Color color) const;

    void movePiece(const Pos& from, const Pos& to);

    const PieceP& getKingP(Color color) const { return _color2KingP.at(color); }

    Hash zobristBitstring(int cInd, int ptInd) { return Board::_zobristTable[cInd][ptInd]; }

    Color2PiecePs color2PiecePs;

private:
    static Short getZIndex(Color color) { return Board::_color2ZIndex.at(color); }
    static Short getZIndex(PieceType pt) { return Board::_pieceType2ZIndex.at(pt); }

    static Color2ZIndex     _color2ZIndex;
    static PieceType2ZIndex _pieceType2ZIndex;
    static ZTable           _zobristTable;

    void   _initPieces();
    bool   _isAtPromotionPos(const Piece&) const;
    string _show() const;
    float  _valuation() const;

    Pos2PieceP    _pos2PieceP;
    Color2KingP   _color2KingP;

    friend struct std::hash<Board>;
    friend ostream& operator<<(ostream& os, const Board& board);
    friend struct std::hash<Board>;
};

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
            for (PieceType pieceType : pieceTypes) {
                Short indPt = _pieceType2ZIndex.at(pieceType);
                zt[indClr][indPt] = random_bitstring();
            }
        }
        return zt;
    }()
};

ostream& operator<<(ostream& os, const Board& board)
{
    os << board._show();
    return os;
}

// ---------- public Board members

Board::Board() {
    _initPieces();
}

Board::Board(const Board& other)
    : color2PiecePs{other.color2PiecePs}
    , _pos2PieceP{other._pos2PieceP}
    {}

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
    _pos2PieceP[pos] = pieceP;
    if (pieceType == PieceType::King) { _color2KingP[color] = pieceP; }
}

void Board::movePiece(const Pos& from, const Pos& to)
{
    Piece& piece = *(pieceAt(from)->get());
    _pos2PieceP[to] = _pos2PieceP[from];
    _pos2PieceP.erase(from);
}

void Board::addPiecePTo(PieceP pieceP, Pos to)
{
    pieceP->moveTo(to);
    color2PiecePs[pieceP->color()].insert(pieceP);
    _pos2PieceP[to] = pieceP;
}

PieceP Board::removePieceFrom(Pos pos)
{
    OptPieceP oPieceP = pieceAt(pos);
    assert(oPieceP != std::nullopt);
    PieceType pieceType = oPieceP->get()->pieceType();
    assert(pieceType != PieceType::King);
    const PieceP pieceP = _pos2PieceP.at(pos);
    color2PiecePs[pieceP->color()].erase(pieceP);
    _pos2PieceP.erase(pos);
    return pieceP;
}

OptPieceP Board::pieceAt(Pos pos) const
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

const PiecePs& Board::piecesWithColor(Color color) const
{
    return color2PiecePs.at(color);
}

float Board::_valuation() const {
    return 0.0;
}

// ---------- private Board members

void Board::_initPieces() {
    for (Color color: {Color::Black, Color::White}) {

        color2PiecePs[color] = PiecePs();
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
        for (Short index : pawn_indexes)   { addPiecePair(PieceType::Pawn,   index); }
    }
}

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
                    int cInd  = Board::getZIndex(oPiece->get()->color());
                    int ptInd = Board::getZIndex(oPiece->get()->pieceType());
                    result = result ^ Board::_zobristTable[cInd][ptInd];
                }
            }
            return result;
        }
    };
}
