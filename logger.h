#define ENTER 10
#define SPACE 32

void set_background() {
    for (int y = 0; y < COLS; y++) {
            for (int x = 0; x < LINES; x++) 
                if (mvinch(x, y) == ' ') mvaddch(x, y, ' ');
    }
}

void logprintf(char* template, int context) {
    FILE *fp;

    fp = fopen("./output.log", "a+");
    fprintf(fp, template, context);
    fclose(fp);
}