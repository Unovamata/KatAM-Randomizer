/* Broom Hatter; ported from Kirby: Nightmare in Dream Land (USA), object type 0x0B (spawn 0x0808D4E8), as NiDL enemy
 * NIDL_BROOM_HATTER (see nidl.h). Variants (0x08743188): 0 (0x0808D558); 1 (0x0808DA00), the same with its own Sweep timing
 * (0x0808DAE8), moving only once it has landed (obj+0x30, set by its landing handler 0x0808D304); 2 (0x0808DF58) sweeps in place
 * forever (0x0808DFC4, the same steps as Stand).
 * NiDL runs it as coroutine scripts (one per state) that set a velocity, a broom dust frame and a body frame, then
 * wait some frames. The scripts are copied here step by step as tables and played back one step at a time;
 * NiDL velocities are 16.16 pixels per frame, KatAM's are 8.8, so 0x10000 there is 0x100 here;
 * NiDL scripts (variant 0):
 * • 0x0808D640 Sweep: faces Kirby (or turns its back to Kirby) and sweeps forward twice.
 * • 0x0808D790 Back & Forth: faces Kirby and sweeps forward and back three times.
 * • 0x0808D964 Stand: sweeps in place forever (used for subtype 1).
 * • 0x0808D3E4 picks the next script at random from {0, 1, 2, 0, 1, 2, 0, 1}, never the same one twice in a row.
 * • 0x0808D364 stops it at walls and ledges; */
#include "../nidl.h"

// Frames: NiDL's body frames 4-7, with the dust (none, 8, 9, 10) on top: (body - 4) * 4 + dust. Its hurt frame is NiDL's 3;

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
    struct NidlObject n;
    const struct Step *steps;
    u8 numSteps;
    u8 loopStart;
    u8 loops;       // Times the steps from loopStart are played; 0 = forever;
    u8 script;
    u8 step;
    u8 wait;
    u8 loop;
    u8 lastAction;
    bool8 landed;   // Variant 1 moves only once it has landed;
};

// 0x0808D640; Velocity table 0x08743144: 1.0, 0.5, 0.25, 0.25, 2.0;
static const struct Step sSweep[] = {
    { 0x100, 0, 6, 5, 0 }, { 0x100, 0, 6, 5, 0 }, { 0x80, 0, 5, 5, 0 }, { 0x80, 2, 4, 5, 0 },
    { 0x40, 2, 4, 1, 0 }, { 0x40, 1, 7, 7, 0 }, { 0x40, 3, 6, 6, 0 }, { 0x200, 3, 6, 2, 0 },
};

// 0x0808DAE8; Variant 1's Sweep: the same speeds, other timing;
static const struct Step sSweep1[] = {
    { 0x100, 0, 6, 7, 0 }, { 0x100, 0, 6, 3, 0 }, { 0x80, 0, 6, 4, 0 }, { 0x80, 0, 5, 6, 0 }, { 0x40, 2, 4, 6, 0 },
    { 0x40, 1, 7, 2, 0 }, { 0x40, 1, 7, 4, 0 }, { 0x40, 3, 6, 4, 0 }, { 0x200, 3, 6, 2, 0 },
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

// PlayStep(); Sets the velocity and frame of the current step;
static void PlayStep(struct BroomHatter *bh) {
    const struct Step *step = &bh->steps[bh->step];
    u8 frame = step->frame;

    if (step->backward && (bh->n.obj.base.flags & 1))
        frame |= 1;
    if (bh->n.variant != 1 || bh->landed)
        bh->n.obj.base.xspeed = (bh->n.obj.base.flags & 1) ? -step->speed : step->speed;
    NidlSetFrame(&bh->n, (frame - 4) * 4 + step->dust);
    bh->wait = step->wait;
}

static void StartScript(struct BroomHatter *bh, u8 script) {
    bh->script = script;
    bh->step = 0;
    bh->loop = 0;
    switch (script) {
    case SCRIPT_SWEEP:
        bh->steps = bh->n.variant == 1 ? sSweep1 : sSweep;
        bh->numSteps = bh->n.variant == 1 ? ARRAY_COUNT(sSweep1) : ARRAY_COUNT(sSweep);
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
    NidlFaceKirby(&bh->n.obj);
    if (action == ACTION_BACK_AND_FORTH) {
        StartScript(bh, SCRIPT_BACK_AND_FORTH);
    } else {
        if (action == ACTION_SWEEP_AWAY)
            bh->n.obj.base.flags ^= 1;
        StartScript(bh, SCRIPT_SWEEP);
    }
}

static void BroomHatterMain(struct Object *obj) {
    struct BroomHatter *bh = (struct BroomHatter *)obj;

    NidlUpdate(&bh->n);
    NidlFall(obj);
    obj->base.flags |= 4;
    if (obj->base.unk62 & 4) bh->landed = TRUE;
    if (bh->wait && --bh->wait) {
        NidlStopAtLedge(obj);
        return;
    }
    if (++bh->step >= bh->numSteps) {
        bh->step = bh->loopStart;
        if (bh->loops && ++bh->loop >= bh->loops) {
            obj->base.xspeed = 0;
            ChooseAction(bh);
            NidlStopAtLedge(obj);
            return;
        }
    }
    PlayStep(bh);
    NidlStopAtLedge(obj);
}

// BroomHatterStart(); Behavior start, also called by the engine after the Broom Hatter recovers from a hit;
void BroomHatterStart(struct Object *obj) {
    struct BroomHatter *bh = (struct BroomHatter *)obj;

    ObjectSetFunc(obj, 0, BroomHatterMain);
    NidlFaceKirby(obj);
    bh->lastAction = ACTION_SWEEP;
    StartScript(bh, bh->n.variant == 2 ? SCRIPT_STAND : SCRIPT_SWEEP);
}

// BroomHatterInit(); Its box, standing on the floor;
void BroomHatterInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -5, -4, 5, 7);
    ObjectSetBounds(&obj->base, -6, -5, 6, 9);
    ((struct BroomHatter *)n)->landed = FALSE;
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
