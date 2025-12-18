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

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = malloc(size + 1);
    if(!content){
        fclose(fp);
        return NULL;
    }
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    return content;
}

void draw_text(WINDOW* win, const char* text, const char* typed, int pos, int* corr, int* incorr)
{
    int len = strlen(text);
    // int y,x;
    // getyx(win, y, x);
    // int start_y = y;
    // int start_x = x;

    *corr = 0;
    *incorr = 0;

    for (int i=0;i< len;i++)
    {
        if(i<pos){
            //Already typed
            if((typed[i]==text[i]))
            {
                (*corr)++;
                wattron(win, COLOR_PAIR(PAIR_WHITE));
            }
            else{
                (*incorr)++;
                wattron(win, COLOR_PAIR(PAIR_RED));
            }
            waddch(win, text[i]);
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
    int correct = 0;
    int incorrect = 0;
    bool timed_mode = false;
    double time_limit = 60.000;
    struct timespec start, finish, delta, now;


    // Initialize ncurses
    initscr();
    //Check if Terminal supports color
    if (has_colors() == FALSE) {
        free(text);
        free(typed);
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
    curs_set(0);
    refresh();
    int ch = getch();
    if(ch == '\n' || ch == '\r' || ch == KEY_ENTER) // Enter
    {
        // Timed Mode
        mvprintw(4,0, "Timed Mode or No? Y/N\nDefault is non-timed");
        refresh();
        int chres = getch();
        if (chres == 'Y' || chres == 'y'){
            timed_mode = true;
            refresh();}

            if(timed_mode){
                clear();
                // Draw initial text timed mode.
                mvprintw(0,0, "Enter the number of seconds you wanna test for\nEnter a number between 0 and 600\nDefault is 60s");
                refresh();
                echo();        // show typed characters
                curs_set(1);
                char buf[8] = {0};
                mvgetnstr(3, 0, buf, sizeof(buf)-1);

                noecho();      // go back to hidden typing
                curs_set(0);

                if (buf[0] != '\0') {
                    double val = atof(buf);
                    if (val > 0.0 && val <= 600.0) {
                        time_limit = val;
                    }
                }

                clear();
                mvprintw(0, 0, "Timed test: %6.3f seconds", time_limit);
                move(2,0);
                refresh();
                draw_text(stdscr, text, typed, pos, &correct, &incorrect);
            }
            else{
                refresh();
                // Draw initial text Non timed mode.
                mvprintw(1,0, "Right, Starting in a second...\n");
                refresh();
                clear();
                draw_text(stdscr, text, typed, pos, &correct, &incorrect);
                move(0,0);
                refresh();
            }
    }
    else{
        free(text);
        free(typed);
        endwin();
        exit(1);
    }
    // ***********************TYPING TEST BEGINS*********************************
    timeout(50); //to have a proper timer without waiting for user input
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while (pos < len) {
        if(timed_mode){
            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            sub_timespec(start, now, &delta);
            double elapsed = delta.tv_sec + delta.tv_nsec / 1e9;
            double time_left = time_limit - elapsed;
            if(time_left < 0){
                time_left = 0;
            }
            if(time_left == 0)
            {
                break;
            }
            char timer_str[32];
            snprintf(timer_str, sizeof(timer_str), "Time: %6.3fs", time_left);

            int x = (COLS - (int)strlen(timer_str)) / 2;
            if (x < 0) x = 0;

            move(0, 0);
            clrtoeol();             // clear the whole top line
            attron(COLOR_PAIR(PAIR_GREEN));
            mvprintw(0, x, "%s", timer_str);
            attroff(COLOR_PAIR(PAIR_GREEN));
        }
        ch = getch();

        if(ch == ERR){
            continue;
        }
        if (ch == 27) {  // ESC key
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            // Handle backspace
            if (pos > 0) {
                pos--;
                typed[pos] = '\0';
            }
        } else if (ch >= 32 && ch <= 126) {  // Printable characters
            typed[pos] = ch;
            pos++;
        }

        // Redraw text
        if (timed_mode) move(2, 0);
        else move(0,0);
        clrtoeol();
        draw_text(stdscr, text, typed, pos, &correct, &incorrect);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
    sub_timespec(start, finish, &delta);

    // Cleanup
    endwin();
    free(text);
    free(typed);

    //  Stats
    printf("Total characters: %d\n", (correct + incorrect));
    printf("Correct characters: %d\n", correct);
    printf("Incorrect characters: %d\n", incorrect);
    printf("Typing accuracy: %.2f%%\n", (correct * 100.0) / (correct + incorrect));
    printf("Time Taken: %.3f s\n", (double)delta.tv_sec + (double)delta.tv_nsec / 1e9);
    return 0;
}
