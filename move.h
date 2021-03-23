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

// TODO: Store CheckType here?
// TODO: When listing valid moves for player, sort piece type: KQRBNP.
// TODO: Complete support of Move::apply() for remaining move types (e.g., en passant).
// TODO: How best to track through apply/applyUndo whether King/Rook have moved?
//
// Types of moves to handle: capture, castling, en passant, pawn promotion.
class Move {
public:
    static CaptureRule getCaptureRule(PieceType pieceType);
    static MoveRule getMoveRule(PieceType pieceType);

    Move(Color color, PieceType pieceType, Pos from, Pos to
            , OptPieceP oCapturedP=std::nullopt
            , bool isEnPassant=false
            , PieceType promotedType=PieceType::None
        )
        : _color{color}, _pieceType{pieceType}, _from{from}, _to{to}
            , _oCapturedP{oCapturedP}
    {}

    Move(Color color, PieceType pieceType, Pos from, Pos to
            , const PieceP& capturedP
        ) : Move(color, pieceType, from, to, std::make_optional<PieceP>(capturedP))
    {}

    Color      color()      const { return _color;     }
    PieceType  pieceType()  const { return _pieceType; }
    const Pos& from()       const { return _from;      }
    const Pos& to()         const { return _to;        }

    // CheckType  checkType()  const { return _checkType; }

    const OptPieceP oCapturedP() const { return _oCapturedP; }
    bool isCapture()   const { return _oCapturedP != std::nullopt; }
    bool isPromotion() const { return _promotedType != PieceType::None; }

    void apply(Board& board) const;
    void applyUndo(Board& board) const;
    const string getAlgNotation() const;

    bool operator==(const Move& other) const;

private:
    static PieceType2CaptureRule createCaptureRules();
    static PieceType2MoveRule    createMoveRules();

    static PieceType2CaptureRule pieceType2CaptureRule;
    static PieceType2MoveRule    pieceType2MoveRule;

    Color     _color;
    PieceType _pieceType;
    Pos       _from;
    Pos       _to;

    // CheckType _checkType;

    OptPieceP _oCapturedP;
    bool      _isEnPassant;
    PieceType _promotedType;

    bool operator<(const Move& other) const;

    friend ostream& operator<<(ostream& os, const Move& move);
};

PieceType2CaptureRule Move::pieceType2CaptureRule = Move::createCaptureRules();
PieceType2MoveRule    Move::pieceType2MoveRule    = Move::createMoveRules();

CaptureRule Move::getCaptureRule(PieceType pieceType)
{
    return Move::pieceType2CaptureRule.at(pieceType);
}
MoveRule Move::getMoveRule(PieceType pieceType)
{
    return Move::pieceType2MoveRule.at(pieceType);
}

// Similar to getValidPlayerMoves.
bool canCaptureImpl( const Board& board , const Piece& attacker , const Piece& target
               , const Dirs& dirs, Short maxSteps=0)
{
    if (maxSteps == 0) { maxSteps = std::max(BOARD_COLS, BOARD_ROWS); }

    Pos dest{attacker.pos()};
    for (Dir dir : dirs) {
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            if (!board.containsPos(dest)) {
                break;  // Done stepping in this direction
            }
            auto oPieceAtDestP = board.pieceAt(dest);
            if (oPieceAtDestP == std::nullopt) {
                continue;
            } else {
                if (oPieceAtDestP->get()->color() == target.color()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool pawnCaptureRule(const Board& b, const Piece& attacker, const Piece& target);

PieceType2CaptureRule Move::createCaptureRules()
{
    PieceType2CaptureRule pieceType2CaptureRule;

    pieceType2CaptureRule[PieceType::King]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return canCaptureImpl(b, attacker, target, allDirs, 1); };
    pieceType2CaptureRule[PieceType::Queen]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return canCaptureImpl(b, attacker, target, allDirs); };
    pieceType2CaptureRule[PieceType::Rook]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return canCaptureImpl(b, attacker, target, orthoDirs); };
    pieceType2CaptureRule[PieceType::Bishop]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return canCaptureImpl(b, attacker, target, diagDirs); };
    pieceType2CaptureRule[PieceType::Knight]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return canCaptureImpl(b, attacker, target, knightDirs, 1); };
    pieceType2CaptureRule[PieceType::Pawn]
        = [&](const Board& b, const Piece& attacker, const Piece& target)
              { return pawnCaptureRule(b, attacker, target); };

    return pieceType2CaptureRule;
}

