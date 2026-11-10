/* Bubble Head; ported from Kirby: Squeak Squad (USA), enemy 0x27 (its body 0x020968D0, vtable 0x02171DBC; brain 0x02095980; its
 * bubbles 0x0209702C, blown by 0x02097114), as SQUEAK_BUBBLE_HEAD and SQUEAK_BUBBLE_HEAD_BUBBLE. Values from param/prmCE21Ml0.kgb
 * (movement; MlG differs only in the other mode's walk), prmCE21Pl0.kgb (brain) and prmCE21Sl0.kgb (HP 20); speeds 20.12 a frame, Y
 * up; its boxes from enemy/bubblehead.rca; it moves as bubblehead.mca's composite animations. A walker that blows bubbles: near Kirby
 * it blows a long stream of them at him, or one bubble that floats along the floor.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12): it faces Kirby (past 1 pixel), then
 *   0 (0x020958B4) and any but 1, 2 and 5: it walks (60 frames, then looks again: 0x02095804), turning at walls (0x02095858); with
 *   Kirby within 96 pixels across and 96 up or down (0x020957CC): within 64 across and 48 up or down, half the time (0x0209575C) it
 *   faces him and holds B 21 frames (the big blow: 0x02095734, 0x020956F0), else (0x0209562C) half the time it faces him and presses
 *   B one frame (one bubble: 0x020955DC, 0x02095594; Kirby 16 pixels above it, up too, let go 3 frames on: before the bubble's
 *   blown, so it never blows one up); then it walks 311 frames after the big blow (0x0209569C), 151 after a bubble (0x02095540)
 *   before it looks again; the other half of the time it walks on;
 *   2 (0x020954F8): the same standing (its pad's direction 0: it only turns at walls);
 *   1 and 5 (0x02095520): it faces Kirby, then stands.
 * • Body: it stands (composite 0, 0x02096708, 0x0209664C) or walks at 0.25 (composite 1, 0x020965A4, 0x020964B0), falling 0.1 a
 *   frame no faster than 2.4 (composite 0: 0x020961E8, 0x020960F4; forward keeps it going in the air: 0x020967B0); A jumps (2.5 up,
 *   composite 7: 0x0209637C, 0x020962F4; its brains never press A); B (0x02096044) draws breath (composite 2, 0x02095EC4): held past
 *   20 frames it blows the big stream (composite 3, then 4: 0x02095C28, 0x02095B74), 150 frames; let go before, it blows a bubble
 *   (composite 5, then 6: 0x02095DF4, 0x02095D04) 16 pixels ahead, 2 under its feet, and stands 20 frames on.
 * • Its bubble (0x02096CB8; anim 25): flies on at 1.6 along the floor (pushed up out of it to its feet's level: its first frame 3.2,
 *   as on the DS) 180 frames, or 1.5 forward and 0.55 up (0x02096D8C, kind 2: pad up, which no brain of its gives); a wall ahead,
 *   the terrain all round it or a ceiling (0x02096C10), or 180 frames over, it stops and pops (0x02096BC8, 0x02096B9C: 8 pixels up
 *   off the floor; unhurting, it waits for its animation's next loop, then a star burst, efc_cmn0's animation 7); touching Kirby (his
 *   body's attack, its touch reaction 0x02096E68's flag 0x400) it pops as it goes on; out of the stage it's gone; blown where the
 *   spot 2 pixels over it is solid, it pops at once. Kirby's inhale doesn't take it (0x02096E68 ignores kinds 3-5: KatAM's unk5C
 *   0x20, as Cret's).
 * • Boxes (bubblehead.rca: its composites' animations, the .rca's 19-26; the bubble's, 18): its body's hit box (8 pixels either side,
 *   1 to 17 up, its kind 0 touch box in it); the big blow's stream (kind 8) 10 to 50 pixels ahead, 9 to 17 up: KatAM's one box takes
 *   both, and Kirby's body doesn't hurt it then (as Bio Spark's slash); the bubble's (kind 8) 6 pixels either side, 6 to 18 up. Kind 8
 *   is Squeak Squad's plain hurt (Kirby's hit reaction 0x0201917C: 8 of 36 HP, as recorded): KatAM's touch.
 * Its sounds (Squeak Squad's, SqueakSfx): 0x145 the big blow, 0x150 a bubble. Not ported: its ability (Squeak Squad's Bubble: KatAM has none), the other mode
 * (the brain's +8 not 3: B and A in the air, MlG's walk), its contact damages (Squeak Squad's 8 for the bubbles and stream: KatAM's
 * 1). Kirby's attacks beat its bubbles as KatAM's (Squeak Squad's those with flag 0x400 pop them). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

// Composite animations (bubblehead.mca) and the bubble's animation (bubblehead.cea);
enum { MCA_STAND, MCA_WALK, MCA_CHARGE, MCA_BIG_START, MCA_BIG, MCA_SHOT_START, MCA_SHOT, MCA_JUMP };
#define ANIM_BUBBLE 25

// prmCE21Ml0.kgb;
#define WALK_SPEED 0x400
#define JUMP_SPEED 0x2800
#define GRAVITY 0x19A
#define CHARGE_TIME 20
#define SHOT_X 0x10000
#define SHOT_Y 0x2000
#define SHOT_TIME 20
#define BIG_TIME 150
#define UP_SPEED_X 0x180D
#define UP_SPEED_Y 0x8C1
#define BUBBLE_SPEED 0x199A
// prmCE21Pl0.kgb;
#define NEAR_X 0x60000
#define NEAR_Y 0x60000
#define CLOSE_X 0x40000
#define CLOSE_Y 0x30000
#define WALK_TIME 60
#define BIG_CHANCE 50
#define BIG_HOLD 20
#define BIG_REST 310
#define SHOT_CHANCE 50
#define SHOT_HOLD 2
#define SHOT_REST 150
#define ABOVE 0x10000
// The code's;
#define MAX_FALL -0x2666
#define FACE_DISTANCE 0x1000
#define BUBBLE_TIME 180
#define POP_RISE 0x800                  // The pop's 8 pixels up (KatAM's units);
#define BUBBLE_MIDDLE 12                // Its box's middle, 12 pixels over its spot (the terrain all round it);

enum {
    BODY_START, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_FALL_ENTRY, BODY_FALL, BODY_JUMP_ENTRY, BODY_JUMP, BODY_BLOW_ENTRY,
    BODY_CHARGE, BODY_BIG_START, BODY_BIG, BODY_SHOT_START, BODY_SHOT,
};
enum {
    BRAIN_START, BRAIN_CHECK, BRAIN_WALK, BRAIN_CLOSE, BRAIN_BIG_PICK, BRAIN_SHOT_PICK, BRAIN_BLOW, BRAIN_BLOW_HOLD,
    BRAIN_BLOW_REST, BRAIN_SHOT, BRAIN_SHOT_HOLD, BRAIN_SHOT_REST, BRAIN_STILL_START, BRAIN_STILL,
};

#define COUNT(s) ((s)->vars[0])         // 0x1F8: the blow's frames;
#define WAIT(s) ((s)->vars[1])          // The brain's 0x98;
#define HOLD(s) ((s)->vars[2])          // The brain's 0x9C;
#define WALK_DIR(s) ((s)->vars[3])      // Its brain's way to walk: 1, or 0 (variant 2);
#define TIMER(s) ((s)->vars[0])         // The bubble's 0x12C;

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// Gravity(); 0x02095B1C: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// AirControl(); 0x020967B0: back turns it round; forward, it goes at WALK_SPEED (no cap); none, it stops across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = WALK_SPEED;
        s->maxVx = 0;
    } else if (s->padDir == 0) {
        s->vx = 0;
    }
}

// Buttons(); 0x02095A94, 0x02095AD8: B blows, A jumps;
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_BLOW_ENTRY);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x02096708 (vtable +0x3C);
        SqueakPlayComposite(s, MCA_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x0209664C;
        if (Buttons(s)) return TRUE;
        if (s->padDir != 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_WALK_ENTRY:   // 0x020965A4;
        SqueakPlayComposite(s, MCA_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x020964B0;
        if (Buttons(s)) return TRUE;
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = WALK_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x020961E8: its speeds as they were;
        SqueakPlayComposite(s, MCA_STAND);
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020960F4: landed, it walks on if it was going across;
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, s->vx == 0 ? BODY_START : BODY_WALK_ENTRY);
            s->vy = s->ay = s->ax = 0;
            return TRUE;
        }
        break;
    case BODY_JUMP_ENTRY:   // 0x0209637C;
        SqueakPlayComposite(s, MCA_JUMP);
        s->vy = s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_JUMP);
        return TRUE;
    case BODY_JUMP:         // 0x020962F4;
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_BLOW_ENTRY:   // 0x02096044;
        SqueakPlayComposite(s, MCA_CHARGE);
        SqueakStop(s);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_CHARGE);
        return TRUE;
    case BODY_CHARGE:       // 0x02095EC4: held past CHARGE_TIME, the big blow; let go before, a bubble;
        if (COUNT(s) > CHARGE_TIME) {
            SqueakPlayComposite(s, MCA_BIG_START);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_BIG_START);
            return TRUE;
        }
        if (!s->padB) {
            SqueakPlayComposite(s, MCA_SHOT_START);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_SHOT_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        COUNT(s)++;
        break;
    case BODY_BIG_START:    // 0x02095C28;
        if (s->animEnded) {
            SqueakPlayComposite(s, MCA_BIG);
            SqueakLoopSfx(s, 0x145);
            SqueakSetState(s, BODY_BIG);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_BIG:          // 0x02095B74: the stream, BIG_TIME frames;
        if (COUNT(s) >= BIG_TIME) {
            COUNT(s) = 0;
            SqueakStopSfx(s, 0x145);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        COUNT(s)++;
        break;
    case BODY_SHOT_START:   // 0x02095DF4;
        if (s->animEnded) {
            SqueakPlayComposite(s, MCA_SHOT);
            SqueakSetState(s, BODY_SHOT);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SHOT:         // 0x02095D04: its first frame, the bubble (0x02097114: pad up, kind 2), SHOT_X ahead, SHOT_Y down;
        if (COUNT(s) >= SHOT_TIME) {
            COUNT(s) = 0;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (COUNT(s) == 0) {
            SqueakSpawn(s, SQUEAK_BUBBLE_HEAD_BUBBLE, s->padY < 1 ? 1 : 2, SHOT_X, -SHOT_Y);
            SqueakSfx(s, 0x150);
        }
        Gravity(s);
        SqueakMove(s);
        COUNT(s)++;
        break;
    }
    return FALSE;
}

// Walk(); Its brain's walk on (0x02095804's and the rests'): its way, back at a wall;
static void Walk(struct SqueakObject *s) {
    s->padDir = WALK_DIR(s);
    if (SqueakWallAhead(s)) s->padDir = -1;
}

/* Brain(); Its pad (0x020958B4 and the states it leads to; variant 2's 0x020954F8 the same, its way 0; variants 1 and 5's
 * 0x02095520): */
