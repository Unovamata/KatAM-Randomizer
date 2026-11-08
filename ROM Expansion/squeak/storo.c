/* Storo; ported from Kirby: Squeak Squad (USA), the Squeak Squad's strongman, enemy 0x44 (its body 0x020E43C0, vtable 0x0217CF88),
 * as SQUEAK_STORO. Values from param/PrmDB03Ml0.kgb (Ml1 for variant 1), PrmDB03Sl0.kgb (HP 130); speeds 20.12 a frame, Y up. Its
 * body switches between three sprites (its +0xE4): enemy/stron, stron_atack (its mallet) and stron_dmg (its hurt poses); here one
 * character (KatAMExpansionSqueak.cs's ExtraFile), the mallet's wood and its sparkles' colors layers of their own (Layers).
 * • Its routine (0x020E1DF0, its start): a chest in its hands, it heads home (the spot it was placed at, Squeak Squad's escape hole);
 *   else Kirby within 64 pixels or no closed chest in the room (0x020E1D24), it heads for Kirby, else for the nearest chest. A leg
 *   (0x020E14F4, 0x020E13B4, 0x020E0F00): 20 frames standing, then it walks (1.0, its pace's) toward its target, turning toward it
 *   once in 120 frames at most, for 120 frames or till it's there (within 8 pixels), then thinks again. More above or under it than
 *   across for 120 frames, it jumps toward it (0x020E0DAC: 1.2 forward, 3.8 up); off a ledge, it falls (0x020E16B8).
 * • Heading for Kirby, him within 64 pixels across and up or down (0x020E0F00): half the time its mallet (0x020E0380, 0x020E0244,
 *   0x020E015C: 60 frames, then its swing; its 46th frame a shockwave rolls off along the ground 76 pixels ahead), else (and always
 *   with a chest in its hands) its body slam (0x020E0AB8-0x020E0440: 60 frames, a jump 1.55 forward and 5.78 up, 6 frames hanging at
 *   the top, slammed down at 0.77 a frame to 6.5, two shockwaves 32 pixels off either side, then a bounce 1.0 forward and 3.78 up).
 * • A chest touched, it takes it (0x020E20A8: its take pose 48 frames), carried over its head; home, its pose 96 frames, it's gone
 *   with it (0x020E1B30); hit, it drops it.
 * • Its pace (0x020E24A4, every 60 frames): heading home, off the screen 0.8 times its speed, Kirby within 80 pixels 1.5; else off
 *   the screen 2.0, Kirby near 0.5 (variant 1: no change).
 * • Kirby's inhale: it can't be inhaled; it curls up (stron_dmg's anim 1: 0x020E004C, 0x020DFE68), facing him and inching forward
 *   (0.12), a pixel up its 2nd and 3rd frames of 4 (Squeak Squad's weak inhale, kind 3, as KatAM's; its strong one, 2 pixels and
 *   a chest dropped after 60 frames, KatAM's Kirby lacks).
 * • Hits: a hit flinches it (0x020E35B4, 0x020E34EC: its hurt pose, 16 frames shaking on the spot: here KatAM's hurt, held where it
 *   was hit; then hopped 1.6 away, 2.0 up, falling till it lands); its HP down to 20% (variant 1: 30%) it's knocked out
 *   (0x020E2FF0-0x020E270C: untouchable, shaking 16 frames (KatAM's hurt), flung 2.0 away and 2.4 up, bouncing 2.0 on and 1.4 up,
 *   lying dizzy 180 frames, shivering from 120; then half the time it gets up, else it flexes 180 frames); hit while dizzy it takes
 *   no damage (KatAM's hurt undone); each hit one time in five (not within 40 frames) a treat (0x020DF95C: KatAM's small food).
 * • Variant 2 starts flexing 300 frames.
 * Not ported: its sounds, the screen's shakes, its elemental hurt poses (Kirby's fire, ice and spark: KatAM's hurt instead), its
 * dizzy stars, its knockback at HP 0 with 7 treats (KatAM's defeat instead), its anger when Kirby carries the stage's chest (stage
 * mode 2: stron_col_ang's palette, 2.0 times its speeds, 36 frames untouchable after each hit), the stages' routes (here straight to
 * its target), its leaving when the stage's Squeak chase is off. */
#include "../squeak.h"
#include "chest.h"
#include "random.h"
#include "constants/object_types.h"

// Animations (stron.cea's 1-13, then stron_atack.cea's 3 and stron_dmg.cea's 1 and 2: KatAMExpansionSqueak.cs's OnlyAnims);
enum {
    ANIM_IDLE, ANIM_IDLE_CARRY, ANIM_WALK, ANIM_WALK_CARRY, ANIM_JUMP, ANIM_AIR, ANIM_JUMP_CARRY, ANIM_AIR_CARRY, ANIM_FLEX,
    ANIM_LAND, ANIM_LYING, ANIM_GET_UP, ANIM_TAKE, ANIM_MALLET, ANIM_CURL, ANIM_HURT,
};

