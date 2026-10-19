/* Star Rod; NiDL's Star Rod flight, the Power Orb's fight (bosses/power_orb.c), for every Kirby in its room, the player's and the
 * helpers. NiDL's Kirby (traced frame by frame in its fight) holds the rod up and flies over the screen, facing right: 2 pixels a
 * frame the way the D-pad points (diagonals 1.41 each way), at once; let go, a quarter of that for 5 frames, then still; never past
 * the screen's x 16-224, y 18-132. A or B swings the rod (21 frames) and shoots a star on its 10th, 28 pixels ahead: it flies right at
 * 4 pixels a frame, spinning, with a sparkle behind it, until it hits something, where it bursts, or leaves the screen; held, it
 * shoots again every 31 frames; pressed again, once the swing ends. A hit knocks him back 2 pixels a frame, slowing, then half that,
 * for 10 frames (11 hurt).
 * Here the Star Rod is ability NIDL_ABILITY_STAR_ROD, which only this fight gives: its motions are NiDL's frames of Kirby with the
 * rod (KatAMExpansionMotions.cs; the rod on the hat sprite, as Light's lamp), its HUD card NiDL's. Each flying Kirby has a rider (a
 * task) that flies him: his state is StarRodHeld, which does nothing, and the rider moves him, plays his motions and shoots. KatAM's
 * own damage would drop his ability as a star (sub_080A9038, on every hit), so he's guarded from it while the boss is alive (flags
 * 0x8000, as KatAM's warp star rider and abilities.c's attacks), and the orb and its shots hit him themselves (StarRodHit(): NiDL's
 * knockback, 1 HP, KatAM's flashing after a hit as in its own last fight's shooter, sub_08052E2C); his HP at 0, KatAM's death takes
 * him (sub_0803FBB4), and the rider goes, as when any other state takes him (the shard's dance); back in the room, he flies again.
 * The helpers (CPU Kirbys) fly by the rider's own AI: a spot on the left, level with the orb or near it, shooting when in line;
 * KatAM's helper AI is kept still meanwhile (HoldAI()), and they're kept on the first player's screen (Viewer()). The fight's
 * screen sinks (NiDL's fall through space): everyone in it sinks with the camera's bounds (StarRodSink()). Kirby's ability before
 * is kept (STAR_ROD_SAVED) and given back after the shard's dance (StarRodRestore(), from NidlShardExit); */
#include "../abilities.h"
#include "kirby.h"
#include "code_0806F780.h"
#include "task.h"
#include "random.h"
#include "constants/songs.h"

void sub_08035E28(u8);
void sub_08035E9C(struct ObjectBase *);
void sub_0806EFF8(struct Kirby *);
void sub_08088640(struct Kirby *, u8, u16);
void sub_080880AC(struct Kirby *, u32);
void sub_0800EE04(u8, u32);

// The ability Kirby had before the Star Rod (his padding byte after NIDL_MIKE_USES), given back after the fight;
#define STAR_ROD_SAVED(kirby) (((u8 *)(kirby))[0x1A7])

#define SPEED 0x200
#define DIAGONAL 0x16A
#define SLOW_TIME 5
#define LEFT 16
#define RIGHT 224
#define TOP 18
#define BOTTOM 132
#define SWING_TIME 21
#define SHOOT_AT 10
#define REPEAT_TIME 31
#define END_TIME 5
#define HURT_TIME 11
#define STAR_X 28
#define STAR_SPEED 0x400
#define STAR_TILES 18        // (its burst's biggest frame: the star itself 16);
#define TRAIL_X 20
#define BURST_TIME 12
// The spit star's anim's frames (KatAMExpansion.SpitStar): the Star Rod's star 5-8, its trail 9-10, its burst 11-21;
#define FRAME_STAR 5
#define FRAME_TRAIL 9
#define FRAME_BURST 11

// The knockback's speed (NiDL's, 8.8) frame by frame;
static const u16 sKnockback[HURT_TIME] = { 0x1F8, 0x1F0, 0x1E8, 0x1E0, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0 };
// Its spin (each of its 4 frames for 3) and burst (NiDL's 0x0874C650 frames, one a frame, two sparkles crossing);
static const u8 sSpin[12] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3 };
static const u8 sBurst[BURST_TIME] = { 0, 0, 1, 6, 2, 7, 3, 8, 4, 9, 5, 10 };

