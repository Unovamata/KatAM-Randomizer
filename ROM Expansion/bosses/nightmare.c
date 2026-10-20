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

/* The intro (NightmareIntroRider()): the Power Orb's warp star brings the Kirbys in (its flight's step after the room change is
 * NIDL_RIDE_FINALE: warpstar.c holds them on it, this moves them), NiDL's, traced frame by frame (the star's and the camera's): the
 * screen from the room's top (34) down a pixel a frame for 158 frames, then a pixel every 4 frames down to its bottom (216, the
 * fight's screen; finale.c's NidlFinaleHoldCamera()), the star flying down into it with the Kirbys (sIntroStar: its moves on the
 * screen, from (0, -35) above its top left; hidden and kept on the screen till it shows, NightmareRideSize(): KatAM points at helpers
 * off it), then about it; 910 frames in, a blue star hits it (see below): the star's gone (NightmareRideSize(): NIDL_RIDE_ALONE,
 * warpstar.c draws the Kirbys without it), and the Kirbys are knocked off, held still (Knock(): NiDL's tumble up and down to the
 * floor, its bounce, then its slide back, KatAM's tumbling animations as the rides' and the Star Rod's back view; the helpers
 * spreading out to the right of the player as they fly); 260 frames on they're up, the flight ends (its last steps: a frame standing,
 * then the end, facing him, KatAMExpansionRooms.cs) and they're the players' again, as his light grows into his shape where he
 * appears; then the fight starts (he waits till then, hidden: NightmareIntroOn(), nidl_boss.c); */
#define INTRO_CAM_X (8 << 8)
#define INTRO_CAM_Y (35 << 8)
#define INTRO_CAM_FAST 158
#define INTRO_CAM_END (216 << 8)
#define INTRO_STAR_X 0
#define INTRO_STAR_Y (-35 << 8)
#define INTRO_OFF (-16 << 8)
#define INTRO_ON_SCREEN (24 << 8)
#define INTRO_HIT 910
#define INTRO_LIGHT 1169
#define INTRO_STANDS 1170
#define INTRO_ENDS 1184
#define NIDL_RIDE_FINALE 0xFC
#define NIDL_RIDE_HIDDEN 0x8000
#define NIDL_RIDE_ALONE 0x4000

void sub_080534D0(struct Kirby *);

// The star's moves on the screen frame by frame from the second (x + 8, (y + 8) << 4, pixels);
// (909 frames)
static const u8 sIntroStar[909] = {
    0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x78, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x88, 0x88,
    0x79, 0x88, 0x88, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0xA8, 0x98, 0x98,
    0x89, 0xA8, 0x98, 0x98, 0x89, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x88, 0x88,
    0x79, 0x88, 0x88, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x89, 0x88, 0x79, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0xA8, 0x98, 0x98,
    0x89, 0xA8, 0x98, 0x98, 0x89, 0x98, 0x99, 0x88, 0x89, 0x98, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x88, 0x88,
    0x89, 0x78, 0x88, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x98, 0x98, 0x98,
    0x99, 0x98, 0x98, 0x98, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x89, 0x88,
    0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x89, 0x88, 0x89, 0x88, 0x89, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88, 0x99, 0x88,
    0x99, 0x88, 0x99, 0x88, 0x99, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x77, 0x88, 0x77,
    0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x87, 0x98, 0x87,
    0x98, 0x87, 0x98, 0x87, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x97, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x87, 0x98, 0x77, 0x88, 0x77,
    0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x78, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x77, 0x88, 0x87, 0x98, 0x87,
    0x98, 0x87, 0x98, 0x87, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x77, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x77, 0x88, 0x78, 0x88, 0x78, 0x88, 0x78, 0x88, 0x87, 0x98, 0x88,
    0x98, 0x88, 0x98, 0x88, 0x98, 0x97, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x87, 0x98, 0x88, 0x98, 0x88, 0x98, 0x88, 0x98, 0x77, 0x88, 0x78,
    0x88, 0x78, 0x88, 0x78, 0x88, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xCC, 0xCC, 0xCC,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4,
    0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x48, 0x48, 0x48,
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C,
    0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,
    0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC4, 0xC4, 0xC4,
    0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0xC4, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x8C, 0x8C, 0x8C,
    0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8,
    0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x67, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x86, 0x96, 0x7A, 0x8A, 0x7A,
    0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0x7A, 0x8A, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9, 0xA9,
};

/* Its effects (NiDL's cutscene, frame by frame as the star's: t; KatAMExpansion.cs's frames after his beams, drawn by sprites of the
 * intro's own, IntroSprite()): the comet he comes as, far off in the sky (FRAME_COMET: the dark orb, then waking up, CometFrame();
 * drifting left, then off right, CometSpeed()), the screen flashing as it wakes (FlashLevel(): the room's palettes brighter, the
 * sprites' darker, Flash(); steady with "Reduce Flashing"), the blue stars it throws (FRAME_BLUE_STAR, sBlueStars: each its moves,
 * growing and spinning by an affine matrix, Spin(), blinking out at the end; all drawn from one sprite) and the bursts they make
 * (FRAME_BURST, sBursts: on their star); */
#define FRAME_COMET 27
#define FRAME_BLUE_STAR 33
#define FRAME_BURST 34
#define COMET_TILES 16
#define BLUE_STAR_TILES 64
#define BURST_TILES 48
#define COMET_X (207 << 8 | 0xA0)
#define COMET_Y (256 << 8)
#define COMET_GONE 596
#define BURST_FRAMES 23
#define INTRO_KEY_A 0x8000      // The comet's palettes' keys (| his anim ID: KatAM takes its low 12 bits);
#define INTRO_KEY_B 0xA000
#define INTRO_MATRIX 1          // The blue stars' affine matrices: 1-5 (KatAM's own use 0, the rides' 7, 15, 23, 31);
#define INTRO_MATRICES 5
#define FLASH_BG (14 * 16)      // The flash's colors: the room's (BG palettes 0-13: not the HUD's 14 and 15), then the sprites';
#define FLASH_COLORS (FLASH_BG + 0x100)
#define FRAME_PUFF 57           // The puff as the star's hit (0-16, NiDL's shared effect's), his light (NiDL's 67 down to 61, then 60);
#define FRAME_LIGHT 74
#define PUFF_TILES 40
#define LIGHT_TILES 121
#define INTRO_KEY_LIGHT 0xC000
#define ENTRANCE_X (113 << 8)   // Where he appears (NiDL's spot: the fight starts there);
#define ENTRANCE_Y (280 << 8)
#define ANIM_TUMBLING 74        // Kirby's tumble (KatAM's: 74, then 89 down to 75, round again: NiDL's 96, then 111 down to 97);
#define ANIM_SLIDE 42           // Kirby's back, the Star Rod held out (NiDL's 0x1009: the Star Rod's poses, KatAMExpansionHats.cs);
#define KNOCK_TUMBLE 154        // The tumble's and bounce's frames, then the slide's (to 258);
#define KNOCK_TURN 258
#define KNOCK_SPREAD 24         // The helpers' spread (pixels apart, over the tumble's first 64 frames);
#define KNOCK_SPREAD_TIME 64

