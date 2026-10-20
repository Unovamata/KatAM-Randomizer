/* Nightmare; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x43 (spawn 0x080AA288), as NiDL boss NIDL_NIGHTMARE,
 * the Fountain of Dreams' last (room 0x004 here). NiDL fights him on the ground with the Star Rod (Kirby walks, jumps and swings it,
 * shooting stars); so does every Kirby in his room here (abilities/star_rod.c; kept, finale.c's NidlFinaleKeep()), on the room's floor,
 * at its bottom, Nightmare flying above Kirby. His frames are streamed frames with three palettes
 * (0x0875456C's 8-15: his collar and arms, his body and its starry tornado, the ends of his cape), so his object draws the first
 * palette's pieces and two sprites follow it with the others' (NightmarePart); his beams (61-63) are attack objects with their own
 * palette. Its frames (KatAMExpansion.cs): 0-7 NiDL's 8-15 (the first palette's pieces), 8-15 the same (the second's), 16-23 the same
 * (the third's), 24-26 the beams.
 * What he does (NiDL's moves, as a fight on the ground): he floats above Kirby, his tornado swaying, and every so often fires a fan of
 * stars at him (enemies/star.c: a Kirby without the Star Rod inhales them and spits them back at him), sends a
 * beam along the floor, swoops down at him and back up, or vanishes and comes back on the other side. Touching him hurts, as do his
 * beams; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/songs.h"

#define FRAMES 8                // Each palette's pieces follow the one before's;
#define FRAME_BEAM 24
#define PART_TILES 32
#define BEAM_TILES 64
#define FLOAT_Y 70              // His height on the screen as he floats (screen pixels: his collar 60 above it);
#define FLOAT_SPEED 0xC0
#define SWOOP_SPEED 0x300
#define STAR_SPEED 0x1C0
#define BEAM_SPEED 0x200
#define BEAM_LIFE 150

enum { STATE_FLOAT, STATE_STARS, STATE_BEAM, STATE_SWOOP, STATE_RISE, STATE_VANISH, STATE_GONE, STATE_APPEAR };

struct Nightmare {
    struct NidlObject n;
    u8 state;
    u16 timer;
    u8 anim;
    u8 lastMove;
    bool8 started;
    bool8 hidden;
};

typedef char sNightmareFits[sizeof(struct Nightmare) <= NIDL_OBJECT_SIZE ? 1 : -1];

static s32 ScreenX(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
}

static s32 ScreenY(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.y >> 8;
}

// Key(); The palette slot of his frames' palettes (the first, second or third) and his beams';
static u16 Key(struct Nightmare *m, u8 frame) {
    u16 animId = NidlGraphicsOf(&m->n)->animId;

    if (frame >= FRAME_BEAM) return animId | 0x6000;
    if (frame >= FRAMES * 2) return animId | 0x4000;
    if (frame >= FRAMES) return animId | NIDL_SECOND_PALETTE;
    return animId;
}

// NightmarePart(); His second or third palette's pieces (unk4: 1, 2), following him (as KatAM's Kracko's eye: sub_080DD9B4);
static void NightmarePartMain(void) {
    struct Sprite sprite;
    struct EffectObject *part = TaskGetStructPtr(gCurTask);
    struct Nightmare *m = part->parent;
    struct Object *obj = &m->n.obj;
    u8 frame = ((m->anim / 6) & 7) + part->unk4 * FRAMES;

    if (part->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (obj->base.flags & 0x1000) {
        part->flags |= 0x1000;
        return;
    }
    Macro_08107BA8_4(part, &part->sprite, &sprite, PART_TILES, &part->sprite);
    if (!part->sprite.palId) part->sprite.palId = NidlLoadPalette(&m->n, Key(m, frame), frame);
    if (Macro_0810B1F4(&obj->base) && !(part->flags & 0x2000)) {
        EffectObjectDisplaySprite(part);
        return;
    }
    part->flags |= 4;
    part->flags = (part->flags & ~0x401) | (obj->base.flags & 0x401);
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->objBase54 = obj->base.objBase54;
    part->objBase55 = obj->base.objBase55;
    part->sprite.variant = frame;
    EffectObjectPostUpdate(part);
}

static void NightmarePart(struct Nightmare *m, u8 which) {
    struct Object *obj = &m->n.obj;
    struct Task *t = TaskCreate(NightmarePartMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *part = TaskGetStructPtr(t);
    u8 frame = which * FRAMES;

    ClearEffectObject(part);
    part->header.kind = 3;
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->parent = obj;
    part->roomId = obj->base.roomId;
    part->unk4 = which;
    // His body in front of his collar, the cape's ends behind it (his priority is 0x6C0 >> 6);
    EffectObjectInitSprite(part, &part->sprite, PART_TILES, NidlGraphicsOf(&m->n)->animId, frame, which == 1 ? 0x1A : 0x1C);
    part->sprite.palId = NidlLoadPalette(&m->n, Key(m, frame), frame);
}

// BeamMain(); A beam along the floor: an attack object (as Mr. Shine & Mr. Bright's), gone after its time or off the screen;
static void BeamMain(void) {
    struct Sprite sprite;
    struct ObjectBase *beam = TaskGetStructPtr(gCurTask);
    struct LevelInfo *level;
    s32 sx;

    if (ObjectPreUpdate(beam)) return;
    beam->flags |= 4;
    if (!(beam->flags & 0x200)) SetPointerSomething(beam);
    level = &gCurLevelInfo[beam->unk56];
    sx = (beam->x - level->viewportPosition.x) >> 8;
    if (++beam->counter >= BEAM_LIFE || sx < -32 || sx > 272) {
        beam->flags |= 0x1000;
        return;
    }
    beam->x += (beam->flags & 1) ? -BEAM_SPEED : BEAM_SPEED;
    beam->sprite.variant = FRAME_BEAM + ((beam->counter >> 2) % 3);
    Macro_08107BA8_4(beam, &beam->sprite, &sprite, BEAM_TILES, &beam->sprite);
    beam->sprite.palId = beam->filler6[0];
    sub_0806F8BC(beam);
}

// Beam(); A beam from under him along the floor, toward Kirby (the floor: Kirby's level);
static void Beam(struct Nightmare *m) {
    struct Object *obj = &m->n.obj;
    struct Task *t = TaskCreate(BeamMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *beam = TaskGetStructPtr(t);

    ClearObjectBase(beam);
    beam->header.kind = 2;
    beam->x = obj->base.x;
    beam->y = obj->kirby3->base.y - 0x1800;
    beam->parent = obj;
    beam->counter = 0;
    beam->roomId = obj->base.roomId;
    beam->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(beam)) beam->flags |= 0x2000;
    beam->unk63 = 1;
    beam->flags |= 0x10000140 | (obj->kirby3->base.x < obj->base.x ? 1 : 0);
    beam->unk68 = 0x20000043;
    beam->unk5C |= 0x80000;
    beam->filler6[0] = NidlLoadPalette(&m->n, Key(m, FRAME_BEAM), FRAME_BEAM);   // Its palette, in the padding;
    sub_0803E2B0(beam, -10, -40, 10, 40);
    ObjectSetBounds(beam, 0, 0, 0, 0);
    ObjectBaseInitSprite(beam, &beam->sprite, BEAM_TILES, NidlGraphicsOf(&m->n)->animId, FRAME_BEAM, obj->base.sprite.unk14 >> 6);
    beam->sprite.palId = beam->filler6[0];
    PlaySfx(&obj->base, SE_ABILITY_SPARK_ATTACK);
}

// Stars(); A fan of three stars at Kirby (inhalable ones), gone with him (sub_0809F840);
static void Stars(struct Nightmare *m) {
    struct Object *obj = &m->n.obj, *star;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) >> 8, d, xs, ys, turn;

    d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    if (d < 1) d = 1;
    xs = dx * STAR_SPEED / d;
    ys = -dy * STAR_SPEED / d;
    for (turn = -1; turn <= 1; turn++) {
        star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, (obj->base.y >> 8) + 16, 0, 0x1F, 0, 0, NIDL_TYPE,
                                       NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        star->base.xspeed = xs - ys * turn / 3;
        star->base.yspeed = ys + xs * turn / 3;
        star->base.parent = obj;
        star->unk7C = sub_0809F840;
    }
    PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
}

static void Set(struct Nightmare *m, u8 state) {
    m->state = state;
    m->timer = 0;
}

// Hide(); Gone (no box, not drawn: his sprites copy his 0x400) or back;
static void Hide(struct Nightmare *m, bool8 hidden) {
    struct Object *obj = &m->n.obj;

    m->hidden = hidden;
    if (hidden) {
        obj->base.flags |= 0x400 | 0x200;
    } else {
        obj->base.flags &= ~(0x400 | 0x200);
    }
}

// Choose(); His next move, never the same twice in a row;
static void Choose(struct Nightmare *m) {
    u8 move;

    do move = Rand16() & 3; while (move == m->lastMove);
    m->lastMove = move;
    Set(m, move == 0 ? STATE_STARS : move == 1 ? STATE_BEAM : move == 2 ? STATE_SWOOP : STATE_VANISH);
}

static void Update(struct Nightmare *m) {
    struct Object *obj = &m->n.obj;
    s32 x = (obj->base.x >> 8) - ScreenX(obj), y = (obj->base.y >> 8) - ScreenY(obj);
    s32 kx = (obj->kirby3->base.x >> 8) - ScreenX(obj), dx, dy;

    switch (m->state) {
    case STATE_FLOAT:
        // Toward over Kirby (his cape inside the screen), at his height;
        if (kx < 64) kx = 64;
        if (kx > 176) kx = 176;
        obj->base.xspeed = x < kx - 8 ? FLOAT_SPEED : x > kx + 8 ? -FLOAT_SPEED : 0;
        obj->base.yspeed = y < FLOAT_Y - 2 ? -FLOAT_SPEED : y > FLOAT_Y + 2 ? FLOAT_SPEED : 0;
        if (++m->timer >= 90) Choose(m);
        break;
    case STATE_STARS:
        obj->base.xspeed = obj->base.yspeed = 0;
        if (++m->timer == 20) Stars(m);
        if (m->timer >= 50) Set(m, STATE_FLOAT);
        break;
    case STATE_BEAM:
        obj->base.xspeed = obj->base.yspeed = 0;
        if (++m->timer == 16) Beam(m);
        if (m->timer >= 50) Set(m, STATE_FLOAT);
        break;
    case STATE_SWOOP:
        // Down at where Kirby is, then back up;
        if (!m->timer++) {
            dx = (obj->kirby3->base.x - obj->base.x) >> 8;
            dy = (obj->kirby3->base.y - obj->base.y) >> 8;
            if (dy < 16) dy = 16;
            obj->base.xspeed = dx * SWOOP_SPEED / (dy + (dx < 0 ? -dx : dx));
            obj->base.yspeed = -dy * SWOOP_SPEED / (dy + (dx < 0 ? -dx : dx));
            PlaySfx(&obj->base, SE_KIRBY_DASH);
        }
        if (m->timer >= 60 || y > 104) Set(m, STATE_RISE);
        break;
    case STATE_RISE:
        obj->base.yspeed = SWOOP_SPEED / 2;
        if (y <= FLOAT_Y || ++m->timer > 90) {
            obj->base.xspeed = obj->base.yspeed = 0;
            Set(m, STATE_FLOAT);
        }
        break;
    case STATE_VANISH:
        // Flickering out;
        obj->base.xspeed = obj->base.yspeed = 0;
        if (++m->timer & 2) obj->base.flags |= 0x400;
        else obj->base.flags &= ~0x400;
        if (m->timer >= 24) {
            Hide(m, TRUE);
            Set(m, STATE_GONE);
        }
        break;
    case STATE_GONE:
        // Back on the other side of the screen from Kirby;
        if (++m->timer >= 40) {
            obj->base.x = (ScreenX(obj) + (kx < 120 ? 184 : 56)) << 8;
            obj->base.y = (ScreenY(obj) + FLOAT_Y) << 8;
            Hide(m, FALSE);
            Set(m, STATE_APPEAR);
        }
        break;
    case STATE_APPEAR:
        if (++m->timer & 2) obj->base.flags |= 0x400;
        else obj->base.flags &= ~0x400;
        if (m->timer >= 24) {
            obj->base.flags &= ~0x400;
            Set(m, STATE_FLOAT);
        }
        break;
    }
    // (the screen's sides);
    if ((x < 48 && obj->base.xspeed < 0) || (x > 192 && obj->base.xspeed > 0)) obj->base.xspeed = 0;
}

static void NightmareMain(struct Object *obj) {
    struct Nightmare *m = (struct Nightmare *)obj;

    NidlUpdate(&m->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (his stars too); he takes it back;
    sub_08034E14(obj);
    m->anim++;
    NidlFaceKirby(obj);
    // (every Kirby in his room fights him with the Star Rod, finale.c);
    NidlFinaleKeep(obj);
    Update(m);
    NidlSetFrame(&m->n, (m->anim / 6) & 7);
}

// NightmareStart(); The fight starts (and after a hit: back where he was);
void NightmareStart(struct Object *obj) {
    struct Nightmare *m = (struct Nightmare *)obj;

    ObjectSetFunc(obj, -1, NightmareMain);
    if (m->started) return;
    m->started = TRUE;
    Set(m, STATE_FLOAT);
    NightmarePart(m, 1);
    NightmarePart(m, 2);
}

// NightmareInit(); His boxes (his collar and body), through anything;
void NightmareInit(struct NidlObject *n) {
    struct Nightmare *m = (struct Nightmare *)n;
    struct Object *obj = &n->obj;

    m->started = FALSE;
    m->hidden = FALSE;
    m->anim = 0;
    m->lastMove = 0xFF;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -20, -40, 20, 32);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
}
