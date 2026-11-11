/* collection.c: the Collection Room as one grid that scrolls sideways (KatAMExpansionCollection.cs builds what it shows): KatAM's
 * 10 x 4 cells first, as KatAM has them, then columns of the expansion's treasures (Squeak Squad's spray paints, the Secret Map's
 * pieces, the NiDL and Squeak Squad areas' maps: gCollectionGrid.items). The screen shows 10 columns; the cursor going past the
 * last one shown (or the first) slides the grid one column (SCROLL_FRAMES frames), past the grid's ends it goes round to the other
 * end, and L / R move it a screen.
 * The room reads its 40 cells from a table (gUnk_08386A50: an id, category << 8 | number, its icon and its text); its literals point
 * at GridRoom's copy in EWRAM instead (the bigger rooms' area, idle in the menus: rooms.c's gNidlBigArea), which holds the columns
 * shown (KatAM's 40 when the grid is at its start: the Sound Player's cell is in KatAM's first column, so its own screens, which look
 * the songs' sheets up in the table, only open on KatAM's table). The cells' owned flags (unk2) and sprites (unk40[0-39]) follow the
 * columns shown: a scroll moves them a column along, frees the column leaving once it has slid out and makes the column coming in
 * (KatAM's cells with the room's own sub_0814389C, the others with their item's anim).
 * The cells' frames slide with them: the lattice leaves BG0's image (its grid area's tiles are put back as the stripes behind it,
 * gCollectionGrid.image*, at BG0's tile GRID_IMAGE_TILE) for BG2 (gCollectionGrid.lattice*: KatAM's first column, its middle ones
 * repeated, its last; 512 x 256, scrolled), shown in window 0 (the grid's box); window 1 (the grid's rows, the screen's width) hides
 * the sprites sliding past the box's sides.
 * The hooks (KatAMExpansionCollection.cs): the main menu opening the room (sub_081428A0) is GridRoomStart first; the step after
 * the room's image is loaded (sub_08143EF4, set by sub_08143E6C) is GridRoomBg first; the room's frame (sub_0814405C, set by
 * sub_08143F88) is GridRoomFrame; its task's end (sub_08143664) is GridRoomEnd; */
#include "global.h"
#include "collection_room.h"
#include "malloc_vram.h"
#include "main.h"
#include "data.h"
#include "task.h"
#include "gba/macro.h"
#include "gba/io_reg.h"
#include "constants/songs.h"
#include "constants/languages.h"
#include "squeak.h"

extern u8 *const gNidlBigArea;
extern void sub_081428A0(void *);
extern void sub_0814405C(struct Unk_081434F8 *);
extern void sub_0814389C(struct Unk_081434F8 *, u8);
extern bool32 sub_08143A38(struct Unk_081434F8 *);
extern bool32 sub_081443D8(struct Unk_081434F8 *);
extern bool32 sub_08144438(struct Unk_081434F8 *);
extern void sub_08144304(struct Unk_081434F8 *);
extern void sub_08143EF4(struct Unk_081434F8 *);
extern void sub_08143664(struct Task *);

#define GRID_ROWS 4
#define GRID_SHOWN 10                       // the columns on the screen: KatAM's
#define GRID_EXTRA 12                       // the most columns after KatAM's
#define KATAM_SPRAYS 14
#define CELL_WIDTH 0x16
#define CELL_HEIGHT 0x15
#define CELL_X(column) (CELL_WIDTH * (column) + 0x15)
#define CELL_Y(row) (CELL_HEIGHT * (row) + 0x23)
#define ICON_TILES 4
#define SCROLL_FRAMES 5
#define GRID_KEYS (DPAD_ANY | L_BUTTON | R_BUTTON | A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON)
// The grid's box on the screen: its cells' frames;
#define GRID_LEFT 12
#define GRID_RIGHT 228
#define GRID_TOP 26
#define GRID_BOTTOM 107
// BG2, the lattice: char base 2, screen base 0x1C, 512 x 256, GRID_MAP_ROWS rows of 64 tiles from tile row GRID_MAP_TOP;
#define LATTICE_CHARS 2
#define LATTICE_SCREEN 0x1C
#define GRID_MAP_TOP 3
#define GRID_MAP_ROWS 11
#define LATTICE_WIDTH 64
// BG0 (char base 0, screen base 0x1F): its grid area's tiles (columns 1-28, rows 3-13) put back without the lattice;
#define GRID_IMAGE_TILE 0x180
#define GRID_IMAGE_LEFT 1
#define GRID_IMAGE_COLUMNS 28
#define BG0_MAP ((u16 *)(VRAM + 0xF800))
#define TEXT_ROWS 7
#define TEXT_COLUMNS 30

