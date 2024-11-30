// common.h - 公共定义
#ifndef _COMMON_H
#define _COMMON_H

// 屏幕尺寸
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

// 棋盘尺寸
#define BOARD_WIDTH     9
#define BOARD_HEIGHT    10

// UI尺寸定义
#define GRID_SIZE       60     // 格子大小
#define PIECE_RADIUS    25     // 棋子半径

// 棋盘位置计算
#define BOARD_WIDTH_PX   ((BOARD_WIDTH-1) * GRID_SIZE)    
#define BOARD_HEIGHT_PX  ((BOARD_HEIGHT-1) * GRID_SIZE)   
#define BOARD_START_X    ((SCREEN_WIDTH - BOARD_WIDTH_PX - 300) / 2)
#define BOARD_START_Y    ((SCREEN_HEIGHT - BOARD_HEIGHT_PX) / 2)

// 信息面板布局
#define INFO_WIDTH      280   
#define INFO_START_X    (BOARD_START_X + BOARD_WIDTH_PX + 100)
#define INFO_START_Y    BOARD_START_Y
#define INFO_HEIGHT     BOARD_HEIGHT_PX

// 按钮布局
#define BUTTON_WIDTH    200
#define BUTTON_HEIGHT   50
#define BUTTON_SPACING  20

// 游戏结果界面
#define RESULT_WIDTH    400
#define RESULT_HEIGHT   200
#define RESULT_BTN_WIDTH   150
#define RESULT_BTN_HEIGHT  40

// 颜色定义
#define COLOR_BOARD    0xD6B37B  // 棋盘底色
#define COLOR_GRID     0x000000  // 棋盘线条
#define COLOR_RED      0xFF0000  // 红方棋子
#define COLOR_BLACK    0x000000  // 黑方棋子
#define COLOR_TEXT     0x000000  // 棋子文字
#define COLOR_SELECT   0x0000FF  // 选中框
#define COLOR_INFO_BG  0xEEEEEE  // 信息面板背景
#define COLOR_LOG      0x333333  // 日志文字

// 触摸事件类型
enum TouchEvent {
    TOUCH_NONE,
    TOUCH_PRESS,
    TOUCH_RELEASE,
    TOUCH_MOVE
};

// 基础类型定义
#include <stdbool.h>
#include <stdint.h>
#include "../common/common.h"

typedef uint32_t Color;

#endif