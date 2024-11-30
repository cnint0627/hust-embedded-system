// chess.h - 棋类核心逻辑
#ifndef _CHESS_H
#define _CHESS_H

#include "common.h"

// 棋子类型
enum PieceType {
    PIECE_NONE = 0,
    PIECE_KING,     // 将/帅
    PIECE_ADVISOR,  // 士/仕
    PIECE_BISHOP,   // 象/相
    PIECE_KNIGHT,   // 马
    PIECE_ROOK,     // 车
    PIECE_CANNON,   // 炮
    PIECE_PAWN      // 卒/兵
};

// 棋子结构
typedef struct {
    enum PieceType type;  // 类型
    int color;            // 0:红方, 1:黑方
    const char *name;     // 显示名称
} Piece;

// 棋盘结构
typedef struct {
    Piece board[BOARD_HEIGHT][BOARD_WIDTH];
} Chess;

// 函数声明

/**
 * 初始化棋盘
 * @param chess 棋盘指针
 */
void chess_init(Chess *chess);

/**
 * 检查移动是否合法
 * @param chess 棋盘指针
 * @param from_x,from_y 起始位置
 * @param to_x,to_y 目标位置
 * @return 是否可移动
 */
bool chess_is_valid_move(Chess *chess, int from_x, int from_y, int to_x, int to_y);

/**
 * 移动棋子
 * @param chess 棋盘指针
 * @param from_x,from_y 起始位置
 * @param to_x,to_y 目标位置
 */
void chess_move_piece(Chess *chess, int from_x, int from_y, int to_x, int to_y);

/**
 * 检查游戏是否结束
 * @param chess 棋盘指针
 * @return 是否结束
 */
bool chess_is_game_over(Chess *chess);

#endif