// A cell (gUnk_08386A50's entries): its id (category << 8 | number), its icon (gUnk_083868C8 + 4), its text (gUnk_08386664);
struct SprayEntry {
    u16 id;
    u8 icon;
    u8 text;
};

// A composed text: LZ77 tiles, a TEXT_COLUMNS x TEXT_ROWS tilemap, as KatAM's (gUnk_082D7850's);
struct SprayText {
    const u32 *tileset;
    const u16 *tilemap;
};

// A cell after KatAM's columns (8 bytes);
struct GridItem {
    u16 id;                         // category << 8 | number (0: an empty cell); see GridOwned
    u16 anim;                       // its icon: anim, variant, OBJ palette (0: none yet)
    u8 variant;
    u8 palette;
    u8 text;                        // its composed text (gCollectionGrid.texts' index; 0xFF: none)
    u8 roomText;                    // the room's text put up under it (gUnk_08386664's index)
};

/* gCollectionGrid; Filled in by KatAMExpansionCollection.cs (the offsets it writes are in comments): KatAM's cells, the columns
 * after them, the composed texts, the lattice and BG0's grid area per language; */
struct CollectionGrid {
    const struct SprayEntry *katam;                 // 0x00 gUnk_08386A50
    u8 extra;                                       // 0x04 the columns after KatAM's
    u8 textCount;                                   // 0x05 texts per language
    const struct SprayText *texts;                  // 0x08 [language][textCount]
    const u32 *latticeTiles;                        // 0x0C LZ77
    const u16 *latticeMap;                          // 0x10 GRID_MAP_ROWS x LATTICE_WIDTH
    const u32 *imageTiles[NUM_LANGUAGES];           // 0x14 LZ77
    const u16 *imageMap[NUM_LANGUAGES];             // 0x2C GRID_MAP_ROWS x GRID_IMAGE_COLUMNS
    struct GridItem items[GRID_ROWS][GRID_EXTRA];   // 0x44
};

const struct CollectionGrid gCollectionGrid = { NULL };

/* The room's state (EWRAM, see above): the cells' table, an entry before it (the room reads the cell before the first while its
 * cursor's last cell is -1: GridRefresh), the first column shown, the scroll going on (1: the cells moving left, -1: right), its
 * frame, the column it leaves, the cell whose composed text was put up and its first column; */
struct GridRoom {
    struct SprayEntry before;
    struct SprayEntry entries[NUM_PUBLIC_TREASURES];
    s8 first;
    s8 direction;
    u8 frame;
    s8 textCell;
    s8 textFirst;
    bool8 leavingOwned[GRID_ROWS];
    struct Sprite leaving[GRID_ROWS];
};
#define ROOM ((struct GridRoom *)gNidlBigArea)

// (how far a scroll has moved the cells, frame by frame);
static const u8 sScrollSteps[SCROLL_FRAMES] = { 4, 9, 13, 18, 22 };

// GridOwned(); Whether the treasure of a cell is owned;
static bool8 GridOwned(u16 id) {
    u8 n = id;

    switch (id >> 8) {
    case 0x01: // spray paints: the default, KatAM's (bits 0-13), Squeak Squad's (colors 17-21, bits 14-18)
        if (n == 0)
            return TRUE;
        if (n <= KATAM_SPRAYS)
            return (gTreasures.sprayPaintField >> (n - 1)) & 1;
        n -= SQUEAK_SPRAY_COLOR + 1;
        return n < SQUEAK_SPRAYS && (gTreasures.sprayPaintField >> (SQUEAK_SPRAY_BIT + n)) & 1;
    case 0x02: // maps: KatAM's areas' (bits 0-9), the expansion's (bits 10-25)
        return (gTreasures.bigChestField >> n) & 1;
    case 0x04: // music sheets
        return (gTreasures.musicPlayerAndSheetsField >> (n + 1)) & 1;
    case 0x08: // vitalities
        return n < gTreasures.vitalityField;
    case 0x10: // the Sound Player
        return gTreasures.musicPlayerAndSheetsField & 1;
    case 0x20: // the Secret Map's pieces (bits 16-22)
        return n < SECRET_MAP_PIECES && (gTreasures.musicPlayerAndSheetsField >> (SECRET_MAP_BIT + n)) & 1;
    }
    return FALSE;
}

