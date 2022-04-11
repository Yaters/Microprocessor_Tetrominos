#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"

// function prototypes
void drawRect(Window* window, int x_start, int y_start, int width, int height, char* data);
void refreshScreen(Window * window);


//function definitions
void drawRect(Window* window, int x_start, int y_start, int width, int height, char* data) {
    char* buff = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;

    int index = 0;
    for (int row = y_start; row < y_start + height; row++) {
        for (int col = x_start; col < x_start + width; col++) {
            if (row >= 0 && row < window->height && col >= 0 && col < window->width) {
                buff[row * window->width + col] = data[index];
            }
            index++;
        }
    }
}

// update curBuff to be one we want to present.
// update old buffer with new buffer data
void refreshScreen(Window * window) {
    window->curBuff = (window->curBuff + 1) % 2;
    char* buffOld = (window->curBuff == 1) ? window->imgBuff1 : window->imgBuff2;
    char* buffNew = (window->curBuff == 0) ? window->imgBuff1 : window->imgBuff2;
    for (int row = 0; row < window->height; row++) {
        for (int col = 0; col < window->width; col++) {
            buffOld[row * window->width + col] = buffNew[row * window->width + col];
        }
    }
}

#endif