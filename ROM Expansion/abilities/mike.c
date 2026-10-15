/* Mike; NiDL's (from Mr. Tick-Tock), measured in NiDL: B makes Kirby stand and sing into the megaphone, then the microphone, then
 * (jumping 26 pixels up and back down) the microphone again, a different song each of the 3 times; then the ability is gone. While
 * he sings, every enemy on the screen is hit hard (a Crash-like hit) and the screen flashes. Each use: the start (24, 34 and 42
 * frames), the song (28, 28, 32 frames) and the end (5 frames); */
#include "../abilities.h"
#include "constants/songs.h"

#define MIKE_USES 3
#define END_FRAMES 5
#define JUMP_START 13           // The third use's jump: frames after the start;
#define JUMP_SPEED 0x455        // 4.27 pixels a frame, slowing by 0.36 a frame: 26 pixels up in 12 frames;
#define JUMP_GRAVITY 92

static const u8 sStartFrames[MIKE_USES] = { 24, 34, 42 }, sSongFrames[MIKE_USES] = { 28, 28, 32 };

// The flash while he sings (BLDY per frame): on and off every 4 frames;
static const u8 sFlash[32] = { 7, 7, 7, 7, 0, 0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0, 7, 7, 7, 7, 0, 0, 0, 0 };

static void MikeSing(struct Kirby *);

static bool32 MikeSinging(struct Kirby *kirby) {
    u8 use = NIDL_MIKE_USES(kirby);

    return kirby->stateFn == MikeSing && use < MIKE_USES && kirby->base.counter >= sStartFrames[use]
        && kirby->base.counter < sStartFrames[use] + sSongFrames[use];
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

    // The third use's jump;
    if (use == 2 && t >= JUMP_START && t < sStartFrames[use]) {
        if (t == JUMP_START) {
            kirby->base.yspeed = JUMP_SPEED;
            kirby->base.flags |= 0x20;
            kirby->base.unk62 &= ~4;
        } else if (kirby->base.unk62 & 4 && kirby->base.yspeed <= 0)
            kirby->base.yspeed = 0;
        else
            kirby->base.yspeed -= JUMP_GRAVITY;
    } else
        kirby->base.yspeed = 0;

    if (t == sStartFrames[use]) {
        PlaySfx(&kirby->base, SE_ABILITY_CRASH_ATTACK);
        NidlAttackCreate(kirby, MikeSinging, 0x10003, 0x10, FALSE, -120, -80, 120, 80);
        NidlFlash(kirby, sFlash, sSongFrames[use]);
    }
    if (t == sStartFrames[use] + sSongFrames[use])
        NidlMotion(kirby, MIKE_MOTION_1_END + use);
    if (t < sStartFrames[use] + sSongFrames[use] + END_FRAMES) return;

    if (++NIDL_MIKE_USES(kirby) >= MIKE_USES)
        NidlAbilityUsedUp(kirby);
    NidlAbilityEnd(kirby);
}