static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_START:       // 0x020958B4, 0x020954F8;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_CHECK;
        break;
    case BRAIN_CHECK:       // 0x02095858, 0x0209549C: Kirby near, a closer look; else it walks;
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) {
            s->brain = BRAIN_CLOSE;
        } else {
            WAIT(s) = 0;
            s->brain = BRAIN_WALK;
        }
        Walk(s);
        break;
    case BRAIN_WALK:        // 0x02095804, 0x02095448;
        if (WAIT(s) > WALK_TIME) {
            WAIT(s) = 0;
            s->brain = BRAIN_CHECK;
        }
        Walk(s);
        WAIT(s)++;
        break;
    case BRAIN_CLOSE:       // 0x020957CC, 0x02095410;
        s->brain = SqueakKirbyWithin(s, CLOSE_X, CLOSE_Y) ? BRAIN_BIG_PICK : BRAIN_SHOT_PICK;
        break;
    case BRAIN_BIG_PICK:    // 0x0209575C, 0x020953A0;
        if (Random() < BIG_CHANCE) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_BLOW;
        } else {
            s->brain = BRAIN_WALK;
        }
        break;
    case BRAIN_SHOT_PICK:   // 0x0209562C, 0x02095270;
        if (Random() < SHOT_CHANCE) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_SHOT;
        } else {
            s->brain = BRAIN_WALK;
        }
        break;
    case BRAIN_BLOW:        // 0x02095734, 0x02095378;
        HOLD(s) = 0;
        s->padDir = 0;
        s->padB = 1;
        s->brain = BRAIN_BLOW_HOLD;
        break;
    case BRAIN_BLOW_HOLD:   // 0x020956F0, 0x02095334;
        if (HOLD(s) > BIG_HOLD) {
            s->padDir = WALK_DIR(s);
            s->padB = 0;
            s->brain = BRAIN_BLOW_REST;
            WAIT(s) = 0;
        }
        HOLD(s)++;
        break;
    case BRAIN_BLOW_REST:   // 0x0209569C, 0x020952E0;
        if (WAIT(s) > BIG_REST) {
            WAIT(s) = 0;
            s->brain = BRAIN_CHECK;
        }
        Walk(s);
        WAIT(s)++;
        break;
    case BRAIN_SHOT:        // 0x020955DC, 0x02095220: Kirby ABOVE over it, up too;
        HOLD(s) = 0;
        s->padDir = 0;
        if (SQ_Y(s->obj.kirby3) > SQ_Y(&s->obj) + ABOVE) s->padY = 1;
        s->padB = 1;
        s->brain = BRAIN_SHOT_HOLD;
        break;
    case BRAIN_SHOT_HOLD:   // 0x02095594, 0x020951D8;
        s->padB = 0;
        if (HOLD(s) > SHOT_HOLD) {
            s->padDir = WALK_DIR(s);
            s->padY = 0;
            s->brain = BRAIN_SHOT_REST;
            WAIT(s) = 0;
        }
        HOLD(s)++;
        break;
    case BRAIN_SHOT_REST:   // 0x02095540, 0x02095184;
        if (WAIT(s) > SHOT_REST) {
            WAIT(s) = 0;
            s->brain = BRAIN_CHECK;
        }
        Walk(s);
        WAIT(s)++;
        break;
    case BRAIN_STILL_START: // 0x02095520;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:       // 0x02095518;
        s->padDir = 0;
        break;
    }
}

