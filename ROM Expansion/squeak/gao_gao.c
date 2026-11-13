/* Gao Gao; ported from Kirby: Squeak Squad (USA), mid-boss 0x33 (its body 0x02100300, vtable 0x0217FF88; brain 0x020FE364, its states
 * 0x020FDD74-0x020FE274), as SQUEAK_GAO_GAO, a mid-boss on mini-boss type 0x40 (squeak.h's SQUEAK_MIDBOSS). Values from
 * param/prmMB09Ml0.kgb (movement: the jump 1.2 across and 2.5 up, gravity 0x19A, the hop back 0.5 and 1.8 up, falls no faster than
 * 3.6), prmMB09Pl0.kgb (brain) and prmMB09Sl0.kgb (HP 140); speeds 20.12 a frame, Y up; its boxes from enemy/gaogao.rca. A clawed
 * beast that hops about, spins at Kirby with its claws out, and drills up off the screen to crash down where he is.
 * • Its fight starts as a mini-boss's (the framework's wait, camera lock, HP bar and music); then, as Squeak Squad's (0x020FFBD4,
 *   0x020FFB40, 0x020FFAC4), it waits 75 frames, untouchable, falling at 0x2B8 a frame to 2.4; its brain runs meanwhile.
 * • Brain (Squeak Squad's random 0-99 for each pick; one state a frame; "landed": its body on the ground and standing): it faces Kirby
 *   (0x020FE274) and rests 60 frames (0x020FE1C0), then faces him and picks: 10 in 100 rests again, 30 jumps in place (A: 0x020FE124),
 *   30 jumps forward (A forward: 0x020FDFE8), 30 hops back (A with down: 0x020FDF68). Landed (0x020FE064), it faces him and picks again:
 *   10 rests (0x020FE1A0), 30 jumps forward, 30 hops back, 30 jumps twice in a row (0x020FDF50, 0x020FDF28: forward, but the
 *   state after lets go of forward before its crouch is over, so in place: logged on the DS too). Past 5 jumps in all, 3 in place,
 *   3 forward or 3 hops back (each count starting over; Squeak Squad never sets the last three's first values, its memory's
 *   leftovers: here 0), it attacks instead (0x020FDE1C): half the time (while it
 *   has spun fewer than 3 times in a row) it spins (B: 0x020FDDFC, 0x020FDDCC), else it drills up (B with up: 0x020FDDAC,
 *   0x020FDD74; its spins in a row counted afresh; every 4th time it spins instead); after a spin it picks as landed, after a drill
 *   it hops back.
 * • Body: it stands (anim 1: 0x020FFA24, 0x020FF940): B with up drills, B spins, A with down hops back, A jumps; back turns it round;
 *   off the ground it falls (anim 2: 0x020FF360, 0x020FF274, gravity 0x19A, no faster than 3.6) and lands (anim 3, a puff: 0x020FF214).
 *   Its jump (0x020FF88C, 0x020FF7D0): it crouches (anim 2), leaps 2.5 up (1.2 forward with the pad forward, and a puff of dust),
 *   lands (0x020FF774, 0x020FF678: anim 3, a big puff, the screen shaking 6 frames: KatAM's shortest, 5) and stands once anim 3 is
 *   over (0x020FF618). Its hop back (0x020FF534, 0x020FF4E0, 0x020FF45C): 0.5 back and 1.8 up, landing with a puff. Its spin
 *   (0x020FF160, 0x020FF068, 0x020FEEFC): it crouches (anim 4) and spins (anim 5) 120 frames at Kirby (0x733 a frame to his right,
 *   0x59A to his left: Squeak Squad's two), a puff every 20 frames, knocking out two stars (50 pixels ahead and behind it, 20 up,
 *   SqueakStar()); then it pants (anim 8, 0x020FEE64) 135 frames, swaying 0.6 forward, back from its 15th frame and forward again
 *   for its last 40. Its drill (0x020FEDB8, 0x020FECA0, 0x020FEB14, 0x020FEAB8, 0x020FE9E8): it crouches (anim 4), drills up (anim 6)
 *   through everything at 4.0 with a big puff (the screen shaking: KatAM's shortest), till its feet are at the screen's top; 20
 *   frames there, then it moves over Kirby (55 in 100) or, 15 in 100 each, the screen's left, middle or right (Squeak Squad's: 54,
 *   128 and 202 pixels into its 256: here 74 either side of the middle of KatAM's 240), and the screen rumbles 20 frames (KatAM's
 *   18-frame shake, 2) as it drills down (anim 7) at 0.6 for 20 frames, then at 4.0 through everything till it's below where it
 *   started plus 8 pixels; then it falls and, landed, faces Kirby with a big puff and two stars (32 pixels either side), the screen
 *   shaking 18 frames (KatAM's 2).
 * • Hit, it doesn't flinch (0x020FFD10, 0x020FFDF4: a shake, 36 frames safe): KatAM's mini-bosses don't either (sub_0809AF38); its
 *   defeat is KatAM's mini-boss's.
 * Its sounds (Squeak Squad's, SqueakSfx): 0x10D, 0x14A, 0x14E, 0x152, 0x153, 0x155, 0x165, 0x178. Not ported: the DS's rumble (0x0221C6EC), Squeak Squad's hit
 * flash (its _dmg palette) and shakes (KatAM's), its defeat (0x020FE8BC: a hop, a slide shedding dust, a shudder: KatAM's defeat
 * instead), its ability (Animal, squeak.c's table: inhaled once beaten, as the DS's), the other mode's air control (0x020FFCAC). Squeak Squad holds it in the stage as it drills up (it
 * would wait there for good under a stage top no higher than the screen's; its arenas leave room above): here it goes on up past
 * the room's top, KatAM's mini-boss rooms often having their camera at the top. */