Moves getValidPieceMoves(
    const Board& board, Color color, Pos pos, PieceType pieceType, Dirs dirs
    , Short maxSteps=0
    , CaptureAbility captureAbility=CaptureAbility::CanCapture
    );

const Moves pawnMoveRule(const Board& b, Color c, const Pos& pos);

PieceType2MoveRule Move::createMoveRules()
{
    PieceType2MoveRule pieceType2MoveRule;

    pieceType2MoveRule[PieceType::King]
        = [&](const Board& b, const auto c, const Pos& from)
              {
                return getValidPieceMoves(b, c, from, PieceType::King,   allDirs,   1);
              };
    pieceType2MoveRule[PieceType::Queen]
        = [&](const Board& b, const auto c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Queen,  allDirs);
              };
    pieceType2MoveRule[PieceType::Rook]
        = [&](const Board& b, const auto c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Rook,   orthoDirs);
              };
    pieceType2MoveRule[PieceType::Bishop]
        = [&](const Board& b, const auto c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Bishop, diagDirs);
              };
    pieceType2MoveRule[PieceType::Knight]
        = [&](const Board& b, const auto c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Knight, knightDirs, 1);
              };
    pieceType2MoveRule[PieceType::Pawn]
        = [&](const auto& b, const auto c, const auto& from)
              {
                  return pawnMoveRule(b, c, from);
              };

    return pieceType2MoveRule;
}

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

void Move::apply(Board& board) const
{
    board.movePiece(_from, _to);
}

void Move::applyUndo(Board& board) const
{
    enum class MoveType {Simple, CastleK, CastleQ, EnPassant, PawnPromotion};
    // ---------- Determine type of move being undone
    MoveType moveType = MoveType::Simple;
    if (_pieceType == PieceType::King) {
        Short xdiff = abs(_from.x - _from.y);
        if (xdiff == 2) { moveType = MoveType::CastleK; }
        else if (xdiff == 3) { moveType = MoveType::CastleQ; }
    }
    if (_pieceType == PieceType::Pawn) {
        if (_promotedType != PieceType::None) {
            moveType = MoveType::PawnPromotion;
        } else if (_isEnPassant) {
            assert(_to.toRelRow(_color) == BOARD_ROWS - 1);
            moveType = MoveType::EnPassant;
        }
    }

    // ---------- Undo move
    // Restore Piece type
    Piece& movedPiece = *(board.pieceAt(_to)->get());
    if (moveType == MoveType::PawnPromotion) {
        movedPiece.setPieceType(PieceType::Pawn);
    }

    // Restore Piece location
    board.pieceAt(_to)->get()->moveTo(_from);

    // Restore captured piece
    if (moveType == MoveType::EnPassant) {
        // Restore captured Pawn
        board.addPieceTo(opponent(movedPiece.color()), PieceType::Pawn, _to.index());
    } else if (_oCapturedP != std::nullopt) {
        // Restore captured piece
        board.addPieceTo(opponent(movedPiece.color()), _oCapturedP->get()->pieceType(), _to.index());
    }

    // Restore locations of Rook involved in en passant
    if (moveType == MoveType::CastleK) {
        // Restore K-side Rook
        // Configuration note: Adjust for different board sizes
        const Pos& from = Pos(5, 0).fromRelPos(_color);
        const Pos& to   = Pos(7, 0).fromRelPos(_color);
        board.pieceAt(from)->get()->moveTo(to);
    } else if (moveType == MoveType::CastleQ) {
        // Restore Q-side Rook
        const Pos& from = Pos(3, 0).fromRelPos(_color);
        const Pos& to   = Pos(0, 0).fromRelPos(_color);
        board.pieceAt(from)->get()->moveTo(to);
    }
}

