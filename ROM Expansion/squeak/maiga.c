/* Maiga; ported from Kirby: Squeak Squad (USA), enemy 0x0F (its body 0x020A1CB0 on, vtable 0x021746A8; brain 0x020A0B58), as
 * SQUEAK_MAIGA. Values from param/prmNE16Ml0.kgb (movement, the bite; MlG the same), prmNE16Pl0.kgb (brain) and prmNE16Sl0.kgb
 * (HP 10); speeds 20.12 a frame, Y up. A little slime that hops about; Kirby's inhale can't swallow it: pulled in, it bites his
 * mouth and holds on till he shakes it off, then flings him away.
 * • Brain (0x020A0A8C): turns to face Kirby (past 1 pixel), then each time its body stands (0x020A0930, the body's mode 1, on the
 *   ground) picks from Squeak Squad's random 0-99 and prmNE16Pl0's weights (25 each; the 4th's sum reaches 100, so the last two,
 *   10 each, never come): hop in place, hop forward, turn round and hop forward, or A (a big hop) in place. A stays pressed until it
 *   picks a hop forward and is in the air (0x020A08BC), so the big hops go on till then. Forward, a wall ahead turns it round
 *   (0x020A08F0, 0x020A0848).
 * • Body: stands (anim 1) 10 frames, crouches (anim 8) and hops (anim 9; then 10, 11 falling, 12 landing): 1.0 up (0x219A, 2.1, with
 *   A), falling 0xCD a frame, no faster than 2.4; its pad moves it at 0.8 in the air (0x020A1D6C: Ml0's 0x800, then 0xCCD over it).
 * • Inhaled (its hit reaction 0x020A1E6C, attack kinds 3-5; here KatAM's inhale reach, NidlInhaler()): it leans in (anim 2) and goes
 *   at the Kirby at 2.0 a frame across and up or down, each frame he goes on inhaling it; he stops, it starts over.
 * • Biting (its touch reaction 0x020A1F90, kind 6: the inhale's mouth; on the DS it caught 8 pixels from him, where it holds him):
 *   it chomps (anim 4) on his face, holding him 8 pixels ahead of it, bobbing 2 pixels up and down (0x020A0D84: up when its count
 *   is 0 in 8, down when 2); the count goes up 1 a frame, 4 more each frame his pad changes (stick across or up and down, A, B); at
 *   120 (2 seconds left alone) it lets go: he's flung back from 3 pixels up at 2.6 (0x299A) and hurt when he lands (DS: 6 of 36 HP),
 *   KatAM's throw that hurts on landing (Kirby_110 flag 2, sub_0804F32C: 1 HP); it starts over. Kirby is held as KatAM's grabbers
 *   hold him (kirby->unk110, see minibosses/bugzzy.c), facing it.
 * Not ported: its sounds (0x132 the hops, 0x133 the bite), kind 5's own inhale (an attacker whose +0x18 is 3), its other mode (the
 * brain's +8 not 3: A or B held in a fall or landing hops big), the bite let go by kind 0x15 (frozen), and a hit while it bites: on
 * the DS it lets go of Kirby and takes no damage (its 0x218 still set: no more hits nor bites after); here it lets go and is hurt
 * as KatAM's enemies are. KatAM's inhale doesn't take it (unk5C 0x20: see MaigaInit()). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

void sub_0804E09C(struct Kirby *);

// Animations (maiga.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_PULLED, ANIM_3, ANIM_BITE, ANIM_5, ANIM_HURT, ANIM_HURT_2, ANIM_CROUCH, ANIM_RISE, ANIM_PEAK,
       ANIM_FALL, ANIM_LAND };

// prmNE16Ml0.kgb;
#define GRAVITY 0xCD
#define HOP 0x1000
#define BIG_HOP 0x219A
#define AIR_SPEED 0xCCD
#define MAX_FALL -0x2667
#define LAND_FALL -0x2666
#define PULL_SPEED 0x2000
#define PULL_ACCEL 0x2000
#define HOLD_X 0x8000
#define ESCAPE 120
// prmNE16Pl0.kgb;
#define STAND_CHANCE 25
#define FORWARD_CHANCE 25
#define BACK_CHANCE 25
// The code's;
#define STAND_WAIT 10
#define PICK_WAIT 6
#define FACE_DISTANCE 0x1000
#define MASH 4
#define BOB 2
#define FLING_UP 3
#define FLING_GUARD 30                  // The flung Kirby's 0xEC: 30 frames he and it don't touch;
#define KIRBY_FEET 7                    // KatAM's Kirby's spot is 7 pixels over his feet (his box's bottom), Squeak Squad's at them;

enum { MODE_STAND = 1, MODE_HOP, MODE_FALL, MODE_PULLED, MODE_BITE };
enum {
    BODY_START, BODY_STAND, BODY_CROUCH_ENTRY, BODY_CROUCH, BODY_RISE, BODY_PEAK, BODY_FALL_ENTRY, BODY_FALL, BODY_LAND,
    BODY_BIG_CROUCH_ENTRY, BODY_BIG_CROUCH, BODY_BIG_RISE, BODY_BIG_PEAK, BODY_PULLED_ENTRY, BODY_PULLED, BODY_BITE,
};
enum { BRAIN_FACE, BRAIN_PICK, BRAIN_WAIT, BRAIN_WAITED, BRAIN_AGAIN, BRAIN_FORWARD, BRAIN_AIR };

#define MODE(s) ((s)->vars[0])          // 0x220: what its body is doing (the brain picks while it stands);
#define WAIT(s) ((s)->vars[1])          // 0x208;
#define PULLING(s) ((s)->vars[2])       // 0x214: inhaled this frame;
#define COUNT(s) ((s)->vars[3])         // 0x21C: the bite's count;
#define KEYS(s) ((s)->vars[4])          // 0x1F8-0x204: the bitten Kirby's pad last frame;
#define BRAIN_WAIT(s) ((s)->vars[5])    // The brain's 0x98;
#define INHALER(s) (*(struct Kirby **)&(s)->vars[6])    // 0x20C, 0x210: whose inhale pulls it;
#define BITTEN(s) (*(struct Kirby **)&(s)->vars[7])     // 0x164;
#define GUARD(s) ((s)->vars[8])         // Frames left of FLING_GUARD;

/* Kirby held (kirby->unk110): 8 pixels before it (his facing's x: he faces it), his feet at its own, bobbing (up, level, down); his
 * pose 0x57 (KatAM's grabbers': Mr. Frosty's, Box Boxer's), behind it; each followed by an end the hold never reaches; */