// BlueStarLife; A blue star: from t `start`, at (x, y) on the screen (1/8 pixels, its middle), its size ((scale + 1) / 64) and turn
// (64ths), blinking out from `blink` frames on (hidden, shown, ...), gone after `end`; its moves: `runs` runs from `run`;
struct BlueStarLife {
    u16 start;
    s16 x, y;
    u8 scale, angle, blink, end, run, runs;
};

// BlueStarRun; `frames` frames of the same moves (1/8 pixels), size and turn steps (the size kept within 0-63);
struct BlueStarRun {
    u8 frames;
    s8 xs, ys, scale, angle;
};

// BlueStar; A blue star as it goes: where it is and was a frame ago (a burst follows it a frame behind), its size, turn, run and
// frames left of it;
struct BlueStar {
    s16 x, y, lastX, lastY;
    u8 scale, angle, run, left;
};

// The blue stars, traced from NiDL's (type 0x63 objects: their speeds and steps are its own; replayed and checked frame by frame);
// (12 lives)
static const struct BlueStarLife sBlueStars[12] = {
    { 562, 1312, 256, 8, 2, 21, 30, 0, 8 },
    { 613, 1952, 256, 8, 2, 21, 30, 8, 8 },
    { 614, 1568, 352, 5, 2, 56, 65, 16, 8 },
    { 638, 1872, 544, 62, 62, 36, 45, 24, 8 },
    { 638, 959, 928, 61, 62, 28, 37, 32, 6 },
    { 638, 1279, -224, 62, 2, 28, 37, 38, 5 },
    { 670, 1920, 640, 62, 62, 35, 68, 43, 8 },
    { 702, 1920, 96, 5, 2, 56, 65, 51, 14 },
    { 734, 1952, 648, 62, 62, 64, 63, 65, 8 },
    { 734, 2000, 448, 5, 34, 21, 30, 73, 8 },
    { 782, 1920, 288, 5, 34, 56, 65, 81, 14 },
    { 863, 1952, 16, 5, 6, 76, 93, 95, 6 },
};
// (101 runs)
static const struct BlueStarRun sBlueStarRuns[101] = {
    { 2, -32, -64, 5, 2 }, { 3, -32, -32, 5, 2 }, { 3, -32, -8, 5, 2 }, { 3, -32, -4, 5, 2 }, { 3, 0, 0, 0, 1 }, { 3, 0, 4, 0, 1 },
    { 3, 0, 8, 0, 1 }, { 10, -64, 16, 0, 1 }, { 2, -32, -64, 5, 2 }, { 3, -32, -32, 5, 2 }, { 3, -32, -8, 5, 2 }, { 3, -32, -4, 5, 2 },
    { 3, 0, 0, 0, 1 }, { 3, 0, 4, 0, 1 }, { 3, 0, 8, 0, 1 }, { 10, -64, 16, 0, 1 }, { 11, -32, 32, 2, 2 }, { 10, -32, 16, 2, 2 },
    { 8, -32, 8, 2, 2 }, { 8, -16, 4, 2, 2 }, { 8, -16, -4, 0, 1 }, { 6, -16, -8, 0, 1 }, { 4, -16, -16, 0, 1 }, { 10, -8, -32, 0, 1 },
    { 7, -48, 32, -1, -2 }, { 3, -32, 16, -1, -2 }, { 5, -48, -16, -1, -2 }, { 6, -32, -32, -1, -1 }, { 3, -48, -16, -1, -1 },
    { 6, -32, 16, -1, -1 }, { 5, -48, 32, -1, -1 }, { 10, -32, 8, -1, -1 }, { 11, -1, -32, -2, -2 }, { 4, -4, -16, -2, -1 },
    { 4, -4, 0, -2, -1 }, { 4, -8, 4, -2, -1 }, { 4, -8, 8, -2, -1 }, { 10, -8, 16, -2, -1 }, { 11, -1, 32, -1, 2 }, { 4, -4, 16, -1, 1 },
    { 4, -4, 0, -1, 1 }, { 4, -8, -4, -1, 1 }, { 14, -8, -8, -1, 1 }, { 7, -64, -64, -1, -2 }, { 7, -48, -32, -1, -2 },
    { 6, -32, -16, -1, -2 }, { 5, -24, -8, -1, -1 }, { 4, -16, -4, -1, -1 }, { 3, -10, 0, -1, -1 }, { 2, -6, 4, -1, -1 },
    { 34, -4, 8, 0, -1 }, { 3, -64, -32, 2, 2 }, { 4, -48, -16, 2, 2 }, { 4, -32, -8, 2, 2 }, { 4, -24, -4, 2, 2 }, { 4, -16, 0, 2, 2 },
    { 4, -10, 4, 2, 2 }, { 4, -8, 6, 2, 2 }, { 4, -6, 8, 2, 2 }, { 4, -4, 10, 0, 1 }, { 4, 0, 16, 0, 1 }, { 4, 4, 24, 0, 1 },
    { 4, 8, 32, 0, 1 }, { 4, 16, 32, 0, 1 }, { 14, 32, 32, 0, 1 }, { 7, -32, 8, -1, -2 }, { 12, -32, 4, -1, -2 }, { 4, -32, -4, -1, -2 },
    { 4, -32, -4, -1, -1 }, { 8, -32, -8, -1, -1 }, { 8, -32, -4, -1, -1 }, { 18, -32, 4, -1, -1 }, { 2, -32, 8, -1, -1 },
    { 2, -32, 64, 2, 2 }, { 3, -32, 32, 2, 2 }, { 3, -32, 8, 2, 2 }, { 3, -32, 4, 2, 2 }, { 3, 0, 0, 2, 2 }, { 3, 0, -4, 2, 2 },
    { 3, 0, -8, 2, 2 }, { 10, -64, -16, 2, 2 }, { 3, -64, 32, 2, 2 }, { 4, -48, 16, 2, 2 }, { 4, -32, 8, 2, 2 }, { 4, -24, 4, 2, 2 },
    { 4, -16, 0, 2, 2 }, { 4, -10, -4, 2, 2 }, { 4, -8, -6, 2, 2 }, { 4, -6, -8, 2, 2 }, { 4, -4, -10, 2, 2 }, { 4, 0, -16, 2, 2 },
    { 4, 4, -24, 2, 2 }, { 4, 8, -32, 2, 2 }, { 4, 16, -32, 2, 2 }, { 14, 32, -32, 2, 2 }, { 15, -32, 16, 2, 2 }, { 28, -32, 8, 2, 1 },
    { 8, 1, 8, 0, 1 }, { 8, 4, 8, 0, 1 }, { 16, 8, 8, 0, 1 }, { 18, 16, 16, 0, 1 },
};

