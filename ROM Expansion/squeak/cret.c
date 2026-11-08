/* Cret; ported from Kirby: Squeak Squad (USA), enemy 0x10 (its body 0x02073044, vtable 0x0216D548; brain 0x020705AC; its drop
 * 0x02073764), as SQUEAK_CRET and SQUEAK_CRET_BALL. Values from param/prmNE17Ml0.kgb (movement; MlG the same; its drop's too),
 * prmNE17Pl0.kgb (brain) and prmNE17Sl0.kgb (HP 10); speeds 20.12 a frame, Y up. A little creature riding a cloud: it drifts and
 * bobs, flies loops, drops spiky balls on Kirby below, blows back at his inhale and, inhaled long enough, loses its cloud and pops
 * into a Gordo or a treat.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12; each first turns to face Kirby, past 1 pixel):
 *   0 (and any other; 0x020703BC): drifts forward; every 90 frames (30%) or 150, if Kirby is then within 16 pixels across and below
 *     it (or less than 160 pixels above), it stops and drops a ball (B);
 *   1, 3 (0x020702F0, 0x020704CC): flies loop C, or loop A (pad up/down 2, 4); the same drop, turning to face Kirby first;
 *   2 (0x020701D8): hovers; once Kirby is within 32 pixels across and 160 up or down, it watches him (facing him every 30 frames)
 *     for 180 frames, turns round, and 10 frames later flies away (pad up/down 5).
 *   (Pad up/down 3, loop B, no brain of Cret's asks for: ported all the same.)
 * • Body (riding its cloud; every state sets its flag 0x200 through vtable +0x40, 0x020709F4: it passes through the terrain, the
 *   stage's edges holding it; SqueakThroughTerrain()):
 *   hovering (0x02072BEC): anim 1, bobbing (0x020707E8: 0.1 up for 20 frames, 0.1 down for 20, then a frame still; 0x19A + 0x67
 *     a frame), its speeds slowing by 0xA4 a frame (0x02070890);
 *   drifting (0x02072A48, any pad direction): forward speeds up by 0xA4 a frame to 0.8 (0x02070958), up/down too; turning reverses
 *     it at once; A backs it up;
 *   flying away (0x02071164, pad up/down 5): 1.2 forward, rising faster and faster (0x334 a frame) to 2.0;
 *   loops (0x020716AC, 0x02071FE8, 0x02072954): quarters of a circle (0x020712A8...): one speed eases to 0 while the other grows to
 *     the most (0x1000 across and up/down, by 0x67 a frame for A, 0xCD for B and C), held 60 frames at some (Ml0[15]); A is an oval,
 *     B a figure eight, C zigzags; turning round mirrors it; in loops B and C one turn's speed across jumps to the most at once
 *     facing the other way (the DS's own comparison: ported as it is);
 *   dropping (B, 0x02070B98): it stops where it is and plays its composite animation (cret.mca's: the ball slides out under the
 *     cloud, 20 frames), then a ball falls from 18 pixels below it (0x02073764: 0x19A a frame, no faster than 3.0; spinning, anim 9)
 *     and bursts on whatever it hits (efc_cmn1's animation 1), gone 8 frames later; it goes on with what it did;
 *   inhaled (its hit reaction 0x02072E3C, kinds 3-5; here KatAM's inhale reach, NidlInhaler()): it faces the inhaler and blows back
 *     (anim 3); still inhaled when that's played once, its cloud is pulled off (anim 4) and it's left alone (anim 5) 20 frames,
 *     then it pops (0x02070C54): 30 in 100 a Gordo (Squeak Squad's enemy 5: KatAM's own here), else a treat: 15 a Maxim Tomato
 *     (heals all), 20 meat (Squeak Squad's heals 24 of 36: KatAM's meat 3 of 6), 20 a drink (12 of 36: KatAM's pop soda, 2), 10
 *     food (6 of 36: KatAM's small food, 1), 5 a 1-UP. The inhale stopping before it's alone, it starts over.
 * Not ported: its sounds (0x134, 0x135 the ball), the ring of cloud puffs it pops in (0x020219C4) and Squeak Squad's appearing
 * effects of what it leaves (KatAM's own puff instead), its other mode (the brain's +8 not 3: the loops start over). KatAM's inhale
 * doesn't take it (unk5C 0x20, as Maiga). */