static const struct Kirby_110 sBite[][2] = {
    { { -8, -KIRBY_FEET - BOB, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    { { -8, -KIRBY_FEET, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    { { -8, -KIRBY_FEET + BOB, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
};
/* Flung back at 2.6 (-0x299A: -42 x 0x10 here, 2.625), hurt when he lands (flag 2: sub_0804F32C), or let go as he is (flag 1); with
 * 0x400 too, or KatAM's hold lets him go its own way first (sub_0804E60C: on an entry without it, a holder whose unk83 is past its type's
 * hurt frame, any for this type, drops him on its spot, hopping); */
static const struct Kirby_110 sFling[] = { { -42, 0, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void MaigaMain(struct Object *obj);

/* KirbyKeys(); A Kirby's pad (A, B, the D-pad: Squeak Squad's 0xE8 fields 0x14-0x20 it watches): the player's keys, alone; else
 * what KatAM takes into movementState (as meta_knight.c's); */
static u16 KirbyKeys(struct Kirby *k) {
    if (!(gUnk_0203AD10 & 0x12) && k->base.unk56 == 0) return gHeldKeys & 0xF3;
    return k->movementState & 0xF3;
}

// Gravity(); 0x020A0D20: on the ground (its distance to the floor, from its last move, under 1), still; else falling 0xCD a frame, no
// faster than 2.4;
static void Gravity(struct SqueakObject *s) {
    if (s->terrain & 4) {
        s->vy = s->ay = 0;
    } else if (s->vy <= MAX_FALL) {
        s->ay = 0;
        s->vy = MAX_FALL;
    } else {
        s->ay = -GRAVITY;
    }
}

// Hop(); 0x020A0C1C: 1 a hop, 2 a big hop (from still), 3 the fall (no caps, accelerations nor friction; its speeds stay);
static void Hop(struct SqueakObject *s, u8 how) {
    if (how == 3) {
        s->maxVx = s->maxVy = s->ax = s->fx = s->fy = 0;
    } else {
        SqueakStop(s);
        s->vy = how == 1 ? HOP : BIG_HOP;
    }
    s->ay = -GRAVITY;
}

// AirControl(); 0x020A1D6C: its pad turns it round; any direction moves it on at 0.8;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir != 0) s->vx = AIR_SPEED;
}

static bool32 KirbyFree(struct Kirby *kirby) {
    return !kirby->base.header.kind && kirby->hp > 0 && kirby->animationIndex != 0x27 && kirby->animationIndex <= 0x7A
           && !kirby->unk110 && !(kirby->base.flags & 0x3800B00);
}

// StillBitten(); The Kirby it bites is still held by it (KatAM's hold lets him go on its own when it's beaten);
static bool32 StillBitten(struct SqueakObject *s) {
    struct Kirby *kirby = BITTEN(s);

    return kirby && kirby->base.unk6C == &s->obj.base && kirby->unk110 >= sBite[0] && kirby->unk110 < sBite[3];
}

// LetGo(); Kirby let go: flung (`how` sFling, from 8 pixels ahead of it, 3 up) or as he is (sLetGo), the table started over;
static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = BITTEN(s);

    if (StillBitten(s)) {
        if (how == sFling) {
            kirby->base.x = s->obj.base.x + ((s->obj.base.flags & 1) ? -SQUEAK_SPEED(HOLD_X) : SQUEAK_SPEED(HOLD_X));
            kirby->base.y = s->obj.base.y - (FLING_UP + KIRBY_FEET) * 0x100;
        }
        kirby->unk110 = how;
        kirby->base.counter = -1;
        if (how == sFling) GUARD(s) = FLING_GUARD;
    }
    BITTEN(s) = NULL;
    s->obj.base.unk6C = NULL;
}

/* Bite(); 0x020A1F90 and 0x020A1054: Kirby's inhale took it to his mouth: it holds him (KatAM's hold, sub_0804E09C, which faces him
 * as it: then turned to face it); */
static void Bite(struct SqueakObject *s, struct Kirby *kirby) {
    BITTEN(s) = kirby;
    PULLING(s) = 0;
    COUNT(s) = 0;
    KEYS(s) = KirbyKeys(kirby);
    kirby->base.unk6C = &s->obj.base;
    kirby->unk110 = sBite[0];
    s->obj.base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    kirby->base.flags ^= 1;
    MODE(s) = MODE_BITE;
    SqueakPlay(s, ANIM_BITE);
    SqueakStop(s);
    SqueakSetState(s, BODY_BITE);
}

static bool32 Body(struct SqueakObject *s) {
    struct Kirby *kirby;
    s32 x, y;

    switch (s->state) {
    case BODY_START:        // 0x020A1CB0 (vtable +0x3C);
        MODE(s) = MODE_STAND;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        WAIT(s) = STAND_WAIT;
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x020A1C18;
        if (WAIT(s) < 1) {
            WAIT(s) = 0;
            Hop(s, 1);
            SqueakSetState(s, BODY_CROUCH_ENTRY);
            return TRUE;
        }
        WAIT(s)--;
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CROUCH_ENTRY: // 0x020A1B6C;
        MODE(s) = MODE_HOP;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakSetState(s, BODY_CROUCH);
        return TRUE;
    case BODY_CROUCH:       // 0x020A1A44: A or B makes it a big hop;
        if (s->padA || s->padB) {
            SqueakSetState(s, BODY_BIG_CROUCH_ENTRY);
            return TRUE;
        }
        if (s->animEnded) {
            Hop(s, 1);
            SqueakPlay(s, ANIM_RISE);
            SqueakSetState(s, BODY_RISE);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_RISE:         // 0x020A1974;
        if (s->vy < 1) {
            SqueakPlay(s, ANIM_PEAK);
            SqueakSetState(s, BODY_PEAK);
            return TRUE;
        }
        AirControl(s);
        SqueakMove(s);
        break;
    case BODY_PEAK:         // 0x020A1900;
    case BODY_BIG_PEAK:     // 0x020A1618;
        if (s->animEnded) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            return TRUE;
        }
        AirControl(s);
        SqueakMove(s);
        break;
    case BODY_FALL_ENTRY:   // 0x020A1564;
        MODE(s) = MODE_FALL;
        SqueakPlay(s, ANIM_FALL);
        Hop(s, 3);
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020A1444;
        AirControl(s);
        SqueakCapFall(s, LAND_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_LAND);
            SqueakSetState(s, BODY_LAND);
        }
        break;
    case BODY_LAND:         // 0x020A1394;
        SqueakCapFall(s, LAND_FALL);
        SqueakMove(s);
        if (s->animEnded) SqueakSetState(s, BODY_START);
        break;
    case BODY_BIG_CROUCH_ENTRY: // 0x020A1854;
        MODE(s) = MODE_HOP;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakSetState(s, BODY_BIG_CROUCH);
        return TRUE;
    case BODY_BIG_CROUCH:   // 0x020A175C;
        if (s->animEnded) {
            Hop(s, 2);
            SqueakPlay(s, ANIM_RISE);
            SqueakSetState(s, BODY_BIG_RISE);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_BIG_RISE:     // 0x020A168C;
        AirControl(s);
        if (s->vy < 1) {
            SqueakPlay(s, ANIM_PEAK);
            SqueakSetState(s, BODY_BIG_PEAK);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_PULLED_ENTRY: // 0x020A12A0 (each frame it's inhaled): it faces the Kirby;
        MODE(s) = MODE_PULLED;
        SqueakPlay(s, ANIM_PULLED);
        SqueakStop(s);
        x = INHALER(s)->base.x;
        if (x > s->obj.base.x) {
            if (s->obj.base.flags & 1) SqueakTurnAround(s);
        } else if (x < s->obj.base.x && !(s->obj.base.flags & 1)) {
            SqueakTurnAround(s);
        }
        SqueakSetState(s, BODY_PULLED);
        return TRUE;
    case BODY_PULLED:       // 0x020A116C: at him, 2.0 across and up or down; no more inhaled, it starts over;
        kirby = INHALER(s);
        y = kirby->base.y + KIRBY_FEET * 0x100;
        if (kirby->base.x != s->obj.base.x) s->ax = PULL_ACCEL;
        if (y < s->obj.base.y) s->ay = PULL_ACCEL;
        else if (y > s->obj.base.y) s->ay = -PULL_ACCEL;
        else s->ay = 0;
        if (s->vx > PULL_SPEED) s->vx = PULL_SPEED;
        if (s->ay == PULL_ACCEL) {
            if (s->vy > PULL_SPEED) s->vy = PULL_SPEED;
        } else if (s->ay == -PULL_ACCEL) {
            if (s->vy < -PULL_SPEED) s->vy = -PULL_SPEED;
        } else {
            s->vy = s->ay = 0;
        }
        SqueakMove(s);
        if (!PULLING(s)) {
            SqueakStop(s);
            SqueakSetState(s, BODY_START);
        } else if (abs(kirby->base.x - s->obj.base.x) <= SQUEAK_SPEED(HOLD_X) && KirbyFree(kirby)) {
            Bite(s, kirby);
        }
        PULLING(s) = 0;
        break;
    case BODY_BITE:         // 0x020A0D84: bobbing him; at the count's end it flings him and starts over;
        if (!StillBitten(s)) {
            BITTEN(s) = NULL;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        kirby = BITTEN(s);
        kirby->unk110 = sBite[COUNT(s) % 8 == 0 ? 0 : COUNT(s) % 8 == 2 ? 2 : 1];
        Gravity(s);
        if (COUNT(s) >= ESCAPE) {
            LetGo(s, sFling);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)++;
        if (KirbyKeys(kirby) != KEYS(s)) {
            KEYS(s) = KirbyKeys(kirby);
            COUNT(s) += MASH;
        }
        break;
    }
    return FALSE;
}

static void Brain(struct SqueakObject *s) {
    u32 pick;

    switch (s->brain) {
    case BRAIN_FACE:        // 0x020A0A8C;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_PICK:        // 0x020A0930: standing, on the ground;
        if (MODE(s) != MODE_STAND || !SqueakOnGround(s)) break;
        pick = Rand16() * 100 >> 16;
        BRAIN_WAIT(s) = PICK_WAIT;
        if (pick < STAND_CHANCE) {
            s->padDir = 0;
            s->brain = BRAIN_WAIT;
        } else if (pick < STAND_CHANCE + FORWARD_CHANCE) {
            s->padDir = 1;
            s->brain = BRAIN_FORWARD;
        } else if (pick < STAND_CHANCE + FORWARD_CHANCE + BACK_CHANCE) {
            s->padDir = -1;
            s->brain = BRAIN_FORWARD;
        } else {
            s->padA = 1;
            s->padDir = 0;
            s->brain = BRAIN_WAIT;
        }
        break;
    case BRAIN_WAIT:        // 0x020A0880;
        if (BRAIN_WAIT(s) < 1) s->brain = BRAIN_WAITED;
        else BRAIN_WAIT(s)--;
        break;
    case BRAIN_WAITED:      // 0x020A0848;
        if (SqueakWallAhead(s)) s->padDir = -1;
        if (SqueakOnGround(s)) {
            BRAIN_WAIT(s) = 12;
            s->brain = BRAIN_AGAIN;
        }
        break;
    case BRAIN_AGAIN:       // 0x020A08A8;
        s->padDir = 0;
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_FORWARD:     // 0x020A08BC: forward; once in the air, A let go;
        s->padDir = 1;
        if (!SqueakOnGround(s)) {
            s->padA = s->padB = 0;
            s->brain = BRAIN_AIR;
        }
        break;
    case BRAIN_AIR:         // 0x020A08F0: a wall ahead turns it round; landed, it picks again;
        s->padDir = 1;
        if (SqueakWallAhead(s)) s->padDir = -1;
        if (SqueakOnGround(s)) {
            BRAIN_WAIT(s) = 10;
            s->padDir = 0;
            s->brain = BRAIN_PICK;
        }
        break;
    }
}

/* Inhaled(); Its hit reaction to an inhale (0x020A1E6C, kinds 3-5; not while it bites, 0x218): KatAM's inhale reach around it
 * (NidlInhaler()), each frame it's in it its pull starts over (0x020A12A0); */
static void Inhaled(struct SqueakObject *s) {
    struct Kirby *kirby;

    if (BITTEN(s) || s->obj.unk80 <= 0) return;
    kirby = NidlInhaler(&s->obj.base);
    if (kirby == NULL) return;
    INHALER(s) = kirby;
    PULLING(s) = 1;
    SqueakSetState(s, BODY_PULLED_ENTRY);
}

static void MaigaMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    Inhaled(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    // Untouchable (KatAM's 0x200, as its inhaled ones') while pulled in, and while the Kirby it flung is guarded;
    if (GUARD(s) > 0) GUARD(s)--;
    if (GUARD(s) > 0 || MODE(s) == MODE_PULLED) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* MaigaWatch(); Its pre-callback (obj->unk7C: every frame, hit or beaten too): KatAM's hit put it out of its own state (unk78 not
 * MaigaMain), or it's beaten: it lets Kirby go as he is; */
static void MaigaWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (BITTEN(s) && (obj->unk78 != MaigaMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

void MaigaStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, MaigaMain);
    obj->unk7C = MaigaWatch;
    if (!s->started) s->brain = BRAIN_FACE;
    if (BITTEN(s)) LetGo(s, sLetGo);
    PULLING(s) = 0;
    SqueakSetState(s, BODY_START);
}

/* MaigaInit(); Its box (maiga.rca's first: 4 pixels behind to 6 ahead, 1 to 10 up; as the others', from its feet), its feet on the
 * ground; KatAM's inhale can't take it: unk5C's attack kinds are those that don't take it (the hit check at 0x08036646: the attacker's
 * unk68 kinds, 0x3FFFF8, none of them in unk5C, and its strength, the low bits, at least unk5C's), and the inhale's box is kind 0x20
 * (sub_08071FC0: unk68 0x20000021, strength 3 after 48 frames; the mini-bosses' unk5C has it too); */
void MaigaInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -4, -9, 6, 0);
    ObjectSetBounds(&s->obj.base, -4, -9, 6, 0);
    s->obj.base.unk5C |= 0x20;
    SqueakStandOnFloor(&s->obj);
}
