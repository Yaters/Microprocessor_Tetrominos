#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"

// function prototypes
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char* data);
void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char color);
void refreshScreen(Window * window);


//function definitions
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char* data) {
    char* buff = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;

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

void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, char color) {
    char* buff = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;

    for (int row = y_start * scaling_v + IMAGE_Y; row < (y_start + height)  * scaling_v + IMAGE_Y; row++) {
        for (int col = x_start * scaling_h + IMAGE_X; col < (x_start + width) * scaling_h + IMAGE_X; col++) {
            if (row >= IMAGE_Y && row < IMAGE_Y + IMAGE_HEIGHT && col >= IMAGE_X && col < IMAGE_WIDTH + IMAGE_X) {
                buff[row * FRAME_WIDTH + col] = color;
            }
        }
    }
}

// update curBuff to be one we want to present.
// update old buffer with new buffer data
void refreshScreen(Window * window) {
    window->curBuff = (window->curBuff + 1) % 2;
    char* buffOld = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;
    char* buffNew = (window->curBuff == 0) ? window->imgBuff1 : window->imgBuff2;
}

#endif