#ifndef MAIN_H
#define MAIN_H

#define SDL_MAIN_HANDLED

// #ifdef _MSC_VER
// #    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
// #endif

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#define msleep(x) Sleep((x))
#else
#include <unistd.h>
#endif

// structs
typedef struct Window {
    int width;
    int height;

    // image buffers
    char * imgBuff1;
    char * imgBuff2;
    int curBuff;
}Window;


// function prototypes
int process_user_input(Window * window);
void update_screen(Window* window);
Window* create_window(int width, int height);
void end_application(Window* window);


// global variables
char shades[] = {' ', '@', 'B', '%', '8', '&', 'W', 'M', '#', '*', 'o', 'a', 'h', 'k', 'b', 'd', 'p', 'q', 'w', 'm', 'Z', 'O', '0', 'Q', 'L', 'C', 'J', 'U', 'Y', 'X', 'z', 'c', 'v', 'u', 'n', 'x', 'r', 'j', 'f', 't', '/', '\\', 
    			'|', '(', ')', '1', '{', '}', '[', ']', '?', '-', '+', '~', '<', '>', 'i', '!', 'l', 'I', ';', ':', ',', '"', '^', '`', '\'', '.' };
int numShades = 68;
#endif