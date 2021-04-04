// Copyright 2021, by Jay M. Coskey
#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <numeric>
#include <string>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"

#include "logger.h"

using std::string;
using std::cout, std::ostream, std::ostringstream;
using std::array, std::map, std::set, std::vector;

using Players = vector<Player>;

// ---------- Piece-related aliases
using PieceData  = std::tuple<Color, PieceType, Short>;
using PieceP     = std::shared_ptr<Piece>;
using PiecePs    = set<PieceP>;
using PieceTypes = vector<PieceType>;

// ---------- Move-related aliases
class Move;

using MoveIndexSet      = set<MoveIndex>;
using MoveIndexVec      = vector<MoveIndex>;
using Hash2MoveIndexSet = map<Hash, MoveIndexSet>;

using Moves             = vector<Move>;
using IsAttackingRule   = std::function<bool(const Board& b, const Piece& attacker, const Pos& tgtPos)>;
using MoveRule          = std::function<Moves(const Board&, Color, const Pos&)>;

// ---------- Board-related aliases
using Color2KingP       = map<Color, PieceP>;
using Color2PiecePs     = map<Color, PiecePs>;
using Pos2PieceP        = map<Pos, PieceP>;

using PieceType2IsAttackingRule = map<PieceType, IsAttackingRule>;
using PieceType2MoveRule        = map<PieceType, MoveRule>;

using ZIndex = int;
using ZTable = array< array<Hash, COLORS_COUNT * PIECE_TYPES_COUNT>
                    , BOARD_COLS * BOARD_ROWS
                    >;

using BoardHashHistory  = map<Color, Hash2MoveIndexSet>;
using VecBool = vector<bool>;

// ----------

enum class GameEnd {
    InPlay,
    Draw,
    WinBlack,
    WinWhite
};

enum class WinType {
    None,
    Agreement,
    Checkmate,
    Conceding
    // Clock - No clock used
};

ostream& operator<<(ostream& os, WinType wt)
{
    static const map<WinType, const char*> wt2s{
        { WinType::Agreement, "agreement" }
        , { WinType::Checkmate, "checkmate" }
        , { WinType::Conceding, "conceding" }
    };
    os << wt2s.at(wt);
    return os;
}

// Note: The FIDE laws for the 50 Move Rule mention "50 moves by each player",
//       but the law is interpreted as meaning 50 moves total.
enum DrawableFlag {
    Drawable_None         = 0,
    Drawable_3xRepetition = 1 << 0,
    Drawable_50MoveRule   = 1 << 1
};

enum DrawFlag {
    Draw_None                  = 0,
    Draw_5xRepetition          = 1 << 0,
    Draw_75MoveRule            = 1 << 1,
    Draw_Agreement             = 1 << 2,
    Draw_Claimed_3xRepetition  = 1 << 3,
    Draw_Claimed_50MoveRule    = 1 << 4,
    // Draw_Clock,
    // Draw_DeadPosition,  // Not including InsufficientResources
    Draw_InsufficientResources = 1 << 5,
    Draw_Stalemate             = 1 << 6
};

// ----------

using DrawableFlags = int;
using DrawFlags     = int;

// ----------

class Board {
public:
    static bool containsPos(const Pos& pos);

    static Pos kInitPos(Color c)     { return Pos(BOARD_KING_COL, homeRow(c)); }
    static Pos kRookInitPos(Color c) { return Pos(7, homeRow(c)); }
    static Pos qRookInitPos(Color c) { return Pos(0, homeRow(c)); }

    static Hash zobristBitstring(int cInd, int ptInd) {
        return Board::_zobristTable[cInd][ptInd];
    }

    // ---------- Constructors
    Board(bool doPopulate=false);
    Board(const Board& other);

    // ---------- Cell / Piece data - read
    PieceP     pieceAt(const Pos& pos)   const;
    PieceP     pieceAt(Col col, Row row) const;
    PieceP     pieceAt(Short index)      const;

    bool isEmpty(const Pos& pos) const { return !pieceAt(pos); }
    bool isEmpty(Col col, Row row) const { return !pieceAt(col, row); }
    const Piece& king(Color c) const { return *_color2KingP.at(c); }

