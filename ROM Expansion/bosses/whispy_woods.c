/* Whispy Woods; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x40 (spawn 0x080B2A00), as NiDL boss
 * NIDL_WHISPY_WOODS, Vegetable Valley's (room 0x9F here). The tree is the room's own (its tiles and collision, NiDL's too); the boss
 * is its face, which NiDL draws as three frames of one table at its spot (0x080B2890: the eyes, 0x087542C0 + i, the nose, + 38 + i,
 * the mouth, + 56 + i; the frames place the nose and mouth lower themselves), one frame each here (KatAMExpansion.cs), 0-17: 4-6 the blink, 7-9
 * surprised and angry, 10-13 crying, 14-16 blowing, 17 its usual face. Its spot is NiDL's, (206, 256), the template's.
 * NiDL script (states 0x0874C12C; 16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • State 0 (0x080B2AE4): face 17 for 8, a blink (0x080B25A4: faces 4, 5, 6, 5 and 4 for 1, 1, 1, 2 and 2), face 17 for 63, a
 *   blink, then the next attack by its phase (0x080B2664, 0 at first): phase 0: a quarter of the time 4 puffs (phase 1), else apples
 *   (phase 2); phase 1: apples (2); phase 2: a quarter of the time apples (3), else 2 puffs (0); phase 3: 2 puffs (0).
 * • States 1 and 2 (0x080B2B40, 0x080B2C18): 2 or 4 puffs: faces 14, 15 and 16 for 1, 1 and 8, a puff (0x080B2768: sound 0x223,
 *   object 0x7E from its mouth), 16 for 8 more, 15 and 14 for 2 each; then 14 for 8, 6 and 5 for 2, and state 0.
 * • State 3 (0x080B2CF0): 3 times faces 4, 5, 6, 5 and 4 for 4 each and 17 for 30; an apple (0x080B27B0: object 0x7D) at the
 *   start of each (its timer, 0x080B2D80: 150, 100 and 50), from the branches (y 224) over one of 5 columns (x 16, 48, 80, 112 or
 *   144; one time in six the one over Kirby, 0x080B2804); then state 0.
 * • Hit (0x080B25E8, timer 0x2C): surprised (9), then 7 and 4, the script held meanwhile here.
 * • Defeated (0x080B2E58): crying, faces 12 and 13 (4 frames each); here while KatAM's defeat holds it, before it explodes.
 * The puffs (object 0x7E, frames 0x0874C568 from NiDL's shared object tiles, palette 15) are attack objects here, as the
 * enemies' stars, blown along the ground away from the tree. The apples are type 0x31 objects (enemies/apple.c): Kirby inhales
 * them and spits them back at the tree; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

#define FACE_IDLE 17
#define FACE_SURPRISED 9
#define FACE_STARTLED 7
#define FACE_BLINK 4
#define FACE_CRY 12
#define FRAME_PUFF 18       // The puff's frames follow the faces (NiDL's 0x0874C568, 0-7);

#define PUFF_TILES 8
#define PUFF_SPEED 0x200
#define PUFF_TIME 100
#define PUFF_X -24          // From its spot: its mouth's left (its frames draw the mouth 26-44 down and 4-28 across);
#define PUFF_Y 34
#define APPLE_Y 224
#define HIT_TIME 32         // The hit faces (0x080B25E8's timer): surprised (9) until 20, startled (7) until 16, then 4;

// Script steps: (face, frames), or a command; NiDL's states;
#define CMD_PUFF -1
#define CMD_APPLE -2
#define CMD_END -3
#define BLINK 4, 1, 5, 1, 6, 1, 5, 2, 4, 2
#define PUFF 14, 1, 15, 1, 16, 8, CMD_PUFF, 16, 8, 15, 2, 14, 2
#define APPLE CMD_APPLE, 4, 4, 5, 4, 6, 4, 5, 4, 4, 4, FACE_IDLE, 30

static const s8 sIdle[] = { FACE_IDLE, 8, BLINK, FACE_IDLE, 63, BLINK, CMD_END };
static const s8 sPuffs2[] = { PUFF, PUFF, 14, 8, 6, 2, 5, 2, CMD_END };
static const s8 sPuffs4[] = { PUFF, PUFF, PUFF, PUFF, 14, 8, 6, 2, 5, 2, CMD_END };
static const s8 sApples[] = { APPLE, APPLE, APPLE, CMD_END };

static const u8 sColumns[] = { 16, 48, 80, 112, 144 };
static const u8 sPuffFrames[] = { 0, 2, 4, 6 };

struct WhispyWoods {
    struct NidlObject n;
    const s8 *script;
    u8 step;
    u8 timer;
    u8 face;
    u8 phase;
    u8 hit;             // Hit faces timer (HIT_TIME down);
    s16 hp;             // What it had last frame (a hit takes some);
    u8 cry;
};

typedef char sWhispyFits[sizeof(struct WhispyWoods) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* PuffMain(); A puff (object 0x7E): an attack object like the enemies' stars, blown along the ground away from the tree, growing
 * (NiDL's frames 0, 2, 4 and 6, 8 frames each), gone after PUFF_TIME; */
