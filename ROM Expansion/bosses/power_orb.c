/* Nightmare's Power Orb; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x42 (spawn 0x080AE3BC), as NiDL boss
 * NIDL_POWER_ORB, the Fountain of Dreams' first (room 0x003 here: the fall through space). As NiDL's, it's fought with the Star Rod
 * (abilities/star_rod.c: every Kirby in the room flies with it, shooting stars), at the top of the room, the screen sinking through
 * space as they fight (0.2 pixels a frame, 513 pixels down: ArenaSink(), its arena's screen; NiDL's camera, 0x0300215C, from 8 to
 * 521), the orb, its shots and the Kirbys keeping their places on it. Its frames (0x087544B4, mini-boss descriptor 37; only those
 * it uses: KatAMExpansion.PowerOrb): NiDL's 4-7, the spinning orb (10 frames each), 8-11 it shrinking (its look as it attacks; 11
 * blinks as it charges), then its shots' (its 20-29 and 39-42, their own tiles), from FRAME_SHOT.
 * What it does, traced frame by frame in NiDL's fight (its speeds change in steps it keeps exactly), from the right of the screen
 * ((192, 75) on it), round and round:
 * • it hovers (STATE_HOVER, 240 frames), bobbing 30 pixels up and down (speeds 1, 0.5, 0.125 pixels a frame, 10 frames each, 20 at
 *   1), shooting at times (+39, +100, +159, +232 frames);
 * • it roams (STATE_ROAM, 240 frames), a figure of eight over the right of the screen (x 174-210, y 36-116; a step every 6 frames),
 *   shooting at times (+48, +97, +176, +225);
 * • it charges (STATE_CHARGE, 120 frames), still and blinking: 40 frames in it fires 4 stars that spread into a column over the
 *   screen's right (their speeds halving every 14 frames) with a gap in its middle, wait, and 88 frames after they came out dash
 *   left at 6 pixels a frame, squashed, trailing sparkles;
 * • it zips (STATE_ZIP, 192 frames) up and down at 6 pixels a frame to rows 16, 112, 40, 136 and 76, at each lunging 20 pixels to
 *   the right and back (8 frames) as it shoots a crescent left at 6 pixels a frame.
 * Its timed shots come only 140 frames or more after the last (NiDL's cooldown, 0x080AF144), in turn: a stream of 3 stars left at 4
 * pixels a frame, 16 frames apart; a fan of 3 (left at 3, and 2.2 left 1.7 up or down); a pair (2 left, 2 up or down, for 16
 * frames, then left at 3). Its stars spin (3, 2, 1, 0, 4 frames each). The states come one after another: hover, then roam or
 * charge in turn; roam, then charge or zip in turn; charge, then roam; zip, then hover (NiDL's: hover, roam, charge, roam, zip,
 * hover, charge, roam...). Its shots hurt (attack objects, as the enemies' stars); so does touching it. NiDL's takes 1 HP a
 * Star Rod star of its 60; so does this one (KatAM's damage: the star's strength); */
#include "../nidl.h"
#include "../abilities.h"
#include "code_0806F780.h"
#include "constants/songs.h"

void ArenaSink(struct Object *, s32);

#define SPOT_X (192 << 8)
#define SPOT_Y (75 << 8)
#define SINK_SPEED 0x33
#define SINK_END (513 << 8)
#define COOLDOWN 140
#define STREAM_GAP 16
#define LUNGE_TIME 8
#define ZIP_WAIT 8
#define ZIP_SPEED 0x600
#define ATTACK_LOOK 16
// Its frames (KatAMExpansion.PowerOrb): the spin, the shrink, then the shots' (NiDL's shot frames 0-9, then 19-22, from FRAME_SHOT);
#define FRAME_SPIN 0
#define FRAME_SHRINK 4
#define FRAME_SMALL 7
#define FRAME_SHOT 8
#define SHOT_STAR 0         // 0-3, spinning (shown 3, 2, 1, 0);
#define SHOT_DASH 4         // 4-7, squashed as it dashes (7, 6, 5, 4, 2 frames each);
#define SHOT_CRESCENT 8     // 8-9, in turn;
#define SHOT_SPARKLE 10     // 10-13 (NiDL's 19-22), the sparkles a dashing star trails;
#define SHOT_TILES 8         // (its biggest frame, a crescent: 16x32);
#define SPARKLE_LIFE 16

