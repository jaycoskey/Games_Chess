// Copyright 2021, by Jay M. Coskey

#pragma once

#include <regex>
#include <string>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"

#include "logger.h"

using std::string, std::ostream, std::to_string;
using std::cin, std::cout, std::ostringstream;
using std::map;


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

enum class MoveType {Simple, CastleK, CastleQ, EnPassant, PawnPromotion};

class PosMovesComparator {
public:
    PosMovesComparator(const Board& b) : _b{b} {}

    bool operator()(const std::pair<Pos, Moves>& ma, const std::pair<Pos, Moves>& mb) {
        float maValue = Piece::pieceValue(_b.pieceAt(ma.first)->get()->pieceType());
        float mbValue = Piece::pieceValue(_b.pieceAt(mb.first)->get()->pieceType());
        return (maValue < mbValue)
            || (maValue == mbValue && !(ma.first < mb.first));
    }
private:
    const Board& _b;
};

// TODO: Store CheckType here?
// TODO: When listing valid moves for player, sort piece type: KQRBNP.
// TODO: Complete support of Move::apply() for remaining move types (e.g., en passant).
// TODO: How best to track through apply/applyUndo whether King/Rook have moved?
//
// Types of moves to handle: capture, castling, en passant, pawn promotion.
class Move {
public:
    // ---------- Static methods
    static Moves _createHistory();

    static const IsAttackingRule& getIsAttackingRule(PieceType pt);
    static const Moves&           getMoveHistory() { return Move::_moveHistory; };
    static const MoveRule&        getMoveRule(PieceType pt);

    // ---------- Constructors
    Move(Color color, PieceType pt, const Pos& from, const Pos& to
            , OptPieceP oCapturedP=std::nullopt
            , bool isPawnMove=false
            , bool isEnPassant=false
            , PieceType promotedType=PieceType::None
        )
        : _color{color}, _pieceType{pt}, _from{from}, _to{to}
            , _oCapturedP{oCapturedP}
            , _isPawnMove{isPawnMove}
            , _isEnPassant{isEnPassant}
            , _promotedTo{promotedType}
    { }

    Move(Color color, PieceType pt, const Pos& from, const Pos& to
            , const PieceP& capturedP
        ) : Move(color, pt, from, to, std::make_optional<PieceP>(capturedP))
    { }

    // ---------- Read data
    Color      color()     const { return _color;     }
    PieceType  pieceType() const { return _pieceType; }
    const Pos& from()      const { return _from;      }
    const Pos& to()        const { return _to;        }

    const string algNotation() const { return _from.algNotation() + ' ' + _to.algNotation(); }
    bool isCapture()             const { return _oCapturedP != std::nullopt; }
    bool isPawnMoveOrCapture()   const { return _isPawnMove || _oCapturedP != std::nullopt; }
    bool isPromotion()           const { return _promotedTo != PieceType::None; }
    const OptPieceP oCapturedP() const { return _oCapturedP; }

    // ---------- Modify Board
    void apply(Board& board);  // Modifies Move only on Pawn promotion
    void applyImpl(Board& b)      const;
    void applyUndo(Board& board)  const;

    bool operator==(const Move& other) const;
    bool operator<(const Move& other) const;

private:
    static PieceType2IsAttackingRule _createIsAttackingRules();
    static PieceType2MoveRule    _createMoveRules();
    static void                  printHistory();

    static PieceType2IsAttackingRule _pieceType2IsAttackingRule;
    static PieceType2MoveRule        _pieceType2MoveRule;
    static Moves                     _moveHistory;

    // ---------- User interactivity
    PieceType _queryPlayerPromotionType() const;

    Color     _color;
    PieceType _pieceType;
    Pos       _from;
    Pos       _to;

    OptPieceP _oCapturedP;
    bool      _isPawnMove;
    bool      _isEnPassant;
    PieceType _promotedTo;

    // ---------- Friends
    friend ostream& operator<<(ostream& os, const Move& move);
};

// ---------- Forward declarations of some Move-related non-member functions

bool isAttacked(const Board& b, const Pos& tgtPos, const Color tgtColor);

bool isAttacking(
        const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor
        , const Dirs& dirs, Short maxSteps=0
        );

