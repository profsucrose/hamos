#include <stdio.h>
#include <stdlib.h>

struct Vector {
    int length;
    char *str;
} null_vector;

struct Vector inputString(size_t initial_size) {
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char) * initial_size);

    if (!str) return null_vector;
    while (EOF != (ch = fgetc(stdin)) && ch != '\n') {
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

int main(void) {
    printf("HamOS calculator, enter operation : ");

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
        printf("ERROR: Not all operands specified\n");
        return -1;
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
            printf("ERROR: Operation not supported or invalid\n");
            return -1;
            break;
    }

    printf("%i\n", result);
    free(expr);
}