/* The Vita's start-up, before Picori's main(): memory sizes, the clocks, the
 * log file and the data folder everything else is found in. */
#include <psp2/io/stat.h>
#include <psp2/power.h>
#include <SDL3/SDL_hints.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "vita_paths.h"

#ifndef TMC_VITA_COMMIT
#define TMC_VITA_COMMIT "unknown"
#endif

/* The newlib heap (Picori's assets, the game's memory, audio) and the main
 * thread's stack (the game runs on it). */
unsigned int _newlib_heap_size_user = 192u * 1024u * 1024u;
unsigned int sceUserMainThreadStackSize = 8u * 1024u * 1024u;

__attribute__((constructor(101))) static void tmc_vita_start(void)
{
    sceIoMkdir("ux0:data", 0777);
    sceIoMkdir(TMC_VITA_DATA_DIR, 0777); /* fails harmlessly when it exists */

    /* stdout and stderr to log.txt, unbuffered, so it survives a crash */
    if (freopen(TMC_VITA_DATA_DIR "/log.txt", "w", stderr))
        setvbuf(stderr, NULL, _IONBF, 0);
    if (freopen(TMC_VITA_DATA_DIR "/log.txt", "a", stdout))
        setvbuf(stdout, NULL, _IONBF, 0);
    fprintf(stderr, "=== The Minish Cap for PS Vita (" TMC_VITA_DATA_DIR "/log.txt) ===\n");
    fprintf(stderr, "[vita] build %s\n", TMC_VITA_COMMIT);

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    /* Picori finds config.json, tmc.sav, the ROM and its asset cache
     * relative to the current folder (as on Android). */
    if (chdir(TMC_VITA_DATA_DIR) != 0)
        fprintf(stderr, "[vita] could not enter " TMC_VITA_DATA_DIR "\n");
    char cwd[256];
    if (getcwd(cwd, sizeof cwd))
        fprintf(stderr, "[vita] data folder: %s\n", cwd);

    /* The front screen's touches as touches, not mouse clicks; the rear pad
     * off until the controls use it. */
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_VITA_ENABLE_BACK_TOUCH, "0");
}
