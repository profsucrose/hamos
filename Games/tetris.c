#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "../logger.h"

#define TEXT_COLOR 9

#define WIDTH 10
#define HEIGHT 20

#define EMPTY 0
#define EMPTY_COLOR 1

#define PIECE_COUNT 7
#define BOARD_OFFSET_Y 2
#define BOARD_OFFSET_X 8

char board[HEIGHT][WIDTH];
int pieces[PIECE_COUNT][4][4] = {
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 2, 2, 0},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 3, 3, 0},
        {3, 3, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {4, 4, 0, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 5, 0},
        {5, 5, 5, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 6, 0, 0},
        {0, 6, 0, 0},
        {0, 6, 0, 0},
        {0, 6, 0, 0}
    },
    {
        {7, 0, 0, 0},
        {7, 7, 7, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }
};

int pieceY, pieceX, pieceType, gravity, score, level, linesClearedTotal;
int holdPieceType = -1;
int piece[3][3];
bool skip, breakPieceLoop, didHoldPiece;
int nextPieces[3];

void msleep(int ms) { usleep(ms * 1000); }

// Drawing functions
void draw_border() {
    attron(COLOR_PAIR(1));
    // Corners
    mvaddch(BOARD_OFFSET_Y - 1, BOARD_OFFSET_X, '+');
    mvaddch(BOARD_OFFSET_Y - 1, WIDTH + BOARD_OFFSET_X, '+');
    mvaddch(HEIGHT + BOARD_OFFSET_Y - 1, BOARD_OFFSET_X, '+');
    mvaddch(HEIGHT + BOARD_OFFSET_Y - 1, WIDTH + BOARD_OFFSET_X, '+');

    // Walls
    for (int y = 0; y < HEIGHT - 1; y++) {
        mvaddch(y + BOARD_OFFSET_Y, BOARD_OFFSET_X, '|');
        mvaddch(y + BOARD_OFFSET_Y, WIDTH + BOARD_OFFSET_X, '|');
    }

    for (int x = 0; x < WIDTH - 1; x++) {
        mvaddch(BOARD_OFFSET_Y - 1, x + 1 + BOARD_OFFSET_X, '-');
        mvaddch(HEIGHT + BOARD_OFFSET_Y - 1, x + 1 + BOARD_OFFSET_X, '-');
    }
}

void print_info() {
    mvprintw(0, 0, "HamOS Tetris");
    mvprintw(2, 21, "Score %i", score);
    mvprintw(4, 21, "Level %i", level);
}

void draw_piece_slot(int pX, int pY, int pT) {
    attron(COLOR_PAIR(1));
    mvaddch(pY, pX + 1, '+');
    mvaddch(pY, pX + 6, '+');
    mvaddch(pY + 5, pX + 1, '+');
    mvaddch(pY + 5, pX + 6, '+');

    // Walls
    for (int y = 1; y < 5; y++) {
        mvaddch(pY + y, pX + 1, '|');
        mvaddch(pY + y, pX + 6, '|');
    }

    for (int x = 1; x < 5; x++) {
        mvaddch(pY, pX + x + 1, '-');
        mvaddch(pY + 5, pX + x + 1, '-');
    }

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (pieces[pT][y][x] != EMPTY) {
                attron(COLOR_PAIR(pT + 2));
                mvaddch(pY + y + 1, pX + x + 2, ' ');
            }
        }
    }
}

void draw_held_piece() {
    mvaddch(BOARD_OFFSET_Y, 1, '+');
    mvaddch(BOARD_OFFSET_Y, 6, '+');
    mvaddch(BOARD_OFFSET_Y + 5, 1, '+');
    mvaddch(BOARD_OFFSET_Y + 5, 6, '+');

    // Walls
    for (int y = 1; y < 5; y++) {
        mvaddch(y + BOARD_OFFSET_Y, 1, '|');
        mvaddch(y + BOARD_OFFSET_Y, 6, '|');
    }

    for (int x = 1; x < 5; x++) {
        mvaddch(BOARD_OFFSET_Y, x + 1, '-');
        mvaddch(BOARD_OFFSET_Y + 5, x + 1, '-');
    }

    if (holdPieceType == -1) return;
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (pieces[didHoldPiece + 1][y][x] != EMPTY) {
                attron(COLOR_PAIR(didHoldPiece + 3));
                mvaddch(BOARD_OFFSET_Y + 1 + y, x + 2, ' ');
            } else {
                attron(COLOR_PAIR(1));
                mvaddch(BOARD_OFFSET_Y + 1 + y, x + 2, ' ');
            }
        }
    }

    
}

void log_board(void) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) logprintf("%i ", board[y][x]);
        logprintf("\n", 0);
    }
}

