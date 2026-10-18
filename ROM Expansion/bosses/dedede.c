/* King Dedede; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3B (spawn 0x0809FC44), as NiDL boss
 * NIDL_KING_DEDEDE, Rainbow Resort's (room 0x313 here, the ring on the Fountain of Dreams, from NiDL's spot (232, 296)). His frames are
 * streamed frames with two palettes each (0x087538E0; the same two in every one), so his object draws the first palette's pieces and
 * a sprite follows it with the second's (DededePart), as Paint Roller's. Only the frames he uses are kept (sFrames: type 0x44's 256
 * frames are shared), by NiDL's numbers: 4-7 standing, 13-16 walking, 17 the hop of his dive, 18-19 diving on his belly, 24-27
 * inhaling, 28 gulping, 39 raising the hammer, 36, 35 swinging it, 34 the slam, 41 crouching, 42 jumping, 43 falling; its frames
 * (KatAMExpansion.cs) are those, the first palette's pieces, then the same, the second's.
 * What he does (NiDL's moves, by its pictures, timed off NiDL's fight): walks at Kirby, stands a while between moves, then dives at
 * him on his belly, or jumps high and comes down at him (stars burst out where he lands, which Kirby inhales and spits at him: KatAM's
 * bosses take only the stronger attacks), or inhales (every Kirby in front of him, the player's and the helpers, is drawn in; one
 * that reaches his mouth is swallowed, held hidden inside him through KatAM's own grab, as its Gobbler's: sSwallowed; he gulps, then
 * spits them all out ahead of him, hurt, as Gobbler spits Kirby: sSpit), or swings his hammer down, one to three times (the last
 * slam knocks out a star). Touching him hurts, as does his hammer as it comes down; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "kirby.h"
#include "constants/songs.h"

void sub_0804E09C(struct Kirby *);

#define FRAMES 23               // The second palette's pieces follow the first's;
#define PART_TILES 64
#define WALK_SPEED 0x100
#define DIVE_SPEED 0x300
#define DIVE_SLOW 0x14
#define JUMP_UP 0x500           // NiDL's: about 80 pixels high, 32 frames up;
#define GRAVITY 0x28
#define MAX_FALL 0x600
#define HOP_UP 0x200
#define CLOSE 56
#define REACH 112               // The inhale's;
#define PULL 0xC0
#define STAR_SPEED 0x100
#define MOUTH 26                // How close to him a Kirby being drawn in is swallowed;
#define CHEW_TIME 48
#define SPIT_TIME 24
// His states for the grab (obj->unk83: a Kirby held follows its script while it stays; a new one restarts it, as with Gobbler's);
#define GRAB_HOLD 0xF0
#define GRAB_SPIT 0xF1

enum { STATE_IDLE, STATE_WALK, STATE_DIVE_HOP, STATE_DIVE, STATE_CROUCH, STATE_JUMP, STATE_LAND, STATE_INHALE, STATE_GULP, STATE_SPIT,
       STATE_RAISE, STATE_SWING };

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
    u8 state83;         // His own unk83, put back after a grab;
};

// His frames by NiDL's numbers (KatAMExpansion.KingDedede's list);
static const u8 sFrames[44] = {
    [4] = 0, [5] = 1, [6] = 2, [7] = 3, [13] = 4, [14] = 5, [15] = 6, [16] = 7, [17] = 8, [18] = 9, [19] = 10, [24] = 11, [25] = 12,
    [26] = 13, [27] = 14, [28] = 15, [34] = 16, [35] = 17, [36] = 18, [39] = 19, [41] = 20, [42] = 21, [43] = 22,
};

/* The grab's scripts (struct Kirby_110, kirby.c's sub_0804E60C: x ahead of him, y, priority, frames, Kirby's animation, flags: 0x400
 * held, 0x40 hidden, 0x100 back to the entry its frames count names, 0x10 thrown out at x, y * 0x10 speed, hurt): hidden inside him
 * (one entry: his state changes as he's hit, restarting it), then out at his mouth and spat. Both the same length, every entry held:
 * a Kirby switched to sSpit is checked at the entry it was at before its script restarts; */
