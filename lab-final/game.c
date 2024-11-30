// game.c - 游戏状态管理实现
#include "game.h"
#include <string.h>

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    chess_init(&game->chess);
    game->state = GAME_PLAYING;
    game->current_player = 0;  // 红方先手
    game->has_selected = false;
}

void game_handle_select(Game *game, int x, int y) {
    if(game->state != GAME_PLAYING) {
        return;
    }
    
    if(!game->has_selected) {
        // 选中棋子
        Piece *piece = &game->chess.board[y][x];
        if(piece->type != PIECE_NONE && piece->color == game->current_player) {
            game->selected_x = x;
            game->selected_y = y;
            game->has_selected = true;
        }
    } else {
        // 移动棋子
        if(chess_is_valid_move(&game->chess, 
            game->selected_x, game->selected_y, x, y)) {
            
            // 执行移动
            chess_move_piece(&game->chess,
                game->selected_x, game->selected_y, x, y);
            
            // 检查游戏是否结束
            if(chess_is_game_over(&game->chess)) {
                game->state = game->current_player == 0 ? 
                             GAME_WIN_RED : GAME_WIN_BLACK;
            } else {
                // 切换玩家
                game->current_player = !game->current_player;
            }
        }
        game->has_selected = false;
    }
}

bool game_is_over(Game *game) {
    return game->state != GAME_PLAYING;
}

void game_get_move_info(Game *game, int from_x, int from_y, 
                       int to_x, int to_y, MoveInfo *info) {
    int dy = to_y - from_y;
    
    if(dy == 0) {
        info->type = MOVE_HORIZONTAL;
        info->steps = game->current_player == 0 ? 9-to_x : to_x+1;
    } else {
        info->steps = abs(dy);
        if(game->current_player == 0) {
            info->type = dy < 0 ? MOVE_ADVANCE : MOVE_RETREAT;
        } else {
            info->type = dy > 0 ? MOVE_ADVANCE : MOVE_RETREAT;
        }
    }
}