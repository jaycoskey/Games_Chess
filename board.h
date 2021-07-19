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

#pragma once

#include <array>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <string>

#include <cassert>

#include "geometry.h"
#include "piece.h"
#include "player.h"
#include "util.h"

#include "logger.h"

using Players = std::vector<Player>;

// ---------- Piece-related aliases
using PieceP = std::shared_ptr<Piece>;
using PiecePs = std::set<PieceP>;

// Supports testing Draw due to InsufficientResources
using PieceTypes = std::vector<PieceType>;

// ---------- Move-related aliases
class Move;

using Moves = std::vector<Move>;
using MoveIndexes =
    std::set<MoveIndex>; // MoveIndexes at which a given Board Hash occurred
using Hash2MoveIndexes = std::map<Hash, MoveIndexes>;

class Board;
using IsAttackingRule = std::function<bool(
    const Board &b, const Piece &attacker, const Pos &tgtPos)>;
using MoveRule = std::function<Moves(const Board &, Color, const Pos &)>;

// ---------- Board-related aliases
using Color2KingP = std::map<Color, PieceP>; // For storage of Kings by Color
using Color2PiecePs =
    std::map<Color, PiecePs>;             // For storage of Pieces by Color
using Pos2PieceP = std::map<Pos, PieceP>; // For storage of Pieces by location

using PieceType2IsAttackingRule = std::map<PieceType, IsAttackingRule>;
using PieceType2MoveRule = std::map<PieceType, MoveRule>;

// For Zobrist hashing. See Wikipedia.
using ZIndex = int;
using ZTable = std::array<std::array<Hash, COLORS_COUNT * PIECE_TYPES_COUNT>,
                          BOARD_COLS * BOARD_ROWS>;

using BoardHashHistory =
    std::map<Color, Hash2MoveIndexes>; // Record of Board Hash history, for Draw
                                       // detection
using VecBool = std::vector<bool>; // Record of Game PMOC progress (PMOC=Pawn
                                   // move or capture)

// Part of GameState, which determines whether the Game has ended
enum class GameEnd { InPlay, Draw, WinBlack, WinWhite };

// Part of GameState, which determines whether the Game has ended
enum class WinType {
    None,
    Agreement,
    Checkmate,
    Conceding
    // Clock - No clock used
};

std::ostream &operator<<(std::ostream &os, WinType wt);

// Conditions that can justify a Player claiming that the Game has ended in a
// Draw Note:
//   * Automatic Draw conditions (5x Rep, 75 Move Rule) are not claimed.
//   * The FIDE laws for the 50 Move Rule mention "50 moves by each player",
//     but the law is interpreted as meaning 50 moves total.
enum DrawableFlag {
    Drawable_None = 0,
    Drawable_3xRepetition = 1 << 0,
    Drawable_50MoveRule = 1 << 1
};

// Part of GameState, which determines whether the Game has ended
enum DrawFlag {
    Draw_None = 0,
    Draw_5xRepetition = 1 << 0,
    Draw_75MoveRule = 1 << 1,
    Draw_Agreement = 1 << 2,
    Draw_Claimed_3xRepetition = 1 << 3,
    Draw_Claimed_50MoveRule = 1 << 4,
    // Draw_Clock,
    // Draw_DeadPosition,  // Not including InsufficientResources
    Draw_InsufficientResources = 1 << 5,
    Draw_Stalemate = 1 << 6
};

// ----------

using DrawableFlags = int; // Bitwise OR of DrawableFlag values
using DrawFlags = int;     // Bitwise OR of DrawFlag values

// ----------

class Board {
  public:
    static Pos kInitPos(Color c) { return Pos(BOARD_KING_COL, homeRow(c)); }
    static Pos kRookInitPos(Color c) { return Pos(7, homeRow(c)); }
    static Pos qRookInitPos(Color c) { return Pos(0, homeRow(c)); }

    static Hash zobristBitstring(int cInd, int ptInd) {
        return Board::_zobristTable[cInd][ptInd];
    }

    // ---------- Constructors
    Board(bool doPopulate = false);
    Board(const Board &other);

