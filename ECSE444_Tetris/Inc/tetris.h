#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "game_logic.h"

void tetris_drawBackground(Window* window);
void drawRect_color(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, uint8_t color);
void drawRect(Window* window, int x_start, int y_start, int width, int height, int scaling_h, int scaling_v, const uint8_t* data);
void tetris_drawEndScreen(Window * window);
void tetris_draw_scoreboard(Window* window);
void draw_str(Window * window, char* buffer, int x, int y);

#endif
