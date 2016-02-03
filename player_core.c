#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "player_core.h"
#include "utils.h"
#include "db.h"
#include <cdk.h>
#include "popen2.h"
#include <signal.h>


int status = 0;  /* play status */
int alive = KILL; /* pipe alive false */
char buf[512];
char *FIFO = "/tmp/my_fifo";
int fd;
pid_t pid;
int infp, outfp;


void init_player(char *path)
{
    mkfifo(FIFO, 0666);
    char *base = "mplayer -slave -quiet -input file=/tmp/my_fifo \"";
    char *tail = "\" < /dev/null 2>&1 &";
    char *result = merge_str(base, path, tail);
    pid = popen2(result, &infp, &outfp);
    /* mvprintw(0, 0, "old pid: %d \n", pid); */
    pid += 2;
    /* mvprintw(0, 20, "pid is %d\n", pid); */
    refresh();
    status = PLAYING;  /* playing status */
    alive = ALIVE;
    free(result);
}

int is_alive(void)
{
    int code = kill(pid, 0);
    /* mvprintw(1, 0, "code: %d", code); */
    return code;
}

void load_song(int id)
{
    char *path = get_song_path(id);
    db_update_song_state(PLAYING, id);
    if (alive == ALIVE && is_alive() == ALIVE)
    {
        char *base = "loadfile \"";
        char *tail = "\"\n";
        char *s = merge_str(base, path, tail);
        fd = open(FIFO, O_WRONLY);
        write(fd, s, strlen(s));
        close(fd);
        free(s);
        status = PLAYING;  /* playing status */
    } else {
        init_player(path);
    }
    /* mvprintw(0, 20, "pid is %d\n", pid); */
    refresh();
}

void stop_song()
{
    if (alive == ALIVE && is_alive() == ALIVE)
    {
        char *s = "stop\n";
        fd = open(FIFO, O_WRONLY);
        write(fd, s, strlen(s));
        close(fd);
    }
    status = STOP;  /* playing status */
}

void pause_song()
{
    if (alive == ALIVE && is_alive() == ALIVE)
    {
        char *s = "pause\n";
        fd = open(FIFO, O_WRONLY);
        write(fd, s, strlen(s));
        close(fd);
        status = status == PAUSE ? PLAYING : PAUSE;  /* playing status */
    }
}

void seek(char *seconds)
{
    if (alive == ALIVE && is_alive() == ALIVE && status == PLAYING)
    {
        char *base = "seek ";
        char *tail = "\n";
        char *s = merge_str(base, seconds, tail);
        fd = open(FIFO, O_WRONLY);
        write(fd, s, strlen(s));
        close(fd);
        free(s);
    }
}

void free_player()
{
    if (status != STOP && is_alive() == ALIVE)
        stop_song();
    status = STOP;
    alive = KILL;
    pclose2(pid);
}