static void PuffMain(void) {
    struct Sprite sprite;
    struct ObjectBase *puff = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(puff)) return;
    puff->flags |= 4;
    if (!(puff->flags & 0x200)) SetPointerSomething(puff);
    puff->x += puff->xspeed;
    if (++puff->counter >= PUFF_TIME) {
        puff->flags |= 0x1000;
        return;
    }
    Macro_08107BA8_4(puff, &puff->sprite, &sprite, PUFF_TILES, &puff->sprite);
    puff->sprite.variant = FRAME_PUFF + sPuffFrames[(puff->counter >> 3) & 3];
    puff->sprite.palId = puff->filler6[0];
    sub_0806F8BC(puff);
}

static void Puff(struct WhispyWoods *w) {
    struct Object *obj = &w->n.obj;
    struct Task *t = TaskCreate(PuffMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *puff = TaskGetStructPtr(t);
    // NiDL's palette 15, which the puff's frames load (under the boss's second palette key);
    u8 palId = NidlLoadPalette(&w->n, NidlGraphicsOf(&w->n)->animId | NIDL_SECOND_PALETTE, FRAME_PUFF);

    ClearObjectBase(puff);
    puff->header.kind = 2;
    puff->x = obj->base.x + PUFF_X * 0x100;
    puff->y = obj->base.y + PUFF_Y * 0x100;
    puff->parent = obj;
    puff->counter = 0;
    puff->roomId = obj->base.roomId;
    puff->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(puff)) puff->flags |= 0x2000;
    puff->unk63 = 1;
    puff->flags |= 0x10000140 | 1;
    puff->unk68 = 0x20000043;
    puff->unk5C |= 0x80000;
    puff->xspeed = -PUFF_SPEED;
    puff->filler6[0] = palId;   // Its palette, kept in the padding;
    sub_0803E2B0(puff, -6, -6, 6, 6);
    ObjectSetBounds(puff, 0, 0, 0, 0);
    ObjectBaseInitSprite(puff, &puff->sprite, PUFF_TILES, NidlGraphicsOf(&w->n)->animId, FRAME_PUFF, obj->base.sprite.unk14 >> 6);
    puff->sprite.palId = palId;
    PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
}

/* Apple(); 0x080B27B0: an apple (type 0x31, NIDL_APPLE) from the branches over one of the columns, the one over Kirby one time in
 * six (0x080B2804: Kirby's x on the screen, in 32 pixel steps up to the last); the columns are from the room's left (its screen's); */
