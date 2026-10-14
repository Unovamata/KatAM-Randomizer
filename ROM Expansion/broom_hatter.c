/* Broom Hatter; ported from Kirby: Nightmare in Dream Land (USA), object type 0x0B (spawn 0x0808D4E8), as KatAM object type 0x31.
 * Built by build.sh and embedded in KatAMExpansion.cs, which also adds its graphics (from the player's NiDL ROM) and object parameters.
 * NiDL runs it as coroutine scripts (one per state) that set a velocity, a broom dust frame and a body frame, then
 * wait some frames. The scripts are copied here step by step as tables and played back one step at a time;
 * NiDL velocities are 16.16 pixels per frame, KatAM's are 8.8, so 0x10000 there is 0x100 here;
 * NiDL scripts (variant 0):
 * • 0x0808D640 Sweep: faces Kirby (or turns its back to Kirby) and sweeps forward twice.
 * • 0x0808D790 Back & Forth: faces Kirby and sweeps forward and back three times.
 * • 0x0808D964 Stand: sweeps in place forever (used for subtype 1).
 * • 0x0808D3E4 picks the next script at random from {0, 1, 2, 0, 1, 2, 0, 1}, never the same one twice in a row.
 * • 0x0808D364 stops it at walls and ledges; */
#include "object.h"
#include "kirby.h"
#include "random.h"
#include "functions.h"
#include "task.h"

// Body frames 4-7 of NiDL, with the dust (none, 8, 9, 10) on top. Anim variant = (frame - 4) * 4 + dust; 16-17 are hurt frames;
#define ANIM_BROOM_HATTER 939
#define DUST_NONE 0
#define FRAME_HURT 16
#define FRAME_HURT_2 17

enum { SCRIPT_SWEEP, SCRIPT_BACK_AND_FORTH, SCRIPT_STAND };
enum { ACTION_BACK_AND_FORTH, ACTION_SWEEP, ACTION_SWEEP_AWAY };

// Step; One step of a NiDL script: velocity (8.8, forward), dust (0 none, 1-3 frames 8-10), body frame (4-7), frames to wait.
// Backward steps of Back & Forth use NiDL's 0x0800617C, which sets frame | 1 when facing left;
struct Step {
    s16 speed;
    u8 dust;
    u8 frame;
    u8 wait;
    u8 backward;
};

struct BroomHatter {
    struct Object obj;
    const struct Step *steps;
    u8 numSteps;
    u8 loopStart;
    u8 loops;       // Times the steps from loopStart are played; 0 = forever;
    u8 script;
    u8 step;
    u8 wait;
    u8 loop;
    u8 lastAction;
};

const struct AnimInfo gBroomHatterAnims[] = {
    { ANIM_BROOM_HATTER, 0, 0 }, { ANIM_BROOM_HATTER, 1, 0 }, { ANIM_BROOM_HATTER, 2, 0 }, { ANIM_BROOM_HATTER, 3, 0 },
    { ANIM_BROOM_HATTER, 4, 0 }, { ANIM_BROOM_HATTER, 5, 0 }, { ANIM_BROOM_HATTER, 6, 0 }, { ANIM_BROOM_HATTER, 7, 0 },
    { ANIM_BROOM_HATTER, 8, 0 }, { ANIM_BROOM_HATTER, 9, 0 }, { ANIM_BROOM_HATTER, 10, 0 }, { ANIM_BROOM_HATTER, 11, 0 },
    { ANIM_BROOM_HATTER, 12, 0 }, { ANIM_BROOM_HATTER, 13, 0 }, { ANIM_BROOM_HATTER, 14, 0 }, { ANIM_BROOM_HATTER, 15, 0 },
    { ANIM_BROOM_HATTER, 16, 0 }, { ANIM_BROOM_HATTER, 17, 0 },
};

// 0x0808D640; Velocity table 0x08743144: 1.0, 0.5, 0.25, 0.25, 2.0;
static const struct Step sSweep[] = {
    { 0x100, 0, 6, 5, 0 }, { 0x100, 0, 6, 5, 0 }, { 0x80, 0, 5, 5, 0 }, { 0x80, 2, 4, 5, 0 },
    { 0x40, 2, 4, 1, 0 }, { 0x40, 1, 7, 7, 0 }, { 0x40, 3, 6, 6, 0 }, { 0x200, 3, 6, 2, 0 },
};

// 0x0808D790; Velocity table 0x08743158: 1.0, 0.5, 0.5, -1.0, -0.5, -0.5. The first step only plays once;
static const struct Step sBackAndForth[] = {
    { 0x100, 0, 6, 8, 0 },
    { 0x100, 2, 6, 2, 0 }, { 0x100, 2, 5, 6, 0 }, { 0x80, 1, 4, 7, 0 }, { 0x80, 3, 4, 1, 0 }, { 0x80, 3, 7, 7, 0 }, { 0x80, 0, 6, 1, 0 },
    { -0x100, 2, 6, 2, 0 }, { -0x100, 2, 5, 6, 0 }, { -0x80, 1, 4, 7, 1 }, { -0x80, 3, 4, 1, 1 }, { -0x80, 3, 7, 7, 1 }, { -0x80, 0, 6, 1, 1 },
};