// GridItemAt(); The item of a cell after KatAM's columns;
static const struct GridItem *GridItemAt(u8 row, u8 column) {
    return &gCollectionGrid.items[row][column - GRID_SHOWN];
}

// GridFree(); A sprite's tiles freed;
static void GridFree(struct Sprite *sprite) {
    if (sprite->tilesVram) {
        VramFree(sprite->tilesVram);
        sprite->tilesVram = 0;
    }
}

// GridCell(); A cell (its sprite's tiles free) made from the grid's column: its entry, its owned flag, its icon;
static void GridCell(struct Unk_081434F8 *a1, u8 cell, u8 column) {
    struct SprayEntry *entry = &ROOM->entries[cell];
    u8 row = cell / GRID_SHOWN;
    const struct GridItem *item;

    if (column < GRID_SHOWN) {
        *entry = gCollectionGrid.katam[row * GRID_SHOWN + column];
        if ((a1->unk2[cell] = GridOwned(entry->id)))
            sub_0814389C(a1, cell);
        return;
    }
    item = GridItemAt(row, column);
    entry->id = item->id;
    entry->icon = 0;
    entry->text = item->roomText;
    a1->unk2[cell] = item->id && item->anim && GridOwned(item->id);
    if (a1->unk2[cell])
        SpriteInitNoPointer2(&a1->unk40[cell], VramMalloc(ICON_TILES), 0x3C0, item->anim, item->variant, 0, 0xFF, 0x10, item->palette,
                             CELL_X(cell % GRID_SHOWN), CELL_Y(row), 0);
}

// GridFill(); The columns from first shown;
static void GridFill(struct Unk_081434F8 *a1, s8 first) {
    u8 cell;

    ROOM->first = first;
    for (cell = 0; cell < NUM_PUBLIC_TREASURES; cell++) {
        GridFree(&a1->unk40[cell]);
        GridCell(a1, cell, first + cell % GRID_SHOWN);
    }
    gBgScrollRegs[2][0] = first * CELL_WIDTH;
}

// GridMove(); A cell's entry, flag and sprite moved to another cell;
static void GridMove(struct Unk_081434F8 *a1, u8 to, u8 from) {
    CpuCopy32(&a1->unk40[from], &a1->unk40[to], sizeof(struct Sprite));
    a1->unk2[to] = a1->unk2[from];
    ROOM->entries[to] = ROOM->entries[from];
}

/* GridShift(); A scroll's start: the cells a column along (direction 1: to the left), the column leaving kept aside until it has
 * slid out, the column coming in made; */
static void GridShift(struct Unk_081434F8 *a1, s8 direction) {
    struct GridRoom *room = ROOM;
    u8 row, column, leaving = direction > 0 ? 0 : GRID_SHOWN - 1, coming = GRID_SHOWN - 1 - leaving;

    room->first += direction;
    for (row = 0; row < GRID_ROWS; row++) {
        u8 base = row * GRID_SHOWN;

        CpuCopy32(&a1->unk40[base + leaving], &room->leaving[row], sizeof(struct Sprite));
        room->leavingOwned[row] = a1->unk2[base + leaving];
        if (direction > 0)
            for (column = 0; column < GRID_SHOWN - 1; column++)
                GridMove(a1, base + column, base + column + 1);
        else
            for (column = GRID_SHOWN - 1; column > 0; column--)
                GridMove(a1, base + column, base + column - 1);
        a1->unk40[base + coming].tilesVram = 0;
        GridCell(a1, base + coming, room->first + coming);
    }
    room->direction = direction;
    room->frame = 0;
}

