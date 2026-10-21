/* King Dedede; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3B (spawn 0x0809FC44), as NiDL boss
 * NIDL_KING_DEDEDE, Rainbow Resort's (room 0x313 here, the ring on the Fountain of Dreams, from NiDL's spot (232, 296)). His frames are
 * streamed frames with two palettes each (0x087538E0; the same two in every one), so his object draws the first palette's pieces and
 * a sprite follows it with the second's (DededePart), as Paint Roller's. Only the frames he uses are kept (sFrames: type 0x44's 256
 * frames are shared), by NiDL's numbers: 4-7 standing, 13-16 walking, 17 the hop of his dive, 18-19 diving on his belly, 24-27
 * inhaling, 28 puffed up with what he swallowed, 29 spitting it out, 39 raising the hammer, 36, 35 swinging it, 34 the slam, 41
 * crouching, 42 jumping, 43 falling; its frames (KatAMExpansion.cs) are those, the first palette's pieces, then the same, the second's.
 * What he does (NiDL's moves, by its pictures, timed off NiDL's fight): walks at Kirby, stands a while between moves, then dives at
 * him on his belly, or jumps high and comes down at him (stars burst out where he lands, which Kirby inhales and spits at him: KatAM's
 * bosses take only the stronger attacks), or inhales, or swings his hammer down, one to three times (the last slam knocks out a star).
 * Touching him hurts, as does his hammer as it comes down.
 * The inhale, as NiDL's (states 8 and 9, 0x080A11E0 and 0x080A13D4, traced in NiDL with Kirby in front of him): his mouth opens (24 for
 * 8 frames, 25 for 20), then he sucks (26 and 27, 4 and 6 frames) for up to 90 frames, sparkles flying into his mouth (0x080A00EC: 3
 * at once, NiDL's shared effects' 45); a Kirby in front of his mouth (NiDL's box object 0xB7, 40x28 pixels 38 ahead of him and 10
 * down) is caught (the player's and every helper there: KatAM's own grab, as its Gobbler's catch, sub_0804E09C) and drawn into his
 * mouth in 6 frames, hidden; he shuts it, puffed up (28), and shakes 2 pixels each way 4 times (2 frames each), still 14 frames more;
 * then he turns to the middle of the room and spits them out (29, for 51 frames) in a star (enemies/spit_star.c, 32 ahead of him on
 * the floor), which flies on until a wall, where they pop out hurt; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "kirby.h"
#include "constants/songs.h"

void sub_0804E09C(struct Kirby *);

#define FRAMES 24               // The second palette's pieces follow the first's;
#define PART_TILES 64
#define WALK_SPEED 0x100
#define DIVE_SPEED 0x300
#define DIVE_SLOW 0x14
#define JUMP_UP 0x500           // NiDL's: about 80 pixels high, 32 frames up;
#define GRAVITY 0x28
#define MAX_FALL 0x600
#define HOP_UP 0x200
#define CLOSE 56
#define STAR_SPEED 0x100
// The inhale (NiDL's): its box's middle ahead of him and down, its half size (40x28) with a Kirby's (8), the mouth he's drawn into;
#define CATCH_X 38
#define CATCH_Y 10
#define CATCH_W (20 + 8)
#define CATCH_H (14 + 8)
#define MOUTH_X 23
#define MOUTH_Y 10
#define OPEN_TIME 28            // 24 for 8 frames, 25 for 20;
#define SUCK_TIME 90
#define PULL_TIME 6
#define SHAKE_TIME 16
#define PUFF_TIME (SHAKE_TIME + 14)
#define SPIT_TIME 51
#define SPIT_X 32               // Where the star comes out: ahead of him, 8 above the floor (his feet 34 down);
#define SPIT_Y 26
#define SPARKLES 3
#define SPARKLE_ACCEL 0x60      // NiDL's 0x6000 (16.16) a frame, toward his middle;
#define SPARKLE_GONE 14
#define SPARKLE_FRAME 4         // Of the spit star's (enemies/spit_star.c): NiDL's shared effects' 45;

enum { STATE_IDLE, STATE_WALK, STATE_DIVE_HOP, STATE_DIVE, STATE_CROUCH, STATE_JUMP, STATE_LAND, STATE_INHALE, STATE_PULL, STATE_GULP,
       STATE_SPIT, STATE_RAISE, STATE_SWING };

struct KingDedede {
    struct NidlObject n;
    u8 state;
    u16 timer;
    u8 frame;
    u8 anim;
    u8 swings;
    u8 lastMove;
    bool8 started;
    bool8 airborne;
    u8 swallowed;       // The Kirbys inside him (a bit each);
    bool8 sucking;      // His inhale's sparkles fly (DededeSparkle);
};

// His frames by NiDL's numbers (KatAMExpansion.KingDedede's list);
static const u8 sFrames[44] = {
    [4] = 0, [5] = 1, [6] = 2, [7] = 3, [13] = 4, [14] = 5, [15] = 6, [16] = 7, [17] = 8, [18] = 9, [19] = 10, [24] = 11, [25] = 12,
    [26] = 13, [27] = 14, [28] = 15, [29] = 16, [34] = 17, [35] = 18, [36] = 19, [39] = 20, [41] = 21, [42] = 22, [43] = 23,
};

/* The grab's scripts (struct Kirby_110, kirby.c's sub_0804E60C: x ahead of him, y, priority, frames, Kirby's animation, flags: 0x400
 * held, 0x40 hidden, 0x200 left where he is, 0x100 back to the entry its frames count names): drawn into his mouth (Update() moves
 * him), then hidden inside him. A Kirby's script restarts when its holder's unk83 (his frame) changes: one entry, held, looping; */
