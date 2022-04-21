#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"

// function prototypes
extern void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t* data);
extern void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color);
extern void refreshScreen(Window * window);


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
extern void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t* data) {
	uint8_t* buff = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;

    int indexRow = 0, indexCol;
    for (int row = y_start * scaling_v + IMAGE_Y; row < (y_start + height)  * scaling_v + IMAGE_Y; row++) {
        indexCol = 0;
        for (int col = x_start * scaling_h + IMAGE_X; col < (x_start + width) * scaling_h + IMAGE_X; col++) {
            if (row >= IMAGE_Y && row < IMAGE_Y + IMAGE_HEIGHT && col >= IMAGE_X && col < IMAGE_WIDTH + IMAGE_X && data[indexRow * width + indexCol] != '\0') {
                buff[row * FRAME_WIDTH + col] = data[indexRow * width + indexCol];
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
extern void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color) {
	uint8_t* buff = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;

    for (int row = y_start * scaling_v + IMAGE_Y; row < (y_start + height)  * scaling_v + IMAGE_Y; row++) {
        for (int col = x_start * scaling_h + IMAGE_X; col < (x_start + width) * scaling_h + IMAGE_X; col++) {
            if (row >= IMAGE_Y && row < IMAGE_Y + IMAGE_HEIGHT && col >= IMAGE_X && col < IMAGE_WIDTH + IMAGE_X) {
                buff[row * FRAME_WIDTH + col] = color;
            }
        }
    }
}

/**
 * @brief Swap the image buffers.
 *
 * @param window window with the image buffers.
 */
extern void refreshScreen(Window * window) {
    window->curBuff = (window->curBuff + 1) % 2;
}

#endif
