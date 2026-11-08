/* KatAM's own enemies with Squeak Squad's extra behaviors. An enemy both games have stays KatAM's own object (its look, moves, speeds,
 * hurt and ability), and the behaviors Squeak Squad gives it that KatAM's lacks are ported onto it as new variants (the template's
 * subtype1), through its spawn: KatAMExpansionSqueak.cs points the spawn table's entry at the Squeak* function here and keeps KatAM's
 * own in gSqueakNatives; each calls KatAM's, then starts its new variant if the template asks for one. KatAM's own variants are left
 * as they are.
 * • Waddle Dee (object 0x00; Squeak Squad's enemy 0x00: body 0x0202A9D0, brain 0x02029760, prmNE01Pl0.kgb). KatAM's variants: 0 walks,
 *   1 turns on a timer, 2 jumps at random, 3 and 4 float down with a parasol (KatAM's own parasol, object 0xA2, as Squeak Squad's:
 *   held while it floats, let go when it lands or is hit, then flying off; 4's kept once landed). Squeak Squad's brains: its 0 (walks,
 *   turning at walls) is KatAM's 0, its 4 (a parasol, then walks: 0x020293C8) KatAM's 3, its 5 (a parasol kept, then stands still:
 *   0x020293F0) KatAM's 4. Its others are new: each walks at KatAM's speed for its subtype2, not turning at walls (as Squeak Squad's
 *   brains don't), and jumps as KatAM's Waddle Dee does (it crouches, two loops of its crouch, as Squeak Squad's does 32 frames, then
 *   leaps 2.5 up, Squeak Squad's speed too, at its walk's speed, turning at walls in the air); KatAM's spawn already faces Kirby, as
 *   Squeak Squad's brains first do:
 *   5, its 1 (0x020295EC, 0x02029540, 0x02029508): it walks back and forth 24 pixels either side of where it started: past that it
 *     turns round (from the next frame, as Squeak Squad's body: logged, a step more the old way) and walks 31 frames before it looks
 *     again;
 *   6, its 2 (0x020294E8, 0x020294B8, 0x020294B0): the first frame Kirby is within 48 pixels across (any height) it jumps, once, and
 *     walks on for good;
 *   7, its 3 (0x02029484, 0x0202944C, 0x0202941C): it walks and jumps 122 frames after it starts, then 122 frames after the frame it
 *     lands, each time. */
#include "global.h"
#include "object.h"
#include "functions.h"
#include "kirby.h"
#include "data.h"
#include "constants/songs.h"

// gSqueakNatives; Filled in by KatAMExpansionSqueak.cs: KatAM's own spawns of the enemies this file adds to;
struct SqueakNatives {
    void *(*waddleDee)(struct ObjectTemplate *, u8);
    void (*waddleDeeStart)(struct Object *);   // Its type's resume after a hit or bump (gUnk_08351648[0].unk10, sub_080A3CF0);
};
const struct SqueakNatives gSqueakNatives = { NULL, NULL };

// Waddle Dee's new variants (subtype1) and anims (its AnimInfos, 0x308's: ObjectSetFunc's index);
enum { WADDLE_DEE_PATROL = 5, WADDLE_DEE_JUMP_AT, WADDLE_DEE_HOP };
enum { WADDLE_DEE_WALK, WADDLE_DEE_CROUCH, WADDLE_DEE_JUMP };
// The brain's phase (unkA6): looking; turning round (variant 5's back, before its turn); back (after it); jumped (variant 6, for good;
// variant 7, till it lands);
enum { PHASE_LOOK, PHASE_TURN, PHASE_BACK, PHASE_JUMPED };

// prmNE01Pl0.kgb, in pixels and frames;
#define PATROL_RANGE 24
#define PATROL_BACK 30
#define JUMP_RANGE 48
#define JUMP_WAIT 120
#define JUMP_SPEED 0x280        // KatAM's Waddle Dee's (WaddleDeeChooseXSpeedAndPlaySfx), Squeak Squad's 0x2800 (20.12);