struct StarRodRider {
    struct Kirby *kirby;
    struct Object *orb;     // The boss (the AI aims at it; NULL once it's beaten: StarRodFightOver());
    s32 lastTop;            // The camera bounds' top last frame (StarRodSink());
    s32 vx, vy;             // His speed (8.8);
    u16 shot;               // Frames since his last swing started;
    u8 motion;              // The motion he plays (STAR_ROD_MOTION_*);
    u8 way;                 // The way he flies in (its motion; 0: none) and its end's frames left;
    u8 ending;
    u8 slow;                // Frames of slowing down left, after letting go;
    u8 hurt;                // Frames of knockback left, and its way (1: right);
    bool8 hurtRight;
    bool8 guarded;          // KatAM's damage kept off him (while the boss is alive), and his attack flags before;
    s32 unk68;
    u8 aiTimer;             // The helpers' AI: frames to the next spot, the spot (screen pixels);
    s16 aiX, aiY;
};

struct StarRodStar {
    struct ObjectBase base;
    s32 lastTop;
    u8 palId;
    u8 burst;               // Frames since it hit (0: flying);
    u8 spin;
};

static void StarRodRiderMain(void);

// StarRodHeld(); A flying Kirby's state: his rider moves him;
void StarRodHeld(struct Kirby *kirby) {
    kirby->base.flags |= 4;
}

// Palette(); The spit star anim's palette (NiDL's palette 15), loaded once its room is the local player's;
static u8 Palette(struct ObjectBase *base, u8 frame) {
    u16 key = gNidlData.enemies[NIDL_SPIT_STAR].animId;
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != base->roomId) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId;
}

/* Viewer(); Whose camera holds a Kirby's fight on screen: a player's own; a helper's, the first player's, in its room (a helper's
 * own camera follows him, and comes back to the fight's screen slowly, as he comes back in after a fall); */
static u8 Viewer(u8 player, u16 roomId) {
    return player >= gNumHumanPlayers && gKirbys[0].base.roomId == roomId ? 0 : player;
}

/* StarRodSink(); The fight's screen sinks (the camera's bounds, bosses/power_orb.c): whoever is in it sinks as much, a little each
 * frame (other changes of the bounds, as the fight's camera lock begins and ends, are left alone); */
s32 StarRodSink(u8 player, s32 *lastTop) {
    s32 top = gCurLevelInfo[player].unk_S32Vec2_7C.y, drop = top - *lastTop;

    *lastTop = top;
    return drop > 0 && drop <= 0x200 ? drop : 0;
}

