#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "utils.h"
#include "db.h"
#include "player_core.h"
#include "file_manager.h"

int main()
{
    int n_choices, id;
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /** db test **/
    db_enable();
    int rc = db_init("./songs.db");
    if (rc != 0) {
        fprintf(stderr, "Fail to init db.\n");
    }

    char *choices[512];
    n_choices = loading_choices(choices);

    init_song_menu(choices, n_choices);


    int c;
    char *filename = 0;
    char *song_name = 0;
    while ((c = wgetch(my_menu_win)) != KEY_F(2)) {
        switch (c) {
        case KEY_DOWN:
        case 'j':
        case KEY_UP:
        case 'k':
        case KEY_NPAGE:
        case KEY_PPAGE:
            handle_menu_scroll(c);
            break;
        case 'q':
            stop_song();
            break;
        case 'p':
            pause_song();
            break;
        case 'a':
            filename = init_file_selector(stdscr);
            destroyCDKFselect(fSelect);
            destroyCDKScreen(cdkscreen);
            if (filename != NULL)
            {
                /* clear(); */
                song_name = extract_file_name(filename);
                db_insert_song(song_name, filename);
                /* free(song_name); */
                mvprintw(2, 0, "%s\n", filename);
                refresh();
                /* free_items(n_choices); */
                destory_menu();
                n_choices = loading_choices(choices);
                init_song_menu(choices, n_choices);
            }
            break;
        case 'd':
            id = get_current_selected_song_id();
            free_items(n_choices);
            destory_menu();
            db_delete_song(id);
            n_choices = loading_choices(choices);
            init_song_menu(choices, n_choices);
            break;
        case 10: /* Enter */
            move(0, 0);
            clrtoeol();
            id = get_current_selected_song_id();
            load_song(get_song_path(id));
            mvprintw(4, 0, "Id is: %d\n", id);
            mvprintw(5, 0, "Path is: %s\n", get_song_path(id));
            refresh();
            pos_menu_cursor(my_menu);
            break;
        }
        wrefresh(my_menu_win);
    }

    free_player();
    destory_menu();

    free_items(n_choices);
    db_close();
    db_disable();

    endwin();

    return 0;
}
