/* Poppy Bros. Sr.; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object type 0x32 (spawn 0x080910C0), as NiDL
 * mini-boss NIDL_POPPY_BROS_SR. NiDL draws it as three objects: the body (frames 0-11 of 0x087530F8), its head (type 0xB3, 0x08091DDC:
 * frames 12-19, at the body's position, playing the head animation the body asks for: 0x08743A00) and, while it throws, its hands
 * holding the bomb (type 0xB4, 0x08091B6C: frames 20-25; the bomb, frames 26-30, is drawn with the second palette).
 * NiDL script (states 0x08743988, per frame 0x087439A4; 16.16 speeds, Y down; KatAM's are 8.8, Y up; gravity 1/16, falling at 3.0
 * at most, 0x0800622C):
 * • State 1 (0x08091320): a few hops (0x087438DC: variant 0 4, 2, 2 or 2; variant 1 4, 6, 2 or 0), then state 2. Hops (0x08091954):
 *   facing Kirby, 0.75 forward, 1.125 up; one within 39 pixels of Kirby hops back instead, and every hop after one goes the other
 *   way. Forward its body shows frames 4, 6, 8, 9, 7, 5 (3 each, 0x08091A30) and the head animation 0 (frame 0); back 5, 7, 9, 8, 6,
 *   4 (0x08091A98) and the head animation 1 (1, 2, 3, 2, 1, 0 for 2, 2, 10, 16, 20).
 * • State 2 (0x08091390): a jump straight up (3.0) with a bomb: frames 5, 7, 9, 8, 6 and 4 (4 each), the head animation 2 (frame 4),
 *   then 3 (5, 6, 7, 6, 5, 4 for 4 each) once it's falling (0x08091558). It throws the bomb ahead near the top of the jump; within 63
 *   pixels of Kirby it holds it longer (traced: thrown 33 frames into the jump, 61 when Kirby is near). The bomb (category 9 index
 *   0x6F, 0x08093BD4) flies at 1.5 ahead, 0.5 up, gravity 1/8, bounces once (2.0 up) and explodes when it lands again.
 * • State 3 (0x080915A4): 2 hops, then state 4 (0x080915F8): a jump forward (1.25, 1.25 up, 6 frames a step) and one back (1.5
 *   up), then state 5 (0x080916EC): a long jump forward (2.5, 1.75 up, 7 a step), then back the same way if Kirby is within 55
 *   pixels, else straight up (2.0, 10 a step), then state 1 again.
 * • State 0 (it falls in) and 6 (defeated) are KatAM's own here.
 * Here the head is drawn with the body: its frames are every body frame (4-9) with every head frame (0-7). The hands and the bomb
 * they hold are an effect object; the thrown bomb is an attack object (like the enemies' stars and ice), exploding into KatAM's
 * small explosion (Foley's); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: body frame (NiDL's 4-9) with head frame (0-7), then the hands (NiDL's 20-25) and the bomb (26-30). Its hurt frame is NiDL's 10;
#define FRAME(body, head) (((body) - 4) * 8 + (head))
#define FRAME_HANDS 48
#define FRAME_BOMB 54

#define GRAVITY 0x10
#define MAX_FALL 0x300
#define HOP_SPEED 0xC0
#define HOP_HEIGHT 0x120
#define HOP_STEP 3
#define BACK_RANGE 39
#define THROW_HEIGHT 0x300
#define THROW_STEP 4
#define NEAR_RANGE 63
#define THROW_TIME 33
#define NEAR_THROW_TIME 61
#define BOMB_SPEED 0x180
#define BOMB_HEIGHT 0x80
#define BOMB_GRAVITY 0x20
#define BOMB_BOUNCE 0x200
#define BOMB_TILES 8
#define BOMB_AHEAD 12       // Held behind its head (NiDL's +12), thrown from ahead of it;
#define BOMB_UP 32
#define BIG_RANGE 55

enum { STATE_HOPS, STATE_THROW, STATE_JUMPS, STATE_DONE };
enum { NEXT_THROW, NEXT_JUMPS, NEXT_HOPS };

// 0x08743A00: the head animations (frame, frames), NIDL_END holds the last one;
static const s16 sHeadForward[] = { 0, 1, NIDL_END };
static const s16 sHeadBack[] = { 1, 2, 2, 2, 3, 10, 2, 16, 1, 20, 0, 1, NIDL_END };
static const s16 sHeadUp[] = { 4, 1, NIDL_END };
static const s16 sHeadThrow[] = { 5, 4, 6, 4, 7, 4, 6, 4, 5, 4, 4, 1, NIDL_END };

static const u8 sForwardFrames[] = { 4, 6, 8, 9, 7, 5 };
static const u8 sBackFrames[] = { 5, 7, 9, 8, 6, 4 };
static const u8 sHops[2][4] = { { 4, 2, 2, 2 }, { 4, 6, 2, 0 } };   // 0x087438DC;

// One jump of states 4 and 5: forward (or back) speed, height, frames per step;
struct Jump {
    s16 speed;
    s16 height;
    u8 step;
};
static const struct Jump sJumps[][2] = {
    { { 0x140, 0x140, 6 }, { -0x140, 0x180, 6 } },  // State 4;
    { { 0x280, 0x1C0, 7 }, { -0x280, 0x1C0, 7 } },  // State 5, back when Kirby is near;
};
static const struct Jump sStraightUp = { 0, 0x200, 10 };

struct PoppyBrosSr {
    struct NidlObject n;
    u8 state;
    u8 next;            // What follows the hops (NEXT_*);
    u8 hops;
    bool8 back;         // The next hop goes back;
    bool8 airborne;
    u8 body;            // Body frame (NiDL's 4-9);
    const u8 *frames;   // The jump's body frames, one per step;
    u8 step;
    u8 stepTime;
    u8 stepTimer;
    const s16 *head;    // Head animation;
    u8 headStep;
    u8 headTimer;
    u8 headFrame;
    u8 jumps;           // Jumps left in state 4 or 5;
    u8 jumpSet;
    u16 timer;
    u16 throwTime;
    struct EffectObject *hands;
    u8 bombPalette;
};

typedef char sPoppySrFits[sizeof(struct PoppyBrosSr) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void ShowFrame(struct PoppyBrosSr *p) {
    NidlSetFrame(&p->n, FRAME(p->body, p->headFrame));
}

static void PlayHead(struct PoppyBrosSr *p, const s16 *head) {
    p->head = head;
    p->headStep = 0;
    p->headTimer = 0;
}

// StepHead(); The head object (0x08091E18) steps its animation once a frame;
static void StepHead(struct PoppyBrosSr *p) {
    const s16 *entry;

    if (!p->head || (p->headTimer && --p->headTimer)) return;
    entry = &p->head[p->headStep * 2];
    if (entry[0] == NIDL_END) {
        p->head = NULL;
        return;
    }
    p->headFrame = entry[0];
    p->headTimer = entry[1];
    p->headStep++;
}

// StepBody(); The jump's body frames, one every stepTime frames, holding the last;
static void StepBody(struct PoppyBrosSr *p) {
    if (!p->frames || --p->stepTimer) return;
    p->stepTimer = p->stepTime;
    if (p->step < 5) p->body = p->frames[++p->step];
}

static s32 KirbyDistance(struct Object *obj) {
    return abs(obj->kirby3->base.x - obj->base.x) >> 8;
}

static void Leap(struct PoppyBrosSr *p, s16 speed, s16 height, u8 step, bool8 back) {
    struct Object *obj = &p->n.obj;

    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
    obj->base.yspeed = height;
    p->airborne = FALSE;
    p->frames = back ? sBackFrames : sForwardFrames;
    p->step = 0;
    p->body = p->frames[0];
    p->stepTime = p->stepTimer = step;
    PlayHead(p, back ? sHeadBack : sHeadForward);
}

// Hop(); 0x08091954: one hop, at Kirby; back when Kirby is close, and the other way after that;
static void Hop(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;

    NidlFaceKirby(obj);
    if (KirbyDistance(obj) <= BACK_RANGE) p->back = TRUE;
    Leap(p, p->back ? -HOP_SPEED : HOP_SPEED, HOP_HEIGHT, HOP_STEP, p->back);
}

static void Hops(struct PoppyBrosSr *p, u8 hops, u8 next) {
    p->state = STATE_HOPS;
    p->hops = hops;
    p->next = next;
    p->back = FALSE;
    if (p->hops) Hop(p);
}

/* HandsMain(); The bomb it holds over its head (NiDL's hands, 0x08091B6C, hold it): an effect object drawing the mini-boss's
 * bomb frame, BOMB_AHEAD pixels behind it and BOMB_UP up; gone once the bomb is thrown or when the mini-boss is hurt or gone; */
