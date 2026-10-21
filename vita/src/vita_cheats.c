/* In-game cheats for the Vita build, toggled in the settings menu (L+R →
 * Cheats) and remembered in ux0:data/tmc/cheats.txt.
 *
 * "Doesn't decrease" cheats block the decrease where the game makes it
 * (ModHealth, ModArrows, ModBombs, ModRupees, ModShells in the decomp, see
 * patches/picori): the count stays at whatever it is, it is never raised to
 * the game's maximum. The "full" cheats top up once per frame. */
#include "global.h"
#include "common.h"
#include "main.h"
#include "player.h"
#include "save.h"
#include "item.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#include "vita_cheats.h"
#include "vita_paths.h"

#define CHEATS_FILE TMC_VITA_DATA_DIR "/cheats.txt"

extern s32 GetIndexInKinstoneBag(u32);
extern void AddKinstoneToBag(u32);

static const struct {
    const char* key;
    const char* name;
    const char* help;
} kCheats[TMC_CHEAT_COUNT] = {
    [TMC_CHEAT_HEALTH] = { "health", "Health doesn't decrease", "Damage, falls and hazards take no hearts." },
    [TMC_CHEAT_ARROWS] = { "arrows", "Arrows don't decrease", "Shooting keeps the arrows you have." },
    [TMC_CHEAT_BOMBS] = { "bombs", "Bombs don't decrease", "Placing a bomb keeps the bombs you have." },
    [TMC_CHEAT_RUPEES] = { "rupees", "Rupees don't decrease", "Buying things is free." },
    [TMC_CHEAT_RUPEES_FULL] = { "rupees_full", "Wallet always full", "Rupees stay at your wallet's size." },
    [TMC_CHEAT_KINSTONES] = { "kinstones", "All regular Kinstones (99 each)",
                              "Red, blue and green pieces in the bag (not gold/story ones); needs the Kinstone Bag." },
    [TMC_CHEAT_SHELLS] = { "shells", "Mysterious Shells always 999", "Spent shells come back." },
    [TMC_CHEAT_BOMBS_FULL] = { "bombs_full", "Bomb bag always full", "Bombs stay at your bag's size (needs a bomb bag)." },
    [TMC_CHEAT_ARROWS_FULL] = { "arrows_full", "Quiver always full", "Arrows stay at your quiver's size (needs the bow)." },
};

static bool sCheat[TMC_CHEAT_COUNT];
static bool sLoaded;

static void Load(void) {
    sLoaded = true;
    FILE* f = fopen(CHEATS_FILE, "r");
    if (!f)
        return;
    char line[96];
    while (fgets(line, sizeof line, f)) {
        char* eq = strchr(line, '=');
        if (!eq)
            continue;
        *eq = '\0';
        for (int i = 0; i < TMC_CHEAT_COUNT; i++) {
            if (strcmp(line, kCheats[i].key) == 0)
                sCheat[i] = eq[1] == '1';
        }
    }
    fclose(f);
}

static void Save(void) {
    FILE* f = fopen(CHEATS_FILE, "w");
    if (!f)
        return;
    for (int i = 0; i < TMC_CHEAT_COUNT; i++)
        fprintf(f, "%s=%d\n", kCheats[i].key, sCheat[i] ? 1 : 0);
    fclose(f);
}

bool TmcVita_Cheat(int id) {
    if (!sLoaded)
        Load();
    return id >= 0 && id < TMC_CHEAT_COUNT && sCheat[id];
}

void TmcVita_SetCheat(int id, bool on) {
    if (!sLoaded)
        Load();
    if (id < 0 || id >= TMC_CHEAT_COUNT || sCheat[id] == on)
        return;
    sCheat[id] = on;
    Save();
    fprintf(stderr, "[cheats] %s %s\n", kCheats[id].key, on ? "on" : "off");
}

const char* TmcVita_CheatName(int id) {
    return (id >= 0 && id < TMC_CHEAT_COUNT) ? kCheats[id].name : "";
}

const char* TmcVita_CheatHelp(int id) {
    return (id >= 0 && id < TMC_CHEAT_COUNT) ? kCheats[id].help : "";
}

/* The regular (non-gold) kinstone pieces: red W/V/E, blue L/S, green C/G/P. */
static void FillKinstones(void) {
    if (GetInventoryValue(ITEM_KINSTONE_BAG) == 0)
        return;
    for (u32 id = 0x6E; id <= 0x75; id++) {
        s32 index = GetIndexInKinstoneBag(id);
        if (index >= 0 && gSave.kinstones.amounts[index] >= 99)
            continue;
        if (index < 0) {
            AddKinstoneToBag(id);
            index = GetIndexInKinstoneBag(id);
        }
        if (index >= 0)
            gSave.kinstones.amounts[index] = 99;
    }
}