static const struct Kirby_110 sSwallowed[] = {
    { 0, 0, -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sSpit[] = {
    { 24, 0, -1, 2, 0x4A, 0x400 },
    { 0x30, 0x10, 0, 1, 0, 0x410 },
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

// Star(); A star (an inhalable one, enemies/star.c) knocked out along the floor, gone with him (as KatAM's Kracko's Waddle Does);
static void Star(struct KingDedede *d, s32 dx, s16 xspeed) {
    struct Object *obj = &d->n.obj, *star;

    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, (obj->base.x >> 8) + dx, (obj->base.y >> 8) + 24, 0, 0x1F, 0, 0, NIDL_TYPE,
                                   NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    star->base.xspeed = xspeed;
    star->base.yspeed = 0;
    star->base.parent = obj;
    star->unk7C = sub_0809F840;
}

/* Swallow(); A Kirby that reached his mouth goes into KatAM's grab (as Gobbler's catch: sub_080E588C; the grab's start, sub_0804E09C,
 * refuses a Kirby hurt, dead or busy, and clears its script then); */
static void Swallow(struct KingDedede *d, struct Kirby *kirby, u8 i) {
    struct Object *obj = &d->n.obj;

    if (!d->swallowed) d->state83 = obj->unk83;
    obj->unk83 = GRAB_HOLD;
    kirby->unk110 = sSwallowed;
    kirby->base.unk6C = obj;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    if (kirby->unk110) {
        d->swallowed |= 1 << i;
        PlaySfx(&obj->base, SE_GOBBLER_EAT_KIRBY);
    }
}

// Spit(); The Kirbys inside him thrown out ahead of him (their scripts restart as his state changes);
static void Spit(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if ((d->swallowed >> i & 1) && gKirbys[i].unk110 == sSwallowed && gKirbys[i].base.unk6C == obj) gKirbys[i].unk110 = sSpit;
    obj->unk83 = GRAB_SPIT;
    PlaySfx(&obj->base, SE_FROSTY_SPIT_KIRBY);
}

// Held(); Whether a Kirby is still inside him, or being spat;
static bool32 Held(struct KingDedede *d) {
    struct Object *obj = &d->n.obj;
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if ((d->swallowed >> i & 1) && gKirbys[i].unk110 && gKirbys[i].base.unk6C == obj) return TRUE;
    return FALSE;
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

// Facing(); His speed toward where he faces;
static s16 Facing(struct Object *obj, s16 speed) {
    return (obj->base.flags & 1) ? -speed : speed;
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
        PlaySfx(&obj->base, SE_KIRBY_INHALE);
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
        d->frame = d->timer < 8 ? 24 : d->timer < 28 ? 25 : 26 + ((d->timer / 5) & 1);
        // Every Kirby in front of him is drawn in (as NiDL's: on the floor's level); one at his mouth is swallowed;
        if (d->timer >= 8) {
            u8 i;

            for (i = 0; i < gNumKirbys; i++) {
                struct Kirby *kirby = &gKirbys[i];

                if (kirby->base.roomId != obj->base.roomId || kirby->hp <= 0 || kirby->unk110 || (d->swallowed >> i & 1)) continue;
                dx = (kirby->base.x - obj->base.x) >> 8;
                dy = (kirby->base.y - obj->base.y) >> 8;
                if (!(obj->base.flags & 1 ? dx < 0 && dx > -REACH : dx > 0 && dx < REACH) || dy <= -40 || dy >= 40) continue;
                kirby->base.x += dx < 0 ? PULL : -PULL;
                if (dx > -MOUTH && dx < MOUTH) Swallow(d, kirby, i);
            }
        }
        if (!(d->timer & 15)) PlaySfx(&obj->base, SE_KIRBY_INHALE);
        if (++d->timer >= 120 || (d->swallowed && d->timer >= 40)) Set(d, d->swallowed ? STATE_GULP : STATE_IDLE);
        break;
    case STATE_GULP:
        // He gulps and chews, then spits them out;
        d->frame = d->timer < 12 || (d->timer / 8) & 1 ? 28 : 4;
        if (++d->timer >= CHEW_TIME) {
            Spit(d);
            Set(d, STATE_SPIT);
        }
        break;
    case STATE_SPIT:
        d->frame = 25;
        if (++d->timer >= SPIT_TIME && !Held(d)) {
            d->swallowed = 0;
            obj->unk83 = d->state83;
            Set(d, STATE_IDLE);
        }
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
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    Box(d, FALSE);
    ObjectSetBounds(&obj->base, -12, -10, 12, 34);
}
