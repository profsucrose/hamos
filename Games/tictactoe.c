#include <ncurses.h>
#include <stdlib.h>

#include "hamos_app.h"

#define PLAYER 'X'
#define BOT 'O'
#define EMPTY '_'

struct Move {
    int row, col;
};

int min(int x, int y) { return x < y ? x : y; }
int max(int x, int y) { return x > y ? x : y; }

int evaluate_bot(char b[3][3]) { 
    // Check rows
    for (int row = 0; row < 3; row++) {
        if (b[row][0] == b[row][1]
        && b[row][1] == b[row][2]) {
            if (b[row][0] == BOT) 
                return 10;
            else if (b[row][0] == PLAYER)
                return -10;
        }
    }

    // Check columns
    for (int col = 0; col < 3; col++) {
        if (b[0][col] == b[1][col]
        && b[1][col] == b[2][col]) {
            if (b[0][col] == BOT) 
                return 10;
            else if (b[0][col] == PLAYER)
                return -10;
        }
    }

    // Check diagonals
    if (b[0][0] == b[1][1]
    && b[1][1] == b[2][2]) {
        if (b[0][0] == BOT)
            return 10;
        else if (b[0][0] == PLAYER)
            return -10;
    }

    if (b[0][2] == b[1][1]
    && b[1][1] == b[2][0]) {
        if (b[0][0] == BOT)
            return 10;
        else if (b[0][0] == PLAYER)
            return -10;
    }

    // No victor, return 0
    return 0; 
} 

bool is_board_full(char b[3][3]) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) 
            if (b[row][col] == '_') return false;
    }
    return true;
}

char game_ended(char b[3][3]) {
    // Check rows
    for (int row = 0; row < 3; row++) {
        if (b[row][0] != EMPTY
        && b[row][0] == b[row][1]
        && b[row][1] == b[row][2])
            return b[row][0];
    }

    // Check columns
    for (int col = 0; col < 3; col++) {
        if (b[0][col] != EMPTY
        && b[0][col] == b[1][col]
        && b[1][col] == b[2][col])
            return b[0][col];
    }

    // Check diagonals
    if (b[0][0] != EMPTY
    && b[0][0] == b[1][1]
    && b[1][1] == b[2][2]) {
        return b[0][0];
    }

    if (b[0][2] != EMPTY
    && b[0][2] == b[1][1]
    && b[1][1] == b[2][0]) {
        return b[0][2];
    }

    // 0 if there's a draw
    if (is_board_full(b)) return 0;

    // -1 if there is no winner
    return -1; 
}

void render_game(char b[3][3]) {
    attron(COLOR_PAIR(1));
    printw("HamOS Tic-Tac-Toe, use arrow keys to select slot : \n\n");
    attron(COLOR_PAIR(3));
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) printw("%c  ", b[row][col]);
        printw("\n\n");
    }
}

void set_selection_char(int selectedRow, int selectedCol, char c) {
    mvaddch(selectedRow * 2 + 3, selectedCol * 3, c);
}

// Considers all ways the game can go and returns the value of the board
int minimax(char board[3][3], int depth, bool isMax) {
    int score = evaluate_bot(board);

    // If Maximizer won the game return their score
    if (score == 10)
        return score;
    
    // If minimizer won the game return their score
    if (score == -10)
        return score;

    // If board is full and there is no winner it is a tie
    if (is_board_full(board))
        return 0;
    
    // If maximizer's move
    if (isMax) {
        int best = -1000;

        // Traverse all cells
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (board[row][col] == EMPTY) {
                    // Make the move
                    board[row][col] = BOT;

                    // Call minimax recursively and choose
                    // the maximum value
                    best = max(best, minimax(board, depth + 1, !isMax));

                    // Undo the move
                    board[row][col] = EMPTY;
                }
            }
        }
        return best;
    } 
    
    // If minimizer's move
    else {
        int best = 1000;

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (board[row][col] == EMPTY) {
                    // Make the move
                    board[row][col] = PLAYER;

                    // Call minimax recursively and choose
                    // the minimum value
                    best = min(best, minimax(board, depth + 1, !isMax));

                    // Undo the move
                    board[row][col] = EMPTY;
                }
            }
        }
        return best;
    }
}

void make_bot_move(char board[3][3]) {
    int bestVal = -1000;
    struct Move bestMove;
    bestMove.row = -1;
    bestMove.col = -1;

    // Traverse all cells, evaluate minimax for 
    // all empty cells and return the cell with the optimal value.
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // Check if cell is empty
            if (board[row][col] == EMPTY) {
                // Make the move
                board[row][col] = BOT;

                // Compute evaluation for move
                int moveVal = minimax(board, 0, false);

                // Undo the move
                board[row][col] = EMPTY;

                // If the value of the current move is more than the best value, update best
                if (moveVal > bestVal) {
                    bestMove.row = row;
                    bestMove.col = col;
                    bestVal = moveVal;
                }
            }
        }
    }

    logprintf("Best move for bot is row: %i, ", bestMove.row);
    logprintf("col: %i with a value of ", bestMove.col);
    logprintf("%i\n", bestVal);

    board[bestMove.row][bestMove.col] = BOT;
}

void start_new_game(void) {
    clear();
    attron(A_BOLD);

    char b[3][3] = { 
        { EMPTY, EMPTY, EMPTY }, 
        { EMPTY, EMPTY, EMPTY }, 
        { EMPTY, EMPTY, EMPTY } 
    };
    char winner;
    int selectedRow = 0,
        selectedCol = 0;

    while ((winner = game_ended(b)) < 0) {
        clear();
        
        render_game(b);
        set_background();
        refresh();
        
        attron(COLOR_PAIR(2));
        
        set_selection_char(selectedRow, selectedCol, '^');
        while (1) {
            int ch = getch();
            set_selection_char(selectedRow, selectedCol, ' ');
            
            if (ch == KEY_RIGHT) 
                if (selectedCol < 2) selectedCol++;

            if (ch == KEY_LEFT) 
                if (selectedCol > 0) selectedCol--;

            if (ch == KEY_DOWN) 
                if (selectedRow < 2) selectedRow++;

            if (ch == KEY_UP) 
                if (selectedRow > 0) selectedRow--;

            if (ch == ENTER && b[selectedRow][selectedCol] == EMPTY) {
                b[selectedRow][selectedCol] = 'X';
                make_bot_move(b);
                break;
            }

            set_selection_char(selectedRow, selectedCol, '^');
        }
    }

    clear();
    render_game(b);
    move(8, 0);
    attron(COLOR_PAIR(2));

    if (winner == 0)
        printw("The game has come to a draw! (Press any character to exit or 'r' to play again)");
    else
        printw("Player %c has won! (Press any character to exit or 'r' to play again)", winner);
    set_background();
    char ch = getch();
    if (ch == 'r') start_new_game();
    clear();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_CYAN, COLOR_WHITE);

    keypad(stdscr, TRUE);

    start_new_game();
}