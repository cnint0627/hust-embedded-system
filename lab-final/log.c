// log.c - 游戏日志实现
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// 数字转中文
static const char* number_to_chinese[] = {
    "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"
};

// 颜色名称
static const char* color_names[] = {"红", "黑"};

// 内部函数：添加日志
static void log_add(GameLog *log, const char *format, ...) {
    if(!log) return;
    
    va_list args;
    va_start(args, format);
    
    // 移动旧日志
    if(log->log_count >= MAX_LOG_LINES) {
        for(int i = 0; i < MAX_LOG_LINES-1; i++) {
            strcpy(log->logs[i], log->logs[i+1]);
        }
        log->log_count = MAX_LOG_LINES-1;
    }
    
    // 添加新日志
    vsnprintf(log->logs[log->log_count], 64, format, args);
    log->log_count++;
    
    va_end(args);
}

void log_init(GameLog *log) {
    if(!log) return;
    memset(log, 0, sizeof(GameLog));
    log_add_start(log);
}

void log_add_start(GameLog *log) {
    log_add(log, "游戏开始，红方先手");
}

void log_add_move(GameLog *log, int color, const char* piece_name,
                 int from_x, int from_y, enum MoveType move_type, int steps) {
    // 坐标转换
    int display_x = color == 0 ? 9-from_x : from_x+1;
    int display_y = color == 0 ? from_y+1 : 10-from_y;
    
    // 移动类型文字
    const char* move_str = NULL;
    switch(move_type) {
        case MOVE_ADVANCE:
            move_str = color == 0 ? "进" : "退";
            break;
        case MOVE_RETREAT:
            move_str = color == 0 ? "退" : "进";
            break;
        case MOVE_HORIZONTAL:
            move_str = "平";
            break;
    }
    
    // 添加移动日志
    log_add(log, "%s%d%s%s%s",
            piece_name,
            display_x,
            number_to_chinese[display_y],
            move_str,
            number_to_chinese[steps]);
}