/* title.c: Nightmare in Dream Land's title screen animation in KatAM, the Kirby swarm (see KatAMExpansion.AddTitle, which copies
 * NiDL's data into gNidlTitle and detours KatAM's intro, sub_08145B64, to CreateNidlTitle).
 * NiDL (USA addresses): after the HAL Laboratory / Nintendo logo, 115 pink Kirbys run, walk, float and tumble across the screen
 * from the right, its title appearing under them. They are data: 0x080CAA3C sets each one up from 0x08757440 (4 halfwords:
 * script, delay, x, y; the draw order is 15 - ((y - 1) >> 4)) and 0x080CAAB8 runs them every frame:
 * - after its delay, a Kirby runs commands of its script (0x087577D8, by index), 14 bytes each: a mask, then 6 slots (velocity x
 *   and y, acceleration x and y, frame, delay, in 8.8 pixels), the slots whose mask bit (0x01-0x20) is set taken; 0x40 plays the
 *   sound effect in slot 0; 0x80 calls script slot 0 (keeping the next command); 0x100 jumps to script slot 0; 0x200 marks a loop
 *   (slot 0, when not 0, its count); 0x400 ends it (forever when its count was 0); 0x800 restarts the script; 0x1000 returns to
 *   the kept command; 0x2000 ends the Kirby. Commands run until one sets a delay or ends it;
 * - then velocity += acceleration, position += velocity; a Kirby with a frame is drawn there (NiDL's frame table 0x087554B8: one
 *   32x32 sprite each, 16 pixels up and left, from NiDL's 2D sprite sheet, some flipped), and ends once it's off the screen (x
 *   outside -15..271, y outside -31..191).
 * NiDL's logo script plays sound 0x10D 129 frames into the swarm, turns its title's background on around the logo at 164, and
 * wipes the logo away behind the swarm from 185 to 271 (a window). Here: KatAM's HAL Laboratory / Nintendo / Flagship logo stays
 * up instead of KatAM's parade, the swarm runs over it (each Kirby drawn with KatAM's OAM slots, the poses through slots of
 * object VRAM in 1D order, NiDL's Kirby palette in palette 15), and at frame 210, when it covers most of the screen, KatAM's own
 * title screen starts under it (KatAM's logo and title share VRAM, so there's no wipe): CreateTitleScreen, whose fade from white
 * leaves the sprites out, so the title fades in while the Kirbys keep running. The swarm ends when its last Kirby is gone, when
 * the title screen's task is gone, or when it takes A or Start.
 * A, B or Start before the title skip to it, as NiDL's logo script's waits do. NiDL's sound 0x9C (the first Kirby's running steps)
 * is KatAM's dash, and 0x10D (the crowd) is the stampede KatAM's own intro plays (SE_INTRO_AMBIENT_2, every 5 frames while 24
 * Kirbys or more are on the screen); */
#include "global.h"
#include "data.h"
#include "functions.h"
#include "main.h"
#include "task.h"
#include "sprite.h"
#include "malloc_vram.h"
#include "constants/songs.h"
#include "title.h"

#define NIDL_TITLE_CROWD_STEP 129
#define NIDL_TITLE_SHOW_STEP 210
#define NIDL_TITLE_CROWD_KIRBYS 24
#define NIDL_TITLE_PALETTE 15
#define NIDL_SE_RUN 0x9C
#define NIDL_TITLE_MAX_COMMANDS 64
#define NIDL_TITLE_INPUT_FRAMES 0x4A
#define NIDL_TITLE_SLOTS 36
#define NIDL_TITLE_MAX_POSES 64
#define NIDL_TITLE_MAX_UPLOADS 8
#define NIDL_TITLE_POSE_BYTES (16 * TILE_SIZE_4BPP)
#define NIDL_TITLE_DMA_QUEUE 0x40

// The title screen (title_screen.c; its main is static there), to find its task;
extern void CreateTitleScreen(void);
extern void TitleScreenMain(void);

struct NidlTitleKirby {
    const u16 *pc, *saved;
    s32 x, y;
    s16 script, frame, delay, count;
    s16 vx, vy, ax, ay;
    s16 screenX, screenY;
    u8 order;
};

struct NidlTitleSwarm {
    u32 vram;
    struct Task *title;
    u16 step, titleFrames;
    u8 slotPose[NIDL_TITLE_SLOTS], poseSlot[NIDL_TITLE_MAX_POSES];
    u16 slotDrawn[NIDL_TITLE_SLOTS];
    struct NidlTitleKirby kirbys[NIDL_TITLE_MAX_KIRBYS];
};

static void NidlTitleMain(void);
static void NidlTitleDestroy(struct Task *);

/* CreateNidlTitle; In place of KatAM's intro (sub_08145B64, detoured: LogoEnd's call, with the logo still up): sets up every Kirby
 * as NiDL's 0x080CAA3C does, the poses and palette; */
