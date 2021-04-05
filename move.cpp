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

#include <regex>
#include <string>

#include "util.h"
#include "geometry.h"
#include "player.h"
#include "piece.h"
#include "board.h"
#include "move.h"

#include "logger.h"

using std::string, std::ostream, std::to_string;
using std::cin, std::cout, std::ostringstream;
using std::pair, std::map, std::vector;
// using GetPlayerMove = std::function<ExtMove(const Board&, Color, const Pos2Moves&)>;

// ========================================
// enum MoveType

ostream& operator<<(ostream& os, MoveType moveType) {
    static const map<MoveType, const char*> mt2str{
        {MoveType::Simple, "Simple_move"}
        , {MoveType::CastleK, "King-side_castle"}
        , {MoveType::CastleQ, "Queen-side_castle"}
        , {MoveType::EnPassant, "En_passant"}
        , {MoveType::PawnPromotion, "Pawn_promotion"}
    };
    os << mt2str.at(moveType);
    return os;
}

// ---------- Initialization of static data
PieceType2IsAttackingRule Move::_pieceType2IsAttackingRule = Move::_createIsAttackingRules();
PieceType2MoveRule        Move::_pieceType2MoveRule = Move::_createMoveRules();
Moves                     Move::_moveHistory        = Move::_createHistory();

// ---------- Public static methods
const IsAttackingRule& Move::getIsAttackingRule(PieceType pt)
{
    return Move::_pieceType2IsAttackingRule.at(pt);
}

const MoveRule& Move::getMoveRule(PieceType pt)
{
    return Move::_pieceType2MoveRule.at(pt);
}

const string Move::history_to_pgn()
{
    ostringstream oss;
    for (Short k = 0; (unsigned long) k < _moveHistory.size(); ++k) {
        if (k % 2 == 0) { oss << k/2 + 1 << ". "; }
        oss << _moveHistory[k].to_pgn() << ' ';
    }
    return oss.str();
}

// ---------- Public static methods (attacking / moving rules)

bool Move::isAttacked(const Board& b, const Pos& tgtPos, Color tgtColor)
{
    auto& attackers = b.color2PiecePs.at(opponent(tgtColor));
    for (const PieceP& attackerP : attackers) {
        IsAttackingRule isAttackingRule = Move::getIsAttackingRule(attackerP->pieceType());
        if (isAttackingRule(b, *attackerP, tgtPos)) {
            return true;
        }
    }
    return false;
}

// The function getValidPieceMoves does something similar, but its result is a collection of Moves.
bool Move::isAttacking(
        const Board& b, const Piece& attacker, const Pos& tgtPos
        , const Dirs& dirs, Short maxSteps // =0
        )
{
    if (maxSteps == 0) { maxSteps = std::max(BOARD_COLS, BOARD_ROWS); }

    // Performance optimization:
    //     Can skip some directions early if they don't lead to the target.
    //     Or determine the needed direction, and check to see if it is in dirs.
    for (Dir dir : dirs) {
        Pos dest{attacker.pos()};
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            if (!dest.isOnBoard()) {
                break;  // Fell off board---done stepping in this direction
            }
            if (dest == tgtPos) {
                return true;
            }
            const PieceP& pieceAtDestP = b.pieceAt(dest);
            if (!pieceAtDestP) {
                continue;  // Empty space
            } else {
                break;  // Can't go past a piece of either color.
            }
        }
    }
    return false;
}

bool Move::isInCheck(const Board& b, Color c) noexcept {
    const Piece& king = b.king(c);
    assert(king.pieceType() == PieceType::King);
    for (const PieceP& attackerP : b.color2PiecePs.at(opponent(c))) {
        IsAttackingRule isAttackingRule = Move::getIsAttackingRule(attackerP->pieceType());
        if (isAttackingRule(b, *attackerP, king.pos())) {
            return true;
        }
    }
    return false;
}

bool Move::pawnIsAttackingRule(const Board& b, const Piece& attacker, const Pos& tgtPos)
{
    // Capture diagonally
    Dir forward = Player::forward(attacker.color());
    Dirs captureDirs = Dirs{forward + Dir{-1,0}, forward + Dir{1, 0}};
    for (const Dir& captureDir : captureDirs) {
        if (attacker.pos() + captureDir == tgtPos) {
            return true;
        }
    }

    // Capture en passant
    if (  attacker.pos().toRelRow(attacker.color()) == 5
       && attacker.row() == tgtPos.y           // Same row
       && abs(attacker.col() - tgtPos.x) == 1  // Adjacent col
       && b.pieceAt(tgtPos) != nullptr         // Piece @ tgtPos
       )
    {
        Color oppColor = opponent(attacker.color());
        const Piece& opp = *(b.pieceAt(tgtPos).get());
        if (  oppColor == opponent(attacker.color())
           && opp.pieceType() == PieceType::Pawn
           && opp.lastMoveIndex() == b.currentMoveIndex() - 1
           )
        {
            // Opponent's Pawn @ e.p. location. Did it advance 2 steps on prev move?
            const Move& prev = Move::prevMove();
            if (  prev._pieceType == PieceType::Pawn
               && prev.to().toRelRow(oppColor) - prev.from().toRelRow(oppColor) == 2)
            {
                return true;
            }
        }
    }
    return false;
}

