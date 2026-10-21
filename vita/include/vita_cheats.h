/* In-game cheats (vita/src/vita_cheats.c). */
#ifndef VITA_CHEATS_H
#define VITA_CHEATS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    TMC_CHEAT_HEALTH,
    TMC_CHEAT_ARROWS,
    TMC_CHEAT_BOMBS,
    TMC_CHEAT_RUPEES,
    TMC_CHEAT_RUPEES_FULL,
    TMC_CHEAT_KINSTONES,
    TMC_CHEAT_SHELLS,
    TMC_CHEAT_BOMBS_FULL,
    TMC_CHEAT_ARROWS_FULL,
    TMC_CHEAT_COUNT
};

bool TmcVita_Cheat(int id);
void TmcVita_SetCheat(int id, bool on);
const char* TmcVita_CheatName(int id);
const char* TmcVita_CheatHelp(int id);
/* "Change name": opens the Vita keyboard; the typed text arrives as an SDL
 * text event that TmcVita_OnTextInput (port_bios.c) takes */
void TmcVita_RequestNameChange(void);
bool TmcVita_OnTextInput(const char* text);
bool TmcVita_SwallowKey(int scancode);
const char* TmcVita_NameStatus(void);
const char* TmcVita_CurrentName(void);
/* once per frame (port_bios.c): the "always full" cheats */
void TmcVita_Cheats_Frame(void);

#ifdef __cplusplus
}
#endif

#endif
