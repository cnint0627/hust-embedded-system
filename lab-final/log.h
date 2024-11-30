// log.h - 游戏日志记录
#ifndef _LOG_H
#define _LOG_H

#define MAX_LOG_LINES 10

// 移动类型
enum MoveType {
    MOVE_ADVANCE,    // 进
    MOVE_RETREAT,    // 退
    MOVE_HORIZONTAL  // 平
};

// 游戏日志结构
typedef struct {
    char logs[MAX_LOG_LINES][64];  // 日志内容
    int log_count;                 // 日志行数
} GameLog;

/**
 * 初始化日志
 * @param log 日志指针
 */
void log_init(GameLog *log);

/**
 * 添加游戏开始日志
 * @param log 日志指针
 */
void log_add_start(GameLog *log);

/**
 * 添加移动日志
 * @param log 日志指针
 * @param color 棋子颜色
 * @param piece_name 棋子名称
 * @param from_x,from_y 起始位置
 * @param move_type 移动类型
 * @param steps 移动步数
 */
void log_add_move(GameLog *log, int color, const char* piece_name,
                 int from_x, int from_y, enum MoveType move_type, int steps);

#endif