bool isInCheck(const Board& b, Color c) noexcept;

Moves getValidPieceMoves(
        const Board& b, Color c, Pos pos, PieceType pt, Dirs dirs
        , Short maxSteps=0
        , CaptureAbility captureAbility=CaptureAbility::CanCapture
        );

bool pawnIsAttackingRule(const Board& b, const Piece& attacker
        , const Pos& tgtPos, const Color tgtColor
        );

const Moves pawnMoveRule(const Board& b, Color c, const Pos& pos);

// ---------- Initialization of static data

PieceType2IsAttackingRule Move::_pieceType2IsAttackingRule = Move::_createIsAttackingRules();
PieceType2MoveRule        Move::_pieceType2MoveRule        = Move::_createMoveRules();
Moves                     Move::_moveHistory               = Move::_createHistory();

// ---------- Static methods

const IsAttackingRule& Move::getIsAttackingRule(PieceType pt)
{
    return Move::_pieceType2IsAttackingRule.at(pt);
}

const MoveRule& Move::getMoveRule(PieceType pt)
{
    return Move::_pieceType2MoveRule.at(pt);
}

PieceType2IsAttackingRule Move::_createIsAttackingRules()
{
    PieceType2IsAttackingRule pt2cr;

    pt2cr[PieceType::King]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return isAttacking(b, attacker, tgtPos, tgtColor, allDirs, 1); };
    pt2cr[PieceType::Queen]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return isAttacking(b, attacker, tgtPos, tgtColor, allDirs); };
    pt2cr[PieceType::Rook]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return isAttacking(b, attacker, tgtPos, tgtColor, orthoDirs); };
    pt2cr[PieceType::Bishop]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return isAttacking(b, attacker, tgtPos, tgtColor, diagDirs); };
    pt2cr[PieceType::Knight]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return isAttacking(b, attacker, tgtPos, tgtColor, knightDirs, 1); };
    pt2cr[PieceType::Pawn]
        = [&](const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
              { return pawnIsAttackingRule(b, attacker, tgtPos, tgtColor); };

    return pt2cr;
}

PieceType2MoveRule Move::_createMoveRules()
{
    PieceType2MoveRule pt2mr;

    pt2mr[PieceType::King]
        = [&](const Board& b, const Color c, const Pos& from)
              {
                Moves kMoves = getValidPieceMoves(b, c, from, PieceType::King, allDirs, 1);
                if (b.king(c).lastMoveIndex() == 0) {
                    // King-side castle
                    OptPieceP okRook = b.pieceAt(b.kRookInitPos(c));
                    if (okRook != std::nullopt) {
                        const Piece& kRook = *(okRook->get());
                        if (kRook.lastMoveIndex() == 0) {
                            if (  !isAttacked(b, from, c)
                               && b.isEmpty(from.posRight(c, 1))
                                   && !isAttacked(b, from.posRight(c, 1), c)
                               && b.isEmpty(from.posRight(c, 2))
                                   && !isAttacked(b, from.posRight(c, 2), c)
                               && !isAttacked(b, from.posRight(c, 3), c)
                               )
                            {
                                kMoves.emplace(kMoves.end(), c, PieceType::King
                                        , from, from.posRight(c, 2)
                                        );
                            }
                        }
                    }
                }
                if (b.king(c).lastMoveIndex() == 0) {
                    // Queen-side castle
                    OptPieceP oqRook = b.pieceAt(b.qRookInitPos(c));
                    if (oqRook != std::nullopt) {
                        const Piece& qRook = *(oqRook->get());
                        if (qRook.lastMoveIndex() == 0) {
                            if (  !isAttacked(b, from, c)
                               && b.isEmpty(from.posLeft(c, 1))
                                   && !isAttacked(b, from.posLeft(c, 1), c)
                               && b.isEmpty(from.posLeft(c, 2))
                                   && !isAttacked(b, from.posLeft(c, 2), c)
                               && b.isEmpty(from.posLeft(c, 3))
                                   && !isAttacked(b, from.posLeft(c, 3), c)
                               && !isAttacked(b, from.posLeft(c, 4), c)
                               )
                            {
                                kMoves.emplace(kMoves.end(), c, PieceType::King
                                        , from, from.posLeft(c, 2)
                                        );
                            }
                        }
                    }
                }
                return kMoves;
              };
    pt2mr[PieceType::Queen]
        = [&](const Board& b, const Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Queen,  allDirs);
              };
    pt2mr[PieceType::Rook]
        = [&](const Board& b, const Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Rook,   orthoDirs);
              };
    pt2mr[PieceType::Bishop]
        = [&](const Board& b, const Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Bishop, diagDirs);
              };
    pt2mr[PieceType::Knight]
        = [&](const Board& b, const Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Knight, knightDirs, 1);
              };
    pt2mr[PieceType::Pawn]
        = [&](const auto& b, const Color c, const auto& from)
              {
                  return pawnMoveRule(b, c, from);
              };

    return pt2mr;
}

