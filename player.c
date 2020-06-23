#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#define ENTER 10
#define NUMBER_OF_GAMES 6
#define NUMBER_OF_APPLICATIONS 3

int cursorY = 0;

char applications[NUMBER_OF_APPLICATIONS][50] = {
    "Calculator",
    "Navigator",
    "Donut"
};

char games[NUMBER_OF_GAMES][50] = {
    "Blastar",
    "Minesweeper",
    "Snake",
    "Tetris",
    "Tic-Tac-Toe",
    "Tron"
};

char file_names[9][50] = {
    "calculator",
    "navigator",
    "donut",
    "blastar",
    "minesweeper",
    "snake",
    "tetris",
    "tictactoe",
    "tron"
};

void draw_cursor_char(char c) {
    mvaddch(
        cursorY + (cursorY > NUMBER_OF_APPLICATIONS - 1 ? 2 : 0) + 4, 
        3, 
        c
    );
}

void clear_cursor(void) { 
    draw_cursor_char(' '); 

}

void draw_cursor(void) { 
    draw_cursor_char('>'); 
}

void draw_select_menu(void) {
    attron(A_BOLD);
    mvprintw(3, 0, "APPLICATIONS");
    attroff(A_BOLD);
    for (int i = 0; i < NUMBER_OF_APPLICATIONS; i++) {
        if (cursorY == i) attron(A_UNDERLINE);
        mvprintw(4 + i, 4, applications[i]);
        attroff(A_UNDERLINE);
    }
    
    attron(A_BOLD);
    mvprintw(5 + NUMBER_OF_APPLICATIONS, 0, "GAMES");
    attroff(A_BOLD);
    for (int i = 0; i < NUMBER_OF_GAMES; i++) {
        if (cursorY - 2 == i) attron(A_UNDERLINE);
        mvprintw(6 + NUMBER_OF_APPLICATIONS + i, 4, games[i]);
        attroff(A_UNDERLINE);
    }
}


int main(void) {
    initscr();
    curs_set(0);

    start_color();
    keypad(stdscr, TRUE);
    noecho();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    attron(COLOR_PAIR(1));

    attron(A_BOLD);
    mvprintw(0, 0, "Welcome to HamOS!");
    attroff(A_BOLD);
    mvprintw(1, 0, "Select program:");

    
    refresh();

    draw_select_menu();
    draw_cursor();
    int ch;
    while ((ch = getch()) != ENTER) {
        clear_cursor();

        if (ch == KEY_DOWN)
            if (cursorY < NUMBER_OF_APPLICATIONS + NUMBER_OF_GAMES - 1) cursorY++;
        
        if (ch == KEY_UP)
            if (cursorY > 0) cursorY--;

        draw_select_menu();
        draw_cursor();
    }

    char command[200];
    sprintf(
        command, 
        "gcc %s/%s.c -lncurses -lm -lpthread && ./a.out",
        cursorY > 2 ? "Games" : "Applications",
        file_names[cursorY]
    );

    clear();
    system(command);
}