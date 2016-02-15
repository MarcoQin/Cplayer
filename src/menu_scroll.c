#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include <locale.h>
#include "db.h"
#include "utils.h"
#include <cdk.h>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4

/* char *choices[] = { */
    /* "Choice 1", "Choice 2", "Choice 3", "Choice 4",  "Choice 5", "Choice 6", */
    /* "Choice 7", "Choice 8", "Choice 9", "Choice 10", "Exit",     (char *)NULL, */
/* }; */
void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color);
static char empty[1];

int main() {
    ITEM **my_items = 0;
    int c;
    MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;


    /* Initialize curses */
    /* setlocale(LC_ALL, ""); */
    setlocale(LC_CTYPE, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);

    /** db test **/
    db_enable();
    int rc = db_init("./songs.db");
    if (rc != 0) {
        fprintf(stderr, "Fail to init db.\n");
    }
    printf("Initialize database\n");
    char **result = 0;
    int j, nrow, ncol, index;
    char *errmsg;
    rc = db_load_songs(&result, &nrow, &ncol, &errmsg);
    char *choices[nrow];
    if (rc == 0) {
        index = ncol;
        for (i = 0; i < nrow; i++) {
            for (j = 0; j < ncol; j++) {
                if (j == 1)
                {
                    /* choices[i] = result[index]; */
                    char *r = malloc(1 + strlen(result[index-1])+ strlen(result[index]));
                    sprintf(r, "%s.%s", result[index-1], result[index]);
                    choices[i] = r;
                }
                index++;
            }
        }
    }

    /** end db test **/


    /* Create items */
    /* n_choices = ARRAY_SIZE(choices); */
    n_choices = nrow;
    printw("n_choices: %d", n_choices);
    /* my_items = (ITEM **)malloc(n_choices * sizeof(ITEM *)*2); */
    my_items = (ITEM **)calloc(n_choices+1, sizeof(ITEM *));
    /* ITEM *items[n_choices]; */
    /* ITEM **my_items = items; */
    for (i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], empty);
    my_items[i+1] = NULL;   /* #####THIS IS VERY IMPORTANT !!!!###### */

    /* Crate menu */
    my_menu = new_menu(my_items);
    /* my_menu = new_menu((ITEM **)my_items); */

    /* Create the window to be associated with the menu */
    my_menu_win = newwin(20, 80, 10, 20);

    /* WINDOW *label = newwin(0, 1, 5, 5); */
    WINDOW *label = subwin(stdscr, 5, 80, 2, 20);
    box(label, 0, 0);
    print_in_middle(label, 0, 0, 0, "hello", COLOR_PAIR(2));


    WINDOW *button = subwin(stdscr, 3, 10, 7, 20);
    box(button, 0, 0);
    print_in_middle(button, 1, 1, 5, "start", COLOR_PAIR(3));

    wrefresh(label);
    wrefresh(button);
    refresh();

    keypad(my_menu_win, TRUE);

    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    /* set_menu_sub(my_menu, derwin(my_menu_win, 6, 68, 3, 1)); */
    set_menu_sub(my_menu, derwin(my_menu_win, 15, 79, 3, 1));
    /* set_menu_format(my_menu, 5, 1); */
    set_menu_format(my_menu, 15, 1);

    /* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " * ");

    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
    print_in_middle(my_menu_win, 1, 0, 80, "Song List", COLOR_PAIR(2));
    /* mvwaddch(my_menu_win, 2, 0, ACS_LTEE); */
    /* mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38); */
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, 78);
    /* mvwaddch(my_menu_win, 2, 80, ACS_RTEE); */

    set_menu_fore(my_menu, COLOR_PAIR(2) | A_REVERSE);
    set_menu_back(my_menu, COLOR_PAIR(2));
    set_menu_grey(my_menu, COLOR_PAIR(3));

    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);

    attron(COLOR_PAIR(2));
    mvprintw(LINES - 2, 0,
             "Use PageUp and PageDown to scoll down or up a page of items");
    mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
    attroff(COLOR_PAIR(2));
    refresh();

    while ((c = wgetch(my_menu_win)) != KEY_F(2)) {
        switch (c) {
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(my_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(my_menu, REQ_SCR_UPAGE);
            break;
        case 10: /* Enter */
            move(0, 0);
            clrtoeol();
            /* mvprintw(0, 0, "Item selected is : %s", */
                    /* item_name(current_item(my_menu))); */
            /* mvprintw(2, 0, "%s\n", item_name(current_item(my_menu))); */
            const char *song_name = item_name(current_item(my_menu));
            int id = extract_song_id(song_name);
            /* mvprintw(4, 0, "Id is: %d\n", id); */
            /* mvprintw(5, 0, "Path is: %s\n", get_song_path(id)); */
            mvwprintw(label, 1, 30, "Id is: %d", id);
            mvwprintw(label, 2, 0, "Path is: %s\n", get_song_path(id));
            mvwprintw(label, 3, 0, "Name is: %s\n", song_name);
            wrefresh(label);
            refresh();
            pos_menu_cursor(my_menu);
            break;
        }
        wrefresh(my_menu_win);
    }

    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i)
        free_item(my_items[i]);

    sqlite3_free_table(result);
    sqlite3_free(errmsg);
    db_close();
    db_disable();



    endwin();
}

void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color) {
    int length, x, y;
    float temp;

    if (win == NULL)
        win = stdscr;
    getyx(win, y, x);
    if (startx != 0)
        x = startx;
    if (starty != 0)
        y = starty;
    if (width == 0)
        width = 80;

    length = strlen(string);
    temp = (width - length) / 2;
    x = startx + (int)temp;
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}