Moves Move::_createHistory() {
    return Moves();
}

// ---------- Modify Board state

void Move::apply(Board& b) {
    applyImpl(b);
    // Pawn promotion
    if (_pieceType == PieceType::Pawn && _to.toRelRow(_color) == BOARD_ROWS - 1) {
        _promotedTo = _queryPlayerPromotionType();
        b.pieceAt(_to)->get()->setPieceType(_promotedTo);
    }
}

void Move::applyImpl(Board& b) const
{
    b.movePiece(_from, _to);
    if (_pieceType == PieceType::King) {
        if (_from.xdiff(_to) == 2) {  // King-side castle
            Pos kRookFrom = Board::kRookInitPos(_color);
            Pos kRookTo = Board::kRookInitPos(_color).posLeft(_color, 2);
            b.movePiece(kRookFrom, kRookTo);
        } else if (_from.xdiff(_to) == -2) {  // Queen-side castle
            Pos qRookFrom = Board::qRookInitPos(_color);
            Pos qRookTo = Board::qRookInitPos(_color).posRight(_color, 3);
            b.movePiece(qRookFrom, qRookTo);
        }
    }
    b.currentMoveIndex_incr();
    Move::_moveHistory.push_back(*this);
}

void Move::applyUndo(Board& b) const
{
    // ---------- Determine type of move being undone
    MoveType moveType = MoveType::Simple;
    if (_pieceType == PieceType::King) {
        Short xdiff = abs(_from.x - _from.y);
        if (xdiff == 2) { moveType = MoveType::CastleK; }
        else if (xdiff == 3) { moveType = MoveType::CastleQ; }
    }
    if (_pieceType == PieceType::Pawn) {
        if (_promotedTo != PieceType::None) {
            moveType = MoveType::PawnPromotion;
        } else if (_isEnPassant) {
            assert(_to.toRelRow(_color) == BOARD_ROWS - 1);
            moveType = MoveType::EnPassant;
        }
    }

    // ---------- Undo move
    // Restore Piece type
    Piece& movedPiece = *(b.pieceAt(_to)->get());
    if (moveType == MoveType::PawnPromotion) {
        movedPiece.setPieceType(PieceType::Pawn);
    }

    // Restore Piece location
    b.movePiece(_to, _from);

    // Restore captured piece
    if (moveType == MoveType::EnPassant) {
        // Restore captured Pawn
        b.addPieceTo(opponent(movedPiece.color()), PieceType::Pawn, _to.index());
    } else if (_oCapturedP != std::nullopt) {
        // Restore captured piece
        b.addPieceTo(opponent(movedPiece.color()), _oCapturedP->get()->pieceType(), _to.index());
    }

    // Restore locations of Rook involved in en passant
    if (moveType == MoveType::CastleK) {
        // Restore K-side Rook
        const Pos& to   = Board::kRookInitPos(_color);
        const Pos& from = to.posLeft(_color, 2);
        b.movePiece(from, to);
    } else if (moveType == MoveType::CastleQ) {
        // Restore Q-side Rook
        const Pos& to   = Board::qRookInitPos(_color);
        const Pos& from = to.posRight(_color, 3);
        b.movePiece(from, to);
    }
    b.currentMoveIndex_decr();
    Move::_moveHistory.pop_back();
}

