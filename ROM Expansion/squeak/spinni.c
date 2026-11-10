/* Spinni; ported from Kirby: Squeak Squad (USA), the Squeak Squad's ninja, enemy 0x42 (its body 0x020AB6D0, vtable 0x02174DF8; brain
 * 0x020A5BB0; the things it throws 0x020ABE08, made by 0x020ABF10), as SQUEAK_SPINNI and SQUEAK_SPINNI_THROWN. Values from
 * param/prmDB01Ml0.kgb (movement, its things' too), prmDB01Pl0.kgb (brain) and prmDB01Sl0.kgb (HP 60); speeds 20.12 a frame, Y up.
 * Its body switches between two sprites (its +0xE4): enemy/spin and enemy/spin_at (its attacks and hurt poses); here one character
 * (KatAMExpansionSqueak.cs's ExtraFile), the shuriken in its hands in other colors a layer of its own (LayerPalettes).
 * • Brain (0x020A5B2C, 0x020A5AC4, then 0x020A5850 each frame): as the Squeakers' (0x020E8F9C), its target the room's nearest
 *   closed chest no one holds; none left to take, Kirby; no chest at all, or one in its hands, its hole (the stage's escape spot:
 *   here the spot it was placed at). Kirby within 64 pixels across and 33 up or down, not with a chest, it faces him (0x020A5838),
 *   waits (0x020A57B4: 60 frames half the time, none, 20, 30 or 45 a tenth, 45 a fifth) and attacks: half the time B (a shuriken),
 *   else up and B (its dive kick), then lets go (0x020A5780). Its pad (0x020A54F4): across toward the target beyond 8 pixels (on the
 *   ground or under a ceiling at any distance); the target higher: up, and 4 frames on A (higher than it's away across: a jump) or
 *   down and A (a long hop), at once in the air; lower: down; a wall ahead, up and A to a higher one, down to a lower; Kirby within 64
 *   across and 32 up or down without a chest, it stands. Within 32 pixels of its hole with a chest, its escape (pad 5, 5).
 * • Its pace (0x020A9E14, every 60 frames; 0x020A9EC0): heading for a chest or Kirby, off the screen 1.7 times its speeds, Kirby on
 *   the screen within 64 pixels half; heading home, off the screen half, Kirby near 1.4 times (the spawn's parameter byte 1: none).
 * • Body (gravity 0.17 a frame, falls no faster than 2.4): it stands (spin's anim 1: 0x020A975C, 0x020A9434), runs at 1.4 (anim 2:
 *   0x020A9388, 0x020A90C4), jumps (A: 4.0 up, anim 6, 7 at the top, 8 falling: 0x020A8C8C, 0x020A8AF4, 0x020A8A1C, 0x020A896C,
 *   0x020A8798; its pad steers it at 1.4, a turn once; A again in the air jumps again), long-hops (down and A: spin_at's anim 6, then
 *   1.8 forward, 3.0 up, 30 frames: 0x020A6F6C, 0x020A6E68, 0x020A6DD8). B: it stands 60 frames (0x020A73E0, 0x020A71CC), winds up
 *   (spin_at's anim 14: 0x020A70F4) and throws (anim 15: 0x020A7058) a shuriken 24 pixels ahead, 24 up, flying on at 2.2 through
 *   everything. Up and B: 60 frames, then up at 1.0 forward and 2.4 (spin_at's anim 11: 0x020A7688) 15 frames, then down at 3.0
 *   forward and 2.0 (anim 10: 0x020A7594, 0x020A74C4), a slash 4 ahead, 8 down from it, going on at 2.8, slowing 0.4 a frame. With a
 *   chest each move has its carrying animation (spin's 14-21).
 * • Kirby's inhale (its hit reaction, 0x020AAEA0, kinds 3-5): it can't be inhaled; it runs from him on the spot (anim 4: 0x020A65F4,
 *   0x020A6484) while he inhales it, turning (60 frames between turns).
 * • Hits: its HP down to 90% (prmDB01Ml0's 90) for the first time, it's knocked back: shaking 16 frames (spin_at's anim 8:
 *   0x020A6C20, 0x020A6B08), flung 2.0 away and 4.0 up (0x020A6910), landing dizzy (anim 4: 0x020A6794) 180 frames (shivering from
 *   120), then gets up (anim 5: 0x020A66D0); half the time its claws fly off (flung 1.0 forward, 2.5 up, lying till 300 frames
 *   are over, then bursting). Hit while dizzy, one time in five (not within 40 frames) it drops a treat (0x020A5CF0: Squeak Squad's
 *   foods 9, 11, 12 (95%) or a Maxim Tomato (item 6) flung up: KatAM's small food or Maxim Tomato).
 * • Chests (as the Squeakers', squeak/squeakers.c's helpers): touching one it takes it (spin's anim 28, 48 frames still: 0x020A9984,
 *   0x020A98C4), carries it 10 pixels behind it, 2 up (jumping: 6 behind, 8 down); at its hole it walks to it a pixel a frame
 *   (0x020A6320, 0x020A618C), goes in (anim 27: 0x020A6110), stops 96 frames and is gone with it (0x020A60D8). Hit, it drops it.
 * • Boxes (spin.rca, spin_at.rca, per animation frame).
 * Its sounds: Squeak Squad's (SqueakSfx). Not ported: its flinch on a hit that doesn't knock it back (0x020AA858: KatAM's hurt instead), its knockback at HP 0
 * and its 300 frames dizzy before it's gone (KatAM's defeat instead), its dizzy stars (0x020A26B8's 0x48), the stages' routes and
 * tunnels (anims 9-13, 22-26), its sprint and skid (a brain's +0x44: none of its), its rage when Kirby carries the stage's chest
 * (0x020A9C68: spin_col_ang's palette, 1.7 times its speeds, 40-frame waits), a strong inhale making it drop its chest (kind 4). */
#include "../squeak.h"
#include "chest.h"
#include "random.h"
#include "constants/object_types.h"

// Animations (spin.cea's, then spin_at.cea's: KatAMExpansionSqueak.cs's OnlyAnims, in this order);
enum {
    ANIM_STAND, ANIM_RUN, ANIM_SPRINT, ANIM_SKID, ANIM_JUMP, ANIM_APEX, ANIM_FALL, ANIM_STAND_CARRY, ANIM_RUN_CARRY,
    ANIM_SPRINT_CARRY, ANIM_SKID_CARRY, ANIM_JUMP_CARRY, ANIM_APEX_CARRY, ANIM_FALL_CARRY, ANIM_HOLE, ANIM_TAKE, AT_HURT, AT_HURT_2,
    AT_DIZZY, AT_GET_UP, AT_HOP_READY, AT_HOP, AT_KNOCKED, AT_DIVE, AT_DIVE_READY, AT_THROW_READY, AT_THROW, AT_POP,
};
// The things it throws (spin_at.cea's 16, 28, 12, 13);
enum { THROWN_ANIM_SHURIKEN, THROWN_ANIM_SLASH, THROWN_ANIM_CLAWS, THROWN_ANIM_CLAWS_DOWN };
enum { THROWN_SHURIKEN, THROWN_SHURIKEN_2, THROWN_SLASH, THROWN_CLAWS };

