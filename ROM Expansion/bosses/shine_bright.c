/* Mr. Shine & Mr. Bright; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3F (spawn 0x080A19CC, states
 * 0x087484D4), as NiDL boss NIDL_SHINE_BRIGHT, Butter Building's (room 0x164 here, on NiDL's spot, (208, 104)). Two of them: one
 * fights on the ground while the other attacks from the sky, and every so often they trade places (NiDL turns night into day
 * then, its room's background; KatAM's room keeps its own). This object is the one on the ground (the one Kirby can hit; one HP
 * bar for both, as NiDL's), and the other is a sprite in the sky (ShinePart); Mr. Bright's face and hands are a sprite of their own
 * too (NiDL's own object over his flaming body, which has its own palette: a streamed frame). Its frames (KatAMExpansion.cs):
 * 0-8 Mr. Shine on the ground (NiDL's 0x08754210: 10-12 standing and landing, 13-14 jumping, 15-18 throwing), 9-14 the moon (4-9),
 * 15-38 Mr. Bright's body (0x0875412C's streamed 21-32 facing, 33-44 rolling), 39-50 his face and hands (4-15: 4-6 angry, 7-9
 * arms up, 10-11 crouching, 12 punching up, 13 back, 14 punching, 15 dizzy), 51-54 the sun (16-19, the last two shining), 55-62 the
 * crescent (0x08754210's 20-27), 63-64 the fireball (28-29), 65-66 the sun's beam (34-35, two each); the moon's falling stars are
 * enemies (enemies/star.c), which Kirby can inhale.
 * What they do (NiDL's scripts; 16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • On the ground both hop at Kirby (0x080A37AC: 1 a frame, 2 up, gravity 0x1500, 3 at most; then 10 frames down), two or three
 *   times, then attack:
 *   Mr. Shine throws a crescent (0x080A3A40: 15 frames standing, 20 holding it up, a lunge (4, 2, 1 for 3 frames each), the throw,
 *   back (1, 2, 4), 20 frames), which flies on, slows and comes back; or jumps high (0x080A36C8: 6 up, gravity 0x4000, 6 at most).
 *   Mr. Bright rolls at Kirby in flames (0x080A4350: 4.5 a frame) until a wall stops him, dizzy; or punches fireballs (0x080A4512);
 *   or jumps high.
 * • In the sky the moon drops falling stars at Kirby (Kirby inhales them and spits them back: KatAM's bosses take only the stronger
 *   attacks), the sun drifts over him and shines a beam straight down.
 * • Every 15 seconds (900 frames) the one on the ground flies up into the sky and the other comes down; Mr. Bright is on the
 *   ground first (it's night in NiDL). Touching the one on the ground hurts, as do the crescents, stars, fireballs and beams
 *   (attack objects, as the enemies' stars); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/songs.h"

#define FRAME_SHINE 0       // + 0-8: NiDL's 10-18;
#define FRAME_MOON 9
#define FRAME_BRIGHT 15     // + 0-11 facing, 12-23 rolling;
#define FRAME_FACE 39       // + NiDL's face - 4;
#define FRAME_SUN 51
#define FRAME_CRESCENT 55
#define FRAME_FIREBALL 63
#define FRAME_BEAM 65

#define SHINE(f) (FRAME_SHINE + (f) - 10)
#define FACE(f) (FRAME_FACE + (f) - 4)

#define PART_TILES 64
#define SHOT_TILES 16
#define BEAM_TILES 80

#define SKY_X 144
#define SKY_Y 28
#define SKY_LEFT 40
#define SKY_RIGHT 200
#define SWAP_TIME 900
#define HOP_SPEED 0x100
#define HOP_UP 0x200
#define HOP_GRAVITY 0x15
#define HOP_FALL 0x300
#define JUMP_UP 0x600
#define JUMP_GRAVITY 0x40
#define JUMP_FALL 0x600
#define ROLL_SPEED 0x480
#define FLY_SPEED 0x400
#define STAR_TIME 100
#define STAR_SPEED 0x200
#define BEAM_TIME 150
#define BEAM_CHARGE 30
#define BEAM_SHINE 40
#define BEAM_Y 16
#define CRESCENT_SPEED 0x300
#define CRESCENT_BRAKE 0x08
#define FIREBALL_SPEED 0x300

enum { SHINE_MR, BRIGHT_MR };   // Who's on the ground;
enum {
    STATE_HOP, STATE_LAND, STATE_THROW, STATE_JUMP, STATE_ROLL, STATE_DIZZY, STATE_PUNCH, STATE_FLY_UP, STATE_COME_DOWN,
};
enum { PART_SKY, PART_FACE };
// Shots: what flies, and how;
enum { SHOT_CRESCENT, SHOT_FIREBALL, SHOT_BEAM };

// Throw (0x080A3A40) and roll's lunges: speeds (pixels) for 3 frames each;
static const s8 sLunge[] = { 4, 2, 1, -1, -2, -4 };

struct ShineBright {
    struct NidlObject n;
    u8 who;             // On the ground;
    u8 state;
    u16 timer;
    u8 hops;
    u8 step;
    u8 face;            // Mr. Bright's face and hands (frame);
    u8 body;            // Its frame;
    u16 swap;
    s32 skyX;           // The one in the sky (pixels, on the screen);
    s32 skyY;
    u8 skyFrame;
    u16 skyTimer;
    bool8 started;
    bool8 airborne;
    u8 anim;
};

typedef char sShineBrightFits[sizeof(struct ShineBright) <= NIDL_OBJECT_SIZE ? 1 : -1];

struct Shot {
    struct ObjectBase base;
    u8 kind;
    u8 palId;
    u16 life;
    s16 speed;          // The crescent's, slowing and coming back;
};

static s32 ScreenX(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
}

static s32 ScreenY(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.y >> 8;
}

static u16 Key(struct ShineBright *s, u8 frame) {
    u16 animId = NidlGraphicsOf(&s->n)->animId;

    if (frame >= FRAME_BRIGHT && frame < FRAME_FACE) return animId | NIDL_SECOND_PALETTE;     // The flames' (streamed) palette;
    if (frame >= FRAME_FIREBALL && frame < FRAME_BEAM) return animId | 0x4000;
    if (frame >= FRAME_BEAM) return animId | 0x6000;
    return animId;
}

/* ShinePartMain(); The one in the sky, or Mr. Bright's face and hands (unk4): a sprite following the boss, as KatAM's Kracko's eye
 * (sub_080DD9B4), gone when the boss is; */