static const struct Kirby_110 sDrawnIn[] = {
    { 0, 0, -1, 1, 0x4A, 0x600 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sSwallowed[] = {
    { MOUTH_X, MOUTH_Y, -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};

typedef char sKingDededeFits[sizeof(struct KingDedede) <= NIDL_OBJECT_SIZE ? 1 : -1];

// DededePart(); The second palette's pieces of his frame, following him (as KatAM's Kracko's eye: sub_080DD9B4);
static void DededePartMain(void) {
    struct Sprite sprite;
    struct EffectObject *part = TaskGetStructPtr(gCurTask);
    struct KingDedede *d = part->parent;
    struct Object *obj = &d->n.obj;
    u8 frame = sFrames[d->frame] + FRAMES;

    if (part->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (obj->base.flags & 0x1000) {
        part->flags |= 0x1000;
        return;
    }
    Macro_08107BA8_4(part, &part->sprite, &sprite, PART_TILES, &part->sprite);
    if (!part->sprite.palId) part->sprite.palId = NidlLoadPalette(&d->n, NidlGraphicsOf(&d->n)->animId | NIDL_SECOND_PALETTE, frame);
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

static void DededePart(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    struct Task *t = TaskCreate(DededePartMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *part = TaskGetStructPtr(t);

    ClearEffectObject(part);
    part->header.kind = 3;
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->parent = obj;
    part->roomId = obj->base.roomId;
    // In front of his first palette's pieces (his face and robe's trim: its priority lower, his is 0x6C0 >> 6);
    EffectObjectInitSprite(part, &part->sprite, PART_TILES, NidlGraphicsOf(&d->n)->animId, FRAMES, 0x1A);
    part->sprite.palId = NidlLoadPalette(&d->n, NidlGraphicsOf(&d->n)->animId | NIDL_SECOND_PALETTE, FRAMES);
}

// Facing(); His speed (or a distance) toward where he faces;
static s32 Facing(struct Object *obj, s32 speed) {
    return (obj->base.flags & 1) ? -speed : speed;
}

// Star(); A star (an inhalable one, enemies/star.c) knocked out along the floor, gone with him (as KatAM's Kracko's Waddle Does);
static void Star(struct KingDedede *d, s32 dx, s16 xspeed) {
    struct Object *obj = &d->n.obj, *star;

    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, (obj->base.x >> 8) + dx, (obj->base.y >> 8) + 24, 0, 0x1F, 0, 0, NIDL_TYPE,
                                   NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    star->base.xspeed = xspeed;
    star->base.yspeed = 0;
    star->base.parent = obj;
    star->unk7C = sub_0809F840;
    StarFree(star);
}

/* Catch(); A Kirby in front of his mouth goes into KatAM's grab (as Gobbler's catch: sub_080E588C; the grab's start, sub_0804E09C,
 * refuses a Kirby hurt, dead or busy, and clears its script then), to be drawn in; */
static void Catch(struct KingDedede *d, struct Kirby *kirby, u8 i) {
    struct Object *obj = &d->n.obj;

    kirby->unk110 = sDrawnIn;
    kirby->base.unk6C = obj;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    if (kirby->unk110) d->swallowed |= 1 << i;
}

// Inside(); Whether a Kirby he caught is still his (not let go: hurt again, or he was beaten);
static bool32 Inside(struct KingDedede *d, u8 i) {
    return (d->swallowed >> i & 1) && gKirbys[i].unk110 && gKirbys[i].base.unk6C == &d->n.obj;
}

// DrawIn(); The Kirbys he caught drawn into his mouth (a frame of PULL_TIME), then hidden inside him;
static void DrawIn(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    s32 mouthX = obj->base.x + Facing(obj, MOUTH_X << 8), mouthY = obj->base.y + (MOUTH_Y << 8);
    s32 left = PULL_TIME - d->timer;
    // (256 / the frames left: no division in this code);
    static const u8 sStep[PULL_TIME + 1] = { 0, 0, 128, 85, 64, 51, 43 };
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!Inside(d, i)) continue;
        if (left <= 1) {
            kirby->unk110 = sSwallowed;
            kirby->base.counter = ~obj->unk83;
            continue;
        }
        kirby->base.x += ((mouthX - kirby->base.x) * sStep[left]) >> 8;
        kirby->base.y += ((mouthY - kirby->base.y) * sStep[left]) >> 8;
    }
}

// Spit(); A star (enemies/spit_star.c) out of his mouth with the Kirbys inside him, along the floor;
static void Spit(struct KingDedede *d) {
    struct Object *obj = &d->n.obj, *star;
    u8 i, kirbys = 0;

    for (i = 0; i < gNumKirbys; i++)
        if (Inside(d, i)) kirbys |= 1 << i;
    d->swallowed = 0;
    PlaySfx(&obj->base, SE_FROSTY_SPIT_KIRBY);
    if (!kirbys) return;
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, (obj->base.x >> 8) + Facing(obj, SPIT_X), (obj->base.y >> 8) + SPIT_Y, 0, 0x1F,
                                   0, 0, NIDL_TYPE, NIDL_SPECIAL << 4 | 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    star->base.parent = obj;
    SpitStarCarry(star, kirbys, obj->base.flags & 1);
}

/* DededeSparkle(); A sparkle of his inhale (NiDL's 0x080A00EC: NiDL's shared effects' 45, from a spot ahead of him, 0x08748268,
 * drawn into his middle faster and faster; gone close to him, it starts again from another spot), while he sucks; */
static const s8 sSparkleX[16] = { 15, 20, 25, 30, 35, 40, 44, 44, 44, 44, 40, 35, 30, 25, 20, 15 };
static const s8 sSparkleY[16] = { -16, -16, -14, -14, -12, -12, -12, -5, 5, 12, 16, 16, 16, 16, 16, 16 };
// (0x10000 / where each starts, sSparkleX + 20: its y follows its x in, without a division);
static const u16 sSparkleInv[16] = { 1872, 1638, 1456, 1310, 1191, 1092, 1024, 1024, 1024, 1024, 1092, 1191, 1310, 1456, 1638, 1872 };

static void DededeSparkleMain(void) {
    struct EffectObject *sparkle = TaskGetStructPtr(gCurTask);
    struct KingDedede *d = sparkle->parent;
    struct Object *obj = &d->n.obj;
    s32 dx;

    if (sparkle->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if ((obj->base.flags & 0x1000) || !d->sucking) {
        sparkle->flags |= 0x1000;
        return;
    }
    if (Macro_0810B1F4(&obj->base) && !(sparkle->flags & 0x2000)) {
        EffectObjectDisplaySprite(sparkle);
        return;
    }
    // unk3C: its x from his middle (8.8, ahead), unk4: its speed, unk8: the spot it started from (sSparkleX, sSparkleY);
    dx = sparkle->unk3C < 0 ? -sparkle->unk3C : sparkle->unk3C;
    if (dx <= SPARKLE_GONE << 8) {
        sparkle->unk8 = Rand16() & 15;
        sparkle->unk3C = (sSparkleX[sparkle->unk8] + 20) << 8;
        sparkle->unk4 = 0;
        dx = sparkle->unk3C;
    }
    sparkle->unk4 -= SPARKLE_ACCEL;
    sparkle->unk3C += sparkle->unk4;
    sparkle->flags |= 4;
    sparkle->x = obj->base.x + Facing(obj, sparkle->unk3C);
    sparkle->y = obj->base.y + ((((sSparkleY[sparkle->unk8] * (dx >> 8) * sSparkleInv[sparkle->unk8]) >> 16) + 16) << 8);
    sparkle->sprite.variant = SPARKLE_FRAME;
    EffectObjectPostUpdate(sparkle);
}

static void DededeSparkle(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    struct Task *t = TaskCreate(DededeSparkleMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *sparkle = TaskGetStructPtr(t);

    ClearEffectObject(sparkle);
    sparkle->header.kind = 3;
    sparkle->x = obj->base.x;
    sparkle->y = obj->base.y;
    sparkle->parent = d;
    sparkle->roomId = obj->base.roomId;
    sparkle->unk3C = 0;
    // (the spit star's frames: its own anim, NiDL's shared palette 15);
    EffectObjectInitSprite(sparkle, &sparkle->sprite, 4, gNidlData.enemies[NIDL_SPIT_STAR].animId, SPARKLE_FRAME, 0x1A);
    sparkle->sprite.palId = NidlLoadPalette(&d->n, gNidlData.enemies[NIDL_SPIT_STAR].animId, SPARKLE_FRAME);
}

static void Set(struct KingDedede *d, u8 state) {
    d->state = state;
    d->timer = 0;
}

// Box(); His box, his hammer's head with it as it comes down (in front of him: NiDL's frames face right, he faces Kirby);
static void Box(struct KingDedede *d, bool8 hammer) {
    struct Object *obj = &d->n.obj;

    if (!hammer) sub_0803E2B0(&obj->base, -22, -22, 22, 34);
    else if (obj->base.flags & 1) sub_0803E2B0(&obj->base, -62, -22, 22, 34);
    else sub_0803E2B0(&obj->base, -22, -22, 62, 34);
}


// Choose(); His next move, never the same twice in a row: the dive, the jump, the inhale or the hammer;
static void Choose(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8;
    u8 move;

    NidlFaceKirby(obj);
    if ((dx < 0 ? -dx : dx) > CLOSE * 2 && (Rand16() & 1)) {
        Set(d, STATE_WALK);
        return;
    }
    do move = Rand16() & 3; while (move == d->lastMove);
    d->lastMove = move;
    switch (move) {
    case 0:
        Set(d, STATE_DIVE_HOP);
        break;
    case 1:
        Set(d, STATE_CROUCH);
        break;
    case 2:
        Set(d, STATE_INHALE);
        break;
    default:
        d->swings = 1 + Rand16() % 3;
        Set(d, STATE_RAISE);
        break;
    }
}

static void Update(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    s32 x = (obj->base.x >> 8) - (gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8), dx, dy;

    switch (d->state) {
    case STATE_IDLE:
        // NiDL's: 4-7, about 14 frames each, between his moves;
        d->frame = 4 + ((d->timer / 14) & 3);
        if (++d->timer >= 56) Choose(d);
        break;
    case STATE_WALK:
        d->frame = 13 + ((d->timer / 12) & 3);
        NidlFaceKirby(obj);
        obj->base.xspeed = Facing(obj, WALK_SPEED);
        dx = (obj->kirby3->base.x - obj->base.x) >> 8;
        if ((dx < 0 ? -dx : dx) < CLOSE || ++d->timer > 150 || (obj->base.unk62 & 3)) {
            obj->base.xspeed = 0;
            Choose(d);
        }
        break;
    case STATE_DIVE_HOP:
        // A hop forward, then on his belly along the floor, slowing;
        d->frame = 17;
        if (d->timer++ == 8) {
            obj->base.xspeed = Facing(obj, DIVE_SPEED);
            obj->base.yspeed = HOP_UP;
            d->airborne = TRUE;
            PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
        }
        if (d->airborne) {
            obj->base.yspeed -= GRAVITY;
            if (obj->base.yspeed <= 0 && (obj->base.unk62 & 4)) {
                obj->base.yspeed = 0;
                d->airborne = FALSE;
                Set(d, STATE_DIVE);
            }
        }
        break;
    case STATE_DIVE:
        d->frame = d->timer < 20 ? 18 : 19;
        if (obj->base.xspeed > DIVE_SLOW) obj->base.xspeed -= DIVE_SLOW;
        else if (obj->base.xspeed < -DIVE_SLOW) obj->base.xspeed += DIVE_SLOW;
        else obj->base.xspeed = 0;
        if (obj->base.unk62 & 3) obj->base.xspeed = 0;
        if (++d->timer >= 40) {
            obj->base.xspeed = 0;
            Set(d, STATE_IDLE);
        }
        break;
    case STATE_CROUCH:
        d->frame = 41;
        NidlFaceKirby(obj);
        if (++d->timer < 24) break;
        // Up high, to come down where Kirby is (about 64 frames in the air);
        dx = (obj->kirby3->base.x - obj->base.x) >> 8;
        obj->base.xspeed = dx * 4;
        obj->base.yspeed = JUMP_UP;
        d->airborne = TRUE;
        Set(d, STATE_JUMP);
        PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
        break;
    case STATE_JUMP:
        d->frame = obj->base.yspeed > 0 ? 42 : 43;
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (obj->base.yspeed <= 0 && (obj->base.unk62 & 4)) {
            obj->base.xspeed = obj->base.yspeed = 0;
            d->airborne = FALSE;
            // The landing knocks out a star each way;
            Star(d, -28, -STAR_SPEED);
            Star(d, 28, STAR_SPEED);
            PlaySfx(&obj->base, SE_BONKERS_HAMMER_ATTACK);
            Set(d, STATE_LAND);
        }
        break;
    case STATE_LAND:
        d->frame = 41;
        if (++d->timer >= 34) Set(d, STATE_IDLE);
        break;
    case STATE_INHALE:
        d->frame = d->timer < 8 ? 24 : d->timer < OPEN_TIME ? 25 : (d->timer - OPEN_TIME) % 10 < 4 ? 26 : 27;
        if (d->timer == OPEN_TIME) {
            u8 i;

            d->sucking = TRUE;
            for (i = 0; i < SPARKLES; i++) DededeSparkle(d);
            PlaySfx(&obj->base, SE_KIRBY_INHALE);
        }
        // A Kirby in front of his mouth is caught (every one there, the player's and the helpers'), and drawn in;
        if (d->timer >= OPEN_TIME) {
            s32 cx = obj->base.x + Facing(obj, CATCH_X << 8), cy = obj->base.y + (CATCH_Y << 8);
            u8 i;

            for (i = 0; i < gNumKirbys; i++) {
                struct Kirby *kirby = &gKirbys[i];

                if (kirby->base.roomId != obj->base.roomId || kirby->hp <= 0 || kirby->unk110) continue;
                dx = (kirby->base.x - cx) >> 8;
                dy = (kirby->base.y - cy) >> 8;
                if (dx > -CATCH_W && dx < CATCH_W && dy > -CATCH_H && dy < CATCH_H) Catch(d, kirby, i);
            }
            if (d->swallowed) {
                Set(d, STATE_PULL);
                break;
            }
        }
        if (++d->timer >= OPEN_TIME + SUCK_TIME) {
            d->sucking = FALSE;
            Set(d, STATE_IDLE);
        }
        break;
    case STATE_PULL:
        // (still sucking, as NiDL's, until they're inside);
        d->frame = (d->timer & 7) < 4 ? 26 : 27;
        DrawIn(d);
        if (++d->timer >= PULL_TIME) {
            d->sucking = FALSE;
            PlaySfx(&obj->base, SE_GOBBLER_EAT_KIRBY);
            Set(d, STATE_GULP);
        }
        break;
    case STATE_GULP:
        // Puffed up, he shakes;
        d->frame = 28;
        if (d->timer < SHAKE_TIME) obj->base.objBase54 = (d->timer / 2) & 1 ? 2 : -2;
        if (++d->timer >= PUFF_TIME) {
            // He turns to the middle of the room, and spits them out;
            if (x <= 120) obj->base.flags &= ~1;
            else obj->base.flags |= 1;
            Spit(d);
            Set(d, STATE_SPIT);
            d->frame = 29;
        }
        break;
    case STATE_SPIT:
        d->frame = 29;
        if (++d->timer >= SPIT_TIME) Set(d, STATE_IDLE);
        break;
    case STATE_RAISE:
        d->frame = 39;
        NidlFaceKirby(obj);
        if (++d->timer >= 24) Set(d, STATE_SWING);
        break;
    case STATE_SWING:
        // 36, 35, then the slam (34), held longer after the last one;
        d->frame = d->timer < 4 ? 36 : d->timer < 6 ? 35 : 34;
        if (d->timer == 6) {
            PlaySfx(&obj->base, SE_ABILITY_HAMMER_GROUND);
            if (d->swings == 1) Star(d, Facing(obj, 52), Facing(obj, STAR_SPEED));
        }
        Box(d, d->timer >= 4 && d->timer < 18);
        if (++d->timer >= (d->swings == 1 ? 32 : 18)) {
            Box(d, FALSE);
            if (--d->swings) Set(d, STATE_SWING);
            else Set(d, STATE_IDLE);
        }
        break;
    }
    // (the screen's sides: the arena holds Kirby there, not him);
    if ((x < 28 && obj->base.xspeed < 0) || (x > 212 && obj->base.xspeed > 0)) obj->base.xspeed = 0;
}

static void KingDededeMain(struct Object *obj) {
    struct KingDedede *d = (struct KingDedede *)obj;

    NidlUpdate(&d->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (his stars too); he takes it back;
    sub_08034E14(obj);
    d->anim++;
    if (!d->airborne) NidlFall(obj);
    Update(d);
    NidlSetFrame(&d->n, sFrames[d->frame]);
}

// KingDededeStart(); The fight starts;
void KingDededeStart(struct Object *obj) {
    struct KingDedede *d = (struct KingDedede *)obj;

    ObjectSetFunc(obj, -1, KingDededeMain);
    if (d->started) return;
    d->started = TRUE;
    NidlFaceKirby(obj);
    Set(d, STATE_WALK);
    DededePart(d);
}

// KingDededeInit(); His boxes (his body, 34 pixels down to his feet);
void KingDededeInit(struct NidlObject *n) {
    struct KingDedede *d = (struct KingDedede *)n;
    struct Object *obj = &n->obj;

    d->started = FALSE;
    d->airborne = FALSE;
    d->frame = 4;
    d->anim = 0;
    d->lastMove = 0xFF;
    d->swallowed = 0;
    d->sucking = FALSE;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    Box(d, FALSE);
    ObjectSetBounds(&obj->base, -12, -10, 12, 34);
}