// The bursts (NiDL's type 0x97): on a blue star (sBlueStars), at its age then;
static const u8 sBursts[][2] = { { 4, 12 }, { 5, 12 }, { 3, 16 }, { 6, 21 }, { 8, 24 }, { 11, 44 } };

// The tumble off the star and the bounce: runs of moves (frames, x and y a frame in 1/32 pixels), NiDL's Kirby's;
static const s8 sKnock[][3] = {
    { 12, 32, -128 }, { 4, 32, -64 }, { 4, 32, -32 }, { 4, 16, -16 }, { 4, 16, -4 }, { 4, 16, -1 }, { 4, 16, 1 }, { 4, 16, 4 },
    { 8, 16, 16 }, { 16, 16, 32 }, { 50, 16, 64 }, { 8, 0, -64 }, { 12, 0, -32 }, { 12, 0, 32 }, { 8, 0, 64 },
};

// A quarter turn's sines (64ths of a turn; 8.8);
static const u16 sQuarterSine[17] = { 0, 25, 50, 74, 98, 121, 142, 162, 181, 198, 213, 226, 237, 245, 251, 255, 256 };

// (calm.c's detours: filled in only with "Reduce Flashing" on);
struct NidlCalm {
    void (*updateScreenDma)(void);
    void (*updateScreenCpuSet)(void);
};
extern const struct NidlCalm gNidlCalm;

struct NightmareIntro {
    struct EffectObject e;      // (its sprite: the comet's);
    s32 camY;                   // The screen's top (8.8, the room's);
    s32 sx, sy;                 // The star on the screen (8.8, its middle: Kirby's spot on it);
    s32 cometX;                 // The comet's x (8.8, the room's);
    u16 t;                      // Frames since it began;
    u8 palA, palB, burstPal;    // The comet's palettes' slots (the blue star's is the first), the bursts' (0: not loaded yet);
    u8 level;                   // The flash on screen (0: none);
    u8 lightPal;                // His light's palette's slot (0: not loaded yet);
    s32 knockX, knockY;         // The knocked-off player's spot (8.8, the room's; the helpers' to his right), the star's then;
    s32 hitX, hitY;
    struct EffectObject *stars; // The blue stars' sprite, the bursts', the puff's, his light's (NULL: none);
    struct EffectObject *bursts[ARRAY_COUNT(sBursts)];
    struct EffectObject *puff, *light;
    struct BlueStar blue[ARRAY_COUNT(sBlueStars)];
    u16 colors[FLASH_COLORS];   // The palettes under the flash, the game's own;
};

static s32 Sine(u8 angle) {
    angle &= 63;
    if (angle < 16) return sQuarterSine[angle];
    if (angle < 32) return sQuarterSine[32 - angle];
    if (angle < 48) return -(s32)sQuarterSine[angle - 32];
    return -(s32)sQuarterSine[64 - angle];
}

// CometSpeed(); The comet's move at t (8.8, right): drifting left, slower, then off right;
static s32 CometSpeed(u16 t) {
    if (t <= 433) return -0x20;
    if (t <= 561) return -0x8;
    return t <= 577 ? 0x200 : 0x400;
}

// CometFrame(); Its frame at t: the dark orb, then each of the next five from every 36 frames on, two frames at a time with the one
// before it;
static u8 CometFrame(u16 t) {
    u8 k;

    if (t < 254) return 0;
    if (t >= 422) return 5;
    if (t >= 416) return 4;
    k = (t - 254) / 36 + 1;
    return (t - 254) & 2 ? k - 1 : k;
}

// FlashLevel(); The flash at t (0-3): brighter three times, each two frames on and two at the one below, then held and fading;
static u8 FlashLevel(u16 t) {
    u16 from;
    u8 high;

    if (t < 291 || t >= 423) return 0;
    if (t >= 419) return 1;
    if (t >= 415) return 2;
    if (t >= 397) return 3;
    if (t >= 361 && t < 365) return 2;
    if (t >= 365) {
        from = 365;
        high = 3;
    } else if (t >= 329) {
        from = 329;
        high = 2;
    } else {
        from = 291;
        high = 1;
    }
    // (with "Reduce Flashing", the one below, without the flicker);
    return gNidlCalm.updateScreenDma != NULL || (t - from) & 2 ? high - 1 : high;
}

/* FlashMap(); A flash level's channels (0-31): the room's toward white by 3/8, 3/4 or all the way, the sprites' toward black by 1/4,
 * 1/2 or all the way (level 0: as they are); */
static void FlashMap(u8 level, u8 *room, u8 *sprites) {
    u8 c;

    for (c = 0; c < 32; c++) {
        room[c] = level == 3 ? 31 : c + (((31 - c) * level * 3) >> 3);
        sprites[c] = level == 3 ? 0 : (c * (4 - level)) >> 2;
    }
}

#define FLASHED(color, map) ((map)[(color) & 0x1F] | (map)[((color) >> 5) & 0x1F] << 5 | (map)[((color) >> 10) & 0x1F] << 10)

/* Flash(); The palettes at a flash's level (0: as they are), as it changes, the game's own kept under it (a color that isn't what the
 * flash made of it is the game's, new); only on the screen of a player in the room; */
