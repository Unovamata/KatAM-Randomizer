/* Acchi; ported from Kirby: Squeak Squad (USA), enemy 0x2A (its body 0x0206B1F0, vtable 0x0216C904; brain 0x02069D6C; its fire
 * 0x0206BCA0, spat by 0x0206BD60), as SQUEAK_ACCHI and SQUEAK_ACCHI_FIRE. Values from param/prmCE27Ml0.kgb (movement and the fire's;
 * MlG is the same), prmCE27Pl0.kgb (brain) and prmCE27Sl0.kgb (HP 30); speeds 20.12 a frame, Y up; its boxes from enemy/acchi.rca (its
 * composites' the .rca's 9-15, its animations' 8-15 the .rca's 1-8); it moves as acchi.mca's composite animations. A fire lizard lying
 * flat: it spits fire at Kirby, low or in a high lob, the fire burning on where it lands; one kind creeps about and backs off after
 * each shot. Swallowed, it gives Fire (as on the DS).
 * • Brain (the spawn's first parameter byte, KatAM's template unk12):
 *   0, 3, 4 and any above 5 (0x02069CA0): it faces Kirby (past 1 pixel); with Kirby within 144 pixels across and 96 up or down
 *     (0x02069C5C) it presses B one frame: within 96 across and 48 up or down a low shot (0x02069BCC, 0x02069BB4), else a lob, its pad
 *     up too (0x02069B90, 0x02069B6C); then it rests 60 frames (0x02069B3C: after a low shot 59, the timer not set back; the first
 *     time at once, the timer never set when it's made: the DS's leftover memory, a big number in the tests) and looks again; Kirby
 *     farther, it waits 61 frames (0x02069C1C) and looks again;
 *   1 (0x02069B1C): it faces Kirby and (0x02069A90) waits 30 frames (40%: 0x02069A58), 60 (20%: 0x02069A20) or none; then
 *     (0x020699CC) with Kirby within 144 pixels across and 144 up or down, at its animation's next loop it faces him and at the one
 *     after (0x02069934) presses B (a lob if he's not within 96 and 48), lets go (0x02069910), faces him again and at the next loop
 *     (0x020698E4) pushes its pad down one frame (0x020698CC, 0x020698B8: it backs off) and starts over; Kirby farther, it creeps
 *     forward 120 frames (0x02069970: its pad forward, back at a wall) and starts over;
 *   2 and 5 (0x02069898): it faces Kirby, then stands (0x02069890).
 *   Facing Kirby turns its body, and the turn makes its pad's "turn round" forward (as on the DS): it creeps till the brain sets it.
 * • Body (gravity 0.17 a frame, falls no faster than 2.4): it stands (composite 0, 0x0206B000, 0x0206AEF0); B draws breath (composite
 *   4, 60 frames: 0x0206A208, 0x0206A0A8; its pad up then or meanwhile: the lob) and spits (composite 5, or 6 for the lob: 0x02069F60):
 *   its animation over, three frames on, the fire, 20 pixels ahead, 12 up; then it stands. Forward, it creeps (composite 1:
 *   0x0206AE20, 0x0206ACA8): still 52 frames, then a lunge at 3.0 slowing 0.1 a frame, till its animation's next loop (66 frames);
 *   down, it backs off the same way (composite 2: 0x0206ABD0, 0x0206AA24; 56 frames, then 3.0 back), and down again after letting go of
 *   it lunges back at 6.0 (0x0206A768, 0x0206A61C). Its brains never press A or its +0x44, which jump (composite 3, 2.8 up: 0x0206A4F0,
 *   0x0206A488) or lunge forward at 6.0 (0x0206A954, 0x0206A808); off the ground it falls (composite 1: 0x0206A384, 0x0206A2EC; forward
 *   keeps it going at 0.8: 0x0206B098).
 * • Its fire (0x0206B948 low: 2.0 forward, 1.2 up; 0x0206BA40 the lob: 1.0 forward, 3.7 up; falling 0.17 a frame, no faster than 2.4;
 *   anim 11): it flies through the terrain (0x0206B84C) till its middle is in it, its box (3 pixels either side) meets the floor or a
 *   ceiling: there it lands (0x0206B780: anim 15, still) and burns (0x0206B5D8, 0x0206B568: anim 13 once anim 15 is over) 121 frames,
 *   then goes out (0x0206B4D4, 0x0206B4BC: anim 14, harmless) and is gone; landed with its box (5 pixels either side, 1 to 10 up) in
 *   the terrain, it goes out at once; out of the stage, it's gone. Its first frame Squeak Squad runs it twice (as recorded: 4 pixels
 *   across). Its touch burns (KatAM's fire, unk68 0x100, as Batafire's fireballs). Kirby's body doesn't touch it (it has no reaction
 *   to his body, as recorded) nor his inhale take it (0x0206BB38 ignores kinds 3-5).
 * • Boxes (acchi.rca): its body's hit box (19 pixels behind to 13 ahead, 1 to 23 up; 28 up jumping), its kind 0 touch box in it; the
 *   fire's (kind 0) 5 pixels round its middle flying, 3 either side and 6 to 12 up burning.
 * Its sound: 0x14C the spit (0x0206A000). Not ported: the other mode (the brain's +8 not 3: A in the air), its contact damages (Squeak Squad's 8
 * the fire: KatAM's 1), the fire going out under Squeak Squad's attacks with flag 0x400 (0x0206BB38: KatAM's attacks beat it as theirs
 * do). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

// Composite animations (acchi.mca) and the fire's animations (acchi.cea);
enum { MCA_STAND, MCA_CREEP, MCA_BACK, MCA_JUMP, MCA_CHARGE, MCA_SPIT, MCA_LOB };
enum { ANIM_FIRE_FLY = 11, ANIM_FIRE_BURN = 13, ANIM_FIRE_OUT = 14, ANIM_FIRE_LAND = 15 };

// prmCE27Ml0.kgb;
#define JUMP_SPEED 0x2CCD
#define GRAVITY 0x2B9
#define FIRE_X 0x14000                  // The fire's spot: 20 pixels ahead, 12 up;
#define FIRE_Y 0xC000
#define FIRE_FRAME 3                    // The spit's frame (once its animation's over);
#define LOB_SPEED_X 0x1000
#define LOB_SPEED_Y 0x3B34
#define FIRE_SPEED_X 0x2000
#define FIRE_SPEED_Y 0x1334
#define AIR_SPEED 0xCCD
#define AIR_BRAKE 0x400
#define CREEP_SPEED 0x3000
#define CREEP_BRAKE 0x400
#define LUNGE_SPEED 0x6000
#define LUNGE_BRAKE 0x400
#define CREEP_WAIT 52
#define BACK_WAIT 56
// prmCE27Pl0.kgb;
#define NEAR_X 0x90000
#define NEAR_Y 0x60000
#define CLOSE_X 0x60000
#define CLOSE_Y 0x30000
#define LOOK_TIME 60
#define ROAM_NEAR 0x90000
#define AIM_X 0x60000
#define AIM_Y 0x30000
#define SHORT_CHANCE 40
#define LONG_CHANCE 20
#define SHORT_WAIT 30
#define LONG_WAIT 60
#define CREEP_TIME 120
// The code's;
#define MAX_FALL -0x2666
#define FACE_DISTANCE 0x1000
#define REST_TIME 60
#define LEFTOVER REST_TIME              // (the rest's timer as made: anything not under REST_TIME ends the first rest at once);
#define BURN_TIME 120

enum {
    BODY_START, BODY_STAND, BODY_FIRE_ENTRY, BODY_CHARGE, BODY_SPIT, BODY_FALL_ENTRY, BODY_FALL, BODY_JUMP_ENTRY, BODY_RISE,
    BODY_CREEP_ENTRY, BODY_CREEP, BODY_BACK_ENTRY, BODY_BACK, BODY_LUNGE_ENTRY, BODY_LUNGE, BODY_BACK_LUNGE_ENTRY,
    BODY_BACK_LUNGE,
};
enum {
    BRAIN_FACE, BRAIN_LOOK, BRAIN_WAIT, BRAIN_NEAR, BRAIN_SHOT, BRAIN_SHOT_LET_GO, BRAIN_LOB, BRAIN_LOB_LET_GO, BRAIN_REST,
    BRAIN_ROAM_FACE, BRAIN_PICK, BRAIN_WAIT_SHORT, BRAIN_WAIT_LONG, BRAIN_CHECK, BRAIN_CREEP, BRAIN_AIM, BRAIN_LET_GO,
    BRAIN_TURN_BACK, BRAIN_BACK, BRAIN_BACK_DONE, BRAIN_STILL_FACE, BRAIN_STILL,
};
enum { FIRE_BORN, FIRE_START, FIRE_ARC, FIRE_LAND, FIRE_BURN, FIRE_BURNING, FIRE_OUT, FIRE_GOING_OUT };

#define KIND(s) ((s)->vars[0])          // 0x1F8: the spit's kind (1: the lob);
#define COUNT(s) ((s)->vars[1])         // 0x1FC: the spit's frames since its animation's end;
#define STEP(s) ((s)->vars[2])          // 0x200: the creep's frames;
#define LET_GO(s) ((s)->vars[3])        // 0x204: backing off, its pad let go of down;
#define LAST_X(s) ((s)->vars[4])        // 0x208: its spot across the last frame in the air;
#define WAIT_FOR(s) ((s)->vars[5])      // 0x20C: the creep's still frames;
#define WAIT(s) ((s)->vars[6])          // The brain's 0x98;
#define TIME(s) ((s)->vars[7])          // The brain's 0x9C;
#define TIMER(s) ((s)->vars[0])         // The fire's 0x12C;
#define REST_X(s) ((s)->vars[1])        // The fire's position past KatAM's units (Squeak Squad's last 4 bits: across, up);
#define REST_Y(s) ((s)->vars[2])

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// Gravity(); 0x02069F08: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

/* AirControl(); 0x0206B098: still across since the last frame, it stops across; back turns it round; forward, slower than AIR_SPEED
 * either way it goes at AIR_SPEED (no cap), going back it brakes; */