#include "../squeak.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (gaogao.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_JUMP, ANIM_LAND, ANIM_CROUCH, ANIM_SPIN, ANIM_DRILL_UP, ANIM_DRILL_DOWN, ANIM_PANT, ANIM_DEFEATED };

// prmMB09Ml0.kgb;
#define JUMP_VX 0x1334
#define JUMP_VY 0x2800
#define GRAVITY 0x19A
#define HOP_VY 0x1CCD
#define HOP_VX 0x800
#define MAX_FALL -0x399A
// Its body's own;
#define INTRO_GRAVITY 0x2B8
#define INTRO_FALL -0x2666
#define INTRO 75
#define SPIN_TIME 120
#define SPIN_PUFF 20
#define SPIN_RIGHT 0x733
#define SPIN_LEFT 0x59A
#define PANT_TIME 135
#define PANT_BACK 120
#define PANT_FORTH 40
#define PANT_SPEED 0x99A
#define DRILL_SPEED 0x4000
#define DRILL_TOP 20
#define DRILL_AIM 20
#define DRILL_AIM_SPEED 0x99A
#define DRILL_ABOVE 8                   // Pixels above where it started: below that, the terrain stops it again;
#define AIM_SIDE 74                     // Squeak Squad's 54/128/202 of its 256-pixel screen: 74 either side of the middle;
#define STAR_SPIN_X 0x32000             // The stars it knocks out (20.12): spinning, 50 ahead and behind, 20 up; landing, 32 either side;
#define STAR_SPIN_Y 0x14000
#define STAR_LAND_X 0x20000
// prmMB09Pl0.kgb and the brain's own;
#define FACE_DISTANCE 0x1000
#define REST 60
// KatAM's screen shakes (RequestScreenShake 1-3: 14, 18, 24 frames; 4-6 on till stopped): 14 frames (its shortest), 18;
#define SHAKE_SHORT 1
#define SHAKE_LAND 2

