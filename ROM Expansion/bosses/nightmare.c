/* Nightmare; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x43 (spawn 0x080AA288), as NiDL boss NIDL_NIGHTMARE,
 * the Fountain of Dreams' last (room 0x004 here). NiDL fights him on the ground with the Star Rod (Kirby walks, jumps and swings it,
 * shooting stars); so does every Kirby in his room here (abilities/star_rod.c; kept, finale.c's NidlFinaleKeep()).
 * His intro (NightmareIntroRider()), his fight (NightmareStart(): NiDL's, 1:1, see "His fight" below) and his defeat (NightmareEnd()),
 * each traced from NiDL frame by frame; */
#include "../abilities.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/songs.h"

static s32 ScreenX(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
}

static s32 ScreenY(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.y >> 8;
}

/* The intro (NightmareIntroRider()): the Power Orb's warp star brings the Kirbys in (its flight's step after the room change is
 * NIDL_RIDE_FINALE: warpstar.c holds them on it, this moves them), NiDL's, traced frame by frame (the star's and the camera's): the
 * screen from the room's top (34) down a pixel a frame for 158 frames, then a pixel every 4 frames down to its bottom (216, the
 * fight's screen; finale.c's NidlFinaleHoldCamera()), the star flying down into it with the Kirbys (sIntroStar: its moves on the
 * screen, from (0, -35) above its top left; hidden and kept on the screen till it shows, NightmareRideSize(): KatAM points at helpers
 * off it), then about it; 910 frames in, a blue star hits it (see below): the star's gone (NightmareRideSize(): NIDL_RIDE_ALONE,
 * warpstar.c draws the Kirbys without it), and the Kirbys are knocked off, held still (Knock(): NiDL's tumble up and down to the
 * floor, its bounce, then its slide back, fallen; NiDL's frames of Kirby with the Star Rod, tumbling, then his back to us, the rod
 * held out (STAR_ROD_MOTION_TUMBLE, _FALLEN); the helpers spreading out to the right of the player as they fly); 260 frames on they're up, the flight ends (its last steps: a frame standing,
 * then the end, facing him, KatAMExpansionRooms.cs) and they're the players' again; 16 frames before, his fight starts, his light
 * growing into his shape where he appears (its first appearance; till then he's held back, hidden: NightmareIntroHolds(),
 * nidl_boss.c); */
#define INTRO_CAM_X (8 << 8)
#define INTRO_CAM_Y (35 << 8)
#define INTRO_CAM_FAST 158
#define INTRO_CAM_END (216 << 8)
#define INTRO_STAR_X 0
#define INTRO_STAR_Y (-35 << 8)
#define INTRO_OFF (-16 << 8)
#define INTRO_ON_SCREEN (24 << 8)
#define INTRO_HIT 910
#define INTRO_LIGHT 1169       // (his fight starts: his light shows, NiDL's 1910);
#define INTRO_STANDS 1170
#define INTRO_ENDS 1184
#define NIDL_RIDE_FINALE 0xFC
#define NIDL_RIDE_HIDDEN 0x8000
#define NIDL_RIDE_ALONE 0x4000

void sub_080534D0(struct Kirby *);

// The star's moves on the screen frame by frame from the second (x + 8, (y + 8) << 4, pixels);
// (909 frames)
static const u8 sIntroStar[909] = {
    0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x88, 0x88,
    0x79, 0x88, 0x88, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0xA8, 0x98, 0x98,
    0x89, 0xA8, 0x98, 0x98, 0x89, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x88, 0x88,
    0x79, 0x88, 0x88, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0xA8, 0x98, 0x98,
    0x89, 0xA8, 0x98, 0x98, 0x89, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x89, 0x88,
    0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88,
    0x99, 0x88, 0x99, 0x88, 0x99, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x77, 0x88, 0x77,
    0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x87, 0x98, 0x87,
    0x98, 0x87, 0x98, 0x87, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x77, 0x88, 0x77,
    0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x87, 0x98, 0x87,
    0x98, 0x87, 0x98, 0x87, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xCC, 0xCC, 0xCC,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
    0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x48, 0x48, 0x48,
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C,
    0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,
    0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC4, 0xC4, 0xC4,
    0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x8C, 0x8C, 0x8C,
    0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8,
    0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x67, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x7A, 0x8A, 0x7A,
    0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
};

/* Its effects (NiDL's cutscene, frame by frame as the star's: t; KatAMExpansion.cs's frames after his beams, drawn by sprites of the
 * intro's own, IntroSprite()): the comet he comes as, far off in the sky (FRAME_COMET: the dark orb, then waking up, CometFrame();
 * drifting left, then off right, CometSpeed()), the screen flashing as it wakes (FlashLevel(): the room's palettes brighter, the
 * sprites' darker, Flash(); steady with "Reduce Flashing"), the blue stars it throws (FRAME_BLUE_STAR, sBlueStars: each its moves,
 * growing and spinning by an affine matrix, Spin(), blinking out at the end; all drawn from one sprite) and the bursts they make
 * (FRAME_BURST, sBursts: on their star); */
#define FRAME_COMET 27
#define FRAME_BLUE_STAR 33
#define FRAME_BURST 34
#define COMET_TILES 16
#define BLUE_STAR_TILES 64
#define BURST_TILES 48
#define COMET_X (207 << 8 | 0xA0)
#define COMET_Y (256 << 8)
#define COMET_GONE 596
#define BURST_FRAMES 23
#define INTRO_KEY_A 0x8000      // The comet's palettes' keys (| his anim ID: KatAM takes its low 12 bits);
#define INTRO_KEY_B 0xA000
#define INTRO_MATRIX 1          // The blue stars' affine matrices: 1-5 (KatAM's own use 0, the rides' 7, 15, 23, 31);
#define INTRO_MATRICES 5
#define FLASH_BG (14 * 16)      // The flash's colors: the room's (BG palettes 0-13: not the HUD's 14 and 15), then the sprites';
#define FLASH_COLORS (FLASH_BG + 0x100)
#define FRAME_PUFF 57           // The puff as the star's hit (0-16, NiDL's shared effect's), his light (NiDL's 67 down to 61, then 60);
#define FRAME_LIGHT 74
#define PUFF_TILES 40
#define LIGHT_TILES 121
#define INTRO_KEY_LIGHT 0xC000
#define ENTRANCE_X (113 << 8)   // Where he appears (NiDL's spot: the fight starts there);
#define ENTRANCE_Y (280 << 8)
#define ANIM_TUMBLING 74        // Kirby's tumble without the Star Rod (KatAM's: 74, then 89 down to 75, round again: NiDL's 96, then
                                // 111 down to 97; with it, NiDL's own: STAR_ROD_MOTION_TUMBLE);
#define ANIM_SLIDE 42           // Kirby's back without it (with it, NiDL's 0x1009: STAR_ROD_MOTION_FALLEN);
#define KNOCK_TUMBLE 154        // The tumble's and bounce's frames, then the slide's (to 258);
#define KNOCK_TURN 258
#define KNOCK_SPREAD 24         // The helpers' spread (pixels apart, over the tumble's first 64 frames);
#define KNOCK_SPREAD_TIME 64

// BlueStarLife; A blue star: from t `start`, at (x, y) on the screen (1/8 pixels, its middle), its size ((scale + 1) / 64) and turn
// (64ths), blinking out from `blink` frames on (hidden, shown, ...), gone after `end`; its moves: `runs` runs from `run`;
struct BlueStarLife {
    u16 start;
    s16 x, y;
    u8 scale, angle, blink, end, run, runs;
};

// BlueStarRun; `frames` frames of the same moves (1/8 pixels), size and turn steps (the size kept within 0-63);
struct BlueStarRun {
    u8 frames;
    s8 xs, ys, scale, angle;
};

// BlueStar; A blue star as it goes: where it is and was a frame ago (a burst follows it a frame behind), its size, turn, run and
// frames left of it;
struct BlueStar {
    s16 x, y, lastX, lastY;
    u8 scale, angle, run, left;
};

// The blue stars, traced from NiDL's (type 0x63 objects: their speeds and steps are its own; replayed and checked frame by frame);
// (12 lives)
static const struct BlueStarLife sBlueStars[12] = {
    { 562, 1312, 256, 8, 2, 21, 30, 0, 8 },
    { 613, 1952, 256, 8, 2, 21, 30, 8, 8 },
    { 614, 1568, 352, 5, 2, 56, 65, 16, 8 },
    { 638, 1872, 544, 62, 62, 36, 45, 24, 8 },
    { 638, 959, 928, 61, 62, 28, 37, 32, 6 },
    { 638, 1279, -224, 62, 2, 28, 37, 38, 5 },
    { 670, 1920, 640, 62, 62, 35, 68, 43, 8 },
    { 702, 1920, 96, 5, 2, 56, 65, 51, 14 },
    { 734, 1952, 648, 62, 62, 64, 63, 65, 8 },
    { 734, 2000, 448, 5, 34, 21, 30, 73, 8 },
    { 782, 1920, 288, 5, 34, 56, 65, 81, 14 },
    { 863, 1952, 16, 5, 6, 76, 93, 95, 6 },
};
// (101 runs)
static const struct BlueStarRun sBlueStarRuns[101] = {
    { 2, -32, -64, 5, 2 }, { 3, -32, -32, 5, 2 }, { 3, -32, -8, 5, 2 }, { 3, -32, -4, 5, 2 }, { 3, 0, 0, 0, 1 }, { 3, 0, 4, 0, 1 },
    { 3, 0, 8, 0, 1 }, { 10, -64, 16, 0, 1 }, { 2, -32, -64, 5, 2 }, { 3, -32, -32, 5, 2 }, { 3, -32, -8, 5, 2 }, { 3, -32, -4, 5, 2 },
    { 3, 0, 0, 0, 1 }, { 3, 0, 4, 0, 1 }, { 3, 0, 8, 0, 1 }, { 10, -64, 16, 0, 1 }, { 11, -32, 32, 2, 2 }, { 10, -32, 16, 2, 2 },
    { 8, -32, 8, 2, 2 }, { 8, -16, 4, 2, 2 }, { 8, -16, -4, 0, 1 }, { 6, -16, -8, 0, 1 }, { 4, -16, -16, 0, 1 }, { 10, -8, -32, 0, 1 },
    { 7, -48, 32, -1, -2 }, { 3, -32, 16, -1, -2 }, { 5, -48, -16, -1, -2 }, { 6, -32, -32, -1, -1 }, { 3, -48, -16, -1, -1 },
    { 6, -32, 16, -1, -1 }, { 5, -48, 32, -1, -1 }, { 10, -32, 8, -1, -1 }, { 11, -1, -32, -2, -2 }, { 4, -4, -16, -2, -1 },
    { 4, -4, 0, -2, -1 }, { 4, -8, 4, -2, -1 }, { 4, -8, 8, -2, -1 }, { 10, -8, 16, -2, -1 }, { 11, -1, 32, -1, 2 }, { 4, -4, 16, -1, 1 },
    { 4, -4, 0, -1, 1 }, { 4, -8, -4, -1, 1 }, { 14, -8, -8, -1, 1 }, { 7, -64, -64, -1, -2 }, { 7, -48, -32, -1, -2 },
    { 6, -32, -16, -1, -2 }, { 5, -24, -8, -1, -1 }, { 4, -16, -4, -1, -1 }, { 3, -10, 0, -1, -1 }, { 2, -6, 4, -1, -1 },
    { 34, -4, 8, 0, -1 }, { 3, -64, -32, 2, 2 }, { 4, -48, -16, 2, 2 }, { 4, -32, -8, 2, 2 }, { 4, -24, -4, 2, 2 }, { 4, -16, 0, 2, 2 },
    { 4, -10, 4, 2, 2 }, { 4, -8, 6, 2, 2 }, { 4, -6, 8, 2, 2 }, { 4, -4, 10, 0, 1 }, { 4, 0, 16, 0, 1 }, { 4, 4, 24, 0, 1 },
    { 4, 8, 32, 0, 1 }, { 4, 16, 32, 0, 1 }, { 14, 32, 32, 0, 1 }, { 7, -32, 8, -1, -2 }, { 12, -32, 4, -1, -2 }, { 4, -32, -4, -1, -2 },
    { 4, -32, -4, -1, -1 }, { 8, -32, -8, -1, -1 }, { 8, -32, -4, -1, -1 }, { 18, -32, 4, -1, -1 }, { 2, -32, 8, -1, -1 },
    { 2, -32, 64, 2, 2 }, { 3, -32, 32, 2, 2 }, { 3, -32, 8, 2, 2 }, { 3, -32, 4, 2, 2 }, { 3, 0, 0, 2, 2 }, { 3, 0, -4, 2, 2 },
    { 3, 0, -8, 2, 2 }, { 10, -64, -16, 2, 2 }, { 3, -64, 32, 2, 2 }, { 4, -48, 16, 2, 2 }, { 4, -32, 8, 2, 2 }, { 4, -24, 4, 2, 2 },
    { 4, -16, 0, 2, 2 }, { 4, -10, -4, 2, 2 }, { 4, -8, -6, 2, 2 }, { 4, -6, -8, 2, 2 }, { 4, -4, -10, 2, 2 }, { 4, 0, -16, 2, 2 },
    { 4, 4, -24, 2, 2 }, { 4, 8, -32, 2, 2 }, { 4, 16, -32, 2, 2 }, { 14, 32, -32, 2, 2 }, { 15, -32, 16, 2, 2 }, { 28, -32, 8, 2, 1 },
    { 8, 1, 8, 0, 1 }, { 8, 4, 8, 0, 1 }, { 16, 8, 8, 0, 1 }, { 18, 16, 16, 0, 1 },
};

// The bursts (NiDL's type 0x97): on a blue star (sBlueStars), at its age then;
static const u8 sBursts[][2] = { { 4, 12 }, { 5, 12 }, { 3, 16 }, { 6, 21 }, { 8, 24 }, { 11, 44 } };

// The tumble off the star and the bounce: runs of moves (frames, x and y a frame in 1/32 pixels), NiDL's Kirby's;
static const s8 sKnock[][3] = {
    { 12, 32, -128 }, { 4, 32, -64 }, { 4, 32, -32 }, { 4, 16, -16 }, { 4, 16, -4 }, { 4, 16, -1 }, { 4, 16, 1 }, { 4, 16, 4 },
    { 8, 16, 16 }, { 16, 16, 32 }, { 50, 16, 64 }, { 8, 0, -64 }, { 12, 0, -32 }, { 12, 0, 32 }, { 8, 0, 64 },
};

// A quarter turn's sines (64ths of a turn; 8.8);
static const u16 sQuarterSine[17] = { 0, 25, 50, 74, 98, 121, 142, 162, 181, 198, 213, 226, 237, 245, 251, 255, 256 };

// (calm.c's detours: filled in only with "Reduce Flashing" on);
struct NidlCalm {
    void (*updateScreenDma)(void);
    void (*updateScreenCpuSet)(void);
};
extern const struct NidlCalm gNidlCalm;

