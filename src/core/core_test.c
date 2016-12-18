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
    int i, count = SDL_GetNumAudioDevices(0);
    SDL_Log("%d\n", count);

    for (i = 0; i < count; ++i) {
        SDL_Log("Audio device %d: %s", i, SDL_GetAudioDeviceName(i, 0));
    }
    /* SDL_AudioSpec want, have; */
    /* SDL_AudioDeviceID dev; */
    /* SDL_memset(&want, 0, sizeof(want)); [> or SDL_zero(want) <] */
    /* want.freq = 48000; */
    /* want.format = AUDIO_F32; */
    /* want.channels = 2; */
    /* want.samples = 4096; */
    /* dev = SDL_OpenAudioDevice("HD-DAC1", 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE); */
    /* if (dev == 0) { */
        /* SDL_Log("Failed to open audio: %s", SDL_GetError()); */
    /* } */
    CPlayer *cp = cp_load_file(argv[1]);
    /* SDL_Delay(10000); */
    /* SDL_WaitEvent(&event); */
    int stop = 0;
    while(!stop){
        if(SDL_PollEvent(&event) > 0){
            switch(event.type) {
                case SDL_QUIT:
                    fprintf(stderr, "quit\n");
                    stop = 1;
                    break;
            }
        }
        else {
            //may defer execution
            SDL_Delay(10);
        }
    }

    player_destory(cp);

    return 0;
}
