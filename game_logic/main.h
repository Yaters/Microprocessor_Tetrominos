#ifndef MAIN_H
#define MAIN_H

#define SDL_MAIN_HANDLED

// #ifdef _MSC_VER
// #    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
// #endif

#include <stdio.h>
#include <stdlib.h>
#include "tetris.h"

#ifdef _WIN32
#include <Windows.h>
#define msleep(x) Sleep((x))
#else
#include <unistd.h>
#endif




// function prototypes
int process_user_input(Window * window);
extern void update_screen(Window* window);
void game_playing(Window* window, int event);
void game_paused(Window * window, int event);
void game_start(Window * window, int event);
void create_window(Window * window);
void end_application(Window* window);


// global variables
char shades[] = {' ', '@', 'B', '%', '8', '&', 'W', 'M', '#', '*', 'o', 'a', 'h', 'k', 'b', 'd', 'p', 'q', 'w', 'm', 'Z', 'O', '0', 'Q', 'L', 'C', 'J', 'U', 'Y', 'X', 'z', 'c', 'v', 'u', 'n', 'x', 'r', 'j', 'f', 't', '/', '\\', 
    			'|', '(', ')', '1', '{', '}', '[', ']', '?', '-', '+', '~', '<', '>', 'i', '!', 'l', 'I', ';', ':', ',', '"', '^', '`', '\'', '.' };
int numShades = 68;
#endif