struct NightmareIntro {
    struct EffectObject e;      // (its sprite: the comet's);
    s32 camY;                   // The screen's top (8.8, the room's);
    s32 sx, sy;                 // The star on the screen (8.8, its middle: Kirby's spot on it);
    s32 cometX;                 // The comet's x (8.8, the room's);
    u16 t;                      // Frames since it began;
    u8 palA, palB, burstPal;    // The comet's palettes' slots (the blue star's is the first), the bursts' (0: not loaded yet);
    u8 level;                   // The flash on screen (0: none);
    u8 lightPal;                // His light's palette's slot (0: not loaded yet; his fight's, loaded first here);
    s32 knockX, knockY;         // The knocked-off player's spot (8.8, the room's; the helpers' to his right), the star's then;
    s32 hitX, hitY;
    struct EffectObject *stars; // The blue stars' sprite, the bursts', the puff's (NULL: none);
    struct EffectObject *bursts[ARRAY_COUNT(sBursts)];
    struct EffectObject *puff;
    struct BlueStar blue[ARRAY_COUNT(sBlueStars)];
    u16 colors[FLASH_COLORS];   // The palettes under the flash, the game's own;
    u16 flashed[2][FLASH_COLORS];   // Them at two flash levels (flashLevels: 0 none), made once, the flicker taking turns (Flash());
    u8 flashLevels[2];
    u16 flashMade[2];           // How many of their colors are made (FlashAhead(): a quarter a frame, ahead of when they show);
    u8 flashSlot;               // The one on screen (when level isn't 0);
};

static s32 Sine(u8 angle) {
    angle &= 63;
    if (angle < 16) return sQuarterSine[angle];
    if (angle < 32) return sQuarterSine[32 - angle];
    if (angle < 48) return -(s32)sQuarterSine[angle - 32];
    return -(s32)sQuarterSine[64 - angle];
}

// CometSpeed(); The comet's move at t (8.8, right): drifting left, slower, then off right;
static s32 CometSpeed(u16 t) {
    if (t <= 433) return -0x20;
    if (t <= 561) return -0x8;
    return t <= 577 ? 0x200 : 0x400;
}

// CometFrame(); Its frame at t: the dark orb, then each of the next five from every 36 frames on, two frames at a time with the one
// before it;
static u8 CometFrame(u16 t) {
    u8 k;

    if (t < 254) return 0;
    if (t >= 422) return 5;
    if (t >= 416) return 4;
    k = (t - 254) / 36 + 1;
    return (t - 254) & 2 ? k - 1 : k;
}

// FlashLevel(); The flash at t (0-3): brighter three times, each two frames on and two at the one below, then held and fading;
static u8 FlashLevel(u16 t) {
    u16 from;
    u8 high;

    if (t < 291 || t >= 423) return 0;
    if (t >= 419) return 1;
    if (t >= 415) return 2;
    if (t >= 397) return 3;
    if (t >= 361 && t < 365) return 2;
    if (t >= 365) {
        from = 365;
        high = 3;
    } else if (t >= 329) {
        from = 329;
        high = 2;
    } else {
        from = 291;
        high = 1;
    }
    // (with "Reduce Flashing", the one below, without the flicker);
    return gNidlCalm.updateScreenDma != NULL || (t - from) & 2 ? high - 1 : high;
}

/* FlashMap(); A flash level's channels (0-31): the room's toward white by 3/8, 3/4 or all the way, the sprites' toward black by 1/4,
 * 1/2 or all the way (level 0: as they are); */
static void FlashMap(u8 level, u8 *room, u8 *sprites) {
    u8 c;

    for (c = 0; c < 32; c++) {
        room[c] = level == 3 ? 31 : c + (((31 - c) * level * 3) >> 3);
        sprites[c] = level == 3 ? 0 : (c * (4 - level)) >> 2;
    }
}

#define FLASHED(color, map) ((map)[(color) & 0x1F] | (map)[((color) >> 5) & 0x1F] << 5 | (map)[((color) >> 10) & 0x1F] << 10)

// FlashMake(); A slot's level's colors made, from the made ones on, `count` of them (the game's own at that level);
static void FlashMake(struct NightmareIntro *intro, u8 slot, u16 count) {
    u8 room[32], sprites[32];
    u16 i, end = intro->flashMade[slot] + count;

    if (end > FLASH_COLORS) end = FLASH_COLORS;
    FlashMap(intro->flashLevels[slot], room, sprites);
    for (i = intro->flashMade[slot]; i < end; i++)
        intro->flashed[slot][i] = i >= FLASH_BG ? FLASHED(intro->colors[i], sprites) : FLASHED(intro->colors[i], room);
    intro->flashMade[slot] = end;
}

/* FlashAhead(); At t, the next level the flash shows within 4 frames made ahead, a quarter of its colors a frame, in a slot whose level
 * doesn't show till then (see Flash()); */
static void FlashAhead(struct NightmareIntro *intro, u16 t) {
    u8 level = FlashLevel(t + 4), slot, k;

    if (!level || gKirbys[gLocalPlayerId].base.roomId != intro->e.roomId) return;
    slot = intro->flashLevels[0] == level ? 0 : intro->flashLevels[1] == level ? 1 : 2;
    if (slot == 2) {
        for (slot = 0; slot < 2; slot++) {
            if (!intro->flashLevels[slot]) break;
            for (k = 0; k < 4 && FlashLevel(t + k) != intro->flashLevels[slot]; k++)
                ;
            if (k >= 4 && (!intro->level || slot != intro->flashSlot)) break;
        }
        if (slot >= 2) return;
        intro->flashLevels[slot] = level;
        intro->flashMade[slot] = 0;
        // (the game's own colors, as they are with no flash on: what it's made from);
        if (!intro->level) {
            CpuCopy16(gBgPalette, intro->colors, FLASH_BG * sizeof(u16));
            CpuCopy16(gObjPalette, intro->colors + FLASH_BG, (FLASH_COLORS - FLASH_BG) * sizeof(u16));
        }
    }
    if (intro->flashMade[slot] < FLASH_COLORS) FlashMake(intro, slot, FLASH_COLORS / 4);
}

/* Flash(); The palettes at a flash's level (0: as they are), as it changes, the game's own kept under it (a color that isn't what the
 * flash made of it is the game's, new: kept, and made again at the levels kept); only on the screen of a player in the room. Each
 * level's palettes are made once, ahead (FlashAhead()), and kept (two: the flicker takes turns between two), copied in as it changes
 * (made every 2 frames, the 480 colors took 105k cycles, and with KatAM's own every 11 frames the intro lagged then); */
static void Flash(struct NightmareIntro *intro, u8 level) {
    u8 room[2][32], sprites[2][32];
    u16 i, *color, *shown;
    u8 slot, k;

    if (gKirbys[gLocalPlayerId].base.roomId != intro->e.roomId) {
        intro->level = 0;
        return;
    }
    if (level == intro->level) return;
    for (k = 0; k < 2; k++)
        if (intro->flashLevels[k]) FlashMap(intro->flashLevels[k], room[k], sprites[k]);
    // (the game's own changes since: kept, and made at the levels kept);
    shown = intro->level ? intro->flashed[intro->flashSlot] : intro->colors;
    for (i = 0; i < FLASH_COLORS; i++) {
        color = i >= FLASH_BG ? &gObjPalette[i - FLASH_BG] : &gBgPalette[i];
        if (*color == shown[i]) continue;
        intro->colors[i] = *color;
        for (k = 0; k < 2; k++)
            if (intro->flashLevels[k] && i < intro->flashMade[k])
                intro->flashed[k][i] = i >= FLASH_BG ? FLASHED(*color, sprites[k]) : FLASHED(*color, room[k]);
    }
    if (level) {
        // (its palettes, made ahead, or now: in the slot not on screen);
        slot = intro->flashLevels[0] == level ? 0 : intro->flashLevels[1] == level ? 1 : 2;
        if (slot == 2) {
            slot = intro->level && intro->flashSlot == 0 ? 1 : 0;
            intro->flashLevels[slot] = level;
            intro->flashMade[slot] = 0;
        }
        if (intro->flashMade[slot] < FLASH_COLORS) FlashMake(intro, slot, FLASH_COLORS);
        intro->flashSlot = slot;
        shown = intro->flashed[slot];
    } else {
        shown = intro->colors;
    }
    CpuCopy16(shown, gBgPalette, FLASH_BG * sizeof(u16));
    CpuCopy16(shown + FLASH_BG, gObjPalette, (FLASH_COLORS - FLASH_BG) * sizeof(u16));
    intro->level = level;
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE | MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

// IntroPalette(); A palette's slot, loaded under `key` as the frame `frame` of the anim in its low 12 bits has it (as
// NidlLoadPalette()), or 0 (not yet: the local player isn't in the room);
static u8 IntroPalette(u16 room, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

static void IntroSpriteMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);

    if (e->flags & 0x1000) TaskDestroy(gCurTask);
}

// IntroSprite(); A sprite of the intro's (`tiles` tiles of VRAM, frame `frame` first): a task of its own (KatAM frees its VRAM with it),
// drawn by the intro (IntroDraw()), gone with it (its flag 0x1000);
static struct EffectObject *IntroSprite(u16 room, u16 tiles, u8 frame) {
    struct Task *t = TaskCreate(IntroSpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = room;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_NIGHTMARE].animId, frame, 0x1A);
    return e;
}

// IntroDraw(); A sprite of the intro's at (x, y) (8.8, the room's), frame `frame`, palette `palId`;
static void IntroDraw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    e->flags = (e->flags & ~0x400) | 4 | 1;
    EffectObjectPostUpdate(e);
}

// Release(); A sprite of the intro's let go (its task ends);
static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

/* Spin(); The OAM pieces drawn since `first` (a blue star's 64x64 one, around its middle) turned and sized by affine matrix `matrix`,
 * as NiDL's: turn `angle` (64ths), size (scale + 1) / 64 (cos, -sin, sin, cos over the size; a flipped piece's row negated); */
static void Spin(u32 first, u8 angle, u8 scale, u32 matrix) {
    s32 c = Sine(angle + 16) * 64 / (scale + 1), s = Sine(angle) * 64 / (scale + 1);
    u16 *m = &gOamBuffer[matrix * 16 + 3];
    u32 attr1 = 0;

    for (; first < gUnk_030024F0; first++) {
        OamData *oam = &gUnk_030031C0[first];

        attr1 = oam->all.attr1;
        oam->all.attr0 |= 0x100;
        oam->all.attr1 = (attr1 & ~0x3E00) | (matrix << 9);
    }
    m[0] = attr1 & 0x1000 ? -c : c;
    m[4] = attr1 & 0x1000 ? s : -s;
    m[8] = attr1 & 0x2000 ? -s : s;
    m[12] = attr1 & 0x2000 ? -c : c;
}

// Comet(); The comet at t (shown till it's off the screen's right);
static void Comet(struct NightmareIntro *intro, u16 t) {
    u8 frame;

    if (t) intro->cometX += CometSpeed(t);
    if (t > COMET_GONE) return;
    frame = CometFrame(t);
    IntroDraw(&intro->e, intro->cometX, COMET_Y, FRAME_COMET + frame, frame < 3 ? intro->palA : intro->palB);
}

// BlueStars(); The blue stars at t: each moved on and drawn (from the one sprite, a matrix each);
static void BlueStars(struct NightmareIntro *intro, u16 t) {
    u8 i, drawn = 0;
    bool8 over = TRUE;

    for (i = 0; i < ARRAY_COUNT(sBlueStars); i++) {
        const struct BlueStarLife *life = &sBlueStars[i];
        struct BlueStar *b = &intro->blue[i];
        const struct BlueStarRun *run;
        s32 scale;
        u16 age;
        u32 first;

        if (t <= life->start + life->end) over = FALSE;
        if (t < life->start || t > life->start + life->end) continue;
        age = t - life->start;
        if (!age) {
            b->x = b->lastX = life->x;
            b->y = b->lastY = life->y;
            b->scale = life->scale;
            b->angle = life->angle;
            b->run = life->run;
            b->left = sBlueStarRuns[b->run].frames;
        } else {
            run = &sBlueStarRuns[b->run];
            b->lastX = b->x;
            b->lastY = b->y;
            b->x += run->xs;
            b->y += run->ys;
            scale = b->scale + run->scale;
            b->scale = scale < 0 ? 0 : scale > 63 ? 63 : scale;
            b->angle = (b->angle + run->angle) & 63;
            if (!--b->left && b->run + 1 < life->run + life->runs) b->left = sBlueStarRuns[++b->run].frames;
        }
        if ((age >= life->blink && !((age - life->blink) & 1)) || drawn >= INTRO_MATRICES) continue;
        if (intro->stars == NULL) intro->stars = IntroSprite(intro->e.roomId, BLUE_STAR_TILES, FRAME_BLUE_STAR);
        first = gUnk_030024F0;
        IntroDraw(intro->stars, INTRO_CAM_X + (b->x << 5), intro->camY + (b->y << 5), FRAME_BLUE_STAR, intro->palA);
        Spin(first, b->angle, b->scale, INTRO_MATRIX + drawn++);
    }
    // (its sprite let go once they're all gone);
    if (over) Release(&intro->stars);
}

// Bursts(); The bursts at t: on their star (a frame behind it from their second), a frame of the burst, then one of its sparkles,
// each in turn;
static void Bursts(struct NightmareIntro *intro, u16 t) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sBursts); i++) {
        const struct BlueStar *b = &intro->blue[sBursts[i][0]];
        u16 from = sBlueStars[sBursts[i][0]].start + sBursts[i][1], k;

        if (t == from + BURST_FRAMES) Release(&intro->bursts[i]);
        if (t < from || t >= from + BURST_FRAMES) continue;
        k = t - from;
        if (intro->bursts[i] == NULL) intro->bursts[i] = IntroSprite(intro->e.roomId, BURST_TILES, FRAME_BURST);
        IntroDraw(intro->bursts[i], INTRO_CAM_X + ((k ? b->lastX : b->x) << 5), intro->camY + ((k ? b->lastY : b->y) << 5),
                  FRAME_BURST + (k & 1 ? 12 + k / 2 : k / 2), intro->burstPal);
    }
}

/* Knock(); The knocked-off player's spot at t (from INTRO_HIT on, the star's spot then): NiDL's tumble up and down to the floor, the
 * bounce, then the slide back (a pixel right, a pixel right, two left, two left, ...) till he turns round; */
static void Knock(struct NightmareIntro *intro, u16 t) {
    u16 k = t - INTRO_HIT, n = 0;
    u8 r;

    if (!k) {
        intro->knockX = intro->hitX = INTRO_CAM_X + intro->sx;
        intro->knockY = intro->hitY = intro->camY + intro->sy;
    }
    if (k < KNOCK_TUMBLE) {
        for (r = 0; k >= n + sKnock[r][0]; r++) n += sKnock[r][0];
        intro->knockX += sKnock[r][1] << 3;
        intro->knockY += sKnock[r][2] << 3;
    } else if (k < KNOCK_TURN) {
        intro->knockX += (k - KNOCK_TUMBLE) & 2 ? -0x200 : 0x100;
    }
}