static void AirControl(struct SqueakObject *s) {
    s->ax = s->fx = 0;
    if (LAST_X(s) == s->obj.base.x) s->vx = 0;
    LAST_X(s) = s->obj.base.x;
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        if (s->vx > -AIR_SPEED && s->vx < AIR_SPEED) {
            s->vx = AIR_SPEED;
            s->maxVx = 0;
        }
        if (s->vx < 0) s->ax = AIR_BRAKE;
    }
}

// Buttons(); 0x02069E80, 0x02069EC4: B spits, A jumps;
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_FIRE_ENTRY);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// Landed(); Its animation's loop over: it stands (or falls);
static bool32 Landed(struct SqueakObject *s) {
    STEP(s) = 0;
    SqueakSetState(s, SqueakOnGround(s) ? BODY_START : BODY_FALL_ENTRY);
    return TRUE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x0206B000 (vtable +0x3C);
        SqueakPlayComposite(s, MCA_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x0206AEF0: B, A, its brain's +0x44 (never), forward, down, back turns it;
        if (Buttons(s)) return TRUE;
        if (s->padDir > 0) {
            SqueakSetState(s, BODY_CREEP_ENTRY);
            return TRUE;
        }
        if (s->padY < 0) {
            SqueakSetState(s, BODY_BACK_ENTRY);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FIRE_ENTRY:   // 0x0206A208: its pad up, the lob;
        SqueakPlayComposite(s, MCA_CHARGE);
        SqueakStop(s);
        KIND(s) = s->padY < 1 ? 0 : 1;
        SqueakSetState(s, BODY_CHARGE);
        return TRUE;
    case BODY_CHARGE:       // 0x0206A0A8: its pad up meanwhile, the lob too;
        if (KIND(s) != 1) KIND(s) = s->padY < 1 ? 0 : 1;
        if (s->animEnded) {
            COUNT(s) = 0;
            SqueakPlayComposite(s, KIND(s) == 1 ? MCA_LOB : MCA_SPIT);
            SqueakSetState(s, BODY_SPIT);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SPIT:         // 0x02069F60: still till its animation's over, then FIRE_FRAME frames on, the fire (0x0206BD60);
        if (COUNT(s) == FIRE_FRAME) {
            COUNT(s) = FIRE_FRAME + 1;
            SqueakSpawn(s, SQUEAK_ACCHI_FIRE, KIND(s), FIRE_X, FIRE_Y);
            SqueakSfx(s, 0x14C);
            if (s->animEnded) {
                SqueakSetState(s, SqueakOnGround(s) ? BODY_START : BODY_FALL_ENTRY);
                return TRUE;
            }
        }
        if (s->animEnded) {
            Gravity(s);
            SqueakMove(s);
            COUNT(s)++;
        }
        break;
    case BODY_FALL_ENTRY:   // 0x0206A384: its speeds as they were;
        SqueakPlayComposite(s, MCA_CREEP);
        LAST_X(s) = 0;
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x0206A2EC;
        AirControl(s);
        Gravity(s);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    case BODY_JUMP_ENTRY:   // 0x0206A4F0;
        SqueakPlayComposite(s, MCA_JUMP);
        LAST_X(s) = 0;
        s->vy = s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_RISE);
        return TRUE;
    case BODY_RISE:         // 0x0206A488;
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_CREEP_ENTRY:  // 0x0206AE20;
        SqueakPlayComposite(s, MCA_CREEP);
        SqueakStop(s);
        STEP(s) = 0;
        WAIT_FOR(s) = CREEP_WAIT;
        SqueakSetState(s, BODY_CREEP);
        return TRUE;
    case BODY_CREEP:        // 0x0206ACA8: still WAIT_FOR frames, then a lunge forward, slowing; its animation's loop over, it stands;
        if (Buttons(s)) return TRUE;
        if (s->animEnded) return Landed(s);
        if (STEP(s) == WAIT_FOR(s)) {
            s->vx = CREEP_SPEED;
            s->ax = -CREEP_BRAKE;
        }
        if (s->vx < 1) s->vx = s->ax = 0;
        Gravity(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        STEP(s)++;
        break;
    case BODY_BACK_ENTRY:   // 0x0206ABD0;
        SqueakPlayComposite(s, MCA_BACK);
        SqueakStop(s);
        STEP(s) = 0;
        LET_GO(s) = 0;
        WAIT_FOR(s) = BACK_WAIT;
        SqueakSetState(s, BODY_BACK);
        return TRUE;
    case BODY_BACK:         // 0x0206AA24: the same backwards; its pad down again once let go of, the lunge back (its turns twice: none);
        if (Buttons(s)) return TRUE;
        if (s->animEnded) return Landed(s);
        if (s->padY >= 0) LET_GO(s) = 1;
        if (LET_GO(s) == 1 && s->padY < 0) {
            SqueakSetState(s, BODY_BACK_LUNGE_ENTRY);
            return TRUE;
        }
        if (STEP(s) == WAIT_FOR(s)) {
            s->vx = -CREEP_SPEED;
            s->ax = CREEP_BRAKE;
        }
        if (s->vx >= 0) s->vx = s->ax = 0;
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        STEP(s)++;
        break;
    case BODY_LUNGE_ENTRY:  // 0x0206A954;
        SqueakPlayComposite(s, MCA_CREEP);
        SqueakStop(s);
        STEP(s) = 0;
        WAIT_FOR(s) = CREEP_WAIT;
        SqueakSetState(s, BODY_LUNGE);
        return TRUE;
    case BODY_LUNGE:        // 0x0206A808;
        if (Buttons(s)) return TRUE;
        if (s->animEnded) return Landed(s);
        if (STEP(s) == WAIT_FOR(s)) {
            s->vx = LUNGE_SPEED;
            s->ax = -LUNGE_BRAKE;
        }
        if (s->vx < 1) s->vx = s->ax = 0;
        Gravity(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        STEP(s)++;
        break;
    case BODY_BACK_LUNGE_ENTRY: // 0x0206A768: its still frames as backing off set them;
        SqueakPlayComposite(s, MCA_BACK);
        SqueakStop(s);
        STEP(s) = 0;
        SqueakSetState(s, BODY_BACK_LUNGE);
        return TRUE;
    case BODY_BACK_LUNGE:   // 0x0206A61C;
        if (Buttons(s)) return TRUE;
        if (s->animEnded) return Landed(s);
        if (STEP(s) == WAIT_FOR(s)) {
            s->vx = -LUNGE_SPEED;
            s->ax = LUNGE_BRAKE;
        }
        if (s->vx >= 0) s->vx = s->ax = 0;
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        STEP(s)++;
        break;
    }
    return FALSE;
}

// Press(); The brain's 0x02069870 and 0x02069868: B (its +0x3C and +0x20);
static void Press(struct SqueakObject *s, bool8 b) {
    s->padB = b;
}

// Brain(); Its pad (0x02069CA0, 0x02069B1C, 0x02069898 and the states they lead to), one state a frame;
static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_FACE:        // 0x02069CA0;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LOOK;
        break;
    case BRAIN_LOOK:        // 0x02069C5C;
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) {
            s->brain = BRAIN_NEAR;
        } else {
            TIME(s) = 0;
            s->brain = BRAIN_WAIT;
        }
        s->padDir = 0;
        break;
    case BRAIN_WAIT:        // 0x02069C1C;
        if (TIME(s) > LOOK_TIME) {
            TIME(s) = 0;
            s->brain = BRAIN_FACE;
        }
        s->padDir = 0;
        TIME(s)++;
        break;
    case BRAIN_NEAR:        // 0x02069BE4;
        s->brain = SqueakKirbyWithin(s, CLOSE_X, CLOSE_Y) ? BRAIN_SHOT : BRAIN_LOB;
        break;
    case BRAIN_SHOT:        // 0x02069BCC;
        Press(s, TRUE);
        s->brain = BRAIN_SHOT_LET_GO;
        break;
    case BRAIN_SHOT_LET_GO: // 0x02069BB4;
        Press(s, FALSE);
        s->brain = BRAIN_REST;
        break;
    case BRAIN_LOB:         // 0x02069B90;
        s->padY = 1;
        Press(s, TRUE);
        s->brain = BRAIN_LOB_LET_GO;
        WAIT(s) = 0;
        break;
    case BRAIN_LOB_LET_GO:  // 0x02069B6C;
        s->padY = 0;
        Press(s, FALSE);
        s->brain = BRAIN_REST;
        WAIT(s) = 0;
        break;
    case BRAIN_REST:        // 0x02069B3C: (counting on after it's done);
        if (WAIT(s) >= REST_TIME) {
            WAIT(s) = 0;
            s->brain = BRAIN_FACE;
        }
        WAIT(s)++;
        break;
    case BRAIN_ROAM_FACE:   // 0x02069B1C;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_PICK: {      // 0x02069A90;
        u32 r = Random();

        s->padDir = 0;
        WAIT(s) = 0;
        if (r < SHORT_CHANCE) s->brain = BRAIN_WAIT_SHORT;
        else if (r < SHORT_CHANCE + LONG_CHANCE) s->brain = BRAIN_WAIT_LONG;
        else s->brain = BRAIN_CHECK;
        break;
    }
    case BRAIN_WAIT_SHORT:  // 0x02069A58;
    case BRAIN_WAIT_LONG:   // 0x02069A20;
        if (WAIT(s) >= (s->brain == BRAIN_WAIT_SHORT ? SHORT_WAIT : LONG_WAIT)) {
            WAIT(s) = 0;
            s->brain = BRAIN_CHECK;
        } else {
            WAIT(s)++;
        }
        break;
    case BRAIN_CHECK:       // 0x020699CC: Kirby near, at its animation's next loop it faces him; else it creeps;
        if (!SqueakKirbyWithin(s, ROAM_NEAR, ROAM_NEAR)) {
            s->brain = BRAIN_CREEP;
            TIME(s) = 0;
        } else if (s->animEnded) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_AIM;
        }
        break;
    case BRAIN_CREEP:       // 0x02069970;
        if (TIME(s) >= CREEP_TIME) {
            TIME(s) = 0;
            s->padDir = 0;
            s->brain = BRAIN_PICK;
            break;
        }
        s->padDir = 1;
        if (SqueakWallAhead(s)) s->padDir = -1;
        TIME(s)++;
        break;
    case BRAIN_AIM:         // 0x02069934: at its animation's next loop, B (Kirby not within AIM_X and AIM_Y: up too, the lob);
        if (s->animEnded) {
            Press(s, TRUE);
            if (!SqueakKirbyWithin(s, AIM_X, AIM_Y)) s->padY = 1;
            s->brain = BRAIN_LET_GO;
        }
        break;
    case BRAIN_LET_GO:      // 0x02069910;
        s->padDir = 0;
        s->padY = 0;
        Press(s, FALSE);
        s->brain = BRAIN_TURN_BACK;
        break;
    case BRAIN_TURN_BACK:   // 0x020698E4;
        SqueakFaceKirby(s, FACE_DISTANCE);
        if (s->animEnded) s->brain = BRAIN_BACK;
        break;
    case BRAIN_BACK:        // 0x020698CC;
        s->padY = -1;
        s->brain = BRAIN_BACK_DONE;
        break;
    case BRAIN_BACK_DONE:   // 0x020698B8;
        s->padY = 0;
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_STILL_FACE:  // 0x02069898;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:       // 0x02069890;
        s->padDir = 0;
        break;
    }
}