enum { STATE_ENTER, STATE_HOVER, STATE_ROAM, STATE_CHARGE, STATE_ZIP };
enum { ATTACK_STREAM, ATTACK_FAN, ATTACK_PAIR, ATTACKS };
enum { KIND_STAR, KIND_PAIR, KIND_CHARGE, KIND_CRESCENT };

struct PowerOrb {
    struct NidlObject n;
    s32 sx, sy;         // Its spot on the fight's screen (8.8, from the screen's top left as it sinks);
    s32 sink;           // How far the screen has sunk (8.8);
    u16 timer;          // Frames into its state;
    u16 cooldown;       // Frames since its last timed shot;
    u8 state;
    u8 step, stepTimer; // The step of its moves (a table's) and frames left of it;
    u8 attack;          // Its next timed shot;
    u8 stream, streamTimer;
    u8 look;            // Frames left of its attack look;
    u8 turns;           // Bit 0: hover went on to charge last; bit 1: roam went on to zip last;
    u8 row;             // ZIP: the row it's going to, and that row's phase;
    u8 phase;
    u8 spin;            // Frames into its spin (4 frames, 10 each);
};

typedef char sPowerOrbFits[sizeof(struct PowerOrb) <= NIDL_OBJECT_SIZE ? 1 : -1];

struct OrbShot {
    struct ObjectBase base;
    s32 vx, vy;         // Its speed (8.8, y down);
    s32 lastTop;
    u8 kind;
    u8 palId;
    u8 side;            // A charge star's: its place in the column (0 outer, 1 inner) and way (y speed's sign);
    bool8 up;
};

// The hover: y speeds (8.8, down) and frames, from the middle up and round twice, back to the middle;
static const s16 sHoverSpeeds[] = { -0x100, -0x80, -0x20, 0x20, 0x80, 0x100, 0x80, 0x20, -0x20, -0x80, -0x100, -0x80, -0x20, 0x20, 0x80, 0x100, 0x80,
                                    0x20, -0x20, -0x80, -0x100 };
static const u8 sHoverTimes[] = { 10, 10, 10, 10, 10, 20, 10, 10, 10, 10, 20, 10, 10, 10, 10, 20, 10, 10, 10, 10, 10 };
static const u8 sHoverShots[] = { 39, 100, 159, 232 };
/* The roam: speeds (8.8, y down) of a figure of eight, 6 frames each but its crossings (12), from the middle going up left (half a
 * crossing first); round twice, back to the middle; */
static const s16 sRoamX[] = { -0x200, -0x100, 0, 0x100, 0x200, 0x200, 0x100, 0, -0x100, -0x200, -0x100, 0, 0x100, 0x200, 0x200, 0x100, 0, -0x100, -0x200 };
static const s16 sRoamY[] = { -0x200, -0x200, -0x140, -0x100, -0x80, 0x80, 0x100, 0x140, 0x200, 0x200, 0x200, 0x140, 0x100, 0x80, -0x80, -0x100, -0x140, -0x200, -0x200 };
static const u8 sRoamTimes[] = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 12, 6, 6, 6, 6, 6, 6, 6, 6, 12 };
static const u8 sRoamShots[] = { 48, 97, 176, 225 };
#define ROAM_LOOP 1
// The charge stars: their speeds (8.8) step by step, outer and inner (y up for the upper two), 14 frames each, then still;
static const s16 sChargeX[2] = { 0x80, 0x20 };
static const s16 sChargeY[2][4] = { { 0x200, 0x100, 0x80, 0x20 }, { 0x100, 0x80, 0x20, 0x08 } };
#define CHARGE_STEP 14
#define CHARGE_DASH 88
#define CHARGE_FIRE 40
// The zip's rows (screen y);
static const u8 sZipRows[] = { 16, 112, 40, 136, 76 };
// The lunge: x speeds (8.8) frame by frame;
static const s16 sLunge[LUNGE_TIME] = { 0x800, 0x800, 0x400, 0x400, -0x400, -0x400, -0x800, -0x800 };