static void HandsMain(void) {
    struct Sprite sprite;
    struct EffectObject *h = TaskGetStructPtr(gCurTask);
    struct PoppyBrosSr *p = h->parent;
    struct Object *obj;

    if (h->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    obj = &p->n.obj;
    if ((obj->base.flags & 0x1000) || p->hands != h || obj->unk83 < 2) {
        if (p->hands == h) p->hands = NULL;
        h->flags |= 0x1000;
        return;
    }
    h->x = obj->base.x + ((obj->base.flags & 1) ? BOMB_AHEAD : -BOMB_AHEAD) * 0x100;
    h->y = obj->base.y - BOMB_UP * 0x100;
    h->flags = (h->flags & ~1) | (obj->base.flags & 1);
    if (Macro_0810B1F4(&obj->base) && !(h->flags & 0x2000)) {
        EffectObjectDisplaySprite(h);
        return;
    }
    Macro_08107BA8_4(h, &h->sprite, &sprite, BOMB_TILES, &h->sprite);
    h->sprite.variant = FRAME_BOMB + 4;
    h->sprite.palId = p->bombPalette;
    EffectObjectPostUpdate(h);
}

static void CreateHands(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;
    struct Task *t = TaskCreate(HandsMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *h = TaskGetStructPtr(t);

    ClearEffectObject(h);
    h->header.kind = 3;
    h->x = obj->base.x;
    h->y = obj->base.y - BOMB_UP * 0x100;
    h->parent = p;
    h->roomId = obj->base.roomId;
    h->flags |= obj->base.flags & 1;
    if (Macro_0810B1F4(&obj->base)) h->flags |= 0x2000;
    // The bomb's palette: NiDL's second palette of the mini-boss, which its bomb frames load;
    p->bombPalette = NidlLoadPalette(&p->n, NidlGraphicsOf(&p->n)->animId | NIDL_SECOND_PALETTE, FRAME_BOMB);
    EffectObjectInitSprite(h, &h->sprite, BOMB_TILES, NidlGraphicsOf(&p->n)->animId, FRAME_BOMB + 4, obj->base.sprite.unk14 >> 6);
    h->sprite.palId = p->bombPalette;
    p->hands = h;
}

/* BombMain(); The thrown bomb (0x08093BD4): an attack object like the enemies' stars, falling with gravity 1/8; it bounces once
 * and explodes (KatAM's small explosion) when it lands again; */
static void BombMain(void) {
    struct Sprite sprite;
    struct ObjectBase *bomb = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(bomb)) return;
    bomb->flags |= 4;
    if (!(bomb->flags & 0x200)) SetPointerSomething(bomb);
    bomb->yspeed -= BOMB_GRAVITY;
    if (bomb->yspeed < -MAX_FALL) bomb->yspeed = -MAX_FALL;
    bomb->x += bomb->xspeed;
    bomb->y -= bomb->yspeed;
    if (bomb->yspeed < 0 && NidlIsFloor((struct Object *)bomb, bomb->x, bomb->y + 0x400)) {
        bomb->y = ((bomb->y + 0x400) & ~0xFFF) - 0x401;
        if (++bomb->counter >= 2) {
            bomb->flags |= 0x1000;
            sub_08073D2C(bomb);
            return;
        }
        bomb->yspeed = BOMB_BOUNCE;
    }
    Macro_08107BA8_4(bomb, &bomb->sprite, &sprite, BOMB_TILES, &bomb->sprite);
    bomb->sprite.variant = FRAME_BOMB + 4;
    bomb->sprite.palId = bomb->filler6[0];
    sub_0806F8BC(bomb);
}

static void Bomb(struct PoppyBrosSr *p, u8 palId) {
    struct Object *obj = &p->n.obj;
    struct Task *t = TaskCreate(BombMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *bomb = TaskGetStructPtr(t);

    ClearObjectBase(bomb);
    bomb->header.kind = 2;
    bomb->x = obj->base.x + ((obj->base.flags & 1) ? -BOMB_AHEAD : BOMB_AHEAD) * 0x100;
    bomb->y = obj->base.y - BOMB_UP * 0x100;
    bomb->parent = obj;
    bomb->counter = 0;
    bomb->roomId = obj->base.roomId;
    bomb->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(bomb)) bomb->flags |= 0x2000;
    bomb->unk63 = 1;
    bomb->flags |= 0x10000140 | (obj->base.flags & 1);
    bomb->unk68 = 0x20000043;
    bomb->unk5C |= 0x80000;
    bomb->xspeed = (obj->base.flags & 1) ? -BOMB_SPEED : BOMB_SPEED;
    bomb->yspeed = BOMB_HEIGHT;
    bomb->filler6[0] = palId;   // Its palette, kept in the padding;
    sub_0803E2B0(bomb, -5, -5, 5, 5);
    ObjectSetBounds(bomb, 0, 0, 0, 0);
    ObjectBaseInitSprite(bomb, &bomb->sprite, BOMB_TILES, NidlGraphicsOf(&p->n)->animId, FRAME_BOMB + 4, obj->base.sprite.unk14 >> 6);
    bomb->sprite.palId = palId;
    PlaySfx(bomb, SE_PRANK_THROW_ITEM);
}

// Throw(); 0x08091390: straight up with a bomb, held longer when Kirby is near;
static void Throw(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;

    p->state = STATE_THROW;
    NidlFaceKirby(obj);
    p->throwTime = KirbyDistance(obj) <= NEAR_RANGE ? NEAR_THROW_TIME : THROW_TIME;
    p->timer = 0;
    obj->base.xspeed = 0;
    obj->base.yspeed = THROW_HEIGHT;
    p->airborne = FALSE;
    p->frames = sBackFrames;
    p->step = 0;
    p->body = sBackFrames[0];
    p->stepTime = p->stepTimer = THROW_STEP;
    PlayHead(p, sHeadUp);
    CreateHands(p);
}

// Jump(); States 4 and 5: the next of their two jumps (0x080915F8, 0x080916EC);
static void Jump(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;
    const struct Jump *j = &sJumps[p->jumpSet][2 - p->jumps];

    if (p->jumps == 2) NidlFaceKirby(obj);
    else if (p->jumpSet == 1 && KirbyDistance(obj) > BIG_RANGE) j = &sStraightUp;
    Leap(p, j->speed, j->height, j->step, p->jumps == 1);
}

static void Jumps(struct PoppyBrosSr *p, u8 set) {
    p->state = STATE_JUMPS;
    p->jumpSet = set;
    p->jumps = 2;
    Jump(p);
}

// Landed(); It came down on the floor (after leaving it);
static bool8 Landed(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;

    if (!(obj->base.unk62 & 4)) p->airborne = TRUE;
    return p->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

static void Next(struct PoppyBrosSr *p) {
    struct Object *obj = &p->n.obj;

    obj->base.xspeed = 0;
    switch (p->next) {
    case NEXT_THROW:
        Throw(p);
        break;
    case NEXT_JUMPS:
        Jumps(p, 0);
        break;
    default:
        Hops(p, sHops[p->n.variant & 1][Rand16() & 3], NEXT_THROW);
        break;
    }
}

static void PoppyBrosSrMain(struct Object *obj) {
    struct PoppyBrosSr *p = (struct PoppyBrosSr *)obj;

    NidlUpdate(&p->n);
    obj->base.flags |= 4;
    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
    StepBody(p);
    StepHead(p);

    switch (p->state) {
    case STATE_HOPS:
        if (p->hops && !Landed(p)) break;
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        if (p->hops && --p->hops) {
            p->back = !p->back;
            Hop(p);
            break;
        }
        Next(p);
        break;
    case STATE_THROW:
        ++p->timer;
        // 0x08091558: falling, it throws (the head's animation 3);
        if (obj->base.yspeed < 0 && p->head == NULL && p->headFrame == 4) PlayHead(p, sHeadThrow);
        if (p->timer == p->throwTime && p->hands) {
            p->hands->flags |= 0x1000;
            p->hands = NULL;
            Bomb(p, p->bombPalette);
        }
        if (!Landed(p)) break;
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        obj->base.xspeed = 0;
        Hops(p, 2, NEXT_JUMPS);
        break;
    case STATE_JUMPS:
        if (!Landed(p)) break;
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        if (--p->jumps) {
            Jump(p);
            break;
        }
        obj->base.xspeed = 0;
        if (p->jumpSet == 0) {
            Jumps(p, 1);
            break;
        }
        p->next = NEXT_HOPS;
        Next(p);
        break;
    }
    ShowFrame(p);
}

// PoppyBrosSrStart(); The fight starts (state 1), and again after a hit;
void PoppyBrosSrStart(struct Object *obj) {
    struct PoppyBrosSr *p = (struct PoppyBrosSr *)obj;

    ObjectSetFunc(obj, 0, PoppyBrosSrMain);
    if (p->hands) {
        p->hands->flags |= 0x1000;
        p->hands = NULL;
    }
    p->frames = NULL;
    p->body = 4;
    PlayHead(p, sHeadForward);
    StepHead(p);
    p->next = NEXT_HOPS;
    Next(p);
    ShowFrame(p);
}

// PoppyBrosSrInit(); Its box, on the floor;
void PoppyBrosSrInit(struct NidlObject *n) {
    struct PoppyBrosSr *p = (struct PoppyBrosSr *)n;
    struct Object *obj = &n->obj;

    p->hands = NULL;
    p->frames = NULL;
    p->head = NULL;
    p->body = 4;
    p->headFrame = 0;
    sub_0803E2B0(&obj->base, -12, -28, 12, 10);
    ObjectSetBounds(&obj->base, -12, -28, 12, 11);
    NidlOnFloor(obj);
}