// prmDB01Ml0.kgb;
#define RUN_SPEED 0x1667
#define JUMP_SPEED 0x4000
#define GRAVITY 0x2B9
#define MAX_FALL 0x2800
#define TAKE_TIME 0x30
#define ESCAPE_TIME 0x60
#define KNOCK_PERCENT 90
#define PACE_TIME 0x3C
#define NEAR_KIRBY 0x40
#define FAST 0x1B34
#define SLOW 0x800
#define FLEE 0x1667
#define HOME_SLOW 0x800
#define CLAWS_X 0x1000
#define CLAWS_Y 0x2800
#define CLAWS_GRAVITY 0x19A
#define CLAWS_TIME 300
#define CLAWS_MAX_FALL -0x2800
#define SLASH_SPEED 0x2CCD
#define SLASH_SLOW -0x667
#define SHURIKEN_SPEED 0x2334
// prmDB01Pl0.kgb and the brain's own;
#define STEER_NEAR 0x8000
#define SIGHT_X 0x40000
#define SIGHT_Y 0x21000
#define STOP_X 0x40000
#define STOP_Y 0x20000
#define HOLE 0x20000
#define JUMP_DELAY 4
// The code's;
#define AIR_SPEED 0x1666                // 0x02158DA4's second (its +0x24C: 1);
#define FALL_CAP -0x2666
#define HOP_X 0x1CCD
#define HOP_Y 0x3000
#define HOP_TIME 30
#define TURN_WAIT 60
#define ATTACK_WAIT 60
#define DIVE_UP_X 0x1000
#define DIVE_UP_Y 0x2666
#define DIVE_UP_GRAVITY 0x19A
#define DIVE_RISE 14
#define DIVE_X 0x3000
#define DIVE_Y -0x2000
#define DIVE_GRAVITY 0x4CD
#define KNOCK_X 0x2000
#define SHAKE_TIME 16
#define DIZZY_TIME 180
#define SHIVER_FROM 120
#define TREAT_WAIT 40
#define TREAT_CHANCE 20
#define HOLD_X -0xA000                  // 0x02187130: its chest 10 pixels behind it, 2 up; 0x02187140: 6 behind, 8 down jumping;
#define HOLD_Y 0x2000
#define JUMP_HOLD_X -0x6000
#define JUMP_HOLD_Y -0x8000
#define STOP_HOLD_X -0xC000             // 12 behind while it stops (0x020A98C4);

enum {
    BODY_START, BODY_STAND, BODY_RUN_ENTRY, BODY_RUN, BODY_JUMP_ENTRY, BODY_JUMP, BODY_APEX, BODY_FALL_ENTRY, BODY_FALL,
    BODY_HOP_ENTRY, BODY_HOP_READY, BODY_HOP, BODY_THROW_ENTRY, BODY_THROW_WAIT, BODY_THROW_READY, BODY_THROWN, BODY_DIVE_ENTRY,
    BODY_DIVE_WAIT, BODY_DIVE_RISE, BODY_DIVE, BODY_BRACE_ENTRY, BODY_BRACE, BODY_KNOCK_ENTRY, BODY_KNOCK_SHAKE, BODY_KNOCK_FLY,
    BODY_DIZZY, BODY_GET_UP, BODY_TAKE, BODY_PAUSE, BODY_ESCAPE_ENTRY, BODY_TO_HOLE, BODY_INTO_HOLE, BODY_GONE,
};
enum { BRAIN_START, BRAIN_INIT, BRAIN_THINK, BRAIN_ATTACK, BRAIN_WAIT, BRAIN_PRESS };
enum { AIM_CHEST, AIM_KIRBY, AIM_HOLE };
#define PAD_ESCAPE 5
// Its pace (its +0x1E4);
enum { PACE_NONE, PACE_FAST, PACE_SLOW, PACE_FLEE, PACE_HOME_SLOW };

#define COUNT(s) (((s16 *)&(s)->vars[0])[0])    // 0x24E (and 0x184);
#define PAUSE(s) (((s16 *)&(s)->vars[0])[1])    // 0x278 and the state after it (0x240);
#define LAST_FLAGS(s) ((s)->vars[1])    // Its flags, unkC and HP as the last frame ended (a hit while dizzy undone: SpinniWatch);
#define PAUSE_NEXT(s) (((u8 *)&(s)->vars[2])[0])
#define PACE_TIMER(s) (((u8 *)&(s)->vars[2])[1])    // 0x238;
#define LAST_HP(s) (((s16 *)&(s)->vars[2])[1])
#define CHEST(s) (*(struct Chest **)&(s)->vars[3])  // 0x16C;
#define SWAY(s) (&(s)->vars[4])         // 0x22C, 0x230;
#define SPOT_X(s) ((s)->vars[6])        // 0x130, 0x134: where it shakes and lies dizzy;
#define SPOT_Y(s) ((s)->vars[7])
#define HOME_X(s) ((s)->vars[8])        // Its hole: where it was placed;
#define HOME_Y(s) ((s)->vars[9])
#define HARM(s) ((s)->vars[10])         // KatAM's unk68 from InitObject;
#define LAST_UNKC(s) ((s)->vars[11])
#define REGRAB(s) (((u8 *)&(s)->vars[12])[0])  // 0x1F8;
#define TREAT_TIMER(s) (((u8 *)&(s)->vars[12])[1])  // 0x280;
#define TURN(s) (((u8 *)&(s)->vars[12])[2])    // 0x234: frames before it turns again;
#define PACE(s) (((u8 *)&(s)->vars[12])[3])    // 0x1E4;
#define PHASE(s) (((u8 *)&(s)->vars[13])[0])   // 0x198: 0, then 1 once knocked back;
#define AIR_TURN(s) (((u8 *)&(s)->vars[13])[1])    // 0x250;
#define BOX(s) (((u8 *)&(s)->vars[13])[2])
#define FLAGS(s) (((u8 *)&(s)->vars[13])[3])   // FLAG_*;
#define WAIT(s) (((u8 *)&(s)->vars[14])[0])    // Its brain's 0x88, 0x9C (AX: the jump's delay) and 0x98 (MAXVY: which);
#define JUMP_TIMER(s) (((s8 *)&(s)->vars[14])[1])
#define JUMP_KIND(s) (((u8 *)&(s)->vars[14])[2])
#define DIZZY(s) (((u8 *)&(s)->vars[14])[3])    // 0x27C: lying dizzy (hits drop treats);
// Its flags;
#define FLAG_AIR 1                      // Its flag 0x40: in a jump or hop (its brain's jump delay off);
#define FLAG_HURT 2                     // In KatAM's hurt (SpinniWatch: a hit's first frame);
#define FLAG_BRACED 4                   // 0x25C: inhaled this frame;
#define FLAG_KNOCK 8                    // A knockback to start once KatAM's hurt is over;
// Its things;
#define T_COUNT(s) ((s)->vars[0])       // 0x134;
#define T_HARM(s) ((s)->vars[1])
#define T_BOX(s) (*(u8 *)&(s)->vars[2])

