#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "player_core.h"
#include "utils.h"
#include <cdk.h>
#include "popen2.h"
#include <signal.h>

#define PLAYING 1
#define STOP 0
#define PAUSE 2
#define ALIVE 0
#define KILL 1

int status = 0;  /* play status */
int alive = KILL; /* pipe alive false */
char buf[512];
char *FIFO = "/tmp/my_fifo";
int fd;
pid_t pid;
int infp, outfp;

char *merge_str(char *base, char *middle, char *tail)
{
    char *result = (char *)malloc(strlen(base) + strlen(middle) + strlen(tail) + 1);
    strcpy(result, base);
    strcat(result, middle);
    strcat(result, tail);
    return result;
}

void init_player(char *path)
{
    mkfifo(FIFO, 0666);
    char *base = "mplayer -slave -quiet -input file=/tmp/my_fifo \"";
    char *tail = "\" < /dev/null 2>&1 &";
    char *result = merge_str(base, path, tail);
    pid = popen2(result, &infp, &outfp);
    mvprintw(0, 0, "old pid: %d \n", pid);
    pid += 2;
    mvprintw(0, 20, "pid is %d\n", pid);
    refresh();
    status = PLAYING;  /* playing status */
    alive = ALIVE;
    free(result);
}

int is_alive(void)
{
    int code = kill(pid, 0);
    mvprintw(1, 0, "code: %d", code);
    return code;
}

void load_song(char *path)
{
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
    mvprintw(0, 20, "pid is %d\n", pid);
    refresh();
}

void stop_song()
{
    if (alive == ALIVE)
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
    if (alive == ALIVE)
    {
        char *s = "pause\n";
        fd = open(FIFO, O_WRONLY);
        write(fd, s, strlen(s));
        close(fd);
        status = status == PAUSE ? PLAYING : PAUSE;  /* playing status */
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
