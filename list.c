#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "list.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
WINDOW *my_menu_win;
ITEM **my_items = 0;
MENU *my_menu;

void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color);
static char empty[1];


void init_song_menu(char **choices, int n_choices)
{
    int i;
    my_items = (ITEM **)calloc(n_choices+1, sizeof(ITEM *));
    for (i = 0; i < n_choices; ++i)
        my_items[i] = new_item(choices[i], empty);
    my_items[i+1] = NULL;   /* #####THIS IS VERY IMPORTANT !!!!###### */
    my_menu = new_menu(my_items);

    /* Create the window to be associated with the menu */
    my_menu_win = newwin(20, 80, 10, 20);
    keypad(my_menu_win, TRUE);

    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 15, 79, 3, 1));
    set_menu_format(my_menu, 15, 1);

    /* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " * ");

    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
    print_in_middle(my_menu_win, 1, 0, 80, "Song List", COLOR_PAIR(2));

    mvwhline(my_menu_win, 2, 1, ACS_HLINE, 78);

    set_menu_fore(my_menu, COLOR_PAIR(2) | A_REVERSE);
    set_menu_back(my_menu, COLOR_PAIR(2));
    set_menu_grey(my_menu, COLOR_PAIR(3));

    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);
}

void destory_menu()
{
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
}

void free_items(int n_choices)
{
    int i;
    for (i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
}

void handle_menu_scroll(int ch)
{
    switch (ch) {
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
    }
}

int get_current_selected_song_id()
{
    int id = extract_song_id(item_name(current_item(my_menu)));
    return id;
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
