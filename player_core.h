#ifndef _player_core_h_
#define _player_core_h_
#define PLAYING 1
#define STOP 0
#define PAUSE 2
#define ALIVE 0
#define KILL 1
extern int pid;
extern int status;
void init_player(char *path);
void load_song(char *path);
void pause_song();
void stop_song();
void seek(char *seconds);
void free_player();
#endif
