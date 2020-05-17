#include <ncurses.h> 
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "../logger.h"

#define HEIGHT 21
#define WIDTH 17

#define BOARD_OFFSET_Y 0

enum Color {
    MAZE_COLOR = 1,
    TEXT_COLOR = 2,
    DOT_COLOR = 3,
    WALL_COLOR = 4,
    PLAYER_COLOR = 5,
    SPACE_COLOR = 6,
    BLINKY_COLOR = 7,
    PINKY_COLOR = 8,
    INKY_COLOR = 9,
    CLYDE_COLOR = 10
}; 

enum Direction {
    UP = 1,
    DOWN = 2,
    RIGHT = 3,
    LEFT = 4
};

enum Piece {
    WALL = 1,
    DOT = 2,
    PLAYER = 3,
    SPACE = 4
};

int cleared_board[HEIGHT][WIDTH] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1 },
    { 1, 2, 1, 1, 2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 1, 2, 1 },
    { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
    { 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1 },
    { 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1 },
    { 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1, 1 },
    { 4, 4, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 4, 4 },
    { 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1 }, 
    { 2, 2, 2, 2, 2, 2, 1, 4, 4, 4, 1, 2, 2, 2, 2, 2, 2 },
    { 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1 },
    { 4, 4, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 4, 4 },
    { 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1 },
    { 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1 },
    { 1, 2, 1, 1, 2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 1, 2, 1 },
    { 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1 },
    { 1, 1, 2, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 1 },
    { 1, 2, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 1 },
    { 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1 },
    { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

int board[HEIGHT][WIDTH];

enum Direction direction;
enum Direction nextDirection;
int playerX = 8;
int playerY = 15;
int turnCounter;
int savedTurnCounter;

const int WARP_SPOT_ONE[2] = { 16, 9 };
const int WARP_SPOT_TWO[2] = { 0, 9 };

int blinkyX, blinkyY;
int pinkyX, pinkyY;
int inkyX, inkyY;
int clydeX, clydeY;

// Drawing
void draw_board(void) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++) {
            char *c = "  ";
            int color = 1;
            switch (board[y][x]) {
                case WALL:
                    color = WALL_COLOR;
                    break;
                case DOT:
                    c = "* ";
                    color = DOT_COLOR;
                    break;
                case PLAYER:
                    color = PLAYER_COLOR;
                    break;
                case SPACE:
                    color = SPACE_COLOR;
                    break;
            }
            attron(COLOR_PAIR(color));
            mvprintw(y, 2 * x, c);
        }
}

void draw(void) {
    draw_board();
    refresh();
}

// Utils
void msleep(int ms) { usleep(ms * 1000); }

void clear_board(void) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            board[y][x] = cleared_board[y][x];
}

bool is_wall(int x, int y) {
    return board[y][x] == WALL;
}

bool dots_left(void) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            if (board[y][x] == DOT) return true;
    return false;
}


// Main
void *game_loop(void *vargp) {
    draw();
    
    while (++turnCounter) {
        msleep(250);
        if (!dots_left()) break;
        board[playerY][playerX] = SPACE;
        int oldPlayerY = playerY;
        int oldPlayerX = playerX;
        if (direction == UP)
            if (!is_wall(playerX, playerY - 1)) playerY--;
        
        if (direction == DOWN)
            if (!is_wall(playerX, playerY + 1)) playerY++;

        if (direction == RIGHT)
            if (!is_wall(playerX + 1, playerY)) playerX++;

        if (direction == LEFT)
            if (!is_wall(playerX - 1, playerY)) playerX--;

        board[playerY][playerX] = PLAYER;
        draw();

        logprintf("%i ", playerX);
        logprintf("%i\n", playerY);
        if (playerX == WARP_SPOT_ONE[0]
        && playerY == WARP_SPOT_ONE[1]) {
            board[playerY][playerX] = SPACE;
            playerX = WARP_SPOT_TWO[0];
        } else if (playerX == WARP_SPOT_TWO[0]
        && playerY == WARP_SPOT_TWO[1]) {
            board[playerY][playerX] = SPACE;
            playerX = WARP_SPOT_ONE[0];
        }
    }
    
    return NULL;
}

void *input_loop(void *vargp) {
    int ch;
    while ((ch = getch())) {
        if (ch == KEY_UP) 
            if (!is_wall(playerX, playerY - 1)) 
                direction = UP;

        if (ch == KEY_DOWN) 
            if (!is_wall(playerX, playerY + 1)) direction = DOWN;

        if (ch == KEY_RIGHT) 
            if (!is_wall(playerX + 1, playerY)) direction = RIGHT;

        if (ch == KEY_LEFT) 
            if (!is_wall(playerX - 1, playerY)) direction = LEFT;
    }
    return NULL;
}

void start_new_game(void) {
    clear();

    clear_board();
    board[playerY][playerX] = PLAYER;
    turnCounter = 0;
    draw();

    direction = LEFT;
    pthread_t game_loop_thread, input_loop_thread; 

    // Create enemy and input threads
    pthread_create(&game_loop_thread, NULL, game_loop, NULL);
    pthread_create(&input_loop_thread, NULL, input_loop, NULL);

    pthread_join(game_loop_thread, NULL);
    pthread_cancel(input_loop_thread);

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(HEIGHT + BOARD_OFFSET_Y + 2, 0, "You win! (Any character to exit or 'r' to play again)");

    if (getch() == 'r') start_new_game();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);
    srand(time(0));

    start_color();
    init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_YELLOW); 
    init_pair(TEXT_COLOR, COLOR_WHITE, COLOR_BLACK); 
    init_pair(DOT_COLOR, COLOR_WHITE, COLOR_BLACK); 
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(BLINKY_COLOR, COLOR_WHITE, COLOR_RED);
    init_pair(PINKY_COLOR, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(INKY_COLOR, COLOR_WHITE, COLOR_CYAN);
    init_pair(CLYDE_COLOR, COLOR_WHITE, COLOR_GREEN); init_color(COLOR_GREEN, 1000, 500, 0); // Orange

    keypad(stdscr, TRUE);
    noecho();

    start_new_game();
}