#ifndef _list_h_
#define _list_h_
#include <menu.h>
#include <ncurses.h>

extern WINDOW *my_menu_win;
extern MENU *my_menu;

void init_label();
void update_label_info(int id);
void init_song_menu(char **choices, int n_choices);
void destory_menu();
void free_items(int n_choices);
void handle_menu_scroll(int ch);
int get_current_selected_song_id();
const char *get_current_song_name();
#endif