static void BubbleHeadMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    /* The big blow's stream (bubblehead.rca's kind 8, 10 to 50 pixels ahead) and its body in KatAM's one box; Kirby's body (KatAM's
     * attack kind 0x80, its touch) doesn't hurt it then: the stream only hurts him there, as on the DS; */
    if (s->composite == MCA_BIG) {
        sub_0803E2B0(&obj->base, -8, -17, 50, -1);
        obj->base.unk5C |= 0x80;
    } else {
        sub_0803E2B0(&obj->base, -8, -17, 8, -1);
        obj->base.unk5C &= ~0x80;
    }
}

void BubbleHeadStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, BubbleHeadMain);
    SqueakThroughTerrain(s, FALSE);
    if (!s->started) {
        WALK_DIR(s) = s->variant == 2 ? 0 : 1;
        s->brain = s->variant == 1 || s->variant == 5 ? BRAIN_STILL_START : BRAIN_START;
    }
    SqueakSetState(s, BODY_START);
}

/* BubbleHeadInit(); Its box (bubblehead.rca's hit box, 8 pixels either side, 1 to 17 up; its body box 6 either side, 1 to 13 up:
 * here from its feet, KatAM's bounds), facing left (0x020968D0); */
void BubbleHeadInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -17, 8, -1);
    ObjectSetBounds(&s->obj.base, -6, -13, 6, 0);
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}