void TmcVita_Cheats_Frame(void) {
    if (!sLoaded)
        Load();
    if (gMain.task != TASK_GAME)
        return;
    if (sCheat[TMC_CHEAT_RUPEES_FULL]) {
        const u16 size = gWalletSizes[gSave.stats.walletType].size;
        if (gSave.stats.rupees < size)
            gSave.stats.rupees = size;
    }
    /* Exactly 999: with Picori's "shells cap 9999" (Reborn tab) a pickup on
     * top of 999 went past 1000, and the 3-digit counter then showed only
     * the last digits (999 + 20 read as "019"). */
    if (sCheat[TMC_CHEAT_SHELLS] && gSave.stats.shells != 999)
        gSave.stats.shells = 999;
    /* bag/quiver type 0 is the first one (10 bombs, 30 arrows): owning the
     * item is what says there is one */
    if (sCheat[TMC_CHEAT_BOMBS_FULL] && GetInventoryValue(ITEM_BOMBS) != 0) {
        const u8 size = gBombBagSizes[gSave.stats.bombBagType];
        if (gSave.stats.bombCount < size)
            gSave.stats.bombCount = size;
    }
    if (sCheat[TMC_CHEAT_ARROWS_FULL] &&
        (GetInventoryValue(ITEM_BOW) != 0 || GetInventoryValue(ITEM_LIGHT_ARROW) != 0)) {
        const u8 size = gQuiverSizes[gSave.stats.quiverType];
        if (gSave.stats.arrowCount < size)
            gSave.stats.arrowCount = size;
    }
    if (sCheat[TMC_CHEAT_KINSTONES])
        FillKinstones();
}

/* ---- Change the player's name (any time) ------------------------------- */

static bool sWaitingName;
static char sNameStatus[64];
/* When the keyboard closed: SDL follows the typed text with an Enter key
 * press, which re-activated the focused "Change name..." button and opened
 * the keyboard again. That Enter is dropped (TmcVita_SwallowKey) and the
 * button ignored for a moment. */
static Uint64 sKeyboardClosedMs;
static Uint64 sKeyboardHiddenSinceMs;
static SDL_Window* sKeyboardWindow;

bool TmcVita_SwallowKey(int scancode) {
    return scancode == SDL_SCANCODE_RETURN && sKeyboardClosedMs != 0 && SDL_GetTicks() - sKeyboardClosedMs < 500;
}

static void PollKeyboardClosed(void);

void TmcVita_RequestNameChange(void) {
    /* asked again while the keyboard is closed: the last one ended empty */
    if (sWaitingName && !(sKeyboardWindow != NULL && SDL_ScreenKeyboardShown(sKeyboardWindow)))
        sWaitingName = false;
    if (sWaitingName || (sKeyboardClosedMs != 0 && SDL_GetTicks() - sKeyboardClosedMs < 500))
        return;
    SDL_Window* window = SDL_GetKeyboardFocus();
    if (window == NULL) {
        int count = 0;
        SDL_Window** windows = SDL_GetWindows(&count);
        if (windows != NULL && count > 0)
            window = windows[0];
        SDL_free(windows);
    }
    sKeyboardWindow = window;
    sKeyboardHiddenSinceMs = 0;
    if (window == NULL || !SDL_StartTextInput(window)) {
        snprintf(sNameStatus, sizeof sNameStatus, "Keyboard unavailable");
        return;
    }
    sWaitingName = true;
    snprintf(sNameStatus, sizeof sNameStatus, "Type the new name...");
}

/* Letters, digits, space and - . , (what the game's own name screen offers
 * in plain ASCII); up to 6 characters. */
bool TmcVita_OnTextInput(const char* text) {
    if (!sWaitingName)
        return false;
    sWaitingName = false;
    sKeyboardClosedMs = SDL_GetTicks();
    SDL_StopTextInput(sKeyboardWindow);
    u8 name[FILENAME_LENGTH];
    int n = 0;
    memset(name, 0, sizeof name);
    for (const char* p = text; *p != '\0' && n < FILENAME_LENGTH; p++) {
        const char c = *p;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' ||
            c == '.' || c == ',')
            name[n++] = (u8)c;
    }
    if (n == 0) {
        snprintf(sNameStatus, sizeof sNameStatus, "Name unchanged (nothing usable typed)");
        return true;
    }
    memcpy(gSave.name, name, sizeof name);
    snprintf(sNameStatus, sizeof sNameStatus, "Name set to %.*s - save in-game to keep it", n, (const char*)name);
    fprintf(stderr, "[cheats] name -> %.*s\n", n, (const char*)name);
    return true;
}

/* The keyboard closed with nothing typed (empty, or cancelled): SDL sends no
 * text then, so notice the closed keyboard instead, after a short grace so a
 * text event still in the queue is taken first. */
static void PollKeyboardClosed(void) {
    if (!sWaitingName)
        return;
    if (sKeyboardWindow != NULL && SDL_ScreenKeyboardShown(sKeyboardWindow)) {
        sKeyboardHiddenSinceMs = 0;
        return;
    }
    const Uint64 now = SDL_GetTicks();
    if (sKeyboardHiddenSinceMs == 0) {
        sKeyboardHiddenSinceMs = now;
        return;
    }
    if (now - sKeyboardHiddenSinceMs >= 300) {
        sWaitingName = false;
        sKeyboardClosedMs = now;
        SDL_StopTextInput(sKeyboardWindow);
        snprintf(sNameStatus, sizeof sNameStatus, "Name unchanged");
    }
}

const char* TmcVita_NameStatus(void) {
    PollKeyboardClosed(); /* drawn every frame while the Cheats tab is open */
    return sNameStatus;
}

const char* TmcVita_CurrentName(void) {
    static char name[FILENAME_LENGTH + 1];
    memcpy(name, gSave.name, FILENAME_LENGTH);
    name[FILENAME_LENGTH] = '\0';
    return name;
}
