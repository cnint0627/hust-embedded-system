// game.h - 游戏状态管理
#ifndef _GAME_H
#define _GAME_H

#include "chess.h"
#include "log.h"

// 游戏状态
enum GameState {
    GAME_PLAYING,
    GAME_WIN_RED,    // 红方胜
    GAME_WIN_BLACK   // 黑方胜
};

// 游戏界面
enum GameScreen {
    SCREEN_MENU,     // 主菜单
    SCREEN_GAME      // 游戏界面
};

// 游戏结构体
typedef struct {
    Chess chess;                 // 棋盘状态
    enum GameState state;        // 游戏状态
    int current_player;          // 当前玩家(0:红方,1:黑方)
    bool has_selected;           // 是否已选中棋子
    int selected_x, selected_y;  // 选中的棋子位置
} Game;

// 移动信息(用于日志)
typedef struct {
    enum MoveType type;  // 移动类型
    int steps;          // 移动步数
} MoveInfo;

/**
 * 初始化游戏
 * @param game 游戏指针
 */
void game_init(Game *game);

/**
 * 处理选择位置
 * @param game 游戏指针
 * @param x,y 选择的位置
 */
void game_handle_select(Game *game, int x, int y);

/**
 * 检查游戏是否结束
 * @param game 游戏指针
 * @return 是否结束
 */
bool game_is_over(Game *game);

/**
 * 获取移动信息(用于日志)
 * @param game 游戏指针
 * @param from_x,from_y 起始位置
 * @param to_x,to_y 目标位置
 * @param info 移动信息输出
 */
void game_get_move_info(Game *game, int from_x, int from_y, 
                       int to_x, int to_y, MoveInfo *info);

#endif