// StarRodTrail(); The sparkle behind a star (NiDL's shared 42 and 43, in turn), following it;
static void StarRodTrailMain(void) {
    struct EffectObject *trail = TaskGetStructPtr(gCurTask);
    struct StarRodStar *star = trail->parent;

    if (trail->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if ((star->base.flags & 0x1000) || star->burst) {
        trail->flags |= 0x1000;
        return;
    }
    trail->flags |= 4;
    trail->x = star->base.x - (TRAIL_X << 8);
    trail->y = star->base.y;
    trail->sprite.variant = FRAME_TRAIL + ((star->base.counter >> 1) & 1);
    trail->sprite.palId = star->palId;
    EffectObjectPostUpdate(trail);
}

static void StarRodTrail(struct StarRodStar *star) {
    struct Task *t = TaskCreate(StarRodTrailMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *trail = TaskGetStructPtr(t);

    ClearEffectObject(trail);
    trail->header.kind = 3;
    trail->x = star->base.x - (TRAIL_X << 8);
    trail->y = star->base.y;
    trail->parent = star;
    trail->roomId = star->base.roomId;
    EffectObjectInitSprite(trail, &trail->sprite, 4, gNidlData.enemies[NIDL_SPIT_STAR].animId, FRAME_TRAIL, 0x1A);
    trail->sprite.palId = star->palId;
}

/* StarRodStarMain(); A star: on right, spinning (3 frames a frame of 4), hitting what it touches (an attack of Kirby's, as his star
 * in KatAM's last fight, sub_0808466C: KatAM takes its strength, 1, off a boss, as NiDL's star does off its 60), then bursting; gone
 * off the screen; */
static void StarRodStarMain(void) {
    struct Sprite sprite;
    struct StarRodStar *star = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &star->base;
    s32 sx;

    if (ObjectPreUpdate(base)) return;
    base->flags |= 4;
    base->y += StarRodSink(Viewer(base->unk56, base->roomId), &star->lastTop);
    if (star->burst) {
        if (++star->burst > BURST_TIME) {
            base->flags |= 0x1000;
            return;
        }
        base->sprite.variant = FRAME_BURST + sBurst[star->burst - 1];
    } else if (base->flags & 0x40000) {
        // (it hit: still, bursting, hitting nothing more);
        star->burst = 1;
        base->flags |= 0x400;
        base->sprite.variant = FRAME_BURST + sBurst[0];
    } else {
        base->x += STAR_SPEED;
        sx = (base->x - gCurLevelInfo[Viewer(base->unk56, base->roomId)].viewportPosition.x) >> 8;
        if (sx > 240 + 32) {
            base->flags |= 0x1000;
            return;
        }
        if (++star->spin >= ARRAY_COUNT(sSpin)) star->spin = 0;
        base->sprite.variant = FRAME_STAR + sSpin[star->spin];
        SetPointerSomething(base);
    }
    ++base->counter;
    Macro_08107BA8_4(base, &base->sprite, &sprite, STAR_TILES, &base->sprite);
    base->sprite.palId = star->palId;
    sub_0806F8BC(base);
}

static void StarRodShoot(struct Kirby *kirby) {
    struct Task *t = TaskCreate(StarRodStarMain, sizeof(struct StarRodStar), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct StarRodStar *star = TaskGetStructPtr(t);
    struct ObjectBase *base = &star->base;

    ClearObjectBase(base);
    base->header.kind = 2;
    base->x = kirby->base.x + (STAR_X << 8);
    base->y = kirby->base.y;
    base->parent = kirby;
    base->counter = 0;
    base->roomId = kirby->base.roomId;
    base->unk56 = kirby->base.unk56;
    if (Macro_0810B1F4(base)) base->flags |= 0x2000;
    /* (as sub_0808466C's: its kind 0x40, which KatAM's hits need (sub_080365C8: a kind the target isn't immune to), and which adds
     * the target's unk94 to its strength, never under 1 (sub_0809AA10): the orb's -2, so 1; but through walls, no gravity; not its
     * 0x4000, VRAM shared by its anim and first frame, sub_0803DE54: its frames change); */
    base->flags |= 0xA0000000 | 0x140;
    base->unk68 |= 0x20000043;
    base->unk64 = 0x280;
    base->unk66 = 0x280;
    base->unk63 = 1;
    sub_0803E2B0(base, -8, -8, 8, 8);
    ObjectSetBounds(base, -8, -8, 8, 8);
    star->lastTop = gCurLevelInfo[Viewer(base->unk56, base->roomId)].unk_S32Vec2_7C.y;
    star->burst = 0;
    star->spin = 0;
    star->palId = Palette(base, FRAME_STAR);
    ObjectBaseInitSprite(base, &base->sprite, STAR_TILES, gNidlData.enemies[NIDL_SPIT_STAR].animId, FRAME_STAR, 0xA);
    base->sprite.palId = star->palId;
    StarRodTrail(star);
    PlaySfx(&kirby->base, SE_KIRBY_STARSHOT_1);
}

/* Give(); His ability, with its motions and hat (as an ability star gives one: sub_0806F260, sub_0806EFF8) and its HUD card (as
 * swallowing one shows it: sub_08035E28, sub_08035E9C); */
static void Give(struct Kirby *kirby, u8 ability) {
    kirby->ability = ability;
    if (gLocalPlayerId == kirby->base.unk56) sub_08035E28(ability);
    sub_08035E9C(&kirby->base);
    sub_0806F260(kirby);
    sub_0806EFF8(kirby);
}

/* HoldAI(); A helper's KatAM AI (gUnk_02038590) kept still while he flies (it would steer him for the floor, and the rider's own
 * flies him), its functions none; ReleaseAI(), started again as in a new room (sub_0800EE04) once he's down; */
static void AIStill(struct Unk_02038590 *ai) {}

static void HoldAI(struct Kirby *kirby) {
    u8 i;

    if (kirby->base.unk56 < gNumHumanPlayers) return;
    for (i = 0; i < 4; i++) gUnk_02038590[kirby->base.unk56].unk0[i] = AIStill;
}

static void ReleaseAI(struct Kirby *kirby) {
    if (kirby->base.unk56 >= gNumHumanPlayers) sub_0800EE04(kirby->base.unk56, 0);
}

// StarRodCanFly(); Whether a Kirby in the fight's room is free to fly (not dying, held, coming through a door...: sub_0804C7D0's);
bool32 StarRodCanFly(struct Kirby *kirby) {
    return !Macro_0810B1F4(&kirby->base) && kirby->hp > 0 && !kirby->unk110 && !(kirby->base.flags & 0x3800B00) && kirby->animationIndex != 39
        && kirby->stateFn != StarRodHeld && !(kirby->base.unkC & 0x8000);
}

// StarRodStart(); A Kirby takes the Star Rod and flies, his rider with him (bosses/power_orb.c, for each one free in its room);
void StarRodStart(struct Kirby *kirby, struct Object *orb) {
    struct Task *t = TaskCreate(StarRodRiderMain, sizeof(struct StarRodRider), 0x3400, TASK_USE_EWRAM, NULL);
    struct StarRodRider *r = TaskGetStructPtr(t);

    r->kirby = kirby;
    r->orb = orb;
    r->lastTop = gCurLevelInfo[Viewer(kirby->base.unk56, kirby->base.roomId)].unk_S32Vec2_7C.y;
    r->vx = r->vy = 0;
    r->shot = SWING_TIME + 1;
    r->way = r->ending = r->slow = r->hurt = 0;
    r->hurtRight = FALSE;
    r->aiTimer = 0;
    r->motion = STAR_ROD_MOTION_HOLD;
    if (kirby->ability != NIDL_ABILITY_STAR_ROD) {
        STAR_ROD_SAVED(kirby) = kirby->ability;
        Give(kirby, NIDL_ABILITY_STAR_ROD);
    }
    r->guarded = TRUE;
    r->unk68 = kirby->base.unk68;
    kirby->base.unk68 = 0x80;
    kirby->transitioningAbility = 0;
    kirby->unk110 = NULL;
    kirby->unk114 = NULL;
    kirby->base.flags &= ~(0x40000 | 1);
    kirby->base.flags |= 0x8000;
    kirby->base.unkC &= ~0x2000;
    kirby->base.xspeed = kirby->base.yspeed = 0;
    kirby->stateFn = StarRodHeld;
    NidlMotion(kirby, STAR_ROD_MOTION_HOLD);
    HoldAI(kirby);
}

// StarRodRestore(); His ability before the Star Rod, back (NidlShardExit: the shard's dance over, as he leaves the room);
void StarRodRestore(struct Kirby *kirby) {
    if (kirby->ability != NIDL_ABILITY_STAR_ROD) return;
    Give(kirby, STAR_ROD_SAVED(kirby) < NIDL_ABILITY_END && STAR_ROD_SAVED(kirby) != NIDL_ABILITY_STAR_ROD ? STAR_ROD_SAVED(kirby) : KIRBY_ABILITY_NORMAL);
}

// StarRodFightOver(); The boss beaten (NidlBossDefeat): the riders stop aiming at it;
void StarRodFightOver(struct Object *orb) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == StarRodRiderMain) {
            struct StarRodRider *r = TaskGetStructPtr(t);
            if (r->orb == orb) r->orb = NULL;
        }
    }
}

