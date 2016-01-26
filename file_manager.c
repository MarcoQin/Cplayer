#include "file_manager.h"


char *init_file_selector(CDKSCREEN *cdkscreen)
{
    CDKFSELECT *fSelect = 0;
    const char *title = "<C>Pick a MUSIC file.";
    const char *label = "File: ";
    const char *directory = ".";
    char *filename = 0;

    /* Get the filename */
    fSelect = newCDKFselect(cdkscreen, CENTER, CENTER, 20, 65,
            title, label, A_NORMAL, '_', A_REVERSE,
            "</5>", "</48>", "</N>", "</N>", TRUE, FALSE);
    /*
     * Set the starting directory. This is not neccessary because when
     * the file selector starts it uses the present directory as a default.
     */
    setCDKFselect(fSelect, directory, A_NORMAL, '.', A_REVERSE,
            "</5>", "</48>", "</N>", "</N>", ObjOf (fSelect)->box);

    /* Activate the file selector. */
    filename = copyChar(activateCDKFselect(fSelect, 0));

    /* Clean up. */
    destroyCDKFselect(fSelect);

    return filename;
}
