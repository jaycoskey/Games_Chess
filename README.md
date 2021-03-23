# Chess

This is an incomplete personal project toaward a single-threaded, text-based human vs human chess game in C++.

Focus (but no promises):
  * Implementation of rules, such as Pawn promotion, Castling, the different types of Draw, etc.
  * Flexibility of board/piece/rules, to enable support of chess variants.
 
Some design choices:
  * Concise move rules via flexible move function(s): slide, leap, etc.
  * Presence of some testing is high priority, but exhaustive testing is low priority.
    * Computer vs. computer play would be a great test driver, if this project ever gets that far.
  * Use scoped enums, except where values are used as superimposable flags.

Implementation outline:
  * Board
    * class Board          // Will use Zobrist hashing to track repeated board positions
    * Contains Pieces      // Sparse storage. Pieces located at x/y or at index? Decide later.
    * Game state enums     // Check/Mate; WinBlack/WinWhite/Draw; Type of Draw, etc.
  * Game
    * class Game
    * enum CheckType       // Check / Checkmate
    * Contains Board
    * Contains Players
  * Geometry: Pos, Dir
    * struct Pos           // x/y vs indexes? Prob x/y short-term; indexes long-term (performance).
    * struct Dir           // Dir != Pos. Pos+Dir and Dir+Dir are defined; Pos+Pos is not.
  * Move
    * CaptureRule          // Per-PieceType test for whether a piece (e.g., King) is being attacked.
    * MoveRule             // Per-PieceType Move generator from Board/Color/Pos.
    * MoveType             // Simple, Castle (K/Q), EnPassant, PawnPromotion, etc.
    * Move                 // apply & applyUndo
    * Move support         // General move function(s) + attacking function(s) (for Check detection)
    * getValidPlayerMoves  // Get list of valid Moves, grouped by 'from' position for easy display.
    * getPlayerMove        // Query players for their Moves. Parse input.
  * Piece
    * class Piece          // Uses PieceType member instead of inheritance
    * PieceType            // King, Queen, Rook, Bishop, Knight, Pawn
  * Player
    * class Player
    * PlayerType           // Human, Computer, though Computer player support is out of scope.
  * Test*
    * Test functions for main classes
  * Util
    * enum Color
    * PRNG                 // For computer players

Out of scope (incomplete):
  * Computer strategy (C vs H, or C vs C)
  * GUI
  * High performance (e.g., bitboards, use of GPUs)

History:
  * This started as a one-day exercise to see how far I can get toward an implementation of a chess game. I decided to continue.