static void AcchiMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    // (its hit box, acchi.rca's 102: higher in the jump's composite);
    sub_0803E2B0(&obj->base, -19, s->composite == MCA_JUMP ? -28 : -23, 13, -1);
}

void AcchiStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, AcchiMain);
    SqueakThroughTerrain(s, FALSE);
    if (!s->started) {
        WAIT(s) = LEFTOVER;
        switch (s->variant) {
        case 1:
            s->brain = BRAIN_ROAM_FACE;
            break;
        case 2:
        case 5:
            s->brain = BRAIN_STILL_FACE;
            break;
        default:
            s->brain = BRAIN_FACE;
            break;
        }
    }
    SqueakSetState(s, BODY_START);
}

/* AcchiInit(); Its box (acchi.rca's hit box, 19 pixels behind to 13 ahead, 1 to 23 up; its body box 6 either side, 1 to 15 up: here
 * from its feet, KatAM's bounds), facing left (0x0206B1F0); */
void AcchiInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -19, -23, 13, -1);
    ObjectSetBounds(&s->obj.base, -6, -15, 6, 0);
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}

/* Its fire (0x0206BCA0: kind 1 the lob, any other low; its own move through the terrain, the stage's edges not holding it, Squeak
 * Squad's flags 0x200 and 0x20000; its spot its middle, KatAM's bounds its terrain box, acchi.rca's 202); */

