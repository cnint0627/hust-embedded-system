// chess.c

#include "chess.h"
#include <string.h>
#include <stdlib.h>

static const char* RED_NAMES[] = {"帅", "仕", "相", "马", "车", "炮", "兵"};
static const char* BLACK_NAMES[] = {"将", "士", "象", "马", "车", "炮", "卒"};

void chess_init(Chess *chess) {
    // 清空棋盘
    memset(chess->board, 0, sizeof(chess->board));
    
    // 初始化布局数组
    const struct {
        int x, y;
        enum PieceType type;
    } init_pos[] = {
        // 红方布局
        {0, 0, PIECE_ROOK}, {8, 0, PIECE_ROOK},
        {1, 0, PIECE_KNIGHT}, {7, 0, PIECE_KNIGHT},
        {2, 0, PIECE_BISHOP}, {6, 0, PIECE_BISHOP},
        {3, 0, PIECE_ADVISOR}, {5, 0, PIECE_ADVISOR},
        {4, 0, PIECE_KING},
        {1, 2, PIECE_CANNON}, {7, 2, PIECE_CANNON},
        {0, 3, PIECE_PAWN}, {2, 3, PIECE_PAWN}, {4, 3, PIECE_PAWN},
        {6, 3, PIECE_PAWN}, {8, 3, PIECE_PAWN}
    };
    
    // 放置红方棋子
    for(int i = 0; i < sizeof(init_pos)/sizeof(init_pos[0]); i++) {
        int x = init_pos[i].x;
        int y = init_pos[i].y;
        chess->board[y][x].type = init_pos[i].type;
        chess->board[y][x].color = 0;  // 红方
        chess->board[y][x].name = (char*)RED_NAMES[init_pos[i].type - 1];
        
        // 放置黑方棋子(对称位置)
        int mirror_y = BOARD_HEIGHT - 1 - y;
        chess->board[mirror_y][x].type = init_pos[i].type;
        chess->board[mirror_y][x].color = 1;  // 黑方
        chess->board[mirror_y][x].name = (char*)BLACK_NAMES[init_pos[i].type - 1];
    }
}

// 统一的移动规则检查函数
bool chess_is_valid_move(Chess *chess, int from_x, int from_y, int to_x, int to_y) {
    Piece *from = &chess->board[from_y][from_x];
    Piece *to = &chess->board[to_y][to_x];
    
    // 基本检查
    if(from->type == PIECE_NONE) return false;
    if(to->type != PIECE_NONE && to->color == from->color) return false;
    
    int dx = abs(to_x - from_x);
    int dy = abs(to_y - from_y);
    int color = from->color;  // 0红方 1黑方
    
    switch(from->type) {
        case PIECE_KING: {
            // 九宫格检查
            if(to_x < 3 || to_x > 5) return false;
            if(color == 0 && to_y > 2) return false;    // 红方限制
            if(color == 1 && to_y < 7) return false;    // 黑方限制
            // 走一格
            return (dx + dy) == 1;
        }
            
        case PIECE_ADVISOR: {
            // 九宫格检查
            if(to_x < 3 || to_x > 5) return false;
            if(color == 0 && to_y > 2) return false;
            if(color == 1 && to_y < 7) return false;
            // 斜走一格
            return dx == 1 && dy == 1;
        }
            
        case PIECE_BISHOP: {
            // 不能过河
            if(color == 0 && to_y > 4) return false;
            if(color == 1 && to_y < 5) return false;
            // 走田字
            if(dx != 2 || dy != 2) return false;
            // 象眼检查
            return chess->board[(from_y + to_y)/2][(from_x + to_x)/2].type == PIECE_NONE;
        }
            
        case PIECE_KNIGHT: {
            // 日字走法
            if(!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) return false;
            // 马脚检查
            if(dx == 1) {
                return chess->board[(from_y + to_y)/2][from_x].type == PIECE_NONE;
            } else {
                return chess->board[from_y][(from_x + to_x)/2].type == PIECE_NONE;
            }
        }
            
        case PIECE_ROOK: {
            // 直线移动
            if(dx != 0 && dy != 0) return false;
            // 路径检查
            if(dx == 0) {
                // 竖直移动
                int step = (to_y > from_y) ? 1 : -1;
                for(int y = from_y + step; y != to_y; y += step) {
                    if(chess->board[y][from_x].type != PIECE_NONE) return false;
                }
            } else {
                // 水平移动
                int step = (to_x > from_x) ? 1 : -1;
                for(int x = from_x + step; x != to_x; x += step) {
                    if(chess->board[from_y][x].type != PIECE_NONE) return false;
                }
            }
            return true;
        }
            
        case PIECE_CANNON: {
            // 直线移动
            if(dx != 0 && dy != 0) return false;
            
            int count = 0;  // 路径上的棋子数
            if(dx == 0) {
                // 竖直移动
                int step = (to_y > from_y) ? 1 : -1;
                for(int y = from_y + step; y != to_y; y += step) {
                    if(chess->board[y][from_x].type != PIECE_NONE) count++;
                }
            } else {
                // 水平移动
                int step = (to_x > from_x) ? 1 : -1;
                for(int x = from_x + step; x != to_x; x += step) {
                    if(chess->board[from_y][x].type != PIECE_NONE) count++;
                }
            }
            // 炮的移动规则：不吃子时路径必须为空，吃子时必须翻过一个棋子
            return (to->type == PIECE_NONE && count == 0) || 
                   (to->type != PIECE_NONE && count == 1);
        }
            
        case PIECE_PAWN: {
            // 不能后退
            if(color == 0 && to_y < from_y) return false;
            if(color == 1 && to_y > from_y) return false;
            
            if(color == 0) {
                // 红方兵
                if(from_y < 5) {
                    // 未过河，只能前进
                    return dx == 0 && dy == 1;
                } else {
                    // 过河后可横走
                    return (dx + dy) == 1;
                }
            } else {
                // 黑方卒
                if(from_y > 4) {
                    // 未过河，只能前进
                    return dx == 0 && dy == 1;
                } else {
                    // 过河后可横走
                    return (dx + dy) == 1;
                }
            }
        }
            
        default:
            return false;
    }
}

void chess_move_piece(Chess *chess, int from_x, int from_y, int to_x, int to_y) {
    chess->board[to_y][to_x] = chess->board[from_y][from_x];
    chess->board[from_y][from_x].type = PIECE_NONE;
}

bool chess_is_game_over(Chess *chess) {
    // 检查双方是否还有将/帅
    bool has_red_king = false;
    bool has_black_king = false;
    
    for(int y = 0; y < BOARD_HEIGHT; y++) {
        for(int x = 0; x < BOARD_WIDTH; x++) {
            if(chess->board[y][x].type == PIECE_KING) {
                if(chess->board[y][x].color == 0) {
                    has_red_king = true;
                } else {
                    has_black_king = true;
                }
            }
        }
    }
    
    return !(has_red_king && has_black_king);
}