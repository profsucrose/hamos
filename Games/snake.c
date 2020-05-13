#include <ncurses.h> 
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>


#include "../logger.h"

#define HEIGHT 20
#define WIDTH 30

#define MAX_SNAKE_LENGTH 100
#define SPEED_INCREASE_COEFFICIENT 0.75
#define BOARD_OFFSET_Y 3

enum Color {
    EMPTY_COLOR = 1, 
    SNAKE_COLOR = 2, 
    APPLE_COLOR = 3,
    TEXT_COLOR = 4
}; 

enum Direction {
    UP = 1, 
    RIGHT = 2, 
    DOWN = 3,
    LEFT = 4
}; 

struct SnakeSegment {
    int x;
    int y;
};

int board[HEIGHT][WIDTH];

enum Direction direction;
enum Direction nextDirection;

struct SnakeSegment segments[MAX_SNAKE_LENGTH];
int segmentCount = 0;
int snakeX = 0;
int snakeY = 0;
int appleX = 0;
int appleY = 0;
int score = 0;
int speed = 100;

// Utils
void msleep(int ms) { usleep(ms * 1000); }

void log_snake() {
    for (int i = 0; i < segmentCount; i++) {
        logprintf("(%i ", segments[i].x);
        logprintf("%i ) ", segments[i].y);
    }
    logprintf("\n", 0);
}

void delta_from_direction(int *deltaX, int *deltaY) {
    switch (direction) {
        case UP:    
            *deltaY = -1;
            break;
        case DOWN:  
            *deltaY = 1;
            break;
        case RIGHT: 
            *deltaX = 1;
            break;
        case LEFT:  
            *deltaX = -1;
            break;
    }
}

bool collides_with_snake(int x, int y) {
    for (int i = 1; i < segmentCount; i++)
        if (segments[i].x == x && segments[i].y == y) return true;
    return false;
}

void clear_board(void) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++) board[y][x] = EMPTY_COLOR;
}

void set_apple_loc() {
    appleX = (rand() % WIDTH);
    appleY = (rand() % HEIGHT);
}

// Drawing functions
void draw_board(void) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            attron(COLOR_PAIR(board[y][x]));
            mvprintw(y + BOARD_OFFSET_Y, 2 * x, "  ");
        }
    }
}

void draw_snake(void) {
    for (int i = 0; i < segmentCount; i++) {
        attron(COLOR_PAIR(SNAKE_COLOR));
        mvprintw(segments[i].y + BOARD_OFFSET_Y, 2 * segments[i].x, "  ");
    }
}

void draw_apple(void) {
    attron(COLOR_PAIR(APPLE_COLOR));
    mvprintw(appleY + BOARD_OFFSET_Y, 2 * appleX, "  ");
}

void clear_apple(void) {
    attron(COLOR_PAIR(EMPTY_COLOR));
    mvprintw(appleY, 2 * appleX, "  ");
}

void draw_info(void) {
    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(0, 0, "HamOS Snake");
    mvprintw(1, 0, "Score %i", score);
}

void draw(void) {
    clear();
    draw_info();
    draw_board();
    draw_snake();
    draw_apple();
    refresh();
}

void add_snake_segment(int x, int y) {
    struct SnakeSegment segment;

    segment.x = x;
    segment.y = y;
    segments[segmentCount] = segment;
    segmentCount++;
}

void init_snake(void) {
    snakeX = WIDTH / 2;
    snakeY = HEIGHT / 2;
    
    add_snake_segment(snakeX, snakeY);
    add_snake_segment(snakeX - 1, snakeY);
}

bool move_snake() {
    int deltaX = 0;
    int deltaY = 0;

    delta_from_direction(&deltaX, &deltaY);

    attron(COLOR_PAIR(EMPTY_COLOR));
    snakeX += deltaX;
    snakeY += deltaY;

    if (snakeX < 0 || snakeX > WIDTH - 1
    || snakeY < 0 || snakeY > HEIGHT - 1) {
        return false;
    }

    for (int i = segmentCount - 1; i > 0; i--) {
        segments[i].x = segments[i - 1].x;
        segments[i].y = segments[i - 1].y;
    }

    segments[0].x = snakeX;
    segments[0].y = snakeY;

    log_snake();

    return true;
}

// Threads
void *input_loop(void *vargp) {
    int ch;
    while ((ch = getch())) {
        if (ch == KEY_UP && direction != DOWN) nextDirection = UP;
        if (ch == KEY_DOWN && direction != UP) nextDirection = DOWN;
        if (ch == KEY_RIGHT && direction != LEFT) nextDirection = RIGHT;
        if (ch == KEY_LEFT && direction != RIGHT) nextDirection = LEFT;
    }

    return NULL;
}

void *game_loop(void *vargp) {
    while (1) {
        direction = nextDirection;
        msleep(speed);
        if (!move_snake() || collides_with_snake(snakeX, snakeY)) return NULL;
        if (snakeX == appleX
        && snakeY == appleY) {
            int deltaX = 0, deltaY = 0;
            delta_from_direction(&deltaX, &deltaY);
            logprintf("dX: %i ", deltaX);
            logprintf("dY: %i\n", deltaY);
            add_snake_segment(snakeX - deltaX, snakeY - deltaY);
            clear_apple();
            set_apple_loc();
            score += 10;
        }
        draw();
        direction = nextDirection;
    }
}

void start_new_game(void) {
    clear();
    clear_board();

    nextDirection = RIGHT;
    segmentCount = 0;
    score = 0;

    set_apple_loc();
    init_snake();
    draw();

    pthread_t game_loop_thread, input_loop_thread; 

    // Create enemy and input threads
    pthread_create(&game_loop_thread, NULL, game_loop, NULL);  
    pthread_create(&input_loop_thread, NULL, input_loop, NULL);  

    pthread_join(game_loop_thread, NULL);
    pthread_cancel(input_loop_thread);

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(HEIGHT + BOARD_OFFSET_Y + 1, 0, "Game over! (Press 'r' to play again!)");

    if (getch() == 'r') start_new_game();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);
    srand(time(0));

    start_color();
    attron(A_BOLD);
    init_pair(EMPTY_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(SNAKE_COLOR, COLOR_WHITE, COLOR_RED); init_color(COLOR_CYAN, 1000, 500, 500);
    init_pair(APPLE_COLOR, COLOR_WHITE, COLOR_CYAN);
    init_pair(TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);

    keypad(stdscr, TRUE);
    noecho();

    start_new_game();
}