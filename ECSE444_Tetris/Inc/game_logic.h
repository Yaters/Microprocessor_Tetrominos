#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdlib.h>
#include <stdio.h>

// needed to use the RNG
#define ARM_MATH_CM4
#include "arm_math.h"
#include "main.h"


// board VGA offset variables
#define BOARD_X 2
#define BOARD_Y 2
#define BOARD_WIDTH 10   //graphics scaled later
#define BOARD_HEIGHT 20  //graphics scaled later

// VGA frame variables
#define FRAME_WIDTH 100
#define FRAME_HEIGHT 449

// VGA image variables
#define IMAGE_WIDTH 80
#define IMAGE_HEIGHT 350
#define IMAGE_X 2	// offset H
#define IMAGE_Y 70	// offset V

#define HORIZ_SCALE 2
#define VERT_SCALE 13

#define EMPTY_BOARD_CHAR 100

#define TETRIS_LESS_RANDOM_FLAG 0

#define FALL_INIT 800	// initial fall speed
#define MAX_POINTS 1

// enums
enum consoleState{Start, Playing, Paused, Ended};

// structs
typedef struct Game {
	uint8_t const * tetromino;
	uint8_t const * swapTetromino;
	uint8_t const * nextTetromino;
	long unsigned int points;
	unsigned int rows_cleared;
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


///////////////////////////////////////////////////
// function prototypes
///////////////////////////////////////////////////

//Game Logic
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


// Window Functions
extern void tetris_drawEndScreen(Window * window);

#endif