static void Apple(struct WhispyWoods *w) {
    struct Object *obj = &w->n.obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 left = level->viewportPosition.x >> 8, column = Rand16() % 6;

    if (column == 0) {
        s32 kirby = (obj->kirby3->base.x >> 8) - left;
        column = kirby < 0 ? 0 : kirby >> 5;
        if (column > 4) column = 4;
    } else {
        column--;
    }
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, left + sColumns[column], APPLE_Y, 0, 0x1F, 0, 0, NIDL_TYPE, NIDL_SPECIAL << 4 | 1,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void Run(struct WhispyWoods *w, const s8 *script) {
    w->script = script;
    w->step = 0;
    w->timer = 0;
}

// Next(); 0x080B2664: the attack after state 0, by its phase;
static void Next(struct WhispyWoods *w) {
    switch (w->phase) {
    case 0:
        if (!(Rand16() & 3)) {
            w->phase = 1;
            Run(w, sPuffs4);
        } else {
            w->phase = 2;
            Run(w, sApples);
        }
        break;
    case 1:
        w->phase = 2;
        Run(w, sApples);
        break;
    case 2:
        if (!(Rand16() & 3)) {
            w->phase = 3;
            Run(w, sApples);
        } else {
            w->phase = 0;
            Run(w, sPuffs2);
        }
        break;
    default:
        w->phase = 0;
        Run(w, sPuffs2);
        break;
    }
}

// Step(); Runs the script until a step that waits;
static void Step(struct WhispyWoods *w) {
    if (w->timer && --w->timer) return;
    for (;;) {
        s8 cmd = w->script[w->step];

        if (cmd == CMD_END) {
            if (w->script == sIdle) Next(w);
            else Run(w, sIdle);
            continue;
        }
        w->step++;
        if (cmd == CMD_PUFF) {
            Puff(w);
            continue;
        }
        if (cmd == CMD_APPLE) {
            Apple(w);
            continue;
        }
        w->face = cmd;
        w->timer = w->script[w->step++];
        return;
    }
}

// WhispyCry(); The pre-callback, every frame: crying once beaten (KatAM's defeat holds it until it explodes, its main not called);
static void WhispyCry(struct Object *obj) {
    struct WhispyWoods *w = (struct WhispyWoods *)obj;

    if (obj->unk80 > 0) return;
    w->cry++;
    NidlSetFrame(&w->n, FACE_CRY + ((w->cry >> 2) & 1));
}

static void WhispyWoodsMain(struct Object *obj) {
    struct WhispyWoods *w = (struct WhispyWoods *)obj;
    u8 face;

    NidlUpdate(&w->n);
    obj->base.flags |= 4;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->kirby3 = FindTargetKirby(&obj->base);
    if (obj->unk80 < w->hp) w->hit = HIT_TIME;
    w->hp = obj->unk80;
    // The HP bar goes to whatever is hit (0x08034E14: an apple too); it takes it back, as Meta Knight does;
    sub_08034E14(obj);

    if (w->hit) {
        w->hit--;
        face = w->hit >= 20 ? FACE_SURPRISED : w->hit >= 16 ? FACE_STARTLED : FACE_BLINK;
    } else {
        Step(w);
        face = w->face;
    }
    NidlSetFrame(&w->n, face);
}

// WhispyWoodsStart(); The fight starts (state 0);
void WhispyWoodsStart(struct Object *obj) {
    struct WhispyWoods *w = (struct WhispyWoods *)obj;

    ObjectSetFunc(obj, -1, WhispyWoodsMain);
    obj->unk7C = WhispyCry;
    w->hp = obj->unk80;
    w->hit = 0;
    w->phase = 0;
    Run(w, sIdle);
    Step(w);
    NidlSetFrame(&w->n, w->face);
}

/* WhispyWoodsInit(); Its boxes: the trunk around its face and a little in front of it (the trunk is the room's wall: Kirby's attacks
 * reach it from there; NiDL's 0x080B2A00 sets the face's spot, which the template has); touching it doesn't hurt (it's a tree); */
void WhispyWoodsInit(struct NidlObject *n) {
    struct WhispyWoods *w = (struct WhispyWoods *)n;
    struct Object *obj = &n->obj;

    w->face = FACE_IDLE;
    w->cry = 0;
    // Facing left, at Kirby (NiDL's frames face right; it draws them flipped);
    obj->base.flags |= 1;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 0xA0;
    sub_0803E2B0(&obj->base, -54, -24, 36, 88);
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
}
