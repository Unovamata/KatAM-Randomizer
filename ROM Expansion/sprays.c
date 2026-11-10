/* sprays.c: Squeak Squad's spray paints that KatAM doesn't have (KatAMExpansionSprays.cs): SHADOW, IVORY, CITRUS, WHITE and
 * LAVENDER (its other 13 are KatAM's: their chests hold items). They are Kirby colors 16-20 (KatAM's are 0-15: anim 0x15's
 * variants, the UFO's 0x1A0's, and the tables KatAM reads a color's palettes from, all lengthened there), owned in
 * gTreasures.sprayPaintField's bits 14-18 (KatAM's 14 sprays take bits 0-13; NumSprayPaintsCollected counts only those, so the
 * completion stays KatAM's), found in Squeak Squad's chests (chest.c's NidlChestOpen) and shown on the Collection Room's second
 * page (collection_room.c).
 * The Collection Room reads its 40 cells from a table (gUnk_08386A50: an id, category << 8 | number, its icon and its text); its
 * literals point at gSprayRoom's copy in EWRAM instead (the bigger rooms' area, idle in the menus: rooms.c's gNidlBigArea), which
 * holds the page shown: KatAM's, or gSqueakSprays.page. A spray's id there is 0x111 + its number: the room takes (u8)id - 1 for
 * its color (16 + the number). The room's frame (sub_0814405C, set by sub_08143F88) is SprayRoomFrame: L or R, Right on the last
 * column or Left on the first, turn the page (the cells' icons freed and made again, the owned flags worked out again, the cursor
 * on the same row), then the room's own frame. The game opens the room through the main menu's step sub_081428A0, which is
 * SprayRoomStart first: KatAM's page in the copy;
 * The second page's cans are anim gSqueakSprays.iconAnim (KatAM's spray can, two to an OBJ palette); a spray's text is composed:
 * "SPRAY PAINT :" in the game's language and the name in KatAM's letters, loaded over the one the room loaded (SprayRoomText); */
#include "global.h"
#include "collection_room.h"
#include "malloc_vram.h"
#include "main.h"
#include "data.h"
#include "gba/macro.h"
#include "constants/songs.h"
#include "constants/languages.h"
#include "squeak.h"

extern u8 *const gNidlBigArea;
extern void sub_081428A0(void *);
extern void sub_0814405C(struct Unk_081434F8 *);
extern void sub_08143730(struct Unk_081434F8 *);
extern void sub_0814389C(struct Unk_081434F8 *, u8);
extern bool32 sub_08143A38(struct Unk_081434F8 *);
extern bool32 sub_081443D8(struct Unk_081434F8 *);
extern bool32 sub_08144438(struct Unk_081434F8 *);
extern void sub_08144304(struct Unk_081434F8 *);

#define TEXT_ROWS 7
#define TEXT_COLUMNS 30
#define SPRAY_KEYS (DPAD_ANY | L_BUTTON | R_BUTTON)

// A cell (gUnk_08386A50's entries): its id (category << 8 | number), its icon (gUnk_083868C8 + 4), its text (gUnk_08386664);
struct SprayEntry {
    u16 id;
    u8 icon;
    u8 text;
};

// A composed text (see above): LZ77 tiles, a TEXT_COLUMNS x TEXT_ROWS tilemap, as KatAM's (gUnk_082D7850's);
struct SprayText {
    const u32 *tileset;
    const u16 *tilemap;
};

// SqueakSprays; Filled in by KatAMExpansionSprays.cs;
struct SqueakSprays {
    const struct SprayEntry *katam;                     // KatAM's page (gUnk_08386A50);
    struct SprayEntry page[NUM_PUBLIC_TREASURES];       // the second page;
    u16 iconAnim;                                       // the cans: variant k spray k's;
    u8 iconPalettes[SQUEAK_SPRAYS];                     // their OBJ palette;
    const struct SprayText *texts;                      // [language][SQUEAK_SPRAYS];
};

const struct SqueakSprays gSqueakSprays = { NULL };

/* The room's state (EWRAM, see above): the cells' table, an entry before it (the room reads the cell before the first while its
 * cursor's last cell is -1: SprayRoomFrame's turn), the page shown and the cell whose text was put up; */
struct SprayRoom {
    struct SprayEntry before;
    struct SprayEntry entries[NUM_PUBLIC_TREASURES];
    u8 page;
    u8 textPage;
    s8 textCell;
};
#define ROOM ((struct SprayRoom *)gNidlBigArea)

// SprayOf(); A second page cell's spray number;
#define SprayOf(id) (((id) & 0xFF) - (SQUEAK_SPRAY_COLOR + 1))