static void Flash(struct NightmareIntro *intro, u8 level) {
    u8 room[32], sprites[32], lastRoom[32], lastSprites[32];
    u16 i, *color;

    if (gKirbys[gLocalPlayerId].base.roomId != intro->e.roomId) {
        intro->level = 0;
        return;
    }
    if (level == intro->level) return;
    FlashMap(level, room, sprites);
    FlashMap(intro->level, lastRoom, lastSprites);
    for (i = 0; i < FLASH_COLORS; i++) {
        color = i >= FLASH_BG ? &gObjPalette[i - FLASH_BG] : &gBgPalette[i];
        if (!intro->level || *color != (i >= FLASH_BG ? FLASHED(intro->colors[i], lastSprites) : FLASHED(intro->colors[i], lastRoom)))
            intro->colors[i] = *color;
        *color = i >= FLASH_BG ? FLASHED(intro->colors[i], sprites) : FLASHED(intro->colors[i], room);
    }
    intro->level = level;
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE | MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

// IntroPalette(); A palette's slot, loaded under `key` as the frame `frame` of the anim in its low 12 bits has it (as
// NidlLoadPalette()), or 0 (not yet: the local player isn't in the room);
static u8 IntroPalette(u16 room, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

static void IntroSpriteMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);

    if (e->flags & 0x1000) TaskDestroy(gCurTask);
}

// IntroSprite(); A sprite of the intro's (`tiles` tiles of VRAM, frame `frame` first): a task of its own (KatAM frees its VRAM with it),
// drawn by the intro (IntroDraw()), gone with it (its flag 0x1000);
static struct EffectObject *IntroSprite(u16 room, u16 tiles, u8 frame) {
    struct Task *t = TaskCreate(IntroSpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = room;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_NIGHTMARE].animId, frame, 0x1A);
    return e;
}

// IntroDraw(); A sprite of the intro's at (x, y) (8.8, the room's), frame `frame`, palette `palId`;
static void IntroDraw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    e->flags = (e->flags & ~0x400) | 4 | 1;
    EffectObjectPostUpdate(e);
}

// Release(); A sprite of the intro's let go (its task ends);
static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

/* Spin(); The OAM pieces drawn since `first` (a blue star's 64x64 one, around its middle) turned and sized by affine matrix `matrix`,
 * as NiDL's: turn `angle` (64ths), size (scale + 1) / 64 (cos, -sin, sin, cos over the size; a flipped piece's row negated); */
static void Spin(u32 first, u8 angle, u8 scale, u32 matrix) {
    s32 c = Sine(angle + 16) * 64 / (scale + 1), s = Sine(angle) * 64 / (scale + 1);
    u16 *m = &gOamBuffer[matrix * 16 + 3];
    u32 attr1 = 0;

    for (; first < gUnk_030024F0; first++) {
        OamData *oam = &gUnk_030031C0[first];

        attr1 = oam->all.attr1;
        oam->all.attr0 |= 0x100;
        oam->all.attr1 = (attr1 & ~0x3E00) | (matrix << 9);
    }
    m[0] = attr1 & 0x1000 ? -c : c;
    m[4] = attr1 & 0x1000 ? s : -s;
    m[8] = attr1 & 0x2000 ? -s : s;
    m[12] = attr1 & 0x2000 ? -c : c;
}

// Comet(); The comet at t (shown till it's off the screen's right);
static void Comet(struct NightmareIntro *intro, u16 t) {
    u8 frame;

    if (t) intro->cometX += CometSpeed(t);
    if (t > COMET_GONE) return;
    frame = CometFrame(t);
    IntroDraw(&intro->e, intro->cometX, COMET_Y, FRAME_COMET + frame, frame < 3 ? intro->palA : intro->palB);
}

// BlueStars(); The blue stars at t: each moved on and drawn (from the one sprite, a matrix each);
static void BlueStars(struct NightmareIntro *intro, u16 t) {
    u8 i, drawn = 0;
    bool8 over = TRUE;

    for (i = 0; i < ARRAY_COUNT(sBlueStars); i++) {
        const struct BlueStarLife *life = &sBlueStars[i];
        struct BlueStar *b = &intro->blue[i];
        const struct BlueStarRun *run;
        s32 scale;
        u16 age;
        u32 first;

        if (t <= life->start + life->end) over = FALSE;
        if (t < life->start || t > life->start + life->end) continue;
        age = t - life->start;
        if (!age) {
            b->x = b->lastX = life->x;
            b->y = b->lastY = life->y;
            b->scale = life->scale;
            b->angle = life->angle;
            b->run = life->run;
            b->left = sBlueStarRuns[b->run].frames;
        } else {
            run = &sBlueStarRuns[b->run];
            b->lastX = b->x;
            b->lastY = b->y;
            b->x += run->xs;
            b->y += run->ys;
            scale = b->scale + run->scale;
            b->scale = scale < 0 ? 0 : scale > 63 ? 63 : scale;
            b->angle = (b->angle + run->angle) & 63;
            if (!--b->left && b->run + 1 < life->run + life->runs) b->left = sBlueStarRuns[++b->run].frames;
        }
        if ((age >= life->blink && !((age - life->blink) & 1)) || drawn >= INTRO_MATRICES) continue;
        if (intro->stars == NULL) intro->stars = IntroSprite(intro->e.roomId, BLUE_STAR_TILES, FRAME_BLUE_STAR);
        first = gUnk_030024F0;
        IntroDraw(intro->stars, INTRO_CAM_X + (b->x << 5), intro->camY + (b->y << 5), FRAME_BLUE_STAR, intro->palA);
        Spin(first, b->angle, b->scale, INTRO_MATRIX + drawn++);
    }
    // (its sprite let go once they're all gone);
    if (over) Release(&intro->stars);
}

// Bursts(); The bursts at t: on their star (a frame behind it from their second), a frame of the burst, then one of its sparkles,
// each in turn;
static void Bursts(struct NightmareIntro *intro, u16 t) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sBursts); i++) {
        const struct BlueStar *b = &intro->blue[sBursts[i][0]];
        u16 from = sBlueStars[sBursts[i][0]].start + sBursts[i][1], k;

        if (t == from + BURST_FRAMES) Release(&intro->bursts[i]);
        if (t < from || t >= from + BURST_FRAMES) continue;
        k = t - from;
        if (intro->bursts[i] == NULL) intro->bursts[i] = IntroSprite(intro->e.roomId, BURST_TILES, FRAME_BURST);
        IntroDraw(intro->bursts[i], INTRO_CAM_X + ((k ? b->lastX : b->x) << 5), intro->camY + ((k ? b->lastY : b->y) << 5),
                  FRAME_BURST + (k & 1 ? 12 + k / 2 : k / 2), intro->burstPal);
    }
}