PieceType Move::_queryPlayerPromotionType() const {
    // TODO: Can use _color to select Player to query
    cout << "Player promotion type (one letter, among KQRBNP)? ";
    string input;
    while (true) {
        getline(cin, input);
        if (input.size() != 1) { continue; }
        switch (input[0]) {
            case 'K': return PieceType::King;
            case 'Q': return PieceType::Queen;
            case 'R': return PieceType::Rook;
            case 'B': return PieceType::Bishop;
            case 'N': return PieceType::Knight;
            case 'P': return PieceType::Pawn;
            default: continue;
        }
    }
}

// ======================================== 
// Non-member classes and functions
// ======================================== 

bool doesMoveSelfCheck(const Board& b, Color c, const Move& move) noexcept {
    logger.info("        doesMoveSelfCheck: pushing move: ", move
            , ". Dest ", move.to(), b.isEmpty(move.to()) ? " is empty" : "is not empty");
    move.applyImpl(const_cast<Board&>(b));  // Temp board alteration
    bool result = isInCheck(b, c);
    logger.info("        doesMoveSelfCheck: popping move: ", move
            , ". Does ", result ? "" : "not ", "self-check.");
    move.applyUndo(const_cast<Board&>(b));  // Undo temp board alteration
    return result;
}

// The function isAttacking does something similar but with a boolean result.
Moves getValidPieceMoves(
    const Board& b, Color c, Pos pos, PieceType pt, Dirs dirs
    , Short maxSteps  // =0
    , CaptureAbility captureAbility  // =CaptureAbility::CanCapture
    )
{
    Moves result{};

    if (maxSteps == 0) { maxSteps = std::max(BOARD_COLS, BOARD_ROWS); }

    for (Dir dir : dirs) {
        Pos dest{pos};
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            if (!Board::containsPos(dest)) {
                logger.trace("    getValidPieceMoves: ", c, pt, "_@_", pos, "-->", dest
                        , ": Moved off board");
                break;  // Done stepping in this direction
            }
            auto oPieceAtDestP = b.pieceAt(dest);
            if (oPieceAtDestP == std::nullopt) {
                if (captureAbility == CaptureAbility::MustCapture) {
                    logger.trace("    getValidPieceMoves: ", c, pt, "_@_", pos, "-->", dest
                            , ": Pawn cannot move diagonally without capture @ ", dest);
                    break;
                }
                // Non-capture move
                int beforeSize = result.size();
                logger.trace("    getValidPieceMoves: ", c, pt, "_@_", pos, "-->", dest
                        , ": Found valid non-capture move");
                // ---------- Record move
                result.emplace(result.end(), c, pt, pos, dest
                        , std::nullopt
                        , pt == PieceType::Pawn
                        , false
                        );
                int afterSize = result.size();
                assert(afterSize = beforeSize + 1);
            } else {
                if (captureAbility == CaptureAbility::MustNotCapture) {
                    logger.trace("    getValidPieceMoves: ", c, pt, "_@_", pos, "-->", dest
                            , ": Pawn cannot move forward onto occupied space @ ", dest);
                    break;
                }
                if (oPieceAtDestP->get()->color() == c) {
                    logger.trace("    getValidPieceMoves: ", c, pt, "_@_", pos, "-->", dest
                            , ": Cannot capture own piece @ ", dest);
                    break;  // Cannot capture one's own piece
                }
                // Capture move
                logger.info(" getValidPieceMoves: ", c, pt, pos, "-->", dest
                        , ": Valid capture move: Point="
                        , c, pt
                        , '@', pos, "-->", dest
                        );
                // ---------- Record move
                result.emplace(result.end(), c, pt, pos, dest
                        , b.pieceAt(dest)
                        , pt == PieceType::Pawn
                        , false
                        );
                break;  // Cannot move past opponent's piece
            }
        }
    }
    if (result.size() > 0) {
        logger.info("    ==> getValidPieceMoves: ", c, pt, "_@_", pos, ": ", result.size(), " valid moves");
    } else {
        logger.info("    ==> getValidPieceMoves: ", c, pt, "_@_", pos, ": ", result.size(), " valid moves");
    }
    return result;
}