#include "../squeak.h"
#include "random.h"
#include "code_0806F780.h"

// Animations (cret.cea);
enum { ANIM_STILL, ANIM_HOVER, ANIM_HOVER_FAST, ANIM_BLOW, ANIM_CLOUD_OFF, ANIM_ALONE, ANIM_HURT, ANIM_HURT_2, ANIM_8, ANIM_BALL,
       ANIM_BALL_OUT, ANIM_BALL_STILL };
#define COMPOSITE_DROP 0

// prmNE17Ml0.kgb;
#define DRIFT_SPEED 0xCCD
#define DRIFT_SLOW 0xA4
#define DRIFT_ACCEL 0xA4
#define BOB_SPEED 0x19A
#define BOB_ACCEL 0x67
#define DROP_X 0
#define DROP_Y -0x12000
#define FLY_SPEED 0x1334
#define FLY_RISE 0x2000
#define FLY_ACCEL 0x334
#define LOOP_A 0x1000
#define LOOP_A_STEP 0x67
#define LOOP_B 0x1000
#define LOOP_B_STEP 0xCD
#define LOOP_HOLD 60
#define BALL_GRAVITY 0x19A
#define BALL_MAX_FALL -0x3000
// prmNE17Pl0.kgb;
#define DROP_RANGE_X 0x10000
#define DROP_RANGE_Y 0xA0000
#define WATCH_RANGE_X 0x20000
#define WATCH_RANGE_Y 0xA0000
#define WAIT_SHORT_CHANCE 30
#define WAIT_SHORT 90
#define WAIT_LONG 150
#define WATCH_TIME 180
#define TURN_WAIT 10
#define WATCH_FACE 30
// The code's;
#define BOB_TIME 40
#define BOB_HALF 20
#define ALONE_TIME 20
#define FACE_DISTANCE 0x1000
#define BALL_GONE 8

enum { PAD_LOOP_C = 2, PAD_LOOP_B = 3, PAD_LOOP_A = 4, PAD_FLY_AWAY = 5 };
enum {
    BODY_START, BODY_HOVER, BODY_DRIFT_ENTRY, BODY_DRIFT, BODY_FLY_ENTRY, BODY_FLY, BODY_DROP_ENTRY, BODY_DROP, BODY_BLOW_ENTRY,
    BODY_BLOW, BODY_CLOUD_OFF, BODY_ALONE, BODY_LOOP_A, BODY_LOOP_B, BODY_LOOP_C, BODY_QUARTER,
};
enum {
    BRAIN_DRIFT_FACE, BRAIN_DRIFT, BRAIN_LOOP_FACE, BRAIN_LOOP_WATCH, BRAIN_LOOP_DROP, BRAIN_LOOP_DROPPED, BRAIN_LOOP_AGAIN,
    BRAIN_WATCH_FACE, BRAIN_WATCH_WAIT, BRAIN_WATCH, BRAIN_TURN_AWAY,
};