/* Their fields (none of which KatAM's hits and bumps touch, so its brain goes on after one as Squeak Squad's does): where it started
 * (variant 5) in unkA0, its walk's speed (KatAM's spawn's) in unkA2, the brain's timer in unkA4, its phase in unkA6; whether it has
 * crouched a loop in unk85 (as KatAM's); the state in unk78; */
static void WaddleDeePatrol(struct Object *);
static void WaddleDeePatrolBack(struct Object *);
static void WaddleDeeJumpAt(struct Object *);
static void WaddleDeeWalkOn(struct Object *);
static void WaddleDeeHop(struct Object *);
static void WaddleDeeHopLanded(struct Object *);
static void WaddleDeeCrouch(struct Object *);
static void WaddleDeeAir(struct Object *);
void SqueakWaddleDeeStart(struct Object *);

// WaddleDeeTurn(); Squeak Squad's turn (its pad back): faces and walks the other way;
static void WaddleDeeTurn(struct Object *obj) {
    obj->base.flags ^= 1;
    obj->base.xspeed = -obj->base.xspeed;
}

// WaddleDeeWalkSpeed(); Its walk's speed the way it faces;
static void WaddleDeeWalkSpeed(struct Object *obj) {
    obj->base.xspeed = obj->base.flags & 1 ? -obj->unkA2 : obj->unkA2;
}

// WaddleDeeJump(); Squeak Squad's A: KatAM's Waddle Dee's jump (sub_080A44C0): it stops and crouches;
static void WaddleDeeJump(struct Object *obj) {
    ObjectSetFunc(obj, WADDLE_DEE_CROUCH, WaddleDeeCrouch);
    obj->base.xspeed = 0;
    obj->unk85 = FALSE;
}

// WaddleDeeCrouch(); sub_080A44E4: two loops of its crouch, then it leaps (WaddleDeeChooseXSpeedAndPlaySfx) at its walk's speed;
static void WaddleDeeCrouch(struct Object *obj) {
    if (!(obj->base.flags & 2)) return;
    if (!obj->unk85) {
        obj->base.flags |= 4;
        obj->unk85 = TRUE;
        return;
    }
    ObjectSetFunc(obj, WADDLE_DEE_JUMP, WaddleDeeAir);
    obj->base.yspeed = JUMP_SPEED;
    WaddleDeeWalkSpeed(obj);
    PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
}

// WaddleDeeAir(); sub_080A4484: a wall turns it round; landed, it walks (variant 6 for good, 7 its timer again the next frame);
static void WaddleDeeAir(struct Object *obj) {
    if (obj->base.unk62 & 1) WaddleDeeTurn(obj);
    if (!(obj->base.unk62 & 4)) return;
    if (obj->objTemplate->subtype1 == WADDLE_DEE_HOP)
        ObjectSetFunc(obj, WADDLE_DEE_WALK, WaddleDeeHopLanded);
    else
        ObjectSetFunc(obj, WADDLE_DEE_WALK, WaddleDeeWalkOn);
}

/* WaddleDeePatrol(); 0x02029540: forward, till it's past PATROL_RANGE from where it started the way it's going: then it turns round,
 * from the next frame as Squeak Squad's body does (logged: a step more the old way); */
static void WaddleDeePatrol(struct Object *obj) {
    s16 x = obj->base.x >> 8;

    obj->base.flags |= 4;
    if (obj->base.flags & 1 ? x < obj->unkA0 - PATROL_RANGE : x > obj->unkA0 + PATROL_RANGE) {
        obj->unkA4 = PATROL_BACK;
        obj->unkA6 = PHASE_TURN;
        obj->unk78 = WaddleDeePatrolBack;
    }
}

// WaddleDeePatrolBack(); 0x02029508: turned round (its first frame), forward, PATROL_BACK + 1 frames;
static void WaddleDeePatrolBack(struct Object *obj) {
    obj->base.flags |= 4;
    if (obj->unkA6 == PHASE_TURN) {
        WaddleDeeTurn(obj);
        obj->unkA6 = PHASE_BACK;
    }
    if (--obj->unkA4 < 0) {
        obj->unkA4 = 0;
        obj->unkA6 = PHASE_LOOK;
        obj->unk78 = WaddleDeePatrol;
    }
}

