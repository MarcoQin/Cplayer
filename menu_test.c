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

void sighandler(int signum)
{
   printf("Caught signal %d, coming out...\n", signum);
    free_player();
    destory_menu();

    db_close();
    db_disable();

    endwin();
   exit(1);
}

void sub_pro()
{
    child_pid=fork();
    if (child_pid < 0)
        printf("error in fork!");
    else if (child_pid == 0) {
        int code = 0;
        parent_pid = getppid();
        mvprintw(3, 0, "i am the child process, my process id is %d\n",getpid());
        mvprintw(4, 0, "parent_pid %d\n", parent_pid);
        mvprintw(5, 0, "pid is %d\n", pid);
        refresh();
        sleep(2);
        while(1)
        {
            code = kill(pid, 0);
            if (code == 0) {  // alive
                mvprintw(1, 0, "code is: %d\n", code);
                refresh();
                sleep(2);
                mvprintw(1, 0, "alive is: %d\n", code);
                refresh();
            } else {
                mvprintw(1, 0, "will exit");
                refresh();
                /* kill(parent_pid, -2); */
                kill(parent_pid, SIGUSR1);
                break;
            }
        }
        exit(1);
    }
    else {
        mvprintw(2, 0, "i am the parent process, my process id is %d\n",getpid());
        refresh();
        /* waitpid(child_pid, NULL, WNOHANG); */
        signal(SIGCHLD, SIG_IGN);
        /* struct sigaction sigchld_action = { */
          /* .sa_handler = SIG_DFL, */
          /* .sa_flags = SA_NOCLDWAIT */
        /* }; */
        /* sigaction(SIGCHLD, &sigchld_action, NULL); */

        /* while here test */
        /* wpid = wait(&status); */
    }
}

int main()
{
    signal(SIGINT, sighandler);
    signal(SIGUSR1, sighandler);
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
            /* mvprintw(4, 0, "Id is: %d\n", id); */
            /* mvprintw(5, 0, "Path is: %s\n", get_song_path(id)); */
            refresh();
            pos_menu_cursor(my_menu);
            sub_pro();
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