/* Knock(); The knocked-off player's spot at t (from INTRO_HIT on, the star's spot then): NiDL's tumble up and down to the floor, the
 * bounce, then the slide back (a pixel right, a pixel right, two left, two left, ...) till he turns round; */
static void Knock(struct NightmareIntro *intro, u16 t) {
    u16 k = t - INTRO_HIT, n = 0;
    u8 r;

    if (!k) {
        intro->knockX = intro->hitX = INTRO_CAM_X + intro->sx;
        intro->knockY = intro->hitY = intro->camY + intro->sy;
    }
    if (k < KNOCK_TUMBLE) {
        for (r = 0; k >= n + sKnock[r][0]; r++) n += sKnock[r][0];
        intro->knockX += sKnock[r][1] << 3;
        intro->knockY += sKnock[r][2] << 3;
    } else if (k < KNOCK_TURN) {
        intro->knockX += (k - KNOCK_TUMBLE) & 2 ? -0x200 : 0x100;
    }
}

// Puff(); The puff where the star's hit (NiDL's: its burst and stars, two frames on its first, then out a while, then its rings);
static void Puff(struct NightmareIntro *intro, u16 t) {
    u16 a = t - INTRO_HIT;

    if (t < INTRO_HIT) return;
    if (a >= 22) {
        Release(&intro->puff);
        return;
    }
    if (a >= 12 && a < 16) return;
    if (intro->puff == NULL) intro->puff = IntroSprite(intro->e.roomId, PUFF_TILES, FRAME_PUFF);
    IntroDraw(intro->puff, intro->hitX, intro->hitY, FRAME_PUFF + (a < 2 ? 0 : a < 12 ? a - 1 : a - 5), intro->burstPal);
}

// Light(); His light where he appears, growing into his shape, two frames each (then the fight starts, he's there);
static void Light(struct NightmareIntro *intro, u16 t) {
    if (t < INTRO_LIGHT) return;
    if (intro->light == NULL) intro->light = IntroSprite(intro->e.roomId, LIGHT_TILES, FRAME_LIGHT);
    IntroDraw(intro->light, ENTRANCE_X, ENTRANCE_Y, FRAME_LIGHT + ((t - INTRO_LIGHT) >> 1), intro->lightPal);
}

// IntroEnd(); Its effects gone, the palettes back;
static void IntroEnd(struct NightmareIntro *intro) {
    u8 i;

    Flash(intro, 0);
    Release(&intro->stars);
    for (i = 0; i < ARRAY_COUNT(sBursts); i++) Release(&intro->bursts[i]);
    Release(&intro->puff);
    Release(&intro->light);
}

static void NightmareIntroMain(void);

// IntroOf(); Nightmare's intro in a room (its task), or NULL;
static struct NightmareIntro *IntroOf(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct NightmareIntro *intro;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != NightmareIntroMain) continue;
        intro = TaskGetStructPtr(t);
        if (intro->e.roomId == room && !(intro->e.flags & 0x1000)) return intro;
    }
    return NULL;
}

// NightmareIntroOn(); Whether his intro runs in a room, or a Kirby rides a warp star in there (it's coming: nidl_boss.c, he waits,
// hidden);
bool32 NightmareIntroOn(u16 room) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == room && gKirbys[i].stateFn == sub_080534D0) return TRUE;
    return IntroOf(room) != NULL;
}

// NightmareRideSize(); The size the star and its Kirbys are drawn at (finale.c's NidlFinaleRideSize()): theirs, hidden over the
// screen, or the Kirbys alone (the star's hit);
u32 NightmareRideSize(u16 room) {
    struct NightmareIntro *intro = IntroOf(room);

    if (intro == NULL) return 0;
    if (intro->t >= INTRO_HIT) return NIDL_RIDE_ALONE;
    return intro->sy <= INTRO_OFF ? NIDL_RIDE_HIDDEN : 0;
}

// IntroRider(); Whether a Kirby rides the star in (on its flight's step held: warpstar.c);
static bool32 IntroRider(struct Kirby *kirby, u16 room) {
    const struct Unk_08353510 *step = kirby->unk114;

    return kirby->base.roomId == room && kirby->stateFn == sub_080534D0 && step != NULL && step->unk8 == NIDL_RIDE_FINALE && !(step->unk9 & 0x40);
}

static void NightmareIntroMain(void) {
    struct NightmareIntro *intro = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &intro->e;
    u16 t = intro->t;
    u8 i;

    if (e->flags & 0x1000) {
        IntroEnd(intro);
        TaskDestroy(gCurTask);
        return;
    }
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys || t > INTRO_ENDS) {
        IntroEnd(intro);
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    if (t) {
        u8 code = sIntroStar[t - 1 < ARRAY_COUNT(sIntroStar) ? t - 1 : ARRAY_COUNT(sIntroStar) - 1];

        if (t - 1 >= ARRAY_COUNT(sIntroStar)) code = 0x88;
        if (t <= INTRO_CAM_FAST || ((t - INTRO_CAM_FAST) & 3) == 3) intro->camY += 0x100;
        if (intro->camY > INTRO_CAM_END) intro->camY = INTRO_CAM_END;
        intro->sx += ((code & 0xF) - 8) << 8;
        intro->sy += ((code >> 4) - 8) << 8;
    }
    NidlFinaleHoldCamera(e->roomId, INTRO_CAM_X, intro->camY);
    if (t >= INTRO_HIT) Knock(intro, t);
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];
        u16 k = t - INTRO_HIT;

        if (!IntroRider(kirby, e->roomId)) continue;
        if (t < INTRO_HIT) {
            kirby->base.x = INTRO_CAM_X + intro->sx;
            kirby->base.y = intro->camY + (intro->sy <= INTRO_OFF ? INTRO_ON_SCREEN : intro->sy);
            continue;
        }
        // (knocked off: the player's spot, the helpers' to his right; tumbling, then his back to us, facing right as NiDL's);
        kirby->base.x = intro->knockX + i * KNOCK_SPREAD * (k < KNOCK_SPREAD_TIME ? k : KNOCK_SPREAD_TIME) * 0x100 / KNOCK_SPREAD_TIME;
        kirby->base.y = intro->knockY;
        kirby->animationIndex = k < KNOCK_TUMBLE ? ANIM_TUMBLING + ((13 - k) & 15) : ANIM_SLIDE;
        kirby->base.flags &= ~1;
        // (up: the flight's end, a frame standing, then theirs again);
        if (t == INTRO_STANDS) {
            kirby->unk114++;
            kirby->base.counter = ((const struct Unk_08353510 *)kirby->unk114)->unk4;
        }
    }
    // (its effects; their palettes once a player's in the room);
    if (!intro->palA) intro->palA = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_A, FRAME_COMET);
    if (!intro->palB) intro->palB = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_B, FRAME_COMET + 3);
    if (!intro->burstPal) intro->burstPal = IntroPalette(e->roomId, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0);
    if (!intro->lightPal) intro->lightPal = IntroPalette(e->roomId, gNidlData.bosses[NIDL_NIGHTMARE].animId | INTRO_KEY_LIGHT, FRAME_LIGHT);
    Comet(intro, t);
    Flash(intro, FlashLevel(t));
    BlueStars(intro, t);
    Bursts(intro, t);
    Puff(intro, t);
    Light(intro, t);
    intro->t++;
}