void CreateNidlTitle(u16 unused) {
    struct Task *task = TaskCreate(NidlTitleMain, sizeof(struct NidlTitleSwarm), 0x1000, TASK_USE_EWRAM, NidlTitleDestroy);
    struct NidlTitleSwarm *swarm = TaskGetStructPtr(task);
    u16 i, count = gNidlTitle.count > NIDL_TITLE_MAX_KIRBYS ? NIDL_TITLE_MAX_KIRBYS : gNidlTitle.count;

    CpuFill16(0, swarm, sizeof(struct NidlTitleSwarm));
    for (i = 0; i < NIDL_TITLE_MAX_KIRBYS; ++i) swarm->kirbys[i].script = -1;
    for (i = 0; i < count; ++i) {
        struct NidlTitleKirby *kirby = &swarm->kirbys[i];
        const s16 *entry = gNidlTitle.entries + i * 4;

        kirby->script = entry[0];
        kirby->pc = gNidlTitle.scripts[entry[0]];
        kirby->frame = -1;
        kirby->delay = entry[1];
        kirby->x = entry[2] << 8;
        kirby->y = entry[3] << 8;
        kirby->order = 15 - ((entry[3] - 1) >> 4);
    }

    // The intro's object VRAM heap (sub_08145B64's), the pose slots at its start;
    gVramHeapMaxTileSlots = 0x400;
    gVramHeapStartAddr = (u32)OBJ_VRAM0;
    swarm->vram = VramMalloc(NIDL_TITLE_SLOTS * 16);
    for (i = 0; i < NIDL_TITLE_SLOTS; ++i) swarm->slotPose[i] = 0xFF;
    for (i = 0; i < NIDL_TITLE_MAX_POSES; ++i) swarm->poseSlot[i] = 0xFF;
    CpuCopy16(gNidlTitle.palette, gObjPalette + NIDL_TITLE_PALETTE * 16, 16 * sizeof(u16));
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
    gDispCnt |= DISPCNT_OBJ_ON;
}

static void NidlTitleDestroy(struct Task *task) {
    struct NidlTitleSwarm *swarm = TaskGetStructPtr(task);

    if (swarm->vram) {
        VramFree(swarm->vram);
        swarm->vram = 0;
    }
}

// NidlTitleRun(); A Kirby's commands, until one sets a delay or ends it (NiDL's 0x080CAAB8);
static void NidlTitleRun(struct NidlTitleKirby *kirby) {
    u16 commands;

    for (commands = 0; commands < NIDL_TITLE_MAX_COMMANDS && kirby->script != -1 && !kirby->delay; ++commands) {
        const u16 *pc = kirby->pc;
        u16 mask = *pc++, bit;
        s16 first = *pc;

        for (bit = 0; bit < 14; ++bit) {
            if (!(mask & (1 << bit))) {
                if (bit <= 5) ++pc;
                continue;
            }
            switch (1 << bit) {
            case 0x1: kirby->vx = *pc++; break;
            case 0x2: kirby->vy = *pc++; break;
            case 0x4: kirby->ax = *pc++; break;
            case 0x8: kirby->ay = *pc++; break;
            case 0x10: kirby->frame = *pc++; break;
            case 0x20: kirby->delay = *pc++; break;
            case 0x40:
                if (first == NIDL_SE_RUN) m4aSongNumStart(SE_KIRBY_DASH);
                break;
            case 0x80:
                kirby->saved = pc;
                pc = gNidlTitle.scripts[first];
                break;
            case 0x100:
                kirby->script = first;
                pc = gNidlTitle.scripts[first];
                break;
            case 0x200:
                kirby->saved = pc;
                if (first) kirby->count = first;
                break;
            case 0x400:
                if (kirby->count && !--kirby->count) break;
                pc = kirby->saved;
                break;
            case 0x800: pc = gNidlTitle.scripts[kirby->script]; break;
            case 0x1000: pc = kirby->saved; break;
            case 0x2000: kirby->script = -1; break;
            }
        }
        kirby->pc = pc;
    }
}

// NidlTitleShow(); KatAM's title screen, under the swarm: its fade from white leaves the sprites out;
static void NidlTitleShow(struct NidlTitleSwarm *swarm) {
    u16 i;

    CreateTitleScreen();
    gBldRegs.bldCnt &= ~BLDCNT_TGT1_OBJ;
    for (i = 0; i < MAX_TASK_NUM; ++i) {
        if (gTasks[i].main == TitleScreenMain) swarm->title = &gTasks[i];
    }
}