    // ---------- Piece data - write
    // void addPiecePTo(PieceP pieceP, const Pos& to);
    void addPieceTo(Color c, PieceType pt, Short index, Short lastMoveIndex=0);
    void addPieceTo(Color c, PieceType pt, const string& posStr, Short lastMoveIndex=0);
    void addPiecePair(PieceType pt, Short index, bool preserveCol=false);
    void movePiece(const Pos& from, const Pos& to);
    PieceTypes pieceTypes(Color c) const;
    const PiecePs& piecesWithColor(Color c) const;
    void removePieceAt(const Pos& pos);

    // ---------- Board data - read
    float boardValue() const;
    float boardValue(Color c) const;
    Short currentMoveIndex() const { return _currentMoveIndex; }
    bool hasInsufficientResources() const;
    std::size_t maxBoardRepetitionCount(Color c) const;
    Short movesSinceLastPmoc() const;
    Short pieceCount(Color c) const { return color2PiecePs.at(c).size(); }
    Short pieceCount() const { return pieceCount(Color::Black) + pieceCount(Color::White); }

    void printBoardHashRepetitions() const;
    void printBoardHashRepetitions(Color c) const;
    void printPieces() const;

    // ---------- Board data - write
    void currentMoveIndex_decr() { _currentMoveIndex--; }
    void currentMoveIndex_incr() { _currentMoveIndex++; }

    void initPieces();

    void rollBackBoardHashHistory(Color c);
    void rollBackPmocHistory();

    void updateBoardHashHistory(Color c);
    void updatePmocHistory(bool isPawnMoveOrCapture);

    Color2PiecePs color2PiecePs;

    // ---------- Testing / Debugging
    void test_assert_pmocHistory_size() const { assert(_pmocHistory.size() < 10'000); }

    static void test_printZobristTable() {
        for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
            for (Short iPiece = 0; iPiece < COLORS_COUNT * PIECE_TYPES_COUNT; ++iPiece) {
            cout << "ZobristTable[" << std::setw(2) << iBoard << ']'
                 <<             '[' << std::setw(2) << iPiece << "] = 0x"
                 << std::hex << std::setfill('0') << std::setw(16)
                 << _zobristTable[iBoard][iPiece]
                 << std::dec << "\n";
            }
        }
    }
    void test_reportStatusAt(const Pos& pos) const;

private:
    static ZIndex _getZIndex(Color c) {
        static map<Color, ZIndex> c2zi
                   { { Color::Black, 0}
                   , { Color::White, 1}
                   };
        return c2zi.at(c);
    }
    static ZIndex _getZIndex(PieceType pt) {
        static map<PieceType, ZIndex> pt2zi
                   { { PieceType::King,   0 }
                   , { PieceType::Queen,  1 }
                   , { PieceType::Rook,   2 }
                   , { PieceType::Bishop, 3 }
                   , { PieceType::Knight, 4 }
                   , { PieceType::Pawn,   5 }
                   };
        return pt2zi.at(pt);
    }
    static ZIndex _getZIndex(PieceP pp) {
        return _getZIndex(pp->color()) * allPieceTypes.size()
                   + _getZIndex(pp->pieceType());
    }

    static ZTable _zobristTable;

    Color2KingP _color2KingP;
    Pos2PieceP  _pos2PieceP;

    // ---------- History
    MoveIndex _currentMoveIndex;  // 1-based. Matches popular notion of turn number.
    BoardHashHistory _boardHashHistory;  // updateBoardHashHistory & maxBoardRepetitionCount
    VecBool _pmocHistory;       // updatePmocHistory      & movesSinceLastPmoc

    friend struct std::hash<Board>;
    friend ostream& operator<<(ostream& os, const Board& board);
public:
    string test_to_string() const;  // For testing/debugging
};

// ---------- Initialization of static data
ZTable Board::_zobristTable{ [] ()
    {
        ZTable zt;
        for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
            for (Short iPiece = 0; iPiece < COLORS_COUNT * PIECE_TYPES_COUNT; ++iPiece) {
                logger.trace( "Board::_zobristTable: Setting  "
                            , "ZobristTable[", iBoard, "][", iPiece, ']'
                            );
                zt[iBoard][iPiece] = random_bitstring();
            }
        }
        return zt;
    }()
};

// ---------- Board specialization of std::hash