// WaddleDeeJumpAt(); 0x020294B8: forward; Kirby within JUMP_RANGE across (any height), it jumps, once;
static void WaddleDeeJumpAt(struct Object *obj) {
    struct Kirby *kirby = FindTargetKirby(&obj->base);
    s32 dx = (kirby->base.x - obj->base.x) >> 8;

    obj->base.flags |= 4;
    if (dx > -JUMP_RANGE && dx < JUMP_RANGE) {
        obj->unkA6 = PHASE_JUMPED;
        WaddleDeeJump(obj);
    }
}

// WaddleDeeWalkOn(); 0x020294B0: forward for good;
static void WaddleDeeWalkOn(struct Object *obj) {
    obj->base.flags |= 4;
}

// WaddleDeeHop(); 0x0202944C: forward; its timer run out (JUMP_WAIT + 1 frames), it jumps;
static void WaddleDeeHop(struct Object *obj) {
    obj->base.flags |= 4;
    if (obj->unkA4 < 0) {
        obj->unkA6 = PHASE_JUMPED;
        WaddleDeeJump(obj);
        return;
    }
    obj->unkA4--;
}

// WaddleDeeHopLanded(); 0x0202941C, the frame after it lands (no speed up or down): its timer again;
static void WaddleDeeHopLanded(struct Object *obj) {
    obj->base.flags |= 4;
    obj->unkA4 = JUMP_WAIT;
    obj->unkA6 = PHASE_LOOK;
    obj->unk78 = WaddleDeeHop;
}

/* SqueakWaddleDeeStart(); Object 0x00's resume after a hit or bump (and the new variants' start): the new variants' brains go on where
 * they were (a jump's landing missed, variant 7 counts again), at its walk's speed the way it faces; KatAM's own variants, KatAM's; */
void SqueakWaddleDeeStart(struct Object *obj) {
    switch (obj->objTemplate->subtype1) {
    case WADDLE_DEE_PATROL:
        ObjectSetFunc(obj, WADDLE_DEE_WALK, obj->unkA6 == PHASE_LOOK ? WaddleDeePatrol : WaddleDeePatrolBack);
        break;
    case WADDLE_DEE_JUMP_AT:
        ObjectSetFunc(obj, WADDLE_DEE_WALK, obj->unkA6 == PHASE_JUMPED ? WaddleDeeWalkOn : WaddleDeeJumpAt);
        break;
    case WADDLE_DEE_HOP:
        if (obj->unkA6 == PHASE_JUMPED) {
            ObjectSetFunc(obj, WADDLE_DEE_WALK, WaddleDeeHopLanded);
        } else {
            ObjectSetFunc(obj, WADDLE_DEE_WALK, WaddleDeeHop);
        }
        break;
    default:
        gSqueakNatives.waddleDeeStart(obj);
        return;
    }
    WaddleDeeWalkSpeed(obj);
}

/* SqueakCreateWaddleDee(); Object 0x00's spawn: KatAM's Waddle Dee (its spawn walks one with a variant it doesn't know, as its 0, at
 * its subtype2's speed), then the new variants' brains; */
void *SqueakCreateWaddleDee(struct ObjectTemplate *template, u8 playerId) {
    struct Object *obj = gSqueakNatives.waddleDee(template, playerId);

    if (obj->type != OBJ_WADDLE_DEE_1) return obj;
    obj->unkA2 = obj->base.xspeed < 0 ? -obj->base.xspeed : obj->base.xspeed;
    obj->unkA0 = obj->base.x >> 8;
    obj->unkA4 = JUMP_WAIT;
    obj->unkA6 = PHASE_LOOK;
    if (template->subtype1 >= WADDLE_DEE_PATROL && template->subtype1 <= WADDLE_DEE_HOP) SqueakWaddleDeeStart(obj);
    return obj;
}