// bool canBeCaptured(Board& b, Color c, const Pos& targetPos)
// {
//     const Piece& target = *(b.pieceAt(targetPos)->get());
//     auto& attackers = b.color2PiecePs.at(opponent(c));
//     for (const PieceP& attackerP : attackers) {
//         CaptureRule captureRule = Move::getCaptureRule(attackerP->pieceType());
//         if (captureRule(b, *attackerP, target)) {
//             return true;
//         }
//     }
//     return false;
// }

const string Move::getAlgNotation() const
{
    return _from.getAlgNotation() + ' ' + _to.getAlgNotation();
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
    , const Color& color
    , const Pos2Moves& validPlayerMoves
    )
{
    const string& help = "\tEnter moves as a pair of board positions in algebraic notation. E.g.: e2 e4";
    const string& prompt = "Enter move (? for help): ";
    cout << Player::getPlayerName(color) << "'s turn\n";

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
                cout << "List of valid moves:\n";
                for (const auto& [from, moves] : validPlayerMoves) {
                    PieceType pt = board.pieceAt(from)->get()->pieceType();
                    cout << "  Moves of " << showPieceType(pt)
                         << " @ " <<  from.getAlgNotation() << " (" << moves.size() << "): ";
                    for (const Move& move : moves) {
                        cout << move.to().getAlgNotation() << ' ';
                    }
                    cout << "\n";
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
            return move;
        }
        catch (const std::invalid_argument& ex) {
            cout << ex.what();
        }
    }
}

bool doesMoveSelfCheck(const Board& board, Color color, const Move& move) noexcept {
    move.apply(const_cast<Board&>(board));  // Temp board alteration
    bool result = false;
    const Piece& king = *(board.getKingP(color));
    for (const PieceP& attackerP : board.color2PiecePs.at(opponent(color))) {
        CaptureRule canCapture = Move::getCaptureRule(attackerP->pieceType());
        if (canCapture(board, *attackerP, king)) {
            result = true;
            break;
        }
    }
    move.applyUndo(const_cast<Board&>(board));  // Undo temp board alteration
    return result;
}

// Similar to canCapture.
const Pos2Moves getValidPlayerMoves(const Board& board, const Color& color)
{
    Pos2Moves result;
    for (const PieceP& pieceP : board.piecesWithColor(color)) {
        MoveRule moveRule = Move::getMoveRule(pieceP->pieceType());
        const Pos& from = pieceP->pos();
        for (Move move : moveRule(board, pieceP->color(), from)) {
            if (!doesMoveSelfCheck(board, color, move)) {
                result[from].insert(result[from].end(), move);
            }
        }
    }
    return result;
}

Moves getValidPieceMoves(
    const Board& board, Color color, Pos pos, PieceType pieceType, Dirs dirs
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
            if (!board.containsPos(dest)) {
                break;  // Done stepping in this direction
            }
            auto oPieceAtDestP = board.pieceAt(dest);
            if (oPieceAtDestP == std::nullopt) {
                if (captureAbility == CaptureAbility::MustCapture) {
                    break;
                }
                // Non-capture move
                result.emplace(result.end(), color, pieceType, pos, dest);  // Capture move
            } else {
                if (captureAbility == CaptureAbility::MustNotCapture) {
                    break;
                }
                if (oPieceAtDestP->get()->color() == color) {
                    break;  // Cannot capture one's own piece
                }
                // Capture move
                result.emplace(result.end(), color, pieceType, pos, dest);  // Non-capture move
                break;  // Cannot move past opponent's piece
            }
        }
    }
    return result;
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

bool pawnCaptureRule(const Board& b, const Piece& attacker, const Piece& target)
{
    // Capture diagonally
    Dir forward = Player::getForward(attacker.color());
    Dirs captureDirs = Dirs{forward + Dir{-1,0}, forward + Dir{1, 0}};
    for (const Dir& captureDir : captureDirs) {
        if (attacker.pos() + captureDir == target.pos()) {
            return true;
        }
    }
    // TODO: En passant
    return false;
}

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

    // En passant capture
    // TODO: if (is en passant possible) { add en passant move }
    return result;
}