/* Drift(); 0x02013D18 without the terrain: its acceleration, then its move, to Squeak Squad's precision (the 20.12 units KatAM's 24.8
 * can't hold carried over: its arc lands where and when the DS's does, a lob's fall rounded down a frame early otherwise); */
static void Drift(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;
    s32 dx, up;

    s->vx += s->ax;
    s->vy += s->ay;
    dx = (b->flags & 1 ? -s->vx : s->vx) + REST_X(s);
    up = s->vy + REST_Y(s);
    b->unk48 = b->x;
    b->unk4C = b->y;
    b->x += dx >> 4;
    b->y -= up >> 4;
    REST_X(s) = dx & 15;
    REST_Y(s) = up & 15;
    b->xspeed = b->yspeed = 0;
}

// Solid(); The terrain at (dx, dy) pixels from the fire's spot (x ahead, y down);
static bool32 Solid(struct SqueakObject *s, s32 dx, s32 dy) {
    return NidlIsFloor(&s->obj, s->obj.base.x + (s->obj.base.flags & 1 ? -dx : dx) * 0x100, s->obj.base.y + dy * 0x100);
}

/* BoxInTerrain(); The stage's vtable +0x1C on its burning terrain box (acchi.rca's 202 for anim 15: 5 pixels either side, 1 to 10 up):
 * any of it solid (its corners, edges' middles and middle, 5 pixels apart at most: no 8-pixel tile slips between them); */
