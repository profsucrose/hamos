#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define KNOWN_CHANCE_LOSS 1

#define ENTER 10
#define BOMBS_MAX_RANGE 10

/*
Unknown = -1
[# of bombs in proximity] = [0 - 8]
Bomb = 9
*/
enum PIECES {
    UNKNOWN = -1,
    KNOWN = 2,
    BOMB = 9
};

enum COLORS {
    CURSOR_COLOR = 1,
    TEXT_COLOR = 1,
    UNKNOWN_COLOR = 1,
    PROX_ONE_COLOR = 2,
    PROX_TWO_COLOR = 3,
    PROX_THREE_COLOR = 4,
    PROX_FOUR_COLOR = 5,
    PROX_FIVE_COLOR = 6,
    PROX_SIX_COLOR = 7,
    PROX_SEVEN_COLOR = 8,
    PROX_EIGHT_COLOR = 9,
    BOMB_COLOR = 10,
    FLAG_COLOR = 11
};

int board[50][50];
bool flaggedBoard[50][50];
int flagsLeft = 0;
int cursorX = 0, cursorY = 0;
bool bombsRevealed = false;
int bombs = 0;

int bombs_min;
int HEIGHTS_PER_DIFFICULTY[3] = {
    5,
    7,
    11
};

int WIDTHS_PER_DIFFICULTY[3] = {
    10,
    18,
    30
};

int BOMB_MIN_PER_DIFFICULTY[3] = {
    9,
    20,
    35
};

int width;
int height;

// Utils
bool is_board_full(void) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
            if (board[y][x] == UNKNOWN) return false;
    }
    return true;
}

void clear_board(void) {
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) board[y][x] = UNKNOWN;
}

void create_bombs(void) {
    for (int i = 0; i < bombs; i++) {
        board[rand() % height][rand() % width] = 9;
    }
}

bool is_outside_board(int x, int y) {
    return y < 0 
        || y > height - 1
        || x < 0 
        || x > width - 1;
}

int is_bomb(int x, int y) { 
    if (is_outside_board(x, y)) return 0;
    return board[y][x] == BOMB; 
}

int get_bomb_proximities(int x, int y) {
    return is_bomb(x, y)
        + is_bomb(x - 1, y)
        + is_bomb(x + 1, y)
        + is_bomb(x, y - 1)
        + is_bomb(x + 1, y - 1)
        + is_bomb(x - 1, y - 1)
        + is_bomb(x, y + 1)
        + is_bomb(x + 1, y + 1)
        + is_bomb(x - 1, y + 1);
}

int get_random_direction(void) {
    return (rand() % 2) ? 1 : -1;
}

void get_knowns(int x, int y, int chance) {
    if (!is_outside_board(x, y)) 
        board[y][x] = KNOWN;

    if ((rand() % 20) <= chance) {
        switch (rand() % 2) {
            case 0: {
                get_knowns(x + get_random_direction(), y, chance - KNOWN_CHANCE_LOSS);
                break;
            }
            case 1: {
                get_knowns(x, y + get_random_direction(), chance - KNOWN_CHANCE_LOSS);
                break;
            }
        }

    }
}

int get_number_of_bombs(void) {
    return rand() % BOMBS_MAX_RANGE + bombs_min;
}

// Drawing functions
void draw_board(void) { 
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (flaggedBoard[y][x]) {
                attron(COLOR_PAIR(FLAG_COLOR) | A_BOLD);
                mvaddch(2 * y + 2, 2 * x, 'F');
                attroff(A_BOLD);
                continue;
            }
            switch (board[y][x]) {
                case BOMB: {
                    char c = bombsRevealed ? 'B' : '?';
                    if (bombsRevealed) {
                        attron(COLOR_PAIR(BOMB_COLOR) | A_BOLD);
                    } else {
                        attron(COLOR_PAIR(UNKNOWN_COLOR));
                    }
                    mvaddch(2 * y + 2, 2 * x, c);
                    attroff(A_BOLD);
                    break;
                }
                case UNKNOWN: {
                    attron(COLOR_PAIR(UNKNOWN_COLOR));
                    mvaddch(2 * y + 2, 2 * x, '?');
                    break;
                }
                case KNOWN: {
                    attron(A_BOLD);
                    int proxNum = get_bomb_proximities(x, y);
                    attron(COLOR_PAIR(proxNum + 1));
                    mvprintw(2 * y + 2, 2 * x, "%i", proxNum);
                    attroff(A_BOLD);
                    break;
                }
            }
        }
    }
}

