#ifndef __TETRIS_H
#define __TETRIS_H
#include <stdlib.h>
#include <time.h>

#define BOARD_X 5
#define BOARD_Y 5

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

#define EMPTY_BOARD_CHAR ' '

#define FRAME_WIDTH 60
#define FRAME_HEIGHT 30
#define IMAGE_WIDTH 60
#define IMAGE_HEIGHT 30
#define IMAGE_X 0
#define IMAGE_Y 0

const char tetromino_I[] = {'1', '1', '1', '1',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_J[] = {'1', '\0', '\0', '\0',
                     '1', '1', '1', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_L[] = {'\0', '\0', '\0', '1',
                     '\0', '1', '1', '1',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_O[] = {'\0', '1', '1', '\0',
                     '\0', '1', '1', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_S[] = {'\0', '1', '1', '\0',
                     '1', '1', '\0', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_T[] = {'\0', '1', '\0', '\0',
                     '1', '1', '1', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};
const char tetromino_Z[] = {'1', '1', '\0', '\0',
                     '\0', '1', '1', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};

// used to cache the rotated state of the tetromino
char tetromino_current[] = {'\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0'};

// used to store the collision between the tetromino and the game board
char tetromino_collision[] = {'\0', '\0', '\0', '\0',
                        '\0', '\0', '\0', '\0',
                        '\0', '\0', '\0', '\0',
                        '\0', '\0', '\0', '\0'};

// enums
enum consoleState{Start, Playing, Paused};

// structs
typedef struct Game {
    char const * tetromino;
    char const * swapTetromino;
    char const * nextTetromino;
    short rotation;
    int x;
    int y;
    char board[BOARD_WIDTH * BOARD_HEIGHT];
    enum consoleState state;
}Game;


typedef struct Window {
    int width;
    int height;

    // image buffers
    char imgBuff1[FRAME_HEIGHT * FRAME_WIDTH];
    char imgBuff2[FRAME_HEIGHT * FRAME_WIDTH];
    int curBuff;
    Game game;
}Window;

// function prototypes
extern void update_screen(Window* window);
extern void refreshScreen(Window * window);
extern void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char* data);
extern void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char color);
void tetris_initialize_game(Window * window);
const char * tetris_get_next_tetromino();
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

/**
 * @brief populate window game variable with a reset tetris game.
 * 
 * @param window window
 */
void tetris_initialize_game(Window * window) {
    // fill game board with empty data
    for (int i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
        window->game.board[i] = EMPTY_BOARD_CHAR;
    }

    // seed random val
    time_t t;
    srand((unsigned) time(&t));
    
    // initialize game state (tetromino, rotation, next tetromino, x, y, game state)
    window->game.rotation = 0;
    window->game.tetromino = tetris_get_next_tetromino();
    window->game.nextTetromino = tetris_get_next_tetromino();
    window->game.x = 3;
    window->game.y = 0;
    window->game.state = Start;

    // initialize the current tetromino
    tetris_update_current_tetromino(window);
}

/**
 * @brief generate random tetromino piece (select random number between 0 and 7)
 * 
 * @return const char* pointer to random tetromino piece
 */
const char * tetris_get_next_tetromino() {
    switch (rand() % 7) {
        case 0:
            return tetromino_I;
        break;
        case 1:
            return tetromino_J;
        break;
        case 2:
            return tetromino_L;
        break;
        case 3:
            return tetromino_O;
        break;
        case 4:
            return tetromino_S;
        break;
        case 5:
            return tetromino_T;
        break;
        case 6:
            return tetromino_Z;
        break;
        default:
            return tetromino_I; // TODO: this is never supposed to happen.
        break;
    }
    
}

/**
 * @brief Updates the tetromino piece used for collision/drawing purposes
 * 
 * @param window window
 */
void tetris_update_current_tetromino(Window * window) {
    // update piece based on rotation state
    int index = 0, row, col;
    switch (window->game.rotation) {
        case 0:
            // no rotation
            for (row = 0; row < 4; row++) {
                for (col = 0; col < 4; col++) {
                    tetromino_current[4 * row + col] = window->game.tetromino[index];
                    index++;
                }
            }
        break;
        case 1:
            // C 1; CC 3;
            for (col = 3; col >= 0; col--) {
                for (row = 0; row < 4; row++) {
                    tetromino_current[4 * row + col] = window->game.tetromino[index];
                    index++;
                }
            }
        break;
        case 2:
            // C 2; CC 2;
            for (row = 3; row >= 0; row--) {
                for (col = 3; col >= 0; col--) {
                    tetromino_current[4 * row + col] = window->game.tetromino[index];
                    index++;
                }
            }
        break;
        case 3:
            // C 3; CC 1;
            for (col = 0; col < 4; col++) {
                for (row = 3; row >= 0; row--) {
                    tetromino_current[4 * row + col] = window->game.tetromino[index];
                    index++;
                }
            }
        break;
    }
}

/**
 * @brief Swap the tetromino pieces. NOT IMPLEMENTED
 * 
 * @param window window
 */
void tetris_swap_tetromino(Window * window) {

}

/**
 * @brief Rotate the tetromino piece clockwise. Abort rotation if will cause collision
 * 
 * @param window window
 */
void tetris_rotate_C_tetromino(Window * window) {
    window->game.rotation = (window->game.rotation + 1) % 4;
    tetris_update_current_tetromino(window);

    // undo rotation if it would lead to collision
    //TODO: make this smarter haha
    if (tetris_validate_position(window, 0, 0)) {
        window->game.rotation = (window->game.rotation + 3) % 4;
        tetris_update_current_tetromino(window);
    }
}

/**
 * @brief Rotate the tetromino piece conter clockwise. Abort rotation if will cause collision
 * 
 * @param window window
 */
void tetris_rotate_CC_tetromino(Window * window) {
    window->game.rotation = (window->game.rotation + 3) % 4;
    tetris_update_current_tetromino(window);

    // undo rotation if it would lead to collision
    //TODO: make this smarter haha
    if (tetris_validate_position(window, 0, 0)) {
        window->game.rotation = (window->game.rotation + 1) % 4;
        tetris_update_current_tetromino(window);
    }
}

/**
 * @brief Move the piece to the left. Abort if causes a collision
 * 
 * @param window 
 * @return int whether the event completed successfully
 */
int tetris_move_left(Window * window) {
    if (!tetris_validate_position(window, -1, 0)) {
        window->game.x--;
        return 1;
    }
    return 0;
}

/**
 * @brief Move the piece to the right. Abort if causes a collision
 * 
 * @param window 
 * @return int whether the event completed successfully
 */
int tetris_move_right(Window * window) {
    if (!tetris_validate_position(window, 1, 0)) {
        window->game.x++;
        return 1;
    }
    return 0;
}

/**
 * @brief Move the piece down. Tetromino piece position is finalized if collision occurs. Get next tetromino & update board accordingly.
 * 
 * @param window 
 * @return int whether the event completed successfully
 */
int tetris_move_down(Window * window) {
    if (!tetris_validate_position(window, 0, 1)) {
        window->game.y++;
        return 1;
    }

    tetris_finished_tetromino(window);
    tetris_detect_rowCompletion(window);
    return 1;
}

/**
 * @brief Checks for all possible collisions based on tetromino current position + offset
 * 
 * @param window tetris game window w/ tetromino data we want to validate
 * @param x_offset added to tetromino current x. X position to validate
 * @param y_offset added to tetromino current y. Y position to validate
 * @return int error code. 0 = no collision. 1 = collision w/ left wall. 2 = collision w/ right wall. 3 = collision with bottom floor. 4 = collision w/another block
 */
int tetris_validate_position(Window * window, int x_offset, int y_offset) {
    int index = 0;
    int wall_colision_flag = 0;
    int block_collision_flag = 0;
    for (int row = window->game.y + y_offset; row < window->game.y + 4 + y_offset; row++) {
        for (int col = window->game.x + x_offset; col < window->game.x + 4 + x_offset; col++) {
            // check for collision w/ bottom floor
            if (row >= BOARD_HEIGHT && tetromino_current[index] != '\0') {
                return 3;
            }
            // collision w/ left wall
            if (col < 0 && tetromino_current[index] != '\0') {
                return 1;
            }
            if (col >= BOARD_WIDTH && tetromino_current[index] != '\0') {
                return 2;
            }
            if (tetromino_current[index] != '\0' && window->game.board[BOARD_WIDTH * row + col] != ' ') {
                return 4;
            }
            index++;
        }
    }
    return 0;
}

// 
/**
 * @brief Finalizes the tetromino position and generates a new tetromino.
 * does NOT care about collisions. Assumes all of those have been remedied before being called.
 * Is responsible for detecting an END of game 
 * @param window tetris game window w/ tetromino data we want to validate
 */
void tetris_finished_tetromino(Window * window) {
    int index = 0;
    for (int row = window->game.y; row < window->game.y + 4; row++) {
        for (int col = window->game.x; col < window->game.x + 4; col++) {
            if (row >= 0 && row < BOARD_HEIGHT && col >= 0 && col < BOARD_WIDTH && tetromino_current[index] != '\0') {
                window->game.board[BOARD_WIDTH * row + col] = tetromino_current[index];

                // check to see if game over!
                if (row < 4) {
                    tetris_initialize_game(window);
                }
            }
            index++;
        }
    }

    // reset tetromino position & spawn next tetromino
    window->game.y = 0;
    window->game.x = 3;
    window->game.rotation = 0;
    window->game.tetromino = window->game.nextTetromino;
    window->game.nextTetromino = tetris_get_next_tetromino(window);
    tetris_update_current_tetromino(window);
}

/**
 * @brief Detect full rows, and clears them out of the board
 * 
 * @param window window w/ game
 */
void tetris_detect_rowCompletion(Window * window) {
    // go over board & detect all lines that need to be cleared.
    int rowCompleted[BOARD_HEIGHT];

    for (int row = 0; row < BOARD_HEIGHT; row++) {
        rowCompleted[row] = 1;
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (window->game.board[BOARD_WIDTH * row + col] == EMPTY_BOARD_CHAR) {
                rowCompleted[row] = 0;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        int anyRowFlag = 0;
        // go over board from bottom to top & turn lines that need to be cleared on/off
        for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
            if (rowCompleted[row]) {
                for (int col = 0; col < BOARD_WIDTH; col++) {
                    window->game.board[BOARD_WIDTH * row + col] = (i%2) ? '1' : EMPTY_BOARD_CHAR;
                }
                anyRowFlag = 1;
            }
        }

        if (!anyRowFlag) {
            break;
        }

        drawRect(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, 1, 1, window->game.board);
        // animation here hehe
        refreshScreen(window);
        // update screen
        update_screen(window);
    }

    // go over board from bottom to top & delete the lines that need to be cleared
    int row_currently_drawn = BOARD_HEIGHT - 1;
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        if (!rowCompleted[row]) {
            for (int col = 0; col < BOARD_WIDTH; col++) {
                window->game.board[BOARD_WIDTH * row_currently_drawn + col] = window->game.board[BOARD_WIDTH * row + col];
            }
            row_currently_drawn--;
        }
    }
    
    for (int row = row_currently_drawn; row >= 0; row--) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            window->game.board[BOARD_WIDTH * row_currently_drawn + col] = EMPTY_BOARD_CHAR;
        }
        row_currently_drawn--;
    }
}
#endif