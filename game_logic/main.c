#include "main.h"
#include "graphics.h"
#include <stdlib.h>


int main(void) {
    HWND consoleWindow;
    AllocConsole();
    consoleWindow = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(consoleWindow, 1);
    Window window;
    create_window(&window);


    while (1) {
        // process button presses (update game state)
        int event = process_user_input(&window);



        if (event > 0) {
            // update the game state, and draw to frame buffer
            update_game_state(&window, event);
            refreshScreen(&window);
            // update screen
            update_screen(&window);
        }

        // refresh rate of 10Hz
        // msleep(100);

        if (event == -1) {
            break;
        }
        
    }
    end_application(&window);
    printf("Press Any Key to Continue\n");  
    getchar();    
}

char data[] = {'1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',
                '1','1','1','1','1','1','1','1','1','1',};
int process_user_input(Window * window) {
    char c = getchar();

    switch (c) {
        // move left = 1
        case 'a':
            return 1;
        break;
        // move right = 2
        case 'd':
            return 2;
        break;
        case '\n':
            return 0;
        break;
        case 'q':
            return -1;
        break;
        // rotate CC = 4
        case 'z':
            return 4;
        break;
        // rotate C = 3
        case 'x':
            return 3;
        break;
    }

    return 10;
}

// only used for testing & visual output on CMD
void update_screen(Window * window) {
    system("cls");
    if (window->curBuff == 0) {
        for (int row = 0; row < FRAME_HEIGHT; row++) {
            for (int col = 0; col < FRAME_WIDTH; col++) {
                printf("%c", window->imgBuff1[row * FRAME_WIDTH + col]);
            }
            printf("\n");
        }
    }
    else {
        for (int row = 0; row < FRAME_HEIGHT; row++) {
            for (int col = 0; col < FRAME_WIDTH; col++) {
                printf("%c", window->imgBuff2[row * FRAME_WIDTH + col]);
            }
            printf("\n");
        }
    }

}

void create_window(Window * window) {
    // initialize window
    window->width = IMAGE_WIDTH;
    window->height = IMAGE_HEIGHT;

    for (int i = 0; i < FRAME_HEIGHT * FRAME_WIDTH; i++) {
        window->imgBuff1[i] = '.';
        window->imgBuff2[i] = '.';
    }
    window->curBuff = 0;
    printf("window w: %d, h: %d\n", window->width, window->height);

    tetris_initialize_game(window);
}

void update_game_state(Window* window, int event) {

    switch (event) {
        // move left = 1
        case 1:
            tetris_move_left(window);
        break;

        // move right = 2
        case 2:
            tetris_move_right(window);
        break;

        // rotate clockwise
        case 3:
            tetris_rotate_C_tetromino(window);
        break;

        // rotate counter clockwise
        case 4:
            tetris_rotate_CC_tetromino(window);
        break;

        // drop piece
        case 5:
            tetris_move_down(window);
        break;

        // swap pieces
        case 6:
            tetris_move_down(window);
        break;

        default:
            tetris_move_down(window);
        break;
    }

    // draw game board
    drawRect_color(window, 0, 0, window->width, window->height, 1, 1, '@');
    // drawRect_color(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, ' ');
    drawRect(window, BOARD_X, BOARD_Y, BOARD_WIDTH, BOARD_HEIGHT, 2, 2, window->game.board);
    drawRect(window, BOARD_X + window->game.x, BOARD_Y + window->game.y, 4, 4, 2, 2, tetromino_current);

}


void end_application(Window* window) {

}