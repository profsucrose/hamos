#include <ncurses.h>
#include <stdlib.h>

#define HEIGHT 20
#define WIDTH 20

/*
Should use an enum oh well

Unknown = -1
[# of bombs in proximity] = [0 - 8]
Bomb = 9
*/
int board[HEIGHT][WIDTH];

int cursorX = 0, cursorY = 0;

// Drawing functions
void draw_board() {

}

// Utils
void clear_board() {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++) board[y][x] = -1;
}

void start_new_game(void) {
    clear_board();
    cursorX = COLS / 2;
    cursorY = LINES / 2;
}

int main(void) {
    initscr();
    curs_set(0);
    srand(time(0));

    start_color();
    attron(A_BOLD);
    init_pair(EMPTY_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(SNAKE_COLOR, COLOR_WHITE, COLOR_RED); 
    init_pair(APPLE_COLOR, COLOR_WHITE, COLOR_CYAN); init_color(COLOR_CYAN, 1000, 500, 500);
    init_pair(TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);

    keypad(stdscr, TRUE);
    noecho();

    start_new_game();
}