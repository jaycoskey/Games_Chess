// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"
#include "geometry.h"

#include "logger.h"

using MoveIndex         = Short;
using MoveIndexHistory  = std::vector<bool>;
using PieceValue        = float;


constexpr PieceValue KING_VALUE = 1'000.0;

enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

constexpr Short PIECE_TYPES_COUNT = 6;

std::ostream& operator<<(std::ostream& os, PieceType pt);

using OptPieceType = std::optional<PieceType>;

const std::vector<PieceType> pieceTypes {
    PieceType::King, PieceType::Queen, PieceType::Rook
    , PieceType::Bishop, PieceType::Knight, PieceType::Pawn
    };

// ========================================
// Piece

// class Board;

class Piece
{
public:
    // Static method
    static PieceValue pieceValue(PieceType pt);

    // Constructor
    Piece(Color color, PieceType pt, Short index, MoveIndex lastMoveIndex=0);

    // Public read methods
    Color color() const { return _color; }
    bool  isBlack() const { return _color == Color::Black; }
    bool  isWhite() const { return _color == Color::White; }

    PieceType pieceType() const { return _pieceType; }

    Short index() const { return _pos.x + _pos.y * BOARD_COLS; }
    Col   col() const { return _pos.x; }
    Row   row() const { return _pos.y; }
    Pos   pos() const { return _pos; }
    Color squareColor() const { return _pos.squareColor(); }

    bool hasMoved() const { return lastMoveIndex() > 0; }
    MoveIndex lastMoveIndex() const;

    // Public write methods
    void moveTo(const Pos& pos) { _pos.moveTo(pos); }
    void rollBackLastMoveIndex(MoveIndex mi);
    void setPieceType(PieceType pt) { _pieceType = pt; }
    void updateMoveIndexHistory(MoveIndex mi);

    // Operator
    bool operator<(const Piece& other) { return _pos < other.pos(); }

private:
    Color     _color;
    PieceType _pieceType;
    Pos       _pos;
    MoveIndexHistory _moveIndexHistory;  // First move=1. Never moved=0.

    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
};