// RiderOf(); A flying Kirby's rider (NULL: none);
static struct StarRodRider *RiderOf(struct Kirby *kirby) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == StarRodRiderMain && ((struct StarRodRider *)TaskGetStructPtr(t))->kirby == kirby) return TaskGetStructPtr(t);
    }
    return NULL;
}

/* StarRodHit(); A flying Kirby hit by the boss or its shots (at x, 8.8): unless he's knocked back already or flashing after a hit
 * (unk5C & 7 of 4 or more, as KatAM's shooter's hits check), NiDL's knockback away from it, 1 HP, KatAM's hurt face on the HUD and
 * flashing (sub_08052E2C's); */
void StarRodHit(struct Kirby *kirby, s32 x) {
    struct StarRodRider *r;

    if (kirby->stateFn != StarRodHeld || kirby->hp <= 0 || (kirby->base.unk5C & 7) >= 4 || (kirby->base.unk5C & 0x80)) return;
    r = RiderOf(kirby);
    if (!r || r->hurt) return;
    r->hurt = HURT_TIME;
    r->hurtRight = x < kirby->base.x;
    PlaySfx(&kirby->base, SE_KIRBY_DMG_1);
    sub_08088640(kirby, 0x1C, 0x20);
    if (gUnk_0203AD10 & 0x60 || --kirby->hp > 0) sub_080880AC(kirby, 110);
}

