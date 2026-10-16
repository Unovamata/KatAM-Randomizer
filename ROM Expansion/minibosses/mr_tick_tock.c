/* Mr. Tick-Tock; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object type 0x35 (spawn 0x0809A0A8), as NiDL
 * mini-boss NIDL_MR_TICK_TOCK. Its body frames (0-16 of 0x08753180) are streamed: each frame has its own tiles and palette, which
 * NiDL uploads with it (see KatAMExpansion.cs). Its rings (17-22) and notes (23-28) are drawn from its descriptor's tiles.
 * NiDL script (24 states, 0x08745750, per frame 0x087457B0; 16.16 speeds, Y down; KatAM's are 8.8, Y up). Variant 1 is faster:
 * • State 0 (0x0809A214): idles (frames 4 and 16, 15 and 9 frames) for 120 frames, then state 1.
 * • State 1 (0x0809A2A0), deciding (0x08099DB0, 0x08099DEC): every 4th time a jump and an alarm (state 9); else one of 16 picks
 *   by where Kirby is (more than 128 pixels away or not, more than 64 above or below or not; 0x087456D4-0x08745704): hops (2,
 *   1 or 3 of them, 0x087456D0), backing off (3), a pounce (4) or a charge (5).
 * • State 2: hops in place (frame 6, 5.0 up, gravity 0x5000, falling at 7.0 at most).
 * • State 3: backs off (1.0, variant 1 1.5) for 64 frames, walking (frames 11, 12, 11, 10, 7 each, variant 1 5).
 * • State 4: faces Kirby, crouches (4, 5 for 8 each) and pounces (6, 5.0 up, gravity 0x3700, 1.0 forward, variant 1 1.25), then
 *   waits 16 frames.
 * • States 5 and 6: backs up (0.5) for 20 frames, then charges (1.5, variant 1 1.75) for 20; Kirby more than 32 pixels away, it
 *   keeps running (12) and slides to a stop ringing its alarm (13: rings around it, 143 frames); else it hops (7) and rings in
 *   place (8: rings around it, 120 frames).
 * • States 9-11: a hop in place and one forward (6, 2.5 up, gravity 0x3000; 1.25 forward), then the alarm (116 frames) throwing 4
 *   notes (0x08099EE4: type 0x77, 16 pixels to either side, 2.625 up, gravity 0x3000, forward at 0.75, 2.25, -1.5 or -3.0 at
 *   random, 0x087456CC; they hop once more where they land and vanish).
 * • State 14 (0x0809ACBC): idles facing Kirby (4 and 16) for a while, then state 1.
 * The rings and the notes are attack objects here (like the enemies' stars), drawing the mini-boss's own frames; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: NiDL's 0-28. Its hurt frame is NiDL's 13;
#define FRAME_IDLE 4
#define FRAME_CROUCH 5
#define FRAME_JUMP 6
#define FRAME_RING 8           // And 9;
#define FRAME_WALK 10          // 10-12;
#define FRAME_IDLE_2 16
#define FRAME_RINGS 17         // 17-22;
#define RING_FRAMES 6
#define FRAME_NOTE 23

#define RINGS_TILES 48
#define NOTE_TILES 8

enum {
    STATE_IDLE, STATE_DECIDE, STATE_HOP, STATE_BACK_OFF, STATE_POUNCE, STATE_WIND_UP, STATE_CHARGE, STATE_RUN, STATE_SLIDE_RING,
    STATE_HOP_RING, STATE_RING, STATE_JUMP, STATE_JUMP_FORWARD, STATE_ALARM, STATE_WAIT,
};

// 0x087456D4-0x08745704: the decisions by where Kirby is (2 hops, 3 backs off, 4 pounces, 5 charges);
static const u8 sDecisions[4][16] = {
    { 3, 4, 3, 2, 4, 3, 4, 3, 2, 4, 3, 4, 3, 4, 2, 4 },    // Far, above or below;
    { 2, 3, 4, 3, 3, 4, 3, 2, 3, 4, 3, 3, 2, 3, 4, 3 },    // Far, level;
    { 5, 5, 4, 5, 5, 2, 5, 5, 4, 5, 5, 5, 2, 5, 5, 5 },    // Near, above or below;
    { 5, 5, 3, 5, 5, 5, 2, 5, 5, 3, 5, 5, 5, 2, 5, 5 },    // Near, level;
};
static const u8 sHops[] = { 1, 3 };                         // 0x087456D0;
static const s16 sNoteSpeeds[] = { 0xC0, 0x240, -0x180, -0x300 };   // 0x087456CC;
static const u8 sNoteTimes[] = { 100, 76, 52, 28 };
static const s16 sWalk[] = { FRAME_WALK + 1, 7, FRAME_WALK + 2, 7, FRAME_WALK + 1, 7, FRAME_WALK, 7, NIDL_LOOP };
static const s16 sWalkFast[] = { FRAME_WALK + 1, 5, FRAME_WALK + 2, 5, FRAME_WALK + 1, 5, FRAME_WALK, 5, NIDL_LOOP };
static const s16 sIdle[] = { FRAME_IDLE, 14, FRAME_IDLE_2, 8, NIDL_LOOP };
static const s16 sRing[] = { FRAME_RING, 2, FRAME_RING + 1, 2, NIDL_LOOP };

#define NOTE_HEIGHT 0x2A0
#define NOTE_GRAVITY 0x30
#define MAX_FALL 0x300

struct MrTickTock {
    struct NidlObject n;
    u8 state;
    u8 count;           // Decisions (every 4th a jump and an alarm);
    u8 hops;
    bool8 airborne;
    u16 timer;
    s16 gravity;
    s16 maxFall;
    s16 speed;          // Sliding speed;
    u8 effectPalette;   // Its descriptor's palette (rings and notes);
    u8 note;
    struct ObjectBase *rings;
};

typedef char sTickTockFits[sizeof(struct MrTickTock) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void SetState(struct MrTickTock *t, u8 state);

static bool8 Fast(struct MrTickTock *t) {
    return t->n.variant & 1;
}

static void Forward(struct Object *obj, s16 speed) {
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static void Leap(struct MrTickTock *t, s16 height, s16 gravity, s16 maxFall) {
    struct Object *obj = &t->n.obj;

    obj->base.yspeed = height;
    t->gravity = gravity;
    t->maxFall = maxFall;
    t->airborne = FALSE;
    t->n.sequence = NULL;
    NidlSetFrame(&t->n, FRAME_JUMP);
}

static bool8 Landed(struct MrTickTock *t) {
    struct Object *obj = &t->n.obj;

    if (!(obj->base.unk62 & 4)) t->airborne = TRUE;
    return t->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

/* RingsMain(); The rings its alarm sends out (type 0x76, 0x08099E9C): an attack object around it, drawing its rings (17-22, one
 * every 2 frames); gone when it stops ringing; */
