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

#define PLAYING 1
#define STOP 0
#define PAUSE 2
#define ALIVE 1

int status = 0;  /* play status */
int alive = 0; /* pipe alive false */
FILE *pp;
char buf[512];
char *FIFO = "/tmp/my_fifo";
int fd;

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
    /* char *result = malloc(strlen(base) + strlen(path) + strlen(tail) + 1); */
    /* strcpy(result, base); */
    /* strcat(result, path); */
    /* strcat(result, tail); */
    pp = popen(result, "r");
    int d = fileno(pp);
    int flags = fcntl(d, F_GETFL, 0);
    fcntl(d, F_SETFL, flags | O_NONBLOCK);
    if (pp == NULL) {
        printf("popen() error!/n");
        exit(1);
    }
    status = PLAYING;  /* playing status */
    alive = ALIVE;
    free(result);
}

void load_song(char *path)
{
    mvprintw(0, 1, "pp is %d\n", pp);
    refresh();
    if (alive == ALIVE && pp != NULL)
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
    mvprintw(0, 20, "pp is %d\n", pp);
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
    if (status != STOP)
        stop_song();
    status = STOP;
    alive = 0;
    pclose(pp);
}
