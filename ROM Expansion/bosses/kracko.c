/* Kracko; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x41 (spawn 0x080A7D2C, states 0x08749150), as NiDL
 * boss NIDL_KRACKO, Grape Garden's (room 0x1D1 here: the cloud tower, fought on its bottom screen). NiDL fights it twice: first its
 * bare eye with four orbs chases Kirby up the tower as the screen scrolls (0x080A7D98's states, 0x08749158), then it grows its
 * cloud (0x080A8878) and the boss fight proper starts (0x080A87C8's states, 0x08749168); KatAM's rooms don't scroll by themselves, so
 * only the cloud fight is ported: it comes down from above the screen. NiDL draws it with three palettes (the cloud's top and
 * bottom, the eye): this object draws the cloud's top, and two sprites follow it (KrackoPart: the cloud's bottom, the eye, which
 * looks at Kirby: 0x080A9738, eight ways). Its frames (KatAMExpansion.cs): 0-2 the cloud's top (its spikes in, half out, out), 3-5
 * its bottom, 6-13 the eye (right, then each eighth of a turn clockwise), 14-29 the lightning bolts (1-8 segments, two of each).
 * NiDL script (16.16 speeds, Y down; KatAM's are 8.8, Y up; the screen is the same, 240x160; positions from its top left):
 * • Its hover: y 48, x 72 or 168 (its side: left or right).
 * • State 1 (0x080A8970), its choice, by what it did (0x2C): after its entrance the lightning (4), then the swoop (5); after a
 *   move it goes back to its hover on its side and bobs (state 2); after that, half the time (always after 5 attacks in a row) it
 *   drops a Waddle Doo (6) if it hasn't one out, else (and always after one) its next attack by where it is on the screen
 *   (0x087490B4: on its hover, 2/8 the sweep (3), 3/8 the swoop, 3/8 the lightning; never the lightning twice in a row: 0x087490DC);
 * • State 2 (0x080A8B6C): it bobs: y speeds 0.5, 1, 0.5, -0.5, -1, -0.5 down (0x087490E8), 8 frames each;
 * • State 3 (0x080A8BF4): it sweeps across to its other side at 1.25, bobbing (6 frames a step);
 * • State 4 (0x080A8D1C): the lightning: the screen darkens (0x080657F8), it backs off and dips (2, 1 down for 16; 1, 1 for 12;
 *   0.5, 0.5 for 8; still for 16), then crosses the screen at 1.25 for 144 frames, wavering (0x08749100: -0.5, 0.125, 0.5,
 *   -0.125 down, 4 frames each) and striking lightning down from its bottom (object 0x0C5, 32 lower, every 4 frames: segments of
 *   0x08754448 down to the floor, each 2 frames; a crackle every 10), then goes back up (1.125 for 4, -0.5 for 8, -1 for 12, -2 for
 *   16, rising 0.5, 1, 1) and the screen brightens again (0x080657CC); its side is the other one then;
 * • State 5 (0x080A8FDC): the swoop: a loop down to the floor and back up across the screen (the moves below), then the other side;
 * • State 6 (0x080A8F40): it stops for 36 frames and drops a Waddle Doo (NiDL type 0x19, 16 lower; KatAM's own here, which goes
 *   when Kracko does, as KatAM's Kracko's do: sub_0809F840);
 * • It stays within 16 pixels of the screen's sides and 32 of its top and bottom (0x080A932C).
 * Its spikes are out while it attacks (states 4-6). Touching it hurts; its lightning does too (attack objects, as the enemies'
 * stars). The Waddle Doo is how Kirby gets a power to hit it with, as in NiDL; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "constants/songs.h"

#define FRAME_CLOUD 0       // + spikes (0-2): the cloud's top;
#define FRAME_CLOUD_BOTTOM 3
#define FRAME_EYE 6         // + its way (0 right, then clockwise);
#define FRAME_BOLT 14       // + (segments - 1) * 2 + jag;
#define BOLT_SEGMENTS 8
#define BOLT_TILES 32
#define BOLT_TIME 4         // A new bolt every 4 frames, each shown 5 (NiDL's segments flicker, these don't);
#define BOLT_LIFE 5
#define BOLT_Y 32
#define PART_TILES 48

#define HOVER_Y 48
#define LEFT_X 72
#define RIGHT_X 168
#define ENTRANCE_Y -16
#define ENTRANCE_SPEED 0x80     // NiDL's 0.5 down;
#define SWEEP_SPEED 0x140       // 1.25;
#define SWEEP_BOB 6
#define DOO_WAIT 36
#define DOO_Y 16
#define DARKEST 8
#define SPIKE_TIME 4
#define CRACKLE_TIME 10

enum { STATE_ENTRANCE, STATE_CHOICE, STATE_BOB, STATE_SWEEP, STATE_LIGHTNING, STATE_SWOOP, STATE_DOO };
enum { PART_CLOUD_BOTTOM, PART_EYE };

/* Moves: speeds in eighths of a pixel a frame (NiDL's x by its facing, toward the screen's other side; y down) and frames; flags:
 * a bolt as it starts, the screen darkens or brightens, the swoop's sound; */
