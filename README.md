# Chess

This is a one-day (or more) exercise to see how far I can get toward an implementation of a chess game: A single-threaded, text-based console game. I considered several languages, and chose C++.

Design choices:
  * Concise move rules via flexible move function(s): slide, leap, etc.
  * Presence of some testing is high priority, but exhaustive testing is low priority.
    * Computer vs. computer play would be a great test driver, if this project ever gets that far.

Rough outline:
  * Board
    - class Board
    - Contains Pieces      // Sparse storage. Pieces located at x/y or at index? Decide later.
    - Game state enums     // Check/Mate; WinBlack/WinWhite/Draw; Type of Draw, etc.
  * Game
    - class Game
    - enum CheckType       // Check / Checkmate
    - Contains Board
    - Contains Players
  * Move
    - class Move
    - enum MoveType        // Simple, Castle (K/Q), EnPassant, PawnPromotion
    - Support for other move types? Concede, Draw, etc.
    - general move function + attacking function (for Check detection)
  * Piece
    - class Piece          // Uses PieceType member instead of inheritance
    - PieceType            // King, Queen, Rook, Bishop, Knight, Pawn
  * Player
    - class Player
    - PlayerType           // Human, Computer
  * Test*
    - Test functions for main classes
  * Util
    - enum Color
    - struct Pos           // x/y vs indexes? Prob x/y short-term; indexes long-term (performance).
    - struct Dir           // Dir != Pos. Pos+Dir and Dir+Dir are defined; Pos+Pos is not.
    - PRNG                 // For computer players