// Puff(); The puff where the star's hit (NiDL's: its burst and stars, two frames on its first, then out a while, then its rings);
static void Puff(struct NightmareIntro *intro, u16 t) {
    u16 a = t - INTRO_HIT;

    if (t < INTRO_HIT) return;
    if (a >= 22) {
        Release(&intro->puff);
        return;
    }
    if (a >= 12 && a < 16) return;
    if (intro->puff == NULL) intro->puff = IntroSprite(intro->e.roomId, PUFF_TILES, FRAME_PUFF);
    IntroDraw(intro->puff, intro->hitX, intro->hitY, FRAME_PUFF + (a < 2 ? 0 : a < 12 ? a - 1 : a - 5), intro->burstPal);
}

// IntroEnd(); Its effects gone, the palettes back;
static void IntroEnd(struct NightmareIntro *intro) {
    u8 i;

    Flash(intro, 0);
    Release(&intro->stars);
    for (i = 0; i < ARRAY_COUNT(sBursts); i++) Release(&intro->bursts[i]);
    Release(&intro->puff);
}

static void NightmareIntroMain(void);

// IntroOf(); Nightmare's intro in a room (its task), or NULL;
static struct NightmareIntro *IntroOf(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct NightmareIntro *intro;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != NightmareIntroMain) continue;
        intro = TaskGetStructPtr(t);
        if (intro->e.roomId == room && !(intro->e.flags & 0x1000)) return intro;
    }
    return NULL;
}

// NightmareIntroOn(); Whether his intro runs in a room, or a Kirby rides a warp star in there (it's coming: nidl_boss.c, he waits,
// hidden);
bool32 NightmareIntroOn(u16 room) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == room && gKirbys[i].stateFn == sub_080534D0) return TRUE;
    return IntroOf(room) != NULL;
}

// NightmareIntroHolds(); Whether his intro holds him back in a room (hidden, nidl_boss.c): till his fight starts, his light showing
// (INTRO_LIGHT), or while a Kirby rides the star in before it;
bool32 NightmareIntroHolds(u16 room) {
    struct NightmareIntro *intro = IntroOf(room);
    u8 i;

    if (intro != NULL) return intro->t < INTRO_LIGHT;
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == room && gKirbys[i].stateFn == sub_080534D0) return TRUE;
    return FALSE;
}

// NightmareRideSize(); The size the star and its Kirbys are drawn at (finale.c's NidlFinaleRideSize()): theirs, hidden over the
// screen, or the Kirbys alone (the star's hit);
u32 NightmareRideSize(u16 room) {
    struct NightmareIntro *intro = IntroOf(room);

    if (intro == NULL) return 0;
    if (intro->t >= INTRO_HIT) return NIDL_RIDE_ALONE;
    return intro->sy <= INTRO_OFF ? NIDL_RIDE_HIDDEN : 0;
}

// IntroRider(); Whether a Kirby rides the star in (on its flight's step held: warpstar.c);
static bool32 IntroRider(struct Kirby *kirby, u16 room) {
    const struct Unk_08353510 *step = kirby->unk114;

    return kirby->base.roomId == room && kirby->stateFn == sub_080534D0 && step != NULL && step->unk8 == NIDL_RIDE_FINALE && !(step->unk9 & 0x40);
}

static void NightmareIntroMain(void) {
    struct NightmareIntro *intro = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &intro->e;
    u16 t = intro->t;
    u8 i;

    if (e->flags & 0x1000) {
        IntroEnd(intro);
        TaskDestroy(gCurTask);
        return;
    }
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys || t > INTRO_ENDS) {
        IntroEnd(intro);
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    if (t) {
        u8 code = sIntroStar[t - 1 < ARRAY_COUNT(sIntroStar) ? t - 1 : ARRAY_COUNT(sIntroStar) - 1];

        if (t - 1 >= ARRAY_COUNT(sIntroStar)) code = 0x88;
        if (t <= INTRO_CAM_FAST || ((t - INTRO_CAM_FAST) & 3) == 3) intro->camY += 0x100;
        if (intro->camY > INTRO_CAM_END) intro->camY = INTRO_CAM_END;
        intro->sx += ((code & 0xF) - 8) << 8;
        intro->sy += ((code >> 4) - 8) << 8;
    }
    NidlFinaleHoldCamera(e->roomId, INTRO_CAM_X, intro->camY);
    if (t >= INTRO_HIT) Knock(intro, t);
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];
        u16 k = t - INTRO_HIT;

        if (!IntroRider(kirby, e->roomId)) continue;
        if (t < INTRO_HIT) {
            kirby->base.x = INTRO_CAM_X + intro->sx;
            kirby->base.y = intro->camY + (intro->sy <= INTRO_OFF ? INTRO_ON_SCREEN : intro->sy);
            continue;
        }
        // (knocked off: the player's spot, the helpers' to his right; tumbling, then fallen, his back to us, facing right as NiDL's:
        // with the Star Rod, NiDL's frames of it, KatAMExpansionMotions.cs, its tumble a frame a step from where NiDL's starts);
        kirby->base.x = intro->knockX + i * KNOCK_SPREAD * (k < KNOCK_SPREAD_TIME ? k : KNOCK_SPREAD_TIME) * 0x100 / KNOCK_SPREAD_TIME;
        kirby->base.y = intro->knockY;
        if (kirby->ability == NIDL_ABILITY_STAR_ROD) NidlMotion(kirby, k < KNOCK_TUMBLE ? STAR_ROD_MOTION_TUMBLE : STAR_ROD_MOTION_FALLEN);
        else kirby->animationIndex = k < KNOCK_TUMBLE ? ANIM_TUMBLING + ((13 - k) & 15) : ANIM_SLIDE;
        kirby->base.flags &= ~1;
        // (up: the flight's end, a frame standing, then theirs again);
        if (t == INTRO_STANDS) {
            kirby->unk114++;
            kirby->base.counter = ((const struct Unk_08353510 *)kirby->unk114)->unk4;
        }
    }
    // (its effects; their palettes once a player's in the room);
    if (!intro->palA) intro->palA = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_A, FRAME_COMET);
    if (!intro->palB) intro->palB = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_B, FRAME_COMET + 3);
    if (!intro->burstPal) intro->burstPal = IntroPalette(e->roomId, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0);
    if (!intro->lightPal) intro->lightPal = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_LIGHT, FRAME_LIGHT);
    Comet(intro, t);
    FlashAhead(intro, t);
    Flash(intro, FlashLevel(t));
    BlueStars(intro, t);
    Bursts(intro, t);
    Puff(intro, t);
    intro->t++;
}

