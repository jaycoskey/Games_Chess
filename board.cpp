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

#include <map>
#include <string>

#include "geometry.h"
#include "util.h"
// #include "player.h"
#include "board.h"
#include "piece.h"

#include "logger.h"

using std::cout, std::ostream, std::ostringstream;
using std::map, std::vector;
using std::string;

// ========================================
// enum class WinType

ostream &operator<<(ostream &os, WinType wt) {
    static const map<WinType, const char *> wt2s{
        {WinType::Agreement, "agreement"},
        {WinType::Checkmate, "checkmate"},
        {WinType::Conceding, "conceding"}
    };
    os << wt2s.at(wt);
    return os;
}

// ========================================
// class Board

// ---------- Initialization of static data
ZTable Board::_zobristTable{[]() {
    ZTable zt;
    for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
        for (Short iPiece = 0; iPiece < COLORS_COUNT * PIECE_TYPES_COUNT;
             ++iPiece) {
            Logger::trace("Board::_zobristTable: Setting  ", "ZobristTable[",
                          iBoard, "][", iPiece, ']');
            zt[iBoard][iPiece] = random_bitstring();
        }
    }
    return zt;
}()};

// ---------- Board specialization of std::hash

namespace std {
template <> struct hash<Board> {
    Hash operator()(const Board &b) const noexcept {
        // Zobrist hashing
        Hash result = 0;
        for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
            const PieceP &pieceP = b.pieceAt(iBoard);
            if (pieceP) {
                int iPiece = Board::_getZIndex(pieceP);
                result = result ^ Board::_zobristTable[iBoard][iPiece];
            }
        }
        return result;
    }
};
} // namespace std

// ---------- Board - Public static methods

// ---------- Board - Constructors

