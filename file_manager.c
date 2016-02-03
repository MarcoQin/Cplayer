#include "file_manager.h"

CDKSCREEN *cdkscreen = 0;
CDKFSELECT *fSelect = 0;

char *init_file_selector(WINDOW *main_win) {
    cdkscreen = initCDKScreen(main_win);
    initCDKColor();
    const char *title = "<C>Pick a MUSIC file.";
    const char *label = "File: ";
    const char *directory = ".";
    char *filename = 0;

    /* Get the filename */
    fSelect = newCDKFselect(cdkscreen, CENTER, CENTER, 20, 65, title, label,
                            A_NORMAL, '_', A_REVERSE, "</5>", "</48>", "</N>",
                            "</N>", TRUE, FALSE);

    /* Activate the file selector. */
    filename = copyChar(activateCDKFselect(fSelect, 0));

    /* Check how the person exited from the widget. */
    if (fSelect->exitType == vESCAPE_HIT) {
        destroyCDKFselect(fSelect);
        return filename;
    }

    /* Clean up. */
    /* destroyCDKFselect(fSelect); */
    /* destroyCDKScreen(cdkscreen); */

    return filename;
}
