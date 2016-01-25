/* $Id: fileview.c,v 1.11 2012/03/21 22:59:31 tom Exp $ */

#include "cdk_test.h"
#include <cdk.h>
#include "utils.h"
#include <fcntl.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "codeViewer";
#endif

/*
 * This program demonstrates the file selector and the viewer widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen  = 0;
   CDKVIEWER *example    = 0;
   CDKFSELECT *fSelect   = 0;
   WINDOW *cursesWin     = 0;
   const char *title     = "<C>Pick a file.";
   const char *label     = "File: ";
   const char *directory = ".";
   char *filename        = 0;
   char **info           = 0;
   const char *button[5];
   const char *mesg[4];
   char vtitle[256];
   char temp[256];
   int selected, lines, ret;

   /* Parse up the command line. */
   while (1)
   {
      ret = getopt (argc, argv, "d:f:");
      if (ret == -1)
      {
	 break;
      }
      switch (ret)
      {
      case 'd':
	 directory = strdup (optarg);
	 break;

      case 'f':
	 filename = strdup (optarg);
	 break;
      }
   }

   /* Create the viewer buttons. */
   button[0] = "</5><OK><!5>";
   button[1] = "</5><Cancel><!5>";

   /* Set up CDK. */
   cursesWin = initscr ();
   cdkscreen = initCDKScreen (cursesWin);

   /* Start color. */
   initCDKColor ();

   /* Get the filename. */
   if (filename == 0)
   {
      fSelect = newCDKFselect (cdkscreen, CENTER, CENTER, 20, 65,
			       title, label, A_NORMAL, '_', A_REVERSE,
			       "</5>", "</48>", "</N>", "</N>", TRUE, FALSE);

      /*
       * Set the starting directory. This is not neccessary because when
       * the file selector starts it uses the present directory as a default.
       */
      setCDKFselect (fSelect, directory, A_NORMAL, '.', A_REVERSE,
		     "</5>", "</48>", "</N>", "</N>", ObjOf (fSelect)->box);

      /* Activate the file selector. */
      filename = copyChar (activateCDKFselect (fSelect, 0));

      /* Check how the person exited from the widget. */
      if (fSelect->exitType == vESCAPE_HIT)
      {
        /* Pop up a message for the user. */
        mesg[0] = "<C>Escape hit. No file selected.";
        mesg[1] = "";
        mesg[2] = "<C>Press any key to continue.";
        popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);

        /* Destroy the file selector. */
        destroyCDKFselect (fSelect);

        /* Exit CDK. */
        destroyCDKScreen (cdkscreen);
        endCDK ();

        ExitProgram (EXIT_SUCCESS);
      }


   }

   /* Destroy the file selector. */
   destroyCDKFselect (fSelect);
    initscr();
    cbreak();
    noecho();
    printw("hahahahaha\n");
    char *f_name = parse_p(filename);
    char buf[512];
    FILE *pp;
    char *FIFO = "/tmp/my_fifo";
    mkfifo(FIFO, 0666);
    /* char *base = "mplayer -slave -input file=/tmp/my_fifo "; */
    char *base = "mplayer -slave -quiet -input file=/tmp/my_fifo ";
    /* char *base = "mplayer -input file=/tmp/my_fifo "; */
    char *tail = " < /dev/null 2>&1 &";
    /* char *tail = " "; */
    char *result = malloc(strlen(base)+strlen(f_name)+strlen(tail)+1);
    strcpy(result, base);
    strcat(result, f_name);
    strcat(result, tail);
    /* printw("filename: %s\n", filename); */
    /* printw("f_name: %s\n", f_name); */
    /* printw("filename: %s\n", f_name); */
    /* printw("result: %s\n", result); */
    free(f_name);
    pp = popen(result, "r");
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
    while((ch = getch()) != 'q')
    {
        while(fgets(buf, sizeof buf, pp))
            {
                printw("reading\n");
                    mvprintw(0, 0, "%s\n", buf);
            }
    }
    getch();

    free(result);
    pclose(pp);
    endwin();


   /* Clean up. */
   destroyCDKScreen (cdkscreen);
   CDKfreeStrings (info);
   freeChar (filename);

   endCDK ();

   ExitProgram (EXIT_SUCCESS);
}