#define BOB(s) ((s)->vars[0])           // 0x200: the bob's count, BOB_TIME down;
#define BASE_VY(s) ((s)->vars[1])       // 0x1FC: its speed up/down under the bob;
#define LOOP_X(s) ((s)->vars[2])        // 0x210, 0x214: a loop's speeds across and up/down;
#define LOOP_Y(s) ((s)->vars[3])
#define LOOP_MAX(s) ((s)->vars[4])      // 0x218 (0x21C the same), 0x220 (0x224 the same): its most and step;
#define LOOP_STEP(s) ((s)->vars[5])
#define LOOP_TURNED(s) ((s)->vars[6])   // 0x20C: turned round (its quarters' sides swap);
#define QUARTER(s) ((s)->vars[7])       // The quarter flown (its state);
#define HOLD(s) ((s)->vars[8])          // 0x208: frames at a quarter's end;
#define RESUME(s) ((s)->vars[9])        // 0x234: the state a drop goes back to (and its quarter);
#define RESUME_QUARTER(s) ((s)->vars[10])
#define INHALED(s) ((s)->vars[11])      // 0x22C: inhaled this frame; 0x228: blowing;
#define BLOWING(s) ((s)->vars[12])
#define ALONE(s) ((s)->vars[13])        // 0x230;
#define BRAIN_WAIT(s) ((s)->vars[14])   // The brain's 0xA0/0xA4;
#define BRAIN_COUNT(s) ((s)->vars[15])  // The brain's 0xA8;

/* A quarter of a loop: a decay (0x020713C0...) eases the speed across to 0 (0x020711F4) while the speed up/down grows to `y` times the
 * most; a turn (0x020712A8...) grows the speed across to `x` times the most (x as it's not turned round: the other way when it is)
 * while the speed up/down eases back to 0 (`y`: from below, 1, or above, -1); then `next`, after LOOP_HOLD frames there if `hold`;
 * `jump`: turned round, the speed across jumps to the most at once; */
struct Quarter {
    bool8 turn;
    s8 x;
    s8 y;
    u8 next;
    bool8 hold;
    bool8 jump;
};
enum { A0, A1, A2, A3, B0, B1, B2, B3, B4, B5, B6, B7, C0, C1, C2, C3, C4, C5, C6, C7 };
static const struct Quarter sQuarters[] = {
    [A0] = { FALSE, 0, -1, A1, FALSE, FALSE },    // 0x020715C0;
    [A1] = { TRUE, -1, 1, A2, TRUE, FALSE },      // 0x020714A8;
    [A2] = { FALSE, 0, 1, A3, FALSE, FALSE },     // 0x020713C0;
    [A3] = { TRUE, 1, -1, A0, TRUE, FALSE },      // 0x020712A8;
    [B0] = { FALSE, 0, -1, B1, TRUE, FALSE },     // 0x02071F28;
    [B1] = { TRUE, -1, 1, B2, FALSE, FALSE },     // 0x02071DF0;
    [B2] = { FALSE, 0, 1, B3, TRUE, FALSE },      // 0x02071D34;
    [B3] = { TRUE, -1, -1, B4, FALSE, FALSE },    // 0x02071BFC;
    [B4] = { FALSE, 0, -1, B5, TRUE, FALSE },     // 0x02071B3C;
    [B5] = { TRUE, 1, 1, B6, FALSE, FALSE },      // 0x02071A04;
    [B6] = { FALSE, 0, 1, B7, TRUE, FALSE },      // 0x02071948;
    [B7] = { TRUE, 1, -1, B0, FALSE, TRUE },      // 0x02071810;
    [C0] = { FALSE, 0, -1, C1, FALSE, FALSE },    // 0x02072868;
    [C1] = { TRUE, -1, 1, C2, TRUE, FALSE },      // 0x02072750;
    [C2] = { FALSE, 0, -1, C3, FALSE, FALSE },    // 0x02072664;
    [C3] = { TRUE, 1, 1, C4, TRUE, FALSE },       // 0x0207254C;
    [C4] = { FALSE, 0, 1, C5, FALSE, FALSE },     // 0x02072464;
    [C5] = { TRUE, -1, -1, C6, TRUE, FALSE },     // 0x0207234C;
    [C6] = { FALSE, 0, 1, C7, FALSE, FALSE },     // 0x02072264;
    [C7] = { TRUE, 1, -1, C0, TRUE, TRUE },       // 0x0207214C;
};