// GridPlace(); The cells' sprites, the column leaving and the lattice where the scroll has them this frame;
static void GridPlace(struct Unk_081434F8 *a1) {
    struct GridRoom *room = ROOM;
    s16 moved = sScrollSteps[room->frame], shift = room->direction * (CELL_WIDTH - moved);
    s16 leavingX = CELL_X(room->direction > 0 ? 0 : GRID_SHOWN - 1) - room->direction * moved;
    u8 cell, row;

    for (cell = 0; cell < NUM_PUBLIC_TREASURES; cell++)
        a1->unk40[cell].x = CELL_X(cell % GRID_SHOWN) + shift;
    for (row = 0; row < GRID_ROWS; row++)
        room->leaving[row].x = leavingX;
    gBgScrollRegs[2][0] = room->first * CELL_WIDTH - shift;
}

/* GridRefresh(); The room's own updates for a cursor that moved (after a scroll or a jump the cell under it changed): from the cell
 * before the first, unk2B -1, they all run; */
static void GridRefresh(struct Unk_081434F8 *a1) {
    a1->unk2B = -1;
    sub_08143A38(a1);
    sub_081443D8(a1);
    sub_08144438(a1);
    sub_08144304(a1);
    m4aSongNumStart(SE_MAIN_MENU_CURSOR);
}

// GridText(); A cell's composed text, over the one the room put up, once the cursor is on it;
static void GridText(struct Unk_081434F8 *a1) {
    struct GridRoom *room = ROOM;
    s8 cell = a1->unk2A;
    u8 column = room->first + cell % GRID_SHOWN, language = gLanguage, i;
    const struct GridItem *item;
    const struct SprayText *text;

    if (cell == room->textCell && room->first == room->textFirst)
        return;
    room->textCell = cell;
    room->textFirst = room->first;
    if (column < GRID_SHOWN || !a1->unk2[cell] || language >= NUM_LANGUAGES)
        return;
    item = GridItemAt(cell / GRID_SHOWN, column);
    if (item->text >= gCollectionGrid.textCount)
        return;
    text = &gCollectionGrid.texts[language * gCollectionGrid.textCount + item->text];
    LZ77UnCompVram(text->tileset, (u16 *)(VRAM + 0x4000));
    for (i = 0; i < TEXT_ROWS; i++)
        DmaCopy16(3, text->tilemap + i * TEXT_COLUMNS, (u16 *)(VRAM + 0xF340) + i * 0x20, TEXT_COLUMNS * sizeof(u16));
}

// GridRoomStart(); The main menu opening the Collection Room (sub_081428A0): the grid at its start;
void GridRoomStart(void *menu) {
    struct GridRoom *room = ROOM;
    u8 i;

    room->before.id = 0xFFFF;
    for (i = 0; i < NUM_PUBLIC_TREASURES; i++)
        room->entries[i] = gCollectionGrid.katam[i];
    room->first = 0;
    room->direction = 0;
    room->textCell = -2;
    for (i = 0; i < GRID_ROWS; i++)
        room->leaving[i].tilesVram = 0;
    sub_081428A0(menu);
}

