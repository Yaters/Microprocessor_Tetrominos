#ifndef __TETRIS_H
#define __TETRIS_H

#include "main.h"

#define BOARD_X 5
#define BOARD_Y 5

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

#define EMPTY_BOARD_CHAR ' '

#define FRAME_WIDTH 100
#define FRAME_HEIGHT 499
#define IMAGE_WIDTH 60
#define IMAGE_HEIGHT 30
#define IMAGE_X 0
#define IMAGE_Y 0

// enums
enum consoleState{Start, Playing, Paused};

// structs
typedef struct Game {
	uint8_t const * tetromino;
	uint8_t const * swapTetromino;
	uint8_t const * nextTetromino;
    short rotation;
    int x;
    int y;
    uint8_t board[BOARD_WIDTH * BOARD_HEIGHT];
    enum consoleState state;
}Game;


typedef struct Window {
    int width;
    int height;

    // image buffers
    uint8_t imgBuff1[FRAME_HEIGHT * FRAME_WIDTH];
    uint8_t imgBuff2[FRAME_HEIGHT * FRAME_WIDTH];
    int curBuff;
    Game game;
}Window;



// function prototypes
void tetris_initialize_game(Window * window);
const uint8_t * tetris_get_next_tetromino();
void tetris_update_current_tetromino(Window * window);
void tetris_swap_tetromino(Window * window);
void tetris_rotate_C_tetromino(Window * window);
void tetris_rotate_CC_tetromino(Window * window);
int tetris_move_left(Window * window);
int tetris_move_right(Window * window);
int tetris_move_down(Window * window);
void tetris_finished_tetromino(Window * window);
void tetris_detect_rowCompletion(Window * window);
int tetris_validate_position(Window * window, int x_offset, int y_offset);


int process_user_input(Window * window);
extern void update_screen(Window* window);
void game_playing(Window* window, int event);
void game_paused(Window * window, int event);
void game_start(Window * window, int event);
void create_window(Window * window);
void end_application(Window* window);


// Window Functions

// function prototypes
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t* data);
void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color);
void refreshScreen(Window * window);


#endif
