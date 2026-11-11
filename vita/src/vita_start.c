/* The Vita's start-up, before Picori's main(): memory sizes, the clocks, the
 * log file and the data folder everything else is found in. */
#include <psp2/io/stat.h>
#include <psp2/power.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_log.h>
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

/* 1 when ux0:data/tmc/debug_log exists (checked once, at start-up):
 * timing reports in log.txt and an unbuffered log (see tmc_vita_start). */
static int sDebugLog = -1;

int TmcVita_DebugLog(void)
{
    if (sDebugLog < 0) {
        SceIoStat st;
        sDebugLog = sceIoGetstat(TMC_VITA_DATA_DIR "/debug_log", &st) >= 0;
    }
    return sDebugLog;
}

/* SDL's own messages (SDL_Log) into the log too */
static void SDLCALL tmc_vita_sdl_log(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
    (void)userdata;
    (void)category;
    (void)priority;
    fprintf(stderr, "[SDL] %s\n", message);
}

__attribute__((constructor(101))) static void tmc_vita_start(void)
{
    sceIoMkdir("ux0:data", 0777);
    sceIoMkdir(TMC_VITA_DATA_DIR, 0777); /* fails harmlessly when it exists */

    /* stdout and stderr to log.txt. Emptied first, then both opened for
     * appending (as sms-vita does): with stderr opened "w" only the lines
     * written here reached the file.
     *
     * The timing reports ([profile], [status], [audio]...) and an unbuffered
     * log are only for tuning and bug reports: create an empty file
     * ux0:data/tmc/debug_log to get them. Without it the log keeps the
     * events (saves, area changes, errors) in an 8 KB buffer written in one
     * go when full: every unbuffered line was a write to the memory card,
     * and one slow write (the card busy with an autosave) stalled whichever
     * thread made it (Deepwood Shrine: 13 fps, a 4.8 s frame). */
    TmcVita_DebugLog();
    FILE* fresh = fopen(TMC_VITA_DATA_DIR "/log.txt", "w");
    if (fresh)
        fclose(fresh);
    static char outBuf[8192], errBuf[8192];
    if (freopen(TMC_VITA_DATA_DIR "/log.txt", "a", stdout)) {
        if (sDebugLog)
            setvbuf(stdout, NULL, _IONBF, 0);
        else
            setvbuf(stdout, outBuf, _IOFBF, sizeof outBuf);
    }
    if (freopen(TMC_VITA_DATA_DIR "/log.txt", "a", stderr)) {
        if (sDebugLog)
            setvbuf(stderr, NULL, _IONBF, 0); /* every line on the card at once: survives a crash */
        else
            setvbuf(stderr, errBuf, _IOFBF, sizeof errBuf);
    }
    fprintf(stderr, "=== The Minish Cap for PS Vita (" TMC_VITA_DATA_DIR "/log.txt) ===\n");
    fprintf(stderr, "[vita] build %s\n", TMC_VITA_COMMIT);
    fprintf(stderr, "[vita] debug log %s\n", sDebugLog ? "on (debug_log found): timing reports, unbuffered"
                                                    : "off (create ux0:data/tmc/debug_log for timing reports)");

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
    SDL_SetLogOutputFunction(tmc_vita_sdl_log, NULL);
    fprintf(stderr, "[vita] start-up done\n");
}
