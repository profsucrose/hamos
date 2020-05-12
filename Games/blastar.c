#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "hamos_app.h"

#include <errno.h>    

#define TEXT_COLOR 1
#define SHIP_COLOR 2
#define MISSLE_COLOR 3
#define DESTROYED_PLAYER_COLOR 3
#define STATUS_BEAM_COLOR 4

int score, 
    ships, 
    playerX, 
    playerY, 
    enemyX, 
    enemyY;
bool misslesFired = false,
    statusBeamFired = false,
    didShootEnemy,
    playerShot = false;

void draw_missles(int x, int y, char c) {
    mvaddch(y, x + 1, c);
    mvaddch(y, x - 1, c);
}

void draw_enemy(int x, int y, char c) {
    // Head of ship
    mvaddch(y, x, c);

    // Body 
    mvaddch(y + 1, x + 1, c);
    mvaddch(y + 1, x - 1, c);

    mvaddch(y, x + 2, c);
    mvaddch(y, x - 2, c);

    mvaddch(y + 2, x + 2, c);
    mvaddch(y + 2, x - 2, c);
}

void draw_enemy_shot(int x, int y, char c) {
    draw_enemy(x, y, ' ');
    mvaddch(y, x + 1, c);
    mvaddch(y, x - 1, c);
    mvaddch(y + 1, x, c);
    mvaddch(y - 1, x, c);
    mvaddch(y - 1, x - 1, c);
    mvaddch(y - 1, x + 1, c);
    mvaddch(y + 1, x - 1, c);
    mvaddch(y - 1, x + 1, c);
}

void draw_score() {
    char scoreString[50];
    sprintf(scoreString, "SCORE %i", score);
    mvprintw(1, 3, scoreString);
}

void draw_ships() {
    char shipsString[50];
    sprintf(shipsString, "SHIPS %i", ships);
    mvprintw(1, 20, shipsString);
}

void *shoot_missles(void *vargp) {
    misslesFired = true;
    int missleY = playerY - 1,
        missleX = playerX,
        missleXDistance;
    while (missleY > 3 
    && !(didShootEnemy = (missleXDistance = abs(missleX - enemyX)) < 4
    && missleY == enemyY + 3)) {
        usleep(100 * 1000);
        draw_missles(missleX, missleY, ' ');
        missleY -= 1;
        attron(COLOR_PAIR(MISSLE_COLOR));
        draw_missles(missleX, missleY, '*');
        refresh();
    }

    draw_missles(missleX, missleY, ' ');
    misslesFired = false;

    if (didShootEnemy) {
        attron(COLOR_PAIR(SHIP_COLOR));
        draw_enemy(enemyX, enemyY, ' ');
        draw_enemy_shot(enemyX, enemyY + 1, '*');
        refresh();
        usleep(2000 * 1000);
        draw_enemy_shot(enemyX, enemyY + 1, ' ');
        refresh();
        didShootEnemy = false;
        draw_enemy(enemyX, enemyY, ' ');
        enemyY += 4;
        score += 80;
        attron(COLOR_PAIR(TEXT_COLOR));
        draw_score();
        refresh();
    }
    
    return NULL;
}

void draw_player(int x, int y, char c) {
    // Head of ship
    mvaddch(y, x, c);

    // Body 
    mvaddch(y + 1, x + 1, c);
    mvaddch(y + 1, x - 1, c);

    // Side wings
    mvaddch(y + 1, x - 2, c);
    mvaddch(y + 1, x + 2, c);

    mvaddch(y, x - 2, c);
    mvaddch(y, x + 2, c);

    mvaddch(y + 2, x - 2, c);
    mvaddch(y + 2, x + 2, c);
}

void draw_player_shot(int x, int y, char c) {
    mvaddch(y, x - 1, c);
    mvaddch(y + 1, x - 1, c);
    mvaddch(y + 1, x, c);
    mvaddch(y + 2, x, c);
    mvaddch(y + 2, x - 1, c);
    mvaddch(y + 2, x + 1, c);
}

void draw_status_beam(int x, int y, char c) {
    mvaddch(y, x - 1, c);
    mvaddch(y, x + 1, c);
    mvaddch(y - 1, x, c);
    mvaddch(y + 1, x, c);
}