static void NidlTitleMain(void) {
    struct NidlTitleSwarm *swarm = TaskGetStructPtr(gCurTask);
    u32 tile = (swarm->vram - (u32)OBJ_VRAM0) / TILE_SIZE_4BPP;
    u16 i, j, shown = 0, alive = 0, uploads = 0;
    bool8 used[NIDL_TITLE_SLOTS];

    if (!swarm->title && gPressedKeys & (START_BUTTON | A_BUTTON | B_BUTTON)) {
        NidlTitleShow(swarm);
        TaskDestroy(gCurTask);
        return;
    }
    /* Once the title screen is gone (TaskDestroy leaves TaskMainDummy3), or takes A or Start (after 0x4A frames, as the USA one's
     * TitleScreenMain), the swarm is over, so the next screen gets its VRAM back; */
    if (swarm->title && (swarm->title->main != TitleScreenMain
                         || (++swarm->titleFrames > NIDL_TITLE_INPUT_FRAMES && gPressedKeys & (START_BUTTON | A_BUTTON)))) {
        TaskDestroy(gCurTask);
        return;
    }

    for (i = 0; i < NIDL_TITLE_MAX_KIRBYS; ++i) {
        struct NidlTitleKirby *kirby = &swarm->kirbys[i];
        s32 x, y;

        if (kirby->script == -1) continue;
        if (kirby->delay) --kirby->delay;
        else NidlTitleRun(kirby);

        kirby->vx += kirby->ax;
        kirby->vy += kirby->ay;
        kirby->x += kirby->vx;
        kirby->y += kirby->vy;
        x = kirby->x >> 8;
        y = kirby->y >> 8;
        if (kirby->script != -1) ++alive;
        if (kirby->frame == -1 || kirby->script == -1) continue;
        if ((u32)(x + 15) > 286 || y <= -32 || y > 191) {
            kirby->script = -1;
            continue;
        }
        kirby->screenX = x;
        kirby->screenY = y;
        ++shown;
    }

    /* Drawn after moving, each with its order (NiDL's: the lower on the screen it started, the nearer). The poses (55: too many
     * for the European title screen's 416 tiles of sprites to fit beside them) go through NIDL_TITLE_SLOTS slots of VRAM, at most
     * 36 being on the screen at once: a pose takes the slot drawn longest ago (none this frame), loaded through KatAM's VBlank DMA queue
     * (gUnk_03002EC0, as bg.c's), so it changes when this frame's sprites show. A Kirby whose pose finds no slot (or no room in
     * the queue) waits for the next frame; */
    for (i = 0; i < NIDL_TITLE_SLOTS; ++i) used[i] = FALSE;
    for (i = 0; i < NIDL_TITLE_MAX_KIRBYS; ++i) {
        struct NidlTitleKirby *kirby = &swarm->kirbys[i];
        u8 pose, slot;
        OamData *oam;

        if (kirby->script == -1 || kirby->frame == -1) continue;
        pose = gNidlTitle.framePoses[kirby->frame];
        slot = swarm->poseSlot[pose];
        if (slot == 0xFF) {
            if (uploads >= NIDL_TITLE_MAX_UPLOADS || ((gUnk_030039A4 + 1) & (NIDL_TITLE_DMA_QUEUE - 1)) == gUnk_03006078) continue;
            for (j = 0, slot = 0xFF; j < NIDL_TITLE_SLOTS; ++j) {
                if (!used[j] && (slot == 0xFF || swarm->slotDrawn[j] < swarm->slotDrawn[slot])) slot = j;
            }
            if (slot == 0xFF) continue;
            if (swarm->slotPose[slot] != 0xFF) swarm->poseSlot[swarm->slotPose[slot]] = 0xFF;
            swarm->slotPose[slot] = pose;
            swarm->poseSlot[pose] = slot;
            gUnk_03002EC0[gUnk_030039A4].unk0 = (u32)gNidlTitle.poseTiles + pose * NIDL_TITLE_POSE_BYTES;
            gUnk_03002EC0[gUnk_030039A4].unk4 = swarm->vram + slot * NIDL_TITLE_POSE_BYTES;
            gUnk_03002EC0[gUnk_030039A4].unk8 = NIDL_TITLE_POSE_BYTES;
            gUnk_030039A4 = (gUnk_030039A4 + 1) & (NIDL_TITLE_DMA_QUEUE - 1);
            ++uploads;
        }
        used[slot] = TRUE;
        swarm->slotDrawn[slot] = swarm->step + 1;
        oam = sub_08156D84(kirby->order);
        if ((void *)oam == iwram_end) break;
        oam->all.attr0 = (kirby->screenY - 16) & 0xFF;
        oam->all.attr1 = ((kirby->screenX - 16) & 0x1FF) | 0x8000 | gNidlTitle.frameFlips[kirby->frame];
        oam->all.attr2 = (tile + slot * 16) | NIDL_TITLE_PALETTE << 12;
    }

    if (swarm->step >= NIDL_TITLE_CROWD_STEP && shown >= NIDL_TITLE_CROWD_KIRBYS && !(swarm->step % 5))
        m4aSongNumStart(SE_INTRO_AMBIENT_2);
    if (++swarm->step == NIDL_TITLE_SHOW_STEP && !swarm->title) NidlTitleShow(swarm);
    if (!alive && swarm->title) TaskDestroy(gCurTask);
}