const Moves Move::pawnMoveRule(const Board& b, Color c, const Pos& pos)
{
    Moves result{};

    // Move forward w/o capture
    Short maxStepCount = pos.isPawnInitialPosition(c) ? 2 : 1;
    const auto& marchMoves
        = getValidPieceMoves( b, c, pos, PieceType::Pawn, std::set{Player::forward(c)}
                            , maxStepCount
                            , CaptureAbility::MustNotCapture
                            );
    result.insert(result.end(), marchMoves.begin(), marchMoves.end());

    // Standard capture
    Dir forward = Player::forward(c);
    Dirs captureDirs = Dirs{forward + Dir{-1,0}, forward + Dir{1, 0}};
    const auto& captureMoves
        = getValidPieceMoves( b, c, pos, PieceType::Pawn, captureDirs
                            , 1
                            , CaptureAbility::MustCapture
                            );
    result.insert(result.end(), captureMoves.begin(), captureMoves.end());

    // En passant
    if (pos.toRelRow(c) == BOARD_EN_PASSANT_FROM_ROW) {
        Dirs lateralDirs = Dirs{Dir{-1,0}, Dir{1, 0}};
        for (const Dir& lateralDir : lateralDirs) {
            const Pos& oppPos = pos + lateralDir;
            const PieceP oppPieceP = b.pieceAt(oppPos);
            if (!oppPieceP) {
                continue;
            }
            const Piece& opp = *oppPieceP.get();
            if (  opp.color() == opponent(c)
               && opp.pieceType() == PieceType::Pawn
               && opp.lastMoveIndex() == b.currentMoveIndex() - 1
               )
            {
                const Move& prevMove = Move::prevMove();
                if (  prevMove.pieceType() == PieceType::Pawn
                   && prevMove.to() == opp.pos()
                   && abs(prevMove.to().ydiff(prevMove.from())) == 2
                   )
                {
                    Move enPassantMove
                        = Move( c, PieceType::Pawn, pos, (pos + forward) + lateralDir
                              , oppPieceP, true, true
                              );
                    result.insert(result.end(), enPassantMove);
                }
            }
        }
    }
    return result;
}

// ---------- Public static methods (Get move / Interactivity / Strategy)

ExtMove getPlayerMoveError{std::nullopt, false, GameEnd::InPlay};

ExtMove Move::getPlayerMove(PlayerType playerType
        , const Board& b, Color c, const Pos2Moves& validPlayerMoves
        )
{
    ExtMove result{};
    switch (playerType) {
        case PlayerType::Human:
            result = Move::queryPlayerMove(b, c, validPlayerMoves);
            break;
        case PlayerType::Computer_Random:
            result = Move::strategyRandom(b, c, validPlayerMoves);
            break;
        case PlayerType::Computer_RandomCapture:
            result = Move::strategyRandomCapture(b, c, validPlayerMoves);
            break;
    }
    return result;
}

