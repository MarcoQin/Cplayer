#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "player_core.h"
#include "utils.h"
#include "db.h"
#include <cdk.h>

int status = 0;   /* play status */
int alive = KILL; /* pipe alive false */

void init_player(char *path) {
    global_init();
    status = PLAYING; /* playing status */
    alive = ALIVE;
    cp_load_file(path);
}

void load_song(int id) {
    char *path = get_song_path(id);
    db_update_song_state(PLAYING, id);
    if (alive == ALIVE) {
        cp_load_file(path);
        status = PLAYING; /* playing status */
    } else {
        init_player(path);
    }
    refresh();
}

void stop_song() {
    if (alive == ALIVE && status != STOP) {
        cp_stop_audio();
    }
    status = STOP; /* playing status */
}

void pause_song() {
    if (alive == ALIVE && status != STOP) {
        status = status == PAUSE ? PLAYING : PAUSE; /* playing status */
        cp_pause_audio();
    }
}

void seek(int seconds) {
    if (alive == ALIVE && status == PLAYING) {
        cp_seek_audio_by_sec(seconds);
    }
}

void free_player() {
    cp_free_player();
    status = STOP;
    alive = KILL;
}