Board::Board(bool doPopulate)
    : color2PiecePs{}, _color2KingP{}, _pos2PieceP{}, _currentMoveIndex{1},
      _boardHashHistory{}, _pmocHistory{1}
{
    assert(_pmocHistory.size() < 10'000);
    if (doPopulate) {
        initPieces();
    }
}

Board::Board(const Board &other)
    : color2PiecePs{other.color2PiecePs},
    _color2KingP{other._color2KingP},
    _pos2PieceP{other._pos2PieceP},
    _currentMoveIndex{1},
    _boardHashHistory{other._boardHashHistory},
    _pmocHistory{other._pmocHistory}
{
    assert(_pmocHistory.size() < 10'000);
}

// ---------- Piece data - read

PieceP Board::pieceAt(const Pos &pos) const {
    if (_pos2PieceP.find(pos) == _pos2PieceP.end()) {
        return nullptr;
    } else {
        return _pos2PieceP.at(pos);
    }
}

PieceP Board::pieceAt(Col col, Row row) const { return pieceAt(Pos(col, row)); }

PieceP Board::pieceAt(Short index) const {
    return pieceAt(Pos(index % BOARD_COLS, index / BOARD_COLS));
}

void Board::addPieceTo(Color c, PieceType pt, Short index,
                       Short lastMoveIndex /* =0 */)
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

void Board::addPieceTo(Color c, PieceType pt, const string &posStr,
                       Short lastMoveIndex /* =0 */)
{
    addPieceTo(c, pt, Pos{posStr}.index(), lastMoveIndex);
}

void Board::addPiecePair(PieceType pt, Short index,
                         bool preserveCol /*=false*/)
{
    addPieceTo(Color::White, pt, index);
    Short blackIndex = preserveCol ? invertRow(index) : invertIndex(index);
    addPieceTo(Color::Black, pt, blackIndex);
}

void Board::movePiece(const Pos &from, const Pos &to) {
    Logger::trace("Board::movePiece: Entering. from =", from, ", to=", to);
    assert(pieceAt(from));
    assert(!pieceAt(to)); // Captured piece has been removed by apply()
    PieceP pieceP = pieceAt(from);
    pieceP->moveTo(to);
    _pos2PieceP[to] = pieceP;
    assert(_pos2PieceP.find(from) != _pos2PieceP.end()); // Is in map
    _pos2PieceP.erase(from);
    assert(_pos2PieceP.find(from) == _pos2PieceP.end()); // Is not in map
    Logger::trace("Board::movePiece: Exiting: from=", from, ", to=", to);
}

PieceTypes Board::pieceTypes(Color c) const {
    PieceTypes pieceTypes;
    std::transform(color2PiecePs.at(c).begin(), color2PiecePs.at(c).end(),
                   std::back_inserter(pieceTypes),
                   [](auto &pp) { return pp->pieceType(); }
                   );
    std::sort(pieceTypes.begin(), pieceTypes.end(),
              [](PieceType pta, PieceType ptb) {
                  return Piece::pieceValue(pta) > Piece::pieceValue(ptb);
              }
              );
    return pieceTypes;
}

const PiecePs &Board::piecesWithColor(Color c) const {
    return color2PiecePs.at(c);
}

void Board::removePieceAt(const Pos &pos) {
    PieceP pieceP = pieceAt(pos);
    assert(pieceP);
    PieceType pt = pieceP.get()->pieceType();
    assert(pt != PieceType::King);
    Color c = pieceP->color();
    color2PiecePs[c].erase(pieceP);
    {
        const auto &beg = color2PiecePs.at(c).begin();
        const auto &end = color2PiecePs.at(c).end();
        assert(std::find(beg, end, pieceP) == end);
    }
    _pos2PieceP.erase(pos);
    assert(!pieceAt(pos));
    // return pieceP;
}

// ---------- Board data - read
float Board::boardValue() const {
    return boardValue(Color::Black) - boardValue(Color::White);
}

float Board::boardValue(Color c) const {
    return std::accumulate(color2PiecePs.at(c).begin(),
                           color2PiecePs.at(c).end(), 0.0,
                           [&](float a, const PieceP b) {
                               return a + Piece::pieceValue(b->pieceType());
                           }
                           );
}

bool Board::hasInsufficientResources() const {
    static PieceTypes pts_KR = vector{PieceType::King, PieceType::Rook};
    static PieceTypes pts_KRB =
        vector{PieceType::King, PieceType::Rook, PieceType::Bishop};
    static PieceTypes pts_KRN =
        vector{PieceType::King, PieceType::Rook, PieceType::Knight};
    static PieceTypes pts_KB = vector{PieceType::King, PieceType::Bishop};
    static PieceTypes pts_KN = vector{PieceType::King, PieceType::Knight};
    static PieceTypes pts_KNN =
        vector{PieceType::King, PieceType::Knight, PieceType::Knight};

    PieceTypes pts[2] = {pieceTypes(Color::Black), pieceTypes(Color::White)};
    PieceTypes::size_type counts[2] = {pts[0].size(), pts[1].size()};
    if (counts[0] > 3 || counts[1] > 3) {
        return false;
    }

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

            for (const auto &pp : color2PiecePs.at(Color::Black)) {
                bps.push_back(pp);
            }
            for (const auto &pp : color2PiecePs.at(Color::White)) {
                wps.push_back(pp);
            }

            const PieceP &bbp =
                bps[0]->pieceType() == PieceType::Bishop ? bps[0] : bps[1];
            const PieceP &wbp =
                wps[0]->pieceType() == PieceType::Bishop ? wps[0] : wps[1];
            if (bbp->squareColor() == wbp->squareColor()) {
                return true;
            }
        }
    }
    return false;
}

// Find the largest number of times a Board position/Hash has occurred for this
// Color.
std::size_t Board::maxBoardRepetitionCount(Color c) const {
    if (_boardHashHistory.find(c) == _boardHashHistory.end()) {
        return 0;
    }
    std::size_t result = 0;
    const Hash2MoveIndexes &h2mi = _boardHashHistory.at(c);
    for (auto const &kv : h2mi) {
        if (kv.second.size() > result) {
            result = kv.second.size();
        }
    }
    return result;
}

