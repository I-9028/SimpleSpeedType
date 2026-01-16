#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#define CLOCK_MONOTONIC_RAW 0
#else
    #include <ncurses.h>
    #include <unistd.h>
#endif

#define COLOR_GREY 8
#define PAIR_GREEN 2
#define PAIR_GREY 16
#define PAIR_RED 4
#define PAIR_WHITE 7

enum { NS_PER_SECOND = 1000000000 };

#ifdef _WIN32
// Windows console color codes
#define WIN_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define WIN_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define WIN_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define WIN_GREY (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

HANDLE hConsole;
CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif


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

#ifdef _WIN32
int clock_gettime(int dummy, struct timespec *spec) {
    __int64 wintime;
    GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -= 116444736000000000LL;  // Unix epoch adjustment
    spec->tv_sec = wintime / 10000000LL;
    spec->tv_nsec = (wintime % 10000000LL) * 100;
    return 0;
}
#endif

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

#ifdef _WIN32
void clear_screen(){
    system("cls");
}

void move_cursor(int x, int y){
    COORD coord={x, y};
    SetConsoleCursorPosition(hConsole, coord);
}

void set_color(int color){
    SetConsoleTextAttribute(hConsole, color);
}

void show_cursor(bool visible){
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = visible;
    SetConsoleCursorInfo(hConsole, &info);
}

void draw_text_windows(const char* text, const char* typed, int pos, int* corr, int* incorr, int start_y) //start-y to denote where to start the text print, 0 if untimed, 2 if timed
{
    int len = strlen(text);
    *corr = 0;
    *incorr = 0;

    move_cursor(0, start_y);


    for (int i=0;i< len;i++)
    {
        if(i<pos){
            //Already typed
            if((typed[i]==text[i]))
            {
                (*corr)++;
                set_color(WIN_WHITE);
            }
            else{
                (*incorr)++;
                set_color(WIN_RED);
            }
            printf("%c", text[i]);
        }
        else if(i==pos) //current letter
        {
            set_color(WIN_GREEN);
            printf("%c", text[i]);
        }
        else //yet to type
        {
            set_color(WIN_GREY);
            printf("%c", text[i]);
        }
    }
    fflush(stdout);

}
#endif

#ifndef _WIN32
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
#endif

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

    #ifdef _WIN32
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
    #else
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
    #endif

    // Let the Games Begin

#ifdef _WIN32
    clear_screen();
    printf("Welcome to this Typing Test.\n");
    printf("Press Enter to Begin\n");
    printf("Press Esc to exit.");
    show_cursor(false);

    while(1){
        if(_kbhit()){
            int ch = _getch();
            if (ch == '\r' || ch == '\n') break;
            if (ch== 27){
                show_cursor(true);
                free(text);
                free(typed);
                return 0;
            }
        }
        Sleep(50);
    }
    // Windows timed mode prompt
    clear_screen();
    printf("Timed Mode or No? Y/N\nDefault is non-timed\n");

    while (1) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'Y' || ch == 'y') {
                timed_mode = true;
                break;
            } else if (ch == 'N' || ch == 'n' || ch == '\r' || ch == '\n') {
                timed_mode = false;
                break;
            }
        }
        Sleep(50);
    }

    if (timed_mode) {
        clear_screen();
        printf("Enter the number of seconds you wanna test for\n");
        printf("Enter a number between 0 and 600\n");
        printf("Default is 60s\n");
        show_cursor(true);

        char buf[8] = {0};
        fgets(buf, sizeof(buf), stdin);

        if (buf[0] != '\0' && buf[0] != '\n') {
            double val = atof(buf);
            if (val > 0.0 && val <= 600.0) {
                time_limit = val;
            }
        }
        show_cursor(false);

        clear_screen();
        printf("Timed test: %.3f seconds\n\n", time_limit);
        draw_text_windows(text, typed, pos, &correct, &incorrect, 2);
    } else {
        clear_screen();
        draw_text_windows(text, typed, pos, &correct, &incorrect, 0);
    }
#else
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
            refresh();

        }

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
#endif
    // ***********************TYPING TEST BEGINS*********************************

#ifdef _WIN32
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while (pos < len){
        if(timed_mode){
            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            sub_timespec(start, now, &delta);
            double elapsed = delta.tv_sec + delta.tv_nsec / 1e9;
            double time_left = time_limit - elapsed;
            if (time_left < 0) time_left = 0;
            if (time_left == 0) break;

            move_cursor(0,0);
            set_color(WIN_GREEN);
            printf("Time: %6.3fs", time_left);
            for (int i = 0; i < 50; i++) printf(" ");
            set_color(WIN_WHITE);
        }

        int ch = -1;

        if(_kbhit()){
            ch = _getch();
            if (ch == 0 || ch == 224){
                _getch();
                ch = -1;
            }
        }

        if(ch==-1)
        {
            Sleep(50);
            continue;
        }

        if(ch == 27){
            break;
        }
        else if(ch == '\b' || ch == 127){//backspace or DEL
            if (pos > 0) {
                pos--;
                typed[pos] = '\0';
            }
        } else if (ch >= 32 && ch <= 126) {  // Printable characters
            typed[pos] = ch;
            pos++;
        }
        draw_text_windows(text, typed, pos, &correct, &incorrect, timed_mode ? 2 : 0);
    }

#else
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
#endif
    clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
    sub_timespec(start, finish, &delta);

    // Cleanup
#ifdef _WIN32
    set_color(WIN_WHITE);
    show_cursor(true);
#else
    endwin();
#endif
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