// GridRoomBg(); After the room's image is loaded (sub_08143E6C): its grid area without the lattice, the lattice on BG2, the windows;
void GridRoomBg(struct Unk_081434F8 *a1) {
    u8 language = gLanguage < NUM_LANGUAGES ? gLanguage : LANGUAGE_ENGLISH, row;

    if (gCollectionGrid.imageTiles[language]) {
        LZ77UnCompVram(gCollectionGrid.imageTiles[language], (u16 *)(VRAM + GRID_IMAGE_TILE * 0x20));
        for (row = 0; row < GRID_MAP_ROWS; row++)
            DmaCopy16(3, gCollectionGrid.imageMap[language] + row * GRID_IMAGE_COLUMNS,
                      BG0_MAP + (GRID_MAP_TOP + row) * 0x20 + GRID_IMAGE_LEFT, GRID_IMAGE_COLUMNS * sizeof(u16));
        LZ77UnCompVram(gCollectionGrid.latticeTiles, (u16 *)(VRAM + LATTICE_CHARS * 0x4000));
        DmaFill16(3, 0, (u16 *)(VRAM + LATTICE_SCREEN * 0x800), 0x1000);
        // (512 x 256: two screen blocks side by side, 32 tiles each);
        for (row = 0; row < GRID_MAP_ROWS; row++) {
            const u16 *line = gCollectionGrid.latticeMap + row * LATTICE_WIDTH;
            u16 *map = (u16 *)(VRAM + LATTICE_SCREEN * 0x800) + (GRID_MAP_TOP + row) * 0x20;

            DmaCopy16(3, line, map, 0x20 * sizeof(u16));
            DmaCopy16(3, line + 0x20, map + 0x400, 0x20 * sizeof(u16));
        }
        gBgCntRegs[2] = BGCNT_PRIORITY(1) | BGCNT_CHARBASE(LATTICE_CHARS) | BGCNT_SCREENBASE(LATTICE_SCREEN) | BGCNT_TXT512x256;
        gBgScrollRegs[2][0] = 0;
        gBgScrollRegs[2][1] = 0;
        gWinRegs[0] = WIN_RANGE(GRID_LEFT, GRID_RIGHT);
        gWinRegs[1] = WIN_RANGE(0, DISPLAY_WIDTH);
        gWinRegs[2] = WIN_RANGE(GRID_TOP, GRID_BOTTOM);
        gWinRegs[3] = WIN_RANGE(GRID_TOP, GRID_BOTTOM);
        gWinRegs[4] = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_CLR;
        gWinRegs[5] = WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;
        gDispCnt |= DISPCNT_BG2_ON | DISPCNT_WIN0_ON | DISPCNT_WIN1_ON;
    }
    sub_08143EF4(a1);
}

// GridRoomFrame(); The room's frame (see above);
void GridRoomFrame(struct Unk_081434F8 *a1) {
    struct GridRoom *room = ROOM;
    u16 pressed = gPressedKeys, repeated = gRepeatedKeys;
    s8 cell = a1->unk2A, column = cell % GRID_SHOWN, last = gCollectionGrid.extra;
    bool8 masked = room->direction != 0;
    u8 row;

    if (!masked && !(pressed & (A_BUTTON | B_BUTTON))) {
        masked = TRUE;
        if (repeated & DPAD_RIGHT && column == GRID_SHOWN - 1) {
            if (room->first < last)
                GridShift(a1, 1);
            else {
                GridFill(a1, 0);
                a1->unk2A = cell - (GRID_SHOWN - 1);
            }
        } else if (repeated & DPAD_LEFT && column == 0) {
            if (room->first > 0)
                GridShift(a1, -1);
            else {
                GridFill(a1, last);
                a1->unk2A = cell + GRID_SHOWN - 1;
            }
        } else if (pressed & R_BUTTON && room->first < last)
            GridFill(a1, room->first + GRID_SHOWN < last ? room->first + GRID_SHOWN : last);
        else if (pressed & L_BUTTON && room->first > 0)
            GridFill(a1, room->first > GRID_SHOWN ? room->first - GRID_SHOWN : 0);
        else
            masked = FALSE;
        if (masked)
            GridRefresh(a1);
    }
    if (masked) {
        gPressedKeys &= ~GRID_KEYS;
        gRepeatedKeys &= ~GRID_KEYS;
    }
    if (room->direction)
        GridPlace(a1);
    sub_0814405C(a1);
    gPressedKeys = pressed;
    gRepeatedKeys = repeated;
    if (room->direction) {
        if (++room->frame < SCROLL_FRAMES) {
            for (row = 0; row < GRID_ROWS; row++)
                if (room->leavingOwned[row])
                    DisplaySprite(&room->leaving[row]);
        } else {
            for (row = 0; row < GRID_ROWS; row++)
                GridFree(&room->leaving[row]);
            room->direction = 0;
        }
    }
    GridText(a1);
}

// GridRoomEnd(); The room's task ending (sub_08143664): BG2 and the windows off again;
void GridRoomEnd(struct Task *t) {
    u8 i;

    for (i = 0; i < GRID_ROWS; i++)
        GridFree(&ROOM->leaving[i]);
    sub_08143664(t);
    gDispCnt &= ~(DISPCNT_BG2_ON | DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
    gBgCntRegs[2] = 0;
    gBgScrollRegs[2][0] = 0;
    for (i = 0; i < 6; i++)
        gWinRegs[i] = 0;
}
