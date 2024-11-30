// ui.c - 用户界面实现
#include "ui.h"
#include <stdio.h>
#include <string.h>

// 主菜单按钮
static Button menu_buttons[] = {
    {
        .x = SCREEN_WIDTH/2 - BUTTON_WIDTH/2,
        .y = SCREEN_HEIGHT/2,
        .w = BUTTON_WIDTH,
        .h = BUTTON_HEIGHT,
        .text = "开始游戏",
        .id = BTN_START,
        .color = 0x4CAF50
    },
    {
        .x = SCREEN_WIDTH/2 - BUTTON_WIDTH/2,
        .y = SCREEN_HEIGHT/2 + BUTTON_HEIGHT + BUTTON_SPACING,
        .w = BUTTON_WIDTH,
        .h = BUTTON_HEIGHT,
        .text = "退出游戏",
        .id = BTN_EXIT,
        .color = 0xF44336
    }
};

// 游戏界面按钮
static Button game_buttons[] = {
    {
        .x = INFO_START_X + (INFO_WIDTH - BUTTON_WIDTH)/2,
        .y = INFO_START_Y + INFO_HEIGHT - BUTTON_HEIGHT*2 - BUTTON_SPACING,
        .w = BUTTON_WIDTH,
        .h = BUTTON_HEIGHT,
        .text = "重新开始",
        .id = BTN_RESTART,
        .color = 0x4CAF50
    },
    {
        .x = INFO_START_X + (INFO_WIDTH - BUTTON_WIDTH)/2,
        .y = INFO_START_Y + INFO_HEIGHT - BUTTON_HEIGHT,
        .w = BUTTON_WIDTH,
        .h = BUTTON_HEIGHT,
        .text = "退出",
        .id = BTN_EXIT,
        .color = 0xF44336
    }
};

// 游戏结果按钮
static Button result_buttons[] = {
    {
        .w = RESULT_BTN_WIDTH,
        .h = RESULT_BTN_HEIGHT,
        .text = "重新开始",
        .id = BTN_RESTART,
        .color = 0x4CAF50
    },
    {
        .w = RESULT_BTN_WIDTH,
        .h = RESULT_BTN_HEIGHT,
        .text = "返回主菜单",
        .id = BTN_EXIT,
        .color = 0xF44336
    }
};

void ui_init(void) {
    fb_init("/dev/fb0");
    font_init("./font.ttc");
}

// 绘制按钮
static void draw_button(Button *btn) {
    fb_draw_rect(btn->x, btn->y, btn->w, btn->h, btn->color);
    fb_draw_border(btn->x, btn->y, btn->w, btn->h, COLOR_BLACK);
    fb_draw_text(
        btn->x + btn->w/2 - strlen(btn->text)*5,
        btn->y + btn->h/2 + 12,
        btn->text,
        32,
        COLOR_TEXT
    );
}

void ui_draw_menu(void) {
    // 清屏
    fb_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BOARD);
    
    // 绘制标题
    fb_draw_text(
        SCREEN_WIDTH/2 - 120,
        SCREEN_HEIGHT/3,
        "中国象棋",
        64,
        COLOR_BLACK
    );
    
    // 绘制按钮
    for(int i = 0; i < sizeof(menu_buttons)/sizeof(menu_buttons[0]); i++) {
        draw_button(&menu_buttons[i]);
    }
    
    fb_update();
}