// PrmDB03Ml0.kgb (+0x28C; Ml1's for variant 1 where they differ);
#define KNOCK_PERCENT 20                // +0x08 (30);
#define WALK_SPEED 0x1000               // +0x0C;
#define CLIMB_X 0x1334                  // +0x10..+0x18: its jump toward a target above or under it;
#define CLIMB_Y 0x3CCD                  // (0x2667);
#define GRAVITY 0x19A
#define MALLET_WAIT 60                  // +0x1C, +0x20;
#define SLAM_WAIT 60
#define SLAM_X 0x18CD                   // +0x24..+0x44: its body slam;
#define SLAM_Y 0x5C7B                   // (0x4C7B);
#define SLAM_RISE 0x2B9
#define SLAM_DOWN 0xC52
#define SLAM_CAP 0x6800
#define BOUNCE_X 0x1000
#define BOUNCE_Y 0x3C7B
#define BOUNCE_RISE 0x6B9
#define BOUNCE_CAP 0x4800
#define TAKE_TIME 48                    // +0x48, +0x4C;
#define HOLE_TIME 96
#define PACE_TIME 60                    // +0x54;
#define NEAR_KIRBY 80                   // +0x58, +0x68;
#define FAST 0x2000                     // +0x60..+0x74;
#define SLOW 0x800
#define HOME_FAST 0x1800
#define HOME_SLOW 0xCCD
// Its code's;
#define MAX_FALL 0x2800                 // Its vtable +0x40's (-2.5, -0.1);
#define LEG_TIME 120
#define STAND_TIME 20
#define TURN_WAIT 120
#define CLIMB_WAIT 120
#define HANG_TIME 6
#define MALLET_HIT 46                   // 0x020E015C: its swing's 46th frame;
#define MALLET_X 0x4C000
#define STAR_X 0x20000                  // 0x020A25C4: the slam's shockwaves;
#define ATTACK_RANGE 0x40000            // 0x020E0F00: Kirby within 64 pixels;
#define ARRIVED 0x8000                  // Within 8 pixels of its target;
#define HOME_RANGE 0x20000              // Within 32 pixels of its spot;
#define FLINCH_X 0x199A                 // 0x020E34EC;
#define FLINCH_Y 0x2000
#define FLINCH_RISE 0x2B8
#define KNOCK_X 0x2000                  // 0x020E2FF0, 0x020E2C04;
#define KNOCK_Y 0x2666
#define KNOCK_BOUNCE_Y 0x1666
#define DIZZY_TIME 180                  // 0x020E270C;
#define SHIVER_TIME 120
#define FLEX_TIME 180
#define FLEX_START 300                  // Variant 2's;
#define BRACE_SPEED 0x1EC               // 0x020DFE68;
#define TREAT_CHANCE 20                 // 0x020DF95C;
#define TREAT_WAIT 40
#define CARRY_Y 0x30000                 // Its chest over its head;

enum {
    BODY_START, BODY_STAND, BODY_WALK, BODY_CLIMB_RISE, BODY_CLIMB_FALL, BODY_FALL_ENTRY, BODY_FALL, BODY_LANDED, BODY_MALLET_WAIT, BODY_MALLET,
    BODY_SLAM_WAIT, BODY_SLAM_RISE, BODY_SLAM_HANG, BODY_SLAM_FALL, BODY_BOUNCE_RISE, BODY_BOUNCE_FALL, BODY_TAKE, BODY_GONE_HOME,
    BODY_FLEX, BODY_BRACE, BODY_FLINCH_ENTRY, BODY_FLINCH_RISE, BODY_FLINCH_FALL, BODY_KNOCK_ENTRY, BODY_KNOCK_RISE,
    BODY_KNOCK_FALL, BODY_KNOCK_BOUNCE_RISE, BODY_KNOCK_BOUNCE_FALL, BODY_LYING, BODY_GET_UP,
};
// Its target (0x020D8C08's point): Kirby, a chest, its spot;
enum { GOAL_KIRBY, GOAL_CHEST, GOAL_HOME };

