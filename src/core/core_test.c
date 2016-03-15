#include "player.h"

/* make:
 *gcc -o player core_test.c player.c -lavformat -lavcodec -lswscale -lavutil -lswresample -lz `sdl2-config --cflags --libs`
 */

int main(int argc, char *argv[])
{

    SDL_Event event;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    // Register all formats and codecs
    global_init();
    CPlayer *cp = cp_load_file(argv[1]);
    SDL_WaitEvent(&event);

    player_destory(cp);

    return 0;
}