static s32 ScreenLeft(struct Object *obj) {
    return (obj->objTemplate->x + obj->objTemplate->unk1A) << 8;
}

static s32 ScreenTop(struct PowerOrb *o) {
    struct Object *obj = &o->n.obj;
    return ((obj->objTemplate->y + obj->objTemplate->unk1C) << 8) + o->sink;
}

static u16 ShotKey(struct PowerOrb *o) {
    return NidlGraphicsOf(&o->n)->animId | NIDL_SECOND_PALETTE;
}

// OrbSparkle(); A sparkle a dashing star leaves (NiDL's objects 0xD3 and 0xD4): left at 5 pixels a frame, a little up or down;
static void OrbSparkleMain(void) {
    struct EffectObject *sparkle = TaskGetStructPtr(gCurTask);

    if (sparkle->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (++sparkle->unk4 > SPARKLE_LIFE) {
        sparkle->flags |= 0x1000;
        return;
    }
    sparkle->flags |= 4;
    sparkle->x -= 0x500;
    sparkle->y += sparkle->unk3C;
    sparkle->sprite.variant = FRAME_SHOT + SHOT_SPARKLE + ((sparkle->unk4 >> 1) & 3);
    EffectObjectPostUpdate(sparkle);
}

static void OrbSparkle(struct OrbShot *shot, bool8 up) {
    struct Task *t = TaskCreate(OrbSparkleMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *sparkle = TaskGetStructPtr(t);
    struct PowerOrb *o = shot->base.parent;

    ClearEffectObject(sparkle);
    sparkle->header.kind = 3;
    sparkle->x = shot->base.x;
    sparkle->y = shot->base.y;
    sparkle->parent = o;
    sparkle->roomId = shot->base.roomId;
    sparkle->unk4 = 0;
    sparkle->unk3C = up ? -0x80 : 0x80;
    EffectObjectInitSprite(sparkle, &sparkle->sprite, 4, NidlGraphicsOf(&o->n)->animId, FRAME_SHOT + SHOT_SPARKLE, 0x1A);
    sparkle->sprite.palId = shot->palId;
}

/* OrbShotMain(); A star or crescent: an attack object like the enemies' stars (as Mr. Shine's crescents), moving with the screen as
 * it sinks; gone off it, or with the orb; */
static void OrbShotMain(void) {
    struct Sprite sprite;
    struct OrbShot *shot = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &shot->base;
    struct PowerOrb *o = base->parent;
    struct LevelInfo *level = &gCurLevelInfo[base->unk56];
    s32 sx, sy;
    u16 t;

    if (ObjectPreUpdate(base)) return;
    base->flags |= 4;
    if ((o->n.obj.base.flags & 0x1000) || o->n.obj.unk80 <= 0) {
        base->flags |= 0x1000;
        return;
    }
    SetPointerSomething(base);
    t = base->counter++;
    switch (shot->kind) {
    case KIND_PAIR:
        // (2 left, 2 up or down, then flat at 3);
        if (t == 16) {
            shot->vx = -0x300;
            shot->vy = 0;
        }
        break;
    case KIND_CHARGE:
        if (t < CHARGE_STEP * 4) {
            u8 step = t >= CHARGE_STEP * 3 ? 3 : t >= CHARGE_STEP * 2 ? 2 : t >= CHARGE_STEP ? 1 : 0;
            shot->vx = sChargeX[shot->side];
            shot->vy = shot->up ? -sChargeY[shot->side][step] : sChargeY[shot->side][step];
        } else if (t < CHARGE_DASH) {
            shot->vx = shot->vy = 0;
        } else {
            shot->vx = -0x600;
            shot->vy = 0;
            if (!((t - CHARGE_DASH) & 3)) OrbSparkle(shot, (t - CHARGE_DASH) & 4);
        }
        break;
    }
    base->x += shot->vx;
    base->y += shot->vy + StarRodSink(base->unk56, &shot->lastTop);
    // (it hits the flying Kirbys itself: KatAM's damage is kept off them, abilities/star_rod.c);
    for (t = 0; t < gNumKirbys; t++)
        if (gKirbys[t].base.roomId == base->roomId
            && StarRodTouches(&gKirbys[t], base->x, base->y, 6, shot->kind == KIND_CRESCENT ? 12 : 6)) StarRodHit(&gKirbys[t], base->x);
    sx = (base->x - level->viewportPosition.x) >> 8;
    sy = (base->y - level->viewportPosition.y) >> 8;
    if (sx < -48 || sx > 288 || sy < -48 || sy > 208) {
        base->flags |= 0x1000;
        return;
    }
    if (shot->kind == KIND_CRESCENT) base->sprite.variant = FRAME_SHOT + SHOT_CRESCENT + (t & 1);
    else if (shot->kind == KIND_CHARGE && t >= CHARGE_DASH) base->sprite.variant = FRAME_SHOT + SHOT_DASH + 3 - (((t - CHARGE_DASH) >> 1) & 3);
    else base->sprite.variant = FRAME_SHOT + SHOT_STAR + 3 - ((t >> 2) & 3);
    Macro_08107BA8_4(base, &base->sprite, &sprite, SHOT_TILES, &base->sprite);
    base->sprite.palId = shot->palId;
    sub_0806F8BC(base);
}

static struct OrbShot *Shoot(struct PowerOrb *o, u8 kind, s32 dx, s32 dy, s32 vx, s32 vy) {
    struct Object *obj = &o->n.obj;
    struct Task *t = TaskCreate(OrbShotMain, sizeof(struct OrbShot), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct OrbShot *shot = TaskGetStructPtr(t);
    struct ObjectBase *base = &shot->base;

    ClearObjectBase(base);
    base->header.kind = 2;
    base->x = obj->base.x + (dx << 8);
    base->y = obj->base.y + (dy << 8);
    base->parent = o;
    base->counter = 0;
    base->roomId = obj->base.roomId;
    base->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(base)) base->flags |= 0x2000;
    base->unk63 = 1;
    base->flags |= 0x10000140;
    base->unk68 = 0x20000043;
    base->unk5C |= 0x80000;
    shot->vx = vx;
    shot->vy = vy;
    shot->kind = kind;
    shot->side = 0;
    shot->up = FALSE;
    shot->lastTop = gCurLevelInfo[base->unk56].unk_S32Vec2_7C.y;
    shot->palId = NidlLoadPalette(&o->n, ShotKey(o), FRAME_SHOT);
    if (kind == KIND_CRESCENT) sub_0803E2B0(base, -6, -12, 6, 12);
    else sub_0803E2B0(base, -6, -6, 6, 6);
    ObjectSetBounds(base, 0, 0, 0, 0);
    ObjectBaseInitSprite(base, &base->sprite, SHOT_TILES, NidlGraphicsOf(&o->n)->animId, FRAME_SHOT + (kind == KIND_CRESCENT ? SHOT_CRESCENT : SHOT_STAR + 3),
                         obj->base.sprite.unk14 >> 6);
    base->sprite.palId = shot->palId;
    return shot;
}

/* TimedShot(); A timed shot, if the last was COOLDOWN frames ago or more, the next of the stream, the fan and the pair; its look (it
 * shrinks a moment) either way; */
static void TimedShot(struct PowerOrb *o) {
    o->look = ATTACK_LOOK;
    if (o->cooldown < COOLDOWN) return;
    o->cooldown = 0;
    switch (o->attack) {
    case ATTACK_STREAM:
        o->stream = 3;
        o->streamTimer = 0;
        break;
    case ATTACK_FAN:
        Shoot(o, KIND_STAR, -3, -2, -0x233, -0x1B3);
        Shoot(o, KIND_STAR, -3, 0, -0x300, 0);
        Shoot(o, KIND_STAR, -3, 1, -0x233, 0x1B3);
        break;
    default:
        Shoot(o, KIND_PAIR, -2, -2, -0x200, -0x200);
        Shoot(o, KIND_PAIR, -2, 2, -0x200, 0x200);
        break;
    }
    PlaySfx(&o->n.obj.base, SE_KIRBY_AIRSHOT);
    if (++o->attack >= ATTACKS) o->attack = 0;
}

static void Set(struct PowerOrb *o, u8 state) {
    o->state = state;
    o->timer = 0;
    o->step = 0;
    o->stepTimer = 0;
    o->row = 0;
    o->phase = 0;
}

// Next(); The state after this one (see above);
static void Next(struct PowerOrb *o) {
    switch (o->state) {
    case STATE_ENTER:
        Set(o, STATE_HOVER);
        break;
    case STATE_HOVER:
        o->turns ^= 1;
        Set(o, o->turns & 1 ? STATE_ROAM : STATE_CHARGE);
        break;
    case STATE_ROAM:
        o->turns ^= 2;
        Set(o, o->turns & 2 ? STATE_CHARGE : STATE_ZIP);
        break;
    case STATE_CHARGE:
        Set(o, STATE_ROAM);
        break;
    default:
        Set(o, STATE_HOVER);
        break;
    }
}

// Zip(); To each row at 6 pixels a frame, a wait, a lunge as it shoots a crescent, a wait; then on to hovering;
static void Zip(struct PowerOrb *o) {
    s32 target;

    switch (o->phase) {
    case 0:
        // (the wait before it goes);
        if (++o->stepTimer < ZIP_WAIT) return;
        o->stepTimer = 0;
        o->phase = 1;
        // fallthrough
    case 1:
        target = sZipRows[o->row] << 8;
        if (o->sy < target - ZIP_SPEED) o->sy += ZIP_SPEED;
        else if (o->sy > target + ZIP_SPEED) o->sy -= ZIP_SPEED;
        else {
            o->sy = target;
            o->phase = 2;
        }
        return;
    case 2:
        if (++o->stepTimer < ZIP_WAIT) return;
        o->stepTimer = 0;
        o->phase = 3;
        o->look = LUNGE_TIME;
        Shoot(o, KIND_CRESCENT, -6, 0, -0x600, 0);
        PlaySfx(&o->n.obj.base, SE_KIRBY_AIRSHOT);
        // fallthrough
    case 3:
        o->sx += sLunge[o->stepTimer];
        if (++o->stepTimer < LUNGE_TIME) return;
        o->stepTimer = 0;
        o->sx = SPOT_X;
        if (++o->row >= ARRAY_COUNT(sZipRows)) {
            o->phase = 4;
            return;
        }
        o->phase = 0;
        return;
    default:
        if (++o->stepTimer >= ZIP_WAIT) Next(o);
        return;
    }
}

// Fly(); Every Kirby free in its room takes the Star Rod (again, coming back, or after a hit KatAM's states took him);
static void Fly(struct Object *obj) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == obj->base.roomId && StarRodCanFly(&gKirbys[i])) StarRodStart(&gKirbys[i], obj);
}