#define COUNT(s) (((s16 *)&(s)->vars[0])[0])
#define LEG(s) (((s16 *)&(s)->vars[0])[1])          // +0x274;
#define LAST_FLAGS(s) ((s)->vars[1])
#define NEXT(s) (((u8 *)&(s)->vars[2])[0])
#define GOAL(s) (((u8 *)&(s)->vars[2])[1])          // +0x248;
#define FLAGS(s) (((u8 *)&(s)->vars[2])[2])
#define BOX(s) (((u8 *)&(s)->vars[2])[3])
#define CHEST(s) (*(struct Chest **)&(s)->vars[3])  // +0x1F4;
#define SWAY(s) (&(s)->vars[4])
#define GOAL_X(s) ((s)->vars[6])
#define GOAL_Y(s) ((s)->vars[7])
#define HOME_X(s) ((s)->vars[8])                    // +0x240, +0x244;
#define HOME_Y(s) ((s)->vars[9])
#define HARM(s) ((s)->vars[10])
#define LAST_UNKC(s) ((s)->vars[11])
#define SPOT_X(s) ((s)->vars[12])                   // Where it was hit (KatAM's hurt holds it there);
#define SPOT_Y(s) ((s)->vars[13])
#define TURN_TIMER(s) (((u8 *)&(s)->vars[14])[0])   // +0x230;
#define CLIMB_TIMER(s) (((u8 *)&(s)->vars[14])[1])  // +0x250;
#define PACE(s) (((u8 *)&(s)->vars[14])[2])         // +0x1E4;
#define PACE_TIMER(s) (((u8 *)&(s)->vars[14])[3])   // +0x234;
#define TREAT_TIMER(s) (((u8 *)&(s)->vars[15])[0])  // +0x288;
#define REGRAB(s) (((u8 *)&(s)->vars[15])[1])       // +0x1F8;
#define PHASE(s) (((u8 *)&(s)->vars[15])[2])        // +0x198: 0, 1 once knocked out;
#define DIZZY(s) (((u8 *)&(s)->vars[15])[3])        // +0x22C;
#define AWAY(s) (((u8 *)&(s)->vars[16])[0])         // +0x190: the hit's side;
#define INHALED(s) (((u8 *)&(s)->vars[16])[1])      // +0x268;
#define LAST_HP(s) (((s16 *)&(s)->vars[17])[0])
// Its flags;
#define FLAG_HURT 1
#define FLAG_KNOCK 2

