 # Chess: A Chess Framework (C++)

 This is a single-threaded chess program that supports console-based two-player chess on a standard (ASCII) chess board. Each player can be either a       human interacting with the console, or a computer player. There are currently only two computer player "strategies" implemented: Random, and              RandomCapture (i.e., select a random capture move if one exists; otherwise choose a random move).
 
 * Rules: This program supports the standard rules of chess, including:
   * Castling and en passant moves, and Pawn promotion.
   * The claimable Draw conditions: 3x board repetition, or 50 move rule.
   * The automatic Draw conditions: 5x board repetition, or 75 move rule, Stalemate, or Draw due to insufficient resources.
     * Insufficient resources:
       * K vs. K (or K+B or K+N or K+N+N)
       * K+R vs. K+B (or K+N or K+R+B or K+R+N)
       * K+B vs. K+B, where both Bishops are on the same color square
 
 * Bots: The computer players (currently, Random and RandomCapture) do not claim Draw conditions, or accept Draw offers, or accept proposals to concede.
 
 * Out of scope:
   * The use of a chess clock is not supported.
   * There are no supported rule variations or board variations.
   * There is no GUI, or means of playing over a network.
 
 ## Chess: How to play interactively
 * Invoke the program as:
   * % chess -1 human -2 human
 * Types of moves:
   * Simple moves: Entered as a pair (from & to) of board positions in algebraic notation.
     * Example: f1 d3 <Enter>
     * When castling, enter the pair of board positions for the King only.
     * When moving a Pawn to the farthest row, follow with a letter indicating promoted type:  Q, R, B, or N.
       * If no promotion piece type is given, Queen is chosen as a default.
     * When capturing en passant, the program will detect the capture details.
   * Post-move Draw claim:
     * Example: f1 d3 draw <Enter>
     * If one (or both!) of the two claimable Draw conditions above will exist after a player moves a piece,
       the player can enter the word 'draw' after the from & to coordinates entered.
 * Non-move game commands:
   * Pre-move Draw claim:
     * Example: draw <Enter>
     * Two conditions can used to claim a Draw:
       * A 3x board repetition
       * 50 moves without a Pawn move or capture
     * If either of these conditions exist, the player is informed before move entry.
     * If neither condition exists, a Draw claim by the player is discarded.
   * Concede:
     * Example: concede <Enter>
   * Propose a draw:
     * Example: draw? <Enter>
   * Propose that the other player concede:
     * Example: win? <Enter>
     * The other player will be asked to accept or reject. If rejected, play returns to the proposing player.
       * The other player will be asked to accept or reject. If rejected, play returns to the proposing player.
 * Special (i.e., non-game) commands:
   * board:   Show the current board layout.
   * history: Show move history.
   * pgn:     Show move history, using a verbose PGN (Portable Game Network) input format, suitable for import into chess programs.
   * moves:   Show legal moves for the current player.
   * pieces:  List the pieces on the board.
   * log_level: Display the current log reporting level.
        "  * log_error, log_warn, log_info, log_debug, log_trace: Change the current log reporting level.
   * exit / quit: Exit the game.
 
 ## Chess: How to run in batch mode
 * To run two random-playing bots against each other, invoke the program as:
   * % chess -1 random -2 random -n 10
 * Upon exiting, the program will output a "batch summary", describing the way each of the match games ended.
 
 ## Personal note
 * This program started as a one-day exercise to see how far I could get toward a chess game implementation. After that first day, I decided to keep       going. I have implemented several variants of hexagonal chess before in a different language, and might some day extend this implementation to include    variant boards and rule.
 * Working on a personal project means you can use comma-first formatting.  :)
