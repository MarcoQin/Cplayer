#include <cdk.h>
#include <fcntl.h>

#include "file_manager.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    CDKSCREEN *cdkscreen = 0;
    WINDOW *cursesWin = 0;
    /* Set up CDK. */
    cursesWin = initscr();
    cdkscreen = initCDKScreen(cursesWin);
    initCDKColor();

    char *filename = 0;
    filename = init_file_selector(cdkscreen);
    if(filename != 0)
    {
        /* initscr(); */
        cbreak();
        noecho();
        printw("hahahahaha\n");
        char *f_name = parse_p(filename);
        char buf[512];
        FILE *pp;
        char *FIFO = "/tmp/my_fifo";
        mkfifo(FIFO, 0666);
        const char *base = "mplayer -slave -quiet -input file=/tmp/my_fifo ";
        const char *tail = " < /dev/null 2>&1 &";
        char *result = malloc(strlen(base)+strlen(f_name)+strlen(tail)+1);
        strcpy(result, base);
        strcat(result, f_name);
        strcat(result, tail);
        free(f_name);
        pp = popen(result, "r");
        free(result);
        int d = fileno(pp);
        int flags = fcntl(d, F_GETFL, 0);
        fcntl(d, F_SETFL, O_NONBLOCK);
        if( pp == NULL )
        {
            printw("popen() error!/n");
            exit(1);
        }
        printw("popen success\n");
        int ch;
        sleep(1);
        while(fgets(buf, sizeof buf, pp))
        {
            printw("reading\n");
            mvprintw(0, 0, "%s\n", buf);
        }
        while((ch = getch()) != 'q')
        {
            while(fgets(buf, sizeof buf, pp))
            {
                printw("reading\n");
                mvprintw(0, 0, "%s\n", buf);
            }
        }
        getch();
        pclose(pp);
    }

    /* Clean up. */
    destroyCDKScreen (cdkscreen);
    endwin();
    endCDK ();
    freeChar(filename);
    return 0;
}