// Boxes (stron.rca's, stron_atack.rca's and stron_dmg.rca's: boxes.py);
static const struct SqueakBox sStoroBoxes[] = {
    { { -23, -44, 29, 1 }, TRUE, 0 },
    { { -23, -38, 29, 1 }, TRUE, 0 },
    { { -23, -42, 29, 1 }, TRUE, 0 },
    { { -23, -49, 29, -6 }, TRUE, 0 },
    { { -23, -51, 29, 1 }, TRUE, 0 },
    { { -23, -51, 29, -5 }, TRUE, 0 },
    { { -23, -50, 29, 1 }, TRUE, 0 },
    { { -23, -44, 29, 1 }, TRUE, 0 },       // 7: stron_atack's;
    { { -23, -92, 29, 1 }, TRUE, 0 },
    { { -23, -91, 29, 1 }, TRUE, 0 },
    { { -23, -68, 70, 1 }, TRUE, 0 },       // 10-12: its swing, reaching 70 pixels ahead;
    { { -23, -55, 70, 1 }, TRUE, 0 },
    { { -23, -44, 72, 1 }, TRUE, 0 },
    { { -23, -45, 19, -2 }, FALSE, 0 },     // 13: stron_dmg's;
    { { -16, -44, 26, -1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sStoroBox0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sStoroBox1[] = { { 1, 1 } };
static const struct SqueakBoxFrame sStoroBox2[] = { { 1, 2 } };
static const struct SqueakBoxFrame sStoroGetUp[] = { { 2, 2 }, { 2, 3 }, { 1, 4 }, { 1, 5 }, { 2, 6 }, { 2, 0 } };
static const struct SqueakBoxFrame sStoroMallet[] = { { 10, 7 }, { 2, 8 }, { 30, 8 }, { 4, 9 }, { 2, 10 }, { 2, 11 }, { 14, 12 } };
static const struct SqueakBoxFrame sStoroCurl[] = { { 1, 13 } };
static const struct SqueakBoxFrame sStoroHurt[] = { { 1, 14 } };
static const struct SqueakBoxAnim sStoroBoxAnims[] = {
    { sStoroBox0, 1 }, { sStoroBox0, 1 }, { sStoroBox0, 1 }, { sStoroBox0, 1 }, { sStoroBox0, 1 }, { sStoroBox0, 1 },
    { sStoroBox0, 1 }, { sStoroBox0, 1 }, { sStoroBox1, 1 }, { sStoroBox2, 1 }, { sStoroBox1, 1 }, { sStoroGetUp, 6 },
    { sStoroBox0, 1 }, { sStoroMallet, 7 }, { sStoroCurl, 1 }, { sStoroHurt, 1 },
};
static const struct SqueakBoxes sStoroBoxData = { sStoroBoxes, sStoroBoxAnims, ARRAY_COUNT(sStoroBoxAnims) };

static void StoroMain(struct Object *obj);
void sub_0809B6A8(struct Object *);     // KatAM's hurt (object.c's, static: build.sh links it);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

static struct Chest *Held(struct SqueakObject *s) {
    return SqueakHeldChest(&CHEST(s));
}

static void Carry(struct SqueakObject *s) {
    SqueakCarryChest(s, &CHEST(s), SWAY(s), 0, CARRY_Y);
}

static void Play(struct SqueakObject *s, u8 anim) {
    SqueakPlay(s, Held(s) && anim <= ANIM_AIR_CARRY ? anim + 1 : anim);
}

static s32 KirbyX(struct SqueakObject *s) {
    return SQ_X(s->obj.kirby3);
}

static s32 KirbyY(struct SqueakObject *s) {
    return SQ_Y(s->obj.kirby3);
}

static void SetPos(struct SqueakObject *s, s32 x, s32 y) {
    s->obj.base.x = x >> 4;
    s->obj.base.y = -y >> 4;
}

static void Still(struct SqueakObject *s) {
    s->vx = s->vy = s->ax = s->ay = s->maxVx = s->maxVy = s->fx = s->fy = 0;
}

static bool32 FacesLeft(struct SqueakObject *s) {
    return (s->obj.base.flags & 1) != 0;
}

// FaceToward(); Its turn toward `x`, once in 120 frames at most (+0x230);
static void FaceToward(struct SqueakObject *s, s32 x) {
    s32 here = SQ_X(&s->obj);

    if (TURN_TIMER(s) != 0) return;
    if (FacesLeft(s) ? here < x : x < here) {
        SqueakTurnAround(s);
        TURN_TIMER(s) = TURN_WAIT;
    }
}

// OnScreen(); 0x020A2D0C: its spot in the camera's view;
static bool32 OnScreen(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x, y = s->obj.base.y - level->viewportPosition.y;

    return x >= 0 && x <= DISPLAY_WIDTH * 0x100 && y >= 0 && y <= DISPLAY_HEIGHT * 0x100;
}

static bool32 KirbyNear(struct SqueakObject *s) {
    s32 dx = (s->obj.kirby3->base.x - s->obj.base.x) >> 8, dy = (s->obj.kirby3->base.y - s->obj.base.y) >> 8;

    return OnScreen(s) && dx * dx + dy * dy < NEAR_KIRBY * NEAR_KIRBY;
}

// Pace(); 0x020E24A4: every 60 frames;
static void Pace(struct SqueakObject *s) {
    if (PACE_TIMER(s) > 0) {
        PACE_TIMER(s)--;
        return;
    }
    PACE_TIMER(s) = PACE_TIME;
    if (GOAL(s) == GOAL_HOME) PACE(s) = !OnScreen(s) ? 4 : KirbyNear(s) ? 3 : 0;
    else PACE(s) = !OnScreen(s) ? 1 : KirbyNear(s) ? 2 : 0;
}

// Paced(); 0x020E253C: a speed at its pace (variant 1's paces are all 1.0);
static s32 Paced(struct SqueakObject *s, s32 v) {
    static const s32 sTimes[] = { 0x1000, FAST, SLOW, HOME_FAST, HOME_SLOW };

    if (s->variant == 1 || PACE(s) == 0) return v;
    return (v * sTimes[PACE(s)] + 0x800) >> 12;      // (its speeds are small: no overflow);
}

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

static void CapFall(struct SqueakObject *s, s32 cap) {
    if (s->vy < -cap + 1) {
        s->vy = -cap;
        s->ay = 0;
    }
}

// Away(); A speed away from the hit's side (forward when it faces that way);
static s32 Away(struct SqueakObject *s, s32 v) {
    return AWAY(s) == !!(s->obj.base.flags & 1) ? v : -v;
}

// Treat(); 0x020DF95C: one hit in five (not within 40 frames of the last), a food flung up from it;
static void Treat(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (TREAT_TIMER(s) > 0 || Random() >= TREAT_CHANCE) return;
    TREAT_TIMER(s) = TREAT_WAIT;
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, OBJ_SMALL_FOOD,
                            RandLessThan(5), 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static bool32 AnyChest(struct SqueakObject *s, struct Chest **chest) {
    bool8 any = FALSE;

    *chest = SqueakNearestChest(s, &any);
    return *chest != NULL;
}

static bool32 Near(struct SqueakObject *s, s32 x, s32 y, s32 range) {
    s32 here = SQ_X(&s->obj), there = SQ_Y(&s->obj);

    return x - range < here && here < x + range && y - range < there && there < y + range;
}

// Leg(); 0x020E14F4: standing 20 frames, then walking toward its goal 120 frames at most;
static void Leg(struct SqueakObject *s) {
    Play(s, ANIM_IDLE);
    Still(s);
    COUNT(s) = 0;
    LEG(s) = LEG_TIME;
    SqueakSetState(s, BODY_STAND);
}

// Landed(); After a jump or a fall: standing out the rest of its leg (0x020E19BC);
static void Landed(struct SqueakObject *s) {
    Play(s, ANIM_IDLE);
    Still(s);
    SqueakSetState(s, BODY_LANDED);
}

static bool32 Body(struct SqueakObject *s) {
    s32 x, y, dx, dy;
    struct Chest *chest;

    switch (s->state) {
    case BODY_START:        // 0x020E1DF0, 0x020E1D24, 0x020E1C68: its goal (Kirby, a chest, home), then a leg toward it;
        SqueakThroughTerrain(s, FALSE);
        if (Held(s)) {
            GOAL(s) = GOAL_HOME;
            GOAL_X(s) = HOME_X(s);
            GOAL_Y(s) = HOME_Y(s);
        } else if (Near(s, KirbyX(s), KirbyY(s), ATTACK_RANGE) || !AnyChest(s, &chest)) {
            GOAL(s) = GOAL_KIRBY;
            GOAL_X(s) = KirbyX(s);
            GOAL_Y(s) = KirbyY(s);
        } else {
            GOAL(s) = GOAL_CHEST;
            SqueakChestFeet(chest, &GOAL_X(s), &GOAL_Y(s));
        }
        NEXT(s) = BODY_START;
        Leg(s);
        return TRUE;
    case BODY_STAND:        // 0x020E13B4: 20 frames, then its walk;
        Gravity(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            return TRUE;
        }
        if (COUNT(s)++ >= STAND_TIME) {
            Play(s, ANIM_WALK);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_WALK);
        }
        Carry(s);
        break;
    case BODY_WALK:         // 0x020E0F00;
        FaceToward(s, GOAL_X(s));
        s->vx = Paced(s, WALK_SPEED);
        Gravity(s);
        x = SQ_X(&s->obj);
        y = SQ_Y(&s->obj);
        dx = GOAL_X(s) > x ? GOAL_X(s) - x : x - GOAL_X(s);
        dy = GOAL_Y(s) > y ? GOAL_Y(s) - y : y - GOAL_Y(s);
        // (more above or under it than across 120 frames: a jump toward it);
        if (dx < dy) {
            if (CLIMB_TIMER(s) == 0) {
                CLIMB_TIMER(s) = CLIMB_WAIT;
                Play(s, ANIM_JUMP);
                s->vx = Paced(s, CLIMB_X);
                s->vy = s->variant == 1 ? 0x2667 : CLIMB_Y;
                s->ay = -GRAVITY;
                FaceToward(s, GOAL_X(s));
                SqueakSetState(s, BODY_CLIMB_RISE);
                return TRUE;
            }
            CLIMB_TIMER(s)--;
        }
        // (heading for Kirby or a chest, him within 64 pixels: an attack; home, within 32 pixels of its spot: in, with its chest);
        if (GOAL(s) != GOAL_HOME && Near(s, KirbyX(s), KirbyY(s), ATTACK_RANGE) && SqueakOnGround(s)) {
            NEXT(s) = BODY_START;
            Play(s, ANIM_IDLE);
            Still(s);
            COUNT(s) = 0;
            SqueakSetState(s, !Held(s) && Random() > 49 ? BODY_MALLET_WAIT : BODY_SLAM_WAIT);
            return TRUE;
        }
        if (GOAL(s) == GOAL_HOME && Near(s, HOME_X(s), HOME_Y(s), HOME_RANGE)) {
            Play(s, ANIM_TAKE);
            Still(s);
            COUNT(s) = HOLE_TIME;
            NEXT(s) = BODY_GONE_HOME;
            SqueakSetState(s, BODY_TAKE);
            return TRUE;
        }
        if (Near(s, GOAL_X(s), GOAL_Y(s), ARRIVED)) {
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        SqueakMove(s);
        if (!SqueakOnGround(s) && s->vy < 0) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            return TRUE;
        }
        Carry(s);
        if (LEG(s)-- < 1) SqueakSetState(s, NEXT(s));
        break;
    case BODY_CLIMB_RISE:   // 0x020E0C54;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (s->vy < 0) {
            Play(s, ANIM_AIR);
            SqueakSetState(s, BODY_CLIMB_FALL);
        }
        Carry(s);
        break;
    case BODY_FALL_ENTRY:   // 0x020E16B8: off a ledge;
        Play(s, ANIM_AIR);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_CLIMB_FALL:   // 0x020E0B88;
    case BODY_FALL:         // 0x020E15AC;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSpawnEffect(s, SQUEAK_FX_LAND_BIG, 0, 0, FALSE);
            Landed(s);
            return TRUE;
        }
        Carry(s);
        break;
    case BODY_LANDED:       // 0x020E19BC, 0x020E1904: the rest of its leg standing, then on;
        Gravity(s);
        SqueakMove(s);
        Carry(s);
        if (LEG(s)-- < 1) SqueakSetState(s, NEXT(s));
        break;
    case BODY_MALLET_WAIT:  // 0x020E0244: 60 frames, then its swing (stron_atack's anim 3);
        Gravity(s);
        SqueakMove(s);
        if (COUNT(s)++ >= MALLET_WAIT) {
            SqueakPlay(s, ANIM_MALLET);
            Still(s);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_MALLET);
        }
        break;
    case BODY_MALLET:       // 0x020E015C: its 46th frame, a shockwave 76 pixels ahead; its animation over, on;
        Gravity(s);
        SqueakMove(s);
        if (COUNT(s)++ == MALLET_HIT) SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, MALLET_X, 0);
        if (s->animEnded) {
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        break;
    case BODY_SLAM_WAIT:    // 0x020E096C: 60 frames, then up;
        Gravity(s);
        SqueakMove(s);
        if (COUNT(s)++ >= SLAM_WAIT) {
            Play(s, ANIM_JUMP);
            Still(s);
            s->vx = SLAM_X;
            s->vy = s->variant == 1 ? 0x4C7B : SLAM_Y;
            s->ay = -SLAM_RISE;
            SqueakSetState(s, BODY_SLAM_RISE);
        }
        Carry(s);
        break;
    case BODY_SLAM_RISE:    // 0x020E083C: at the top, still 6 frames;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (s->vy < 0) {
            Still(s);
            Play(s, ANIM_IDLE);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_SLAM_HANG);
        }
        Carry(s);
        break;
    case BODY_SLAM_HANG:    // 0x020E07A0;
        SqueakMove(s);
        if (COUNT(s)++ > HANG_TIME - 1) {
            s->ay = -SLAM_DOWN;
            SqueakSetState(s, BODY_SLAM_FALL);
        }
        Carry(s);
        break;
    case BODY_SLAM_FALL:    // 0x020E06D4: landed, two shockwaves (0x020A25C4), its big puff, then its bounce;
        SqueakMove(s);
        CapFall(s, SLAM_CAP);
        if (SqueakOnGround(s)) {
            SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, STAR_X, 0);
            SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_BEHIND, -STAR_X, 0);
            SqueakSpawnEffect(s, SQUEAK_FX_LAND_BIG, 0, 0, FALSE);
            Play(s, ANIM_JUMP);
            Still(s);
            s->vx = BOUNCE_X;
            s->vy = BOUNCE_Y;
            s->ay = -BOUNCE_RISE;
            SqueakSetState(s, BODY_BOUNCE_RISE);
        }
        Carry(s);
        break;
    case BODY_BOUNCE_RISE:  // 0x020E0500;
        CapFall(s, BOUNCE_CAP);
        SqueakMove(s);
        if (s->vy < 0) {
            Play(s, ANIM_AIR);
            SqueakSetState(s, BODY_BOUNCE_FALL);
        }
        Carry(s);
        break;
    case BODY_BOUNCE_FALL:  // 0x020E0440: landed, its puff, on;
        CapFall(s, BOUNCE_CAP);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSpawnEffect(s, SQUEAK_FX_LAND, 0, 0, FALSE);
            Play(s, ANIM_IDLE);
            Still(s);
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        Carry(s);
        break;
    case BODY_TAKE:         // 0x020E20A8, 0x020E2010: its pose (COUNT frames), then NEXT;
        Gravity(s);
        SqueakMove(s);
        Carry(s);
        if (COUNT(s)-- < 0) {
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        break;
    case BODY_GONE_HOME:    // 0x020E1B30: into its hole with its chest;
        if (Held(s)) CHEST(s)->obj.base.flags |= 0x1000;
        CHEST(s) = NULL;
        s->obj.base.flags |= 0x1000;
        break;
    case BODY_FLEX:         // 0x020E1820, 0x020E1770: COUNT frames, then on;
        Gravity(s);
        SqueakMove(s);
        Carry(s);
        if (COUNT(s)-- < 1) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    case BODY_BRACE:        // 0x020DFE68: curled up, inching forward, a pixel up its 2nd and 3rd frames of 4; the inhale over, 20 frames standing;
        FaceToward(s, KirbyX(s));
        s->vx = BRACE_SPEED;
        Gravity(s);
        SqueakMove(s);
        if (++COUNT(s) > 4) COUNT(s) = 1;
        if (!INHALED(s)) {
            SetPos(s, SQ_X(&s->obj), SPOT_Y(s));
            TURN_TIMER(s) = 60;
            LEG(s) = STAND_TIME;
            NEXT(s) = BODY_START;
            Landed(s);
            return TRUE;
        }
        SetPos(s, SQ_X(&s->obj), SPOT_Y(s) + (COUNT(s) == 2 || COUNT(s) == 3 ? 0x1000 : 0));
        INHALED(s) = FALSE;
        Carry(s);
        break;
    case BODY_FLINCH_ENTRY: // 0x020E35B4: its hurt pose, its 16 frames' shake KatAM's hurt (StoroWatch), then hopped away;
        SqueakPlay(s, Random() < 50 ? ANIM_CURL : ANIM_HURT);
        Still(s);
        s->vx = Away(s, FLINCH_X);
        s->vy = FLINCH_Y;
        s->ay = -FLINCH_RISE;
        SqueakSetState(s, BODY_FLINCH_RISE);
        return TRUE;
    case BODY_FLINCH_RISE:  // 0x020E33DC;
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FLINCH_FALL);
        break;
    case BODY_FLINCH_FALL:  // 0x020E32C8: landed, it starts over;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            Still(s);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    case BODY_KNOCK_ENTRY:  // 0x020E2FF0, 0x020E2F0C: its shake KatAM's hurt, then flung away;
        SqueakPlay(s, ANIM_HURT);
        Still(s);
        s->vx = Away(s, KNOCK_X);
        s->vy = KNOCK_Y;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_KNOCK_RISE);
        return TRUE;
    case BODY_KNOCK_RISE:   // 0x020E2D94;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (s->vy < 0) {
            SqueakPlay(s, ANIM_CURL);
            SqueakSetState(s, BODY_KNOCK_FALL);
        }
        break;
    case BODY_KNOCK_FALL:   // 0x020E2C04: landed, a bounce on, lying;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakPlay(s, ANIM_LYING);
            s->vx = Away(s, KNOCK_X);
            s->vy = KNOCK_BOUNCE_Y;
            s->ay = -GRAVITY;
            SqueakSetState(s, BODY_KNOCK_BOUNCE_RISE);
        }
        break;
    case BODY_KNOCK_BOUNCE_RISE:    // 0x020E2B40;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_KNOCK_BOUNCE_FALL);
        break;
    case BODY_KNOCK_BOUNCE_FALL:    // 0x020E29AC: landed, lying dizzy;
        CapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            Still(s);
            SqueakSpawnEffect(s, SQUEAK_FX_LAND_BIG, 0, 0, FALSE);
            SPOT_X(s) = SQ_X(&s->obj);
            SPOT_Y(s) = SQ_Y(&s->obj);
            COUNT(s) = 0;
            DIZZY(s) = TRUE;
            SqueakSetState(s, BODY_LYING);
        }
        break;
    case BODY_LYING:        // 0x020E270C: 180 frames (shivering from 120), then half the time up, else flexing 180 frames;
        x = SPOT_X(s);
        if (COUNT(s) >= SHIVER_TIME && TREAT_TIMER(s) == 0) x += COUNT(s) % 8 < 2 ? 0x2000 : COUNT(s) % 8 < 6 ? -0x2000 : 0;
        SetPos(s, x, SPOT_Y(s));
        if (COUNT(s)++ >= DIZZY_TIME) {
            SetPos(s, SPOT_X(s), SPOT_Y(s));
            DIZZY(s) = FALSE;
            if (Random() < 50) {
                SqueakPlay(s, ANIM_GET_UP);
                SqueakSetState(s, BODY_GET_UP);
            } else {
                SqueakPlay(s, ANIM_FLEX);
                COUNT(s) = FLEX_TIME;
                SqueakSetState(s, BODY_FLEX);
            }
        }
        break;
    case BODY_GET_UP:       // 0x020E2628;
        Gravity(s);
        SqueakMove(s);
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* StoroMain(); Its body; its pace; Kirby's inhale reaching it, it curls up (its hit reaction to kinds 3-5, not while knocked or
 * dizzy); a chest touched, it takes it; */