static void ShinePartMain(void) {
    struct Sprite sprite;
    struct EffectObject *part = TaskGetStructPtr(gCurTask);
    struct ShineBright *s = part->parent;
    struct Object *obj = &s->n.obj;
    u8 frame;
    bool8 hidden;

    if (part->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (obj->base.flags & 0x1000) {
        part->flags |= 0x1000;
        return;
    }
    if (part->unk4 == PART_SKY) {
        frame = s->skyFrame;
        hidden = FALSE;
    } else {
        frame = s->face;
        hidden = s->who != BRIGHT_MR || (obj->base.flags & 0x400);
    }
    Macro_08107BA8_4(part, &part->sprite, &sprite, PART_TILES, &part->sprite);
    // A new palette when the frame needs another one (the sky's changes hands);
    if (!part->sprite.palId || part->unk8 != Key(s, frame)) {
        part->unk8 = Key(s, frame);
        part->sprite.palId = NidlLoadPalette(&s->n, part->unk8, frame);
    }
    if (Macro_0810B1F4(&obj->base) && !(part->flags & 0x2000)) {
        EffectObjectDisplaySprite(part);
        return;
    }
    part->flags |= 4;
    part->flags = (part->flags & ~0x401) | (hidden ? 0x400 : 0);
    if (part->unk4 == PART_SKY) {
        part->x = (ScreenX(obj) + s->skyX) * 0x100;
        part->y = (ScreenY(obj) + s->skyY) * 0x100;
    } else {
        part->x = obj->base.x;
        part->y = obj->base.y;
        part->flags |= obj->base.flags & 1;
    }
    part->objBase54 = obj->base.objBase54;
    part->objBase55 = obj->base.objBase55;
    part->sprite.variant = frame;
    EffectObjectPostUpdate(part);
}

static void Part(struct ShineBright *s, u8 which, u8 frame) {
    struct Object *obj = &s->n.obj;
    struct Task *t = TaskCreate(ShinePartMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *part = TaskGetStructPtr(t);

    ClearEffectObject(part);
    part->header.kind = 3;
    part->unk4 = which;
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->parent = obj;
    part->roomId = obj->base.roomId;
    // The face in front of the body (a smaller priority is drawn over: the body's is 0x6C0 >> 6);
    EffectObjectInitSprite(part, &part->sprite, PART_TILES, NidlGraphicsOf(&s->n)->animId, frame, which == PART_FACE ? 0x1A : 0x1B);
    part->unk8 = Key(s, frame);
    part->sprite.palId = NidlLoadPalette(&s->n, part->unk8, frame);
}

/* ShotMain(); A crescent, falling star, fireball or beam: an attack object like the enemies' stars (as Whispy Woods' puffs), gone
 * after its time or off the screen; */
static void ShotMain(void) {
    struct Sprite sprite;
    struct Shot *shot = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &shot->base;
    struct LevelInfo *level;
    s32 sx, sy;

    if (ObjectPreUpdate(base)) return;
    base->flags |= 4;
    if (!(base->flags & 0x200)) SetPointerSomething(base);
    level = &gCurLevelInfo[base->unk56];
    sx = (base->x - level->viewportPosition.x) >> 8;
    sy = (base->y - level->viewportPosition.y) >> 8;
    if (++base->counter >= shot->life || sx < -32 || sx > 272 || sy < -48 || sy > 200) {
        base->flags |= 0x1000;
        return;
    }
    switch (shot->kind) {
    case SHOT_CRESCENT:
        // It flies on, slows and comes back (0x080A3B56);
        shot->speed -= CRESCENT_BRAKE;
        base->x += (base->flags & 1) ? -shot->speed : shot->speed;
        base->sprite.variant = FRAME_CRESCENT + ((base->counter >> 2) & 7);
        break;
    case SHOT_FIREBALL:
        base->x += (base->flags & 1) ? -FIREBALL_SPEED : FIREBALL_SPEED;
        base->sprite.variant = FRAME_FIREBALL + ((base->counter >> 2) & 1);
        break;
    default:
        base->sprite.variant = FRAME_BEAM + ((base->counter >> 1) & 1);
        break;
    }
    Macro_08107BA8_4(base, &base->sprite, &sprite, shot->kind == SHOT_BEAM ? BEAM_TILES : SHOT_TILES, &base->sprite);
    base->sprite.palId = shot->palId;
    sub_0806F8BC(base);
}

static struct Shot *Shoot(struct ShineBright *s, u8 kind, s32 x, s32 y, bool8 left) {
    static const u8 sFrames[] = { FRAME_CRESCENT, FRAME_FIREBALL, FRAME_BEAM };
    static const u16 sLives[] = { 150, 90, BEAM_SHINE };
    struct Object *obj = &s->n.obj;
    struct Task *t = TaskCreate(ShotMain, sizeof(struct Shot), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct Shot *shot = TaskGetStructPtr(t);
    struct ObjectBase *base = &shot->base;
    u8 frame = sFrames[kind];

    ClearObjectBase(base);
    base->header.kind = 2;
    base->x = x;
    base->y = y;
    base->parent = obj;
    base->counter = 0;
    base->roomId = obj->base.roomId;
    base->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(base)) base->flags |= 0x2000;
    base->unk63 = 1;
    base->flags |= 0x10000140 | (left ? 1 : 0);
    base->unk68 = 0x20000043;
    base->unk5C |= 0x80000;
    shot->kind = kind;
    shot->life = sLives[kind];
    shot->speed = CRESCENT_SPEED;
    shot->palId = NidlLoadPalette(&s->n, Key(s, frame), frame);
    if (kind == SHOT_BEAM) sub_0803E2B0(base, -8, 0, 8, 96);
    else sub_0803E2B0(base, -6, -6, 6, 6);
    ObjectSetBounds(base, 0, 0, 0, 0);
    ObjectBaseInitSprite(base, &base->sprite, kind == SHOT_BEAM ? BEAM_TILES : SHOT_TILES, NidlGraphicsOf(&s->n)->animId, frame,
                         obj->base.sprite.unk14 >> 6);
    base->sprite.palId = shot->palId;
    return shot;
}

// Sky(); The one in the sky: the moon drops falling stars at Kirby, the sun drifts over him and shines a beam down;
static void Sky(struct ShineBright *s) {
    struct Object *obj = &s->n.obj;
    s32 kirbyX = (obj->kirby3->base.x >> 8) - ScreenX(obj), kirbyY = (obj->kirby3->base.y >> 8) - ScreenY(obj), dx, dy, d;
    struct Object *star;

    ++s->skyTimer;
    if (s->state == STATE_FLY_UP || s->state == STATE_COME_DOWN) {
        s->skyTimer = 0;
        s->skyFrame = s->who == BRIGHT_MR ? FRAME_MOON + 2 : FRAME_SUN + ((s->anim >> 3) & 1);
        return;
    }
    if (s->who == BRIGHT_MR) {
        // The moon (Mr. Shine): it looks at Kirby; a falling star every STAR_TIME frames, at him;
        dx = kirbyX - s->skyX;
        s->skyFrame = FRAME_MOON + (dx > 24 ? 1 : dx < -24 ? 4 : 2);
        if (s->skyX < kirbyX && s->skyX < SKY_RIGHT && !(s->anim & 1)) s->skyX++;
        else if (s->skyX > kirbyX && s->skyX > SKY_LEFT && !(s->anim & 1)) s->skyX--;
        if (s->skyTimer < STAR_TIME) return;
        s->skyTimer = 0;
        dx = kirbyX - s->skyX;
        dy = kirbyY - s->skyY;
        d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (d < 1) d = 1;
        // An inhalable star (enemies/star.c), gone with the boss (as KatAM's Kracko's Waddle Does: sub_0809F840);
        star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, ScreenX(obj) + s->skyX, ScreenY(obj) + s->skyY + 12, 0, 0x1F, 0, 0, NIDL_TYPE,
                                       NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        star->base.xspeed = dx * STAR_SPEED / d;
        star->base.yspeed = -dy * STAR_SPEED / d;
        star->base.parent = obj;
        star->unk7C = sub_0809F840;
        PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
        return;
    }
    // The sun (Mr. Bright): over Kirby, it shines (its frames 18-19), then a beam straight down;
    if (s->skyTimer < BEAM_TIME - BEAM_CHARGE) {
        s->skyFrame = FRAME_SUN + ((s->anim >> 3) & 1);
        if (s->skyX < kirbyX && s->skyX < SKY_RIGHT) s->skyX++;
        else if (s->skyX > kirbyX && s->skyX > SKY_LEFT) s->skyX--;
        return;
    }
    s->skyFrame = FRAME_SUN + 2 + ((s->anim >> 2) & 1);
    if (s->skyTimer < BEAM_TIME) return;
    s->skyTimer = 0;
    Shoot(s, SHOT_BEAM, (ScreenX(obj) + s->skyX) * 0x100, (ScreenY(obj) + s->skyY + BEAM_Y) * 0x100, FALSE);
    PlaySfx(&obj->base, SE_KRACKO_LIGHTNING_ATTACK);
}

static void Set(struct ShineBright *s, u8 state) {
    s->state = state;
    s->timer = 0;
    s->step = 0;
}

static void Hop(struct ShineBright *s) {
    struct Object *obj = &s->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -HOP_SPEED : HOP_SPEED;
    obj->base.yspeed = HOP_UP;
    s->airborne = TRUE;
    Set(s, STATE_HOP);
    s->timer = 1;   // (airborne: its gravity is the hop's);
}

static void Jump(struct ShineBright *s) {
    struct Object *obj = &s->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -HOP_SPEED : HOP_SPEED;
    obj->base.yspeed = JUMP_UP;
    s->airborne = TRUE;
    Set(s, STATE_JUMP);
    PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
}

// Next(); After its hops: an attack (Mr. Shine: the crescent, or a high jump; Mr. Bright: the roll, punches or a high jump);
static void Next(struct ShineBright *s) {
    struct Object *obj = &s->n.obj;
    u16 r = Rand16() % 10;

    obj->base.xspeed = 0;
    NidlFaceKirby(obj);
    if (s->hops) {
        s->hops--;
        Hop(s);
        return;
    }
    s->hops = 1 + (Rand16() & 1);
    if (s->who == SHINE_MR) {
        if (r < 6) Set(s, STATE_THROW);
        else Jump(s);
    } else {
        if (r < 4) Set(s, STATE_ROLL);
        else if (r < 7) Set(s, STATE_PUNCH);
        else Jump(s);
    }
}

// Fall(); Its gravity in the air (the hop's or the jump's), until it lands;
static bool32 Fall(struct ShineBright *s, s16 gravity, s16 fall) {
    struct Object *obj = &s->n.obj;

    obj->base.yspeed -= gravity;
    if (obj->base.yspeed < -fall) obj->base.yspeed = -fall;
    if (obj->base.yspeed > 0 || !(obj->base.unk62 & 4)) return FALSE;
    obj->base.xspeed = obj->base.yspeed = 0;
    s->airborne = FALSE;
    return TRUE;
}

static void Update(struct ShineBright *s) {
    struct Object *obj = &s->n.obj;
    s32 x, left = ScreenX(obj);
    s8 lunge;

    switch (s->state) {
    case STATE_HOP:
        s->body = s->who == SHINE_MR ? SHINE(13) : FRAME_BRIGHT;
        s->face = FACE(7);
        if (Fall(s, HOP_GRAVITY, HOP_FALL)) Set(s, STATE_LAND);
        break;
    case STATE_LAND:
        s->body = s->who == SHINE_MR ? SHINE(12) : FRAME_BRIGHT;
        s->face = FACE(4) + ((s->anim >> 3) % 3);
        if (++s->timer >= 10) Next(s);
        break;
    case STATE_JUMP:
        s->body = s->who == SHINE_MR ? SHINE(13 + ((s->anim >> 3) & 1)) : FRAME_BRIGHT;
        s->face = FACE(7 + ((s->anim >> 3) % 3));
        if (Fall(s, JUMP_GRAVITY, JUMP_FALL)) {
            PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
            Set(s, STATE_LAND);
        }
        break;
    case STATE_THROW:
        // 0x080A3A40: standing 15, holding the crescent up 20, the lunge, the throw, back, 20;
        s->timer++;
        if (s->timer < 15) s->body = SHINE(11);
        else if (s->timer < 35) s->body = SHINE(15 + ((s->timer >> 3) & 1));
        else if (s->timer < 35 + 3 * ARRAY_COUNT(sLunge)) {
            s->body = s->timer < 44 ? SHINE(16) : SHINE(17);
            lunge = sLunge[(s->timer - 35) / 3];
            obj->base.xspeed = (obj->base.flags & 1) ? -lunge * 0x100 : lunge * 0x100;
            if (s->timer == 44) {
                Shoot(s, SHOT_CRESCENT, obj->base.x + ((obj->base.flags & 1) ? -0x1800 : 0x1800), obj->base.y - 0x800, obj->base.flags & 1);
                PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
            }
        } else {
            obj->base.xspeed = 0;
            s->body = SHINE(18);
            if (s->timer >= 35 + 3 * ARRAY_COUNT(sLunge) + 20) Next(s);
        }
        break;
    case STATE_ROLL:
        // 0x080A4350: crouching, then rolling at 4.5 until a wall (or the screen's side) stops him;
        if (++s->timer < 16) {
            s->body = FRAME_BRIGHT;
            s->face = FACE(10 + ((s->timer >> 3) & 1));
            break;
        }
        if (s->timer == 16) PlaySfx(&obj->base, SE_KIRBY_DASH);
        s->body = FRAME_BRIGHT + 12 + ((s->anim >> 1) % 12);
        s->face = FACE(13);
        obj->base.xspeed = (obj->base.flags & 1) ? -ROLL_SPEED : ROLL_SPEED;
        x = (obj->base.x >> 8) - left;
        if ((obj->base.unk62 & 3) || x < 20 || x > 220 || s->timer > 120) {
            // Dizzy, bounced back a little;
            obj->base.xspeed = (obj->base.flags & 1) ? HOP_SPEED : -HOP_SPEED;
            obj->base.yspeed = HOP_UP;
            s->airborne = TRUE;
            Set(s, STATE_DIZZY);
        }
        break;
    case STATE_DIZZY:
        s->body = FRAME_BRIGHT;
        s->face = FACE(15);
        if (s->airborne) {
            Fall(s, HOP_GRAVITY, HOP_FALL);
            break;
        }
        if (++s->timer >= 40) Next(s);
        break;
    case STATE_PUNCH:
        // 0x080A4512: three punches, a fireball each;
        s->body = FRAME_BRIGHT;
        s->timer++;
        if (s->timer < 20) {
            s->face = FACE(12);
        } else if (s->timer < 20 + 3 * 16) {
            s->face = ((s->timer - 20) & 15) < 8 ? FACE(14) : FACE(4);
            if (((s->timer - 20) & 15) == 0) {
                Shoot(s, SHOT_FIREBALL, obj->base.x + ((obj->base.flags & 1) ? -0x1800 : 0x1800), obj->base.y, obj->base.flags & 1);
                PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
            }
        } else {
            s->face = FACE(4);
            if (s->timer >= 20 + 3 * 16 + 20) Next(s);
        }
        break;
    case STATE_FLY_UP:
        // Trading places: up into the sky, off the screen;
        s->body = s->who == SHINE_MR ? SHINE(13) : FRAME_BRIGHT;
        s->face = FACE(7);
        obj->base.xspeed = 0;
        obj->base.yspeed = FLY_SPEED;
        if ((obj->base.y >> 8) - ScreenY(obj) < -48) {
            // The other one comes down from the sky, this one is up there now;
            obj->base.x = (left + s->skyX) * 0x100;
            obj->base.y = (ScreenY(obj) + s->skyY) * 0x100;
            s->skyX = SKY_X;
            s->who ^= 1;
            obj->base.sprite.palId = NidlLoadPalette(&s->n, Key(s, s->who == SHINE_MR ? FRAME_SHINE : FRAME_BRIGHT),
                                                     s->who == SHINE_MR ? FRAME_SHINE : FRAME_BRIGHT);
            obj->base.flags &= ~0x100;
            Set(s, STATE_COME_DOWN);
        }
        break;
    case STATE_COME_DOWN:
        s->body = s->who == SHINE_MR ? SHINE(13) : FRAME_BRIGHT;
        s->face = FACE(7);
        obj->base.yspeed = -FLY_SPEED / 2;
        if (obj->base.unk62 & 4) {
            obj->base.yspeed = 0;
            s->swap = 0;
            s->hops = 1;
            Set(s, STATE_LAND);
        }
        break;
    }
}

// ShineDefeated(); The pre-callback, every frame: once beaten (KatAM's defeat holds it, its main not called), dizzy;
static void ShineDefeated(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;

    if (obj->unk80 > 0) return;
    s->face = FACE(15);
}

static void ShineBrightMain(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;

    NidlUpdate(&s->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; it takes it back;
    sub_08034E14(obj);

    s->anim++;
    if (s->state != STATE_FLY_UP && s->state != STATE_COME_DOWN && !s->airborne && ++s->swap >= SWAP_TIME) {
        // Through the ceiling and floor while it flies (the room's top);
        obj->base.flags |= 0x100;
        Set(s, STATE_FLY_UP);
        PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
    }
    Update(s);
    Sky(s);
    // Mr. Bright's flames flicker (his facing frames, 3 frames each);
    if (s->who == BRIGHT_MR && s->body == FRAME_BRIGHT) s->body = FRAME_BRIGHT + ((s->anim / 3) % 12);
    NidlSetFrame(&s->n, s->body);
}

// ShineBrightStart(); The fight starts: Mr. Bright on the ground, the moon (Mr. Shine) in the sky;
void ShineBrightStart(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;

    ObjectSetFunc(obj, -1, ShineBrightMain);
    if (s->started) return;
    s->started = TRUE;
    obj->unk7C = ShineDefeated;
    s->swap = 0;
    s->hops = 1;
    s->skyTimer = 0;
    Set(s, STATE_LAND);
    // Mr. Bright's palette (his flames', a streamed frame's: the object made its sprite with frame 0's, Mr. Shine's);
    obj->base.sprite.palId = NidlLoadPalette(&s->n, Key(s, FRAME_BRIGHT), FRAME_BRIGHT);
    Part(s, PART_SKY, FRAME_MOON);
    Part(s, PART_FACE, FACE(4));
}

// ShineBrightInit(); Its boxes (its body, 24 pixels down to its feet), on the floor; Mr. Bright first, with his palette;
void ShineBrightInit(struct NidlObject *n) {
    struct ShineBright *s = (struct ShineBright *)n;
    struct Object *obj = &n->obj;

    s->who = BRIGHT_MR;
    s->body = FRAME_BRIGHT;
    s->face = FACE(4);
    s->skyX = SKY_X;
    s->skyY = SKY_Y;
    s->skyFrame = FRAME_MOON + 2;
    s->started = FALSE;
    s->airborne = FALSE;
    s->anim = 0;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -18, -20, 18, 22);
    ObjectSetBounds(&obj->base, -12, -12, 12, 24);
}