static s32 Clamp(s32 v, s32 lo, s32 hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

// Toward(); `v` a step closer to 0;
static s32 Toward0(s32 v, s32 step) {
    if (v > 0) return v - step < 0 ? 0 : v - step;
    if (v < 0) return v + step > 0 ? 0 : v + step;
    return 0;
}

/* Bob(); 0x020707E8: its speed up/down is BASE_VY with the bob (`amount` 1) or without (0): up BOB_SPEED (and BOB_ACCEL a frame) the
 * first half of its count, down the second, at the count's end still a frame; */
static void Bob(struct SqueakObject *s, bool8 amount) {
    s32 speed = amount ? BOB_SPEED : 0, accel = amount ? BOB_ACCEL : 0;

    if (BOB(s) <= BOB_HALF) {
        if (BOB(s) > 0) {
            s->vy = BASE_VY(s) - speed;
            s->ax = 0;
            s->ay = -accel;
        }
    } else {
        s->vy = BASE_VY(s) + speed;
        s->ax = 0;
        s->ay = accel;
    }
    if (BOB(s) < 1) {
        s->vy = BASE_VY(s);
        s->ax = s->ay = 0;
        BOB(s) = BOB_TIME;
    } else {
        BOB(s)--;
    }
}

// Slow(); 0x02070890: no pad across, its speed across slows by DRIFT_SLOW a frame; none up/down, BASE_VY too;
static void Slow(struct SqueakObject *s) {
    if (s->padA == 0 && s->padDir == 0 && s->vx != 0) s->vx = Toward0(s->vx, DRIFT_SLOW);
    if (s->padY == 0) BASE_VY(s) = Toward0(BASE_VY(s), DRIFT_SLOW);
}

// Steer(); 0x02070958: forward speeds it up to DRIFT_SPEED; up and down speed BASE_VY up to it;
static void Steer(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = s->vx < DRIFT_SPEED ? s->vx + DRIFT_ACCEL : DRIFT_SPEED;
    if (s->padY > 0) BASE_VY(s) = BASE_VY(s) < DRIFT_SPEED ? BASE_VY(s) + DRIFT_ACCEL : DRIFT_SPEED;
    if (s->padY < 0) BASE_VY(s) = BASE_VY(s) <= -DRIFT_SPEED ? -DRIFT_SPEED : BASE_VY(s) - DRIFT_ACCEL;
}

// Dropping(); 0x02070A1C: B drops a ball, then back to `state` (`quarter` for a loop's);
static bool32 Dropping(struct SqueakObject *s, u8 state, u8 quarter) {
    if (!s->padB) return FALSE;
    RESUME(s) = state;
    RESUME_QUARTER(s) = quarter;
    SqueakSetState(s, BODY_DROP_ENTRY);
    return TRUE;
}

// LoopStart(); 0x020716AC (A), 0x02071FE8 (B), 0x02072954 (C): from still, the speed across at the most;
static void LoopStart(struct SqueakObject *s, s32 most, s32 step, u8 first) {
    SqueakStop(s);
    LOOP_MAX(s) = LOOP_X(s) = most;
    LOOP_Y(s) = 0;
    LOOP_STEP(s) = step;
    LOOP_TURNED(s) = FALSE;
    HOLD(s) = 0;
    QUARTER(s) = first;
    SqueakSetState(s, BODY_QUARTER);
}

/* Quarter(); A quarter's frame: done (and held), the next; else the speeds' steps; then 0x020706DC: its pad turns it round (the sides
 * swap), it flies at the loop's speeds (no bob), and moves; */
static bool32 Quarter(struct SqueakObject *s) {
    const struct Quarter *q = &sQuarters[QUARTER(s)];
    s32 most = LOOP_MAX(s), step = LOOP_STEP(s), x = (LOOP_TURNED(s) ? -q->x : q->x) * most;
    bool32 done = q->turn ? LOOP_X(s) == x && LOOP_Y(s) == 0 : LOOP_X(s) == 0 && LOOP_Y(s) == q->y * most;

    if (Dropping(s, BODY_QUARTER, QUARTER(s))) return TRUE;
    if (done) {
        if (!q->hold || HOLD(s) >= LOOP_HOLD) {
            HOLD(s) = 0;
            QUARTER(s) = q->next;
            SqueakSetState(s, BODY_QUARTER);
            return TRUE;
        }
        HOLD(s)++;
    }
    if (q->turn) {
        if (x > 0) LOOP_X(s) = LOOP_X(s) + step > most ? most : LOOP_X(s) + step;
        else if (q->jump && LOOP_TURNED(s)) LOOP_X(s) = LOOP_X(s) - step >= most ? LOOP_X(s) - step : -most;
        else LOOP_X(s) = LOOP_X(s) - step < -most ? -most : LOOP_X(s) - step;
        LOOP_Y(s) = q->y > 0 ? (LOOP_Y(s) + step > 0 ? 0 : LOOP_Y(s) + step) : (LOOP_Y(s) - step < 0 ? 0 : LOOP_Y(s) - step);
    } else {
        LOOP_X(s) = Toward0(LOOP_X(s), step);
        LOOP_Y(s) = Clamp(LOOP_Y(s) + q->y * step, -most, most);
    }
    if (s->padDir < 0) {
        SqueakTurnAround(s);
        LOOP_TURNED(s) = !LOOP_TURNED(s);
        LOOP_X(s) = -LOOP_X(s);
    }
    BASE_VY(s) = LOOP_Y(s);
    Bob(s, FALSE);
    s->vx = LOOP_X(s);
    SqueakMove(s);
    return FALSE;
}

// FaceInhaler(); 0x02072E3C: it turns to face the Kirby inhaling it;
static void FaceInhaler(struct SqueakObject *s, struct Kirby *kirby) {
    if (kirby->base.x > s->obj.base.x) {
        if (s->obj.base.flags & 1) SqueakTurnAround(s);
    } else if (kirby->base.x < s->obj.base.x && !(s->obj.base.flags & 1)) {
        SqueakTurnAround(s);
    }
}

/* Pop(); 0x02070C54: its spot gets a Gordo (KatAM's own) or a treat (KatAM's nearest), and it's gone in a puff; */
static void Pop(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    u32 pick = Rand16() * 100 >> 16;
    u8 type;

    if (pick < 30) type = OBJ_GORDO;
    else if (pick < 45) type = OBJ_TOMATO;
    else if (pick < 65) type = OBJ_MEAT;
    else if (pick < 85) type = OBJ_POP_SODA;
    else if (pick < 95) type = OBJ_SMALL_FOOD;
    else type = OBJ_1UP;
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, type, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0);
    CreateEffectObject(&obj->base, 0, 0x292, 0);
    obj->base.flags |= 0x1000;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x02072D88 (vtable +0x3C);
        SqueakPlay(s, ANIM_HOVER);
        SqueakSetState(s, BODY_HOVER);
        return TRUE;
    case BODY_HOVER:        // 0x02072BEC;
        if (s->padY == PAD_FLY_AWAY || s->padY == PAD_LOOP_A || s->padY == PAD_LOOP_B || s->padY == PAD_LOOP_C) {
            if (s->padDir < 0) SqueakTurnAround(s);
            if (s->padY == PAD_FLY_AWAY) SqueakSetState(s, BODY_FLY_ENTRY);
            else SqueakSetState(s, s->padY == PAD_LOOP_A ? BODY_LOOP_A : s->padY == PAD_LOOP_B ? BODY_LOOP_B : BODY_LOOP_C);
            return TRUE;
        }
        if (Dropping(s, BODY_START, 0)) return TRUE;
        if (s->padDir == 0 && s->padY == 0 && s->padA == 0) {
            Slow(s);
            Bob(s, TRUE);
            SqueakMove(s);
            break;
        }
        SqueakSetState(s, BODY_DRIFT_ENTRY);
        return TRUE;
    case BODY_DRIFT_ENTRY:  // 0x02072B98;
        SqueakStop(s);
        SqueakSetState(s, BODY_DRIFT);
        return TRUE;
    case BODY_DRIFT:        // 0x02072A48;
        if (Dropping(s, BODY_START, 0)) return TRUE;
        if (s->padY == PAD_FLY_AWAY) {
            if (s->padDir < 0) SqueakTurnAround(s);
            SqueakSetState(s, BODY_FLY_ENTRY);
            return TRUE;
        }
        Steer(s);
        Slow(s);
        Bob(s, TRUE);
        if (s->padDir < 0) {
            s->vx = -s->vx;
            SqueakTurnAround(s);
        }
        if (s->padA) s->vx = s->vx <= -DRIFT_SPEED ? -DRIFT_SPEED : s->vx - DRIFT_ACCEL;
        SqueakMove(s);
        break;
    case BODY_FLY_ENTRY:    // 0x02071164;
        SqueakStop(s);
        s->vx = FLY_SPEED;
        s->vy = 0;
        SqueakSetState(s, BODY_FLY);
        return TRUE;
    case BODY_FLY:          // 0x020710AC;
        s->vy = s->vy < FLY_RISE ? s->vy + FLY_ACCEL : FLY_RISE;
        SqueakMove(s);
        break;
    case BODY_DROP_ENTRY:   // 0x02070B98: the ball slides out (cret.mca's composite), BASE_VY none;
        SqueakPlayComposite(s, COMPOSITE_DROP);
        BASE_VY(s) = 0;
        SqueakSetState(s, BODY_DROP);
        return TRUE;
    case BODY_DROP:         // 0x02070AC0: still; once played, the ball, and back to what it did;
        if (s->animEnded) {
            SqueakSpawn(s, SQUEAK_CRET_BALL, 0, DROP_X, DROP_Y);
            SqueakPlay(s, ANIM_HOVER);
            QUARTER(s) = RESUME_QUARTER(s);
            SqueakSetState(s, RESUME(s));
        }
        break;
    case BODY_BLOW_ENTRY:   // 0x02070FA4;
        BLOWING(s) = TRUE;
        SqueakStop(s);
        SqueakPlay(s, ANIM_BLOW);
        SqueakSetState(s, BODY_BLOW);
        return TRUE;
    case BODY_BLOW:         // 0x02070EB0;
    case BODY_CLOUD_OFF:    // 0x02070DB4;
        if (s->animEnded) {
            if (s->state == BODY_BLOW) {
                SqueakPlay(s, ANIM_CLOUD_OFF);
                SqueakSetState(s, BODY_CLOUD_OFF);
            } else {
                SqueakPlay(s, ANIM_ALONE);
                ALONE(s) = ALONE_TIME;
                SqueakSetState(s, BODY_ALONE);
            }
            return TRUE;
        }
        SqueakMove(s);
        if (!INHALED(s)) {
            BLOWING(s) = FALSE;
            SqueakSetState(s, BODY_START);
            break;
        }
        INHALED(s) = FALSE;
        break;
    case BODY_ALONE:        // 0x02070C54;
        if (s->animEnded) {
            if (ALONE(s) < 1) {
                Pop(s);
                break;
            }
            ALONE(s)--;
        }
        SqueakMove(s);
        break;
    case BODY_LOOP_A:
        LoopStart(s, LOOP_A, LOOP_A_STEP, A0);
        return TRUE;
    case BODY_LOOP_B:
        LoopStart(s, LOOP_B, LOOP_B_STEP, B0);
        return TRUE;
    case BODY_LOOP_C:
        LoopStart(s, LOOP_B, LOOP_B_STEP, C0);
        return TRUE;
    case BODY_QUARTER:
        return Quarter(s);
    }
    return FALSE;
}

