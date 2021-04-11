# Chess

This is an incomplete personal project toaward a single-threaded, text-based human vs human chess game in C++.

Focus (but no promises):
  * Implementation of rules, such as Pawn promotion, Castling, the different types of Draw, etc.
  * Flexibility of board/piece/rules, to enable future support of chess variants.
  * Prioritizing (enjoyably) refreshing C++ familiarity over following a particular methodology.
 
Some design choices:
  * Concise move rules via flexible move function(s): slide, leap, etc.
  * Use of C++ template parameter packs in class Logger, to support flexible logging.
  * Presence of some testing is high priority, but exhaustive testing is low priority.
  * Use scoped enums, except where values are used as superimposable flags.
  * Note: Having all code in header files is a temporary convenience. Source files to be added later.

Computer players:
  * There are current two computer players: Random and RandomCapture.
  * Neither of these computer players currently claim Draw conditions.
  * The computer players cannot current be given Draw offers, or accept proposals to concede.
 
Implementation outline:
  * Board
    * class Board      // Uses Zobrist hashing.
    * Contains Pieces  // Sparse storage. Pieces located at x/y or at index? Decide later.
    * Game state enums // Check/Mate; WinBlack/WinWhite/Draw; Type of Draw, etc.
  * Game
    * class Game
    * Contains Board
    * Contains Players
  * GameState
    * GameEnd          // InPlay, Draw, WinBlack, WinWhite
    * WinType          // Agreement, Checkmate, Conceding
    * DrawFlag (auto)  // Stalemate, 5x Rep, 75 Move Rule, Insufficient Resources
    * DrawFlag (others)// Agreement, 3x Rep (claimed), 50 Move Rule (claimed)
    * Note:            Draw can be claimed as part of a move, or in place of a move
    * Insufficient Resources: K vs. K (or K+B or K+N or K+N+N)
    * Insufficient Resources: K+R vs. K+B (or K+N or K+R+B or K+R+N)
    * Insufficient Resources: K+B vs. K+B, where both Bishops are on the same color square
  * Geometry: Pos, Dir
    * struct Pos       // x/y vs indexes? Prob x/y short-term; indexes long-term (performance).
    * struct Dir       // Dir != Pos. Pos+Dir and Dir+Dir are defined; Pos+Pos is not.
  * Move
    * CaptureRule      // Per-PieceType test for whether a piece (e.g., King) is being attacked.
    * MoveRule         // Per-PieceType Move generator from Board/Color/Pos.
    * MoveType         // Simple, Castle (K/Q), EnPassant, PawnPromotion, etc.
    * Move             // apply & applyUndo
    * Move support     // General move function(s) + attacking function(s) (for Check detection)
    * getValidPieceMoves   // Get list of valid Piece Moves, grouped by 'from' position for easy display.
    * getValidPlayerMoves  // Filters output of getValidPieceMoves, removing moves that 'self-check'.
    * getPlayerMove    // Query players for their Moves. Parse input.
  * Piece
    * class Piece      // Uses PieceType member instead of inheritance
    * PieceType        // King, Queen, Rook, Bishop, Knight, Pawn
  * Player
    * class Player
    * PlayerType       // Human, Computer, though Computer player support is out of scope.
  * Test*
    * Test functions for main classes
  * Util
    * enum Color
    * PRNG             // For computer players

Out of scope (partial list):
  * Sophisticated computer play.
  * GUI.
  * Board, piece, or rule variations (though these should be feasible to add)
  * Use of a chess clock
  * High performance (e.g., bitboards, parallelism, use of GPUs)

History:
  * This started as a one-day exercise to see how far I can get toward an implementation of a chess game. I decided to continue.
