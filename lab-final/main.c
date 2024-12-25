// main.c - 主程序
#include <stdio.h>
#include <stdlib.h>
#include "ui.h"
#include "game.h"
#include "log.h"
#include "bluetooth.h"

int main(int argc, char* argv[]) {

    printf("argc: %d\n", argc);

    int event_id = 1;
    int init_current_player = 0;
    if (argc >= 2) {
        sscanf(argv[1], "%d", &event_id);
    }
    if (argc >= 3) {
        sscanf(argv[2], "%d", &init_current_player);
    }
    char event_path[64];
    snprintf(event_path, sizeof(event_path), "/dev/input/event%d", event_id);

    // 初始化
    ui_init();
    int touch_fd = touch_init(event_path);
    if(touch_fd < 0) return -1;
    int flags = fcntl(touch_fd, F_GETFL, 0);
    fcntl(touch_fd, F_SETFL, flags | O_NONBLOCK);

    if(!bluetooth_init()) return -1;

    Game *game = NULL;
    GameLog *log = NULL;
    enum GameScreen current_screen = SCREEN_MENU;

    ui_draw_menu();

    game = calloc(sizeof(Game), sizeof(char));
    game_init(game);
    game->init_current_player = init_current_player;

    // 主循环
    while(1) {
        int x, y, finger;
        int event = touch_read(touch_fd, &x, &y, &finger);

        // receive
        TouchResult remote_touch = {0};
        int is_remote = 0; // {0, 1}

        if(!is_my_round(game)){
            if(bluetooth_receive_move(&remote_touch) > 0) {
                is_remote = 1;
                event = TOUCH_PRESS;
            }
        }

        if(event == TOUCH_PRESS) {
            enum ButtonID btn_id = is_remote ? GAME_BTN_NONE : ui_check_button(x, y, current_screen, game);

            switch(btn_id) {
                case GAME_BTN_START:
                    if(current_screen == SCREEN_MENU) {
                        // game = malloc(sizeof(Game));
                        // game_init(game);
                        log = malloc(sizeof(GameLog));
                        log_init(log);
                        current_screen = SCREEN_GAME;
                        ui_draw_board(game, log);
                    }
                    break;

                case GAME_BTN_RESTART:
                    if(current_screen == SCREEN_GAME) {
                        game_init(game);
                        log_init(log);
                        ui_draw_board(game, log);
                    }
                    break;

                case GAME_BTN_EXIT:
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

                case GAME_BTN_NONE:
                    if(current_screen == SCREEN_GAME) {
                        TouchResult touch = is_my_round(game) ? ui_handle_touch(x, y) : remote_touch;

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
                            if (is_remote == 0) bluetooth_send_move(&touch);

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
                        } else {
                            printf("touch not valid\n");
                        }
                    }
                    break;
            }
        }

        task_delay(16);  // ~60fps
    }

    // bluetooth_close();
}