static bool32 BoxInTerrain(struct SqueakObject *s) {
    static const s8 sX[] = { -5, 0, 5 }, sY[] = { -10, -5, -1 };
    u8 i, j;

    for (i = 0; i < ARRAY_COUNT(sX); i++)
        for (j = 0; j < ARRAY_COUNT(sY); j++)
            if (Solid(s, sX[i], sY[j])) return TRUE;
    return FALSE;
}

static bool32 Fire(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;

    switch (s->state) {
    case FIRE_BORN:         // (spat this frame: Squeak Squad's runs it from the next, KatAM's this one);
        SqueakSetState(s, FIRE_START);
        break;
    case FIRE_START:        // 0x0206B948 (low), 0x0206BA40 (the lob);
        b->flags &= ~0x400;
        SqueakPlay(s, ANIM_FIRE_FLY);
        if (s->variant == 1) {
            s->vx = LOB_SPEED_X;
            s->vy = LOB_SPEED_Y;
        } else {
            s->vx = FIRE_SPEED_X;
            s->vy = FIRE_SPEED_Y;
        }
        s->ay = -GRAVITY;
        SqueakSetState(s, FIRE_ARC);
        return TRUE;
    case FIRE_ARC:          // 0x0206B84C: its middle in the terrain, the floor under its box or a ceiling over it, it lands;
        if (s->vy < MAX_FALL + 1) {
            s->vy = MAX_FALL;
            s->ay = 0;
        }
        Drift(s);
        if (Solid(s, 0, 0)) {
            SqueakSetState(s, FIRE_LAND);
            return TRUE;
        }
        if (!SqueakTouches(s, SQUEAK_DOWN, 0, FALSE) && !SqueakTouches(s, SQUEAK_UP, 0, FALSE)) {
            if (!SqueakInLevel(s)) b->flags |= 0x1000;
            break;
        }
        s->vx = s->vy = 0;
        SqueakSetState(s, FIRE_LAND);
        return TRUE;
    case FIRE_LAND:         // 0x0206B780;
        SqueakPlay(s, ANIM_FIRE_LAND);
        s->vx = s->vy = s->ax = s->ay = 0;
        SqueakSetState(s, FIRE_BURN);
        TIMER(s) = BURN_TIME;
        return TRUE;
    case FIRE_BURN:         // 0x0206B5D8: its box in the terrain, it goes out; anim 15 over, anim 13;
        Drift(s);
        if (BoxInTerrain(s)) {
            SqueakSetState(s, FIRE_OUT);
            return TRUE;
        }
        if (s->animEnded) {
            SqueakPlay(s, ANIM_FIRE_BURN);
            SqueakSetState(s, FIRE_BURNING);
            return TRUE;
        }
        if (!SqueakInLevel(s)) b->flags |= 0x1000;
        TIMER(s)--;
        break;
    case FIRE_BURNING:      // 0x0206B568: its time over, it goes out;
        Drift(s);
        if (TIMER(s)-- < 0) {
            SqueakSetState(s, FIRE_OUT);
            return TRUE;
        }
        if (!SqueakInLevel(s)) b->flags |= 0x1000;
        break;
    case FIRE_OUT:          // 0x0206B4D4: harmless now;
        SqueakPlay(s, ANIM_FIRE_OUT);
        b->unk68 = 0;
        b->flags |= 0x200;
        SqueakSetState(s, FIRE_GOING_OUT);
        return TRUE;
    case FIRE_GOING_OUT:    // 0x0206B4BC;
        if (s->animEnded) b->flags |= 0x1000;
        break;
    }
    return FALSE;
}