// NightmareIntroRider(); A Kirby the star brings into his room, held on it (finale.c's NidlFinaleHold()): the intro, once;
void NightmareIntroRider(struct Kirby *kirby) {
    struct Task *t;
    struct NightmareIntro *intro;
    u8 i;

    if (IntroOf(kirby->base.roomId) != NULL) return;
    t = TaskCreate(NightmareIntroMain, sizeof(struct NightmareIntro), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    intro = TaskGetStructPtr(t);
    ClearEffectObject(&intro->e);
    intro->e.header.kind = 3;
    intro->e.roomId = kirby->base.roomId;
    intro->e.parent = NULL;
    intro->e.flags |= 0x400;
    intro->camY = INTRO_CAM_Y;
    intro->sx = INTRO_STAR_X;
    intro->sy = INTRO_STAR_Y;
    intro->t = 0;
    intro->cometX = COMET_X;
    intro->palA = intro->palB = intro->burstPal = intro->lightPal = 0;
    intro->level = 0;
    intro->flashLevels[0] = intro->flashLevels[1] = 0;
    intro->flashMade[0] = intro->flashMade[1] = 0;
    intro->flashSlot = 0;
    intro->knockX = intro->knockY = intro->hitX = intro->hitY = 0;
    intro->stars = intro->puff = NULL;
    for (i = 0; i < ARRAY_COUNT(sBursts); i++) intro->bursts[i] = NULL;
    EffectObjectInitSprite(&intro->e, &intro->e.sprite, COMET_TILES, gNidlData.bosses[NIDL_NIGHTMARE].animId, FRAME_COMET, 0x1A);
}


/* His fight (NightmareStart()); NiDL's, from its code: his object (0x080AA338: 12 states, each a script, 0x087493F8, and a function
 * every frame after it, 0x08749428; then his boxes, 0x080AA3A8), his companion (type 0xD0, 0x080AC511: his touch), his shots (type 0x88,
 * 0x080ACC9C) and his effects (types 0xC9-0xCF), checked frame by frame against NiDL's (its states, frames and spots: the same random
 * draws give the same fight). The scripts run as NiDL's (WAIT(): resumed that many frames on; a state set starts the frame after),
 * then each frame his move (NiDL's: his speed plus its change, capped, then his spot plus his speed; 8.8 here, NiDL's 16.16 >> 8: all
 * its values are whole there), then the state's frame function (his frame sequence stepped; state 1's countdown, state 8's sound).
 * Positions are the room's, NiDL's (the rooms are NiDL's, KatAMExpansionRooms.cs); the screen is KatAM's camera, NiDL's there.
 * His moves come in NiDL's order (sMoves; each after he reappears somewhere new: Teleport(), sSpot*): 0 the cape (state 5: up 32
 * pixels, slowing, then his cape thrown open and a fan of five stars, in two orders by turns), 1 and 2 the dash (state 4: back to the
 * screen's edge on Kirby's side, then across it at 5.5 pixels a frame if Kirby is within 111 pixels; then 1 the hand (state 6: down
 * to Kirby's height, then 4 stars from his hand, 6 under a third of his HP, each a fifth of a turn on from the last at random) or 2
 * the charge (state 7: his hand raised, its sparkles growing, then 6 stars pointed down at Kirby, two at a time on every other one
 * under a third of his HP)), 3 the tornado (state 8: down at Kirby, steering after him; under a third of his HP up off the screen and
 * down through it at 8 pixels a frame) and 4 the swoop (state 9: down and back up across the screen). Between them he hovers (state
 * 1: 15 to 60 frames, NiDL's draws), vanishes (state 3) and reappears (state 2), every fourth time three times quickly (state 10).
 * KatAM's damage, NiDL's numbers: 60 HP, a star 10 (unk94: KatAM's strength 1 + 9), and only in his frames 4-35 (his cape thrown
 * open, his tornado, his hand out: NiDL's 0x08749380), within that frame's box (sBodyBoxes); else no box (flag 0x200); hit, he faces
 * the Kirby whose star it was (NiDL's faces its Kirby) and is knocked back (state 11, NiDL's 0x080AB854). The Star Rod's stars burst on his box whatever (NightmareBlocks():
 * NiDL keeps it, even while he's gone, abilities/star_rod.c); his touch hurts (NiDL's companion: sTouches, two attack objects).
 * His graphics (KatAMExpansionNightmare.cs, gNidlNightmareFight): NiDL draws his body frame and his top half over it with four
 * palettes (his collar, his tornado: C and D by turns every two frames, his cape: A and B blended to and fro over 64 frames, his
 * light); KatAM's sprites have one: three sprites (sParts: collar, tornado, cape or light), their pieces cut down to what NiDL shows of
 * each, and his palettes written each frame (Colors(); his tornado's held on C with "Reduce Flashing"). His shots and effects are
 * NiDL's frames too (0x08754738's, his descriptor's second palette; the sparkles' in his tornado's). His stars, as NiDL's, stop where
 * they hit the floor and burst into a puff there (0x0806951C, 0x0806A344), and go 60 pixels past the screen's edges (KatAM's own cut
 * at 168 from its middle is off for them: flag 0x2000000). His object's own sprite (type 0x44's, hidden) gives its VRAM back (unkC
 * 0x200 keeps KatAM from taking more), which his sprites need with four Kirbys in the room.
 * NiDL's sounds are its own: KatAM's closest play (Dark Mind's: his teleports, his stars, his charge). Not ported: a shot NiDL keeps
 * 255 frames homes in on Kirby (0x080645A4): none lives that long in his room (the floor or the screen's edge takes them first); */
struct NidlNightmareFight {
    u16 animId;
    u8 shots, sparkles;     // The first frames of NiDL's 0x08754738 (0-12) and of the sparkles (16);
    u8 frames[76][3];       // Per NiDL frame, its collar's, tornado's and cape's (or light's) frames, FIGHT_NONE: none;
    u16 colors[4][16];      // NiDL's A and B (his cape's, blended), C and D (his tornado's, by turns);
    u8 tiles[256];          // Each frame's tiles;
};

const struct NidlNightmareFight gNidlNightmareFight = { 0 };

#define FIGHT_NONE 0xFF
#define PARTS 3
#define PART_COLLAR 0
#define PART_TWINKLE 1
#define PART_CAPE 2
#define FRAME_HIDDEN -1
#define LIGHT_FIRST 60              // His light (appearing, vanishing), drawn in his frame's own palette;
#define LIGHT_LAST 67
#define FIGHT_KEY_TWINKLE 0x2000    // His palettes' keys (| the fight anim's ID), but his collar's (his anim's) and light's (his intro's);
#define FIGHT_KEY_BLEND 0x4000
#define FIGHT_KEY_FX 0x6000
#define FIGHT_SPARKLES_C 4          // (the sparkles: 0x087546F8's from 0, 0x08754708's from 4, 0x08754718's from 8);
#define FIGHT_SPARKLES_E 8
#define NO_MAX 0x7FFF
#define PC_END 0xFFFF
#define HIT_KNOCKED 1               // NiDL's 0x02007D00;
#define HIT_DYING 2
#define MAX_HP 60
#define STAR_STRENGTH 9             // (+ KatAM's star's 1: NiDL's 10);
#define FRONT_ORDER 0x18            // Sprite priorities (NiDL's: his effects 7-9 in front of him, 11, the hand's sparkles 12 behind);
#define SPARKLE_ORDER 0x19
#define BODY_ORDER 0x1A
#define BACK_ORDER 0x1C
#define SHOT_HALF 6
#define SHOT_BOTTOM 7               // (its terrain box's bottom: it stops on the floor 7 pixels above it);
#define SHOT_LIFE 510               // (255 frames, then 255 homing: none lives that long, all leave the screen first);
#define SHOT_LANDED -1              // (its counter: on the floor, its puff from the next frame);
#define OFF_SCREEN 60               // (NiDL's objects go 60 pixels off the screen);
#define PUFF_LAST 21                // (its puff: as the intro's, Puff());

enum { PAL_COLLAR, PAL_TWINKLE, PAL_BLEND, PAL_LIGHT, PAL_FX, PAL_PUFF, PALS };
enum { BOX_NONE = 0xFF, BOX_KNOCK = 4, BOX_DIVE = 5 };
enum { SUB_APPEAR, SUB_VANISH, SUB_FRAMES, SUB_KNOCK };
enum { FX_CAPE_GLOW, FX_CAPE_STARS, FX_CAPE_SPARKLES, FX_HAND_GLOW, FX_HAND_SPARKLES, FX_CHARGE, FX_CHARGE_SPARKLES, FX_PUFF };

// NiDL's tables (0x08749220 on): his hover's waits, his moves in order and their states, where he reappears for each (x from the
// screen's middle, y from its top, both give or take half a range: NiDL's draws), his hand's stars' waits and counts (under a third of
// his HP, then over);
static const u8 sWaits[4] = { 15, 30, 45, 60 };
static const u8 sMoves[10] = { 0, 1, 3, 2, 4, 0, 3, 1, 4, 2 };
static const u8 sMoveStates[5] = { 5, 4, 4, 8, 9 };
static const u8 sSpotXRange[6] = { 0x60, 0x90, 0x90, 0x60, 0, 0x90 };
static const u8 sSpotY[6] = { 0x40, 0x30, 0x38, 0x34, 0x30, 0x30 };
static const u8 sSpotYRange[6] = { 0, 8, 8, 0, 0, 0x20 };
static const u8 sShotWaits[2] = { 10, 18 };
static const u8 sShotCounts[2] = { 6, 4 };

// His frame sequences (NiDL's: frame, frames shown; NIDL_LOOP);
static const s8 sCaped[] = { 48, 8, 49, 8, 50, 8, 51, 8, NIDL_LOOP };
static const s8 sDash[] = { 40, 10, 41, 10, 42, 10, 43, 10, NIDL_LOOP };
static const s8 sCapeOpen[] = { 4, 8, 5, 8, 6, 8, 7, 8, NIDL_LOOP };
static const s8 sHandOut[] = { 24, 8, 25, 8, 26, 8, 27, 8, NIDL_LOOP };
static const s8 sHandUp[] = { 28, 8, 29, 8, 30, 8, 31, 8, NIDL_LOOP };
static const s8 sPointing[] = { 32, 8, 33, 8, 34, 8, 35, 8, NIDL_LOOP };
static const s8 sTornado[] = { 8, 1, 9, 1, 10, 1, 11, 1, 12, 1, 13, 1, 14, 1, 15, 1, NIDL_LOOP };
static const s8 sDive[] = { 16, 1, 17, 1, 18, 1, 19, 1, 20, 1, 21, 1, 22, 1, 23, 1, NIDL_LOOP };
static const s8 sHurt[] = { 68, 3, 69, 3, 70, 3, 71, 3, NIDL_LOOP };
static const s8 sHurtDiving[] = { 72, 3, 73, 3, 74, 3, 75, 3, NIDL_LOOP };

/* His boxes (NiDL's: left, top, right, bottom, facing right; mirrored facing left): his body's in his frames 0-35 (0x08749380, NiDL's
 * 0x08749358: none in 0-3), knocked back and knocked back from his dive (0x08749870, 0x0874988C); his touch's (his companion's,
 * 0x087494C8: per frame two, 0x08749458 and 0x08749490, -1 none); */
static const s8 sBodyBoxes[6][4] = {
    { -12, 0, 12, 34 }, { -12, -34, 12, 0 }, { -24, 16, -16, 48 }, { -12, 28, 4, 52 }, { -16, -16, 16, 32 }, { -16, -32, 16, 16 },
};
static const u8 sBodyBoxOf[36] = {
    BOX_NONE, BOX_NONE, BOX_NONE, BOX_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 3, 3,
};
static const s8 sTouches[14][2][4] = {
    { { -28, -32, 28, 16 }, { -16, 16, 16, 48 } }, { { -40, -32, 24, 16 }, { -24, 16, 8, 48 } }, { { -20, -8, 8, 48 }, { 8, -32, 32, 24 } },
    { { -32, -32, 0, 20 }, { -8, -10, 20, 48 } }, { { -40, -24, 44, 8 }, { 0, 0, 0, 0 } }, { { -16, -32, 44, 20 }, { 0, 0, 0, 0 } },
    { { -56, -32, 56, 4 }, { 0, 0, 0, 0 } }, { { -64, -32, 32, 0 }, { 0, 0, 0, 0 } }, { { -24, -32, 28, 12 }, { -16, 16, 8, 64 } },
    { { -32, -24, -12, 48 }, { -12, -32, 24, 28 } }, { { -48, -32, 48, 0 }, { 0, 0, 0, 0 } }, { { -48, 0, 48, 32 }, { 0, 0, 0, 0 } },
    { { -64, -32, 32, -8 }, { 0, 0, 0, 0 } }, { { -64, 24, 32, 56 }, { 0, 0, 0, 0 } },
};
static const s8 sTouchOf[76] = {
    -1, -1, -1, -1, 6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,
    3, 3, 3, 3, 4, 4, 4, 4, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1, 5, 5, 5, 5, -1, -1, -1, -1, -1, -1, -1, -1, 12, 12, 12, 12, 13, 13, 13, 13,
};

/* His shots (0x080ACC9C): from his cape (variant 4: five ways, 0x08749BAC, NiDL's angles: 512 a turn, 0 right, 128 down), his hand
 * (variant 3: five ways around the way he faces, 0x08749BB1), at 4 pixels a frame; pointed (variants 0-2: speeds, 0x08749BB8 and
 * 0x08749BC4, x the way he faces); a quarter turn of NiDL's cosines (0x0872FB30: 256 times, truncated); */
static const u8 sCapeAngles[5] = { 14, 242, 72, 184, 128 };
static const s8 sHandAngles[5] = { -28, -14, 0, 14, 28 };
static const s16 sShotSpeedX[3] = { 0x800, 0xA00, 0x600 };
static const s16 sShotSpeedY[3] = { 0x680, 0x400, 0x800 };
static const u16 sQuarterCos[129] = {
    256, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 252, 251, 251, 250, 249, 249, 248, 247, 246, 245,
    244, 244, 243, 242, 241, 239, 238, 237, 236, 235, 234, 232, 231, 230, 228, 227, 225, 224, 222, 221, 219, 217, 216, 214,
    212, 211, 209, 207, 205, 203, 201, 199, 197, 195, 193, 191, 189, 187, 185, 183, 181, 178, 176, 174, 171, 169, 167, 164,
    162, 159, 157, 155, 152, 149, 147, 144, 142, 139, 136, 134, 131, 128, 126, 123, 120, 117, 115, 112, 109, 106, 103, 100,
    97, 95, 92, 89, 86, 83, 80, 77, 74, 71, 68, 65, 62, 59, 56, 53, 49, 46, 43, 40, 37, 34, 31, 28,
    25, 21, 18, 15, 12, 9, 6, 3, 0,
};

struct Nightmare {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sNightmareFits[sizeof(struct Nightmare) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; His fight's state (a task of its own, NIDL_OBJECT_SIZE has no room), NiDL's object's fields: */
struct Fight {
    struct Object *obj;                 // (NULL: gone, NightmareGone());
    struct EffectObject *parts[PARTS];  // His sprites;
    struct ObjectBase *touches[2];      // His touch;
    u16 pc, subPc, wait;                // His state's script and the one it calls (SUB_*), and its wait;
    u8 state, next, sub, subArg;        // (+0x15, +0x14);
    s8 facing;                          // (+0x43: 1 right, -1 left, 0 at first: drawn and moved as left);
    s8 animStep;
    s16 frame;                          // His frame (NiDL's, FRAME_HIDDEN: none; +0x3C);
    const s8 *anim;                     // His frame sequence (+0x8C's +0x2C) and its countdown (+0x18);
    s16 animCounter;
    s16 vx, vy, ax, ay, maxY;           // 8.8 (+0x54 on);
    s16 timer, count, loop;             // (+0x20, +0x24, +0x6C);
    s8 move;                            // His move's index in sMoves (+0x28);
    u8 waitIndex, capes, vanishes;      // (+0x2C, +0x30, +0x34);
    u8 shotWait, shotCount, shotWay;    // His hand's stars' wait, count and last way (0x02007D18, 0x02007D1C; +0x20);
    u8 hit;                             // HIT_* (0x02007D00);
    bool8 diving, handDone;             // (0x02007D04, 0x02007D14);
    u8 charge;                          // His charge: 0 growing, 1 done, 2 over (0x02007D1C);
    u8 box;                             // His body box (BOX_*, sBodyBoxes; NiDL keeps it when his frame has none);
    bool8 custom;                       // Drawn as his body (his palettes) or as his frame's own (his light: NiDL's two draws);
    bool8 restart;                      // His next state starts this frame;
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    u16 t;                              // Frames since his fight started (his palettes' cycle: NiDL's 0x03001EA4);
    s16 hp;                             // His HP last frame (less now: hit);
    u8 pal[PALS];
};

// Rand(); NiDL's (0x08002EE8): 0 to n - 1;
static u16 Rand(struct Fight *f, u16 n) {
    f->seed = (f->seed * 61 + 0x579) & 0xFFF;
    return f->seed * n >> 12;
}

// Cos(); NiDL's cosine (0x0872FB30, 256 times) of an angle of its;
static s32 Cos(u16 angle) {
    angle &= 511;
    if (angle <= 128) return sQuarterCos[angle];
    if (angle <= 256) return -(s32)sQuarterCos[256 - angle];
    if (angle <= 384) return -(s32)sQuarterCos[angle - 256];
    return sQuarterCos[512 - angle];
}

static s32 X(struct Fight *f) { return f->obj->base.x >> 8; }
static s32 Y(struct Fight *f) { return f->obj->base.y >> 8; }

// Closest(); The Kirby closest to him across (NiDL's 0x08063B38: in his room, alive), or NULL;
static struct Kirby *Closest(struct Fight *f) {
    struct Kirby *best = NULL;
    s32 d, bestD = 0;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];

        if (k->base.roomId != f->obj->base.roomId || k->hp <= 0) continue;
        d = abs((k->base.x >> 8) - X(f));
        if (best == NULL || d < bestD) {
            best = k;
            bestD = d;
        }
    }
    return best;
}

// DxKirby(), DyKirby(); The closest Kirby from him (0x08063CD0, 0x08063D2C; 0: none);
static s32 DxKirby(struct Fight *f) {
    struct Kirby *k = Closest(f);

    return k ? (k->base.x >> 8) - X(f) : 0;
}

static s32 DyKirby(struct Fight *f) {
    struct Kirby *k = Closest(f);

    return k ? (k->base.y >> 8) - Y(f) : 0;
}

static void Stop(struct Fight *f) {
    f->vx = f->vy = f->ax = f->ay = 0;
    f->maxY = NO_MAX;
}

static void StopY(struct Fight *f) {
    f->vy = f->ay = 0;
    f->maxY = NO_MAX;
}

// VelX(); NiDL's 0x080061C0: x speed for facing right, mirrored otherwise (facing 0 too);
static void VelX(struct Fight *f, s16 vx) {
    f->vx = f->facing == 1 ? vx : -vx;
}

// Face(); 0x08063DAC: towards a spot (right when level);
static void Face(struct Fight *f, s32 x) {
    f->facing = x - X(f) >= 0 ? 1 : -1;
}

// StepAnim(); His sequence's next frame (0x080640FC), and how long it shows;
static s16 StepAnim(struct Fight *f) {
    if (f->anim[f->animStep * 2] == NIDL_LOOP) f->animStep = 0;
    f->frame = f->anim[f->animStep * 2];
    return f->anim[f->animStep++ * 2 + 1];
}

static void PlayAnim(struct Fight *f, const s8 *anim) {
    f->anim = anim;
    f->animStep = 0;
    f->animCounter = StepAnim(f);
}

// Animate(); A frame of his sequence (0x08064188);
static void Animate(struct Fight *f) {
    if (f->anim == NULL) return;
    if (f->animCounter <= 0) f->animCounter = StepAnim(f);
    f->animCounter--;
}

// Teleport(); To his spot for a move (0x080AB4D0);
static void Teleport(struct Fight *f, u8 spot) {
    struct Object *obj = f->obj;
    s32 x = ScreenX(obj) + 0x80, y = ScreenY(obj) + sSpotY[spot];

    if (sSpotXRange[spot]) x += Rand(f, sSpotXRange[spot] + 1) - (sSpotXRange[spot] >> 1);
    if (sSpotYRange[spot]) y += Rand(f, sSpotYRange[spot] + 1) - (sSpotYRange[spot] >> 1);
    obj->base.x = x << 8;
    obj->base.y = y << 8;
}

// TeleportNext(); To his next move's spot (0x080AB4A0);
static void TeleportNext(struct Fight *f) {
    if (++f->move == ARRAY_COUNT(sMoves)) f->move = 0;
    Teleport(f, sMoves[f->move]);
}

static struct EffectObject *Effect(struct Fight *f, u8 kind);
static void Shot(struct Fight *f, u8 variant, u8 way, s8 dx, s8 dy);
static void NightmareMain(struct Object *obj);
static void Orphan(struct Fight *f);

/* The scripts: WAIT() resumes them that many frames on; CALL() runs a SUB_* one (Sub()) to its end; SET_STATE() ends them, the state
 * starting the frame after (NiDL's setState and endScript); the variables they keep across waits are the Fight's; */
#define SCRIPT switch (f->pc) { case 0:
#define WAIT(n) do { f->wait = (n); f->pc = __LINE__; return; case __LINE__:; } while (0)
#define CALL(s, arg) do { f->sub = (s); f->subArg = (arg); f->subPc = 0; f->pc = __LINE__; case __LINE__: if (Sub(f)) return; } while (0)
#define SET_STATE(s) do { f->next = (s); f->pc = PC_END; return; } while (0)
#define SCRIPT_END } f->pc = PC_END
#define SUB switch (f->subPc) { case 0:
#define SUB_WAIT(n) do { f->wait = (n); f->subPc = __LINE__; return TRUE; case __LINE__:; } while (0)
#define SUB_END } return FALSE

/* The scripts' shared parts (TRUE: waiting; each a script of its own, the scripts can't hold a switch): appearing (0x080AB670: his
 * light shrinking into his shape, 67 down to 60, then caped), vanishing (0x080AB728: the other way, then gone), four frames two each
 * (0x080AB7CC: 44-47, 0x080AB810: 36-39), knocked back (0x080AB5C0); */
static bool32 SubAppear(struct Fight *f) {
    SUB;
    f->custom = FALSE;
    if (f->subArg) PlaySfx(&f->obj->base, SE_DARK_MIND_TELE_2);
    for (f->frame = LIGHT_LAST; ; f->frame--) {
        SUB_WAIT(2);
        if (f->frame == LIGHT_FIRST) break;
    }
    f->custom = TRUE;
    f->frame = 50;
    SUB_WAIT(5);
    f->frame = 51;
    SUB_WAIT(10);
    SUB_END;
}

static bool32 SubVanish(struct Fight *f) {
    SUB;
    Stop(f);
    f->custom = FALSE;
    if (f->subArg) PlaySfx(&f->obj->base, SE_DARK_MIND_TELE_1);
    for (f->frame = LIGHT_FIRST; ; f->frame++) {
        SUB_WAIT(2);
        if (f->frame == LIGHT_LAST) break;
    }
    f->frame = FRAME_HIDDEN;
    SUB_END;
}

static bool32 SubFrames(struct Fight *f) {
    SUB;
    for (f->frame = f->subArg; ; f->frame++) {
        SUB_WAIT(2);
        if (f->frame == f->subArg + 3) break;
    }
    SUB_END;
}

static bool32 SubKnock(struct Fight *f) {
    SUB;
    Stop(f);
    PlayAnim(f, f->diving ? sHurtDiving : sHurt);
    f->diving = FALSE;
    VelX(f, -0x400);
    SUB_WAIT(16);
    VelX(f, -0x200);
    SUB_WAIT(16);
    VelX(f, -0x100);
    SUB_WAIT(16);
    VelX(f, -0x80);
    SUB_WAIT(16);
    VelX(f, -0x40);
    SUB_WAIT(16);
    f->vx = 0;
    SUB_END;
}

static bool32 Sub(struct Fight *f) {
    if (f->sub == SUB_APPEAR) return SubAppear(f);
    if (f->sub == SUB_VANISH) return SubVanish(f);
    if (f->sub == SUB_FRAMES) return SubFrames(f);
    return SubKnock(f);
}

// State0(); His first appearance (0x080AA47C): his first move's spot, caped, bobbing up and down twice;
static void State0(struct Fight *f) {
    SCRIPT;
    f->anim = NULL;
    TeleportNext(f);
    Stop(f);
    CALL(SUB_APPEAR, TRUE);
    PlayAnim(f, sCaped);
    f->box = BOX_KNOCK;
    for (f->loop = 0; f->loop < 2; f->loop++) {
        f->vy = -0x40;
        WAIT(15);
        f->vy = -0x80;
        WAIT(15);
        f->vy = -0x40;
        WAIT(15);
        f->vy = 0x40;
        WAIT(15);
        f->vy = 0x80;
        WAIT(15);
        f->vy = 0x40;
        WAIT(15);
    }
    Stop(f);
    SET_STATE(1);
    SCRIPT_END;
}

// State1(); Hovering, caped, bobbing (not before the swoop), his countdown (Frame()) till his move (0x080AA560);
static void State1(struct Fight *f) {
    SCRIPT;
    PlayAnim(f, sCaped);
    f->count = sMoves[f->move];
    if (f->count == 0) {
        f->timer = 60;
    } else if (f->count == 3) {
        f->timer = 18;
    } else {
        f->waitIndex = (f->waitIndex + Rand(f, 3) + 1) & 3;
        f->timer = sWaits[f->waitIndex];
    }
    if (f->count != 4) {
        for (;;) {
            f->vy = -0x80;
            WAIT(10);
            f->vy = -0x100;
            WAIT(10);
            f->vy = -0x80;
            WAIT(10);
            f->vy = 0x80;
            WAIT(10);
            f->vy = 0x100;
            WAIT(10);
            f->vy = 0x80;
            WAIT(10);
        }
    }
    SCRIPT_END;
}

// State2(); Reappearing at his next move's spot (0x080AA67C);
static void State2(struct Fight *f) {
    SCRIPT;
    TeleportNext(f);
    Stop(f);
    CALL(SUB_APPEAR, TRUE);
    SET_STATE(1);
    SCRIPT_END;
}

// State3(); Vanishing, then back after 30 to 60 frames, every fourth time by state 10 (0x080AA6D0);
static void State3(struct Fight *f) {
    SCRIPT;
    Stop(f);
    CALL(SUB_VANISH, TRUE);
    if (++f->vanishes & 3) {
        WAIT(Rand(f, 31) + 30);
        SET_STATE(2);
    }
    SET_STATE(10);
    SCRIPT_END;
}

// State4(); The dash (0x080AA744): back to the screen's edge on the closest Kirby's side (facing the other way), then across it if
// he's within 111 pixels, else turning and back; then the hand (move 1) or the charge (move 2);
static void State4(struct Fight *f) {
    struct Kirby *k;
    s32 sx;

    SCRIPT;
    k = Closest(f);
    f->facing = k && (k->base.x >> 8) - ScreenX(f->obj) > 0x80 ? -1 : 1;
    VelX(f, -0x300);
    f->frame = 36;
    f->count = 2;
    for (;;) {
        if (!--f->count) {
            if (++f->frame > 39) f->frame = 36;
            f->count = 2;
        }
        sx = X(f) - ScreenX(f->obj);
        if ((f->facing == 1 && sx <= 31) || (f->facing == -1 && sx > 208)) break;
        WAIT(1);
    }
    if (abs(DxKirby(f)) <= 111) {
        VelX(f, -0x200);
        for (f->loop = 0; f->loop < 2; f->loop++) CALL(SUB_FRAMES, 44);
        PlayAnim(f, sDash);
        VelX(f, 0x580);
        for (f->loop = 0; f->loop < 24; f->loop++) {
            Animate(f);
            WAIT(1);
        }
        VelX(f, 0x200);
        f->frame = 56;
        WAIT(8);
        f->frame++;
        WAIT(8);
        f->facing = -f->facing;
        VelX(f, -0x100);
        CALL(SUB_FRAMES, 44);
        VelX(f, -0x80);
    } else {
        Stop(f);
        f->facing = -f->facing;
        f->frame = 56;
        WAIT(8);
        f->frame++;
        WAIT(8);
        f->facing = -f->facing;
        CALL(SUB_FRAMES, 44);
    }
    for (f->loop = 0; f->loop < 6; f->loop++) CALL(SUB_FRAMES, 36);
    Stop(f);
    SET_STATE(sMoves[f->move] == 2 ? 7 : 6);
    SCRIPT_END;
}

// State5(); The cape (0x080AAD98): up, slowing, then his cape thrown open, its glow and sparkles, five stars (in two orders by turns);
static void State5(struct Fight *f) {
    SCRIPT;
    PlayAnim(f, sCaped);
    f->vy = 0;
    f->ay = -0x20;
    WAIT(16);
    f->vy = -0x200;
    f->ay = 0x20;
    WAIT(16);
    StopY(f);
    PlayAnim(f, sCapeOpen);
    Effect(f, FX_CAPE_STARS);
    Effect(f, FX_CAPE_GLOW);
    Effect(f, FX_CAPE_SPARKLES);
    WAIT(20);
    if (f->capes & 1) {
        Shot(f, 4, 0, 0, 0);
        Shot(f, 4, 1, 0, 0);
        WAIT(16);
        Shot(f, 4, 2, 0, 0);
        Shot(f, 4, 3, 0, 0);
        WAIT(16);
        Shot(f, 4, 4, 0, 0);
    } else {
        Shot(f, 4, 4, 0, 0);
        WAIT(16);
        Shot(f, 4, 2, 0, 0);
        Shot(f, 4, 3, 0, 0);
        WAIT(16);
        Shot(f, 4, 0, 0, 0);
        Shot(f, 4, 1, 0, 0);
    }
    f->capes++;
    SET_STATE(3);
    SCRIPT_END;
}

// State6(); The hand (0x080AAB80): down (to the screen's 64, or Kirby's height), then stars from his hand, each a way on at random;
static void State6(struct Fight *f) {
    bool8 high;

    SCRIPT;
    f->anim = NULL;
    f->vy = 0;
    f->ay = 0x15;
    f->maxY = 0x300;
    f->frame = 36;
    f->count = 2;
    for (;;) {
        if (!--f->count) {
            if (++f->frame > 39) f->frame = 36;
            f->count = 2;
        }
        if (Y(f) - ScreenY(f->obj) > 0x40 || DyKirby(f) <= 0 || abs(DyKirby(f)) <= 7) break;
        WAIT(1);
    }
    Stop(f);
    PlayAnim(f, sHandOut);
    high = f->obj->unk80 > MAX_HP / 3;
    f->shotWait = sShotWaits[high];
    f->shotCount = sShotCounts[high];
    f->handDone = FALSE;
    Effect(f, FX_HAND_GLOW);
    Effect(f, FX_HAND_SPARKLES);
    f->shotWay = 0;
    for (f->loop = 0; f->loop < f->shotCount; f->loop++) {
        WAIT(f->shotWait);
        f->shotWay += 1 + Rand(f, 4);
        if (f->shotWay > 4) f->shotWay -= 5;
        Shot(f, 3, f->shotWay, 0, 8);
    }
    WAIT(4);
    f->anim = NULL;
    f->handDone = TRUE;
    for (f->loop = 0; f->loop < 3; f->loop++) CALL(SUB_FRAMES, 36);
    SET_STATE(3);
    SCRIPT_END;
}

// State7(); The charge (0x080AA998): his hand raised, the charge's sparkles growing till it's done, then six stars pointed down;
static void State7(struct Fight *f) {
    SCRIPT;
    f->charge = 0;
    f->anim = NULL;
    for (f->frame = 52; ; f->frame++) {
        WAIT(8);
        if (f->frame == 55) break;
    }
    Effect(f, FX_CHARGE);
    Effect(f, FX_CHARGE_SPARKLES);
    PlaySfx(&f->obj->base, SE_DMK_CHARGE);
    PlayAnim(f, sHandUp);
    while (!f->charge) WAIT(1);
    PlayAnim(f, sPointing);
    PlaySfx(&f->obj->base, SE_DARK_MIND_LASER_SHOT);
    for (f->loop = 0; f->loop < 6; f->loop++) {
        if (f->obj->unk80 <= MAX_HP / 3 && (f->loop & 1)) {
            Shot(f, 1, 0, 0x30, 0x10);
            Shot(f, 2, 0, 0x30, 0x10);
        } else {
            Shot(f, 0, 0, 0x30, 0x10);
        }
        WAIT(8);
    }
    WAIT(31);
    f->charge = 2;
    f->anim = NULL;
    for (f->loop = 0; f->loop < 3; f->loop++) CALL(SUB_FRAMES, 36);
    SET_STATE(3);
    SCRIPT_END;
}

// Steer(); The tornado's steering (0x080AB570): a quarter pixel a frame faster towards the closest Kirby, 1.5 at most;
static void Steer(struct Fight *f) {
    if (DxKirby(f) > 0) {
        f->vx += 0x40;
        if (f->vx > 0x180) f->vx = 0x180;
    } else {
        f->vx -= 0x40;
        if (f->vx < -0x180) f->vx = -0x180;
    }
}

// State8(); The tornado (0x080AAF6C): down at the closest Kirby, steering, then back up; under a third of his HP, up off the screen,
// then down through it fast, slowing, and back up;
static void State8(struct Fight *f) {
    SCRIPT;
    f->count = 1;
    PlayAnim(f, sTornado);
    if (f->obj->unk80 > MAX_HP / 3) {
        WAIT(60);
        f->vy = 0xA0;
        for (f->loop = 0; f->loop < 10; f->loop++) {
            Steer(f);
            WAIT(8);
        }
        f->vy = 0;
        for (f->loop = 0; f->loop < 4; f->loop++) {
            Steer(f);
            WAIT(8);
        }
        f->vy = -0xA0;
        for (f->loop = 0; f->loop < 10; f->loop++) {
            Steer(f);
            WAIT(8);
        }
    } else {
        f->ay = -0x19;
        while (Y(f) - ScreenY(f->obj) >= -40) WAIT(1);
        StopY(f);
        f->anim = NULL;
        f->frame = FRAME_HIDDEN;
        WAIT(30);
        PlayAnim(f, sDive);
        f->diving = TRUE;
        f->vy = 0x800;
        WAIT(12);
        f->vy = 0x400;
        WAIT(10);
        f->vy = 0x200;
        WAIT(5);
        f->vy = 0x100;
        WAIT(5);
        f->vy = 0x80;
        WAIT(5);
        f->vy = 0;
        WAIT(16);
        f->vy = -0x80;
        WAIT(5);
        f->vy = -0x100;
        WAIT(5);
        f->vy = -0x200;
        WAIT(5);
        f->vy = -0x400;
        WAIT(10);
        f->vy = -0x800;
        WAIT(6);
        f->diving = FALSE;
    }
    Stop(f);
    SET_STATE(3);
    SCRIPT_END;
}

// State9(); The swoop (0x080AB1A8): facing the closest Kirby, down and across, turning, then down, across and up the other way;
static void State9(struct Fight *f) {
    struct Kirby *k;

    SCRIPT;
    WAIT(24);
    PlaySfx(&f->obj->base, SE_KRACKO_SWEEP_ATTACK);
    k = Closest(f);
    if (k) Face(f, k->base.x >> 8);
    PlayAnim(f, sDash);
    VelX(f, -0x400);
    f->vy = 0x120;
    WAIT(24);
    VelX(f, -0x100);
    f->vy = 0x100;
    WAIT(8);
    f->anim = NULL;
    f->facing = -f->facing;
    VelX(f, 0x80);
    f->vy = 0x100;
    f->frame = 56;
    WAIT(8);
    VelX(f, 0);
    f->vy = 0x100;
    f->frame++;
    WAIT(8);
    VelX(f, -0x100);
    f->vy = 0x80;
    f->frame++;
    WAIT(8);
    PlayAnim(f, sDash);
    VelX(f, -0x300);
    f->vy = 0x80;
    WAIT(4);
    VelX(f, -0x400);
    f->vy = 0;
    WAIT(24);
    VelX(f, -0x300);
    f->vy = -0x100;
    WAIT(8);
    VelX(f, -0x300);
    f->vy = -0x200;
    WAIT(8);
    f->anim = NULL;
    f->facing = -f->facing;
    VelX(f, 0x200);
    f->vy = -0x80;
    f->frame = 56;
    WAIT(8);
    VelX(f, 0x100);
    f->vy = -0x100;
    f->frame++;
    WAIT(8);
    f->facing = -f->facing;
    VelX(f, -0x80);
    f->vy = -0x100;
    CALL(SUB_FRAMES, 44);
    VelX(f, 0);
    f->vy = -0x80;
    for (f->loop = 0; f->loop < 4; f->loop++) CALL(SUB_FRAMES, 36);
    Stop(f);
    SET_STATE(3);
    SCRIPT_END;
}

// State10(); Three quick reappearances about the screen's top (0x080AB3C8), then back for good (state 2);
static void State10(struct Fight *f) {
    SCRIPT;
    for (f->loop = 0; f->loop < 3; f->loop++) {
        Teleport(f, 5);
        CALL(SUB_APPEAR, TRUE);
        CALL(SUB_VANISH, TRUE);
        WAIT(2);
    }
    SET_STATE(2);
    SCRIPT_END;
}

// State11(); Knocked back (0x080AB440), then gone (state 3);
static void State11(struct Fight *f) {
    SCRIPT;
    CALL(SUB_KNOCK, 0);
    f->hit = 0;
    SET_STATE(3);
    SCRIPT_END;
}

static void (*const sStates[12])(struct Fight *) = {
    State0, State1, State2, State3, State4, State5, State6, State7, State8, State9, State10, State11,
};

// Frame(); His state's function each frame, after his script and his move (0x08749428): his sequence stepped (not while dashing,
// gone or coming back: NiDL's 2, 3, 4, 10), state 1's countdown to his move, state 8's sound every 6 frames; a new state starts the
// frame after;
static void Frame(struct Fight *f) {
    switch (f->state) {
    case 1:
        Animate(f);
        if (!--f->timer) {
            Stop(f);
            f->next = sMoveStates[f->count];
        }
        break;
    case 8:
        if (!--f->count) {
            PlaySfx(&f->obj->base, SE_ABILITY_TORNADO_ATTACK);
            f->count = 6;
        }
        Animate(f);
        break;
    case 2: case 3: case 4: case 10:
        break;
    default:
        Animate(f);
        break;
    }
    if (f->next != f->state) f->restart = TRUE;
}

// Move(); His move (NiDL's: his speed plus its change, capped, then his spot plus his speed);
static void Move(struct Fight *f) {
    struct Object *obj = f->obj;

    f->vx += f->ax;
    f->vy += f->ay;
    if (f->maxY != NO_MAX) {
        if (f->vy > f->maxY) f->vy = f->maxY;
        if (f->vy < -f->maxY) f->vy = -f->maxY;
    }
    obj->base.x += f->vx;
    obj->base.y += f->vy;
    obj->base.xspeed = obj->base.yspeed = 0;
}

// Box(); A box of NiDL's (facing right) set where he faces (KatAM's boxes don't turn with him);
static void Box(struct Fight *f, struct ObjectBase *base, const s8 *box) {
    if (f->facing == -1) sub_0803E2B0(base, -box[2], box[1], -box[0], box[3]);
    else sub_0803E2B0(base, box[0], box[1], box[2], box[3]);
}

// Boxes(); His body box as NiDL's (0x080AA3A8): his frame's in 0-35, where he takes hits; knocked back, its own; else as it was, but
// none (flag 0x200);
static void Boxes(struct Fight *f) {
    struct Object *obj = f->obj;
    bool8 hurtable = f->frame >= 0 && f->frame <= 35;

    if (hurtable) f->box = sBodyBoxOf[f->frame];
    else if (f->hit & HIT_KNOCKED) f->box = f->diving ? BOX_DIVE : BOX_KNOCK;
    if (hurtable && f->box != BOX_NONE) {
        obj->base.flags &= ~0x200;
        Box(f, &obj->base, sBodyBoxes[f->box]);
    } else {
        obj->base.flags |= 0x200;
    }
}

// Hit(); A star took HP off him (NiDL's 0x080AB854): facing whoever shot it (NiDL's: its hitter's slot, +0x7F, Kirby's for his
// stars; the closest Kirby if KatAM's hitter isn't a Kirby's), knocked back (state 11, starting now: NiDL's the frame after its
// hit, KatAM's hits come after his frame);
static void Hit(struct Fight *f) {
    struct ObjectBase *by = f->obj->base.unk6C;
    struct Kirby *shooter = NULL;
    u8 i;

    for (i = 0; i < gNumKirbys && by; i++)
        if (by == &gKirbys[i].base || (by->header.kind == 2 && by->parent == &gKirbys[i])) shooter = &gKirbys[i];
    if (shooter == NULL) shooter = Closest(f);
    f->hit |= HIT_KNOCKED;
    Face(f, (shooter ? shooter->base.x : f->obj->base.x) >> 8);
    f->next = 11;
    f->state = 11;
    f->pc = 0;
    f->wait = 0;
    f->restart = FALSE;
}

/* Colors(); His palettes this frame (NiDL's 0x080A9EF4): his cape's, A blended into B and back over 64 frames (each channel, 8 steps
 * of 256 a frame), his tornado's C and D by turns every two frames (C alone with "Reduce Flashing": calm.c), in gObjPalette and
 * the copy KatAM puts back after a flash; */
void sub_0803D280(u8, u8);      // A palette slot's colours into the copy KatAM puts back after a flash;

static void Colors(struct Fight *f) {
    const u16 *a = gNidlNightmareFight.colors[0], *b = gNidlNightmareFight.colors[1];
    u16 *blend, *twinkle;
    s32 amount = f->t & 0x3F, i;

    if (gKirbys[gLocalPlayerId].base.roomId != f->obj->base.roomId || !f->pal[PAL_BLEND] || !f->pal[PAL_TWINKLE]) return;
    if (amount > 31) amount = 64 - amount;
    amount *= 8;
    blend = &gObjPalette[f->pal[PAL_BLEND] * 16];
    twinkle = &gObjPalette[f->pal[PAL_TWINKLE] * 16];
    for (i = 0; i < 16; i++) {
        s32 r = a[i] & 0x1F, g = a[i] & 0x3E0, bl = a[i] & 0x7C00;

        r += ((b[i] & 0x1F) - r) * amount >> 8;
        g += ((b[i] & 0x3E0) - g) * amount >> 8;
        bl += ((b[i] & 0x7C00) - bl) * amount >> 8;
        blend[i] = (r & 0x1F) | (g & 0x3E0) | (bl & 0x7C00);
        twinkle[i] = gNidlNightmareFight.colors[2 + (gNidlCalm.updateScreenDma ? 0 : (f->t >> 1) & 1)][i];
    }
    sub_0803D280(f->pal[PAL_BLEND] * 16, 16);
    sub_0803D280(f->pal[PAL_TWINKLE] * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

extern u16 gUnk_02022E60[];     // The keys of the palettes loaded in OBJ slots 8-14 (KatAM's sub_0803DF24, sub_0803DFAC), and how many;
extern u8 gUnk_02022EA0;

/* FightPalette(); His palette `which`: its slot if loaded, else the slot of `take` (his intro's, done with: KatAM keeps 7 a room,
 * slots 8-14, and loads past them into the last), else loaded; 0: not yet (no player in his room); */
static u8 FightPalette(struct Fight *f, u16 key, u8 frame, u16 take) {
    struct Sprite sprite;
    u8 i;

    if (gKirbys[gLocalPlayerId].base.roomId != f->obj->base.roomId) return 0;
    for (i = 0; i < gUnk_02022EA0; i++)
        if (gUnk_02022E60[i] == key) return i + 8;
    for (i = 0; i < gUnk_02022EA0 && take; i++) {
        if (gUnk_02022E60[i] != take) continue;
        gUnk_02022E60[i] = key;
        SpriteSomething(&sprite, 0x6000000, key & 0xFFF, frame, 0xFF, 0, 0, 0, 0, 0x10, i + 8, 0x80000);
        sub_0803D280((i + 8) * 0x10, 0x10);
        return i + 8;
    }
    sub_0803DFAC(key, frame);
    i = sub_0803DF24(key);
    return i == 0xFF ? 0 : i;
}

static void FightPalettes(struct Fight *f) {
    u16 animId = gNidlData.bosses[NIDL_NIGHTMARE].animId, fight = gNidlNightmareFight.animId;

    if (!f->pal[PAL_COLLAR]) f->pal[PAL_COLLAR] = FightPalette(f, animId, 0, 0);
    if (!f->pal[PAL_TWINKLE]) f->pal[PAL_TWINKLE] = FightPalette(f, fight | FIGHT_KEY_TWINKLE, gNidlNightmareFight.sparkles, animId | INTRO_KEY_A);
    if (!f->pal[PAL_BLEND]) f->pal[PAL_BLEND] = FightPalette(f, fight | FIGHT_KEY_BLEND, gNidlNightmareFight.frames[48][PART_CAPE], animId | INTRO_KEY_B);
    if (!f->pal[PAL_LIGHT]) f->pal[PAL_LIGHT] = FightPalette(f, animId | INTRO_KEY_LIGHT, FRAME_LIGHT, 0);
    if (!f->pal[PAL_FX]) f->pal[PAL_FX] = FightPalette(f, fight | FIGHT_KEY_FX, gNidlNightmareFight.shots, 0);
    if (!f->pal[PAL_PUFF]) f->pal[PAL_PUFF] = FightPalette(f, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0, 0);
}

// FightSprite(); A sprite of his fight's (`tiles` tiles of VRAM, frame `frame` first), as his intro's (IntroSprite());
static struct EffectObject *FightSprite(u16 room, u16 tiles, u8 frame, u8 order) {
    struct Task *t = TaskCreate(IntroSpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = room;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlNightmareFight.animId, frame, order);
    return e;
}

/* FightDraw(); A sprite of his fight's at (x, y) (8.8), frame `frame`, palette `palId`, facing as NiDL's draws it: its frames as they
 * are facing right, mirrored facing left or 0 (KatAMExpansion.cs builds them as NiDL has them, which KatAM draws so with flag 1);
 * a pixel up from KatAM's enemies' (it builds NiDL's frames a pixel lower, to stand as KatAM's: he floats where NiDL's is); */
static void FightDraw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId, s8 facing) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    e->flags = (e->flags & ~0x401) | 4 | (facing == 1 ? 1 : 0);
    e->objBase55 = -1;
    EffectObjectPostUpdate(e);
}

// FightMain(); His fight's task: his three sprites drawn where he is (frozen as KatAM's explosion takes him), his palettes, gone
// with him;
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    u8 i, frame;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        for (i = 0; i < PARTS; i++) Release(&f->parts[i]);
        for (i = 0; i < 2; i++) {
            if (f->touches[i]) f->touches[i]->flags |= 0x1000;
            f->touches[i] = NULL;
        }
        if (obj) ((struct Nightmare *)obj)->f = NULL;
        Orphan(f);
        TaskDestroy(gCurTask);
        return;
    }
    FightPalettes(f);
    if (obj->unk78 == NightmareMain) Colors(f);
    for (i = 0; i < PARTS; i++) {
        frame = f->frame < 0 || f->frame > 75 ? FIGHT_NONE : gNidlNightmareFight.frames[f->frame][i];
        if (frame == FIGHT_NONE) {
            if (f->parts[i]) f->parts[i]->flags |= 0x400;
            continue;
        }
        if (f->parts[i] == NULL) {
            u8 j, tiles = 0;

            // (VRAM for his biggest frame of that part);
            for (j = 4; j < 76; j++)
                if (gNidlNightmareFight.frames[j][i] != FIGHT_NONE && gNidlNightmareFight.tiles[gNidlNightmareFight.frames[j][i]] > tiles)
                    tiles = gNidlNightmareFight.tiles[gNidlNightmareFight.frames[j][i]];
            f->parts[i] = FightSprite(obj->base.roomId, tiles, frame, BODY_ORDER);
        }
        FightDraw(f->parts[i], obj->base.x, obj->base.y, frame,
                  f->pal[i == PART_COLLAR ? PAL_COLLAR : i == PART_TWINKLE ? PAL_TWINKLE : f->custom ? PAL_BLEND : PAL_LIGHT], f->facing);
    }
}