void draw_board(void) {
    attron(COLOR_PAIR(EMPTY_COLOR));
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            attron(COLOR_PAIR(board[y][x] + 1));
            mvaddch(y + BOARD_OFFSET_Y, x + 1 + BOARD_OFFSET_X, ' ');
        }
        printw("\n");
    }
}

void set_piece(int piece[3][3], int pieceX, int pieceY) {
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (piece[y][x] != EMPTY) board[pieceY + y][pieceX + x] = piece[y][x];
        }
    }
}

void clear_piece(int piece[3][3], int pieceX, int pieceY) {
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (piece[y][x] != EMPTY) board[pieceY + y][pieceX + x] = EMPTY;
        }
    }
}

bool check_collision(int piece[3][3], int pieceX, int pieceY) {
    // log_board();
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            if (piece[y][x] != EMPTY) {
                // logprintf("Piece pixel %i \n", piece[y][x]);
                // logprintf("Board pixel %i \n", board[pieceY + y][pieceX + x]);
                // logprintf("In-Piece Y: %i \n", y);
                // logprintf("In-Piece X: %i \n", x);
                // logprintf("Piece X: %i \n", pieceX);
                // logprintf("Piece Y: %i \n", pieceY);
                // logprintf("boardY: %i \n", pieceY + y);
                // logprintf("boardX: %i \n", pieceX + x);
            }
            
            if (piece[y][x] != EMPTY && board[pieceY + y][pieceX + x] != EMPTY) return true;
        }
    }
    return false;
}

void rotate_piece_clockwise() {
    int newPiece[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++) 
            newPiece[y][x] = 0;
    
    // Transpose
    for (int y = 0; y < 3; y++) 
        for (int x = 0; x < 3; x++) 
            newPiece[x][y] = piece[y][x];

    // Swap columns
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3/2; x++) {
            int tmp = newPiece[y][3 - x - 1];
            newPiece[y][3 - x - 1] = newPiece[y][x];
            newPiece[y][x] = tmp;
        }
    }

    for (int y = 0; y < 3; y++) { 
        for (int x = 0; x < 3; x++) {
            logprintf("%i ", newPiece[y][x]);
            piece[y][x] = newPiece[y][x];
        }
        logprintf("\n", 0);
    }
}

void draw_piece_slots() {
    draw_piece_slot(20, 6, nextPieces[0]);
    draw_piece_slot(20, 11, nextPieces[1]);
    draw_piece_slot(20, 16, nextPieces[2]);
}

// Utils
int get_left_edge() {
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) 
            if (piece[y][x] != EMPTY) return x;
    }
    return 2;
}

int get_right_edge() {
    for (int x = 2; x >= 0; x--) {
        for (int y = 0; y < 3; y++) 
            if (piece[y][x] != EMPTY) return x;
    }
    return 0;
}

int get_height() {
    for (int y = 2; y > 0; y--) {
        for (int x = 0; x < 3; x++)
            if (piece[y][x] != EMPTY) return y + 1;
    }
    return 0;
}

int get_distance_to_ground() {
    int y = pieceY;
    while (!check_collision(piece, pieceX, y + 1)) {
            y++;
    }
    logprintf("%i \n", y - pieceY);
    return y;
}

void hard_drop() {
    gravity = 0;
}

int score_formula(int lines) {
    return (lines == 1 ? 40 : lines == 2 ? 100 : 300) * pow(2, level);
}

int get_gravity() {
    return 500 * pow(0.9, level);
}

void clear_lines() {
    bool linesCleared[4];
    int linesClearedCount = 0;
    // Check for full lines
    for (int y = HEIGHT - 1; y >= 0; y--) {
        bool isLineCleared = true;
        for (int x = 0; x < WIDTH - 1; x++) 
            if (board[y][x] == EMPTY) isLineCleared = false;
        if (isLineCleared) {
            linesCleared[linesClearedCount] = y;
            linesClearedCount++;
        }
    }

    // Shift lines downward if line(s) were cleared
    if (linesClearedCount > 0) {
        logprintf("Cleared at least one line \n", 0);
        for (int i = 0; i < linesClearedCount; i++) {
            logprintf("Cleared layer %i \n", HEIGHT - linesCleared[i]);
            for (int y = HEIGHT - linesCleared[i]; y > 0; y--) {
                for (int x = 0; x < WIDTH - 1; x++) {
                    board[y + 1][x] = board[y][x];
                    board[y][x] = EMPTY;
                }
            }
        }

        score += score_formula(linesClearedCount);
        linesClearedTotal += linesClearedCount;
        level = linesClearedTotal / 3;
        print_info();
        refresh();
    }
}