// The function isAttacking does something similar but with a boolean result.
Moves Move::getValidPieceMoves(
    const Board& b, Color c, Pos pos, PieceType pt, Dirs dirs
    , Short maxSteps  // =0
    , CaptureAbility captureAbility  // =CaptureAbility::CanCapture
    )
{
    Logger::trace("  getValidPieceMoves: Entering. ", c, pt, "_@_", pos);
    Moves result{};

    if (maxSteps == 0) { maxSteps = std::max(BOARD_COLS, BOARD_ROWS); }

    for (Dir dir : dirs) {
        Pos dest{pos};
        for (Short stepCount = 1; stepCount <= maxSteps; ++stepCount) {
            dest = dest + dir;  // Step in direction dir
            ostringstream moveDesc_oss;
            moveDesc_oss << c << pt << "_@_" << pos << "-->" << dest << " (dir=" << dir << ')';
            string moveDesc = moveDesc_oss.str();
            Logger::trace("    getValidPieceMoves: Checking move: ", moveDesc);
            if (!dest.isOnBoard()) {
                break;  // Done stepping in this direction
            }
            auto pieceAtDestP = b.pieceAt(dest);
            if (!pieceAtDestP) {
                if (captureAbility == CaptureAbility::MustCapture) {
                    Logger::trace("    getValidPieceMoves: ", moveDesc
                        , ": Pawn cannot move diagonally without capture @ ", dest);
                    break;
                }

                // Non-capture move
                Logger::trace("    getValidPieceMoves: ", moveDesc
                    , ": Found valid non-capture move");
                result.emplace(result.end(), c, pt, pos, dest
                        , nullptr
                        , pt == PieceType::Pawn
                        , false
                        );
                // Continue loop: Continue stepping in this direction
            } else {
                if (captureAbility == CaptureAbility::MustNotCapture) {
                    Logger::trace("    getValidPieceMoves: ", moveDesc
                        , ": Pawn cannot move forward onto occupied space @ ", dest);
                    break;
                }
                if (pieceAtDestP.get()->color() == c) {
                    Logger::trace("    getValidPieceMoves: ", moveDesc
                        , ": Cannot capture own piece");
                    break;  // Cannot capture own piece
                }

                // Capture move
                Logger::trace(" getValidPieceMoves: ", moveDesc, ": Valid capture move");
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
        Logger::trace( "    ==> getValidPieceMoves: ", c, pt, "_@_", pos
                    , ": ", result.size(), " valid moves"
                    );
    }
    return result;
}

// Aggregates valid moves from the Player's pieces' Move rules.
const Pos2Moves Move::getValidPlayerMoves(const Board& b, Color c)
{
    Pos2Moves result;

    for (const PieceP& pieceP : b.piecesWithColor(c)) {
        MoveRule moveRule = Move::getMoveRule(pieceP->pieceType());
        const Pos& from = pieceP->pos();
        Logger::trace("getValidPlayerMoves(", c, "): Looking at piece ", *pieceP);
        const Moves& moves = moveRule(b, pieceP->color(), from);
        for (Move move : moves) {
            if (!move.doesCauseSelfCheck(b, c)) {
                result[from].insert(result[from].end(), move);
            }
        }
    }
    Logger::trace( "getValidPlayerMoves(", c, "): "
               , "Returning moves from ", result.size(), " positions: ", result
               );
    return result;
}


ExtMove Move::queryPlayerMove(
    const Board& b
    , Color c
    , const Pos2Moves& validPlayerMoves
    )
{
    DrawableFlags drawableFlags = Drawable_None;
    if (b.maxBoardRepetitionCount(c) >= 3) {
        drawableFlags |= Drawable_3xRepetition;
    }
    if (b.movesSinceLastPmoc() >= 50) {
        drawableFlags |= Drawable_50MoveRule;
    }

    const string& helpMsg =
        "Piece moves:\n"
        "  * Simple moves: Entered as a pair (from & to) of board positions in algebraic notation.\n"
        "      - Example: f1 d3 <Enter>\n"
        "      - When capturing en passant, the program will detect the capture details.\n"
        "      - When moving a Pawn to the farthest row, follow with a letter indicating promoted type: Q, R, B, or N.\n"
        "          If no promotion piece type is given, Queen is chosen as a default.\n"
        "      - When castling, enter the pair of board positions for the King only.\n"
        "  * Post-move Draw claim:\n"
        "      - Example: f1 d3 draw <Enter>\n"
        "      - If one of the two conditions above (or both!) will exist after a player moves a piece,\n"
        "        the player can enter the word 'draw' after the from & to coordinates entered.\n"
        "\n"
        "Other types of \"moves\":\n"
        "  * Pre-move Draw claim:\n"
        "      - Example: draw <Enter>\n"
        "      - Two conditions can used to claim a Draw:\n"
        "          o A 3x board repetition\n"
        "          o 50 moves without a Pawn move or capture\n"
        "          o (Note: The mandatory Draw conditions (5x repetition, 75 Move Rule, insufficient resources) take effect automatically.)\n"
        "      - If either of these conditions exist, the player is informed before move entry.\n"
        "  * Concede:\n"
        "      - Example: concede <Enter>\n"
        "  * Propose that the other player concede:\n"
        "      - Example: win? <Enter>\n"
        "      - The other player will be asked to accept or reject. If rejected, play returns to the proposing player.\n"
        "  * Propose a draw:\n"
        "      - Example: draw? <Enter>\n"
        "      - The other player will be asked to accept or reject. If rejected, play returns to the proposing player.\n"
        "\n"
        "Special commands:\n"
        "  * board:   Print out the board.\n"
        "  * history: Show move history in a compact, but easily-readable format.\n"
        "  *   pgn: Show move history in a verbose PGN input format.\n"
        "  * moves:   Show legal moves.\n"
        "  * pieces:  List the pieces on the board.\n"
        "\n"
        "  * log_level: Display the current log reporting level.\n"
        "  * log_error, log_warn, log_info, log_debug, log_trace: Change the current log reporting level.\n"
        "\n"
        "  * exit / quit: Exit the game. (There will not be amatch summmary printed.)\n"
        "\n"
        ;

    ostringstream prompt_oss;
    prompt_oss << "Enter move #" << to_string(b.currentMoveIndex())
               << (drawableFlags == Drawable_None ? "" : " or 'draw' to claim draw")
               << " (or '?' for more options): "
               ;

    PosMovesComparator pmComparator{b};

    cout << "========================================\n";
    cout << (c == Color::Black ? "Black" : "White") << " (" << Player::playerName(c) << ") to play.\n";
    if (Move::prevMove().isCheck()) { cout << "You are in check.\n"; }
    if (drawableFlags != Drawable_None) {
        cout << "Game can be called a draw: ";
        if ((drawableFlags & Drawable_3xRepetition) != Drawable_None) { cout << "3x Repetition. "; }
        if ((drawableFlags & Drawable_50MoveRule)   != Drawable_None) { cout << "50 Move Rule. "; }
        cout << "\n";
    }
    while (true) {
        cout << prompt_oss.str();
        string input;
        getline(cin, input);

        std::regex blankRegex{"^\\s*$"};
        std::smatch blankMatch;
        if (std::regex_match(input, blankMatch, blankRegex)) {
            continue;
        }

        std::regex singleCmdRegex{"^\\s*(\\S+)\\s*$"};
        std::smatch singleCmdMatch;
        if (std::regex_match(input, singleCmdMatch, singleCmdRegex)) {
            string cmd = singleCmdMatch[1];

            Logger::write("Command entered: ", cmd, "\n");

            // Commands not involving piece movement
            if (cmd == "?" || cmd == "help") {
                cout << helpMsg << "\n";
                continue;
            }
            if (cmd == "board") {
                cout << b << "\n";
                continue;
            }
            if (cmd == "concede") {
                return ExtMove( std::nullopt, false
                              , c == Color::Black ? GameEnd::WinWhite : GameEnd::WinBlack
                              );
            }
            if (cmd == "draw") {
                // Player claiming pre-move Draw claim (via 3x Repetition or 50 Move Rule)
                if (drawableFlags != Drawable_None) {
                    return ExtMove(std::nullopt, true, GameEnd::InPlay);
                } else {
                    cout << "There are no draw-claimable conditions present.\n";
                    continue;
                }
            }
            if (cmd == "draw?") {
                if (Player::playerType(opponent(c)) != PlayerType::Human) {
                    cout << "Cannot offer a draw to a non-human Player.\n";
                    return getPlayerMoveError;
                }
                bool isDrawAccepted =
                    Player::offerBool( std::make_optional<Color>(opponent(c))
                                     , "Do you accept a Draw (y/n)? "
                                     );
                if (isDrawAccepted) {
                    return ExtMove(std::nullopt, true, GameEnd::Draw);
                }
            }
            if (cmd == "history") {
                cout << Move::getMoveHistory() << "\n";
                continue;
            }

            if (cmd == "log_level") {
                LogLevel ll = Logger::reportLevel();
                cout << "Log reporting level = "
                     << ( ll == LogError ? "LogError"
                        : ll == LogWarn ?  "LogWarn"
                        : ll == LogInfo ?  "LogInfo"
                        : ll == LogDebug ? "LogDebug"
                        : ll == LogTrace ? "LogTrace"
                        : "Unknown!"
                        )
                     << "\n"
                     ;
                continue;
            }
            if (cmd == "log_error") {
                Logger::setReportLevel(LogError);
                cout << "Log level set to LogError";
                continue;
            }
            if (cmd == "log_warn")  {
                Logger::setReportLevel(LogWarn);
                cout << "Log level set to LogWarn";
                continue;
            }
            if (cmd == "log_info")  {
                Logger::setReportLevel(LogInfo);
                cout << "Log level set to LogInfo";
                continue;
            }
            if (cmd == "log_debug") {
                Logger::setReportLevel(LogDebug);
                cout << "Log level set to LogDebug";
                continue;
            }
            if (cmd == "log_trace") {
                Logger::setReportLevel(LogTrace);
                cout << "Log level set to LogTrace";
                continue;
            }

            if (cmd == "moves") {
                cout << "List of valid moves:\n";
                vector<pair<Pos, Moves>> playerMoves = mapToVector(validPlayerMoves);
                std::sort(playerMoves.begin(), playerMoves.end(), pmComparator);
                for (const auto& [from, moves] : playerMoves) {
                    PieceType pt = b.pieceAt(from).get()->pieceType();
                    cout << "  Moves of " << pt
                         << " @ " <<  from.algNotation() << " (" << moves.size() << "): ";
                    for (const Move& move : moves) {
                        cout << move.to().algNotation() << ' ';
                    }
                    cout << "\n";
                }
                continue;
            }
            if (cmd == "pieces") {
                b.printPieces();
                continue;
            }
            if (cmd == "pgn") {
                cout << Move::history_to_pgn() << "\n";
                continue;
            }

            if (cmd == "repetitions") {
                b.printBoardHashRepetitions();
            }
            // if (cmd == "undo") {
            //     This would modify Board, which is const.
            //     if (Move::_moveHistory.size() < 2) {
            //         cout << "Can only undo when two or more moves have been made.\n";
            //         continue;
            //     }
            //     // Undo last two moves, whether the opponent is a human or computer.
            //     const Move& prev = Move::_moveHistory.back();
            //     prev.applyUndo(b);
            //     const Move& prev2 = Move::_moveHistory.back();
            //     prev2.applyUndo(b);
            //     continue;
            // }
            if (cmd == "win?") {
                if (Player::playerType(opponent(c)) != PlayerType::Human) {
                    cout << "Cannot propose resigning to a non-human Player.\n";
                    return getPlayerMoveError;
                }
                bool isLossAccepted =
                    Player::offerBool( std::make_optional<Color>(opponent(c))
                                     , "Do you agree to concede (y/n)? "
                                     );
                if (isLossAccepted) {
                    return ExtMove( std::nullopt, true
                                  , c == Color::Black ? GameEnd::WinBlack : GameEnd::WinWhite
                                  );
                }
            }
            if (cmd == "exit" || cmd == "quit") {
                exit(0);
            }
            cout << "Unrecognized command: " << cmd << "\n";
            continue;
        }

        ExtMove extMove = _parseMoveInAlgNotation(b, c, input);
        if (extMove.isErrorValue()) {
            continue;
        }
        Move& move = *(extMove.optMove);
        Logger::trace("queryPlayerMove: Getting valid piece moves: from=", move.from());
        if (validPlayerMoves.find(move.from()) == validPlayerMoves.end()) {
            cout << "There are no moves from that board location." << "\n";
            continue;
        }
        Moves validPieceMoves = validPlayerMoves.at(move.from());
        const auto& beg = validPieceMoves.begin();
        const auto& end = validPieceMoves.end();
        if (find(beg, end, move) == end) {
            cout << "That is not a legal move." << "\n";
            continue;
        }
        return extMove;
    }
}

ExtMove Move::randomMove(Color c, const Moves& moves) {
    std::uniform_int_distribution<int> randIntGen{0, (int) moves.size() - 1};
    int randInt = randIntGen(prng());
    Move move = moves[randInt];
    if (move._pieceType == PieceType::Pawn && move._to.isPawnPromotionRow(c)) {
        move._oPromotedTo = std::make_optional<PieceType>(PieceType::Queen);
    }
    OptMove om = std::make_optional<Move>(move);
    bool isDrawClaim = false;
    GameEnd agreedGameEnd = GameEnd::InPlay;
    return ExtMove(om, isDrawClaim, agreedGameEnd);
}

ExtMove Move::strategyRandom(
    [[maybe_unused]] const Board& b
    , [[maybe_unused]] Color c
    , const Pos2Moves& validPlayerMoves
    )
{
    const Moves& moves = concatMap(validPlayerMoves);
    return Move::randomMove(c, moves);
}

ExtMove Move::strategyRandomCapture(
    [[maybe_unused]] const Board& b
    , [[maybe_unused]] Color c
    , const Pos2Moves& validPlayerMoves
    )
{
    const Moves& allMoves = concatMap(validPlayerMoves);
    Moves captureMoves{};
    std::copy_if( allMoves.begin(), allMoves.end(), std::back_inserter(captureMoves)
                , [&](const Move& m) { return m.isCapture(); }
                );
    const Moves& moves = captureMoves.size() == 0 ? allMoves : captureMoves;
    return Move::randomMove(c, moves);
}

// ---------- Constructors
Move::Move( Color color, PieceType pt, const Pos from, const Pos to
    , PieceP capturedP /* =nullptr */
    , bool isPawnMove /* =false */
    , bool isEnPassant /* =false */
    , OptPieceType promotedType /* =std::nullopt */
    )
    : _color{color}, _pieceType{pt}, _from{from}, _to{to}
    , _capturedP{capturedP}
    , _isPawnMove{isPawnMove}
    , _isEnPassant{isEnPassant}
    , _oPromotedTo{promotedType}
    , _isCheck{false}
    , _isCheckmate{false}
{
    assert(from.isOnBoard() && to.isOnBoard());
}

// ---------- Public read methods

bool Move::isCastling() const {
    return _pieceType == PieceType::King && abs(_to.xdiff(_from)) == 2;
}

bool Move::isCastlingK() const {
    return _pieceType == PieceType::King && _to.xdiff(_from) == 2;
}

bool Move::isCastlingQ() const {
    return _pieceType == PieceType::King && _to.xdiff(_from) == -2;
}

// Verbose input PGN format
const string Move::to_pgn() const
{
    ostringstream oss;
    if (_pieceType == PieceType::King && isCastling()) {
        Col xdiff = _to.xdiff(_from);
        if (xdiff == 2) { oss << "O-O"; }
        if (xdiff == -2) { oss << "O-O-O"; }
    } else {
        oss << _pieceType << _from;
        if (capturedP()) { oss << 'x'; }
        oss << _to;
        if (isPromotion()) { oss << '=' << promotionType(); }
    }
    if (isCheck()) { oss << '+'; }
    if (isCheckmate()) { oss << '#'; }
    if (isEnPassant()) { oss << " {e.p.}"; }
    return oss.str();
}

// ---------- Public read methods (inspection)

bool Move::doesCauseSelfCheck(const Board& b, Color c) const noexcept {
    Logger::trace("      Move::doesCauseSelfCheck: pushing move: ", *this
            , ". Dest ", _to, b.isEmpty(_to) ? " is empty" : " is not empty");
    apply(const_cast<Board&>(b));  // Temp board alteration
    bool result = isInCheck(b, c);
    Logger::trace("      Move::doesCauseSelfCheck: popping move: ", *this
            , ". Does ", result ? "" : "not ", "self-check.");
    applyUndo(const_cast<Board&>(b));  // Undo temp board alteration
    return result;
}

// ---------- Public read methods (Board modification)
void Move::apply(Board& b) const {
    Logger::trace("Move::apply: Entering. move=", *this, ", board=\n", b);

    // Capture, including en passant
    if (_capturedP) {
        b.removePieceAt(_capturedP.get()->pos());
        assert(!b.pieceAt(_capturedP.get()->pos()));
    }

    // Move & promote
    b.movePiece(_from, _to);
    if (isPromotion()) {
       b.pieceAt(_to).get()->setPieceType(*_oPromotedTo);
    }

    // Move secondary pieces
    if (_pieceType == PieceType::King) {
        if (isCastlingK()) {  // King-side castle
            Pos kRookFrom = Board::kRookInitPos(_color);
            Pos kRookTo = Board::kRookInitPos(_color).posLeft(2);
            b.movePiece(kRookFrom, kRookTo);
            b.pieceAt(kRookTo)->updateMoveIndexHistory(b.currentMoveIndex());
        } else if (isCastlingQ()) {  // Queen-side castle
            Pos qRookFrom = Board::qRookInitPos(_color);
            Pos qRookTo = Board::qRookInitPos(_color).posRight(3);
            b.movePiece(qRookFrom, qRookTo);
            b.pieceAt(qRookTo)->updateMoveIndexHistory(b.currentMoveIndex());
        }
    }

    // Update MoveIndex history
    b.pieceAt(_to)->updateMoveIndexHistory(b.currentMoveIndex());
    b.updatePmocHistory(_isPawnMove || isCapture());
    b.currentMoveIndex_incr();
    Move::_moveHistory.push_back(*this);

    Logger::trace("Move::apply: Exiting. move=", *this);
}

void Move::applyUndo(Board& b) const
{
    Logger::trace("Move::applyUndo: Entering. move=", *this);

    b.currentMoveIndex_decr();
    b.rollBackPmocHistory();
    b.pieceAt(_to)->rollBackLastMoveIndex(b.currentMoveIndex());
    Move::_moveHistory.pop_back();

    // Determine type of move being undone
    MoveType moveType = MoveType::Simple;
    if (_pieceType == PieceType::King) {
        Short xdiff = _to.xdiff(_from);
        if (xdiff == 2) { moveType = MoveType::CastleK; }
        else if (xdiff == -2) { moveType = MoveType::CastleQ; }
    }
    if (_pieceType == PieceType::Pawn) {
        if (_isEnPassant) {
            moveType = MoveType::EnPassant;
        } else if (_oPromotedTo != std::nullopt) {
            moveType = MoveType::PawnPromotion;
            assert(_to.toRelRow(_color) == BOARD_PAWN_PROMOTION_ROW);
        }
    }
    Logger::trace("Move::applyUndo: moveType=", moveType, ", move=", *this);

    // Restore locations of secondary pieces (castled Rooks)
    if (moveType == MoveType::CastleK) {
        // Restore K-side Rook
        const Pos& to   = Board::kRookInitPos(_color);
        const Pos& from = to.posLeft(2);
        b.movePiece(from, to);
        Piece& moved = *b.pieceAt(to);
        moved.rollBackLastMoveIndex(b.currentMoveIndex());
    } else if (moveType == MoveType::CastleQ) {
        // Restore Q-side Rook
        const Pos& to   = Board::qRookInitPos(_color);
        const Pos& from = to.posRight(3);
        b.movePiece(from, to);
        Piece& moved = *b.pieceAt(to);
        moved.rollBackLastMoveIndex(b.currentMoveIndex());
    }

    // Restore Piece type (un-promote)
    Piece& movedPiece = *(b.pieceAt(_to).get());
    if (moveType == MoveType::PawnPromotion) {
        movedPiece.setPieceType(PieceType::Pawn);
    }

    // Restore Piece location (un-move)
    b.movePiece(_to, _from);

    // Restore captured piece
    if (moveType == MoveType::EnPassant) {
        // Restore captured Pawn
        Short restorePos = (_to + Player::backward(movedPiece.color())).index();
        b.addPieceTo(opponent(movedPiece.color()), PieceType::Pawn, restorePos);
    } else if (_capturedP) {
        // Restore captured piece
        b.addPieceTo(opponent(movedPiece.color()), _capturedP.get()->pieceType(), _to.index());
    }

    Logger::trace("Move::applyUndo: Exiting. move=", *this);
}

// ---------- Operators
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

// ---------- Private static methods
PieceType2IsAttackingRule Move::_createIsAttackingRules()
{
    PieceType2IsAttackingRule pt2cr;

    pt2cr[PieceType::King] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return isAttacking(b, attacker, tgtPos, Dir::allDirs(),    1); };
    pt2cr[PieceType::Queen] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return isAttacking(b, attacker, tgtPos, Dir::allDirs(),    0); };
    pt2cr[PieceType::Rook] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return isAttacking(b, attacker, tgtPos, Dir::orthoDirs(),  0); };
    pt2cr[PieceType::Bishop] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return isAttacking(b, attacker, tgtPos, Dir::diagDirs(),   0); };
    pt2cr[PieceType::Knight] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return isAttacking(b, attacker, tgtPos, Dir::knightDirs(), 1); };
    pt2cr[PieceType::Pawn] =
        [&](const Board& b, const Piece& attacker, const Pos& tgtPos)
            { return pawnIsAttackingRule(b, attacker, tgtPos); };

    return pt2cr;
}