bool fire_status_beam() {
    int statusBeamY = enemyY + 4,
        statusBeamX = enemyX;
    statusBeamFired = true;
    while (statusBeamY < playerY) {
        usleep(100 * 1000);
        draw_status_beam(statusBeamX, statusBeamY, ' ');
        statusBeamY++;
        attron(COLOR_PAIR(STATUS_BEAM_COLOR));
        draw_status_beam(statusBeamX, statusBeamY, '*');
        refresh();
    }

    playerShot = true;
    draw_status_beam(statusBeamX, statusBeamY, ' ');
    attron(COLOR_PAIR(DESTROYED_PLAYER_COLOR));
    draw_player_shot(playerX, playerY, '*');
    refresh();

    usleep(2000 * 1000);
    draw_player_shot(playerX, playerY, ' ');

    ships--;
    attron(COLOR_PAIR(TEXT_COLOR));
    draw_ships();
    attron(COLOR_PAIR(SHIP_COLOR));
    draw_player_shot(playerX, playerY, ' ');
    draw_player(playerX, playerY, '|');
    statusBeamFired = false;
    playerShot = false;
    return ships == -1;
}

void *enemy_loop(void *vargp) {
    while (1) {
        usleep(100 * 1000);
        if (didShootEnemy) continue;

        if (rand() % 15 == 0 && abs(enemyX - playerX) < 4) {
            if (fire_status_beam()) break;
        }

        draw_enemy(enemyX, enemyY, ' ');
        
        if (enemyX + 3 >= COLS) {
            enemyX = 0;
        } else {
            enemyX += 1;
        }

        attron(COLOR_PAIR(SHIP_COLOR));
        draw_enemy(enemyX, enemyY, '*');
        refresh();
    }
    return NULL;
}

void *player_loop(void *vargp) {
    int ch;
    while (1) {
        ch = getch();
        if (playerShot) continue;
        draw_player(playerX, playerY, ' ');

        if (ch == KEY_LEFT && !statusBeamFired) 
            if (playerX - 2 > 0) playerX -= 2;

        if (ch == KEY_RIGHT && !statusBeamFired) 
            if (playerX + 2 > 0) playerX += 2;

        if (ch == KEY_UP) 
            if (playerY > 0) playerY--;

        if (ch == KEY_DOWN) 
            if (playerY < LINES - 3) playerY++;

        if (ch == SPACE && !misslesFired) {
            pthread_t missle_thread_id; 

            // Create enemy and input threads
            pthread_create(&missle_thread_id, NULL, shoot_missles, NULL);  
        }

        attron(COLOR_PAIR(SHIP_COLOR));
        draw_player(playerX, playerY, '|');
        refresh();
    }
    return NULL;
}

void start_new_game(void) {
    clear();
    playerX = COLS / 2;
    playerY = LINES / 2;
    enemyX = COLS / 2;
    enemyY = 5;
    ships = 5;
    score = 0;

    attron(COLOR_PAIR(SHIP_COLOR));
    draw_player(playerX, playerY, '|');

    draw_enemy(enemyX, enemyY, '*');

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(1, 3, "SCORE0");
    mvprintw(1, 20, "SHIPS 5");

    pthread_t player_loop_thread; 
    pthread_t enemy_loop_thread; 

    // Create enemy and input threads
    pthread_create(&player_loop_thread, NULL, player_loop, NULL);  
    pthread_create(&enemy_loop_thread, NULL, enemy_loop, NULL);  

    pthread_join(enemy_loop_thread, NULL);
    pthread_cancel(player_loop_thread);

    clear();

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(0, COLS / 2 - 3, "BLASTAR");
    mvprintw(2, COLS / 2 - 7, "FLEET DESTROYED");
    mvprintw(4, COLS / 2 - 17, "WOULD YOU LIKE ANOTHER GAME (Y/N)");

    char ch = getch();
    if (ch == 'y') {
        start_new_game();
    }

    clear();
    endwin();
}

int main(void) {
    initscr();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    attron(COLOR_PAIR(TEXT_COLOR) | A_BOLD);

    keypad(stdscr, TRUE);
    noecho();

    mvprintw(0, LINES - 7, "BLASTAR");
    refresh();
    usleep(2000 * 1000);

    clear();
    mvprintw(0, 3, "BY HAM");
    refresh();
    usleep(2000 * 1000);

    clear();
    mvprintw(5, COLS / 2 - 3, "BLASTAR");
    mvprintw(6, COLS / 2 - 8, "DO YOU NEED INSTRUCTIONS");
    mvprintw(7, COLS / 2 - 5, "(Y/N)");
    refresh();

    char ch = getch();
    clear();
    if (ch == 'n') goto startGame;

    mvprintw(1, 6, "USE ARROW KEYS");
    mvprintw(3, 6, "FOR CONTROL AND");
    mvprintw(5, 6, "SPACE BUTTON");
    mvprintw(7, 6, "TO SHOOT");

    mvprintw(10, 3, "MISSION:DESTROY ALIEN FREIGHTER");
    mvprintw(12, 3, "CARRYING DEADLY HYDROGEN BOMBS");
    mvprintw(14, 3, "AND STATUS BEAM MACHINES");
    
    getch();

    clear();
    startGame: start_new_game();
}