/* Its bubble (0x0209702C: kind 2 up, any other forward; its own move on the terrain, the stage's edges not holding it, Squeak Squad's
 * flag 0x20000); */

// Solid(); The terrain at (dx, dy) pixels from the bubble's spot (y down);
static bool32 Solid(struct SqueakObject *s, s32 dx, s32 dy) {
    return NidlIsFloor(&s->obj, s->obj.base.x + dx * 0x100, s->obj.base.y + dy * 0x100);
}

enum { BUBBLE_BORN, BUBBLE_START, BUBBLE_FLY, BUBBLE_POP, BUBBLE_POPPING };

static bool32 Bubble(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;

    switch (s->state) {
    case BUBBLE_BORN:       // (blown this frame: Squeak Squad's runs it from the next, KatAM's this one);
        SqueakSetState(s, BUBBLE_START);
        break;
    case BUBBLE_START:      // 0x02096CB8, 0x02096D8C;
        b->flags &= ~0x400;
        SqueakPlay(s, ANIM_BUBBLE);
        if (s->variant == 2) {
            s->vx = UP_SPEED_X;
            s->vy = UP_SPEED_Y;
        } else {
            s->vx = BUBBLE_SPEED;
            s->vy = 0;
        }
        TIMER(s) = BUBBLE_TIME;
        // (blown 2 pixels into the floor, Squeak Squad's move pushes it out, to its feet's level);
        if (Solid(s, 0, 0)) b->y = (b->y & ~0xFFF) - 1;
        SqueakSetState(s, BUBBLE_FLY);
        return TRUE;
    case BUBBLE_FLY:        // 0x02096C10: a wall ahead, the terrain all round it (0x8000), a ceiling, or its time over: it pops;
        SqueakMove(s);
        if (!SqueakWallAhead(s) && !Solid(s, 0, -BUBBLE_MIDDLE) && !SqueakCeiling(s) && TIMER(s) > 0) {
            TIMER(s)--;
        } else {
            s->vx = s->vy = 0;
            SqueakSetState(s, BUBBLE_POP);
        }
        if (!SqueakInLevel(s)) b->flags |= 0x1000;
        break;
    case BUBBLE_POP:        // 0x02096BC8: 8 pixels up (on the floor, Squeak Squad's move keeps it there); it touches nothing more;
        b->unk68 = 0;
        b->flags |= 0x200;
        if (!Solid(s, 0, 1) && !SqueakCeiling(s)) b->y -= POP_RISE;
        SqueakSetState(s, BUBBLE_POPPING);
        return TRUE;
    case BUBBLE_POPPING:    // 0x02096B9C: at its animation's next loop, a star burst (0x020B48E0's type 3), and gone;
        SqueakMove(s);
        if (TIMER(s) < 0 ? ++TIMER(s) == 0 : s->animEnded) {
            SqueakSpawnEffect(s, SQUEAK_FX_STARS, 0, 0, FALSE);
            b->flags |= 0x1000;
        }
        break;
    }
    return FALSE;
}

