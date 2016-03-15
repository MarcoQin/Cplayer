#ifndef _player_core_h_
#define _player_core_h_
#define PLAYING 1
#define STOP 0
#define PAUSE 2
#define ALIVE 0
#define KILL 1
#include "core/player.h"
extern int pid;
extern int status;
void init_player(char *path);
void load_song(int id);
void pause_song();
void stop_song();
void seek(int seconds);
void free_player();
#endif
