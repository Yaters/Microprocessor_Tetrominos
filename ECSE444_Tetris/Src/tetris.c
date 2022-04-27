#include <tetris.h>
#include "fontlib.h"

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
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, const uint8_t* data) {
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
 *  Print a string to the image buffer at position x, y
 *
 *	@param window window with the image buffers.
 *	@param buffer string to write to screen
 *	@param x horizontal position
 *	@param y vertical position
 */
void draw_str(Window * window, char* buffer, int x, int y) {

	x += 3; // Avoid back porch
	y += 70;

	char cur_char = buffer[0];
	if(cur_char >= 97) cur_char -= 32;
	int i = 0;
	while(cur_char != '\0') {
		char* bitmap = font_map[cur_char - 32]; // 32 = ' '
		// y-value precursor
		for(int h = 0; h < 70; h++) {
			for (int w = 0; w < 7; w++) {
				// Pre- and Post- empty space on x & y
				if (h < 10 || w == 0 || h >= 60 || w == 6) {
					window->frame[y+h][x+w] = 100;
					continue;
				}
				int array_index = ((h-10)/10) * 5 + (w-1); // h/10 = floor division, to stretch
				window->frame[y+h][x+w] = bitmap[array_index] ? 170 : 100;
			}
		}
		x += 7; // 1 pre, 5 char, 1 post
		i++;
		cur_char = buffer[i];
		//'a' -> 'A' for example
		if(cur_char >= 97) cur_char -= 32;
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
	draw_str(window, "Points:", 27, 8); // I make the points PART of the background
	draw_str(window, "Next:", 27, 187); // I make the next tetromino PART of the background
	tetris_draw_scoreboard(window);
 }

/**
 * @brief Write points and next tetromino to the screen while playing.
 *
 * @param window window
 */
void tetris_draw_scoreboard(Window* window) {
    char point_str[10];
    sprintf(point_str, "%lu", window->game.points);
    draw_str(window, point_str, 27, 97);
    drawRect_color(window, 14, 21, 6, 5, HORIZ_SCALE, VERT_SCALE, (uint8_t) 100);
    drawRect(window, 15, 22, 4, 4, HORIZ_SCALE, VERT_SCALE, window->game.nextTetromino);
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
			if (window->game.points < MAX_POINTS) {
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