namespace std
{
    template<> struct std::hash<Board>
    {
        Hash operator()(const Board& b) const noexcept
        {
            // Zobrist hashing
            Hash result = 0;
            for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
                const PieceP& pieceP = b.pieceAt(iBoard);
                if (pieceP) {
                    int iPiece = Board::_getZIndex(pieceP);
                    result = result ^ Board::_zobristTable[iBoard][iPiece];
                }
            }
            return result;
        }
    };
}

// ---------- Board - Public static methods

bool Board::containsPos(const Pos& pos)
{
    return pos.x >= 0 && pos.x < BOARD_COLS
        && pos.y >= 0 && pos.y < BOARD_ROWS;
}

// ---------- Board - Constructors

Board::Board(bool doPopulate)
    : color2PiecePs{}
    , _color2KingP{}
    , _pos2PieceP{}
    , _currentMoveIndex{1}
    , _boardHashHistory{}
    , _pmocHistory{1}
{
    assert(_pmocHistory.size() < 10'000);
    if (doPopulate) { initPieces(); }
}

Board::Board(const Board& other)
    : color2PiecePs{other.color2PiecePs}
    , _color2KingP{other._color2KingP}
    , _pos2PieceP{other._pos2PieceP}
    , _currentMoveIndex{1}
    , _boardHashHistory{other._boardHashHistory}
    , _pmocHistory{other._pmocHistory}
{
    assert(_pmocHistory.size() < 10'000);
}

// For custom board layouts, including testing.
// Board::Board(const vector<PieceData>& layout)
//     : _currentMoveIndex{1}
// {
//     for (const PieceData& pd : layout) {
//         addPieceTo(std::get<0>(pd), std::get<1>(pd), std::get<2>(pd));
//     }
// }

// ---------- Piece data - read


PieceP Board::pieceAt(const Pos& pos) const
{
    if (_pos2PieceP.find(pos) == _pos2PieceP.end()) {
        return nullptr;
    } else {
        return _pos2PieceP.at(pos);
    }
}

PieceP Board::pieceAt(Col col, Row row) const {
    return pieceAt(Pos(col, row));
}

PieceP Board::pieceAt(Short index) const {
    return pieceAt(Pos(index % BOARD_COLS, index / BOARD_COLS));
}

void Board::addPieceTo(Color c, PieceType pt, Short index, Short lastMoveIndex /* =0 */)
{
    auto pieceP = std::make_shared<Piece>(c, pt, index, lastMoveIndex);

    // Workaround: Not initialized by make_shared
    pieceP->updateMoveIndexHistory(lastMoveIndex);

    color2PiecePs[c].insert(pieceP);
    Pos pos{index};
    _pos2PieceP[pos] = pieceP;
    if (pt == PieceType::King) {
        _color2KingP[c] = pieceP;
    }
}

void Board::addPieceTo(Color c, PieceType pt, const string& posStr, Short lastMoveIndex /* =0 */)
{
    addPieceTo(c, pt, Pos{posStr}.index(), lastMoveIndex);
}

void Board::addPiecePair(PieceType pt, Short index, bool preserveCol /*=false*/)
{
    addPieceTo(Color::White, pt, index);
    Short blackIndex = preserveCol ? invertRow(index) : invertIndex(index);
    addPieceTo(Color::Black, pt, blackIndex);
}

void Board::movePiece(const Pos& from, const Pos& to)
{
    logger.trace("Board::movePiece: Entering. from =", from, ", to=", to);
    assert(pieceAt(from));
    assert(!pieceAt(to));  // When capturing, apply() removes the captured piece beforehand.
    PieceP pieceP = pieceAt(from);
    pieceP->moveTo(to);
    _pos2PieceP[to] = pieceP;
    assert(_pos2PieceP.find(from) != _pos2PieceP.end());  // Is in map
    _pos2PieceP.erase(from);
    assert(_pos2PieceP.find(from) == _pos2PieceP.end());  // Is not in map
    logger.trace("Board::movePiece: Exiting: from=", from, ", to=", to);
}

PieceTypes Board::pieceTypes(Color c) const {
    PieceTypes pieceTypes;
    std::transform( color2PiecePs.at(c).begin()
                  , color2PiecePs.at(c).end()
                  , std::back_inserter(pieceTypes)
                  , [](auto& pp) { return pp->pieceType(); }
                  );
    std::sort( pieceTypes.begin(), pieceTypes.end()
             , [](PieceType pta, PieceType ptb)
                 { return Piece::pieceValue(pta) > Piece::pieceValue(ptb); }
             );
    return pieceTypes;
}

const PiecePs& Board::piecesWithColor(Color c) const
{
    return color2PiecePs.at(c);
}

void Board::removePieceAt(const Pos& pos)
{
    PieceP pieceP = pieceAt(pos);
    assert(pieceP);
    PieceType pt = pieceP.get()->pieceType();
    assert(pt != PieceType::King);
    Color c = pieceP->color();
    color2PiecePs[c].erase(pieceP);
    {
        const auto& beg = color2PiecePs.at(c).begin();
        const auto& end = color2PiecePs.at(c).end();
        assert(std::find(beg, end, pieceP) == end);
    }
    _pos2PieceP.erase(pos);
    assert(!pieceAt(pos));
    // return pieceP;
}

// ---------- Board data - read
float Board::boardValue() const
{
   return boardValue(Color::Black) - boardValue(Color::White);
}

float Board::boardValue(Color c) const
{
    return std::accumulate(
               color2PiecePs.at(c).begin(), color2PiecePs.at(c).end(), 0.0
               , [&](float a, const PieceP b) { return a + Piece::pieceValue(b->pieceType()); }
               );
}

bool Board::hasInsufficientResources() const
{
    static PieceTypes pts_KR
        = vector{PieceType::King, PieceType::Rook};
    static PieceTypes pts_KRB
        = vector{PieceType::King, PieceType::Rook, PieceType::Bishop};
    static PieceTypes pts_KRN
        = vector{PieceType::King, PieceType::Rook, PieceType::Knight};
    static PieceTypes pts_KB
        = vector{PieceType::King, PieceType::Bishop};
    static PieceTypes pts_KN
        = vector{PieceType::King, PieceType::Knight};
    static PieceTypes pts_KNN
        = vector{PieceType::King, PieceType::Knight, PieceType:: Knight};

    PieceTypes pts[2] = { pieceTypes(Color::Black)
                        , pieceTypes(Color::White)
                        };
    PieceTypes::size_type counts[2] = { pts[0].size(), pts[1].size() };
    if (counts[0] > 3 || counts[1] > 3) { return false; }

    // Player-symmetric
    if (counts[0] == 1 && counts[1] == 1) {
        return true;
    }

    // Player-asymmetric
    for (int i = 0; i < 2; ++i) {
        int j = 1 - i;

        if (counts[i] == 1 && counts[j] == 2) {
            if (pts[j] == pts_KB || pts[j] == pts_KN) {
                return true;
            }
        }
        if (counts[i] == 1 && counts[j] == 3) {
            if (pts[j] == pts_KNN) {
                return true;
            }
        }
        if (counts[i] == 2 && counts[j] == 2) {
            if (pts[i] == pts_KR && (pts[j] == pts_KB || pts[j] == pts_KN)) {
                return true;
            }
        }
        if (counts[i] == 2 && counts[j] == 3) {
            if (pts[i] == pts_KR && (pts[j] == pts_KRB || pts[j] == pts_KRN)) {
                return true;
            }
        }
    }

    if (counts[0] == 2 && counts[1] == 2) {
        // Player-symmetric: Two Bishops on the same color square
        if (pts[0] == pts_KB && pts[1] == pts_KB) {
            vector<PieceP> bps{};
            vector<PieceP> wps{};

            for (const auto& pp : color2PiecePs.at(Color::Black)) {
                bps.push_back(pp);
            }
            for (const auto& pp : color2PiecePs.at(Color::White)) {
                wps.push_back(pp);
            }

            const PieceP& bbp = bps[0]->pieceType() == PieceType::Bishop
                                    ? bps[0] : bps[1];
            const PieceP& wbp = wps[0]->pieceType() == PieceType::Bishop
                                    ? wps[0] : wps[1];
            if (bbp->squareColor() == wbp->squareColor()) {
                return true;
            }
        }
    }
    return false;
}

std::size_t Board::maxBoardRepetitionCount(Color c) const {
    if (_boardHashHistory.find(c) == _boardHashHistory.end()) {
        return 0;
    }
    const Hash2MoveIndexSet& h2mi = _boardHashHistory.at(c);
    vector<MoveIndex> repCounts{};
    std::transform( h2mi.begin(), h2mi.end(), std::back_inserter(repCounts)
                  , [](const pair<const Hash, MoveIndexSet>& hmi)
                      { return hmi.second.size(); }
                  );
    return repCounts.size() == 0 ? 0 : *std::max_element(repCounts.begin(), repCounts.end());
}

MoveIndex Board::movesSinceLastPmoc() const {
    assert(_pmocHistory.size() <= 10'000);
    auto found = std::find(_pmocHistory.rbegin(), _pmocHistory.rend(), true);
    return _currentMoveIndex - std::distance(found, _pmocHistory.rend());
}

void Board::printBoardHashRepetitions() const {
    printBoardHashRepetitions(Color::Black);
    printBoardHashRepetitions(Color::White);
}

void Board::printBoardHashRepetitions(Color c) const {
    cout << "Color: " << (c == Color::Black ? "Black" : "White") << ":\n";
    bool foundRepetition = false;
    for (const auto& [h, moveIndexes] : _boardHashHistory.at(c)) {
        if (moveIndexes.size() > 1) {
            foundRepetition = true;
            cout << "\tHash: 0x"
                 << std::hex << std::setfill('0') << std::setw(16) << h
                 << " - " << std::dec << moveIndexes.size()
                 << " - " << moveIndexes << "\n";
        }
    }
    if (!foundRepetition) {
        cout << "\tNo board hash repetitions\n";
    }
}

void Board::printPieces() const {
    for (const auto& [c, piecePs] : color2PiecePs) {
        cout << "Pieces with color " << c << '(' << piecePs.size() << "):\n";
        for (const PieceP& pieceP : piecePs) {
            cout << "\t" << *pieceP << "\n";
        }
    }
}

// ---------- Board data - write
void Board::initPieces() {
    // board.erase();

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

void Board::rollBackBoardHashHistory(Color c) {
    Hash h = std::hash<Board>{}(*this);
    _boardHashHistory[c][h].erase(_currentMoveIndex);
}

void Board::rollBackPmocHistory() {
    // TODO: Determine what causes this to be called with empty history.
    if (_pmocHistory.size() > 1) {
        _pmocHistory.pop_back();
    }
}

void Board::updateBoardHashHistory(Color c) {
    Hash h = std::hash<Board>{}(*this);
    _boardHashHistory[c][h].insert(_currentMoveIndex);
}

void Board::updatePmocHistory(bool isPawnMoveOrCapture) {
    assert(_pmocHistory.size() == (unsigned long) _currentMoveIndex);
    _pmocHistory.push_back(isPawnMoveOrCapture);
}

// ---------- Custom printing
ostream& operator<<(ostream& os, const Board& b)
{
    string hRule{1, '+'};
    hRule += repeatString("--+", BOARD_COLS);

    os << hRule << "\n";
    for (Row row = BOARD_ROWS - 1; row >= 0; --row) {
        os << '|';
        for (Col col = 0; col < BOARD_COLS; ++col) {
            PieceP op = b.pieceAt(col, row);
            if (op) {
                Color c = op.get()->color();
                PieceType pt = op.get()->pieceType();
                os << c << pt;
            } else {
                os << "  ";
            }
            os << '|';
        }
        os << "\n" << hRule << "\n";
    }
    return os;
}

// ---------- Testing / debugging
bool operator==(const Board& lhs, const Board& rhs)
{
    for (Col col = 0; col < BOARD_COLS; ++col) {
        for (Row row = 0; row < BOARD_ROWS; ++row) {
            const PieceP& opp1 = lhs.pieceAt(col, row);
            const PieceP& opp2 = rhs.pieceAt(col, row);
            if (!opp1 && !opp2) {
                continue;
            }
            if (!opp1 || !opp2) {
                return false;
            }
            const Piece& p1 = *(opp1.get());
            const Piece& p2 = *(opp2.get());
            if (p1.color() == p2.color() && p1.pieceType() == p2.pieceType()) {
                continue;
            } else {
                return false;
            }
        }
    }
    return true;
}

void Board::test_reportStatusAt(const Pos& pos) const {
    PieceP pieceP = pieceAt(pos);
    if (!pieceP) {
        cout << "Position " << pos << " is empty.\n";
    } else {
        cout << "Position " << pos << " contains " << *pieceP.get() << "\n";
    }
}

string Board::test_to_string() const {
    ostringstream oss;
    oss << *this;
    return oss.str();
}
