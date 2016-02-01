#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "db.h"

int main()
{
    int n_choices, i;
    setlocale(LC_ALL, "");
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
    n_choices = nrow;

    init_song_menu(choices, n_choices);

    int c;
    while ((c = wgetch(my_menu_win)) != KEY_F(2)) {
        switch (c) {
        case KEY_DOWN:
        case KEY_UP:
        case KEY_NPAGE:
        case KEY_PPAGE:
            handle_menu_scroll(c);
            break;
        case 10: /* Enter */
            move(0, 0);
            clrtoeol();
            int id = get_current_selected_song_id();
            mvprintw(4, 0, "Id is: %d\n", id);
            mvprintw(5, 0, "Path is: %s\n", get_song_path(id));
            refresh();
            pos_menu_cursor(my_menu);
            break;
        }
        wrefresh(my_menu_win);
    }

    destory_menu();
    free_items(n_choices);

    sqlite3_free_table(result);
    sqlite3_free(errmsg);
    db_close();
    db_disable();

    endwin();

    return 0;
}