enum {
    BODY_START, BODY_STAND, BODY_INTRO, BODY_FALL_ENTRY, BODY_FALL, BODY_LANDED, BODY_JUMP_ENTRY, BODY_JUMP_CROUCH, BODY_JUMP_RISE,
    BODY_JUMP_FALL, BODY_JUMP_LAND, BODY_HOP_ENTRY, BODY_HOP_RISE, BODY_HOP_FALL, BODY_SPIN_ENTRY, BODY_SPIN_CROUCH, BODY_SPIN,
    BODY_PANT, BODY_DRILL_ENTRY, BODY_DRILL_CROUCH, BODY_DRILL_UP, BODY_DRILL_AIM, BODY_DRILL_DOWN,
};
enum {
    BRAIN_START, BRAIN_REST, BRAIN_REST_AGAIN, BRAIN_JUMP, BRAIN_JUMP_FORWARD, BRAIN_HOP, BRAIN_LANDING, BRAIN_TWO_JUMPS,
    BRAIN_JUMP_ON, BRAIN_ATTACK, BRAIN_SPIN, BRAIN_SPIN_WAIT, BRAIN_DRILL, BRAIN_DRILL_WAIT,
};

#define BUSY(s) ((s)->vars[0])          // 0x204: 0 standing, 4 jumping, 5 hopping back, 6 falling, 7 spinning, 8 drilling;
#define COUNT(s) ((s)->vars[1])         // 0x210: the intro's, the spin's, the pant's frames, the drill's at the top;
#define AIM(s) ((s)->vars[2])           // 0x208: the drill's slow frames down;
#define AT_TOP(s) ((s)->vars[3])        // 0x220: the drill is at the screen's top;
#define START_Y(s) ((s)->vars[4])       // 0x1A4: where the drill started (KatAM's y);
#define BRAIN_TIMER(s) ((s)->vars[5])   // The brain's 0xB4;
#define JUMPS(s) ((s)->vars[6])         // The brain's 0x98: jumps in all; 0x9C, 0xA0, 0xA4: in place, forward, hops back;
#define JUMPS_UP(s) ((s)->vars[7])
#define JUMPS_FORWARD(s) ((s)->vars[8])
#define HOPS(s) ((s)->vars[9])
#define SPINS(s) ((s)->vars[10])        // The brain's 0xA8: spins in a row; 0xAC: attacks since a spin was forced; 0xB0: jumps on;
#define DRILLS(s) ((s)->vars[11])
#define JUMPS_ON(s) ((s)->vars[12])
#define BOX(s) (*(u8 *)&(s)->vars[13])  // This frame's box;
#define HARM(s) ((s)->vars[14])         // KatAM's unk68 from InitObject: its touch's harm;

