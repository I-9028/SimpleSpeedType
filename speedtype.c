#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define COLOR_GREY 8
#define PAIR_GREEN 2
#define PAIR_GREY 16
#define PAIR_RED 4
#define PAIR_WHITE 7

char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    return content;
}

void draw_text(WINDOW* win, const char* text, const char* typed, int pos, int corr, int incorr)
{
    int len = strlen(text);
    int y,x;
    getyx(win, y, x);
    int start_y = y;
    int start_x = x;

    for (int i=0;i< len;i++)
    {
        if(i<pos){
            //Already typed
            if((typed[i]==text[i]))
            {
                corr++;
                wattron(win, COLOR_PAIR(PAIR_WHITE));
            }
            else{
                incorr++;
                wattron(win, COLOR_PAIR(PAIR_RED));
            }
        }
        else if(i==pos) //current letter
        {
            wattron(win, COLOR_PAIR(PAIR_GREEN));
            waddch(win, text[i]);
        }
        else //yet to type
        {
            wattron(win, COLOR_PAIR(PAIR_GREY));
            waddch(win, text[i]);
        }
    }
    wrefresh(win);
}

int main(int argc, char *argv[])
{
    char* text = read_file("data.txt");
    if (!text) {
        fprintf(stderr, "Error: Could not read data.txt\n");
        return 1;
    }

    int len = strlen(text);
    if(text[len-1]=='\n'){
        text[len-1]='\0';
        len--;
    }

    char* typed = calloc(len + 1, sizeof(char));
    int pos = 0;


    // Initialize ncurses
    initscr();
    //Check if Terminal supports color
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    start_color();
    //char *typed = calloc(len+1, sizeof(char));
    init_color(8, 500, 500, 500);
    // 8 from history, for Grey/Bright Black

    init_pair(PAIR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(PAIR_GREY, COLOR_GREY, COLOR_BLACK);
    init_pair(PAIR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(PAIR_WHITE, COLOR_WHITE, COLOR_BLACK);

    // Let the Games Begin
    clear();
    mvprintw(0,0, "Welcome to this Typing Test.\n");
    mvprintw(1,0, "Press Enter to Begin\n");
    mvprintw(2,0, "Press Esc to exit.");
    move(3,0);
    curs_set(2);
    refresh();
    int input = getch();
    if( input == 27){ //ESC KEY
        endwin();
        exit(1);
    }
    else if (input == '\n' || input == '\r' || input == KEY_ENTER) // Enter
    {
        // Draw initial text
        mvprintw(4,0, "Right, Starting in a second...\n");
        refresh();
        clear();
        draw_text(stdscr, text, typed, pos,0,0);
        move(0,0);
        refresh();
        getch();

    }
    else{
        mvprintw(4,0, "Wrong Key! Exiting in a second...\n");
        refresh();
        sleep(1);
        endwin();
        exit(1);
    }

    // Cleanup
    endwin();
    free(text);
    free(typed);
    return 0;
}