// Aggregates valid moves from the Player's pieces' Move rules.
const Pos2Moves getValidPlayerMoves(const Board& b, const Color& c)
{
    Pos2Moves result;

    logger.info("getValidPlayerMoves: ====================");
    for (const PieceP& pieceP : b.piecesWithColor(c)) {
        MoveRule moveRule = Move::getMoveRule(pieceP->pieceType());
        const Pos& from = pieceP->pos();
        logger.info("getValidPlayerMoves: Looking at piece ", *pieceP);
        const Moves& moves = moveRule(b, pieceP->color(), from);
        for (Move move : moves) {
            logger.trace("    Calling doesMoveSelfCheck on move: ", move);
            if (!doesMoveSelfCheck(b, c, move)) {
                logger.info("    ==> getValidPlayerMoves: Adding move of ", *pieceP, " to result");
                result[from].insert(result[from].end(), move);
            }
        }
    }
    return result;
}

bool isAttacked(const Board& b, const Pos& tgtPos, const Color tgtColor)
{
    auto& attackers = b.color2PiecePs.at(opponent(tgtColor));
    for (const PieceP& attackerP : attackers) {
        IsAttackingRule isAttackingRule = Move::getIsAttackingRule(attackerP->pieceType());
        if (isAttackingRule(b, *attackerP, tgtPos, tgtColor)) {
            return true;
        }
    }
    return false;
}