void ui_draw_board(Game *game, GameLog *log) {
    // 清屏
    fb_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BOARD);
    
    // 绘制棋盘网格
    for(int i = 0; i < BOARD_HEIGHT; i++) {
        fb_draw_line(
            BOARD_START_X, 
            BOARD_START_Y + i * GRID_SIZE,
            BOARD_START_X + BOARD_WIDTH_PX, 
            BOARD_START_Y + i * GRID_SIZE,
            COLOR_GRID
        );
    }
    
    for(int i = 0; i < BOARD_WIDTH; i++) {
        if(i == 0 || i == BOARD_WIDTH-1) {
            // 边框
            fb_draw_line(
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y,
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y + BOARD_HEIGHT_PX,
                COLOR_GRID
            );
        } else {
            // 中间格子
            fb_draw_line(
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y,
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y + 4 * GRID_SIZE,
                COLOR_GRID
            );
            fb_draw_line(
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y + 5 * GRID_SIZE,
                BOARD_START_X + i * GRID_SIZE,
                BOARD_START_Y + BOARD_HEIGHT_PX,
                COLOR_GRID
            );
        }
    }
    
    // 楚河汉界
    fb_draw_text(
        BOARD_START_X + BOARD_WIDTH_PX/2 - 120,
        BOARD_START_Y + BOARD_HEIGHT_PX/2 + 15,
        "楚河          汉界",
        32,
        COLOR_GRID
    );
    
    // 九宫格
    // 红方
    fb_draw_line(
        BOARD_START_X + 3 * GRID_SIZE,
        BOARD_START_Y,
        BOARD_START_X + 5 * GRID_SIZE,
        BOARD_START_Y + 2 * GRID_SIZE,
        COLOR_GRID
    );
    fb_draw_line(
        BOARD_START_X + 5 * GRID_SIZE,
        BOARD_START_Y,
        BOARD_START_X + 3 * GRID_SIZE,
        BOARD_START_Y + 2 * GRID_SIZE,
        COLOR_GRID
    );
    
    // 黑方
    fb_draw_line(
        BOARD_START_X + 3 * GRID_SIZE,
        BOARD_START_Y + 7 * GRID_SIZE,
        BOARD_START_X + 5 * GRID_SIZE,
        BOARD_START_Y + 9 * GRID_SIZE,
        COLOR_GRID
    );
    fb_draw_line(
        BOARD_START_X + 5 * GRID_SIZE,
        BOARD_START_Y + 7 * GRID_SIZE,
        BOARD_START_X + 3 * GRID_SIZE,
        BOARD_START_Y + 9 * GRID_SIZE,
        COLOR_GRID
    );
    
    // 绘制棋子
    for(int y = 0; y < BOARD_HEIGHT; y++) {
        for(int x = 0; x < BOARD_WIDTH; x++) {
            if(game->chess.board[y][x].type != PIECE_NONE) {
                int screen_x = BOARD_START_X + x * GRID_SIZE;
                int screen_y = BOARD_START_Y + y * GRID_SIZE;
                
                fb_draw_circle(screen_x, screen_y, PIECE_RADIUS, 2,
                    game->chess.board[y][x].color == 0 ? COLOR_RED : COLOR_BLACK);
                
                fb_draw_text(
                    screen_x - 15,
                    screen_y + 15,
                    game->chess.board[y][x].name,
                    32,
                    COLOR_TEXT
                );
            }
        }
    }
    
    // 选中框
    if(game->has_selected) {
        int screen_x = BOARD_START_X + game->selected_x * GRID_SIZE;
        int screen_y = BOARD_START_Y + game->selected_y * GRID_SIZE;
        fb_draw_border(
            screen_x - PIECE_RADIUS - 2,
            screen_y - PIECE_RADIUS - 2,
            PIECE_RADIUS * 2 + 4,
            PIECE_RADIUS * 2 + 4,
            COLOR_SELECT
        );
    }
    
    // 信息面板
    fb_draw_rect(INFO_START_X, INFO_START_Y, INFO_WIDTH, INFO_HEIGHT, COLOR_INFO_BG);
    fb_draw_border(INFO_START_X, INFO_START_Y, INFO_WIDTH, INFO_HEIGHT, COLOR_GRID);
    
    // 当前回合
    char turn_info[32];
    sprintf(turn_info, "当前回合：%s方", game->current_player == 0 ? "红" : "黑");
    fb_draw_text(
        INFO_START_X + 20,
        INFO_START_Y + 40,
        turn_info,
        32,
        COLOR_BLACK
    );
    
    // 操作日志
    fb_draw_text(
        INFO_START_X + 20,
        INFO_START_Y + 100,
        "操作日志：",
        32,
        COLOR_BLACK
    );
    
    for(int i = 0; i < log->log_count; i++) {
        fb_draw_text(
            INFO_START_X + 20,
            INFO_START_Y + 150 + i * 40,
            log->logs[i],
            24,
            COLOR_LOG
        );
    }
    
    // 按钮
    for(int i = 0; i < sizeof(game_buttons)/sizeof(game_buttons[0]); i++) {
        draw_button(&game_buttons[i]);
    }
    
    fb_update();
}