Moves Move::_createHistory() {
    return Moves();
}

PieceType2MoveRule Move::_createMoveRules()
{
    PieceType2MoveRule pt2mr;

    pt2mr[PieceType::King]
        = [&](const Board& b, Color c, const Pos& from)
              {
                Moves kMoves = getValidPieceMoves(b, c, from, PieceType::King, Dir::allDirs(), 1);
                if (!b.king(c).hasMoved()
                   // Workaround for lastMoveIndex issue
                   && b.king(c).pos() == Board::kInitPos(c)
                   )
                {
                    // King-side castle
                    PieceP kRookP = b.pieceAt(b.kRookInitPos(c));
                    if (kRookP) {
                        const Piece& kRook = *(kRookP.get());
                        if (!kRook.hasMoved())
                        {
                            if (  !isAttacked(b, from, c)
                               && !isAttacked(b, from.posRight(1), c)
                               && !isAttacked(b, from.posRight(2), c)
                               && !isAttacked(b, from.posRight(3), c)
                               && b.isEmpty(from.posRight(1))
                               && b.isEmpty(from.posRight(2))
                               )
                            {
                                kMoves.emplace_back(c, PieceType::King, from, from.posRight(2));
                            }
                        }
                    }

                    // Queen-side castle
                    PieceP qRookP = b.pieceAt(b.qRookInitPos(c));
                    if (qRookP) {
                        const Piece& qRook = *(qRookP.get());
                        if (!qRook.hasMoved())
                        {
                            if (  !isAttacked(b, from, c)
                               && !isAttacked(b, from.posLeft(1), c)
                               && !isAttacked(b, from.posLeft(2), c)
                               && !isAttacked(b, from.posLeft(3), c)
                               && !isAttacked(b, from.posLeft(4), c)
                               && b.isEmpty(from.posLeft(1))
                               && b.isEmpty(from.posLeft(2))
                               && b.isEmpty(from.posLeft(3))
                               )
                            {
                                kMoves.emplace_back(c, PieceType::King, from, from.posLeft(2));
                            }
                        }
                    }
                }
                return kMoves;
              };
    pt2mr[PieceType::Queen]
        = [&](const Board& b, Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Queen,  Dir::allDirs());
              };
    pt2mr[PieceType::Rook]
        = [&](const Board& b, Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Rook,   Dir::orthoDirs());
              };
    pt2mr[PieceType::Bishop]
        = [&](const Board& b, Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Bishop, Dir::diagDirs());
              };
    pt2mr[PieceType::Knight]
        = [&](const Board& b, Color c, const Pos& from)
              {
                  return getValidPieceMoves(b, c, from, PieceType::Knight, Dir::knightDirs(), 1);
              };
    pt2mr[PieceType::Pawn]
        = [&](const auto& b, Color c, const auto& from)
              {
                  return pawnMoveRule(b, c, from);
              };

    return pt2mr;
}

