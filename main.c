#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
    int ch;
    initscr();
    clear();
    noecho();
    cbreak();
    printw("Now will play a.wav, press 'q' to stop and leave\n");
    char buf[128];
    FILE *pp;
    char *FIFO = "/tmp/my_fifo";
    mkfifo(FIFO, 0666);

    if ((pp = popen("mplayer -slave -quiet -input file=/tmp/my_fifo a.wav",
                    "r")) == NULL) {
        printf("popen() error!/n");
        exit(1);
    }

    /* int i = 0; */

    /* while(fgets(buf, sizeof buf, pp)) */
    /* { */
    /* printf("%s", buf); */
    /* if(i > 2) */
    /* break; */
    /* i++; */
    /* } */
    int fd;
    char *s = "stop\n";
    fd = open(FIFO, O_WRONLY);
    /* while(fgets(buf, sizeof buf, pp)) */
    /* { */
    /* printf("%s", buf); */
    /* } */
    while ((ch = getch()) != 'q') {

        switch (ch) {
        case 'p':
            s = "pause\n";
            write(fd, s, strlen(s));
        }

        /* char *s = "pause\n"; */
        /* s = "get_time_length\n"; */
        /* int fd = open(FIFO,O_WRONLY); */
        /* write(fd,s,strlen(s)); */
        /* sleep(10); */
        /* s = "stop\n"; */
        /* write(fd,s,strlen(s)); */
        /* close(fd); */
        /* while(fgets(buf, sizeof buf, pp)) */
        /* { */
        /* printf("%s", buf); */
        /* } */
        /* pclose(pp); */
    }
    s = "stop\n";
    write(fd, s, strlen(s));
    close(fd);
    pclose(pp);
    endwin();
    return 0;
}