// Moves since last Pawn move or capture. Used to determine Draw from lack of
// progress.
MoveIndex Board::movesSinceLastPmoc() const {
    assert(_pmocHistory.size() <= 10'000);
    auto found = std::find(_pmocHistory.rbegin(), _pmocHistory.rend(), true);
    return _currentMoveIndex - std::distance(found, _pmocHistory.rend());
}

void Board::printBoardHashRepetitions() const {
    printBoardHashRepetitions(Color::Black);
    printBoardHashRepetitions(Color::White);
}

// For each Board repetition, print how many times it has recurred.
void Board::printBoardHashRepetitions(Color c) const {
    cout << "Color: " << (c == Color::Black ? "Black" : "White") << ":\n";
    bool foundRepetition = false;
    for (const auto &[h, moveIndexes] : _boardHashHistory.at(c)) {
        if (moveIndexes.size() > 1) {
            foundRepetition = true;
            cout << "\tHash: 0x" << std::hex << std::setfill('0')
                 << std::setw(16) << h << " - " << std::dec
                 << moveIndexes.size() << " - " << moveIndexes << "\n";
        }
    }
    if (!foundRepetition) {
        cout << "\tNo board hash repetitions\n";
    }
}

// Print a list of Pieces still on the Board.
void Board::printPieces() const {
    for (const auto &[c, piecePs] : color2PiecePs) {
        cout << "Pieces with color " << c << '(' << piecePs.size() << "):\n";
        for (const PieceP &pieceP : piecePs) {
            cout << "\t" << *pieceP << "\n";
        }
    }
}

// ---------- Board data - write

// Set up standard Board layout.
void Board::initPieces() {
    // board.erase();

    int king_indexes[] = {4};
    int queen_indexes[] = {3};
    int rook_indexes[] = {0, 7};
    int bishop_indexes[] = {2, 5};
    int knight_indexes[] = {1, 6};
    int pawn_indexes[] = {8, 9, 10, 11, 12, 13, 14, 15};

    for (Short index : king_indexes) {
        addPiecePair(PieceType::King, index, true);
    }
    for (Short index : queen_indexes) {
        addPiecePair(PieceType::Queen, index, true);
    }
    for (Short index : rook_indexes) {
        addPiecePair(PieceType::Rook, index);
    }
    for (Short index : bishop_indexes) {
        addPiecePair(PieceType::Bishop, index);
    }
    for (Short index : knight_indexes) {
        addPiecePair(PieceType::Knight, index);
    }
    for (Short index : pawn_indexes) {
        addPiecePair(PieceType::Pawn, index);
    }
}

// To support undo, remove record of the last Board Hash and the MoveIndex when
// it occurred.
void Board::rollBackBoardHashHistory(Color c) {
    Hash h = std::hash<Board>{}(*this);
    _boardHashHistory[c][h].erase(_currentMoveIndex);
}

// To support undo, remove the last Move from the Pawn move or capture history.
void Board::rollBackPmocHistory() {
    // TODO: Determine what causes this to be called with empty history.
    if (_pmocHistory.size() > 1) {
        _pmocHistory.pop_back();
    }
}

// To support Draw conditions, record the current Board hash and the current
// MoveIndex.
void Board::updateBoardHashHistory(Color c) {
    Hash h = std::hash<Board>{}(*this);
    _boardHashHistory[c][h].insert(_currentMoveIndex);
}

// To support Draw conditions, record whether this MoveIndex has a Pawn move or
// capture.
void Board::updatePmocHistory(bool isPawnMoveOrCapture) {
    assert(_pmocHistory.size() == (unsigned long)_currentMoveIndex);
    _pmocHistory.push_back(isPawnMoveOrCapture);
}

// ---------- Custom printing
ostream &operator<<(ostream &os, const Board &b) {
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
bool operator==(const Board &lhs, const Board &rhs) {
    for (Col col = 0; col < BOARD_COLS; ++col) {
        for (Row row = 0; row < BOARD_ROWS; ++row) {
            const PieceP &opp1 = lhs.pieceAt(col, row);
            const PieceP &opp2 = rhs.pieceAt(col, row);
            if (!opp1 && !opp2) {
                continue;
            }
            if (!opp1 || !opp2) {
                return false;
            }
            const Piece &p1 = *(opp1.get());
            const Piece &p2 = *(opp2.get());
            if (p1.color() == p2.color() && p1.pieceType() == p2.pieceType()) {
                continue;
            } else {
                return false;
            }
        }
    }
    return true;
}

void Board::test_reportStatusAt(const Pos &pos) const {
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