/* Touch(); His touch (NiDL's companion, type 0xD0: two boxes a frame, sTouches): attack objects on him (as Mr. Shine & Mr.
 * Bright's), hurting Kirby, none while he's gone or bursting; */
static void TouchMain(void) {
    struct ObjectBase *touch = TaskGetStructPtr(gCurTask);
    struct Fight *f = touch->parent;
    struct Object *obj;
    s8 box;

    if (touch->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (ObjectPreUpdate(touch)) return;
    obj = f->obj;
    touch->flags |= 4 | 0x400;
    box = f->frame >= 0 && f->frame < 76 ? sTouchOf[f->frame] : -1;
    if (obj == NULL || box < 0 || obj->unk78 != NightmareMain || (touch->counter && !sTouches[box][1][0] && !sTouches[box][1][2])) {
        touch->flags |= 0x200;
        return;
    }
    touch->flags &= ~0x200;
    touch->x = obj->base.x;
    touch->y = obj->base.y;
    Box(f, touch, sTouches[box][touch->counter]);
    SetPointerSomething(touch);
}

static struct ObjectBase *Touch(struct Fight *f, u8 which) {
    struct Object *obj = f->obj;
    struct Task *t = TaskCreate(TouchMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *touch = TaskGetStructPtr(t);

    ClearObjectBase(touch);
    touch->header.kind = 2;
    touch->x = obj->base.x;
    touch->y = obj->base.y;
    touch->parent = f;
    touch->counter = which;
    touch->roomId = obj->base.roomId;
    touch->unk56 = obj->base.unk56;
    touch->unk63 = 1;
    touch->flags |= 0x10000140 | 0x600;
    touch->unk68 = 0x20000043;
    touch->unk5C |= 0x80000;
    ObjectSetBounds(touch, 0, 0, 0, 0);
    return touch;
}

static void ShotPuff(struct Fight *f, struct ObjectBase *shot);

/* ShotMain(); A star of his (type 0x88, 0x080ACC9C): its frames 0-3, two each (0x08749BD0), on at its speed, hurting Kirby (an attack
 * object); stopped on the floor (NiDL's terrain check, 0x0806951C: the floor its box's bottom, SHOT_BOTTOM below it), then its puff
 * (NiDL's 0x0806A344: the shared effect's, ShotPuff()); gone off the screen (KatAM's own culling off: NiDL's are 60 pixels off it),
 * after its time, or as he bursts; */
static void ShotMain(void) {
    struct Sprite sprite;
    struct ObjectBase *shot = TaskGetStructPtr(gCurTask);
    struct Fight *f = shot->parent;
    struct LevelInfo *level;
    s32 sx, sy, bottom;
    s16 age;

    if (ObjectPreUpdate(shot)) return;
    shot->flags |= 4;
    if (shot->counter == SHOT_LANDED || f->obj == NULL || f->obj->unk78 != NightmareMain) {
        if (shot->counter == SHOT_LANDED && f->obj && f->obj->unk78 == NightmareMain) ShotPuff(f, shot);
        shot->flags |= 0x1000;
        return;
    }
    level = &gCurLevelInfo[shot->unk56];
    age = shot->counter++;
    shot->x += shot->xspeed;
    shot->y += shot->yspeed;
    sx = (shot->x - level->viewportPosition.x) >> 8;
    sy = (shot->y - level->viewportPosition.y) >> 8;
    if (shot->counter >= SHOT_LIFE || sx < -OFF_SCREEN || sx > 240 + OFF_SCREEN || sy < -OFF_SCREEN || sy > 160 + OFF_SCREEN) {
        shot->flags |= 0x1000;
        return;
    }
    // (the floor under its box: on it, this frame its last);
    bottom = shot->y + SHOT_BOTTOM * 0x100 - 1;
    if (shot->yspeed > 0 && shot->x >= level->levelMinPosition.x && shot->x <= level->levelMaxPosition.x
        && NidlIsFloor((struct Object *)shot, shot->x, bottom)) {
        shot->y = (bottom & ~0xFFF) - SHOT_BOTTOM * 0x100;
        shot->counter = SHOT_LANDED;
    }
    shot->sprite.variant = gNidlNightmareFight.shots + ((age >> 1) & 3);
    Macro_08107BA8_4(shot, &shot->sprite, &sprite, gNidlNightmareFight.tiles[gNidlNightmareFight.shots], &shot->sprite);
    shot->sprite.palId = f->pal[PAL_FX];
    shot->flags &= ~0x400;
    // (drawn as FightDraw() draws: its flag 1 the way it faces, KatAM's, but for the draw);
    shot->objBase55 = -1;
    SetPointerSomething(shot);
    shot->flags ^= 1;
    sub_0806F8BC(shot);
    shot->flags ^= 1;
}

// Shot(); A star of his: variant 4 from his cape (way 0-4), 3 from his hand (way 0-4 around the way he faces), 0-2 pointed; from
// (dx, dy) off his whole pixel (dx the way he faces), facing as he does; KatAM's attack object (as his old beams);
static void Shot(struct Fight *f, u8 variant, u8 way, s8 dx, s8 dy) {
    struct Object *obj = f->obj;
    struct Task *t = TaskCreate(ShotMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *shot = TaskGetStructPtr(t);
    u16 angle;

    ClearObjectBase(shot);
    shot->header.kind = 2;
    shot->x = (obj->base.x & ~0xFF) + (f->facing == 1 ? dx : -dx) * 0x100;
    shot->y = (obj->base.y & ~0xFF) + dy * 0x100;
    shot->parent = f;
    shot->counter = 0;
    shot->roomId = obj->base.roomId;
    shot->unk56 = obj->base.unk56;
    if (variant >= 3) {
        angle = variant == 4 ? sCapeAngles[way] : (sHandAngles[way] + 0x280 - f->facing * 128) & 0x1FF;
        shot->xspeed = Cos(angle) * 4;
        shot->yspeed = Cos(angle - 128) * 4;
    } else {
        shot->xspeed = f->facing == 1 ? sShotSpeedX[variant] : -sShotSpeedX[variant];
        shot->yspeed = sShotSpeedY[variant];
    }
    shot->unk63 = 1;
    shot->flags |= 0x10000140 | 0x2000000 | (f->facing == 1 ? 0 : 1);
    shot->unk68 = 0x20000043;
    shot->unk5C |= 0x80000;
    sub_0803E2B0(shot, -SHOT_HALF, -SHOT_HALF, SHOT_HALF, SHOT_HALF);
    ObjectSetBounds(shot, 0, 0, 0, 0);
    ObjectBaseInitSprite(shot, &shot->sprite, gNidlNightmareFight.tiles[gNidlNightmareFight.shots], gNidlNightmareFight.animId,
                         gNidlNightmareFight.shots, SPARKLE_ORDER);
    shot->sprite.palId = f->pal[PAL_FX];
    PlaySfx(&obj->base, SE_DARK_MIND_SHOOT_STAR);
}

/* EffectMain(); His effects (NiDL's types 0xCD, 0xCF, 0xCE: his cape's; 0xC9, 0xCA: his hand's; 0xCB, 0xCC: his charge), on him:
 * • the cape's glow (0xCD: 0x08754738's 4, up a pixel a frame for three, then back down) and stars (0xCF: its 5-8, three frames
 *   each), its sparkles (0xCE: 0x087546F8's 0-3, two each), while his cape's open (state 5);
 * • the hand's glow (0xC9: its 9) and sparkles behind him (0xCA: 0x08754708's 0-3, the first for a frame, then two each), till his
 *   last star from it;
 * • the charge (0xCB: its 10 for 85 frames where it starts, 11 for 3, then 12 on him: the charge's done, his script sees it the
 *   frame after) and its sparkles (0xCC: 0x08754718's 0-3 while it grows, then 4-7, each time the first for a frame, then two
 *   each), till it's over;
 * all gone if he's hit (or bursts); and a star's puff where it stopped on the floor (FX_PUFF: NiDL's shared effect's 0x0874C9D8,
 * as the intro's, Puff(); facing as the star did, not hurting), gone as he bursts; unk4 the kind (FX_*), unk8 its age, unk3C the
 * sparkles' frames since the charge changed, unk3E the charge then (the puff's: its facing); */
static void EffectMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);
    struct Fight *f = e->parent;
    struct Object *obj;
    u16 k;
    u8 frame, palId;
    s32 dy = 0;
    bool8 gone;

    if (e->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    obj = f->obj;
    if (obj == NULL || obj->unk78 != NightmareMain || (f->hit && e->unk4 != FX_PUFF)) {
        e->flags |= 0x1000;
        return;
    }
    k = e->unk8++;
    palId = f->pal[PAL_FX];
    switch (e->unk4) {
    case FX_PUFF:
        if (k > PUFF_LAST) {
            e->flags |= 0x1000;
        } else if (k >= 12 && k < 16) {
            e->flags |= 0x400;
        } else {
            FightDraw(e, e->x, e->y, FRAME_PUFF + (k < 2 ? 0 : k < 12 ? k - 1 : k - 5), f->pal[PAL_PUFF], e->unk3E);
        }
        return;
    case FX_CAPE_GLOW:
        gone = f->next != 5;
        frame = gNidlNightmareFight.shots + 4;
        dy = k % 6 < 3 ? -(s32)(k % 6) - 1 : (s32)(k % 6) - 5;
        break;
    case FX_CAPE_STARS:
        gone = f->next != 5;
        frame = gNidlNightmareFight.shots + 5 + (k / 3) % 4;
        break;
    case FX_CAPE_SPARKLES:
        gone = f->next != 5;
        frame = gNidlNightmareFight.sparkles + (k / 2) % 4;
        palId = f->pal[PAL_TWINKLE];
        break;
    case FX_HAND_GLOW:
        gone = f->handDone;
        frame = gNidlNightmareFight.shots + 9;
        break;
    case FX_HAND_SPARKLES:
        gone = f->handDone;
        frame = gNidlNightmareFight.sparkles + FIGHT_SPARKLES_C + (k + 1) / 2 % 4;
        palId = f->pal[PAL_TWINKLE];
        break;
    case FX_CHARGE:
        gone = f->charge == 2;
        frame = gNidlNightmareFight.shots + (k < 85 ? 10 : k < 88 ? 11 : 12);
        if (k == 88) f->charge = 1;
        break;
    default:
        gone = f->charge == 2;
        if (e->unk3E != f->charge) e->unk3C = 0;
        e->unk3E = f->charge;
        frame = gNidlNightmareFight.sparkles + FIGHT_SPARKLES_E + (f->charge ? 4 : 0) + (e->unk3C++ + 1) / 2 % 4;
        palId = f->pal[PAL_TWINKLE];
        break;
    }
    if (gone) {
        e->flags |= 0x1000;
        return;
    }
    // (on him; the charge where it started till it's done);
    if (e->unk4 != FX_CHARGE || f->charge == 1) {
        e->x = obj->base.x;
        e->y = obj->base.y + dy * 0x100;
    }
    FightDraw(e, e->x, e->y, frame, palId, f->facing);
}

// Effect(); One of his effects (FX_*), at him, with VRAM for its biggest frame;
static struct EffectObject *Effect(struct Fight *f, u8 kind) {
    static const u8 sOrders[7] = { FRONT_ORDER, FRONT_ORDER, SPARKLE_ORDER, FRONT_ORDER, BACK_ORDER, FRONT_ORDER, SPARKLE_ORDER };
    struct Object *obj = f->obj;
    struct Task *t = TaskCreate(EffectMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);
    bool8 fx = kind <= FX_CAPE_STARS || kind == FX_HAND_GLOW || kind == FX_CHARGE;
    u8 first = fx ? gNidlNightmareFight.shots + 4 : gNidlNightmareFight.sparkles, last = fx ? first + 8 : first + 15, i, tiles = 0;

    for (i = first; i <= last; i++)
        if (gNidlNightmareFight.tiles[i] > tiles) tiles = gNidlNightmareFight.tiles[i];
    ClearEffectObject(e);
    e->header.kind = 3;
    e->x = obj->base.x;
    e->y = obj->base.y;
    e->parent = f;
    e->roomId = obj->base.roomId;
    e->unk4 = kind;
    e->unk8 = 0;
    e->unk3C = 0;
    e->unk3E = 0;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlNightmareFight.animId, first, sOrders[kind]);
    return e;
}

// ShotPuff(); A star's puff where it stopped (FX_PUFF), in his intro's frames (its puff's: FRAME_PUFF);
static void ShotPuff(struct Fight *f, struct ObjectBase *shot) {
    struct Task *t = TaskCreate(EffectMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->x = shot->x;
    e->y = shot->y;
    e->parent = f;
    e->roomId = shot->roomId;
    e->unk4 = FX_PUFF;
    e->unk8 = 0;
    e->unk3E = shot->flags & 1 ? -1 : 1;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, PUFF_TILES, gNidlData.bosses[NIDL_NIGHTMARE].animId, FRAME_PUFF, SPARKLE_ORDER);
}

// Orphan(); His fight's task ends: his shots and effects with it;
static void Orphan(struct Fight *f) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == ShotMain && ((struct ObjectBase *)TaskGetStructPtr(t))->parent == f)
            ((struct ObjectBase *)TaskGetStructPtr(t))->flags |= 0x1000;
        if (t->main == EffectMain && ((struct EffectObject *)TaskGetStructPtr(t))->parent == f)
            ((struct EffectObject *)TaskGetStructPtr(t))->flags |= 0x1000;
    }
}

