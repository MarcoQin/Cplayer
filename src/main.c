#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "ui.h"
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
        exit(1);
    else if (child_pid == 0) {
        // child process
        int code = 0;
        parent_pid = getppid();
        sleep(2);
        while(1)
        {
            code = kill(pid, 0);
            if (code == 0) {  // alive
                sleep(2);
            } else {
                kill(parent_pid, SIGUSR1);  // custom sinal
                break;
            }
        }
        exit(1);
    }
    else {
        // parent process
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

    /** db init**/
    db_enable();

    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    char *path = merge_str((char *)homedir, "/.cplayer/", "songs.db");
    char *main_path = merge_str((char *)homedir, "/.cplayer", "/");

    if( access( path, F_OK ) == -1 ) {
        // file doesn't exist, create one
        status = mkdir(main_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    int rc = db_init(path);
    free(path);
    free(main_path);
    if (rc != 0) {
        fprintf(stderr, "Fail to init db.\n");
        exit(1);
    }

    char *choices[512];
    n_choices = loading_choices(choices);
    /** db end **/

    /** init UI **/
    init_song_menu(choices, n_choices);
    init_label();
    /** end UI **/


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
        case 's':
            stop_song();
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
                song_name = extract_file_name(filename);
                db_insert_song(song_name, filename);
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
