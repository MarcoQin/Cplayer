#ifndef _db_h_
#define _db_h_
#define NEXT 1
#define PREVIOUS 0

void db_enable();
void db_disable();
int get_db_enabled();
int db_init(const char *path);
void db_close();
void db_insert_song(char *name, char *path);
void db_update_song_state(int is_playing, int id);
void db_delete_song(int id);
int db_load_songs(char ***result, int *nrow, int *ncolumn, char **pzErrmsg);
char *get_song_path(int id);
char *get_song_name(int id);
int loading_choices(char ***choices);
int get_next_or_previous_song_id(int type);

#endif