// The function getValidPieceMoves does something similar, but its result is a collection of Moves.
bool isAttacking(
        const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor
        , const Dirs& dirs, Short maxSteps // =0
        )
{
    if (maxSteps == 0) { maxSteps = std::max(BOARD_COLS, BOARD_ROWS); }

    for (Dir dir : dirs) {
        Pos dest{attacker.pos()};
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            if (!Board::containsPos(dest)) {
                break;  // Fell off board---done stepping in this direction
            }
            auto oPieceAtDestP = b.pieceAt(dest);
            if (oPieceAtDestP == std::nullopt) {
                continue;  // Empty space
            } else {
                if (oPieceAtDestP->get()->color() == tgtColor) {
                    break;  // Reached own piece---done stepping in this direction
                }
                if (oPieceAtDestP->get()->pos() == tgtPos) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isInCheck(const Board& b, Color c) noexcept {
    const Piece& king = b.king(c);
    assert(king.pieceType() == PieceType::King);
    for (const PieceP& attackerP : b.color2PiecePs.at(opponent(c))) {
        IsAttackingRule isAttackingRule = Move::getIsAttackingRule(attackerP->pieceType());
        if (isAttackingRule(b, *attackerP, king.pos(), king.color())) {
            logger.debug("        ->->", c, " king is being attacked by ", *attackerP);
            return true;
        }
    }
    return false;
}

const Move parseMoveInAlgNotation(const Board& b, const string& input) noexcept(false)
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
    const Piece& fromPiece = *(b.pieceAt(*fromP)->get());
    return Move{fromPiece.color(), fromPiece.pieceType(), *fromP, *toP};
}

bool pawnIsAttackingRule(const Board& b, const Piece& attacker, const Pos& tgtPos, const Color tgtColor)
{
    // Capture diagonally
    Dir forward = Player::getForward(attacker.color());
    Dirs captureDirs = Dirs{forward + Dir{-1,0}, forward + Dir{1, 0}};
    for (const Dir& captureDir : captureDirs) {
        if (attacker.pos() + captureDir == tgtPos) {
            return true;
        }
    }
    // TODO: En passant
    return false;
}

// TODO: En passant & other Pawn details
const Moves pawnMoveRule(const Board& b, Color c, const Pos& pos)
{
    Moves result{};

    // Move forward w/o capture
    Short stepCount = isPawnInitialPosition(c, pos) ? 2 : 1;
    const auto& marchMoves
        = getValidPieceMoves( b, c, pos, PieceType::Pawn, std::set{Player::getForward(c)}
                            , stepCount
                            , CaptureAbility::MustNotCapture
                            );
    result.insert(result.end(), marchMoves.begin(), marchMoves.end());

    // Standard capture
    Dir forward = Player::getForward(c);
    Dirs captureDirs = Dirs{forward + Dir{-1,0}, forward + Dir{1, 0}};
    const auto& captureMoves
        = getValidPieceMoves( b, c, pos, PieceType::Pawn, captureDirs
                            , 1
                            , CaptureAbility::MustCapture
                            );
    result.insert(result.end(), captureMoves.begin(), captureMoves.end());

    // En passant
    Dirs lateralDirs = Dirs{Dir{-1,0}, Dir{1, 0}};
    for (const Dir& lateralDir : lateralDirs) {
        const Pos& opponentPos = pos + lateralDir;
        const OptPieceP& optOpponentPieceP = b.pieceAt(opponentPos);
        if (optOpponentPieceP != std::nullopt
                && optOpponentPieceP->get()->color() == opponent(c)
                && optOpponentPieceP->get()->lastMoveIndex() == b.currentMoveIndex() - 1
                )
        {
            Move enPassantMove = Move(c, PieceType::Pawn, pos, pos + lateralDir
                    , optOpponentPieceP, true, true);
            result.insert(result.end(), enPassantMove);
        }

    }
    return result;
}

const Move queryPlayerMove(
    const Board& b
    , const Color& c
    , const Pos2Moves& validPlayerMoves
    )
{
    const string& help = "\tEnter moves as a pair of board positions in algebraic notation. E.g.: e2 e4";
    const string& prompt = "Enter move #" + to_string(b.currentMoveIndex())
                               + " (? for help): ";
    cout << Player::playerName(c) << "'s turn\n";

    PosMovesComparator pmComparator{b};

    while (true) {
        cout << prompt;
        try {
            string input;
            getline(cin, input);
            if (input[0] == '?') {
                cout << help << "\n";
                continue;
            }
            if (input == "history") {
                cout << showVector(Move::getMoveHistory()) << "\n";
                continue;
            }
            if (input == "list") {
                cout << "List of valid moves:\n";
                auto /* vector<Pos, Moves> */ playerMoves = mapToVector(validPlayerMoves);
                std::sort(playerMoves.rbegin(), playerMoves.rend(), pmComparator);
                for (const auto& [from, moves] : playerMoves) {
                    PieceType pt = b.pieceAt(from)->get()->pieceType();
                    cout << "  Moves of " << pt
                         << " @ " <<  from.algNotation() << " (" << moves.size() << "): ";
                    for (const Move& move : moves) {
                        cout << move.to().algNotation() << ' ';
                    }
                    cout << "\n";
                }
                continue;
            }

            if (input == "_board")  { cout << b << "\n";        continue; }
            if (input == "_pieces") { b.listPieces();           continue; }

            if (input == "_error")  { logger.setReportLevel(LogError); continue; }
            if (input == "_warn")   { logger.setReportLevel(LogWarn);  continue; }
            if (input == "_info")   { logger.setReportLevel(LogInfo);  continue; }
            if (input == "_debug")  { logger.setReportLevel(LogDebug); continue; }
            if (input == "_trace")  { logger.setReportLevel(LogTrace); continue; }

            if (input == "")  { continue; }

            Move move = parseMoveInAlgNotation(b, input);
            logger.info("Getting valid piece moves: from=", move.from());
            Moves validPieceMoves = validPlayerMoves.at(move.from());
            const auto& beg = validPieceMoves.begin();
            const auto& end = validPieceMoves.end();
            if (find(beg, end, move) == end) {
                cout << "That is not a legal move." << "\n";
                continue;
            }
            return move;
        }
        catch (const std::invalid_argument& ex) {
            // pass
        }
    }
}

// ---------- Operators

ostream& operator<<(ostream& os, MoveType moveType) {
    static const map<MoveType, const char*> mt2str{
        {MoveType::Simple, "Simple move"}
        , {MoveType::CastleK, "King-side castle"}
        , {MoveType::CastleQ, "Queen-side castle"}
        , {MoveType::EnPassant, "En passant"}
        , {MoveType::PawnPromotion, "Pawn promotion"}
    };
    os << mt2str.at(moveType);
    return os;
}

ostream& operator<<(ostream& os, const Move& move) {
    os << move._from << "-->" << move._to;
    return os;
}

bool Move::operator<(const Move& other) const
{
    if (  _from < other._from
       || (_from.x == other._from.x && _from.y == other._from.y && _to < other._to)
       )  { return true; }
    return false;
}

bool Move::operator==(const Move& other) const
{
    return _color == other._color
        && _pieceType == other._pieceType
        && _from == other._from
        && _to == other._to;
}