    // Rule of three
    Board &operator=(const Board &other) {
        color2PiecePs = other.color2PiecePs;
        _color2KingP = other._color2KingP;
        _pos2PieceP = other._pos2PieceP;
        _currentMoveIndex = other._currentMoveIndex;
        _boardHashHistory = other._boardHashHistory;
        _pmocHistory = other._pmocHistory;

        return *this;
    }
    ~Board() {}

    // ---------- Cell / Piece data - read
    PieceP pieceAt(const Pos &pos) const;
    PieceP pieceAt(Col col, Row row) const;
    PieceP pieceAt(Short index) const;

    bool isEmpty(const Pos &pos) const { return !pieceAt(pos); }
    bool isEmpty(Col col, Row row) const { return !pieceAt(col, row); }
    const Piece &king(Color c) const { return *_color2KingP.at(c); }

    // ---------- Piece data - write
    // void addPiecePTo(PieceP pieceP, const Pos& to);
    void addPieceTo(Color c, PieceType pt, Short index,
                    Short lastMoveIndex = 0);
    void addPieceTo(Color c, PieceType pt, const std::string &posStr,
                    Short lastMoveIndex = 0);
    void addPiecePair(PieceType pt, Short index, bool preserveCol = false);
    void movePiece(const Pos &from, const Pos &to);
    PieceTypes pieceTypes(Color c) const;
    const PiecePs &piecesWithColor(Color c) const;
    void removePieceAt(const Pos &pos);

    // ---------- Board data - read
    float boardValue() const;
    float boardValue(Color c) const;
    Short currentMoveIndex() const { return _currentMoveIndex; }
    bool hasInsufficientResources() const;
    std::size_t maxBoardRepetitionCount(Color c) const;
    Short movesSinceLastPmoc() const;
    Short pieceCount(Color c) const { return color2PiecePs.at(c).size(); }
    Short pieceCount() const {
        return pieceCount(Color::Black) + pieceCount(Color::White);
    }

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
    void test_assert_pmocHistory_size() const {
        assert(_pmocHistory.size() < 10'000);
    }

    static void test_printZobristTable() {
        for (Short iBoard = 0; iBoard < BOARD_SPACES; ++iBoard) {
            for (Short iPiece = 0; iPiece < COLORS_COUNT * PIECE_TYPES_COUNT;
                 ++iPiece) {
                std::cout << "ZobristTable[" << std::setw(2) << iBoard << ']'
                          << '[' << std::setw(2) << iPiece << "] = 0x"
                          << std::hex << std::setfill('0') << std::setw(16)
                          << _zobristTable[iBoard][iPiece] << std::dec << "\n";
            }
        }
    }
    void test_reportStatusAt(const Pos &pos) const;

  private:
    static ZIndex _getZIndex(Color c) {
        static std::map<Color, ZIndex>
                c2zi{
                    {Color::Black, 0},
                    {Color::White, 1}
                };
        return c2zi.at(c);
    }
    static ZIndex _getZIndex(PieceType pt) {
        static std::map<PieceType, ZIndex> pt2zi{
            {PieceType::King, 0},   {PieceType::Queen, 1},
            {PieceType::Rook, 2},   {PieceType::Bishop, 3},
            {PieceType::Knight, 4}, {PieceType::Pawn, 5}};
        return pt2zi.at(pt);
    }
    static ZIndex _getZIndex(PieceP pp) {
        static const std::vector<PieceType> allPieceTypes{
            PieceType::King,   PieceType::Queen,  PieceType::Rook,
            PieceType::Bishop, PieceType::Knight, PieceType::Pawn
        };
        return _getZIndex(pp->color()) * allPieceTypes.size() +
               _getZIndex(pp->pieceType());
    }

    static ZTable _zobristTable;

    Color2KingP _color2KingP;
    Pos2PieceP _pos2PieceP;

    // ---------- History
    MoveIndex
        _currentMoveIndex; // 1-based. Matches popular notion of turn number.
    BoardHashHistory
        _boardHashHistory; // updateBoardHashHistory & maxBoardRepetitionCount
    VecBool _pmocHistory;  // updatePmocHistory      & movesSinceLastPmoc

    friend struct std::hash<Board>;
    friend std::ostream &operator<<(std::ostream &os, const Board &board);

  public:
    std::string test_to_string() const; // For testing/debugging
};

// For testing/debugging
bool operator==(const Board &lhs, const Board &rhs);