// NightmareMain(); His object each frame: a hit, his state's script, his move, his state's frame function, his boxes; his sprites
// are his fight's task's (FightMain()), hidden here;
static void NightmareMain(struct Object *obj) {
    struct Nightmare *m = (struct Nightmare *)obj;
    struct Fight *f = m->f;

    NidlUpdate(&m->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; he takes it back;
    sub_08034E14(obj);
    // (every Kirby in his room fights him with the Star Rod, finale.c);
    NidlFinaleKeep(obj);
    if (f == NULL) return;
    f->t++;
    if (obj->unk80 < f->hp) Hit(f);
    f->hp = obj->unk80;
    if (f->restart) {
        f->restart = FALSE;
        f->state = f->next;
        f->pc = 0;
        f->wait = 0;
    }
    if (f->pc != PC_END && (!f->wait || !--f->wait)) sStates[f->state](f);
    Move(f);
    Frame(f);
    Boxes(f);
}

/* NightmareBlocks(); A Star Rod star (abilities/star_rod.c) at (x, y) touching his body box (+-8): it bursts on it (NiDL's keep their
 * boxes, his even while he's gone; only his frames 0-35 take hits: KatAM's, Boxes()); */
bool32 NightmareBlocks(u16 room, s32 x, s32 y) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Fight *f;
        const s8 *box;
        s32 dx, dy, left, right;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != FightMain) continue;
        f = TaskGetStructPtr(t);
        if (f->obj == NULL || f->obj->base.roomId != room || f->obj->unk78 != NightmareMain || f->box == BOX_NONE) continue;
        box = sBodyBoxes[f->box];
        dx = (x - f->obj->base.x) >> 8;
        dy = (y - f->obj->base.y) >> 8;
        left = f->facing == -1 ? -box[2] : box[0];
        right = f->facing == -1 ? -box[0] : box[2];
        if (dx + 8 > left && dx - 8 < right && dy + 8 > box[1] && dy - 8 < box[3]) return TRUE;
    }
    return FALSE;
}