// Its boxes (spin.rca, spin_at.rca: per animation frame, the frames each box lasts);
static const struct SqueakBox sSpinniBoxes[] = {
    { { -6, -22, 15, 0 }, TRUE, 0 },
    { { -4, -22, 17, 0 }, TRUE, 0 },
    { { -8, -22, 13, 0 }, TRUE, 0 },
    { { -10, -21, 11, 1 }, TRUE, 0 },
    { { -6, -20, 15, 2 }, TRUE, 0 },
    { { -7, -23, 14, 0 }, TRUE, 0 },
    { { -5, -22, 16, 0 }, TRUE, 0 },
    { { -9, -27, 12, 0 }, TRUE, 0 },
    { { -10, -24, 11, 0 }, TRUE, 0 },
    { { -8, -20, 13, 2 }, TRUE, 0 },
    { { -5, -23, 16, 0 }, TRUE, 0 },
    { { -9, -16, 19, 0 }, TRUE, 0 },
    { { -7, -22, 14, 0 }, TRUE, 0 },
    { { -12, -27, 9, -5 }, FALSE, 0 },
    { { -14, -21, 7, 1 }, FALSE, 0 },
    { { -6, -17, 16, 1 }, FALSE, 0 },
    { { -9, -44, 12, -22 }, TRUE, 0 },
    { { -8, -33, 13, -11 }, TRUE, 0 },
    { { -4, -24, 17, -2 }, TRUE, 0 },
    { { -8, -25, 13, -3 }, TRUE, 0 },
    { { -7, -21, 17, -3 }, TRUE, 0 },
    { { -8, -22, 13, 0 }, FALSE, 0 },
    { { -6, -24, 24, 1 }, TRUE, 0 },
    { { -8, -26, 27, 0 }, TRUE, 0 },
    { { -8, -33, 27, 0 }, TRUE, 0 },
    { { -8, -35, 23, 0 }, TRUE, 0 },
    { { -14, -25, 7, -3 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sSpinniAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sSpinniAnim1[] = { { 32, 1 } };
static const struct SqueakBoxFrame sSpinniAnim2[] = { { 16, 0 } };
static const struct SqueakBoxFrame sSpinniAnim3[] = { { 1, 2 } };
static const struct SqueakBoxFrame sSpinniAnim4[] = { { 1, 3 } };
static const struct SqueakBoxFrame sSpinniAnim5[] = { { 12, 4 } };
static const struct SqueakBoxFrame sSpinniAnim6[] = { { 16, 0 } };
static const struct SqueakBoxFrame sSpinniAnim7[] = { { 1, 5 } };
static const struct SqueakBoxFrame sSpinniAnim8[] = { { 32, 6 } };
static const struct SqueakBoxFrame sSpinniAnim9[] = { { 16, 6 } };
static const struct SqueakBoxFrame sSpinniAnim10[] = { { 1, 7 } };
static const struct SqueakBoxFrame sSpinniAnim11[] = { { 1, 8 } };
static const struct SqueakBoxFrame sSpinniAnim12[] = { { 12, 9 } };
static const struct SqueakBoxFrame sSpinniAnim13[] = { { 16, 10 } };
static const struct SqueakBoxFrame sSpinniAnim14[] = { { 8, 11 } };
static const struct SqueakBoxFrame sSpinniAnim15[] = { { 8, 12 } };
static const struct SqueakBoxFrame sSpinniAnim16[] = { { 2, 13 } };
static const struct SqueakBoxFrame sSpinniAnim17[] = { { 2, 14 } };
static const struct SqueakBoxFrame sSpinniAnim18[] = { { 1, 15 } };
static const struct SqueakBoxFrame sSpinniAnim19[] = { { 2, 15 }, { 6, 12 }, { 1, 16 }, { 3, 16 }, { 4, 17 }, { 4, 10 } };
static const struct SqueakBoxFrame sSpinniAnim20[] = { { 3, 18 }, { 5, 19 } };
static const struct SqueakBoxFrame sSpinniAnim21[] = { { 1, 20 } };
static const struct SqueakBoxFrame sSpinniAnim22[] = { { 1, 21 } };
static const struct SqueakBoxFrame sSpinniAnim23[] = { { 9, 22 } };
static const struct SqueakBoxFrame sSpinniAnim24[] = { { 2, 23 }, { 2, 24 }, { 5, 25 } };
static const struct SqueakBoxFrame sSpinniAnim25[] = { { 9, 2 } };
static const struct SqueakBoxFrame sSpinniAnim26[] = { { 10, 2 } };
static const struct SqueakBoxFrame sSpinniAnim27[] = { { 8, 26 } };
static const struct SqueakBoxAnim sSpinniBoxAnims[] = {
    { sSpinniAnim0, 1 },
    { sSpinniAnim1, 1 },
    { sSpinniAnim2, 1 },
    { sSpinniAnim3, 1 },
    { sSpinniAnim4, 1 },
    { sSpinniAnim5, 1 },
    { sSpinniAnim6, 1 },
    { sSpinniAnim7, 1 },
    { sSpinniAnim8, 1 },
    { sSpinniAnim9, 1 },
    { sSpinniAnim10, 1 },
    { sSpinniAnim11, 1 },
    { sSpinniAnim12, 1 },
    { sSpinniAnim13, 1 },
    { sSpinniAnim14, 1 },
    { sSpinniAnim15, 1 },
    { sSpinniAnim16, 1 },
    { sSpinniAnim17, 1 },
    { sSpinniAnim18, 1 },
    { sSpinniAnim19, 6 },
    { sSpinniAnim20, 2 },
    { sSpinniAnim21, 1 },
    { sSpinniAnim22, 1 },
    { sSpinniAnim23, 1 },
    { sSpinniAnim24, 3 },
    { sSpinniAnim25, 1 },
    { sSpinniAnim26, 1 },
    { sSpinniAnim27, 1 },
};
static const struct SqueakBoxes sSpinniBoxData = { sSpinniBoxes, sSpinniBoxAnims, ARRAY_COUNT(sSpinniBoxAnims) };
static const struct SqueakBox sThrownBoxes[] = {
    { { -7, -9, 8, 9 }, TRUE, 0 },
    { { 2, -35, 11, -7 }, TRUE, 0 },
    { { -9, -9, 9, 7 }, FALSE, 0 },
    { { -10, -18, 5, 0 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sThrownAnim0[] = { { 8, 0 } };
static const struct SqueakBoxFrame sThrownAnim1[] = { { 7, 1 } };
static const struct SqueakBoxFrame sThrownAnim2[] = { { 16, 2 } };
static const struct SqueakBoxFrame sThrownAnim3[] = { { 1, 3 } };
static const struct SqueakBoxAnim sThrownBoxAnims[] = {
    { sThrownAnim0, 1 },
    { sThrownAnim1, 1 },
    { sThrownAnim2, 1 },
    { sThrownAnim3, 1 },
};
static const struct SqueakBoxes sThrownBoxData = { sThrownBoxes, sThrownBoxAnims, ARRAY_COUNT(sThrownBoxAnims) };

static void SpinniMain(struct Object *obj);
void sub_0809B6A8(struct Object *);     // KatAM's hurt (object.c's, static: build.sh links it);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

static struct Chest *Held(struct SqueakObject *s) {
    return SqueakHeldChest(&CHEST(s));
}

// Carry(); Its vtable +0x78 (0x020A5EEC): its chest at (dx, dy) from it, swaying;
static void Carry(struct SqueakObject *s, s32 dx, s32 dy) {
    SqueakCarryChest(s, &CHEST(s), SWAY(s), dx, dy);
}

// Play(); Spin's animation, with a chest its carrying one;
static void Play(struct SqueakObject *s, u8 anim, u8 carrying) {
    SqueakPlay(s, Held(s) ? carrying : anim);
}

/* InView(); 0x02209D00 (the camera's view, the margins of its table at 0x022275F4: 0 48 pixels either side, 16 above, 48 below; 4
 * none): its spot within KatAM's screen and those margins; */
static bool32 InView(struct SqueakObject *s, bool8 margins) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x - (DISPLAY_WIDTH / 2 << 8);
    s32 y = s->obj.base.y - level->viewportPosition.y - (DISPLAY_HEIGHT / 2 << 8);
    s32 side = margins ? 48 << 8 : 0, up = margins ? 16 << 8 : 0, down = margins ? 48 << 8 : 0;

    return x > -(DISPLAY_WIDTH / 2 * 0x100 + side) && x < DISPLAY_WIDTH / 2 * 0x100 + side
           && y > -(DISPLAY_HEIGHT / 2 * 0x100 + up) && y < DISPLAY_HEIGHT / 2 * 0x100 + down;
}

// OnScreen(); 0x02209D00's 4: its spot in the camera's view;
static bool32 OnScreen(struct SqueakObject *s) {
    return InView(s, FALSE);
}

// KirbyNear(); 0x020A2C24: on the screen, Kirby within 64 pixels of it (Squeak Squad's: in the stage's part next to its too);
static bool32 KirbyNear(struct SqueakObject *s) {
    s32 dx = (s->obj.kirby3->base.x - s->obj.base.x) >> 8, dy = (s->obj.kirby3->base.y - s->obj.base.y) >> 8;

    return OnScreen(s) && dx * dx + dy * dy < NEAR_KIRBY * NEAR_KIRBY;
}

// Pace(); 0x020A9E14: its pace every 60 frames (none for the spawn's parameter byte 1);
static void Pace(struct SqueakObject *s, bool8 home) {
    if (s->variant == 1) {
        PACE(s) = PACE_NONE;
        return;
    }
    if (PACE_TIMER(s) > 0) {
        PACE_TIMER(s)--;
        return;
    }
    PACE_TIMER(s) = PACE_TIME;
    if (!home) PACE(s) = !OnScreen(s) ? PACE_FAST : KirbyNear(s) ? PACE_SLOW : PACE_NONE;
    else PACE(s) = !OnScreen(s) ? PACE_HOME_SLOW : KirbyNear(s) ? PACE_FLEE : PACE_NONE;
}

// Paced(); 0x020A9EC0: a speed at its pace;
static s32 Paced(struct SqueakObject *s, s32 v) {
    static const s32 sTimes[] = { 0x1000, FAST, SLOW, FLEE, HOME_SLOW };

    return (v * sTimes[PACE(s)] + 0x800) >> 12;
}

// Gravity(); Its vtable +0x40 with MAX_FALL and GRAVITY: on the ground, still; else falling;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
        return;
    }
    s->ay = -GRAVITY;
    if (s->vy < -MAX_FALL) {
        s->vy = -MAX_FALL;
        s->ay = 0;
    }
}

static void Still(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// TurnBack(); Its pad back turns it round, not within 60 frames of the last turn (0x234);
static void TurnBack(struct SqueakObject *s) {
    if (s->padDir < 0 && TURN(s) == 0) {
        SqueakTurnAround(s);
        TURN(s) = TURN_WAIT;
    }
}

// AirControl(); 0x020A9D1C: `turns` (1): back turns it round, forward AIR_SPEED, none still across; 0: forward and back AIR_SPEED;
static void AirControl(struct SqueakObject *s, bool8 turns) {
    if (!turns) {
        if (s->padDir > 0) s->vx = AIR_SPEED;
        if (s->padDir < 0) s->vx = -AIR_SPEED;
        if (s->padDir == 0) s->vx = s->maxVx = s->ax = s->fx = 0;
        return;
    }
    s->ax = s->fx = 0;
    TurnBack(s);
    if (s->padDir > 0) {
        s->vx = AIR_SPEED;
        s->maxVx = 0;
    } else if (s->padDir == 0) {
        s->vx = s->maxVx = 0;
    }
}

static bool32 Escaping(struct SqueakObject *s) {
    if (s->padY == PAD_ESCAPE && s->padDir == PAD_ESCAPE) {
        SqueakSetState(s, BODY_ESCAPE_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// Attacks(); Its pad's attacks, without a chest: up and B its dive kick, B its shuriken;
static bool32 Attacks(struct SqueakObject *s) {
    if (Held(s)) return FALSE;
    if (s->padY >= 1 && s->padB) {
        SqueakSetState(s, BODY_DIVE_ENTRY);
        return TRUE;
    }
    if (s->padB) {
        SqueakSetState(s, BODY_THROW_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// Throw(); 0x020A6090: one of its things (kind), from its spot: the shurikens 24 ahead (or 0), 24 up; the slash 4 ahead, 8 down;
static void Throw(struct SqueakObject *s, u8 kind) {
    static const s32 sFrom[4][2] = { { 0x18000, 0x18000 }, { 0, 0x18000 }, { 0x4000, -0x8000 }, { 0, 0 } };

    SqueakSpawn(s, SQUEAK_SPINNI_THROWN, kind, sFrom[kind][0], sFrom[kind][1]);
}

// Treat(); 0x020A5CF0 (hit while dizzy): one time in five, not within 40 frames: a food (95%) or a Maxim Tomato flung up from it;
static void Treat(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    u32 pick;

    if (TREAT_TIMER(s) > 0 || Random() >= TREAT_CHANCE) return;
    TREAT_TIMER(s) = TREAT_WAIT;
    pick = Random();
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0,
                            pick < 95 ? OBJ_SMALL_FOOD : OBJ_TOMATO, pick < 95 ? RandLessThan(5) : 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static bool32 Body(struct SqueakObject *s) {
    s32 x, y;

    switch (s->state) {
    case BODY_START:        // 0x020A975C (vtable +0x3C);
        Play(s, ANIM_STAND, ANIM_STAND_CARRY);
        SqueakStop(s);
        SqueakMove(s);
        SqueakSetState(s, BODY_STAND);
        break;
    case BODY_STAND:        // 0x020A9434;
        if (Escaping(s) || Attacks(s)) return TRUE;
        if (s->padY < 0 && s->padA) {
            SqueakSetState(s, BODY_HOP_ENTRY);
            return TRUE;
        }
        if (s->padA == 1) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        if (s->padDir != 0) {
            SqueakSetState(s, BODY_RUN_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_RUN_ENTRY:    // 0x020A9388;
        Play(s, ANIM_RUN, ANIM_RUN_CARRY);
        SqueakSetState(s, BODY_RUN);
        return TRUE;
    case BODY_RUN:          // 0x020A90C4;
        if (Escaping(s) || Attacks(s)) return TRUE;
        s->vx = Paced(s, RUN_SPEED);
        if (s->padA == 1) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        TurnBack(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_JUMP_ENTRY:   // 0x020A8C8C;
        Play(s, ANIM_JUMP, ANIM_JUMP_CARRY);
        SqueakSfx(s, 0xF9);
        s->vy = 0;
        Still(s);
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        FLAGS(s) |= FLAG_AIR;
        AIR_TURN(s) = 0;
        SqueakSetState(s, BODY_JUMP);
        return TRUE;
    case BODY_JUMP:         // 0x020A8AF4: its pad steers it (a turn back once); falling, its top;
    case BODY_APEX:         // 0x020A8A1C;
        if (Escaping(s)) return TRUE;
        if (AIR_TURN(s) < 1 && s->padDir < 0) AIR_TURN(s) = 1;
        if (AIR_TURN(s) == 1) {
            AirControl(s, TRUE);
            AIR_TURN(s) = 2;
        } else {
            AirControl(s, FALSE);
        }
        if (s->state == BODY_JUMP) {
            if (s->vy >= 0) {
                SqueakMove(s);
                Carry(s, JUMP_HOLD_X, JUMP_HOLD_Y);
                break;
            }
            Play(s, ANIM_APEX, ANIM_APEX_CARRY);
            SqueakSetState(s, BODY_APEX);
            return TRUE;
        }
        if (s->animEnded) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_FALL_ENTRY:   // 0x020A896C;
        Play(s, ANIM_FALL, ANIM_FALL_CARRY);
        Still(s);
        s->vy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020A8798: down and A long-hops, A jumps again; landed, it stands or runs on;
        if (Escaping(s)) return TRUE;
        AirControl(s, TRUE);
        if (s->padY < 0 && s->padA) {
            SqueakSetState(s, BODY_HOP_ENTRY);
            return TRUE;
        }
        if (s->padA == 1) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        if (s->vy < FALL_CAP) {
            s->vy = FALL_CAP;
            s->ay = 0;
        }
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        if (SqueakOnGround(s)) {
            FLAGS(s) &= ~FLAG_AIR;
            SqueakSetState(s, s->vx == 0 ? BODY_START : BODY_RUN_ENTRY);
            s->vy = s->ay = s->ax = 0;
        }
        break;
    case BODY_HOP_ENTRY:    // 0x020A6F6C;
        SqueakPlay(s, AT_HOP_READY);
        SqueakStop(s);
        COUNT(s) = 0;
        SqueakSfx(s, 0x127);
        SqueakSetState(s, BODY_HOP_READY);
        return TRUE;
    case BODY_HOP_READY:    // 0x020A6E68: crouched, then off 1.8 forward, 3.0 up;
        if (s->animEnded) {
            FLAGS(s) |= FLAG_AIR;
            SqueakPlay(s, AT_HOP);
            s->vx = HOP_X;
            s->vy = HOP_Y;
            SqueakSetState(s, BODY_HOP);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_HOP:          // 0x020A6DD8: 30 frames (falling from its second);
        if (COUNT(s) == HOP_TIME) {
            FLAGS(s) &= ~FLAG_AIR;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (COUNT(s) > 1) Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        COUNT(s)++;
        break;
    case BODY_THROW_ENTRY:  // 0x020A73E0;
    case BODY_DIVE_ENTRY:   // 0x020A78B0;
        Play(s, ANIM_STAND, ANIM_STAND_CARRY);
        SqueakStop(s);
        COUNT(s) = ATTACK_WAIT;
        SqueakSetState(s, s->state == BODY_THROW_ENTRY ? BODY_THROW_WAIT : BODY_DIVE_WAIT);
        return TRUE;
    case BODY_THROW_WAIT:   // 0x020A71CC: 60 frames, then it winds up;
        TurnBack(s);
        if (COUNT(s) < 1) {
            SqueakPlay(s, AT_THROW_READY);
            SqueakSfx(s, 0x100);
            SqueakStop(s);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_THROW_READY);
            return TRUE;
        }
        COUNT(s)--;
        if (COUNT(s) == 7) SqueakSfx(s, 0x13F);
        Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_THROW_READY:  // 0x020A70F4;
        if (s->animEnded) {
            SqueakPlay(s, AT_THROW);
            SqueakSetState(s, BODY_THROWN);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_THROWN:       // 0x020A7058: its 7th frame, the shuriken;
        if (COUNT(s) == 6) {
            Throw(s, THROWN_SHURIKEN);
            COUNT(s) = 7;
        } else {
            COUNT(s)++;
        }
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_DIVE_WAIT:    // 0x020A7688: 60 frames, then up;
        TurnBack(s);
        if (COUNT(s) < 1) {
            SqueakPlay(s, AT_DIVE_READY);
            SqueakSfx(s, 0x13F);
            SqueakStop(s);
            s->vx = DIVE_UP_X;
            s->vy = DIVE_UP_Y;
            s->ay = -DIVE_UP_GRAVITY;
            COUNT(s) = 0;
            SqueakSetState(s, BODY_DIVE_RISE);
            return TRUE;
        }
        COUNT(s)--;
        Gravity(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_DIVE_RISE:    // 0x020A7594: 15 frames, then the dive;
        if (COUNT(s) > DIVE_RISE) {
            COUNT(s) = 0;
            SqueakSfx(s, 0x120);
            SqueakPlay(s, AT_DIVE);
            s->vx = DIVE_X;
            s->vy = DIVE_Y;
            s->ay = -DIVE_GRAVITY;
            SqueakSetState(s, BODY_DIVE);
            return TRUE;
        }
        COUNT(s)++;
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_DIVE:         // 0x020A74C4: its 6th frame, the slash; on the ground it stops; its 10 frames and animation over, it stands;
        if (COUNT(s) == 5) Throw(s, THROWN_SLASH);
        if (COUNT(s) > 9) {
            COUNT(s) = 0;
            if (s->animEnded) SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)++;
        SqueakMove(s);
        if (SqueakOnGround(s)) SqueakStop(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_BRACE_ENTRY:  // 0x020A65F4: it runs from the inhale on the spot;
        Play(s, ANIM_SPRINT, ANIM_SPRINT_CARRY);
        SqueakStop(s);
        SqueakSetState(s, BODY_BRACE);
        return TRUE;
    case BODY_BRACE:        // 0x020A6484: its back to the Kirby; no more inhaled, it starts over;
        if (s->animEnded) Play(s, ANIM_SPRINT, ANIM_SPRINT_CARRY);
        if (s->obj.kirby3->base.x < s->obj.base.x) {
            if ((s->obj.base.flags & 1) && TURN(s) == 0) {
                SqueakTurnAround(s);
                TURN(s) = TURN_WAIT;
            }
        } else if (s->obj.kirby3->base.x > s->obj.base.x && !(s->obj.base.flags & 1) && TURN(s) == 0) {
            SqueakTurnAround(s);
            TURN(s) = TURN_WAIT;
        }
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        if (!(FLAGS(s) & FLAG_BRACED)) {
            SqueakSetState(s, BODY_START);
            break;
        }
        FLAGS(s) &= ~FLAG_BRACED;
        break;
    case BODY_KNOCK_ENTRY:  // 0x020A6C20: half the time its claws fly off;
        SqueakPlay(s, AT_KNOCKED);
        SqueakStop(s);
        SPOT_X(s) = s->obj.base.x;
        SPOT_Y(s) = s->obj.base.y;
        COUNT(s) = 0;
        if (Random() < 50) Throw(s, THROWN_CLAWS);
        SqueakSetState(s, BODY_KNOCK_SHAKE);
        return TRUE;
    case BODY_KNOCK_SHAKE:  // 0x020A6B08: 16 frames shaking (3 up, 3 down), then flung away;
        if (COUNT(s) > SHAKE_TIME - 1) {
            SqueakStop(s);
            s->vy = JUMP_SPEED;
            s->ay = -GRAVITY;
            s->vx = s->obj.kirby3->base.x > s->obj.base.x ? (s->obj.base.flags & 1 ? KNOCK_X : -KNOCK_X)
                                                            : (s->obj.base.flags & 1 ? -KNOCK_X : KNOCK_X);
            SqueakSetState(s, BODY_KNOCK_FLY);
            return TRUE;
        }
        s->obj.base.x = SPOT_X(s);
        s->obj.base.y = SPOT_Y(s) + (COUNT(s) % 4 == 0 ? -0x300 : COUNT(s) % 4 == 2 ? 0x300 : 0);
        COUNT(s)++;
        break;
    case BODY_KNOCK_FLY:    // 0x020A6910: walls send it back; landed, dizzy;
        if (s->vy < -MAX_FALL + 1) {
            s->vy = -MAX_FALL;
            s->ay = 0;
        }
        if (s->terrain & 3) s->vx = -s->vx;
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        if (SqueakOnGround(s)) {
            DIZZY(s) = TRUE;
            SqueakPlay(s, AT_DIZZY);
            SqueakStop(s);
            SPOT_X(s) = s->obj.base.x;
            SPOT_Y(s) = s->obj.base.y;
            COUNT(s) = 0;
            SqueakSetState(s, BODY_DIZZY);
        }
        break;
    case BODY_DIZZY:        // 0x020A6794: 180 frames, shivering from 120 (2 pixels either way), then it gets up;
        if (COUNT(s) > DIZZY_TIME - 1) {
            SqueakPlay(s, AT_GET_UP);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_GET_UP);
            return TRUE;
        }
        x = SPOT_X(s);
        if (TREAT_TIMER(s) < 1 && COUNT(s) > SHIVER_FROM - 1) {
            if (COUNT(s) % 8 < 2) x += 0x200;
            else if (COUNT(s) % 8 < 6) x -= 0x200;
        }
        COUNT(s)++;
        s->obj.base.x = x;
        s->obj.base.y = SPOT_Y(s);
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_GET_UP:       // 0x020A66D0;
        if (s->animEnded && COUNT(s)++ > 0) {
            DIZZY(s) = FALSE;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_TAKE:         // 0x020A9984: stopped, still (PAUSE frames), then on (PAUSE_NEXT);
        SqueakPlay(s, ANIM_TAKE);
        SqueakStop(s);
        SqueakSfx(s, 0x104);
        SqueakSetState(s, BODY_PAUSE);
        return TRUE;
    case BODY_PAUSE:        // 0x020A98C4 (its chest 12 behind it);
        if (!Held(s)) {
            PAUSE(s) = 0;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (PAUSE(s) < 0) {
            PAUSE(s) = 0;
            SqueakSetState(s, PAUSE_NEXT(s));
            return TRUE;
        }
        PAUSE(s)--;
        SqueakMove(s);
        Carry(s, STOP_HOLD_X, 0);
        break;
    case BODY_ESCAPE_ENTRY: // 0x020A6320;
        SqueakPlay(s, ANIM_STAND_CARRY);
        SqueakStop(s);
        SqueakSfx(s, 0x141);
        SqueakSetState(s, BODY_TO_HOLE);
        return TRUE;
    case BODY_TO_HOLE:      // 0x020A618C: a pixel a frame each way, till within 2 across and 1 up or down;
        x = SQ_X(&s->obj);
        y = SQ_Y(&s->obj);
        if (x > HOME_X(s) - 0x2000 && x < HOME_X(s) + 0x2000 && y > HOME_Y(s) - 0x1000 && y < HOME_Y(s) + 0x1000) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_HOLE);
            SqueakSetState(s, BODY_INTO_HOLE);
            break;
        }
        x >>= 12;
        y >>= 12;
        if (HOME_X(s) >> 12 < x) x--;
        else if (x < HOME_X(s) >> 12) x++;
        if (HOME_Y(s) >> 12 < y) y--;
        else if (y < HOME_Y(s) >> 12) y++;
        s->obj.base.x = x << 8;
        s->obj.base.y = -y << 8;
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_INTO_HOLE:    // 0x020A6110: in, then 96 frames still, then gone;
        if (s->animEnded) {
            PAUSE(s) = ESCAPE_TIME;
            PAUSE_NEXT(s) = BODY_GONE;
            SqueakSetState(s, BODY_TAKE);
            return TRUE;
        }
        Carry(s, HOLD_X, HOLD_Y);
        break;
    case BODY_GONE:         // 0x020A60D8: gone with its chest (0x020A2968);
        if (Held(s)) CHEST(s)->obj.base.flags |= 0x1000;
        CHEST(s) = NULL;
        s->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void ClearPad(struct SqueakObject *s) {
    s->padDir = s->padY = s->padA = s->padB = 0;
}

// FaceKirby(); 0x020AB824: Kirby behind it, its pad back;
static void FaceKirby(struct SqueakObject *s) {
    s32 kirby = SQ_X(s->obj.kirby3), x = SQ_X(&s->obj);

    if ((kirby > x && (s->obj.base.flags & 1)) || (kirby < x && !(s->obj.base.flags & 1))) s->padDir = -1;
}

// Toward(); Its pad across toward tx (forward or back as it faces);
static void Toward(struct SqueakObject *s, s32 x, s32 tx) {
    if (tx < x) s->padDir = s->obj.base.flags & 1 ? 1 : -1;
    else if (x < tx) s->padDir = s->obj.base.flags & 1 ? -1 : 1;
}

// Steer(); 0x020A54F4;
static void Steer(struct SqueakObject *s, s32 tx, s32 ty) {
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj), kx = SQ_X(s->obj.kirby3), ky = SQ_Y(s->obj.kirby3), dx;

    if (tx < x + STEER_NEAR && x - STEER_NEAR < tx) s->padDir = 0;
    else Toward(s, x, tx);
    if (ty < y + STEER_NEAR && y - STEER_NEAR < ty) {
        s->padY = 0;
        s->padA = 0;
    } else if (y < ty) {
        s->padY = 1;
        if (JUMP_TIMER(s) == 0) {
            dx = x < tx ? tx - x : x - tx;
            JUMP_KIND(s) = dx < ty - y ? 1 : 2;
            JUMP_TIMER(s) = JUMP_DELAY;
        }
    } else {
        s->padY = -1;
        s->padA = 0;
    }
    if (FLAGS(s) & FLAG_AIR) JUMP_TIMER(s) = 0;
    if (--JUMP_TIMER(s) < 1) {
        if (JUMP_KIND(s) == 1) {
            s->padY = 0;
            s->padA = 1;
        } else if (JUMP_KIND(s) == 2) {
            s->padY = -1;
            s->padA = 1;
        }
        JUMP_KIND(s) = 0;
        JUMP_TIMER(s) = 0;
    }
    if (SqueakWallAhead(s)) {
        if (ty < y) {
            s->padA = 0;
            s->padY = -1;
        } else if (y < ty) {
            s->padY = 1;
            s->padA = 1;
        }
    }
    if (SqueakCeiling(s) || (s->terrain & 4)) Toward(s, x, tx);
    if (!Held(s) && kx < x + STOP_X && x - STOP_X < kx && ky < y + STOP_Y && y - STOP_Y < ky) s->padDir = s->padY = s->padA = 0;
}

// Think(); 0x020A5850;
static void Think(struct SqueakObject *s) {
    struct Chest *chest = NULL;
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj), kx = SQ_X(s->obj.kirby3), ky = SQ_Y(s->obj.kirby3), tx, ty;
    bool8 any = FALSE;
    u8 aim;
    u32 roll;

    if (!Held(s)) chest = SqueakNearestChest(s, &any);
    if (Held(s) || !any) {
        aim = AIM_HOLE;
        tx = HOME_X(s);
        ty = HOME_Y(s);
    } else if (chest) {
        aim = AIM_CHEST;
        SqueakChestFeet(chest, &tx, &ty);
    } else {
        aim = AIM_KIRBY;
        tx = kx;
        ty = ky;
    }
    Pace(s, aim == AIM_HOLE);
    if (!Held(s) && kx < x + SIGHT_X && x - SIGHT_X < kx && ky < y + SIGHT_Y && y - SIGHT_Y < ky) {
        roll = Random();
        if (roll < 50) WAIT(s) = 60;
        else if (roll < 60) WAIT(s) = 0;
        else if (roll < 70) WAIT(s) = 20;
        else if (roll < 80) WAIT(s) = 30;
        else WAIT(s) = 45;
        s->brain = BRAIN_ATTACK;
    }
    Steer(s, tx, ty);
    if (aim == AIM_HOLE && Held(s) && x > tx - HOLE && x < tx + HOLE && y > ty - HOLE && y < ty + HOLE) {
        ClearPad(s);
        s->padDir = s->padY = PAD_ESCAPE;
    }
}

static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_START:       // 0x020A5B2C;
        FaceKirby(s);
        s->brain = BRAIN_INIT;
        break;
    case BRAIN_INIT:        // 0x020A5AC4;
        s->padDir = 0;
        JUMP_TIMER(s) = JUMP_KIND(s) = 0;
        s->brain = BRAIN_THINK;
        break;
    case BRAIN_THINK:       // 0x020A5850;
        Think(s);
        break;
    case BRAIN_ATTACK:      // 0x020A5838;
        FaceKirby(s);
        s->brain = BRAIN_WAIT;
        break;
    case BRAIN_WAIT:        // 0x020A57B4: its wait, then B (half the time with up);
        if (WAIT(s) > 0) {
            WAIT(s)--;
            break;
        }
        WAIT(s) = 0;
        s->padDir = 0;
        s->padY = Random() < 50 ? 0 : 1;
        s->padB = 1;
        s->brain = BRAIN_PRESS;
        break;
    case BRAIN_PRESS:       // 0x020A5780;
        s->padDir = 0;
        if (WAIT(s) > 0) {
            WAIT(s)--;
            break;
        }
        s->padB = s->padY = 0;
        s->brain = BRAIN_THINK;
        break;
    }
}

/* SpinniMain(); Its brain and body; Kirby's inhale reaching it, it braces (its hit reaction to kinds 3-5, not while knocked back or
 * dizzy); a closed chest it touches, it takes (not within 10 touches of dropping one); */
static void SpinniMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Chest *chest;

    if (TREAT_TIMER(s) > 0) TREAT_TIMER(s)--;
    if (TURN(s) > 0) TURN(s)--;
    Held(s);
    if (s->state < BODY_KNOCK_ENTRY && NidlInhaler(&obj->base)) {
        FLAGS(s) |= FLAG_BRACED;
        if (s->state != BODY_BRACE) SqueakSetState(s, BODY_BRACE_ENTRY);
    }
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    if (!Held(s) && s->state < BODY_KNOCK_ENTRY && (chest = SqueakTouchedChest(s)) != NULL) {
        if (REGRAB(s) > 0) {
            REGRAB(s)--;
        } else {
            SqueakTakeChest(&CHEST(s), SWAY(s), chest);
            PAUSE(s) = TAKE_TIME;
            PAUSE_NEXT(s) = BODY_START;
            SqueakSetState(s, BODY_TAKE);
        }
    }
    SqueakSetBoxes(s, &sSpinniBoxData, HARM(s), &BOX(s));
    LAST_FLAGS(s) = obj->base.flags;
    LAST_UNKC(s) = obj->base.unkC;
    LAST_HP(s) = obj->unk80;
}

/* SpinniWatch(); Its pre-callback: KatAM's hit put it out of its own state, or it's beaten or gone: it drops its chest (then 10
 * touches before it takes one again); a hit's first frame, its HP down to 90% for the first time, a knockback once KatAM's hurt is
 * over; */
static void SpinniWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    /* (hit while dizzy: 0x020AAEA0's 0x27C, only its treat roll; KatAM's hit undone, its HP, flags and push back as they were, its
     * dizzy spell going on; only KatAM's plain hurt: a grab or its defeat go on as KatAM's); */
    if (DIZZY(s) && obj->unk78 == sub_0809B6A8 && !(FLAGS(s) & FLAG_HURT)) {
        Treat(s);
        obj->unk80 = LAST_HP(s);
        obj->base.flags = LAST_FLAGS(s);
        obj->base.unkC = LAST_UNKC(s);
        obj->unk98 = obj->unk9A = 0;
        obj->base.xspeed = obj->base.yspeed = 0;
        obj->base.objBase55 = 0;
        obj->unk78 = SpinniMain;
        return;
    }
    if (obj->unk78 != SpinniMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000)) {
        if (Held(s)) {
            SqueakDropChest(s, &CHEST(s));
            REGRAB(s) = 10;
        }
        if (!(FLAGS(s) & FLAG_HURT) && !(obj->base.flags & 0x1000)) {
            if (PHASE(s) == 0 && obj->unk80 > 0 && obj->unk80 * 100 <= gSqueakEnemyHp[s->enemy] * KNOCK_PERCENT) {
                PHASE(s) = 1;
                FLAGS(s) |= FLAG_KNOCK;
            }
        }
        FLAGS(s) |= FLAG_HURT;
    } else {
        FLAGS(s) &= ~FLAG_HURT;
    }
}

// SpinniGone(); Its end (squeak.c's SqueakDestroy): it drops its chest;
void SpinniGone(struct SqueakObject *s) {
    SqueakDropChest(s, &CHEST(s));
}

void SpinniStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, SpinniMain);
    obj->unk7C = SpinniWatch;
    if (!s->started) {
        HOME_X(s) = SQ_X(obj);
        HOME_Y(s) = SQ_Y(obj);
        s->brain = BRAIN_START;
    }
    FLAGS(s) &= ~(FLAG_AIR | FLAG_BRACED);
    if (FLAGS(s) & FLAG_KNOCK) {
        FLAGS(s) &= ~FLAG_KNOCK;
        SqueakSetState(s, BODY_KNOCK_ENTRY);
        return;
    }
    DIZZY(s) = FALSE;
    SqueakSetState(s, BODY_START);
}

/* SpinniInit(); Its box (spin.rca's, set each frame), facing left (0x020AB6D0's flag 4); KatAM's inhale can't take it (unk5C 0x20, as
 * Maiga's); */
void SpinniInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -22, 15, 0);
    ObjectSetBounds(&s->obj.base, -6, -20, 6, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    s->obj.base.unk5C |= 0x20;
    SqueakStandOnFloor(&s->obj);
}

/* Its things (0x020ABE08): the shuriken (0x020AB970, 0x020AB944) flies on at 2.2 through everything, gone 48 pixels off the screen
 * (0x02209D00); the slash (0x020ABA60, 0x020ABA20) at 2.8, slowing 0.4 a frame, till its animation's over; its claws knocked off
 * (0x020ABCD0, 0x020ABBD0, 0x020ABB1C; harmless) are flung 1.0 forward, 2.5 up, falling 0.1 a frame, lie where they land, and 300
 * frames on burst (0x020AB908); */
enum { THROWN_START, THROWN_FLY, THROWN_SLASHING, THROWN_TOSSED, THROWN_SITTING };

static bool32 Thrown(struct SqueakObject *s) {
    switch (s->state) {
    case THROWN_START:
        if (s->variant == THROWN_SLASH) {
            s->vx = SLASH_SPEED;
            s->ax = SLASH_SLOW;
            SqueakPlay(s, THROWN_ANIM_SLASH);
            SqueakSetState(s, THROWN_SLASHING);
        } else if (s->variant == THROWN_CLAWS) {
            SqueakPlay(s, THROWN_ANIM_CLAWS);
            s->vx = CLAWS_X;
            s->vy = CLAWS_Y;
            s->ay = -CLAWS_GRAVITY;
            T_COUNT(s) = CLAWS_TIME;
            SqueakSetState(s, THROWN_TOSSED);
        } else {
            s->vx = SHURIKEN_SPEED;
            SqueakPlay(s, THROWN_ANIM_SHURIKEN);
            SqueakSetState(s, THROWN_FLY);
        }
        return TRUE;
    case THROWN_FLY:
    case THROWN_SLASHING:
        SqueakMove(s);
        if (s->state == THROWN_SLASHING ? s->animEnded : !InView(s, TRUE)) s->obj.base.flags |= 0x1000;
        break;
    case THROWN_TOSSED:
    case THROWN_SITTING:
        if (T_COUNT(s) < 1) {
            SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
            s->obj.base.flags |= 0x1000;
            break;
        }
        T_COUNT(s)--;
        SqueakMove(s);
        if (s->state == THROWN_TOSSED) {
            if (SqueakOnGround(s)) {
                s->vx = 0;
                SqueakPlay(s, THROWN_ANIM_CLAWS_DOWN);
                SqueakSetState(s, THROWN_SITTING);
            }
        } else if (SqueakOnGround(s)) {
            s->vy = s->ay = 0;
        } else {
            s->ay = -CLAWS_GRAVITY;
            if (s->vy < CLAWS_MAX_FALL) {
                s->vy = CLAWS_MAX_FALL;
                s->ay = 0;
            }
        }
        break;
    }
    return FALSE;
}

static void SpinniThrownMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Thrown);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sThrownBoxData, T_HARM(s), &T_BOX(s));
}

void SpinniThrownStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, SpinniThrownMain);
    if (s->started) return;
    // (the shurikens and the slash through everything: 0x020AB970's and 0x020ABA60's flag 0x200);
    SqueakThroughTerrain(s, s->variant != THROWN_CLAWS);
    SqueakSetState(s, THROWN_START);
}

// SpinniThrownInit(); Its box (spin_at.rca's; set each frame), its terrain box;
void SpinniThrownInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -7, -9, 8, 9);
    ObjectSetBounds(&s->obj.base, -5, -5, 5, 5);
    T_HARM(s) = s->obj.base.unk68;
    /* (Kirby's body doesn't hurt it, nor shows its HP bar, 0x080365C8's flag 0x1000000: Squeak Squad's attack things go through him,
     * their hit handler 0x020B4964 taking no touch); */
    s->obj.base.unk5C |= 0x80;
    s->obj.base.flags |= 0x1000000;
}
