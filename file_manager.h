#ifndef _file_manager_h_
#define _file_manager_h_
#include <cdk.h>

extern CDKSCREEN *cdkscreen;
extern CDKFSELECT *fSelect;
char *init_file_selector(WINDOW *main_win);
#endif
