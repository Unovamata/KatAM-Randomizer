/* Mike; NiDL's (from Mr. Tick-Tock; its script 0x080455C8, its 3 uses phases 1-3 of Kirby's +0x73, the first press's phase 0 taking
 * the megaphone out), frame by frame as NiDL's (recorded, B pressed each use): B makes Kirby stand and sing into the megaphone, then
 * the microphone, then (jumping 26 pixels up and back down) the microphone again, a different song each of the 3 times; then the
 * ability is gone. While he sings, every enemy on the screen is hit hard (a Crash-like hit) and the screen flashes. Each use, from
 * the frame B is pressed: the start (25, 33 and 43 frames: the singing frames show from there), the song (32 frames each) and the
 * end (5 frames: Kirby back on the 62nd, 70th and 80th frame, as NiDL's); */
#include "../abilities.h"
#include "constants/songs.h"

#define MIKE_USES 3
#define END_FRAMES 5
#define SONG_FRAMES 32
#define JUMP_START 14           // The third use's jump: its first frame (NiDL's: up 26 pixels, 4 frames there, back down by frame 42);
#define JUMP_FRAMES 28

static const u8 sStartFrames[MIKE_USES] = { 25, 33, 43 };

// The third use's jump: Kirby's height each frame from JUMP_START (pixels over where he stood; NiDL's, recorded);
static const u8 sJump[JUMP_FRAMES] = { 3, 7, 10, 12, 15, 17, 19, 21, 22, 24, 25, 25, 26, 26, 26, 26, 25, 25, 24, 22, 21, 19, 17, 15, 12, 10, 7, 3 };

// The flash while he sings (BLDY per frame, from the song's start): NiDL's, on its 1st-3rd, 5th-7th, then every other frame to the
// 15th, then the 19th and 23rd;
#define FLASH_FRAMES 24
static const u8 sFlash[FLASH_FRAMES] = { 0, 7, 7, 7, 0, 7, 7, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 0, 0, 7, 0, 0, 0, 7 };

static void MikeSing(struct Kirby *);

static bool32 MikeSinging(struct Kirby *kirby) {
    u8 use = NIDL_MIKE_USES(kirby);

    return kirby->stateFn == MikeSing && use < MIKE_USES && kirby->base.counter >= sStartFrames[use]
        && kirby->base.counter < sStartFrames[use] + SONG_FRAMES;
}

// MikeStart(); The B button;
void MikeStart(struct Kirby *kirby) {
    u8 use = NIDL_MIKE_USES(kirby);

    if (use >= MIKE_USES) use = NIDL_MIKE_USES(kirby) = MIKE_USES - 1;
    NidlMotion(kirby, MIKE_MOTION_1 + use);
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.xspeed = 0;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags |= 0x40;     // Stays where he is;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    kirby->stateFn = MikeSing;
}

static void MikeSing(struct Kirby *kirby) {
    u8 use = NIDL_MIKE_USES(kirby);
    u16 t;

    if (NidlAbilityInWater(kirby)) return;
    t = ++kirby->base.counter;
    kirby->base.xspeed = 0;

    // The third use's jump: NiDL's heights, frame by frame (the speed that takes him there this frame; up is positive);
    if (use == 2 && t >= JUMP_START && t <= JUMP_START + JUMP_FRAMES) {
        s32 now = t < JUMP_START + JUMP_FRAMES ? sJump[t - JUMP_START] : 0, before = t > JUMP_START ? sJump[t - JUMP_START - 1] : 0;

        kirby->base.yspeed = (now - before) * 0x100;
        kirby->base.flags |= 0x20;
        kirby->base.unk62 &= ~4;
    } else
        kirby->base.yspeed = 0;

    if (t == sStartFrames[use]) {
        PlaySfx(&kirby->base, SE_ABILITY_CRASH_ATTACK);
        NidlAttackCreate(kirby, MikeSinging, 0x10003, 0x10, FALSE, -120, -80, 120, 80);
        NidlFlash(kirby, sFlash, FLASH_FRAMES);
    }
    if (t == sStartFrames[use] + SONG_FRAMES)
        NidlMotion(kirby, MIKE_MOTION_1_END + use);
    if (t < sStartFrames[use] + SONG_FRAMES + END_FRAMES) return;

    if (++NIDL_MIKE_USES(kirby) >= MIKE_USES)
        NidlAbilityUsedUp(kirby);
    NidlAbilityEnd(kirby);
}