static void PowerOrbMain(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;
    u8 i, frame;

    NidlUpdate(&o->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (its stars too); it takes it back;
    sub_08034E14(obj);
    Fly(obj);

    // The screen sinks (its arena's, the camera with it);
    if (o->sink < SINK_END) {
        o->sink += SINK_SPEED;
        ArenaSink(obj, o->sink);
    }
    if (o->cooldown < 0xFFFF) o->cooldown++;
    o->timer++;
    switch (o->state) {
    case STATE_ENTER:
        // (in from the left, as NiDL's: to its spot in 20 frames, then still 20);
        if (o->timer <= 20) {
            o->sx += (SPOT_X - (160 << 8)) / 20;
            o->sy += (SPOT_Y - (82 << 8)) / 20;
        }
        if (o->timer >= 40) {
            o->sx = SPOT_X;
            o->sy = SPOT_Y;
            Next(o);
        }
        break;
    case STATE_HOVER:
        if (!o->stepTimer) o->stepTimer = sHoverTimes[o->step];
        o->sy += sHoverSpeeds[o->step];
        if (!--o->stepTimer && ++o->step >= ARRAY_COUNT(sHoverTimes)) {
            o->sy = SPOT_Y;
            Next(o);
            break;
        }
        for (i = 0; i < ARRAY_COUNT(sHoverShots); i++)
            if (o->timer == sHoverShots[i]) TimedShot(o);
        break;
    case STATE_ROAM:
        if (!o->stepTimer) o->stepTimer = sRoamTimes[o->step];
        o->sx += sRoamX[o->step];
        o->sy += sRoamY[o->step];
        if (!--o->stepTimer && ++o->step >= ARRAY_COUNT(sRoamTimes)) o->step = ROAM_LOOP;
        for (i = 0; i < ARRAY_COUNT(sRoamShots); i++)
            if (o->timer == sRoamShots[i]) TimedShot(o);
        if (o->timer >= 240) {
            o->sx = SPOT_X;
            o->sy = SPOT_Y + 0x100;
            Next(o);
        }
        break;
    case STATE_CHARGE:
        if (o->timer == CHARGE_FIRE) {
            static const s8 sDy[4] = { -2, -1, 1, 2 };
            for (i = 0; i < 4; i++) {
                struct OrbShot *shot = Shoot(o, KIND_CHARGE, 0, sDy[i], 0, 0);
                shot->side = i == 0 || i == 3 ? 0 : 1;
                shot->up = i < 2;
            }
            PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
        }
        if (o->timer >= 120) Next(o);
        break;
    default:
        Zip(o);
        break;
    }
    // A stream's stars, 16 frames apart;
    if (o->stream && !o->streamTimer--) {
        Shoot(o, KIND_STAR, -4, 0, -0x400, 0);
        o->streamTimer = STREAM_GAP - 1;
        o->stream--;
    }

    obj->base.x = ScreenLeft(obj) + o->sx;
    obj->base.y = ScreenTop(o) + o->sy;
    obj->base.xspeed = obj->base.yspeed = 0;
    // Touching it hurts (the flying Kirbys: it hits them itself);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == obj->base.roomId && StarRodTouches(&gKirbys[i], obj->base.x, obj->base.y, 18, 18)) StarRodHit(&gKirbys[i], obj->base.x);
    // Its look: spinning; shrinking a moment as it shoots; blinking small as it charges;
    frame = FRAME_SPIN + (o->spin >= 30 ? 3 : o->spin >= 20 ? 2 : o->spin >= 10 ? 1 : 0);
    if (++o->spin >= 40) o->spin = 0;
    if (o->look) {
        o->look--;
        frame = FRAME_SHRINK + (o->look >= ATTACK_LOOK / 2 ? (ATTACK_LOOK - 1 - o->look) >> 1 : o->look >> 1);
    } else if (o->state == STATE_CHARGE && (o->timer & 4)) {
        frame = FRAME_SMALL;
    }
    NidlSetFrame(&o->n, frame);
}

// PowerOrbStart(); The fight starts (also the engine's after a hit: it goes on where it was);
void PowerOrbStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, PowerOrbMain);
}

// PowerOrbInit(); Its boxes (40x40, through anything), its start (coming in from the screen's left of its spot);
void PowerOrbInit(struct NidlObject *n) {
    struct PowerOrb *o = (struct PowerOrb *)n;
    struct Object *obj = &n->obj;

    o->sx = 160 << 8;
    o->sy = 82 << 8;
    o->sink = 0;
    o->cooldown = COOLDOWN;
    o->attack = ATTACK_STREAM;
    o->stream = o->streamTimer = 0;
    o->look = 0;
    o->turns = 0;
    Set(o, STATE_ENTER);
    o->spin = 0;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -20, -20, 20, 20);
    ObjectSetBounds(&obj->base, -20, -20, 20, 20);
}