// SprayRoomStart(); The main menu opening the Collection Room (sub_081428A0): KatAM's page;
void SprayRoomStart(void *menu) {
    struct SprayRoom *room = ROOM;
    u8 i;

    room->before.id = 0xFFFF;
    for (i = 0; i < NUM_PUBLIC_TREASURES; i++)
        room->entries[i] = gSqueakSprays.katam[i];
    room->page = 0;
    room->textPage = 0;
    room->textCell = -2;
    sub_081428A0(menu);
}

// SprayRoomPage(); The cells of a page, their owned flags and icons;
static void SprayRoomPage(struct Unk_081434F8 *a1, u8 page) {
    struct SprayRoom *room = ROOM;
    u8 i;

    for (i = 0; i < NUM_PUBLIC_TREASURES; i++) {
        struct Sprite *sprite = &a1->unk40[i];

        if (sprite->tilesVram) {
            VramFree(sprite->tilesVram);
            sprite->tilesVram = 0;
        }
        room->entries[i] = page ? gSqueakSprays.page[i] : gSqueakSprays.katam[i];
        a1->unk2[i] = FALSE;
    }
    room->page = page;
    if (page == 0) {
        sub_08143730(a1);
        for (i = 0; i < NUM_PUBLIC_TREASURES; i++)
            sub_0814389C(a1, i);
        return;
    }
    for (i = 0; i < NUM_PUBLIC_TREASURES; i++) {
        u16 id = room->entries[i].id;
        u8 spray = SprayOf(id);
        struct Sprite *sprite = &a1->unk40[i];

        if (id >> 8 != 1 || spray >= SQUEAK_SPRAYS || !(gTreasures.sprayPaintField & (1 << (SQUEAK_SPRAY_BIT + spray))))
            continue;
        a1->unk2[i] = TRUE;
        SpriteInitNoPointer2(sprite, VramMalloc(4), 0x3C0, gSqueakSprays.iconAnim, spray, 0, 0xFF, 0x10,
                             gSqueakSprays.iconPalettes[spray], 0x16 * (i % 10) + 0x15, 0x15 * (i / 10) + 0x23, 0);
    }
}

// SprayRoomText(); A spray's composed text, over the one the room put up, once the cursor is on it;
static void SprayRoomText(struct Unk_081434F8 *a1) {
    struct SprayRoom *room = ROOM;
    s8 cell = a1->unk2A;
    const struct SprayText *text;
    u8 spray, i;

    if (cell == room->textCell && room->page == room->textPage)
        return;
    room->textCell = cell;
    room->textPage = room->page;
    if (room->page == 0 || !a1->unk2[cell])
        return;
    spray = SprayOf(room->entries[cell].id);
    if (spray >= SQUEAK_SPRAYS || gLanguage >= NUM_LANGUAGES)
        return;
    text = &gSqueakSprays.texts[gLanguage * SQUEAK_SPRAYS + spray];
    LZ77UnCompVram(text->tileset, (u16 *)0x6004000);
    for (i = 0; i < TEXT_ROWS; i++)
        DmaCopy16(3, text->tilemap + i * TEXT_COLUMNS, (u16 *)0x600F340 + i * 0x20, TEXT_COLUMNS * sizeof(u16));
}

// SprayRoomFrame(); The room's frame (see above);
void SprayRoomFrame(struct Unk_081434F8 *a1) {
    struct SprayRoom *room = ROOM;
    u16 pressed = gPressedKeys, repeated = gRepeatedKeys;
    s8 cell = a1->unk2A, column = cell % 10, to = -1;

    if (pressed & (L_BUTTON | R_BUTTON))
        to = cell;
    else if (repeated & DPAD_RIGHT && column == 9)
        to = cell - 9;
    else if (repeated & DPAD_LEFT && column == 0)
        to = cell + 9;
    if (to >= 0 && !(pressed & (A_BUTTON | B_BUTTON))) {
        SprayRoomPage(a1, room->page ^ 1);
        a1->unk2A = to;
        // (the room's own updates for a cursor that moved: from the cell before the first, unk2B -1, they all run);
        a1->unk2B = -1;
        sub_08143A38(a1);
        sub_081443D8(a1);
        sub_08144438(a1);
        sub_08144304(a1);
        m4aSongNumStart(SE_MAIN_MENU_CURSOR);
        gPressedKeys &= ~SPRAY_KEYS;
        gRepeatedKeys &= ~SPRAY_KEYS;
    }
    sub_0814405C(a1);
    gPressedKeys = pressed;
    gRepeatedKeys = repeated;
    SprayRoomText(a1);
}
