#include <ncurses.h> 
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define HEIGHT 20
#define WIDTH 30

#define MAX_SEGMENTS 100
#define BOARD_OFFSET_Y 3

enum Color {
    EMPTY_COLOR = 1, 
    SEGMENT_COLOR = 2, 
    TEXT_COLOR = 3
}; 

enum Direction {
    UP = 1, 
    RIGHT = 2, 
    DOWN = 3,
    LEFT = 4
}; 

struct Segment {
    int x;
    int y;
};

int board[HEIGHT][WIDTH];

enum Direction direction;
enum Direction nextDirection;

struct Segment segments[MAX_SEGMENTS];
int segmentCount = 0;
int snakeX = 0;
int snakeY = 0;
int score = 0;
int speed = 100;

// Utils
void msleep(int ms) { usleep(ms * 1000); }

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

bool collides_with_segment(int x, int y) {
    for (int i = 1; i < segmentCount; i++)
        if (segments[i].x == x && segments[i].y == y) return true;
    return false;
}

void clear_board(void) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++) board[y][x] = EMPTY_COLOR;
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
        attron(COLOR_PAIR(SEGMENT_COLOR));
        mvprintw(segments[i].y + BOARD_OFFSET_Y, 2 * segments[i].x, "  ");
    }
}

void draw_info(void) {
    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(0, 0, "HamOS Tron");
    mvprintw(1, 0, "Score %i", score);
}

void draw(void) {
    clear();
    draw_info();
    draw_board();
    draw_snake();
    refresh();
}

void add_new_segment(int x, int y) {
    struct Segment segment;

    segment.x = x;
    segment.y = y;
    segments[segmentCount] = segment;
    segmentCount++;
}

void init_snake(void) {
    snakeX = WIDTH / 2;
    snakeY = HEIGHT / 2;
        
    add_new_segment(snakeX, snakeY);
}

bool move_tron() {
    int deltaX = 0;
    int deltaY = 0;

    delta_from_direction(&deltaX, &deltaY);

    snakeX += deltaX;
    snakeY += deltaY;

    if (snakeX < 0 || snakeX > WIDTH - 1
    || snakeY < 0 || snakeY > HEIGHT - 1
    || collides_with_segment(snakeX, snakeY)) return false;

    add_new_segment(snakeX, snakeY);
    score++;
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
        if (!move_tron()) return NULL;
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
    init_pair(SEGMENT_COLOR, COLOR_WHITE, COLOR_CYAN); 
    init_pair(TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);

    keypad(stdscr, TRUE);
    noecho();

    start_new_game();
}