void ui_draw_game_result(Game *game) {
    // 计算结果面板位置
    int x = (SCREEN_WIDTH - RESULT_WIDTH) / 2;
    int y = (SCREEN_HEIGHT - RESULT_HEIGHT) / 2;
    
    // 绘制结果面板
    fb_draw_rect(x, y, RESULT_WIDTH, RESULT_HEIGHT, COLOR_INFO_BG);
    fb_draw_border(x, y, RESULT_WIDTH, RESULT_HEIGHT, COLOR_GRID);
    
    // 结果标题
    const char* title = game->state == GAME_WIN_RED ? "红方获胜！" : "黑方获胜！";
    Color title_color = game->state == GAME_WIN_RED ? COLOR_RED : COLOR_BLACK;
    
    fb_draw_text(
        x + RESULT_WIDTH/2 - strlen(title)*5,
        y + 80,
        title,
        32,
        title_color
    );
    
    // 更新并绘制按钮
    result_buttons[0].x = x + RESULT_WIDTH/4 - RESULT_BTN_WIDTH/2;
    result_buttons[0].y = y + RESULT_HEIGHT - RESULT_BTN_HEIGHT - 20;
    result_buttons[1].x = x + RESULT_WIDTH*3/4 - RESULT_BTN_WIDTH/2;
    result_buttons[1].y = y + RESULT_HEIGHT - RESULT_BTN_HEIGHT - 20;
    
    for(int i = 0; i < 2; i++) {
        draw_button(&result_buttons[i]);
    }
}

TouchResult ui_handle_touch(int screen_x, int screen_y) {
    TouchResult result = {0};
    
    // 转换屏幕坐标到棋盘坐标
    result.board_x = (screen_x - BOARD_START_X + GRID_SIZE/2) / GRID_SIZE;
    result.board_y = (screen_y - BOARD_START_Y + GRID_SIZE/2) / GRID_SIZE;
    
    // 检查是否在棋盘范围内
    result.valid = (result.board_x >= 0 && result.board_x < BOARD_WIDTH &&
                   result.board_y >= 0 && result.board_y < BOARD_HEIGHT);
    
    return result;
}

enum ButtonID ui_check_button(int x, int y, enum GameScreen screen, Game *game) {
    Button *buttons;
    int button_count;
    
    // 根据当前界面选择按钮组
    if(screen == SCREEN_MENU) {
        buttons = menu_buttons;
        button_count = sizeof(menu_buttons)/sizeof(menu_buttons[0]);
    } else {
        if(game_is_over(game)) {
            buttons = result_buttons;
            button_count = sizeof(result_buttons)/sizeof(result_buttons[0]);
        } else {
            buttons = game_buttons;
            button_count = sizeof(game_buttons)/sizeof(game_buttons[0]);
        }
    }
    
    // 检查点击位置
    for(int i = 0; i < button_count; i++) {
        if(x >= buttons[i].x && x < buttons[i].x + buttons[i].w &&
           y >= buttons[i].y && y < buttons[i].y + buttons[i].h) {
            return buttons[i].id;
        }
    }
    
    return BTN_NONE;
}