/* RandomWait(); 0x02070088: Squeak Squad's random 0-99 under prmNE17Pl0's 30, 90 frames; else 150 (its 120 needs the same 30 again:
 * never); */
static s32 RandomWait(void) {
    return (Rand16() * 100 >> 16) < WAIT_SHORT_CHANCE ? WAIT_SHORT : WAIT_LONG;
}

// KirbyBelow(); 0x02070324's test: Kirby within `dx` across and under it (or less than `dy` above);
static bool32 KirbyBelow(struct SqueakObject *s, s32 dx, s32 dy) {
    s32 kx = SQ_X(s->obj.kirby3), ky = SQ_Y(s->obj.kirby3), x = SQ_X(&s->obj), y = SQ_Y(&s->obj);

    return ky < y + dy && kx > x - dx && kx < x + dx;
}

static void Brain(struct SqueakObject *s) {
    s32 kx, ky, x, y;

    switch (s->brain) {
    case BRAIN_DRIFT_FACE:  // 0x020703BC;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_WAIT(s) = RandomWait();
        s->brain = BRAIN_DRIFT;
        break;
    case BRAIN_DRIFT:       // 0x02070324;
        s->padDir = 1;
        if (BRAIN_WAIT(s) > 0) {
            BRAIN_WAIT(s)--;
            break;
        }
        if (BRAIN_WAIT(s) == 0 && KirbyBelow(s, DROP_RANGE_X, DROP_RANGE_Y)) {
            s->padDir = 0;
            s->padB = 1;
        }
        if (BRAIN_WAIT(s) == -1) {
            s->padB = 0;
            BRAIN_WAIT(s) = RandomWait();
        } else {
            BRAIN_WAIT(s)--;
        }
        break;
    case BRAIN_LOOP_FACE:   // 0x020702F0, 0x020704CC (the loop: its variable, pad up/down);
        s->padY = s->variant == 3 ? PAD_LOOP_A : PAD_LOOP_C;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_WAIT(s) = 0;
        s->brain = BRAIN_LOOP_AGAIN;
        break;
    case BRAIN_LOOP_WATCH:  // 0x0207024C, 0x02070428;
        s->padY = s->variant == 3 ? PAD_LOOP_A : PAD_LOOP_C;
        if (BRAIN_WAIT(s) >= 1) {
            BRAIN_WAIT(s)--;
            break;
        }
        if (KirbyBelow(s, DROP_RANGE_X, DROP_RANGE_Y)) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            if (s->padDir < 0) s->padA = 1;
            s->brain = BRAIN_LOOP_DROP;
            BRAIN_WAIT(s) = RandomWait();
        }
        break;
    case BRAIN_LOOP_DROP:   // 0x02070228, 0x02070404;
        s->padA = 0;
        s->padDir = 0;
        s->padB = 1;
        s->brain = BRAIN_LOOP_DROPPED;
        break;
    case BRAIN_LOOP_DROPPED:    // 0x02070208, 0x020703E4;
        s->padB = 0;
        s->brain = BRAIN_LOOP_AGAIN;
        break;
    case BRAIN_LOOP_AGAIN:  // 0x020702D8, 0x020704B4;
        s->padDir = 0;
        s->brain = BRAIN_LOOP_WATCH;
        break;
    case BRAIN_WATCH_FACE:  // 0x020701D8;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_WATCH_WAIT;
        BRAIN_WAIT(s) = BRAIN_COUNT(s) = 0;
        break;
    case BRAIN_WATCH_WAIT:  // 0x0207017C: Kirby near;
        kx = SQ_X(s->obj.kirby3);
        ky = SQ_Y(s->obj.kirby3);
        x = SQ_X(&s->obj);
        y = SQ_Y(&s->obj);
        if (ky > y - WATCH_RANGE_Y && ky < y + WATCH_RANGE_Y && kx > x - WATCH_RANGE_X && kx < x + WATCH_RANGE_X) {
            s->brain = BRAIN_WATCH;
            BRAIN_WAIT(s) = WATCH_TIME;
        }
        break;
    case BRAIN_WATCH:       // 0x020700FC;
        if (BRAIN_WAIT(s) == 0) {
            BRAIN_WAIT(s) = TURN_WAIT;
            s->padDir = -1;
            s->brain = BRAIN_TURN_AWAY;
            break;
        }
        BRAIN_WAIT(s)--;
        if (BRAIN_COUNT(s) == 0) s->padDir = 0;
        if (BRAIN_COUNT(s) == WATCH_FACE) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            BRAIN_COUNT(s) = 0;
        } else {
            BRAIN_COUNT(s)++;
        }
        break;
    case BRAIN_TURN_AWAY:   // 0x020700D8;
        if (BRAIN_WAIT(s) == 0) {
            s->padDir = 0;
            s->padY = PAD_FLY_AWAY;
        } else {
            BRAIN_WAIT(s)--;
        }
        break;
    }
}

