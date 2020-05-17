#include <ncurses.h>
#include <dirent.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  

#define ENTER 10

enum COLORS {
    TITLE = 1,
    ITEM = 2,
    CURSOR = 3
};

int selectionId = 0;

// Drawing
void draw_items(char files[50][50], int fileNum) {
    attron(COLOR_PAIR(ITEM));
    for (int i = 0; i < fileNum; i++) {
        if (i == selectionId)
            attron(A_UNDERLINE);
        mvprintw(i + 3, 4, files[i]);
        attroff(A_UNDERLINE);
    }
}

void draw_cursor_char(char c) {
    attron(COLOR_PAIR(CURSOR));
    mvaddch(selectionId + 3, 3, c);
}
void draw_cursor() {
    draw_cursor_char('>');
}

void clear_cursor() {
    draw_cursor_char(' ');
}

// Main 
void load_dir(char* dirPath) {
    clear();
    attron(COLOR_PAIR(TITLE) | A_BOLD);
    if (chdir(dirPath) != 0) {
        mvprintw(0, 0, "ERROR: Could not change directory (press any key to exit)");
        getch();
        endwin();
        return;
    }

    clear();
    selectionId = 0;

    struct dirent *de;
    DIR *dir = opendir(".");

    if (dir == NULL) {
        mvprintw(0, 0, "ERROR: Could not open current directory (press any key to exit)");
        getch();
        endwin();
        return;
    }

    mvprintw(0, 0, "HamOS Navigator");
    mvprintw(1, 0, "Select file or subdirectory ('c' to exit):");

    attron(COLOR_PAIR(ITEM) | A_NORMAL);

    char files[50][50];
    strcpy(files[0], "UP");
    int fileNum = 1;
    while ((de = readdir(dir)) != NULL) {
        char *name = de->d_name;
        if (name[0] == '.') continue;
        strcpy(files[fileNum], name);
        fileNum++;
    }

    draw_items(files, fileNum);

    draw_cursor();
    int ch;
    while ((ch = getch())) {
        if (ch == 'c') {
            endwin();
            return;
        }

        if (ch == ENTER
        && strchr(files[selectionId], '.') == NULL) 
            break;

        clear_cursor();
        if (ch == KEY_UP)
            if (selectionId > 0) selectionId--;
        
        if (ch == KEY_DOWN)
            if (selectionId < fileNum - 1) selectionId++;
        draw_cursor();
        draw_items(files, fileNum);
    }

    load_dir(selectionId == 0 ? ".." : files[selectionId]);
}

int main(void) {
    char* currentDirectory = "~";

    chdir(currentDirectory);

    // Initialize curses
    initscr();
    curs_set(0);
    start_color();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(TITLE, COLOR_WHITE, COLOR_BLACK);
    init_pair(CURSOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(ITEM, COLOR_CYAN, COLOR_BLACK);

    load_dir(".");
}