#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

struct Vector {
    int length;
    char *str;
} null_vector;

int yInc = 0;

struct Vector inputString(size_t initial_size) {
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char) * initial_size);

    if (!str) return null_vector;
    while (EOF != (ch = getch()) && ch != '\n') {
        str[len++] = ch;
        if (len * sizeof(char) > sizeof(str)) {
            str = realloc(str, (len + initial_size) * 2 * sizeof(char));
        }
    }

    str[len++] = '\0';
    str = realloc(str, sizeof(char) * len);

    struct Vector final_input_string;
    final_input_string.length = len;
    final_input_string.str = str;

    return final_input_string;
}

void calc_prompt(void) {
    mvprintw(yInc, 1, "Enter operation : ");
    yInc += 1;
    refresh();

    struct Vector expr_raw = inputString(10);
    int expr_size = expr_raw.length;
    char *expr = expr_raw.str;

    int token_count = 0, len = 0, val1, val2, oper, result;
    char *current_str = realloc(NULL, 10 * sizeof(char));
    int current_str_len = 0;
    for (int i = 0; i < expr_size; i++) {
        char ch = expr[i];

        if (ch == ' ' || i == expr_size - 1) {
            switch (token_count) {
                case 0:
                    val1 = atoi(current_str);
                    break;
                case 1:
                    oper = current_str[0];
                    break;
                case 2:
                    val2 = atoi(current_str);
                    break;
            }

            current_str = realloc(NULL, 10 * sizeof(char));
            len = 0;
            token_count++;
            continue;
        }

        current_str[len] = ch;
        len++;
        if (len * sizeof(char) == sizeof(current_str)) {
            current_str = realloc(current_str, len * 2 * sizeof(char));
        }
    }

    if (token_count == 0) {
        mvprintw(yInc, 1, "ERROR: Not all operands specified\n");
        yInc += 2;
        refresh();
        return;
    }

    switch (oper) {
        case '+':
            result = val1 + val2;
            break;
        case '-':
            result = val1 - val2;
            break;
        case '/':
            result = val1 / val2;
            break;
        case '*':
            result = val1 * val2;
            break;
        default:
            mvprintw(yInc, 1, "ERROR: Operation not supported or invalid\n");
            yInc += 2;
            refresh();
            return;
            break;
    }

    mvprintw(yInc, 1, "%i\n", result);
    yInc += 2;
    refresh();
    free(expr);
    calc_prompt();
}

int main(void) {

    initscr();
    curs_set(0);

    start_color();
    
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 

    keypad(stdscr, TRUE);
    attron(COLOR_PAIR(1) | A_BOLD);
    clear();

    mvprintw(yInc, 0, "HamOS calculator");
    yInc += 1;
    calc_prompt();
}