// StarRodTouches(); Whether a box (its middle and half sizes, 8.8 and pixels) touches a flying Kirby (KatAM's shooter's box: 16
// wide, 6 up and 11 down);
bool32 StarRodTouches(struct Kirby *kirby, s32 x, s32 y, s32 halfWidth, s32 halfHeight) {
    s32 dx = (x - kirby->base.x) >> 8, dy = (y - kirby->base.y) >> 8;

    return kirby->stateFn == StarRodHeld && dx > -(halfWidth + 8) && dx < halfWidth + 8 && dy > -(halfHeight + 11) && dy < halfHeight + 6;
}

/* AI(); A helper's buttons (KatAM's key bits: A 1, B 2, right 0x10, left 0x20, up 0x40, down 0x80): to a spot, shooting in line;
 * the boss beaten, none: he stays where he is, clear of the shard in the screen's middle (the player's to take: the helpers don't
 * take it); */
static u16 AI(struct StarRodRider *r, s32 x, s32 y) {
    struct Kirby *kirby = r->kirby;
    struct LevelInfo *level = &gCurLevelInfo[Viewer(kirby->base.unk56, kirby->base.roomId)];
    s32 targetY;
    u16 keys = 0;

    if (!r->orb) return 0;
    targetY = (r->orb->base.y - level->viewportPosition.y) >> 8;
    if (!r->aiTimer--) {
        r->aiTimer = 40 + (Rand16() & 63);
        r->aiX = 24 + (Rand16() & 63) + kirby->base.unk56 * 8;
        r->aiY = targetY + (s16)(Rand16() & 31) - 16;
        if (r->aiY < TOP) r->aiY = TOP;
        if (r->aiY > BOTTOM) r->aiY = BOTTOM;
    }
    if (x < r->aiX - 4) keys |= 0x10;
    else if (x > r->aiX + 4) keys |= 0x20;
    if (y < r->aiY - 4) keys |= 0x80;
    else if (y > r->aiY + 4) keys |= 0x40;
    if (y > targetY - 24 && y < targetY + 24) keys |= 2;
    return keys;
}

// Way(); The way the D-pad points, as a motion (left first, then up, down, right, as NiDL's frames show); 0: none;
static u8 Way(u16 keys) {
    if (keys & 0x20) return STAR_ROD_MOTION_LEFT;
    if (keys & 0x40) return STAR_ROD_MOTION_UP;
    if (keys & 0x80) return STAR_ROD_MOTION_DOWN;
    if (keys & 0x10) return STAR_ROD_MOTION_RIGHT;
    return 0;
}

