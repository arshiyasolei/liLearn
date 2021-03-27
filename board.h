#pragma once
#define PawnWhite 2
#define PawnBlack 1
#define rookWhite 10
#define rookBlack 5
#define KnightWhite 11
#define KnightBlack 4
#define QueenWhite 13
#define QueenBlack 6
#define BishopWhite 12
#define BishopBlack 3
#define KingBlack 7
#define KingWhite 14
#include <stdio.h>
#include <stdlib.h>

#include <array>
struct movePiece {
    int i;
    int j;
    int goalI;
    int goalJ;
};

struct liBoard {
    std::array<std::array<int,8>,8> board;
    int isJumpingOverPiece(movePiece *mPiece) {
        // get start piece pos
        int i = mPiece->i;
        int j = mPiece->j;

        // determine movement type
        // vertical
        if (abs(mPiece->goalI - i) == 0) {
            if (j < mPiece->goalJ) {
                for (int start = j + 1; start <= mPiece->goalJ; ++start) {
                    //
                    if (board[i][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            
                            return 0;
                        }
                        return 1;
                    }
                }
                return 0;
            } else {
                for (int start = j - 1; start >= mPiece->goalJ; --start) {
                    if (board[i][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                }
                return 0;
            }
        }
        // horizontal
        else if (abs(mPiece->goalJ - j) == 0) {
            if (i < mPiece->goalI) {
                for (int start = i + 1; start <= mPiece->goalI; ++start) {
                    if (board[start][j]) {
                        if (start == mPiece->goalI &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                }
                return 0;
            } else {
                for (int start = i - 1; start >= mPiece->goalI; --start) {
                    if (board[start][j]) {
                        if (start == mPiece->goalI &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                }
                return 0;
            }
        }
        // everything else (diagonals)
        else {
            int temp = 0;
            if ((j < mPiece->goalJ) && (i < mPiece->goalI)) {
                temp = i + 1;
                for (int start = j + 1; start <= mPiece->goalJ; ++start) {
                    if (board[temp][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                    temp += 1;
                }
                return 0;
            } else if ((j < mPiece->goalJ) && (i > mPiece->goalI)) {
                
                temp = i - 1;
                for (int start = j + 1; start <= mPiece->goalJ; ++start) {
                    if (board[temp][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                    temp -= 1;
                }
                return 0;
            } else if ((j > mPiece->goalJ) && (i < mPiece->goalI)) {
                temp = i + 1;
                for (int start = j - 1; start >= mPiece->goalJ; --start) {
                    if (board[temp][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                    temp += 1;
                }
                return 0;
            } else {
                temp = i - 1;
                for (int start = j - 1; start >= mPiece->goalJ; --start) {
                    if (board[temp][start]) {
                        if (start == mPiece->goalJ &&
                            board[mPiece->goalI][mPiece->goalJ] == 99) {
                            return 0;
                        }
                        return 1;
                    }
                    temp -= 1;
                }
                return 0;
            }
        }
    }
    int validateMoveRook(movePiece *mPiece) {
        // make sure goali , goalj reaches the first blocking piece
        int i = mPiece->i;
        int j = mPiece->j;

        if ((abs(mPiece->goalI - i) == 0) || (abs(mPiece->goalJ - j) == 0)) {
            if (!isJumpingOverPiece(mPiece)) {
                return 1;
            }
        }
        return 0;
    }
    int validateMoveBishop(movePiece *mPiece) {
        int i = mPiece->i;
        int j = mPiece->j;
        // part one of validity
        if ((abs(mPiece->goalI - i) == abs(mPiece->goalJ - j))) {
            if (!isJumpingOverPiece(mPiece)) {
                return 1;
            }
        }
        return 0;
    }
    int validateMoveQueen(movePiece *mPiece) {
        int i = mPiece->i;
        int j = mPiece->j;
        // part one of validity
        if ((abs(mPiece->goalI - i) == 0) || (abs(mPiece->goalJ - j) == 0) ||
            (abs(mPiece->goalI - i) == abs(mPiece->goalJ - j))) {
            if (
                !isJumpingOverPiece(mPiece)) {
                return 1;
            } 
        }
        return 0;
    }


    int validateMoveKnight(movePiece *mPiece) {
        int i = mPiece->i;
        int j = mPiece->j;
        // part one of validity
        if ((abs(mPiece->goalI - i) == 2 && abs(mPiece->goalJ - j) == 1) ||
            (abs(mPiece->goalI - i) == 1 && abs(mPiece->goalJ - j) == 2)) {

                return 50;
             
        }
        return 0;
    }
    int validateMove(movePiece *mPiece) {
        // leap of faith
        // if the piece that we are trying to move exists
        int moveWasValid = 0;
        if (board[mPiece->i][mPiece->j]) {
            switch (board[mPiece->i][mPiece->j]) {
                case 1:
                case 2:
                    return 1;
                    // moveWasValid = validateMovePawn(mPiece);
                    break;
                case 3:
                case 12:
                    
                    moveWasValid = validateMoveBishop(mPiece);
                    break;
                case 4:
                case 11:
                    
                    moveWasValid = validateMoveKnight(mPiece);
                    break;
                case 5:
                case 10:
                    moveWasValid = validateMoveRook(mPiece);
                    break;
                case 7:
                case 14:
                    return 1;
                    // moveWasValid = validateMoveKing(mPiece);
                    break;
                case 6:
                case 13:
                    
                    moveWasValid = validateMoveQueen(mPiece);
                    break;
                default:
                    break;
            }

            if (moveWasValid) {
                return 1;
            } else {
                return 0;
            }
        }
        return 0;
    }
    void updateBoard(movePiece *mPiece) {
        int temp = board[mPiece->i][mPiece->j];
        board[mPiece->goalI][mPiece->goalJ] = temp;
        board[mPiece->i][mPiece->j] = 0;
    }
};
