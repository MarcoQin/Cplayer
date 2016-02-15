#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "list.h"
#include "utils.h"
#include "db.h"
#include "player_core.h"
#include "file_manager.h"

pid_t parent_pid, child_pid, wpid;
int status;

void sub_pro();

void quit(int signum)
{
    free_player();
    destory_menu();

    db_close();
    db_disable();

    endwin();
    exit(1);
}

void stopping(int signum)
{
    if (status == STOP)  /* stop */
    {
        quit(2);
    }
    int id = get_next_or_previous_song_id(NEXT);
    load_song(id);
    update_label_info(id);
    /* mvprintw(0, 0, "next id is: %d\n", id); */
    /* refresh(); */
    sub_pro();
}


void sub_pro()
{
    if (child_pid)
    {
        kill(child_pid, SIGKILL);
    }
    child_pid=fork();
    if (child_pid < 0)
        printf("error in fork!");
    else if (child_pid == 0) {
        int code = 0;
        parent_pid = getppid();
        /* mvwprintw(stdscr, 3, 0, "i am the child process, my process id is %d\n",getpid()); */
        /* mvwprintw(stdscr, 4, 0, "parent_pid %d\n", parent_pid); */
        /* mvwprintw(stdscr, 5, 0, "pid is %d\n", pid); */
        /* refresh(); */
        sleep(2);
        while(1)
        {
            code = kill(pid, 0);
            if (code == 0) {  // alive
                /* mvwprintw(stdscr, 1, 0, "code is: %d\n", code); */
                /* refresh(); */
                sleep(2);
                /* mvwprintw(stdscr, 1, 0, "alive is: %d\n", code); */
                /* refresh(); */
            } else {
                /* mvwprintw(stdscr, 1, 0, "will exit"); */
                /* refresh(); */
                kill(parent_pid, SIGUSR1);
                break;
            }
        }
        exit(1);
    }
    else {
        /* mvprintw(2, 0, "i am the parent process, my process id is %d\n",getpid()); */
        /* refresh(); */
        signal(SIGCHLD, SIG_IGN);
    }
}

int main()
{
    signal(SIGINT, quit);
    signal(SIGUSR1, stopping);
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

    /* char **choices = 0; */
    /* n_choices = loading_choices(&choices); */

    init_song_menu(choices, n_choices);
    init_label();


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
        case KEY_RIGHT:
        case 'l':
            seek("20");
            break;
        case KEY_LEFT:
        case 'h':
            seek("-20");
            break;
        case 'q':
            quit(2);
            break;
        case 32:  /* space key */
            pause_song();
            break;
        case 'a':
            filename = init_file_selector(stdscr);
            sleep(1);
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
        case 'n':
            id = get_next_or_previous_song_id(NEXT);
            load_song(id);
            handle_menu_scroll('j');
            update_label_info(id);
            break;
        case 'p':
            id = get_next_or_previous_song_id(PREVIOUS);
            load_song(id);
            handle_menu_scroll('k');
            update_label_info(id);
            break;
        case 10: /* Enter */
            move(0, 0);
            clrtoeol();
            id = get_current_selected_song_id();
            load_song(id);
            update_label_info(id);
            /* mvprintw(4, 0, "Id is: %d\n", id); */
            /* mvprintw(5, 0, "Path is: %s\n", get_song_path(id)); */
            refresh();
            pos_menu_cursor(my_menu);
            sub_pro();
            break;
        }
        wrefresh(my_menu_win);
    }


    free_items(n_choices);

    quit(2);

    return 0;
}