// NightmareIntroRider(); A Kirby the star brings into his room, held on it (finale.c's NidlFinaleHold()): the intro, once;
void NightmareIntroRider(struct Kirby *kirby) {
    struct Task *t;
    struct NightmareIntro *intro;
    u8 i;

    if (IntroOf(kirby->base.roomId) != NULL) return;
    t = TaskCreate(NightmareIntroMain, sizeof(struct NightmareIntro), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    intro = TaskGetStructPtr(t);
    ClearEffectObject(&intro->e);
    intro->e.header.kind = 3;
    intro->e.roomId = kirby->base.roomId;
    intro->e.parent = NULL;
    intro->e.flags |= 0x400;
    intro->camY = INTRO_CAM_Y;
    intro->sx = INTRO_STAR_X;
    intro->sy = INTRO_STAR_Y;
    intro->t = 0;
    intro->cometX = COMET_X;
    intro->palA = intro->palB = intro->burstPal = intro->lightPal = 0;
    intro->level = 0;
    intro->knockX = intro->knockY = intro->hitX = intro->hitY = 0;
    intro->stars = intro->puff = intro->light = NULL;
    for (i = 0; i < ARRAY_COUNT(sBursts); i++) intro->bursts[i] = NULL;
    EffectObjectInitSprite(&intro->e, &intro->e.sprite, COMET_TILES, gNidlData.bosses[NIDL_NIGHTMARE].animId, FRAME_COMET, 0x1A);
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

// NightmareInit(); His boxes (his collar and body), through anything; his spot, where his intro has him appear;
void NightmareInit(struct NidlObject *n) {
    struct Nightmare *m = (struct Nightmare *)n;
    struct Object *obj = &n->obj;

    m->started = FALSE;
    m->hidden = FALSE;
    m->anim = 0;
    m->lastMove = 0xFF;
    obj->base.x = ENTRANCE_X;
    obj->base.y = ENTRANCE_Y;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -20, -40, 20, 32);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
}

/* His defeat (NightmareEnd(), as KatAM's explosion bursts him: finale.c's NightmareBeaten()): NiDL's, traced frame by frame (his
 * object's frames and spots, the light over him, the floor's bursts), run by a task of its own (KatAM's explosion takes him away,
 * as its bosses'), the Kirbys free (NiDL's walks and jumps about with the Star Rod), drawn mirrored as NiDL draws him through it:
 * • DEFEAT_BACK frames after he bursts (as the Power Orb's end), he flies off right from where he was (NiDL's 68-71, 3 frames
 *   each), 16 frames each at 4, 2, 1, 1/2 and 1/4 pixels a frame; there he turns to light (60 up to 67) and comes back as light at
 *   DEFEAT_SPOT (66 down to 60), caped (50, 51, his top half over them: 96, as NiDL draws his body frames), then spreads out and
 *   breaks up, blinking (76-88: sDefeat, from his light on, NiDL's
 *   frame or NONE and how long), gone 535 frames after the hit; a light over him meanwhile (sDefeatLight: 0x08754738's 13-17);
 * • the floor's bursts (sFloorBursts: when, from his flight's start, and where; NiDL's shared burst, as his intro's), then every
 *   Kirby still in his room is flown out (finale.c's NidlEndingFlight(): the fade to white, then the Fountain of Dreams hub);
 * the screen's held where the fight left it (NiDL's doesn't move); */
#define FRAME_FLEE 82           // His frames flying off (NiDL's 68-71: the first palette's pieces, then the second's, the third's);
#define FRAME_CAPED 94          // (50, 51: his third palette);
#define FRAME_SPREAD 96         // (76-80: the first palette's pieces, then the second's);
#define FRAME_BREAK 106         // (81-88: 81 his first palette, 82-88 his light's);
#define FRAME_GLOW 114          // The light over him (0x08754738's 13-17, his descriptor's first palette);
#define FRAME_TOP 119           // His top half over his caped body (96: the first palette's pieces, then the second's);
#define DEFEAT_KEY_GLOW 0xE000  // The light over him's palette's key (| his anim ID);
#define DEFEAT_BACK 24
#define DEFEAT_FLIGHT 80
#define DEFEAT_SPEEDS 16        // (frames at each speed flying off: 4, 2, 1, 1/2, 1/4 pixels a frame);
#define DEFEAT_REAPPEAR 16      // (frames into sDefeat: back, at DEFEAT_SPOT);
#define DEFEAT_SPOT_X (136 << 8)
#define DEFEAT_SPOT_Y (282 << 8)
#define DEFEAT_GLOW_FROM 227
#define DEFEAT_FLOOR_Y (352 << 8)
#define DEFEAT_OUT 840          // (from his flight's start: the Kirbys flown out);
#define DEFEAT_BURSTS 4         // (at once, at most);
#define DEFEAT_TILES LIGHT_TILES
#define DEFEAT_B_TILES 64
#define DEFEAT_C_TILES 72
#define GLOW_TILES 48
#define NONE 0xFF

static const u8 sDefeat[][2] = {
    { 60, 2 }, { 61, 2 }, { 62, 2 }, { 63, 2 }, { 64, 2 }, { 65, 2 }, { 66, 2 }, { 67, 4 }, { 66, 2 }, { 65, 2 }, { 64, 2 }, { 63, 2 },
    { 62, 2 }, { 61, 2 }, { 60, 2 }, { 50, 5 }, { 51, 10 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 },
    { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 }, { 77, 2 }, { 76, 2 },
    { 77, 2 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 },
    { NONE, 1 }, { 78, 2 }, { NONE, 1 }, { 79, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 },
    { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 80, 2 }, { NONE, 1 }, { 81, 2 }, { NONE, 1 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 },
    { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 82, 2 }, { NONE, 2 }, { 85, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 },
    { NONE, 2 }, { 83, 2 }, { NONE, 2 }, { 84, 2 }, { NONE, 2 }, { 86, 2 }, { NONE, 2 }, { 87, 2 }, { NONE, 2 }, { 88, 2 }, { NONE, 10 },
};

static const u8 sDefeatLight[][2] = {
    { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 },
    { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 16, 2 }, { NONE, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 }, { NONE, 1 }, { 14, 2 }, { 13, 2 },
    { NONE, 1 }, { 14, 4 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 },
    { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 },
    { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 14, 2 }, { NONE, 1 }, { 13, 2 }, { 15, 2 }, { 14, 2 }, { 13, 2 }, { 16, 2 }, { 17, 2 },
};

static const s16 sFloorBursts[][2] = {
    { 661, 88 }, { 691, 232 }, { 706, 216 }, { 706, 56 }, { 736, 200 }, { 746, 120 }, { 756, 40 }, { 766, 224 }, { 811, 88 }, { 811, 208 }, { 841, 232 },
};

extern u16 gUnk_02022E60[];     // The keys of the palettes loaded in OBJ slots 8-14 (KatAM's sub_0803DF24, sub_0803DFAC), and how many;
extern u8 gUnk_02022EA0;
void sub_0803D280(u8, u8);

/* DefeatPalette(); A palette of his defeat's, as frame `frame` of his anim has it: loaded into the slot of `take` (one of his intro's,
 * done with), else its slot if `key`'s loaded, else as the game loads one; `force` loads it again where it is (KatAM keeps 7 a
 * room, slots 8-14, and loads past them into the last: by his fight, his intro's and his own fill them, his second and third
 * palettes taking the last in turn, its colours the last's), or 0 (not yet: the local player isn't in the room); */
static u8 DefeatPalette(u16 room, u16 key, u8 frame, u16 take, bool8 force) {
    struct Sprite sprite;
    u8 i, slot = 0xFF;

    if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
    for (i = 0; i < gUnk_02022EA0 && slot == 0xFF && take; i++)
        if (gUnk_02022E60[i] == take) slot = i + 8, force = TRUE;
    for (i = 0; i < gUnk_02022EA0 && slot == 0xFF; i++)
        if (gUnk_02022E60[i] == key) slot = i + 8;
    if (slot == 0xFF) return IntroPalette(room, key, frame);
    if (force) {
        gUnk_02022E60[slot - 8] = key;
        SpriteSomething(&sprite, 0x6000000, key & 0xFFF, frame, 0xFF, 0, 0, 0, 0, 0x10, slot, 0x80000);
    }
    return slot;
}

struct NightmareEnd {
    struct EffectObject e;
    s32 x, y;                   // Where he is (8.8, the room's): where he burst, then flying off;
    s32 camX, camY;             // The screen, held where the fight left it (NiDL's doesn't move), 8.8;
    u16 t;                      // Frames since he burst;
    u16 run, left;              // sDefeat's run, and its frames left;
    u16 glowRun, glowLeft;      // sDefeatLight's;
    u8 palA, palB, palC, lightPal, glowPal, burstPal;
    struct EffectObject *layers[3], *glow, *bursts[DEFEAT_BURSTS];
};

// DefeatDraw(); A sprite of his defeat's at (x, y) (8.8, the room's), frame `frame`, palette `palId`, mirrored or not;
static void DefeatDraw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId, bool8 mirrored) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    // (KatAM draws effects mirrored unless they face right: NiDL draws him mirrored, its effects as they are);
    e->flags = (e->flags & ~0x401) | 4 | (mirrored ? 0 : 1);
    EffectObjectPostUpdate(e);
}