// gaogao.rca's boxes (its animations from the .rca's second; boxes.py);
static const struct SqueakBox sGaoGaoBoxes[] = {
    { { -23, -34, 29, 0 }, TRUE, 0 },
    { { -25, -44, 29, 2 }, TRUE, 0 },
    { { -25, -47, 29, 2 }, TRUE, 0 },
    { { -25, -34, 15, 2 }, TRUE, 0 },
    { { -38, -37, 38, 0 }, TRUE, 0 },
    { { -17, -66, 15, 0 }, TRUE, 0 },
    { { -34, -34, 30, 0 }, TRUE, 0 },
    { { -34, -34, 29, 0 }, TRUE, 0 },
    { { -24, -34, 29, 0 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sGaoGaoAnim0[] = { { 6, 0 } };
static const struct SqueakBoxFrame sGaoGaoAnim1[] = { { 74, 0 } };
static const struct SqueakBoxFrame sGaoGaoAnim2[] = { { 3, 0 }, { 3, 1 }, { 3, 2 } };
static const struct SqueakBoxFrame sGaoGaoAnim3[] = { { 3, 1 }, { 3, 0 } };
static const struct SqueakBoxFrame sGaoGaoAnim4[] = { { 23, 3 } };
static const struct SqueakBoxFrame sGaoGaoAnim5[] = { { 8, 4 } };
static const struct SqueakBoxFrame sGaoGaoAnim6[] = { { 8, 5 } };
static const struct SqueakBoxFrame sGaoGaoAnim7[] = { { 8, 5 } };
static const struct SqueakBoxFrame sGaoGaoAnim8[] = { { 27, 6 }, { 27, 7 } };
static const struct SqueakBoxFrame sGaoGaoAnim9[] = { { 1, 8 } };
static const struct SqueakBoxAnim sGaoGaoBoxAnims[] = {
    { sGaoGaoAnim0, 1 }, { sGaoGaoAnim1, 1 }, { sGaoGaoAnim2, 3 }, { sGaoGaoAnim3, 2 }, { sGaoGaoAnim4, 1 },
    { sGaoGaoAnim5, 1 }, { sGaoGaoAnim6, 1 }, { sGaoGaoAnim7, 1 }, { sGaoGaoAnim8, 2 }, { sGaoGaoAnim9, 1 },
};
static const struct SqueakBoxes sGaoGaoBoxData = { sGaoGaoBoxes, sGaoGaoBoxAnims, 10 };

// Roll(); Squeak Squad's random 0-99 (its LCG at 0x01FF8120: here KatAM's);
static s32 Roll(void) {
    return Rand16() * 100 >> 16;
}

// Gravity(); 0x020FE588: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// Puff(); 0x02021864 / 0x0202181C: a puff where it is (Squeak Squad's effect types 1, 2 and 3), and two stars either side;
static void Puff(struct SqueakObject *s, u8 effect, bool8 stars) {
    if (stars) {
        SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, STAR_LAND_X, 0);
        SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_BEHIND, -STAR_LAND_X, 0);
    }
    SqueakSpawnEffect(s, effect, 0, 0, FALSE);
}

// FaceKirbyNow(); 0x020FE9E8's: turned to face Kirby;
static void FaceKirbyNow(struct SqueakObject *s) {
    if (s->obj.kirby3->base.x < s->obj.base.x ? !(s->obj.base.flags & 1) : (s->obj.base.flags & 1)) SqueakTurnAround(s);
}

// MoveFree(); The drill's move: through everything, not held in the room (see above);
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

// Buttons(); 0x020FE51C (B: with up the drill, else the spin) and 0x020FE4B0 (A: with down the hop back, else the jump);
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, s->padY > 0 ? BODY_DRILL_ENTRY : BODY_SPIN_ENTRY);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, s->padY < 0 ? BODY_HOP_ENTRY : BODY_JUMP_ENTRY);
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    struct LevelInfo *level = &gCurLevelInfo[base->unk56];
    s32 w;

    switch (s->state) {
    case BODY_START:        // 0x020FFA24 (vtable +0x3C);
        BUSY(s) = 0;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x020FF940;
        if (Buttons(s)) return TRUE;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_INTRO:        // 0x020FFAC4: untouchable, falling, INTRO frames;
        if (COUNT(s) < 1) {
            base->flags &= ~0x200;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        SqueakCapFall(s, INTRO_FALL);
        SqueakMove(s);
        break;
    case BODY_FALL_ENTRY:   // 0x020FF360;
        BUSY(s) = 6;
        SqueakPlay(s, ANIM_JUMP);
        SqueakStop(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020FF274;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_LAND);
            SqueakSfx(s, 0x155);
            Puff(s, SQUEAK_FX_LAND, FALSE);
            SqueakSetState(s, BODY_LANDED);
        }
        break;
    case BODY_LANDED:       // 0x020FF214 (and the jump's, 0x020FF618): its animation over, it stands;
    case BODY_JUMP_LAND:
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_JUMP_ENTRY:   // 0x020FF88C;
        BUSY(s) = 4;
        SqueakPlay(s, ANIM_JUMP);
        SqueakStop(s);
        SqueakSetState(s, BODY_JUMP_CROUCH);
        return TRUE;
    case BODY_JUMP_CROUCH:  // 0x020FF7D0: its crouch over, it leaps (forward with the pad forward, a puff of dust);
        if (s->animEnded) {
            SqueakStop(s);
            if (s->padDir > 0) {
                s->vx = JUMP_VX;
                Puff(s, SQUEAK_FX_DUST, FALSE);
            }
            s->vy = JUMP_VY;
            s->ay = -GRAVITY;
            SqueakSfx(s, 0x14E);
            SqueakSetState(s, BODY_JUMP_RISE);
        }
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        break;
    case BODY_JUMP_RISE:    // 0x020FF774;
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_JUMP_FALL);
        break;
    case BODY_JUMP_FALL:    // 0x020FF678;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            RequestScreenShake(SHAKE_SHORT, base);
            SqueakStop(s);
            SqueakPlay(s, ANIM_LAND);
            SqueakSfx(s, 0x155);
            SqueakSetState(s, BODY_JUMP_LAND);
            Puff(s, SQUEAK_FX_LAND_BIG, FALSE);
        }
        break;
    case BODY_HOP_ENTRY:    // 0x020FF534;
        BUSY(s) = 5;
        SqueakPlay(s, ANIM_JUMP);
        SqueakStop(s);
        s->vx = -HOP_VX;
        s->vy = HOP_VY;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_HOP_RISE);
        return TRUE;
    case BODY_HOP_RISE:     // 0x020FF4E0;
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_HOP_FALL);
        break;
    case BODY_HOP_FALL:     // 0x020FF45C;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            Puff(s, SQUEAK_FX_LAND, FALSE);
            SqueakSfx(s, 0x155);
            SqueakSetState(s, BODY_START);
        }
        break;
    case BODY_SPIN_ENTRY:   // 0x020FF160;
        BUSY(s) = 7;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakStop(s);
        SqueakSetState(s, BODY_SPIN_CROUCH);
        return TRUE;
    case BODY_SPIN_CROUCH:  // 0x020FF068: its crouch over, it spins, knocking out two stars;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_SPIN);
            SqueakSetState(s, BODY_SPIN);
            COUNT(s) = SPIN_TIME;
            SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, STAR_SPIN_X, STAR_SPIN_Y);
            SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_BEHIND, -STAR_SPIN_X, STAR_SPIN_Y);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SPIN:         // 0x020FEEFC: at Kirby (Squeak Squad's speeds: SPIN_RIGHT to his right, SPIN_LEFT to his left);
        if (COUNT(s) % SPIN_PUFF == 0) {
            SqueakSpawnEffect(s, SQUEAK_FX_LAND, 0, 0, FALSE);
            SqueakSfx(s, 0x178);
        }
        w = s->obj.kirby3->base.x > base->x ? SPIN_RIGHT : s->obj.kirby3->base.x < base->x ? -SPIN_LEFT : 0;
        s->vx = base->flags & 1 ? -w : w;
        if (COUNT(s) < 1) {
            SqueakStop(s);
            s->vx = PANT_SPEED;
            SqueakPlay(s, ANIM_PANT);
            SqueakSetState(s, BODY_PANT);
            COUNT(s) = PANT_TIME;
            return TRUE;
        }
        COUNT(s)--;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_PANT:         // 0x020FEE64: swaying;
        if (COUNT(s) == PANT_BACK) s->vx = -PANT_SPEED;
        if (COUNT(s) == PANT_FORTH) s->vx = PANT_SPEED;
        if (COUNT(s) < 1) {
            SqueakStop(s);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_DRILL_ENTRY:  // 0x020FEDB8;
        BUSY(s) = 8;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakStop(s);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_DRILL_CROUCH);
        return TRUE;
    case BODY_DRILL_CROUCH: // 0x020FECA0: its crouch over, up through everything, a big puff;
        if (s->animEnded) {
            START_Y(s) = base->y;
            SqueakThroughTerrain(s, TRUE);
            s->vy = DRILL_SPEED;
            SqueakPlay(s, ANIM_DRILL_UP);
            SqueakSfx(s, 0x165);
            SqueakSetState(s, BODY_DRILL_UP);
            COUNT(s) = AT_TOP(s) = 0;
            Puff(s, SQUEAK_FX_LAND_BIG, FALSE);
            RequestScreenShake(SHAKE_SHORT, base);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_DRILL_UP:     // 0x020FEB14: held at the screen's top DRILL_TOP frames, then over its target, drilling down slowly;
        if (COUNT(s) > DRILL_TOP - 1) {
            w = Roll();
            w = w < 15 ? -1 : w < 30 ? 0 : w < 45 ? 1 : 2;
            if (w > 1) base->x = s->obj.kirby3->base.x;
            else base->x = level->viewportPosition.x + ((DISPLAY_WIDTH / 2 + w * AIM_SIDE) << 8);
            AIM(s) = DRILL_AIM;
            RequestScreenShake(SHAKE_LAND, base);
            s->vy = -DRILL_AIM_SPEED;
            SqueakPlay(s, ANIM_DRILL_DOWN);
            SqueakSetState(s, BODY_DRILL_AIM);
            return TRUE;
        }
        if (base->y <= level->viewportPosition.y) {
            AT_TOP(s) = TRUE;
            SqueakStop(s);
            base->y = level->viewportPosition.y;
            COUNT(s)++;
        }
        if (AT_TOP(s)) base->y = level->viewportPosition.y;
        MoveFree(s);
        break;
    case BODY_DRILL_AIM:    // 0x020FEAB8;
        if (AIM(s) < 1) {
            s->vy = -DRILL_SPEED;
            SqueakSetState(s, BODY_DRILL_DOWN);
            return TRUE;
        }
        AIM(s)--;
        MoveFree(s);
        break;
    case BODY_DRILL_DOWN:   // 0x020FE9E8: below where it started plus DRILL_ABOVE, the terrain again; landed, facing Kirby, two stars;
        if (base->y > START_Y(s) - (DRILL_ABOVE << 8)) {
            SqueakThroughTerrain(s, FALSE);
            if (SqueakOnGround(s)) {
                FaceKirbyNow(s);
                SqueakSfx(s, 0x14A);
                Puff(s, SQUEAK_FX_LAND_BIG, TRUE);
                RequestScreenShake(SHAKE_LAND, base);
                SqueakSetState(s, BODY_START);
                return TRUE;
            }
            Gravity(s);
            SqueakMove(s);
        } else {
            MoveFree(s);
        }
        break;
    }
    return FALSE;
}