void hold_piece() {
    int tmp = holdPieceType;
    if (holdPieceType != -1) {
        int tmp = pieceType;
        holdPieceType = pieceType;
        pieceType = tmp;
        clear_piece(piece, pieceX, pieceY);
        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++) piece[y][x] = pieces[holdPieceType][y][x];
    } else {
        didHoldPiece = true;
    }
    holdPieceType = pieceType;
}

// Input loop
void *input_loop(void *vargp) {
    while (1) {
        int ch = getch();
        gravity = get_gravity();

        clear_piece(piece, pieceX, pieceY);
        if (ch == KEY_LEFT)
            if (pieceX > -get_left_edge()) pieceX--;

        if (ch == KEY_RIGHT)
            if (pieceX < WIDTH - 2 - get_right_edge()) pieceX++;

        // Hacky solution to make square pieces not rotatable
        if (ch == KEY_UP && pieceType != 1) {
            rotate_piece_clockwise();
        }

        if (ch == KEY_DOWN) {
            if (!check_collision(piece, pieceX, pieceY + 2) && pieceY < HEIGHT - get_height() - 2) pieceY++;
        }

        if (ch == SPACE) {
            hard_drop();
        }

        if (ch == 'c') {
            hold_piece();
        }

        if (check_collision(piece, pieceX, pieceY + 1)) {
            breakPieceLoop = true;
        }
        
        set_piece(piece, pieceX, pieceY);
        draw_board();
        draw_border();
        print_info();
        draw_held_piece();
        draw_piece_slots();
        refresh();
    }
}


// Main game function
void *game_loop(void *vargp) {
    draw_board();
    refresh();
    
    for (int i = 0; i < 3; i++) nextPieces[i] = rand() % PIECE_COUNT;

    while (1) {
        pieceY = 0, pieceX = 0, pieceType = nextPieces[0], gravity = get_gravity();

        nextPieces[0] = nextPieces[1];
        nextPieces[1] = nextPieces[2];
        nextPieces[2] = rand() % PIECE_COUNT;

        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) piece[y][x] = pieces[pieceType][y][x];
        if (check_collision(piece, pieceX, pieceY)) break;
        set_piece(piece, pieceX, pieceY);
        while (pieceY < HEIGHT - get_height() - 1) {
            clear();
            draw_board();
            draw_border();
            print_info();
            draw_held_piece();
            draw_piece_slots();
            refresh();
            if (didHoldPiece) {
                clear_piece(piece, pieceX, pieceY);
                didHoldPiece = false;
                break;
            }
            msleep(gravity);
            if (skip) {
                gravity = get_gravity();
                skip = false;
            }

            clear_piece(piece, pieceX, pieceY);
            if (check_collision(piece, pieceX, pieceY + 2)) {
                set_piece(piece, pieceX, pieceY + 1);
                logprintf("Collision detected \n", 0);
                break;
            }
            pieceY++;

            set_piece(piece, pieceX, pieceY);
        }

        clear_lines();
    }
    set_piece(piece, pieceX, pieceY);

    return NULL;
}

void start_new_game() {
    clear();
    level = 0, score = 0;
    for (int y = 0; y < HEIGHT; y++) 
        for (int x = 0; x < WIDTH; x++) board[y][x] = EMPTY;
    pthread_t game_loop_thread, input_loop_thread; 

    // Create enemy and input threads
    pthread_create(&game_loop_thread, NULL, game_loop, NULL);  
    pthread_create(&input_loop_thread, NULL, input_loop, NULL);  

    pthread_join(game_loop_thread, NULL);
    pthread_cancel(input_loop_thread);

    attron(COLOR_PAIR(1));
    mvprintw(HEIGHT + BOARD_OFFSET_Y, 0, "Game Over!");
    mvprintw(HEIGHT + BOARD_OFFSET_Y + 1, 0, "Press any character to exit or 'r' to play again!");
    refresh();
    char ch = getch();
    if (ch == 'r') start_new_game();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);

    // Tetromino colors
    init_pair(2, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    init_pair(4, COLOR_WHITE, COLOR_GREEN);
    init_pair(5, COLOR_WHITE, COLOR_RED);
    init_pair(6, COLOR_WHITE, COLOR_WHITE); init_color(COLOR_WHITE, 1000,500,0); // Orange
    init_pair(7, COLOR_WHITE, COLOR_CYAN);
    init_pair(8, COLOR_WHITE, COLOR_BLUE);

    init_pair(9, COLOR_WHITE, COLOR_BLACK);

    attron(COLOR_PAIR(TEXT_COLOR) | A_BOLD);

    

    srand(time(NULL));

    keypad(stdscr, TRUE);
    noecho();

    start_new_game();
}