static void StoroMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Chest *chest;

    if (TREAT_TIMER(s) > 0) TREAT_TIMER(s)--;
    if (TURN_TIMER(s) > 0) TURN_TIMER(s)--;
    Held(s);
    Pace(s);
    if (s->state < BODY_FLINCH_ENTRY && NidlInhaler(&obj->base)) {
        INHALED(s) = TRUE;
        if (s->state != BODY_BRACE) {
            SqueakPlay(s, ANIM_CURL);
            Still(s);
            COUNT(s) = 0;
            SPOT_Y(s) = SQ_Y(obj);
            SqueakSetState(s, BODY_BRACE);
        }
    }
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    // (not while its mallet's out: 0x020E3BA8 takes no chest with its +0x23C 7);
    if (!Held(s) && s->state < BODY_FLINCH_ENTRY && s->state != BODY_TAKE && s->state != BODY_MALLET_WAIT && s->state != BODY_MALLET
        && (chest = SqueakTouchedChest(s)) != NULL) {
        if (REGRAB(s) > 0) {
            REGRAB(s)--;
        } else {
            SqueakTakeChest(&CHEST(s), SWAY(s), chest);
            Play(s, ANIM_TAKE);
            Still(s);
            COUNT(s) = TAKE_TIME;
            NEXT(s) = BODY_START;
            SqueakSetState(s, BODY_TAKE);
        }
    }
    SqueakSetBoxes(s, &sStoroBoxData, HARM(s), &BOX(s));
    // (knocked out, till it lies dizzy: untouchable, as Squeak Squad's +0x1C0 2 has its hit handler take nothing);
    if (s->state >= BODY_KNOCK_ENTRY && !DIZZY(s)) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
    LAST_FLAGS(s) = obj->base.flags;
    LAST_UNKC(s) = obj->base.unkC;
    LAST_HP(s) = obj->unk80;
}