// Landed(); The brain's check: its body on the ground (0x84 under 1) and standing (0x204 0);
static bool32 Landed(struct SqueakObject *s) {
    return SqueakOnGround(s) && BUSY(s) == 0;
}

// Jump(); 0x020FE124, 0x020FDFE8, 0x020FDF68: past 5 jumps in all or 3 of this kind, the attack (each count starting over); else A;
static void Jump(struct SqueakObject *s, s32 *kind, s32 dir, s32 y) {
    bool32 all = JUMPS(s) > 5, these = *kind > 3;

    if (all) JUMPS(s) = 0;
    if (these) *kind = 0;
    if (all || these) {
        JUMPS_ON(s) = 0;
        s->brain = BRAIN_ATTACK;
        return;
    }
    s->padDir = dir;
    if (y != 2) s->padY = y;
    s->padA = 1;
    s->brain = BRAIN_LANDING;
    JUMPS(s)++;
    (*kind)++;
}

// Brain(); Its pad (0x020FE274 and the states it leads to), one state a frame;
static void Brain(struct SqueakObject *s) {
    s32 roll;

    switch (s->brain) {
    case BRAIN_START:       // 0x020FE274;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_TIMER(s) = REST;
        JUMPS(s) = SPINS(s) = DRILLS(s) = JUMPS_ON(s) = 0;
        s->brain = BRAIN_REST;
        break;
    case BRAIN_REST:        // 0x020FE1C0;
        s->padDir = s->padY = 0;
        if (BRAIN_TIMER(s) > 0) {
            BRAIN_TIMER(s)--;
            break;
        }
        roll = Roll();
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = roll < 10 ? BRAIN_REST_AGAIN : roll < 40 ? BRAIN_JUMP : roll < 70 ? BRAIN_JUMP_FORWARD : BRAIN_HOP;
        break;
    case BRAIN_REST_AGAIN:  // 0x020FE1A0;
        BRAIN_TIMER(s) = REST;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_REST;
        break;
    case BRAIN_JUMP:        // 0x020FE124: in place;
        Jump(s, &JUMPS_UP(s), 0, 0);
        break;
    case BRAIN_JUMP_FORWARD:    // 0x020FDFE8: forward (its up and down as they were);
        Jump(s, &JUMPS_FORWARD(s), 1, 2);
        break;
    case BRAIN_HOP:         // 0x020FDF68: down, the hop back;
        Jump(s, &HOPS(s), 1, -1);
        break;
    case BRAIN_LANDING:     // 0x020FE064;
        s->padA = 0;
        if (!Landed(s)) break;
        s->padDir = s->padY = 0;
        roll = Roll();
        SqueakFaceKirby(s, FACE_DISTANCE);
        if (roll < 10) {
            BRAIN_TIMER(s) = REST;
            s->brain = BRAIN_REST_AGAIN;
        } else {
            s->brain = roll < 40 ? BRAIN_JUMP_FORWARD : roll < 70 ? BRAIN_HOP : BRAIN_TWO_JUMPS;
        }
        break;
    case BRAIN_TWO_JUMPS:   // 0x020FDF50;
        JUMPS_ON(s) = 2;
        s->brain = BRAIN_JUMP_ON;
        break;
    case BRAIN_JUMP_ON:     // 0x020FDF28: forward, A;
        s->padDir = 1;
        s->padA = 1;
        s->brain = BRAIN_ATTACK;
        JUMPS_ON(s)--;
        break;
    case BRAIN_ATTACK:      // 0x020FDE1C;
        s->padDir = s->padY = 0;
        s->padA = 0;
        if (!Landed(s)) break;
        if (JUMPS_ON(s) > 0) {
            s->brain = BRAIN_JUMP_ON;
            break;
        }
        roll = Roll();
        if (roll < 50 && SPINS(s) < 3) {
            BRAIN_TIMER(s) = REST;
            s->brain = BRAIN_SPIN;
            SPINS(s)++;
        } else if (++DRILLS(s) < 4) {
            s->brain = BRAIN_DRILL;
            SPINS(s) = 0;
        } else {
            DRILLS(s) = 0;
            BRAIN_TIMER(s) = REST;
            s->brain = BRAIN_SPIN;
        }
        break;
    case BRAIN_SPIN:        // 0x020FDDFC;
        s->padDir = s->padY = 0;
        s->padB = 1;
        s->brain = BRAIN_SPIN_WAIT;
        break;
    case BRAIN_SPIN_WAIT:   // 0x020FDDCC;
        s->padB = 0;
        if (Landed(s)) s->brain = BRAIN_LANDING;
        break;
    case BRAIN_DRILL:       // 0x020FDDAC: B with up;
        s->padDir = 0;
        s->padY = 1;
        s->padB = 1;
        s->brain = BRAIN_DRILL_WAIT;
        break;
    case BRAIN_DRILL_WAIT:  // 0x020FDD74;
        s->padDir = s->padY = 0;
        s->padB = 0;
        if (Landed(s)) s->brain = BRAIN_HOP;
        break;
    }
}

static void GaoGaoMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakMidbossUpdate(s);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sGaoGaoBoxData, HARM(s), &BOX(s));
}

/* GaoGaoStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do): the first time, its intro (untouchable,
 * falling at INTRO_GRAVITY a frame to 2.4, INTRO frames), then its body's start; */
void GaoGaoStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, GaoGaoMain);
    if (s->started) return;
    SqueakPlay(s, ANIM_STAND);
    SqueakStop(s);
    s->ay = -INTRO_GRAVITY;
    COUNT(s) = INTRO;
    s->obj.base.flags |= 0x200;
    SqueakSetState(s, BODY_INTRO);
}

// GaoGaoInit(); Its box (gaogao.rca's standing one), its bounds (KatAM's), facing left, standing;
void GaoGaoInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -23, -34, 29, 0);
    ObjectSetBounds(&s->obj.base, -16, -32, 16, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
    SqueakPlay(s, ANIM_STAND);
}