ExtMove Move::_parseMoveInAlgNotation(const Board& b, Color c, const string& input) noexcept(false)
{
    std::regex extMoveRegex{"^\\s*(\\S+)\\s+(\\S+)(\\s+(\\S+))*$"};
    std::regex algNotationRegex{"^\\s*([a-z]+)([1-9][0-9]*)$"};

    std::smatch extMoveMatch;
    std::unique_ptr<Pos> fromP;
    std::unique_ptr<Pos> toP;
    OptPieceType promotedType = std::nullopt;
    bool isDrawClaim = false;
    if (std::regex_match(input, extMoveMatch, extMoveRegex)) {
        Short mCount = extMoveMatch.size();
        if (mCount != 5) {
            cout << "Input has the wrong format";
            return getPlayerMoveError;
        }
        string arg3 = extMoveMatch[4];  // ...[3] includes the leading blank space.
        if (arg3 != "") {
            if (arg3.size() == 1) {
                string promotableTypes{"QRBN"};
                if (promotableTypes.find(arg3) == std::string::npos) {
                    cout << "Not a valid promotion type: " << arg3 << "\n";
                }
                switch(arg3[0]) {
                    case 'Q':
                        promotedType = std::make_optional<PieceType>(PieceType::Queen);
                        break;
                    case 'R':
                        promotedType = std::make_optional<PieceType>(PieceType::Rook);
                        break;
                    case 'B':
                        promotedType = std::make_optional<PieceType>(PieceType::Bishop);
                        break;
                    case 'N':
                        promotedType = std::make_optional<PieceType>(PieceType::Knight);
                        break;
                    default:
                        cout << "Internal error: Unrecognized PieceType";
                        return getPlayerMoveError;
                }
            }
            else if (arg3 == "draw") {
                isDrawClaim = true;
            } else {
                ostringstream oss;
                oss << "Unrecognized term in entered move: '" << arg3 << "'";
                return getPlayerMoveError;
            }
        }

        for (Short k = 1; k <= 2; ++k) {
            string half = extMoveMatch[k];
            string halfName = k == 1 ? "From" : "To";
            std::smatch algNotationMatch;
            if (std::regex_match(half, algNotationMatch, algNotationRegex)) {
                string colStr = algNotationMatch[1];
                string rowStr = algNotationMatch[2];

                int col = tolower(colStr[0]) - 'a';
                if (col < 0 or col >= BOARD_COLS) {
                    cout << halfName << " position has an illegal column number: " << col;
                    return getPlayerMoveError;
                }

                int row = stoi(rowStr);
                if (row < 1 or row > BOARD_ROWS) {
                    cout << halfName << " position has an illegal row number: " << row;
                    return getPlayerMoveError;
                }
                row -= 1;  // Adjust for difference between alg chess notation and C++ indexing
                if (k == 1) {
                    fromP = std::make_unique<Pos>(col, row);
                    if (b.isEmpty(*fromP)) {
                        cout << "No piece at that board position\n";
                        return getPlayerMoveError;
                    }
                }
                else { toP = std::make_unique<Pos>(col, row); }
            } else {
                cout << halfName << " position has the wrong format";
                return getPlayerMoveError;
            }
        }
    } else {
        cout << "Input has the wrong format";
        return getPlayerMoveError;
    }

    const Piece& fromPiece = *(b.pieceAt(*fromP).get());
    if (fromPiece.color() != c) {
        cout << "That's not your piece!\n";
        return getPlayerMoveError;
    }
    PieceType fromPieceType = fromPiece.pieceType();
    bool isPawnMove = fromPieceType == PieceType::Pawn;

    // Move might be invalid; queryPlayerMove tests against list of valid moves.
    bool isEnPassant =
        isPawnMove && (fromP->x != toP->x) && !b.pieceAt(*toP);

    if (promotedType == std::nullopt) {
        if (isPawnMove && toP->isPawnPromotionRow(c)) {
            // Use Queen as default promotion type.
            promotedType = std::make_optional<PieceType>(PieceType::Queen);
        }
    } else {
        if (!isPawnMove || !toP->isPawnPromotionRow(c)) {
            cout << "Can only promote when moving a Pawn to the farthest row.";
            return getPlayerMoveError;
        }
    }
    Move move = Move{ c, fromPieceType, *fromP, *toP
                    , b.pieceAt(*toP)
                    , isPawnMove
                    , isEnPassant
                    , promotedType
                    };
    ExtMove extMove = ExtMove(std::make_optional<Move>(move), isDrawClaim, GameEnd::InPlay);
    return extMove;
}

