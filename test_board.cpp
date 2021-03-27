// Copyright 2021, by Jay M. Coskey

#include <iostream>

#include "util.h"
#include "piece.h"
#include "board.h"

using std::cout;


void test_board_pieceCounts() {
    Board board;

    for (const auto& [c, piecePs] : board.color2PiecePs) {
        cout << "Color = " << c << "\n";

        Short kingCount   = 0;
        Short queenCount  = 0;
        Short rookCount   = 0;
        Short bishopCount = 0;
        Short knightCount = 0;
        Short pawnCount   = 0;

        for (const PieceP& pieceP : piecePs) {

            switch(pieceP->pieceType()) {
              case PieceType::King:
                kingCount++;
                break;
              case PieceType::Queen:
                queenCount++;
                break;
              case PieceType::Rook:
                rookCount++;
                break;
              case PieceType::Bishop:
                bishopCount++;
                break;
              case PieceType::Knight:
                knightCount++;
                break;
              case PieceType::Pawn:
                pawnCount++;
                break;
              case PieceType::None:
                throw new std::invalid_argument("Invalid piece type in board data");
            }
        }
        assert(kingCount   == 1);
        assert(queenCount  == 1);
        assert(rookCount   == 2);
        assert(bishopCount == 2);
        assert(knightCount == 2);
        assert(pawnCount   == 8);
    }
}

int main() {
    test_board_pieceCounts();
    return 0;
}