/* BubbleHeadBubbleMain(); Its first frame Squeak Squad runs it twice (as recorded: its sprite's time 2 after it, its move 3.2, its
 * time 178); touching Kirby, his body's attack pops it, going on as it was (0x02096E68: where KatAM's touch found it, before it
 * moves); */
static void BubbleHeadBubbleMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    u8 runs = s->state == BUBBLE_START ? 2 : 1;

    while (runs--) {
        if (s->state == BUBBLE_FLY && SqueakTouchingKirby(s)) SqueakSetState(s, BUBBLE_POP);
        SqueakRun(s, Bubble);
        SqueakFinishFrame(s);
        SqueakAnimate(s);
    }
}

/* BubbleHeadBubbleStart(); Unseen till its start (KatAM's 0x400: Squeak Squad shows it from then); blown where the spot 2 pixels over
 * it is solid (Squeak Squad's stage vtable +0x14), it pops at once (0x02097114): its start never runs, so no animation shows and the
 * next frame it's done (the sprite's query with none playing); a hit it lives through (KatAM's) has it go on as it was; */
void BubbleHeadBubbleStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, BubbleHeadBubbleMain);
    SqueakThroughTerrain(s, FALSE);
    if (s->started) return;
    obj->base.flags |= 0x400;
    // (2 pixels over it: its blower's feet; the floor's top line isn't in it, as on the DS);
    if (NidlIsFloor(obj, obj->base.x, obj->base.y - 0x200 - 1)) {
        TIMER(s) = -2;
        SqueakSetState(s, BUBBLE_POP);
    } else {
        SqueakSetState(s, BUBBLE_BORN);
    }
}

/* BubbleHeadBubbleInit(); Its box (bubblehead.rca's 18: 6 pixels either side, 6 to 18 up; its body box 4 either side to 16 up, its
 * feet at its spot); Kirby's body doesn't hurt it (it pops instead) nor his inhale take it; */
void BubbleHeadBubbleInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -18, 6, -6);
    ObjectSetBounds(&s->obj.base, -4, -16, 4, 0);
    s->obj.base.unk5C |= 0xA0;
}
