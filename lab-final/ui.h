// ui.h - 用户界面
#ifndef _UI_H
#define _UI_H

#include "game.h"
#include "log.h"
#include "common.h"

// 按钮ID
enum ButtonID {
    BTN_NONE,
    BTN_START,    // 开始游戏
    BTN_RESTART,  // 重新开始
    BTN_EXIT      // 退出游戏
};

// 按钮结构
typedef struct {
    int x, y;           // 位置
    int w, h;           // 尺寸
    const char *text;   // 文字
    enum ButtonID id;   // ID
    Color color;        // 颜色
} Button;

// 触摸结果
typedef struct {
    int board_x, board_y;  // 棋盘坐标
    bool valid;            // 是否有效
} TouchResult;

/**
 * 初始化UI
 */
void ui_init(void);

/**
 * 绘制主菜单
 */
void ui_draw_menu(void);

/**
 * 绘制游戏界面
 * @param game 游戏指针
 * @param log 日志指针
 */
void ui_draw_board(Game *game, GameLog *log);

/**
 * 绘制游戏结果
 * @param game 游戏指针
 */
void ui_draw_game_result(Game *game);

/**
 * 处理触摸事件
 * @param screen_x,screen_y 屏幕坐标
 * @return 触摸结果
 */
TouchResult ui_handle_touch(int screen_x, int screen_y);

/**
 * 检查按钮点击
 * @param x,y 触摸坐标
 * @param screen 当前界面
 * @param game 游戏指针
 * @return 按钮ID
 */
enum ButtonID ui_check_button(int x, int y, enum GameScreen screen, Game *game);

#endif