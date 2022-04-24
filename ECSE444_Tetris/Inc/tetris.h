#ifndef __TETRIS_H
#define __TETRIS_H

//#include "main.h"
#include <stdlib.h>
#include <stdio.h>


#define ARM_MATH_CM4
#include "arm_math.h"

#define BOARD_X 2
#define BOARD_Y 2

#define BOARD_WIDTH 10   //graphics scaled later
#define BOARD_HEIGHT 20  //graphics scaled later

#define EMPTY_BOARD_CHAR 100

#define FRAME_WIDTH 100
#define FRAME_HEIGHT 499
#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 350
#define IMAGE_X 2
#define IMAGE_Y 70

// enums
enum consoleState{Start, Playing, Paused, Ended};

// structs
typedef struct Game {
	uint8_t const * tetromino;
	uint8_t const * swapTetromino;
	uint8_t const * nextTetromino;
	long unsigned int points;
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
    uint8_t** frame;
    uint8_t** true;
    uint8_t frameBuff[FRAME_HEIGHT * FRAME_WIDTH];
    uint8_t trueBuff[FRAME_HEIGHT * FRAME_WIDTH];
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
extern void print_str(Window* window, char* buffer, int x, int y);




// Window Functions

// function prototypes
void tetris_write_points(Window* window);
void tetris_drawBackground(Window* window);
void tetris_drawEndScreen(Window * window);
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t* data);
void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color);
void refreshScreen(Window * window);


#endif