/* StoroWatch(); Its pre-callback: hit while dizzy: no damage, a treat roll (KatAM's hurt undone); KatAM's hit put it out of its own
 * state: it drops its chest; a hit's first frame: a treat roll, its HP down to 20% (30%) the first time a knockout once KatAM's hurt
 * is over, else its flinch; KatAM's hurt in the place of its 16 frames' shake: held where it was hit; */
static void StoroWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (DIZZY(s) && obj->unk78 == sub_0809B6A8 && !(FLAGS(s) & FLAG_HURT)) {
        Treat(s);
        obj->unk80 = LAST_HP(s);
        obj->base.flags = LAST_FLAGS(s);
        obj->base.unkC = LAST_UNKC(s);
        obj->unk98 = obj->unk9A = 0;
        obj->base.xspeed = obj->base.yspeed = 0;
        obj->base.objBase55 = 0;
        obj->unk78 = StoroMain;
        return;
    }
    if (obj->unk78 != StoroMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000)) {
        if (Held(s)) {
            SqueakDropChest(s, &CHEST(s));
            REGRAB(s) = 10;
        }
        if (!(FLAGS(s) & FLAG_HURT) && !(obj->base.flags & 0x1000) && obj->unk80 > 0) {
            Treat(s);
            SPOT_X(s) = SQ_X(obj);
            SPOT_Y(s) = SQ_Y(obj);
            AWAY(s) = obj->kirby3->base.x > obj->base.x;
            if (PHASE(s) == 0 && obj->unk80 * 100 <= gSqueakEnemyHp[s->enemy] * (s->variant == 1 ? 30 : KNOCK_PERCENT)) {
                PHASE(s) = 1;
                FLAGS(s) |= FLAG_KNOCK;
            }
        }
        FLAGS(s) |= FLAG_HURT;
        if (obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
            obj->base.xspeed = obj->base.yspeed = 0;
            obj->unk98 = obj->unk9A = 0;
            SetPos(s, SPOT_X(s), SPOT_Y(s));
        }
    } else {
        FLAGS(s) &= ~FLAG_HURT;
    }
}