// DefeatLayer(); His sprite `which` (0 his first palette's pieces and the frames of one palette, 1 and 2 his second's and third's,
// 2 his caped body too), made as it's needed, in front of or behind the others as his own (his body in front of his collar, the
// cape's ends and his caped body behind);
static struct EffectObject *DefeatLayer(struct NightmareEnd *end, u8 which) {
    static const u8 sTiles[3] = { DEFEAT_TILES, DEFEAT_B_TILES, DEFEAT_C_TILES };
    static const u8 sOrder[3] = { 0x1B, 0x1A, 0x1C };

    if (end->layers[which] == NULL) {
        end->layers[which] = IntroSprite(end->e.roomId, sTiles[which], FRAME_FLEE);
        end->layers[which]->sprite.unk14 = sOrder[which] << 6;
    }
    return end->layers[which];
}

// DefeatFrame(); His frame `frame` (NiDL's) at (x, y): its palettes' pieces, each sprite one palette, those it hasn't let go;
static void DefeatFrame(struct NightmareEnd *end, u8 frame, s32 x, s32 y) {
    u8 i;

    if (frame >= 68 && frame <= 71) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_FLEE + frame - 68, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_FLEE + 4 + frame - 68, end->palB, TRUE);
        DefeatDraw(DefeatLayer(end, 2), x, y, FRAME_FLEE + 8 + frame - 68, end->palC, TRUE);
        return;
    }
    if (frame == 50 || frame == 51) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_TOP, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_TOP + 1, end->palB, TRUE);
        DefeatDraw(DefeatLayer(end, 2), x, y, FRAME_CAPED + frame - 50, end->palC, TRUE);
        return;
    }
    for (i = 1; i < 3; i++)
        if (!(frame >= 76 && frame <= 80 && i == 1)) Release(&end->layers[i]);
    if (frame >= 60 && frame <= 67) DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_LIGHT + 67 - frame, end->lightPal, TRUE);
    else if (frame >= 76 && frame <= 80) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_SPREAD + frame - 76, end->palA, TRUE);
        DefeatDraw(DefeatLayer(end, 1), x, y, FRAME_SPREAD + 5 + frame - 76, end->palB, TRUE);
    } else if (frame >= 81 && frame <= 88) {
        DefeatDraw(DefeatLayer(end, 0), x, y, FRAME_BREAK + frame - 81, frame == 81 ? end->palA : end->lightPal, TRUE);
    }
}