/* NightmareTarget(); Whether a Star Rod star shot now from x at height y (abilities/star_rod.c's helpers, StarRodAim()) would meet him
 * where he takes hits (his box, +-8 as NightmareBlocks(), past where the star comes out, within TARGET_REACH), which way, and how
 * far his box is (pixels; under 0: over x); */
#define TARGET_REACH 160
#define TARGET_STAR_X 20
bool32 NightmareTarget(u16 room, s32 x, s32 y, bool8 *left, s32 *gap) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Fight *f;
        const s8 *box;
        s32 dy, from, to;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != FightMain) continue;
        f = TaskGetStructPtr(t);
        if (f->obj == NULL || f->obj->base.roomId != room || f->obj->unk78 != NightmareMain || f->box == BOX_NONE
            || (f->obj->base.flags & 0x200)) continue;
        box = sBodyBoxes[f->box];
        dy = (y - f->obj->base.y) >> 8;
        if (dy + 8 <= box[1] || dy - 8 >= box[3]) continue;
        from = (f->obj->base.x >> 8) + (f->facing == -1 ? -box[2] : box[0]) - 8 - (x >> 8);
        to = (f->obj->base.x >> 8) + (f->facing == -1 ? -box[0] : box[2]) + 8 - (x >> 8);
        *left = from + to < 0;
        if (*left ? to < -TARGET_REACH || from > -TARGET_STAR_X : from > TARGET_REACH || to < TARGET_STAR_X) continue;
        *gap = *left ? -to : from;
        return TRUE;
    }
    return FALSE;
}

// NightmareGone(); His object's gone (nidl.c's NidlDestroy()): his fight's task lets go of it;
void NightmareGone(struct Object *obj) {
    struct Nightmare *m = (struct Nightmare *)obj;

    if (m->f) m->f->obj = NULL;
    m->f = NULL;
}

// NightmareStart(); The fight starts (NidlBossStart(): as his intro has him appear, nidl_boss.c), his fight's task with it;
void NightmareStart(struct Object *obj) {
    struct Nightmare *m = (struct Nightmare *)obj;
    struct Task *t;
    struct Fight *f;
    u8 i;

    ObjectSetFunc(obj, -1, NightmareMain);
    if (m->started) return;
    m->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = m->f = TaskGetStructPtr(t);
    f->obj = obj;
    for (i = 0; i < PARTS; i++) f->parts[i] = NULL;
    f->state = f->next = 0;
    f->pc = 0;
    f->wait = 0;
    f->restart = FALSE;
    f->facing = 0;
    f->frame = FRAME_HIDDEN;
    f->anim = NULL;
    f->animCounter = 0;
    Stop(f);
    f->timer = f->count = f->loop = 0;
    f->move = -1;
    f->waitIndex = f->capes = f->vanishes = 0;
    f->shotWait = f->shotCount = f->shotWay = 0;
    f->hit = 0;
    f->diving = f->handDone = FALSE;
    f->charge = 0;
    f->box = BOX_KNOCK;
    f->custom = FALSE;
    f->seed = Rand16() & 0xFFF;
    f->t = 0;
    f->hp = obj->unk80;
    for (i = 0; i < PALS; i++) f->pal[i] = 0;
    f->touches[0] = Touch(f, 0);
    f->touches[1] = Touch(f, 1);
    obj->unk94 = STAR_STRENGTH;
    // (his own sprite's never shown again, his fight's task's are him, then his defeat's: its VRAM, his biggest frame's, theirs (the
    // room's sprites have half of KatAM's); unkC 0x200 keeps KatAM from giving it VRAM again (sub_0809A630, as Dark Mind's), flag 8
    // from animating it into none, and NidlUploadHurtTiles() skips none);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
}

// NightmareInit(); Where his intro has him appear (his fight puts him at his first move's spot), through anything;
void NightmareInit(struct NidlObject *n) {
    struct Nightmare *m = (struct Nightmare *)n;
    struct Object *obj = &n->obj;

    m->f = NULL;
    m->started = FALSE;
    obj->base.x = ENTRANCE_X;
    obj->base.y = ENTRANCE_Y;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -16, -16, 16, 32);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
}

