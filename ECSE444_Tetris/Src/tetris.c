#include "tetris.h"

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


/**
 * @brief Write points to the screen while playing.
 *
 * @param window window
 */
void tetris_write_points(Window* window) {
    char point_str[10];
    sprintf(point_str, "%lu", window->game.points);
    print_str(window, "Points:", 27, 60);
    print_str(window, point_str, 27, 150);
}

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
    srand((unsigned) HAL_GetTick());

    // initialize game state (tetromino, rotation, next tetromino, x, y, game state)
    window->game.rotation = 0;
    window->game.tetromino = tetris_get_next_tetromino();
    window->game.nextTetromino = tetris_get_next_tetromino();
    window->game.x = 3;
    window->game.y = 0;
    window->game.state = Start;
    window->game.points = 0;

    // initialize the current tetromino
    tetris_update_current_tetromino(window);
}

/**
 * @brief generate random tetromino piece (select random number between 0 and 7)
 *
 * @return const char* pointer to random tetromino piece
 */
const uint8_t * tetris_get_next_tetromino() {
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
 * @brief Draws the final screen of Tetris.
 * does NOT care about collisions. Assumes all of those have been remedied before being called.
 * Is responsible for detecting an END of game
 * @param window tetris game window w/ tetromino data we want to validate
 */
void tetris_drawEndScreen(Window * window) {
	// Draw a smiley face
	for(int i = IMAGE_Y; i < IMAGE_HEIGHT + IMAGE_Y; i++) {
		for(int j = IMAGE_X; j < IMAGE_WIDTH + IMAGE_X; j++) {
			float y = IMAGE_HEIGHT-(i-IMAGE_Y) - (((float)IMAGE_HEIGHT)/2);
			float x = (((float)IMAGE_HEIGHT)/IMAGE_WIDTH)*(j-IMAGE_X) - (((float)IMAGE_HEIGHT)/2);
			float rad_head = x*x + y*y;
			float rad_eyes = (abs(x)-70)*(abs(x)-70) + (y-30)*(y-30);
			float quad_rad;
			// Happy if we get over 80k :)
			// Even though that point count may overflow the screen-
			if (window->game.points < 80000) {
				quad_rad = abs((y+100)+0.01*x*x);
			} else {
				quad_rad = abs((y+100)-0.01*x*x);
			}
			if(rad_head > 150*150 && rad_head < 170*170) {
				window->frame[i][j] = (uint8_t) 255;
			} else if (rad_eyes < 20*20) {
				window->frame[i][j] = (uint8_t) 255;
			} else if (quad_rad < 10 && y < -55) {
				window->frame[i][j] = (uint8_t) 255;
			} else {
				window->frame[i][j] = (uint8_t) 0;
			}
		}
	}
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

    for (int i = 0; i < 4; i++) {
        int anyRowFlag = 0;
        // go over board from bottom to top & turn lines that need to be cleared on/off
        for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
            if (rowCompleted[row]) {
                for (int col = 0; col < BOARD_WIDTH; col++) {
                    window->game.board[BOARD_WIDTH * row + col] = (i%2) ? 150 : EMPTY_BOARD_CHAR;
                }
                anyRowFlag = 1;
            }
        }

        if (!anyRowFlag) {
            break;
        }

        drawRect(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, 1, 1, window->game.board);
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

//function definitions
/**
 * @brief Draw a rectangle of data into the window frame buffer
 *
 * @param window window to draw to
 * @param x_start initial x position in image
 * @param y_start initial y position in image
 * @param width width of 2D data rectangle
 * @param height height of 2D data rectangle
 * @param scaling_h scaling factor applied to rectangle (horizontal)
 * @param scaling_v scaling factor applied to rectangle (vertical)
 * @param data data to write to screen
 */
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t* data) {
	uint8_t** buff = window->frame;

    int indexRow = 0, indexCol;
    for (int row = y_start * scaling_v + IMAGE_Y; row < (y_start + height)  * scaling_v + IMAGE_Y; row++) {
        indexCol = 0;
        for (int col = x_start * scaling_h + IMAGE_X; col < (x_start + width) * scaling_h + IMAGE_X; col++) {
            if (row >= IMAGE_Y && row < IMAGE_Y + IMAGE_HEIGHT && col >= IMAGE_X && col < IMAGE_WIDTH + IMAGE_X && data[indexRow * width + indexCol] != 10) {
                buff[row][col] = data[indexRow * width + indexCol];
            }
            if ((col - (x_start * scaling_h + IMAGE_X) + 1) % scaling_h == 0) {
                indexCol++;
            }
        }
        if ((row - (y_start * scaling_v + IMAGE_Y) + 1) % scaling_v == 0) {
            indexRow++;
        }
    }
}

/**
 * @brief draw a rectangle with a single color
 *
 * @param window window to draw to
 * @param x_start initial x position in image
 * @param y_start initial y position in image
 * @param width width of rectangle
 * @param height height of rectangle
 * @param scaling_h scaling factor applied to rectangle (horizontal)
 * @param scaling_v scaling factor applied to rectangle (vertical)
 * @param color color to draw in rectangle
 */
void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color) {
	uint8_t** buff = window->frame;

    for (int row = y_start * scaling_v + IMAGE_Y; row < (y_start + height)  * scaling_v + IMAGE_Y; row++) {
        for (int col = x_start * scaling_h + IMAGE_X; col < (x_start + width) * scaling_h + IMAGE_X; col++) {
            if (row >= IMAGE_Y && row < IMAGE_Y + IMAGE_HEIGHT && col >= IMAGE_X && col < IMAGE_WIDTH + IMAGE_X) {
            	buff[row][col] = color;
            }
        }
    }
}

/**
 * @brief draw the background - a series of sine waves
 *
 * @param window window to draw to
 */
void tetris_drawBackground(Window* window) {
	float y_repeat = IMAGE_HEIGHT / 9.0;
	for (int row = IMAGE_Y; row < IMAGE_HEIGHT + IMAGE_Y; row++) {
	        for (int col = IMAGE_X; col < IMAGE_WIDTH + IMAGE_X; col++) {
	        	if (col < IMAGE_X + BOARD_X + BOARD_WIDTH * 2 + 5) {
	        		window->frame[row][col] = 125;
	        	} else {

		        	float sin_diff = fabs(9.0 * arm_sin_f32(0.9 * col) + 9.0 - fmod(row, y_repeat) );
		        	window->frame[row][col] = (sin_diff < 4.0) ? 20 : 70;
		        	if( (col+2) % 7 == 0) window->frame[row][col] = 20;
	        	}

	        }
	    }
 }