static void RingsMain(void) {
    struct Sprite sprite;
    struct ObjectBase *rings = TaskGetStructPtr(gCurTask);
    struct MrTickTock *t = rings->parent;
    struct Object *obj = &t->n.obj;

    if (ObjectPreUpdate(rings)) return;
    if ((obj->base.flags & 0x1000) || t->rings != rings || obj->unk83 < 2) {
        if (t->rings == rings) t->rings = NULL;
        rings->flags |= 0x1000;
        return;
    }
    rings->flags |= 4;
    if (!(rings->flags & 0x200)) SetPointerSomething(rings);
    rings->x = obj->base.x;
    rings->y = obj->base.y;
    if (++rings->counter >= RING_FRAMES * 2) rings->counter = 0;
    Macro_08107BA8_4(rings, &rings->sprite, &sprite, RINGS_TILES, &rings->sprite);
    rings->sprite.variant = FRAME_RINGS + (rings->counter >> 1);
    rings->sprite.palId = t->effectPalette;
    sub_0806F8BC(rings);
}

static void Rings(struct MrTickTock *t) {
    struct Object *obj = &t->n.obj;
    struct Task *task;
    struct ObjectBase *rings;

    if (t->rings) return;
    task = TaskCreate(RingsMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    rings = TaskGetStructPtr(task);
    ClearObjectBase(rings);
    rings->header.kind = 2;
    rings->x = obj->base.x;
    rings->y = obj->base.y;
    rings->parent = t;
    rings->counter = 0;
    rings->roomId = obj->base.roomId;
    rings->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(rings)) rings->flags |= 0x2000;
    rings->unk63 = 1;
    rings->flags |= 0x10000140;
    rings->unk68 = 0x20000043;
    rings->unk5C |= 0x80000;
    sub_0803E2B0(rings, -32, -32, 32, 32);
    ObjectSetBounds(rings, 0, 0, 0, 0);
    ObjectBaseInitSprite(rings, &rings->sprite, RINGS_TILES, NidlGraphicsOf(&t->n)->animId, FRAME_RINGS, obj->base.sprite.unk14 >> 6);
    rings->sprite.palId = t->effectPalette;
    t->rings = rings;
}