static void StarRodRiderMain(void) {
    struct StarRodRider *r = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = r->kirby;
    u8 viewer = Viewer(kirby->base.unk56, kirby->base.roomId);
    struct LevelInfo *level = &gCurLevelInfo[viewer];
    s32 x, y, dx, dy;
    u16 keys, pressed;
    u8 way, motion;

    if (Macro_0810B1F4(&kirby->base)) return;
    // (the boss beaten, or another state taking him: KatAM's damage back);
    if (r->guarded && (!r->orb || kirby->stateFn != StarRodHeld || kirby->hp <= 0)) {
        r->guarded = FALSE;
        kirby->base.flags &= ~0x8000;
        kirby->base.unk68 = r->unk68;
    }
    if (kirby->stateFn != StarRodHeld || kirby->hp <= 0) {
        ReleaseAI(kirby);
        TaskDestroy(gCurTask);
        return;
    }
    HoldAI(kirby);
    if (r->guarded) kirby->base.flags |= 0x8000;
    if (kirby->ability != NIDL_ABILITY_STAR_ROD) Give(kirby, NIDL_ABILITY_STAR_ROD);

    x = (kirby->base.x - level->viewportPosition.x) >> 8;
    y = (kirby->base.y - level->viewportPosition.y) >> 8;
    if (kirby->base.unk56 < gNumHumanPlayers) {
        keys = kirby->movementState;
        pressed = kirby->unk11A;
    } else {
        keys = AI(r, x, y);
        pressed = keys;
    }
    if (r->hurt) {
        // (knocked back: no flying, no shooting);
        r->vx = r->hurtRight ? sKnockback[HURT_TIME - r->hurt] : -sKnockback[HURT_TIME - r->hurt];
        r->vy = 0;
        keys = pressed = 0;
        r->way = r->ending = r->slow = 0;
        if (!--r->hurt) r->vx = 0;
    } else {
        dx = (keys & 0x10 ? 1 : 0) - (keys & 0x20 ? 1 : 0);
        dy = (keys & 0x80 ? 1 : 0) - (keys & 0x40 ? 1 : 0);
        if (dx || dy) {
            s32 speed = dx && dy ? DIAGONAL : SPEED;
            r->vx = dx * speed;
            r->vy = dy * speed;
            r->slow = SLOW_TIME;
        } else if (r->slow) {
            // (let go: a quarter of his speed a while, NiDL's);
            if (r->slow == SLOW_TIME) {
                r->vx /= 4;
                r->vy /= 4;
            }
            if (!--r->slow) r->vx = r->vy = 0;
        } else {
            r->vx = r->vy = 0;
        }
    }

    // The swing: pressed once the last one's over, or held every REPEAT_TIME frames; its star on its SHOOT_AT-th frame;
    if (r->shot < 0xFFFF) r->shot++;
    if (((pressed & 3) && r->shot > SWING_TIME) || ((keys & 3) && r->shot >= REPEAT_TIME)) r->shot = 0;
    if (r->shot == SHOOT_AT) StarRodShoot(kirby);

    // The motion: the swing; else the way he flies, its end played when he lets go or turns;
    way = Way(keys);
    if (r->ending && !--r->ending) r->way = 0;
    if (!r->ending && way != r->way) {
        if (r->way) r->ending = END_TIME;
        else r->way = way;
    }
    if (r->hurt) motion = STAR_ROD_MOTION_HURT;
    else if (r->shot < SWING_TIME) motion = STAR_ROD_MOTION_SHOOT;
    else if (r->ending) motion = r->way + STAR_ROD_MOTION_RIGHT_END - STAR_ROD_MOTION_RIGHT;
    else if (r->way) motion = r->way;
    else motion = STAR_ROD_MOTION_HOLD;
    r->motion = motion;
    NidlMotion(kirby, motion);

    // He moves (with the screen as it sinks), kept on it;
    kirby->base.x += r->vx;
    kirby->base.y += r->vy + StarRodSink(viewer, &r->lastTop);
    x = (kirby->base.x - level->viewportPosition.x) >> 8;
    y = (kirby->base.y - level->viewportPosition.y) >> 8;
    if (x < LEFT) kirby->base.x = level->viewportPosition.x + (LEFT << 8);
    if (x > RIGHT) kirby->base.x = level->viewportPosition.x + (RIGHT << 8);
    if (y < TOP) kirby->base.y = level->viewportPosition.y + (TOP << 8);
    if (y > BOTTOM) kirby->base.y = level->viewportPosition.y + (BOTTOM << 8);
    kirby->base.xspeed = kirby->base.yspeed = 0;
    kirby->base.flags &= ~1;
}