// AcchiFireMain(); Its first frame Squeak Squad runs it twice (as recorded: 4 pixels across, 1.03 and 0.86 up); its box as it burns;
static void AcchiFireMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    u8 runs = s->state == FIRE_START ? 2 : 1;

    while (runs--) {
        SqueakRun(s, Fire);
        SqueakFinishFrame(s);
        SqueakAnimate(s);
    }
    if (s->anim == ANIM_FIRE_LAND || s->anim == ANIM_FIRE_BURN) sub_0803E2B0(&obj->base, -3, -12, 3, -6);
    else sub_0803E2B0(&obj->base, -5, -5, 5, 5);
}

/* AcchiFireStart(); Unseen till its start (KatAM's 0x400: Squeak Squad shows it from then); a hit it lives through (KatAM's) has it go
 * on as it was; */
void AcchiFireStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, AcchiFireMain);
    SqueakThroughTerrain(s, TRUE);
    if (s->started) return;
    obj->base.flags |= 0x400;
    SqueakSetState(s, FIRE_BORN);
}

/* AcchiFireInit(); Its box (acchi.rca's flying one: 5 pixels round its middle; its terrain box 3), its touch fire (KatAM's 0x100, as
 * Batafire's fireballs'); Kirby's body doesn't touch it nor his inhale take it; */
void AcchiFireInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -5, -5, 5, 5);
    ObjectSetBounds(&s->obj.base, -3, -3, 3, 3);
    s->obj.base.unk68 |= 0x100;
    s->obj.base.unk5C |= 0xA0;
}
