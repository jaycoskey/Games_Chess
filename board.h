// Copyright 2021, by Jay M. Coskey

#pragma once

#include <map>
#include <numeric>

#include "util.h"


class Move;

// TODO?: Include storage of captured pieces for Undo? (Could use std::move)
// TODO: Privacy
class Board {
public:
    Board();
    void        pieceAdd(Color color, PieceType type, Short index);
    void        pieceAddPair(PieceType type, Short index);
    Piece       pieceRemove(Col col, Row row);
    Piece       pieceRemove(Short index);

    bool        containsPos(Pos pos) const { return pos.x < BOARD_COLS && pos.y < BOARD_ROWS; }
    std::optional<Piece> pieceAt(Pos pos);

private:
    void        execMove(const Move & move);  // TODO: execMoveStep.... 
    void        initializePieces();
    bool        isAtLastRow(Piece) const;
    std::string show() const;
    float       valuation() const;

private:
    std::map<Color, std::set<std::unique_ptr<Piece>>> piecesByColor;

    friend struct std::hash<Board>;
};

Board::Board() {
    initializePieces();
}

void Board::pieceAdd(Color color, PieceType type, Short index) {
    piecesByColor[color].insert(std::make_unique<Piece>(color, type, index));
}

void Board::pieceAddPair(PieceType type, Short index) {
    pieceAdd(White, type, index);
    pieceAdd(Black, type, invertIndex(index));
}

void Board::initializePieces() {
    // TODO: assert that board is uninitialized
    for (Color color: {Black, White}) {
        piecesByColor[color] = std::set<std::unique_ptr<Piece>>();
        int king_indexes[]   = {4};
        int queen_indexes[]  = {3};
        int rook_indexes[]   = {0, 7};
        int bishop_indexes[] = {2, 5};
        int knight_indexes[] = {1, 6};
        int pawn_indexes[]   = {8, 9, 10, 11, 12, 13, 14, 15};
        
        for (Short index : king_indexes)   { pieceAddPair(King,   index); }
        for (Short index : queen_indexes)  { pieceAddPair(Queen,  index); }
        for (Short index : rook_indexes)   { pieceAddPair(Rook,   index); }
        for (Short index : bishop_indexes) { pieceAddPair(Bishop, index); }
        for (Short index : knight_indexes) { pieceAddPair(Knight, index); }
        for (Short index : pawn_indexes)   { pieceAddPair(Knight, index); }
    }
}

namespace std {
    template<> struct hash<Board> {
        // Note: Uniqueness seems likely, but not guaranteed! (Fallback: Check board positions)
        std::size_t operator()(const Board& board) const noexcept {
            std::size_t result = 0;
            for (auto const& [color, piece_ptrs] : board.piecesByColor) {
                for (auto const& piece_ptr : piece_ptrs) {
                    std::size_t color_id     = static_cast<std::size_t>(to_underlying(color));
                    std::size_t piece_type_id = static_cast<std::size_t>(to_underlying(piece_ptr->type()));
                    std::size_t pos_index     = static_cast<std::size_t>(piece_ptr->pos().index());

                    result += primes[pos_index + 10] * primes[color_id + 80] * primes[piece_type_id + 90];
                }
            }
            return result;
        }
    };
}