static void StopRings(struct MrTickTock *t) {
    if (!t->rings) return;
    t->rings->flags |= 0x1000;
    t->rings = NULL;
}

/* NoteMain(); A note (type 0x77, 0x0809B9F0): an attack object falling with gravity 0x30, hopping once where it lands (2.625 up),
 * gone when it lands again; counter: its landings; */
static void NoteMain(void) {
    struct Sprite sprite;
    struct ObjectBase *note = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(note)) return;
    note->flags |= 4;
    if (!(note->flags & 0x200)) SetPointerSomething(note);
    note->yspeed -= NOTE_GRAVITY;
    if (note->yspeed < -MAX_FALL) note->yspeed = -MAX_FALL;
    note->x += note->xspeed;
    note->y -= note->yspeed;
    if (note->yspeed < 0 && NidlIsFloor((struct Object *)note, note->x, note->y + 0x800)) {
        if (++note->counter >= 2) {
            note->flags |= 0x1000;
            return;
        }
        note->y = ((note->y + 0x800) & ~0xFFF) - 0x801;
        note->xspeed = 0;
        note->yspeed = NOTE_HEIGHT;
    }
    Macro_08107BA8_4(note, &note->sprite, &sprite, NOTE_TILES, &note->sprite);
    note->sprite.variant = FRAME_NOTE + (note->yspeed < 0);
    note->sprite.palId = note->filler6[0];
    sub_0806F8BC(note);
}

// Note(); 0x08099EE4: 16 pixels to one side (by the note's number), at one of 4 speeds;
static void Note(struct MrTickTock *t) {
    struct Object *obj = &t->n.obj;
    struct Task *task = TaskCreate(NoteMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *note = TaskGetStructPtr(task);
    s16 speed = sNoteSpeeds[Rand16() & 3];

    ClearObjectBase(note);
    note->header.kind = 2;
    note->x = obj->base.x + ((t->note & 1) ? 0x1000 : -0x1000);
    note->y = obj->base.y;
    note->parent = obj;
    note->counter = 0;
    note->roomId = obj->base.roomId;
    note->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(note)) note->flags |= 0x2000;
    note->unk63 = 1;
    note->flags |= 0x10000140 | (obj->base.flags & 1);
    note->unk68 = 0x20000043;
    note->unk5C |= 0x80000;
    note->xspeed = (obj->base.flags & 1) ? -speed : speed;
    note->yspeed = NOTE_HEIGHT;
    note->filler6[0] = t->effectPalette;
    sub_0803E2B0(note, -5, -8, 5, 8);
    ObjectSetBounds(note, 0, 0, 0, 0);
    ObjectBaseInitSprite(note, &note->sprite, NOTE_TILES, NidlGraphicsOf(&t->n)->animId, FRAME_NOTE, obj->base.sprite.unk14 >> 6);
    note->sprite.palId = t->effectPalette;
    t->note++;
}