// DefeatBursts(); The floor's bursts at k (frames since his flight's start): each a frame of the burst, then one of its sparkles,
// in turn (as his intro's), a sprite of their own each (the next one's sprite after it: at most DEFEAT_BURSTS at once);
static void DefeatBursts(struct NightmareEnd *end, u16 k) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sFloorBursts); i++) {
        u16 a = k - sFloorBursts[i][0];
        struct EffectObject **e = &end->bursts[i % DEFEAT_BURSTS];

        if (k < sFloorBursts[i][0] || a > BURST_FRAMES) continue;
        if (a == BURST_FRAMES) {
            Release(e);
            continue;
        }
        if (*e == NULL) *e = IntroSprite(end->e.roomId, BURST_TILES, FRAME_BURST);
        DefeatDraw(*e, sFloorBursts[i][1] << 8, DEFEAT_FLOOR_Y, FRAME_BURST + (a & 1 ? 12 + a / 2 : a / 2), end->burstPal, FALSE);
    }
}

// DefeatGone(); Its sprites let go;
static void DefeatGone(struct NightmareEnd *end) {
    u8 i;

    for (i = 0; i < 3; i++) Release(&end->layers[i]);
    Release(&end->glow);
    for (i = 0; i < DEFEAT_BURSTS; i++) Release(&end->bursts[i]);
}

static void NightmareEndMain(void) {
    struct NightmareEnd *end = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &end->e;
    u16 animId = gNidlData.bosses[NIDL_NIGHTMARE].animId, k;
    u8 i;

    if (e->flags & 0x1000) {
        DefeatGone(end);
        TaskDestroy(gCurTask);
        return;
    }
    // (gone with the room: every Kirby left it);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys) {
        DefeatGone(end);
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    NidlFinaleHoldCamera(e->roomId, end->camX, end->camY);
    // (his palettes, his light's, his glow's and the bursts', once a player's in the room: his second loaded again, his third and
    // the glow's where his intro's comet's and blue star's were);
    if (!end->palA) end->palA = DefeatPalette(e->roomId, animId, FRAME_FLEE, 0, FALSE);
    if (!end->palC) end->palC = DefeatPalette(e->roomId, animId | NIDL_THIRD_PALETTE, FRAME_CAPED, animId | INTRO_KEY_A, FALSE);
    if (!end->palB) end->palB = DefeatPalette(e->roomId, animId | NIDL_SECOND_PALETTE, FRAME_FLEE + 4, 0, TRUE);
    if (!end->lightPal) end->lightPal = DefeatPalette(e->roomId, animId | INTRO_KEY_LIGHT, FRAME_LIGHT, 0, FALSE);
    if (!end->glowPal) end->glowPal = DefeatPalette(e->roomId, animId | DEFEAT_KEY_GLOW, FRAME_GLOW, animId | INTRO_KEY_B, FALSE);
    if (!end->burstPal) end->burstPal = IntroPalette(e->roomId, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0);
    k = end->t++;
    if (k < DEFEAT_BACK) return;
    k -= DEFEAT_BACK;
    // (flying off: 4 pixels a frame, half that every DEFEAT_SPEEDS frames);
    if (k < DEFEAT_FLIGHT) {
        end->x += 0x400 >> (k / DEFEAT_SPEEDS);
        DefeatFrame(end, 68 + (k / 3) % 4, end->x, end->y);
    } else if (end->run < ARRAY_COUNT(sDefeat)) {
        u8 frame = sDefeat[end->run][0];

        if (!end->left) end->left = sDefeat[end->run][1];
        if (k - DEFEAT_FLIGHT == DEFEAT_REAPPEAR) {
            end->x = DEFEAT_SPOT_X;
            end->y = DEFEAT_SPOT_Y;
        }
        if (frame != NONE) DefeatFrame(end, frame, end->x, end->y);
        if (!--end->left) end->run++;
    } else {
        for (i = 0; i < 3; i++) Release(&end->layers[i]);
    }
    // (the light over him);
    if (k >= DEFEAT_GLOW_FROM && end->glowRun < ARRAY_COUNT(sDefeatLight)) {
        u8 frame = sDefeatLight[end->glowRun][0];

        if (!end->glowLeft) end->glowLeft = sDefeatLight[end->glowRun][1];
        if (frame != NONE) {
            if (end->glow == NULL) end->glow = IntroSprite(e->roomId, GLOW_TILES, FRAME_GLOW);
            DefeatDraw(end->glow, DEFEAT_SPOT_X, DEFEAT_SPOT_Y, FRAME_GLOW + frame - 13, end->glowPal, FALSE);
        }
        if (!--end->glowLeft && ++end->glowRun >= ARRAY_COUNT(sDefeatLight)) Release(&end->glow);
    }
    DefeatBursts(end, k);
    if (k == DEFEAT_OUT) NidlEndingFlight(e->roomId);
}

// NightmareEnd(); He's beaten (finale.c's NightmareBeaten(), as KatAM's explosion bursts him): his defeat (see above);
void NightmareEnd(struct Object *obj) {
    struct Task *t = TaskCreate(NightmareEndMain, sizeof(struct NightmareEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct NightmareEnd *end = TaskGetStructPtr(t);
    u8 i;

    ClearEffectObject(&end->e);
    end->e.header.kind = 3;
    end->e.roomId = obj->base.roomId;
    end->e.parent = NULL;
    end->e.flags |= 0x400;
    end->x = obj->base.x;
    end->y = obj->base.y;
    end->camX = gCurLevelInfo[obj->base.unk56].viewportPosition.x;
    end->camY = gCurLevelInfo[obj->base.unk56].viewportPosition.y;
    end->t = end->run = end->left = end->glowRun = end->glowLeft = 0;
    end->palA = end->palB = end->palC = end->lightPal = end->glowPal = end->burstPal = 0;
    for (i = 0; i < 3; i++) end->layers[i] = NULL;
    end->glow = NULL;
    for (i = 0; i < DEFEAT_BURSTS; i++) end->bursts[i] = NULL;
}