// ---------- Friend operator
ostream& operator<<(ostream& os, const Move& move) {
    static const char* blueBold = "\033[1;34m";
    static const char* cyanBold = "\033[1;36m";
    static const char* greenBold = "\033[1;32m";
    static const char* magentaBold = "\033[1;35m";
    static const char* redBold = "\033[1;31m";
    static const char* resetCode = "\033[0m";

    ostringstream oss;
    if (move._pieceType == PieceType::King && move.isCastling()) {
        Col xdiff = move._to.xdiff(move._from);
        if (xdiff == 2) {
            oss << move._color << cyanBold << "O-O" << resetCode;
        }
        if (xdiff == -2) {
            oss << move._color << cyanBold << "O-O-O" << resetCode;
        }
    } else {
        oss << move._color << move._pieceType
            << '@' << move._from << "->" << move._to;
        if (move.capturedP()) {
            oss << redBold
                << 'x' << move._capturedP->pieceType()
                << resetCode;
        }
        if (move.isEnPassant()) {
            oss << blueBold << "ep" << resetCode;
        }
        if (move.isPromotion()) {
            oss << greenBold << '=' << move.promotionType() << resetCode;
        }
    }
    if (move.isCheck()) { oss << magentaBold << '+' << resetCode; }
    if (move.isCheckmate()) { oss << magentaBold << '#' << resetCode; }
    os << oss.str();
    return os;
}
