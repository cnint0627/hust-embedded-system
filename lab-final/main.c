// main.c - 主程序
#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "game.h"
#include "log.h"

int main(void) {
    // 初始化
    ui_init();
    int touch_fd = touch_init("/dev/input/event1");
    if(touch_fd < 0) return -1;
    
    Game *game = NULL;
    GameLog *log = NULL;
    enum GameScreen current_screen = SCREEN_MENU;
    
    ui_draw_menu();
    
    // 主循环
    while(1) {
        int x, y, finger;
        int event = touch_read(touch_fd, &x, &y, &finger);
        
        if(event == TOUCH_PRESS) {
            enum ButtonID btn_id = ui_check_button(x, y, current_screen, game);
            
            switch(btn_id) {
                case BTN_START:
                    if(current_screen == SCREEN_MENU) {
                        game = malloc(sizeof(Game));
                        log = malloc(sizeof(GameLog));
                        game_init(game);
                        log_init(log);
                        current_screen = SCREEN_GAME;
                        ui_draw_board(game, log);
                    }
                    break;
                    
                case BTN_RESTART:
                    if(current_screen == SCREEN_GAME) {
                        game_init(game);
                        log_init(log);
                        ui_draw_board(game, log);
                    }
                    break;
                    
                case BTN_EXIT:
                    if(current_screen == SCREEN_MENU) {
                        if(game) free(game);
                        if(log) free(log);
                        close(touch_fd);
                        system("clear");
                        return 0;
                    } else {
                        free(game);
                        free(log);
                        game = NULL;
                        log = NULL;
                        current_screen = SCREEN_MENU;
                        ui_draw_menu();
                    }
                    break;
                    
                case BTN_NONE:
                    if(current_screen == SCREEN_GAME) {
                        TouchResult touch = ui_handle_touch(x, y);
                        
                        if(touch.valid) {
                            // 记录移动前状态
                            int prev_player = game->current_player;
                            bool prev_selected = game->has_selected;
                            int prev_x = game->selected_x;
                            int prev_y = game->selected_y;
                            const char* piece_name = NULL;
                            
                            if(prev_selected) {
                                piece_name = game->chess.board[prev_y][prev_x].name;
                            }
                            
                            // 处理移动
                            game_handle_select(game, touch.board_x, touch.board_y);
                            
                            // 记录日志
                            if(prev_selected && !game->has_selected && 
                               prev_player != game->current_player) {
                                MoveInfo move_info;
                                game_get_move_info(game, prev_x, prev_y,
                                    touch.board_x, touch.board_y, &move_info);
                                log_add_move(log, prev_player, piece_name,
                                    prev_x, prev_y, move_info.type, move_info.steps);
                            }
                            
                            // 更新界面
                            ui_draw_board(game, log);
                            if(game->state != GAME_PLAYING) {
                                ui_draw_game_result(game);
                                fb_update();
                            }
                        }
                    }
                    break;
            }
        }
        
        task_delay(16);  // ~60fps
    }
}