// Decide(); 0x08099DB0, 0x08099DEC;
static void Decide(struct MrTickTock *t) {
    struct Object *obj = &t->n.obj;
    s32 dx = abs(obj->kirby3->base.x - obj->base.x) >> 8, dy = abs(obj->kirby3->base.y - obj->base.y) >> 8;
    u8 state;

    t->count = (t->count + 1) & 0x13;
    if (t->count == 3) {
        SetState(t, STATE_JUMP);
        return;
    }
    state = sDecisions[(dx > 128 ? 0 : 2) + (dy > 64 ? 0 : 1)][Rand16() & 15];
    if (state == 2) t->hops = sHops[Rand16() & 1];
    SetState(t, state == 2 ? STATE_HOP : state == 3 ? STATE_BACK_OFF : state == 4 ? STATE_POUNCE : STATE_WIND_UP);
}

static void SetState(struct MrTickTock *t, u8 state) {
    struct Object *obj = &t->n.obj;

    t->state = state;
    t->timer = 0;
    switch (state) {
    case STATE_IDLE:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        NidlPlay(&t->n, sIdle);
        t->timer = 66;
        break;
    case STATE_HOP:
        obj->base.xspeed = 0;
        Leap(t, 0x500, 0x50, 0x700);
        break;
    case STATE_BACK_OFF:
        Forward(obj, Fast(t) ? -0x180 : -0x100);
        NidlPlay(&t->n, Fast(t) ? sWalkFast : sWalk);
        t->timer = 64;
        break;
    case STATE_POUNCE:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        t->n.sequence = NULL;
        NidlSetFrame(&t->n, FRAME_IDLE);
        t->timer = 16;
        t->hops = 0;
        break;
    case STATE_WIND_UP:
        NidlFaceKirby(obj);
        Forward(obj, -0x80);
        NidlPlay(&t->n, sWalk);
        t->timer = Fast(t) ? 22 : 20;
        break;
    case STATE_CHARGE:
        Forward(obj, Fast(t) ? 0x1C0 : 0x180);
        NidlPlay(&t->n, sWalkFast);
        t->timer = 20;
        break;
    case STATE_RUN:
        t->timer = 48;
        break;
    case STATE_SLIDE_RING:
        t->speed = abs(obj->base.xspeed);
        NidlPlay(&t->n, sRing);
        t->timer = 143;
        break;
    case STATE_HOP_RING:
        obj->base.xspeed = 0;
        Leap(t, 0x500, 0x50, 0x300);
        break;
    case STATE_RING:
        obj->base.xspeed = 0;
        NidlPlay(&t->n, sRing);
        Rings(t);
        t->timer = 120;
        break;
    case STATE_JUMP:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        Leap(t, 0x280, 0x30, 0x300);
        break;
    case STATE_JUMP_FORWARD:
        NidlFaceKirby(obj);
        Forward(obj, 0x140);
        Leap(t, 0x280, 0x30, 0x300);
        break;
    case STATE_ALARM:
        obj->base.xspeed = 0;
        NidlPlay(&t->n, sRing);
        t->timer = 116;
        t->note = 0;
        break;
    case STATE_WAIT:
        obj->base.xspeed = 0;
        t->n.sequence = NULL;
        NidlSetFrame(&t->n, FRAME_IDLE);
        t->timer = 16;
        break;
    }
}

