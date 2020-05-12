#include <ncurses.h>
#include <dirent.h> 
#include <unistd.h>
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>

#include "hamos_app.h"

int load_dir(char* dirPath) {
    logprintf("Current path: %s \n", dirPath);
    
    if (chdir(dirPath) != 0) {
        printw("ERROR: Could not change directory (press any key to exit)");
        refresh();
        getch();
        endwin();
    }

    clear();
    int selectionId = 0;
    struct dirent *de;
    DIR *dir = opendir(".");
    attron(COLOR_PAIR(1) | A_BOLD);

    

    if (dir == NULL) {
        printw("ERROR: Could not open current directory (press any key to exit)");
        refresh();
        getch();
        endwin();
    }

    printw("HamOS Navigator, select file or subdirectory ('c' to exit):\n\n");

    attron(COLOR_PAIR(3));
    printw("UP\n");

    int listLength = 1;
    int listCapacity = 100;
    int* listFileLengths = realloc(NULL, listCapacity * sizeof(int));
    listFileLengths[0] = 2;
    while ((de = readdir(dir)) != NULL) {
        char* name = de->d_name;
        if (name[0] == '.') continue;
        listFileLengths[listLength] = strlen(name);
        listLength++;
        if (listLength == listCapacity) {
            listCapacity = listCapacity * 2;
            listFileLengths = realloc(NULL, listCapacity * sizeof(int));
        }
        printw("%s\n", name);
    }

    logprintf("Finished print loop\n", "");

    attron(COLOR_PAIR(2));
    mvprintw(selectionId + 2, listFileLengths[selectionId], "<");
    curs_set(0);
    set_background();
    refresh();

    keypad(stdscr, TRUE);
    logprintf("Starting input loop\n", "");
    while (1) {
        logprintf("Waiting for char\n", "");
        int ch = getch();
        mvprintw(selectionId + 2, listFileLengths[selectionId], " ");

        if (ch == 'c') {
            endwin();
            exit(1);
        }

        if (ch == ENTER) {
            logprintf("Enter key pressed\n", "");
            rewinddir(dir);
            logprintf("%s\n", dirPath);
            char* selectedDirectoryName;
            if (selectionId == 0) {
                selectedDirectoryName = "..";
            } else {
                int iter = 1;
                
                while ((de = readdir(dir)) != NULL) {
                    
                    char* name = de->d_name;

                    if (name[0] == '.') continue;
                    
                    if (selectionId == iter) {
                        selectedDirectoryName = name;
                        break;
                    }

                    iter++;
                }

                if (strstr(selectedDirectoryName, ".") != NULL) goto skipDirLoad;              
            }

            load_dir(selectedDirectoryName);
            skipDirLoad:;
        }

        if (ch == KEY_UP) {
            logprintf("UP Key Pressed\n", "");
            if (selectionId > 0) selectionId--;
        }

        if (ch == KEY_DOWN) 
            if (selectionId < listLength - 1) selectionId++;

        mvprintw(selectionId + 2, listFileLengths[selectionId], "<");
        refresh();
    }

    endwin();
}

int main(void) {
    char* currentDirectory = "~";

    chdir(currentDirectory);

    // Initialize curses
    initscr();
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_CYAN, COLOR_WHITE);

    load_dir(".");
}