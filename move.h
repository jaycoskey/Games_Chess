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

#include <string>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"

#include "logger.h"

struct ExtMove;
class  Move;
class  PosMovesComparator;

using OptMove   = std::optional<Move>;
using Pos2Moves = std::map<Pos, Moves>;


enum class CaptureAbility {
    CanCapture,
    MustCapture,
    MustNotCapture
};

enum class CheckType {
    None,
    Check,
    CheckMate
};

// ========================================
// MoveType

enum class MoveType {
    Simple,
    CastleK,
    CastleQ,
    EnPassant,
    PawnPromotion
};

std::ostream& operator<<(std::ostream& os, MoveType moveType);

// ========================================
// Move

class Move {
public:
    // ---------- Public static methods (accessors)
    static const IsAttackingRule& getIsAttackingRule(PieceType pt);
    static const MoveRule&        getMoveRule(PieceType pt);
    static const Moves&           getMoveHistory() { return Move::_moveHistory; };

    static const std::string      history_to_pgn();
    static Move&                  prevMove() { return _moveHistory.back(); }

    static       void             reset() { _moveHistory.clear(); }

    // ---------- Public static methods (attacking / moving rules)
    static bool isAttacked(const Board& b, const Pos& tgtPos, Color tgtColor);
    static bool isAttacking(const Board& b, const Piece& attacker, const Pos& tgtPos
                           , const Dirs& dirs, Short maxSteps=0
                           );
    static bool isInCheck(const Board& b, Color c) noexcept;
    static bool pawnIsAttackingRule(const Board& b, const Piece& attacker, const Pos& tgtPos);
    static const Moves pawnMoveRule(const Board& b, Color c, const Pos& pos);

    // ---------- Public static methods (get move / interactivity / strategy)
    static ExtMove getPlayerMove(
                       PlayerType playerType
                       , const Board& b, Color c, const Pos2Moves& validPlayerMoves
                       );
    static Moves   getValidPieceMoves(
                       const Board& b, Color c, Pos pos, PieceType pt, Dirs dirs
                       , Short maxSteps=0
                       , CaptureAbility captureAbility=CaptureAbility::CanCapture
                       );
    static const Pos2Moves getValidPlayerMoves(const Board& b, Color c);
    static ExtMove queryPlayerMove(
                       const Board& b, Color c
                       , const Pos2Moves& validPlayerMoves
                       );
    static ExtMove randomMove(Color c, const Moves& moves);
    static ExtMove strategyRandom(
                       const Board& b, Color c
                       , const Pos2Moves& validPlayerMoves
                       );
    static ExtMove strategyRandomCapture(
                       const Board& b, Color c
                       , const Pos2Moves& validPlayerMoves
                       );

    // ---------- Constructors
    Move(Color color, PieceType pt, const Pos from, const Pos to
            , PieceP capturedP=nullptr
            , bool isPawnMove=false
            , bool isEnPassant=false
            , OptPieceType promotedType=std::nullopt
        );

    // ---------- Public read methods
    Color      color()     const { return _color;     }
    PieceType  pieceType() const { return _pieceType; }
    const Pos& from()      const { return _from;      }
    const Pos& to()        const { return _to;        }

    const std::string algNotation() const { return _from.algNotation() + ' ' + _to.algNotation(); }
    bool         isCapture()   const { return static_cast<bool>(_capturedP); }

    bool         isCastling()  const;
    bool         isCastlingK() const;
    bool         isCastlingQ() const;

    bool         isCheck()     const { return _isCheck; }
    bool         isCheckmate() const { return _isCheckmate; }
    bool         isEnPassant() const { return _isEnPassant; }
    bool         isPawnMoveOrCapture() const { return _isPawnMove || _capturedP; }
    bool         isPromotion() const { return _oPromotedTo != std::nullopt; }
    const PieceP capturedP()   const { return _capturedP; }
    PieceType    promotionType() const { return *_oPromotedTo; }
    const std::string to_pgn() const;

    // ---------- Public read methods (inspection)
    bool doesCauseSelfCheck(const Board& b, Color c) const noexcept;

    // ---------- Public read methods (Board modification)
    void apply(Board& b) const;
    void applyUndo(Board& board)  const;

    // ---------- Public write method
    void setCheck(bool isCheck)         { _isCheck     = isCheck; }
    void setCheckmate(bool isCheckmate) { _isCheckmate = isCheckmate; }

    // ---------- Operators
    bool operator==(const Move& other) const;
    bool operator<(const Move& other) const;

private:
    static PieceType2IsAttackingRule _createIsAttackingRules();
    static Moves                     _createHistory();
    static PieceType2MoveRule        _createMoveRules();
    static ExtMove _parseMoveInAlgNotation(const Board& b, Color c, const std::string& input) noexcept(false);

    static PieceType2IsAttackingRule _pieceType2IsAttackingRule;
    static PieceType2MoveRule        _pieceType2MoveRule;
    static Moves                     _moveHistory;

    Color     _color;
    PieceType _pieceType;
    Pos       _from;
    Pos       _to;

    PieceP    _capturedP;
    bool      _isPawnMove;
    bool      _isEnPassant;
    OptPieceType _oPromotedTo;
    bool      _isCheck;
    bool      _isCheckmate;

    // ---------- Friend operator
    friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

// ========================================
// ExtMove

struct ExtMove {
    ExtMove()
        : optMove{std::nullopt}, isDrawClaim{false}, agreedGameEnd{GameEnd::InPlay}
    {};

    ExtMove(OptMove om, bool idc, GameEnd ge)
        : optMove{om}, isDrawClaim{idc}, agreedGameEnd{ge}
    {};

    bool isErrorValue() const {
        return optMove == std::nullopt
            && isDrawClaim == false
            && agreedGameEnd == GameEnd::InPlay;
    }

    OptMove optMove;
    bool    isDrawClaim;
    GameEnd agreedGameEnd;
};

// ========================================

class PosMovesComparator {
public:
    PosMovesComparator(const Board& b) : _b{b} {}

    bool operator()(const std::pair<Pos, Moves>& pma, const std::pair<Pos, Moves>& pmb)
    {
        const Pos& aPos = pma.first;
        assert(aPos.isOnBoard());
        const Pos& bPos = pmb.first;
        assert(bPos.isOnBoard());
        PieceValue pmaVal = Piece::pieceValue(_b.pieceAt(aPos).get()->pieceType());
        PieceValue pmbVal = Piece::pieceValue(_b.pieceAt(bPos).get()->pieceType());
        return (pmaVal < pmbVal)
            || (pmaVal == pmbVal && !(pma.first < pmb.first));
    }
private:
    const Board& _b;
};
