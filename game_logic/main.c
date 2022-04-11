#include "main.h"
#include "graphics.h"
#include <stdlib.h>


int main(void) {
    HWND consoleWindow;
    AllocConsole();
    consoleWindow = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(consoleWindow, 1);
    Window* window = create_window(150, 50);


    while (1) {
        // process button presses (update game state)
        int event = process_user_input(window);

        if (event > 0) {
            // update screen
            update_screen(window);
        }

        // refresh rate of 10Hz
        msleep(100);

        if (event == -1) {
            break;
        }
        
    }
    end_application(window);
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
                int x = rand() % 150;
            int y = rand() % 50;
    switch (c) {
        case 'a':
            refreshScreen(window);
        break;
        case '\n':
            return 0;
        break;
        case 'q':
            return -1;
        break;
        case 'z':
            drawRect(window, x, y, 10, 10, data);
        break;
        case 'x':
            drawRect(window, 30, 30, 10, 10, data);
        break;
    }

    return 1;
}

void update_screen(Window * window) {
    system("cls");
    if (window->curBuff == 0) {
        for (int row = 0; row < window->height; row++) {
            for (int col = 0; col < window->width; col++) {
                printf("%c", window->imgBuff1[row * window->width + col]);
            }
            printf("\n");
        }
    }
    else {
        for (int row = 0; row < window->height; row++) {
            for (int col = 0; col < window->width; col++) {
                printf("%c", window->imgBuff2[row * window->width + col]);
            }
            printf("\n");
        }
    }

}

Window* create_window(int width, int height) {
    // create window handle
    Window * window = (Window*) malloc(sizeof(Window));
    window->width = width;
    window->height = height;

    window->imgBuff1 = (char*) malloc(sizeof(char) * (width * height));
    window->imgBuff2 = (char*) malloc(sizeof(char) * (width * height));

    for (int i = 0; i < window->height * window->width; i++) {
        window->imgBuff1[i] = '@';
        window->imgBuff2[i] = '.';
    }
    window->curBuff = 0;
    printf("window w: %d, h: %d\n", window->width, window->height);
    return window;
}

void end_application(Window* window) {
    free(window);
}