/* Inhaled(); Its hit reaction to an inhale (0x02072E3C, kinds 3-5): KatAM's inhale reach around it (NidlInhaler()); not blowing yet,
 * it faces the inhaler and blows back; */
static void Inhaled(struct SqueakObject *s) {
    struct Kirby *kirby;

    if (s->obj.unk80 <= 0 || s->state == BODY_ALONE || (kirby = NidlInhaler(&s->obj.base)) == NULL) return;
    INHALED(s) = TRUE;
    if (!BLOWING(s)) {
        FaceInhaler(s, kirby);
        SqueakSetState(s, BODY_BLOW_ENTRY);
    }
}

static void CretMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    Inhaled(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    if (!(obj->base.flags & 0x1000)) SqueakAnimate(s);
}

// The brain's first state per variant (0x020705AC's switch on the spawn's first parameter byte);
static const u8 sFirstBrain[] = { BRAIN_DRIFT_FACE, BRAIN_LOOP_FACE, BRAIN_WATCH_FACE, BRAIN_LOOP_FACE };

void CretStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, CretMain);
    SqueakThroughTerrain(s, TRUE);
    if (!s->started) s->brain = s->variant < ARRAY_COUNT(sFirstBrain) ? sFirstBrain[s->variant] : BRAIN_DRIFT_FACE;
    BLOWING(s) = INHALED(s) = FALSE;
    SqueakSetState(s, BODY_START);
}