// StoroGone(); Its end (squeak.c's SqueakDestroy): it drops its chest;
void StoroGone(struct SqueakObject *s) {
    SqueakDropChest(s, &CHEST(s));
}

void StoroStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, StoroMain);
    obj->unk7C = StoroWatch;
    if (!s->started) {
        HOME_X(s) = SQ_X(obj);
        HOME_Y(s) = SQ_Y(obj);
        if (s->variant == 2) {
            SqueakPlay(s, ANIM_FLEX);
            COUNT(s) = FLEX_START;
            SqueakSetState(s, BODY_FLEX);
            return;
        }
        SqueakSetState(s, BODY_START);
        return;
    }
    // (KatAM's hurt over: its own flinch, or its knockout; away from Kirby: the hit's side);
    if (FLAGS(s) & FLAG_KNOCK) {
        FLAGS(s) &= ~FLAG_KNOCK;
        SqueakSetState(s, BODY_KNOCK_ENTRY);
        return;
    }
    SqueakSetState(s, BODY_FLINCH_ENTRY);
}

/* StoroInit(); Its box (stron.rca's, set each frame), facing left (0x020E43C0's flag 4); KatAM's inhale can't take it (unk5C 0x20),
 * nor Kirby's body hurt it (0x80: on the DS, Kirby held inside it 120 frames, its HP stayed), as Squeak Squad's; */
void StoroInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -23, -44, 29, 1);
    ObjectSetBounds(&s->obj.base, -14, -36, 14, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.unk5C |= 0x20 | 0x80;
}
