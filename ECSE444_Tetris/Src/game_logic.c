#include <game_logic.h>

const uint8_t tetromino_I[] = {  150, 150, 150, 150,
								 10, 10, 10, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_J[] = {  150, 10, 10, 10,
								 150, 150, 150, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_L[] = {  10, 10, 10, 150,
								 10, 150, 150, 150,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_O[] = {  10, 150, 150, 10,
								 10, 150, 150, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_S[] = {  10, 150, 150, 10,
								 150, 150, 10, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_T[] = {  10, 150, 10, 10,
								 150, 150, 150, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

const uint8_t tetromino_Z[] = {  150, 150, 10, 10,
								 10, 150, 150, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

// used to cache the rotated state of the tetromino
uint8_t tetromino_current[] = {  10, 10, 10, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10,
								 10, 10, 10, 10};

// used to store the collision between the tetromino and the game board
uint8_t tetromino_collision[]={ 10, 10, 10, 10,
								10, 10, 10, 10,
								10, 10, 10, 10,
								10, 10, 10, 10};

extern RNG_HandleTypeDef hrng;
uint32_t rand_block;


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

    // initialize game state (tetromino, rotation, next tetromino, x, y, game state)
    window->game.rotation = 0;
    window->game.tetromino = tetris_get_next_tetromino();
    const uint8_t* next = tetris_get_next_tetromino();
	#ifdef TETRIS_LESS_RANDOM_FLAG
	if (next == window->game.tetromino) next = tetris_get_next_tetromino();
	#endif
    window->game.nextTetromino = next;
    window->game.x = 3;
    window->game.y = 0;
    window->game.state = Start;
    window->game.points = 0;
    window->game.rows_cleared = 0;

    // initialize the current tetromino
    tetris_update_current_tetromino(window);
}

/**
 * @brief generate random tetromino piece (select random number between 0 and 7)
 *
 * @return const char* pointer to random tetromino piece
 */
const uint8_t * tetris_get_next_tetromino() {
	HAL_RNG_GenerateRandomNumber(&hrng, &rand_block);
    switch (rand_block % 7) {
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
    for (int row = window->game.y + y_offset; row < window->game.y + 4 + y_offset; row++) {
        for (int col = window->game.x + x_offset; col < window->game.x + 4 + x_offset; col++) {
            // check for collision w/ bottom floor
            if (row >= BOARD_HEIGHT && tetromino_current[index] != 10) {
                return 3;
            }
            // collision w/ left wall
            if (col < 0 && tetromino_current[index] != 10) {
                return 1;
            }
            if (col >= BOARD_WIDTH && tetromino_current[index] != 10) {
                return 2;
            }
            if (tetromino_current[index] != 10 && window->game.board[BOARD_WIDTH * row + col] != EMPTY_BOARD_CHAR) {
                return 4;
            }
            index++;
        }
    }
    return 0;
}




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
            if (row >= 0 && row < BOARD_HEIGHT && col >= 0 && col < BOARD_WIDTH && tetromino_current[index] != 10) {
                window->game.board[BOARD_WIDTH * row + col] = tetromino_current[index];

                // check to see if game over!
                if (row < 4) {
                    tetris_initialize_game(window);
                    tetris_drawEndScreen(window);
                    window->game.state = Ended;
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
    const uint8_t* next = tetris_get_next_tetromino();
    #ifdef TETRIS_LESS_RANDOM_FLAG
    if (next == window->game.tetromino) next = tetris_get_next_tetromino();
	#endif
    window->game.nextTetromino = next;
    tetris_update_current_tetromino(window);
    // Write next tetromino and points to the screen in both buffers

}

/**
 * @brief Detect full rows, and clears them out of the board
 *
 * @param window window w/ game
 */
void tetris_detect_rowCompletion(Window * window) {
    // go over board & detect all lines that need to be cleared.
    int rowCompleted[BOARD_HEIGHT];
    int numRowsCompleted = 0;
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        rowCompleted[row] = 1;
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (window->game.board[BOARD_WIDTH * row + col] == EMPTY_BOARD_CHAR) {
                rowCompleted[row] = 0;
            }
        }
        if(rowCompleted[row]) numRowsCompleted++;
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

    window->game.rows_cleared += numRowsCompleted;
    switch(numRowsCompleted) {
    case 1:
    	window->game.points += 40;
    	break;
    case 2:
    	window->game.points += 100;
    	break;
    case 3:
    	window->game.points += 300;
    	break;
    case 4:
    	window->game.points += 1200;
    	break;
    default:
    	break;
    }
}