void draw_info(void) {
    mvprintw(1, 0, "Flags %i", flagsLeft);
}

void draw(void) {
    draw_info();
    draw_board();
    refresh();
}

// Main functions
void start_new_game(void) {
    clear();
    mvprintw(0, 0, "HamOS Minesweeper", flagsLeft);
    mvprintw(2, 0, "Select Difficulty:");

    mvprintw(4, 0, "1. EASY");
    mvprintw(5, 0, "2. MEDIUM");
    mvprintw(6, 0, "3. HARD");

    int ch;
    while ((ch = getch())) {
        if (ch == '1'
        || ch == '2'
        || ch == '3') break;
    }

    int selectedDifficulty = ch - 49; // 50 is charcode for 1
    width = WIDTHS_PER_DIFFICULTY[selectedDifficulty];
    height = HEIGHTS_PER_DIFFICULTY[selectedDifficulty];
    bombs_min = BOMB_MIN_PER_DIFFICULTY[selectedDifficulty];

    clear();
    mvprintw(0, 0, "HamOS Minesweeper", flagsLeft);

    bombsRevealed = false;
    bombs = get_number_of_bombs();
    flagsLeft = bombs;

    clear_board();
    create_bombs();

    cursorX = rand() % (width - 1) + 1;
    cursorY = rand() % (height - 1) + 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++)
            flaggedBoard[y][x] = false;
    }
    mvaddch(2 * cursorY + 3, 2 * cursorX, '^');

    get_knowns(cursorX, cursorY, 300);
    draw();

    while ((ch = getch())) {
        attron(COLOR_PAIR(CURSOR_COLOR));
        mvaddch(2 * cursorY + 3, 2 * cursorX, ' ');

        if (ch == KEY_UP)
            if (cursorY > 0) cursorY--;

        if (ch == KEY_DOWN)
            if (cursorY < height - 1) cursorY++;
        
        if (ch == KEY_LEFT)
            if (cursorX > 0) cursorX--;
        
        if (ch == KEY_RIGHT)
            if (cursorX < width - 1) cursorX++;
        
        if (ch == 'f') {
            if (flaggedBoard[cursorY][cursorX] || flagsLeft) {
                flaggedBoard[cursorY][cursorX] = !flaggedBoard[cursorY][cursorX];
                flagsLeft += flaggedBoard[cursorY][cursorX] ? -1 : 1;
            }
        }
            

        if (ch == ENTER) {
            if (board[cursorY][cursorX] == UNKNOWN) {
                board[cursorY][cursorX] = KNOWN;
                
            } else if (board[cursorY][cursorX] == BOMB) {
                bombsRevealed = true;
                break;
            }

            if (is_board_full()) {
                bombsRevealed = true;
                mvprintw(height * 2 + 1, 0, "You won! (Any character to exit or 'r' to play again)");
                draw();
                ch = getch();
                if (ch == 'r') start_new_game();
                endwin();
            }
        }

        attron(COLOR_PAIR(CURSOR_COLOR));
        mvaddch(2 * cursorY + 3, 2 * cursorX, '^');
        draw();
    }

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(height * 2 + 1, 0, "Game over! You mined a bomb! (Any character to exit or 'r' to play again)");
    draw();
    ch = getch();
    if (ch == 'r') start_new_game();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);
    srand(time(0));

    start_color();
    keypad(stdscr, TRUE);
    noecho();

    init_pair(UNKNOWN_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(PROX_ONE_COLOR, COLOR_BLUE, COLOR_BLACK);
    init_pair(PROX_TWO_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(PROX_THREE_COLOR, COLOR_GREEN, COLOR_BLACK);
    init_pair(PROX_FOUR_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(PROX_FIVE_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(PROX_SIX_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(PROX_SEVEN_COLOR, COLOR_GREEN, COLOR_BLACK);
    init_pair(PROX_EIGHT_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(BOMB_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(FLAG_COLOR, COLOR_GREEN, COLOR_RED);

    start_new_game();
}