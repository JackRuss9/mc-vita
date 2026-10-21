/* A one-line game status in log.txt next to each [profile] report (every
 * 120 frames) while the Vita port is being brought up: which task/state the
 * game is in, the fade, the display registers and how many sprites reach
 * the PPU. Enough to tell "black because faded out" from "black because
 * nothing is drawn" from a log. */
#include "global.h"
#include "fade.h"
#include "main.h"
#include "menu.h"
#include "structures.h"
#include "entity.h"

#include <stdio.h>

extern u8 gIoMem[0x400];
extern u16 gOamMem[0x400 / 2];
extern int Port_PPU_VisibleFrameWidth(void);
extern int gTmcVitaDbgUpdated, gTmcVitaDbgDrawCalls, gTmcVitaDbgDrawListed, gTmcVitaDbgOamMax, gTmcVitaDbgFrameMiss,
    gTmcVitaDbgPieces;
extern LinkedList gEntityLists[9];

static unsigned io16(unsigned off)
{
    return (unsigned)gIoMem[off] | ((unsigned)gIoMem[off + 1] << 8);
}

void TmcVita_LogStatus(void)
{
    /* sprites the PPU will see: OAM entries not hidden (attr0 bits 8-9 != 2) */
    int visible = 0;
    for (int i = 0; i < 128; i++) {
        if (((gOamMem[i * 4] >> 8) & 3) != 2)
            visible++;
    }
    /* entities per list (0-7 entities, 8 managers) */
    char lists[64];
    int n = 0;
    for (int l = 0; l < 9; l++) {
        int count = 0;
        Entity* e = gEntityLists[l].first;
        while (e != NULL && e != (Entity*)&gEntityLists[l] && count < 999) {
            count++;
            e = e->next;
        }
        n += snprintf(lists + n, sizeof lists - (size_t)n, "%s%d", l ? "," : "", count);
    }
    fprintf(stderr,
            "[entities] lists=%s | per 120 frames: updated=%d DrawEntity=%d listed=%d frame-miss=%d pieces=%d "
            "oam-max=%d\n",
            lists, gTmcVitaDbgUpdated, gTmcVitaDbgDrawCalls, gTmcVitaDbgDrawListed, gTmcVitaDbgFrameMiss,
            gTmcVitaDbgPieces, gTmcVitaDbgOamMax);
    gTmcVitaDbgUpdated = gTmcVitaDbgDrawCalls = gTmcVitaDbgDrawListed = gTmcVitaDbgFrameMiss = 0;
    gTmcVitaDbgPieces = gTmcVitaDbgOamMax = 0;
    fprintf(stderr,
            "[status] task=%u state=%u sub=%u overlay=%u | fade active=%u type=%04x progress=%u color=%u | "
            "DISPCNT=%04x(%04x) BLDCNT=%04x BLDY=%04x WIN0H=%04x WININ=%04x WINOUT=%04x | "
            "oam visible=%d built=%u | width=%d\n",
            gMain.task, gMain.state, gMain.substate, gMenu.overlayType, gFadeControl.active, gFadeControl.type,
            gFadeControl.progress, gFadeControl.color, io16(0x00), (unsigned)gScreen.lcd.displayControl, io16(0x50),
            io16(0x54), io16(0x40), io16(0x48), io16(0x4A), visible, (unsigned)gOAMControls.updated,
            Port_PPU_VisibleFrameWidth());
}