// 0x0808D964;
static const struct Step sStand[] = {
    { 0, 0, 6, 8, 0 }, { 0, 0, 6, 8, 0 }, { 0, 0, 5, 8, 0 }, { 0, 2, 4, 8, 0 }, { 0, 1, 7, 8, 0 }, { 0, 3, 6, 8, 0 },
};

// 0x0874313C;
static const u8 sNextActions[] = { 0, 1, 2, 0, 1, 2, 0, 1 };

static void BroomHatterMain(struct Object *);

// FaceKirby(); 0x08063E14: faces the closest Kirby;
static void FaceKirby(struct Object *obj) {
    if (obj->base.x > obj->kirby3->base.x)
        obj->base.flags |= 1;
    else
        obj->base.flags &= ~1;
}

// PlayStep(); Sets the velocity and frame of the current step;
static void PlayStep(struct BroomHatter *bh) {
    const struct Step *step = &bh->steps[bh->step];
    u8 frame = step->frame;

    if (step->backward && (bh->obj.base.flags & 1))
        frame |= 1;
    bh->obj.base.xspeed = (bh->obj.base.flags & 1) ? -step->speed : step->speed;
    bh->obj.unk83 = (frame - 4) * 4 + step->dust;
    bh->wait = step->wait;
}

static void StartScript(struct BroomHatter *bh, u8 script) {
    bh->script = script;
    bh->step = 0;
    bh->loop = 0;
    switch (script) {
    case SCRIPT_SWEEP:
        bh->steps = sSweep;
        bh->numSteps = ARRAY_COUNT(sSweep);
        bh->loopStart = 0;
        bh->loops = 2;
        break;
    case SCRIPT_BACK_AND_FORTH:
        bh->steps = sBackAndForth;
        bh->numSteps = ARRAY_COUNT(sBackAndForth);
        bh->loopStart = 1;
        bh->loops = 3;
        break;
    default:
        bh->steps = sStand;
        bh->numSteps = ARRAY_COUNT(sStand);
        bh->loopStart = 0;
        bh->loops = 0;
        break;
    }
    PlayStep(bh);
}

// ChooseAction(); 0x0808D3E4: next script, never the last one again. Sweeping away turns its back to Kirby;
static void ChooseAction(struct BroomHatter *bh) {
    u8 action;

    do {
        action = sNextActions[RandLessThan(8)];
    } while (action == bh->lastAction);
    bh->lastAction = action;
    FaceKirby(&bh->obj);
    if (action == ACTION_BACK_AND_FORTH) {
        StartScript(bh, SCRIPT_BACK_AND_FORTH);
    } else {
        if (action == ACTION_SWEEP_AWAY)
            bh->obj.base.flags ^= 1;
        StartScript(bh, SCRIPT_SWEEP);
    }
}

// IsFloor(); Anything but an empty tile (gCollisionAttributes bit 0 marks empty tiles);
static bool32 IsFloor(struct Object *obj, s32 x, s32 y) {
    return (gCollisionAttributes[GetCollisionTile(obj->base.unk56, x >> 12, y >> 12)] & 1) == 0;
}

// StopAtLedge(); 0x0808D364: stops when the floor ends ahead (walls already stop it). A slope going down continues one tile lower;
static void StopAtLedge(struct Object *obj) {
    s32 x, y;

    if (!obj->base.xspeed || !(obj->base.unk62 & 4)) return;
    if (obj->base.x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x || obj->base.x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x
        || obj->base.y > gCurLevelInfo[obj->base.unk56].levelMaxPosition.y || obj->base.y < gCurLevelInfo[obj->base.unk56].levelMinPosition.y)
        return;
    x = obj->base.x + obj->base.xspeed;
    y = obj->base.y + (obj->base.unk3F + 9) * 0x100;
    if (!IsFloor(obj, x, y) && !IsFloor(obj, x, y + 0x1000))
        obj->base.xspeed = 0;
}

static void BroomHatterMain(struct Object *obj) {
    struct BroomHatter *bh = (struct BroomHatter *)obj;

    obj->base.flags |= 4;
    if (bh->wait && --bh->wait) {
        StopAtLedge(obj);
        return;
    }
    if (++bh->step >= bh->numSteps) {
        bh->step = bh->loopStart;
        if (bh->loops && ++bh->loop >= bh->loops) {
            obj->base.xspeed = 0;
            ChooseAction(bh);
            StopAtLedge(obj);
            return;
        }
    }
    PlayStep(bh);
    StopAtLedge(obj);
}

// BroomHatterStart(); Behavior start, also called by the engine after the Broom Hatter recovers from a hit;
void BroomHatterStart(struct Object *obj) {
    struct BroomHatter *bh = (struct BroomHatter *)obj;

    ObjectSetFunc(obj, 0, BroomHatterMain);
    obj->kirbyAbility = 0;
    FaceKirby(obj);
    bh->lastAction = ACTION_SWEEP;
    StartScript(bh, obj->objTemplate->subtype1 == 1 ? SCRIPT_STAND : SCRIPT_SWEEP);
}

void *CreateBroomHatter(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct BroomHatter), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct Object *obj = TaskGetStructPtr(task);

    InitObject(obj, template, playerId);
    sub_0803E2B0(&obj->base, -5, -4, 5, 7);
    ObjectSetBounds(&obj->base, -6, -5, 6, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    ObjectInitSprite(obj);
    BroomHatterStart(obj);
    return obj;
}