#define MOVE_BOLT 1
#define MOVE_DARK 2
#define MOVE_LIGHT 4
#define MOVE_SOUND 8
struct KrackoMove {
    s8 x, y;
    u8 time;
    u8 flags;
};

#define WAVER { 10, -4, 4, MOVE_BOLT }, { 10, 1, 4, MOVE_BOLT }, { 10, 4, 4, MOVE_BOLT }, { 10, -1, 4, MOVE_BOLT }
static const struct KrackoMove sLightning[] = {
    { -16, 8, 16, MOVE_DARK }, { -8, 8, 12, 0 }, { -4, 4, 8, 0 }, { 0, 0, 16, 0 },
    WAVER, WAVER, WAVER, WAVER, WAVER, WAVER, WAVER, WAVER, WAVER,
    { 10, 0, 6, MOVE_BOLT },
    { 9, 0, 4, MOVE_LIGHT }, { -4, -4, 8, 0 }, { -8, -8, 12, 0 }, { -16, -8, 16, 0 },
    { 0, 0, 0, 0 },
};

static const struct KrackoMove sSwoop[] = {
    { 16, -8, 10, 0 }, { 8, -4, 10, 0 }, { 4, -1, 10, 0 }, { 0, 0, 10, 0 },
    { -32, 4, 8, MOVE_SOUND }, { -24, 8, 8, 0 }, { -16, 16, 8, 0 }, { -8, 16, 4, 0 }, { -4, 24, 4, 0 }, { 4, 24, 4, 0 },
    { 8, 16, 4, 0 }, { 16, 16, 8, 0 }, { 24, 8, 8, 0 }, { 32, 4, 8, 0 }, { 48, 0, 10, 0 }, { 32, -16, 8, 0 },
    { 24, -32, 4, 0 }, { 16, -48, 4, 0 }, { 8, -32, 4, 0 }, { -8, -32, 4, 0 }, { -16, -16, 4, 0 }, { -32, -8, 4, 0 },
    { -24, -4, 8, 0 }, { -16, 4, 4, 0 }, { -8, 8, 4, 0 }, { -4, 16, 4, 0 }, { 4, 8, 4, 0 }, { 8, 8, 4, 0 }, { 8, 4, 4, 0 },
    { 1, 0, 6, 0 },
    { 0, 0, 0, 0 },
};

// 0x087490E8: the bob's y speeds (eighths, down);
static const s8 sBob[] = { 4, 8, 4, -4, -8, -4 };

// 0x087490B4: its next attack by where it is on the screen (zone), one of eight;
static const u8 sNext[5][8] = {
    { 3, 3, 3, 5, 5, 5, 5, 5 },
    { 3, 3, 3, 3, 3, 5, 5, 5 },
    { 3, 3, 5, 5, 5, 4, 4, 4 },
    { 3, 3, 3, 3, 5, 4, 4, 4 },
    { 5, 5, 5, 5, 4, 4, 4, 4 },
};

struct Kracko {
    struct NidlObject n;
    const struct KrackoMove *moves;
    u8 state;
    u8 step;
    u8 timer;
    u8 after;           // NiDL's 0x2C: 0 its first attacks, 1 after it bobbed, 2 after a move (back to its hover), 3 in one;
    u8 side;            // 0 left, 1 right;
    u8 last;            // Its last attack (NiDL's 0x24 by state);
    u8 attacks;         // Attacks since its last Waddle Doo, | 0x80 right after one;
    s8 facing;          // 1 right, -1 left (NiDL's 0x43);
    u8 spikes;          // 0-2;
    u8 spikeTimer;
    u8 eye;
    u8 dark;            // How dark the screen is (BLDY, 0-DARKEST);
    bool8 darkening;
    bool8 darkened;     // It changed the screen's blending (and restores it);
    bool8 started;
    u8 crackle;
    u16 bldCnt;
    u16 bldY;
    struct Object *doo; // Its Waddle Doo, while out;
};