static void MrTickTockMain(struct Object *obj) {
    struct MrTickTock *t = (struct MrTickTock *)obj;
    u8 i;

    NidlUpdate(&t->n);
    NidlAnimate(&t->n);
    obj->base.flags |= 4;
    obj->base.yspeed -= t->gravity;
    if (obj->base.yspeed < -t->maxFall) obj->base.yspeed = -t->maxFall;

    switch (t->state) {
    case STATE_IDLE:
    case STATE_BACK_OFF:
    case STATE_WAIT:
        if (!--t->timer) SetState(t, STATE_DECIDE);
        break;
    case STATE_DECIDE:
        Decide(t);
        break;
    case STATE_HOP:
        if (!Landed(t)) break;
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        if (--t->hops) Leap(t, 0x500, 0x50, 0x700);
        else SetState(t, STATE_DECIDE);
        break;
    case STATE_POUNCE:
        // Crouches (4, 5 for 8 each), then pounces (hops: 0 crouching, 1 in the air); after landing it waits 16 frames;
        if (!t->hops) {
            if (t->timer == 8) NidlSetFrame(&t->n, FRAME_CROUCH);
            if (--t->timer) break;
            Forward(obj, Fast(t) ? 0x140 : 0x100);
            Leap(t, 0x500, 0x37, 0x300);
            t->hops = 1;
            break;
        }
        if (!Landed(t)) break;
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        SetState(t, STATE_WAIT);
        break;
    case STATE_WIND_UP:
        if (!--t->timer) SetState(t, STATE_CHARGE);
        break;
    case STATE_CHARGE:
        if (--t->timer) break;
        SetState(t, (abs(obj->kirby3->base.x - obj->base.x) >> 8) > 32 ? STATE_RUN : STATE_HOP_RING);
        break;
    case STATE_RUN:
        if (--t->timer && (abs(obj->kirby3->base.x - obj->base.x) >> 8) > 24 && !(obj->base.unk62 & 3)) break;
        SetState(t, STATE_SLIDE_RING);
        break;
    case STATE_SLIDE_RING:
        // It slides to a stop (traced), ringing, its rings out after 10 frames;
        if (t->speed > 6) t->speed -= 6;
        else t->speed = 0;
        Forward(obj, t->speed);
        if (t->timer == 133) Rings(t);
        if (--t->timer) break;
        StopRings(t);
        SetState(t, STATE_IDLE);
        break;
    case STATE_HOP_RING:
        if (Landed(t)) SetState(t, STATE_RING);
        break;
    case STATE_RING:
        if (--t->timer) break;
        StopRings(t);
        SetState(t, STATE_IDLE);
        break;
    case STATE_JUMP:
        if (Landed(t)) SetState(t, STATE_JUMP_FORWARD);
        break;
    case STATE_JUMP_FORWARD:
        if (Landed(t)) SetState(t, STATE_ALARM);
        break;
    case STATE_ALARM:
        --t->timer;
        for (i = 0; i < ARRAY_COUNT(sNoteTimes); i++) {
            if (t->timer == sNoteTimes[i]) {
                PlaySfx(&obj->base, SE_PRANK_THROW_ITEM);
                Note(t);
            }
        }
        if (!t->timer) SetState(t, STATE_IDLE);
        break;
    }
}

// MrTickTockStart(); The fight starts, and again after a hit;
void MrTickTockStart(struct Object *obj) {
    struct MrTickTock *t = (struct MrTickTock *)obj;

    ObjectSetFunc(obj, 0, MrTickTockMain);
    StopRings(t);
    t->effectPalette = NidlLoadPalette(&t->n, NidlGraphicsOf(&t->n)->animId | NIDL_SECOND_PALETTE, FRAME_RINGS);
    t->gravity = 0x15;
    t->maxFall = MAX_FALL;
    obj->base.yspeed = 0;
    SetState(t, STATE_IDLE);
}

// MrTickTockInit(); Its box, on the floor;
void MrTickTockInit(struct NidlObject *n) {
    struct MrTickTock *t = (struct MrTickTock *)n;
    struct Object *obj = &n->obj;

    t->rings = NULL;
    t->count = 0;
    t->gravity = 0x15;
    t->maxFall = MAX_FALL;
    NidlSetFrame(n, FRAME_IDLE);
    sub_0803E2B0(&obj->base, -20, -18, 20, 22);
    ObjectSetBounds(&obj->base, -22, -18, 22, 24);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