/* CretInit(); Its box (cret.rca's first: 6 pixels either side, 3 under its spot to 15 over it); KatAM's inhale can't take it (unk5C
 * 0x20, as Maiga's); */
void CretInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -15, 6, 3);
    ObjectSetBounds(&s->obj.base, -6, -15, 6, 3);
    s->obj.base.unk5C |= 0x20;
}

/* Its ball (0x020734EC, 0x02073438): falls (BALL_GRAVITY a frame, no faster than BALL_MAX_FALL), spinning; on whatever it hits it
 * bursts and, unseen, is gone 8 frames later (0x020732F8, 0x020732C8); out of the stage, it's gone; */
static bool32 Ball(struct SqueakObject *s) {
    switch (s->state) {
    case 0:
        SqueakPlay(s, ANIM_BALL);
        s->vx = s->vy = 0;
        s->ay = -BALL_GRAVITY;
        SqueakSetState(s, 1);
        return TRUE;
    case 1:
        if (s->vy <= BALL_MAX_FALL) s->vy = BALL_MAX_FALL;
        SqueakMove(s);
        if (!SqueakInLevel(s)) {
            s->obj.base.flags |= 0x1000;
        } else if (s->terrain || SqueakOnGround(s)) {
            SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
            SqueakStop(s);
            SqueakPlay(s, ANIM_BALL_STILL);
            s->obj.base.flags |= 0x400 | 0x200;
            s->vars[0] = 0;
            SqueakSetState(s, 2);
        }
        break;
    case 2:
        if (s->vars[0]++ >= BALL_GONE) s->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void CretBallMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Ball);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void CretBallStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, CretBallMain);
    SqueakSetState((struct SqueakObject *)obj, 0);
}

// CretBallInit(); Its box (cret.rca's for the ball: 4 pixels behind to 6 ahead, 1 to 10 up, as Maiga's: about its size);
void CretBallInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -4, -9, 6, 0);
    ObjectSetBounds(&s->obj.base, -4, -9, 6, 0);
}