typedef char sKrackoFits[sizeof(struct Kracko) <= NIDL_OBJECT_SIZE ? 1 : -1];

static s32 ScreenX(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
}

static s32 ScreenY(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.y >> 8;
}

// Way(); 0x080A9738: where Kirby is from it, eight ways (0 right, then clockwise: 2 down);
static u8 Way(struct Object *obj) {
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) >> 8;
    s32 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;

    if (ay * 5 < ax * 2) return dx >= 0 ? 0 : 4;
    if (ax * 5 < ay * 2) return dy >= 0 ? 2 : 6;
    if (dx >= 0) return dy >= 0 ? 1 : 7;
    return dy >= 0 ? 3 : 5;
}

/* KrackoPartMain(); The cloud's bottom or the eye (unk4): a sprite drawn where Kracko is, as KatAM's Kracko's eye (sub_080DD9B4),
 * gone when Kracko is; */
static void KrackoPartMain(void) {
    struct Sprite sprite;
    struct EffectObject *part = TaskGetStructPtr(gCurTask);
    struct Kracko *k = part->parent;
    u8 frame;

    if (part->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (k->n.obj.base.flags & 0x1000) {
        part->flags |= 0x1000;
        return;
    }
    frame = part->unk4 == PART_EYE ? FRAME_EYE + k->eye : FRAME_CLOUD_BOTTOM + k->spikes;
    Macro_08107BA8_4(part, &part->sprite, &sprite, PART_TILES, &part->sprite);
    if (!part->sprite.palId) part->sprite.palId = NidlLoadPalette(&k->n, NidlGraphicsOf(&k->n)->animId | (part->unk4 == PART_EYE ? 0x4000 : NIDL_SECOND_PALETTE), frame);
    if (Macro_0810B1F4(&k->n.obj.base) && !(part->flags & 0x2000)) {
        EffectObjectDisplaySprite(part);
        return;
    }
    part->flags |= 4;
    part->flags = (part->flags & ~0x400) | (k->n.obj.base.flags & 0x400);
    part->x = k->n.obj.base.x;
    part->y = k->n.obj.base.y;
    part->objBase54 = k->n.obj.base.objBase54;
    part->objBase55 = k->n.obj.base.objBase55;
    part->sprite.variant = frame;
    EffectObjectPostUpdate(part);
}

static void Part(struct Kracko *k, u8 which) {
    struct Object *obj = &k->n.obj;
    struct Task *t = TaskCreate(KrackoPartMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *part = TaskGetStructPtr(t);
    u8 frame = which == PART_EYE ? FRAME_EYE : FRAME_CLOUD_BOTTOM;

    ClearEffectObject(part);
    part->header.kind = 3;
    part->unk4 = which;
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->parent = obj;
    part->roomId = obj->base.roomId;
    // The eye in front of the cloud (a smaller priority is drawn over: the cloud's is 0x6C0 >> 6);
    EffectObjectInitSprite(part, &part->sprite, PART_TILES, NidlGraphicsOf(&k->n)->animId, frame, which == PART_EYE ? 0x1A : 0x1B);
    part->sprite.palId = NidlLoadPalette(&k->n, NidlGraphicsOf(&k->n)->animId | (which == PART_EYE ? 0x4000 : NIDL_SECOND_PALETTE), frame);
}

/* BoltMain(); A lightning bolt (object 0x0C5's segments, down to the floor): an attack object like KatAM's Kracko's (sub_080DE42C),
 * under Kracko for BOLT_TIME frames; */
static void BoltMain(void) {
    struct Sprite sprite;
    struct ObjectBase *bolt = TaskGetStructPtr(gCurTask);
    struct Kracko *k = bolt->parent;

    Macro_08107BA8_4(bolt, &bolt->sprite, &sprite, BOLT_TILES, &bolt->sprite);
    if (bolt->roomId != 0xFFFF && (k->n.obj.base.flags & 0x1000)) bolt->roomId = 0xFFFF;
    bolt->sprite.palId = bolt->filler6[0];
    if (ObjectPreUpdate(bolt)) return;
    if (++bolt->counter > BOLT_LIFE) {
        bolt->flags |= 0x1000;
        return;
    }
    if (bolt->roomId != 0xFFFF) {
        bolt->x = k->n.obj.base.x;
        bolt->y = k->n.obj.base.y + BOLT_Y * 0x100;
    }
    SetPointerSomething(bolt);
    sub_0806F8BC(bolt);
}

// Bolt(); 0x080A9C28: lightning from its bottom down to the floor (or BOLT_SEGMENTS segments);
static void Bolt(struct Kracko *k) {
    struct Object *obj = &k->n.obj;
    struct Task *t = TaskCreate(BoltMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *bolt = TaskGetStructPtr(t);
    s32 y = obj->base.y + BOLT_Y * 0x100;
    u8 segments = 1, frame;

    while (segments < BOLT_SEGMENTS && !NidlIsFloor(obj, obj->base.x, y + segments * 0x1000 + 0x800)) segments++;
    frame = FRAME_BOLT + (segments - 1) * 2 + (Rand16() & 1);
    ClearObjectBase(bolt);
    bolt->header.kind = 2;
    bolt->x = obj->base.x;
    bolt->y = y;
    bolt->parent = obj;
    bolt->counter = 0;
    bolt->roomId = obj->base.roomId;
    bolt->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(bolt)) bolt->flags |= 0x2000;
    bolt->unk63 = 1;
    // (Not 0x4000, KatAM's own bolts': that's VRAM shared by every sprite of one anim frame, these change frames);
    bolt->flags |= 0x10000000 | 0x2000000;
    bolt->unk68 = 0x20000403;
    bolt->unk5C |= 0x80000;
    bolt->filler6[0] = NidlLoadPalette(&k->n, NidlGraphicsOf(&k->n)->animId | 0x4000, frame);   // Its palette, kept in the padding;
    sub_0803E2B0(bolt, -6, 0, 6, segments * 16);
    ObjectSetBounds(bolt, 0, 0, 0, 0);
    ObjectBaseInitSprite(bolt, &bolt->sprite, BOLT_TILES, NidlGraphicsOf(&k->n)->animId, frame, 0x1A);
    bolt->sprite.palId = bolt->filler6[0];
}

// Doo(); 0x080A8F40: a Waddle Doo dropped from under it (KatAM's, its own until Kracko goes, as KatAM's Kracko's: sub_080DE064);
static void Doo(struct Kracko *k) {
    struct Object *obj = &k->n.obj;

    k->doo = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, (obj->base.y >> 8) + DOO_Y, 0, 0x1F, 0, 0, OBJ_WADDLE_DOO,
                                     0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    k->doo->base.parent = obj;
    k->doo->unk7C = sub_0809F840;
    PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
}

// DooOut(); Its Waddle Doo is still around (NiDL counts them: 0x02007D08);
static bool32 DooOut(struct Kracko *k) {
    struct Object *doo = k->doo;

    if (!doo) return FALSE;
    if (doo->base.header.kind && doo->type == OBJ_WADDLE_DOO && doo->base.parent == &k->n.obj && !(doo->base.flags & 0x1000) && doo->unk80 > 0)
        return TRUE;
    k->doo = NULL;
    return FALSE;
}

/* Darken(); The screen darker (BLDCNT's darken effect on the backgrounds, BLDY up to DARKEST, a step a frame) while it strikes
 * lightning, then back as it was; */
static void Darken(struct Kracko *k) {
    if (k->darkening) {
        if (!k->darkened) {
            k->bldCnt = gBldRegs.bldCnt;
            k->bldY = gBldRegs.bldY;
            k->darkened = TRUE;
        }
        if (k->dark < DARKEST) k->dark++;
    } else if (k->darkened) {
        if (k->dark) k->dark--;
        if (!k->dark) {
            gBldRegs.bldCnt = k->bldCnt;
            gBldRegs.bldY = k->bldY;
            k->darkened = FALSE;
            return;
        }
    } else {
        return;
    }
    gBldRegs.bldCnt = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG3 | BLDCNT_TGT1_BD;
    gBldRegs.bldY = k->dark;
}

static void Run(struct Kracko *k, u8 state, const struct KrackoMove *moves) {
    k->state = state;
    k->moves = moves;
    k->step = 0;
    k->timer = 0;
}

static void Start(struct Kracko *k, u8 state) {
    struct Object *obj = &k->n.obj;
    s32 x = (obj->base.x >> 8) - ScreenX(obj);

    switch (state) {
    case STATE_BOB:
        Run(k, state, NULL);
        break;
    case STATE_SWEEP:
        k->side = x > 127;
        k->facing = k->side ? -1 : 1;
        Run(k, state, NULL);
        break;
    case STATE_LIGHTNING:
        k->facing = k->side ? -1 : 1;
        Run(k, state, sLightning);
        break;
    case STATE_SWOOP:
        k->facing = k->side ? -1 : 1;
        Run(k, state, sSwoop);
        PlaySfx(&obj->base, SE_KRACKO_SWEEP_ATTACK);
        break;
    default:
        Run(k, state, NULL);
        break;
    }
}

// Choose(); State 1 (0x080A8970): what next;
static void Choose(struct Kracko *k) {
    struct Object *obj = &k->n.obj;
    s32 x = (obj->base.x >> 8) - ScreenX(obj);
    u8 zone, next;

    obj->base.xspeed = obj->base.yspeed = 0;
    switch (k->after) {
    case 0:
        if (!k->last) {
            k->last = STATE_LIGHTNING;
            Start(k, STATE_LIGHTNING);
        } else {
            k->last = STATE_SWOOP;
            Start(k, STATE_SWOOP);
            k->after = 3;
        }
        return;
    case 2:
        // Back on its hover, on its side;
        obj->base.x = (ScreenX(obj) + (k->side ? RIGHT_X : LEFT_X)) * 0x100;
        obj->base.y = (ScreenY(obj) + HOVER_Y) * 0x100;
        Start(k, STATE_BOB);
        k->after = 3;
        return;
    }
    if (k->attacks & 0x80) {
        k->attacks = 0;
    } else if (((Rand16() & 1) || k->attacks > 4) && !DooOut(k)) {
        k->attacks = 0x80;
        Start(k, STATE_DOO);
        k->after = 3;
        return;
    }
    k->attacks++;
    if (x <= LEFT_X - 1) zone = k->side ? 3 : 4;
    else if (x > RIGHT_X) zone = k->side ? 4 : 3;
    else zone = 2;
    next = sNext[zone][Rand16() & 7];
    if (next == STATE_LIGHTNING && k->last == STATE_LIGHTNING) next = (Rand16() & 1) ? STATE_SWOOP : STATE_SWEEP;
    Start(k, next);
    k->last = next;
    k->after = 3;
}

// Moved(); A move list is done: its side is the other one, back to its hover;
static void Moved(struct Kracko *k) {
    k->side ^= 1;
    if (k->after) k->after = 2;
    Start(k, STATE_CHOICE);
}

// Step(); The move list's next step when the last one is done;
static void Step(struct Kracko *k) {
    struct Object *obj = &k->n.obj;
    const struct KrackoMove *move;

    if (k->timer && --k->timer) {
        move = &k->moves[k->step - 1];
        if ((move->flags & MOVE_BOLT) && k->timer == move->time - BOLT_TIME) Bolt(k);
        return;
    }
    move = &k->moves[k->step];
    if (!move->time) {
        Moved(k);
        return;
    }
    k->step++;
    k->timer = move->time;
    obj->base.xspeed = move->x * 32 * k->facing;
    obj->base.yspeed = -move->y * 32;
    if (move->flags & MOVE_BOLT) Bolt(k);
    if (move->flags & MOVE_DARK) k->darkening = TRUE;
    if (move->flags & MOVE_LIGHT) k->darkening = FALSE;
    if (move->flags & MOVE_SOUND) PlaySfx(&obj->base, SE_KRACKO_SWEEP_ATTACK);
}

static void Update(struct Kracko *k) {
    struct Object *obj = &k->n.obj;
    s32 left = ScreenX(obj), top = ScreenY(obj), x;

    switch (k->state) {
    case STATE_ENTRANCE:
        obj->base.yspeed = -ENTRANCE_SPEED;
        if ((obj->base.y >> 8) >= top + HOVER_Y) {
            obj->base.y = (top + HOVER_Y) * 0x100;
            obj->base.yspeed = 0;
            k->after = 0;
            Start(k, STATE_CHOICE);
        }
        break;
    case STATE_CHOICE:
        Choose(k);
        break;
    case STATE_BOB:
        obj->base.xspeed = 0;
        obj->base.yspeed = -sBob[k->step] * 32;
        if (++k->timer < 8) break;
        k->timer = 0;
        if (++k->step < ARRAY_COUNT(sBob)) break;
        k->after = 1;
        Start(k, STATE_CHOICE);
        break;
    case STATE_SWEEP:
        obj->base.xspeed = SWEEP_SPEED * k->facing;
        obj->base.yspeed = -sBob[k->step] * 32;
        if (++k->timer >= SWEEP_BOB) {
            k->timer = 0;
            if (++k->step >= ARRAY_COUNT(sBob)) k->step = 0;
        }
        // 0x080A8C84: at its other side;
        x = (obj->base.x >> 8) - left;
        if (k->facing > 0 ? x >= RIGHT_X : x <= LEFT_X) {
            obj->base.x = (left + (k->facing > 0 ? RIGHT_X : LEFT_X)) * 0x100;
            Moved(k);
        }
        break;
    case STATE_LIGHTNING:
    case STATE_SWOOP:
        Step(k);
        if (k->state == STATE_LIGHTNING && k->moves == sLightning && k->step > 4 && k->darkening && !--k->crackle) {
            k->crackle = CRACKLE_TIME;
            PlaySfx(&obj->base, SE_KRACKO_LIGHTNING_ATTACK);
        }
        break;
    case STATE_DOO:
        obj->base.xspeed = obj->base.yspeed = 0;
        if (++k->timer < DOO_WAIT) break;
        Doo(k);
        k->after = 2;
        Start(k, STATE_CHOICE);
        break;
    }
}

// Keep(); 0x080A932C: within 16 pixels of the screen's sides, 32 of its top and bottom;
static void Keep(struct Object *obj) {
    s32 left = ScreenX(obj) * 0x100, top = ScreenY(obj) * 0x100;

    if (obj->base.x < left - 0x1000) obj->base.x = left - 0x1000;
    if (obj->base.x > left + 0x10000) obj->base.x = left + 0x10000;
    if (obj->base.y < top - 0x2000) obj->base.y = top - 0x2000;
    if (obj->base.y > top + 0xC000) obj->base.y = top + 0xC000;
}

// KrackoDefeated(); The pre-callback, every frame: once beaten (KatAM's defeat holds it, its main not called), the screen back;
static void KrackoDefeated(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;

    if (obj->unk80 > 0) return;
    k->darkening = FALSE;
    Darken(k);
}

static void KrackoMain(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;
    bool8 attacking;

    NidlUpdate(&k->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (0x08034E14: its Waddle Doo too); it takes it back;
    sub_08034E14(obj);

    Update(k);
    Keep(obj);
    Darken(k);
    attacking = k->state == STATE_LIGHTNING || k->state == STATE_SWOOP || k->state == STATE_DOO;
    if (++k->spikeTimer >= SPIKE_TIME) {
        k->spikeTimer = 0;
        if (attacking && k->spikes < 2) k->spikes++;
        else if (!attacking && k->spikes) k->spikes--;
    }
    k->eye = Way(obj);
    NidlSetFrame(&k->n, FRAME_CLOUD + k->spikes);
}

// KrackoStart(); The fight starts: it comes down from above the screen, on its right;
void KrackoStart(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;

    ObjectSetFunc(obj, -1, KrackoMain);
    if (k->started) return;
    k->started = TRUE;
    obj->unk7C = KrackoDefeated;
    obj->base.flags &= ~0x400;
    obj->base.x = (ScreenX(obj) + RIGHT_X) * 0x100;
    obj->base.y = (ScreenY(obj) + ENTRANCE_Y) * 0x100;
    obj->base.xspeed = obj->base.yspeed = 0;
    k->side = 1;
    k->facing = -1;
    k->last = 0;
    k->attacks = 0;
    k->after = 0;
    k->crackle = CRACKLE_TIME;
    Run(k, STATE_ENTRANCE, NULL);
    Part(k, PART_CLOUD_BOTTOM);
    Part(k, PART_EYE);
}

// KrackoInit(); Its boxes (the cloud), unseen until the fight starts; it flies, through anything;
void KrackoInit(struct NidlObject *n) {
    struct Kracko *k = (struct Kracko *)n;
    struct Object *obj = &n->obj;

    k->spikes = 0;
    k->spikeTimer = 0;
    k->eye = 4;
    k->dark = 0;
    k->darkening = FALSE;
    k->darkened = FALSE;
    k->doo = NULL;
    k->started = FALSE;
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;    // (Not KatAM's Kracko's electric touch: NiDL's only hurts);
    sub_0803E2B0(&obj->base, -40, -22, 40, 22);
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
}