/* His defeat (NightmareEnd(), as KatAM's explosion bursts him: finale.c's NightmareBeaten()): NiDL's, traced frame by frame (his
 * object's frames and spots, the light over him, the floor's bursts), run by a task of its own (KatAM's explosion takes him away,
 * as its bosses'), the Kirbys free (NiDL's walks and jumps about with the Star Rod), drawn mirrored as NiDL draws him through it:
 * • DEFEAT_BACK frames after he bursts (as the Power Orb's end), he flies off right from where he was (NiDL's 68-71, 3 frames
 *   each), 16 frames each at 4, 2, 1, 1/2 and 1/4 pixels a frame; there he turns to light (60 up to 67) and comes back as light at
 *   DEFEAT_SPOT (66 down to 60), caped (50, 51, his top half over them: 96, as NiDL draws his body frames), then spreads out and
 *   breaks up, blinking (76-88: sDefeat, from his light on, NiDL's
 *   frame or NONE and how long), gone 535 frames after the hit; a light over him meanwhile (sDefeatLight: 0x08754738's 13-17);
 * • the floor's bursts (sFloorBursts: when, from his flight's start, and where; NiDL's shared burst, as his intro's), then every
 *   Kirby still in his room is flown out (finale.c's NidlEndingFlight(): the fade to white, then the Fountain of Dreams hub);
 * the screen's held where the fight left it (NiDL's doesn't move); */
#define FRAME_FLEE 82           // His frames flying off (NiDL's 68-71: the first palette's pieces, then the second's, the third's);
#define FRAME_CAPED 94          // (50, 51: his third palette);
#define FRAME_SPREAD 96         // (76-80: the first palette's pieces, then the second's);
#define FRAME_BREAK 106         // (81-88: 81 his first palette, 82-88 his light's);
#define FRAME_GLOW 114          // The light over him (0x08754738's 13-17, his descriptor's first palette);
#define FRAME_TOP 119           // His top half over his caped body (96: the first palette's pieces, then the second's);
#define DEFEAT_KEY_GLOW 0xE000  // The light over him's palette's key (| his anim ID);
#define DEFEAT_BACK 24
#define DEFEAT_FLIGHT 80
#define DEFEAT_SPEEDS 16        // (frames at each speed flying off: 4, 2, 1, 1/2, 1/4 pixels a frame);
#define DEFEAT_REAPPEAR 16      // (frames into sDefeat: back, at DEFEAT_SPOT);
#define DEFEAT_SPOT_X (136 << 8)
#define DEFEAT_SPOT_Y (282 << 8)
#define DEFEAT_GLOW_FROM 227
#define DEFEAT_FLOOR_Y (352 << 8)
#define DEFEAT_OUT 840          // (from his flight's start: the Kirbys flown out);
#define DEFEAT_BURSTS 4         // (at once, at most);
#define DEFEAT_TILES LIGHT_TILES
#define DEFEAT_B_TILES 64
#define DEFEAT_C_TILES 72
#define GLOW_TILES 48
#define NONE 0xFF

static const u8 sDefeat[][2] = {
    { 60, 2 }, { 61, 2 }, { 62, 2 }, { 63, 2 }, { 64, 2 }, { 65, 2 }, { 66, 2 }, { 67, 4 }, { 66, 2 }, { 65, 2 }, { 64, 2 }, { 63, 2 },
    { 62, 2 }, { 61, 2 }, { 60, 2 }, { 50, 5 }, { 51, 10 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 },
    { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 },
    { 77, 2 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 86, 2 }, { NONE, 2 }, { 87, 2 }, { NONE, 2 }, { 88, 2 }, { NONE, 10 },
};

static const u8 sDefeatLight[][2] = {
    { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 },
    { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 4 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 },
    { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 },
    { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 15, 2 }, { 14, 2 }, { 13, 2 }, { 16, 2 }, { 17, 2 },
};

static const s16 sFloorBursts[][2] = {
    { 661, 88 }, { 691, 232 }, { 706, 216 }, { 706, 56 }, { 736, 200 }, { 746, 120 }, { 756, 40 }, { 766, 224 }, { 811, 88 }, { 811, 208 }, { 841, 232 },
};

/* DefeatPalette(); A palette of his defeat's, as frame `frame` of his anim has it: loaded into the slot of `take` (one of his fight's,
 * done with), else its slot if `key`'s loaded, else as the game loads one; `force` loads it again where it is (KatAM keeps 7 a
 * room, slots 8-14, and loads past them into the last: his fight's fill them, FightPalettes()), or 0 (not yet: the local player
 * isn't in the room); */
static u8 DefeatPalette(u16 room, u16 key, u8 frame, u16 take, bool8 force) {
    struct Sprite sprite;
    u8 i, slot = 0xFF;

    if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
    for (i = 0; i < gUnk_02022EA0 && slot == 0xFF && take; i++)
        if (gUnk_02022E60[i] == take) slot = i + 8, force = TRUE;
    for (i = 0; i < gUnk_02022EA0 && slot == 0xFF; i++)
        if (gUnk_02022E60[i] == key) slot = i + 8;
    if (slot == 0xFF) return IntroPalette(room, key, frame);
    if (force) {
        gUnk_02022E60[slot - 8] = key;
        SpriteSomething(&sprite, 0x6000000, key & 0xFFF, frame, 0xFF, 0, 0, 0, 0, 0x10, slot, 0x80000);
        sub_0803D280(slot * 0x10, 0x10);
    }
    return slot;
}

struct NightmareEnd {
    struct EffectObject e;
    s32 x, y;                   // Where he is (8.8, the room's): where he burst, then flying off;
    s32 camX, camY;             // The screen, held where the fight left it (NiDL's doesn't move), 8.8;
    u16 t;                      // Frames since he burst;
    u16 run, left;              // sDefeat's run, and its frames left;
    u16 glowRun, glowLeft;      // sDefeatLight's;
    u8 palA, palB, palC, lightPal, glowPal, burstPal;
    struct EffectObject *layers[3], *glow, *bursts[DEFEAT_BURSTS];
};

// DefeatDraw(); A sprite of his defeat's at (x, y) (8.8, the room's), frame `frame`, palette `palId`, mirrored or not;
static void DefeatDraw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId, bool8 mirrored) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    // (KatAM draws effects mirrored unless they face right: NiDL draws him mirrored, its effects as they are);
    e->flags = (e->flags & ~0x401) | 4 | (mirrored ? 0 : 1);
    EffectObjectPostUpdate(e);
}

// DefeatLayer(); His sprite `which` (0 his first palette's pieces and the frames of one palette, 1 and 2 his second's and third's,
// 2 his caped body too), made as it's needed, in front of or behind the others as his own (his body in front of his collar, the
// cape's ends and his caped body behind);
static struct EffectObject *DefeatLayer(struct NightmareEnd *end, u8 which) {
    static const u8 sTiles[3] = { DEFEAT_TILES, DEFEAT_B_TILES, DEFEAT_C_TILES };
    static const u8 sOrder[3] = { 0x1B, 0x1A, 0x1C };

    if (end->layers[which] == NULL) {
        end->layers[which] = IntroSprite(end->e.roomId, sTiles[which], FRAME_FLEE);
        end->layers[which]->sprite.unk14 = sOrder[which] << 6;
    }
    return end->layers[which];
}

// DefeatFrame(); His frame `frame` (NiDL's) at (x, y): its palettes' pieces, each sprite one palette, those it hasn't let go;
static void DefeatFrame(struct NightmareEnd *end, u8 frame, s32 x, s32 y) {
    u8 i;

    if (frame >= 68 && frame <= 71) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_FLEE + frame - 68, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_FLEE + 4 + frame - 68, end->palB, TRUE);
        DefeatDraw(DefeatLayer(end, 2), x, y, FRAME_FLEE + 8 + frame - 68, end->palC, TRUE);
        return;
    }
    if (frame == 50 || frame == 51) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_TOP, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_TOP + 1, end->palB, TRUE);
        DefeatDraw(DefeatLayer(end, 2), x, y, FRAME_CAPED + frame - 50, end->palC, TRUE);
        return;
    }
    for (i = 1; i < 3; i++)
        if (!(frame >= 76 && frame <= 80 && i == 1)) Release(&end->layers[i]);
    if (frame >= 60 && frame <= 67) DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_LIGHT + 67 - frame, end->lightPal, TRUE);
    else if (frame >= 76 && frame <= 80) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_SPREAD + frame - 76, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_SPREAD + 5 + frame - 76, end->palB, TRUE);
    } else if (frame >= 81 && frame <= 88) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_BREAK + frame - 81, frame == 81 ? end->palA : end->lightPal, TRUE);
    }
}

// DefeatBursts(); The floor's bursts at k (frames since his flight's start): each a frame of the burst, then one of its sparkles,
// in turn (as his intro's), a sprite of their own each (the next one's sprite after it: at most DEFEAT_BURSTS at once);
static void DefeatBursts(struct NightmareEnd *end, u16 k) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sFloorBursts); i++) {
        u16 a = k - sFloorBursts[i][0];
        struct EffectObject **e = &end->bursts[i % DEFEAT_BURSTS];

        if (k < sFloorBursts[i][0] || a > BURST_FRAMES) continue;
        if (a == BURST_FRAMES) {
            Release(e);
            continue;
        }
        if (*e == NULL) *e = IntroSprite(end->e.roomId, BURST_TILES, FRAME_BURST);
        DefeatDraw(*e, sFloorBursts[i][1] << 8, DEFEAT_FLOOR_Y, FRAME_BURST + (a & 1 ? 12 + a / 2 : a / 2), end->burstPal, FALSE);
    }
}

// DefeatGone(); Its sprites let go;
static void DefeatGone(struct NightmareEnd *end) {
    u8 i;

    for (i = 0; i < 3; i++) Release(&end->layers[i]);
    Release(&end->glow);
    for (i = 0; i < DEFEAT_BURSTS; i++) Release(&end->bursts[i]);
}

static void NightmareEndMain(void) {
    struct NightmareEnd *end = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &end->e;
    u16 animId = gNidlData.bosses[NIDL_NIGHTMARE].animId, k;
    u8 i;

    if (e->flags & 0x1000) {
        DefeatGone(end);
        TaskDestroy(gCurTask);
        return;
    }
    // (gone with the room: every Kirby left it);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys) {
        DefeatGone(end);
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    NidlFinaleHoldCamera(e->roomId, end->camX, end->camY);
    // (his palettes and the bursts', once a player's in the room: his first his fight's collar's, his second and third where his
    // fight's tornado's and cape's were, FightPalettes(), done with);
    if (!end->palA) end->palA = DefeatPalette(e->roomId, animId, FRAME_FLEE, 0, FALSE);
    if (!end->palC) end->palC = DefeatPalette(e->roomId, animId | NIDL_THIRD_PALETTE, FRAME_CAPED, gNidlNightmareFight.animId | FIGHT_KEY_BLEND, FALSE);
    if (!end->palB) end->palB = DefeatPalette(e->roomId, animId | NIDL_SECOND_PALETTE, FRAME_FLEE + 4, gNidlNightmareFight.animId | FIGHT_KEY_TWINKLE, TRUE);
    if (!end->burstPal) end->burstPal = IntroPalette(e->roomId, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0);
    k = end->t++;
    if (k < DEFEAT_BACK) return;
    k -= DEFEAT_BACK;
    // (his light's as his fight had it, and the glow's as it's first needed, where his shots' was: they went with him);
    if (!end->lightPal && k >= DEFEAT_FLIGHT) end->lightPal = DefeatPalette(e->roomId, animId | INTRO_KEY_LIGHT, FRAME_LIGHT, 0, FALSE);
    if (!end->glowPal && k >= DEFEAT_GLOW_FROM)
        end->glowPal = DefeatPalette(e->roomId, animId | DEFEAT_KEY_GLOW, FRAME_GLOW, gNidlNightmareFight.animId | FIGHT_KEY_FX, FALSE);
    // (flying off: 4 pixels a frame, half that every DEFEAT_SPEEDS frames);
    if (k < DEFEAT_FLIGHT) {
        end->x += 0x400 >> (k / DEFEAT_SPEEDS);
        DefeatFrame(end, 68 + (k / 3) % 4, end->x, end->y);
    } else if (end->run < ARRAY_COUNT(sDefeat)) {
        u8 frame = sDefeat[end->run][0];

        if (!end->left) end->left = sDefeat[end->run][1];
        if (k - DEFEAT_FLIGHT == DEFEAT_REAPPEAR) {
            end->x = DEFEAT_SPOT_X;
            end->y = DEFEAT_SPOT_Y;
        }
        if (frame != NONE) DefeatFrame(end, frame, end->x, end->y);
        if (!--end->left) end->run++;
    } else {
        for (i = 0; i < 3; i++) Release(&end->layers[i]);
    }
    // (the light over him);
    if (k >= DEFEAT_GLOW_FROM && end->glowRun < ARRAY_COUNT(sDefeatLight)) {
        u8 frame = sDefeatLight[end->glowRun][0];

        if (!end->glowLeft) end->glowLeft = sDefeatLight[end->glowRun][1];
        if (frame != NONE) {
            if (end->glow == NULL) end->glow = IntroSprite(e->roomId, GLOW_TILES, FRAME_GLOW);
            DefeatDraw(end->glow, DEFEAT_SPOT_X, DEFEAT_SPOT_Y, FRAME_GLOW + frame - 13, end->glowPal, FALSE);
        }
        if (!--end->glowLeft && ++end->glowRun >= ARRAY_COUNT(sDefeatLight)) Release(&end->glow);
    }
    DefeatBursts(end, k);
    if (k == DEFEAT_OUT) NidlEndingFlight(e->roomId);
}

// NightmareEnd(); He's beaten (finale.c's NightmareBeaten(), as KatAM's explosion bursts him): his defeat (see above);
void NightmareEnd(struct Object *obj) {
    struct Task *t = TaskCreate(NightmareEndMain, sizeof(struct NightmareEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct NightmareEnd *end = TaskGetStructPtr(t);
    u8 i;

    ClearEffectObject(&end->e);
    end->e.header.kind = 3;
    end->e.roomId = obj->base.roomId;
    end->e.parent = NULL;
    end->e.flags |= 0x400;
    end->x = obj->base.x;
    end->y = obj->base.y;
    end->camX = gCurLevelInfo[obj->base.unk56].viewportPosition.x;
    end->camY = gCurLevelInfo[obj->base.unk56].viewportPosition.y;
    end->t = end->run = end->left = end->glowRun = end->glowLeft = 0;
    end->palA = end->palB = end->palC = end->lightPal = end->glowPal = end->burstPal = 0;
    for (i = 0; i < 3; i++) end->layers[i] = NULL;
    end->glow = NULL;
    for (i = 0; i < DEFEAT_BURSTS; i++) end->bursts[i] = NULL;
}
