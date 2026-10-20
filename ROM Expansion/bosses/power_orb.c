/* Nightmare's Power Orb; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x42 (spawn 0x080AE3BC), as NiDL boss
 * NIDL_POWER_ORB, the Fountain of Dreams' first (room 0x003 here: the fall through space). As NiDL's, it's fought with the Star Rod
 * (abilities/star_rod.c: every Kirby in the room flies with it, shooting stars), at the top of the room, the screen sinking through
 * space as they fight (0.2 pixels a frame, 513 pixels down: ArenaSink(), its arena's screen; NiDL's camera, 0x0300215C, from 8 to
 * 521), the orb, its shots and the Kirbys keeping their places on it. Its frames (0x087544B4, mini-boss descriptor 37; only those
 * it uses: KatAMExpansion.PowerOrb): NiDL's 4-7, the spinning orb (10 frames each), 8-11 it shrinking (its look as it attacks; 11
 * blinks as it charges), then its shots' (its 20-29 and 39-42, their own tiles), from FRAME_SHOT, then the intro's slash (14-19).
 * Its intro (STATE_ENTER), NiDL's, traced frame by frame from the room's first (as the fight, its speeds change in steps it keeps
 * exactly): Kirby comes in at the room's bottom (KatAMExpansionRooms.cs: NiDL's start there; this and its arena stand there too,
 * KatAM making objects only near the camera) and the screen rises from there to the fight's (sRiseSpeeds: 761 pixels, 4 a frame,
 * slowing to 1 as it gets there: ArenaRise(), its arena's lock under its screen, the cameras right on it); 66 frames in, every
 * Kirby flies up into it from below its left (abilities/star_rod.c's StarRodIntro(): NiDL's flight and frames) and the orb comes
 * in from its top right, spinning, down to the left; Kirby swings at it (a slash across the screen: OrbSlash()), it pulses 5 times
 * and spins fast as it loops round to its spot, and the fight starts there (319 frames in, NiDL's). Meanwhile it's untouchable and
 * harmless and shoots nothing, and the Kirbys neither move nor shoot (the riders fly them).
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
void ArenaRise(struct Object *, s32);

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
// The intro (frames into it: the orb and the Kirbys show; it moves; Kirby's swing hits it; its pulses; its fast spin; the fight);
#define INTRO_SHOW 66
#define INTRO_MOVE 94
#define INTRO_CLASH 202
#define INTRO_PULSE 204
#define INTRO_FAST 242
#define INTRO_END 319
#define INTRO_X (256 << 8)
#define INTRO_Y (-8 << 8)
#define INTRO_BELOW 761
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
// The intro's slash (NiDL's 14-19, from FRAME_SLASH; its second palette): where it is on the fight's screen (NiDL's (144, 118) in
// the room, the screen still rising past it);
#define FRAME_SLASH (FRAME_SHOT + 14)
#define SLASH_TILES 16
#define SLASH_X 136
#define SLASH_Y 110
// Its end's dot (PowerOrbEnd(): the orb, and the star with its Kirbys, far off in the moon; NiDL's palette 15);
#define FRAME_DOT (FRAME_SLASH + 6)

enum { STATE_ENTER, STATE_HOVER, STATE_ROAM, STATE_CHARGE, STATE_ZIP };
enum { ATTACK_STREAM, ATTACK_FAN, ATTACK_PAIR, ATTACKS };
enum { KIND_STAR, KIND_PAIR, KIND_CHARGE, KIND_CRESCENT };

struct PowerOrb {
    struct NidlObject n;
    s32 sx, sy;         // Its spot on the fight's screen (8.8, from the screen's top left as it sinks);
    s32 sink;           // How far the screen has sunk (8.8);
    s32 below;          // How far under its screen the intro's screen is (pixels);
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
// The intro's screen rising: pixels a frame and frames;
static const u8 sRiseSpeeds[] = { 0, 8, 4, 3, 2, 1 };
static const u8 sRiseTimes[] = { 2, 1, 141, 32, 32, 29 };
/* The intro's moves from INTRO_MOVE: speeds (8.8, y down) and frames, from (256, -8) on the screen (above its right) down to the left,
 * past Kirby, its swing knocking it back up to the right, then round to the left and back to its spot (192.5, 75.25); */
static const s16 sIntroX[] = { -0x100, -0x100, -0x100, -0x100, 0x400, 0x200, 0x200, 0x100, 0x80, -0x200, -0x400, -0x400, -0x100, 0x100, 0x200, 0x400, 0x200, 0x80, 0x40 };
static const s16 sIntroY[] = { 0x100, 0x80, 0x20, 0, -0x100, -0x100, -0x200, -0x100, -0x80, 0x80, 0xC0, 0x180, 0x180, 0x180, 0x180, 0xC0, -0x40, -0xC0, -0xC0 };
static const u8 sIntroTimes[] = { 72, 10, 10, 16, 8, 8, 8, 6, 4, 6, 6, 11, 6, 6, 6, 6, 6, 6, 6 };
// Its pulses (8 frames apart): the shrink's frames, 6 frames; the slash's frames, frame by frame;
static const u8 sPulse[6] = { 0, 1, 3, 2, 1, 0 };
static const u8 sSlash[] = { 0, 0, 1, 2, 3, 3, 4, 4, 5, 5 };

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

// OrbSlash(); The slash Kirby's swing makes in the intro (NiDL's object 0xDC): growing across the screen from its spot, 10 frames;
static void OrbSlashMain(void) {
    struct EffectObject *slash = TaskGetStructPtr(gCurTask);

    if (slash->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (slash->unk4 >= ARRAY_COUNT(sSlash)) {
        slash->flags |= 0x1000;
        return;
    }
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    slash->flags |= 4 | 1;
    slash->sprite.variant = FRAME_SLASH + sSlash[slash->unk4++];
    EffectObjectPostUpdate(slash);
}

static void OrbSlash(struct PowerOrb *o) {
    struct Task *t = TaskCreate(OrbSlashMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *slash = TaskGetStructPtr(t);

    ClearEffectObject(slash);
    slash->header.kind = 3;
    slash->x = ScreenLeft(&o->n.obj) + (SLASH_X << 8);
    slash->y = ScreenTop(o) + (SLASH_Y << 8);
    slash->parent = o;
    slash->roomId = o->n.obj.base.roomId;
    slash->unk4 = 0;
    EffectObjectInitSprite(slash, &slash->sprite, SLASH_TILES, NidlGraphicsOf(&o->n)->animId, FRAME_SLASH, 0x1A);
    slash->sprite.palId = NidlLoadPalette(&o->n, NidlGraphicsOf(&o->n)->animId | NIDL_THIRD_PALETTE, FRAME_SLASH);
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

/* Intro(); The intro's frame t (see above): the screen rising, the orb's place and look (hidden until INTRO_SHOW; spinning, 8 frames
 * a frame; the pulses, the spin going on between them; spinning fast, 1 frame a frame); its frame, or -1 (hidden); */
static s16 Intro(struct PowerOrb *o, u16 t) {
    s32 n, moves;
    u8 i;

    for (n = t, i = 0, o->below = INTRO_BELOW; i < ARRAY_COUNT(sRiseTimes); n -= sRiseTimes[i++]) {
        o->below -= sRiseSpeeds[i] * (n < sRiseTimes[i] ? n + 1 : sRiseTimes[i]);
        if (n < sRiseTimes[i]) break;
    }
    o->sx = INTRO_X;
    o->sy = INTRO_Y;
    for (moves = t - INTRO_MOVE + 1, i = 0; moves > 0 && i < ARRAY_COUNT(sIntroTimes); moves -= sIntroTimes[i++]) {
        n = moves < sIntroTimes[i] ? moves : sIntroTimes[i];
        o->sx += sIntroX[i] * n;
        o->sy += sIntroY[i] * n;
    }
    if (t < INTRO_SHOW) return -1;
    if (t < INTRO_PULSE) return FRAME_SPIN + ((((t < INTRO_CLASH ? t : INTRO_CLASH - 1) - INTRO_SHOW) >> 3) & 3);
    if (t < INTRO_FAST) return ((t - INTRO_PULSE) & 7) < 6 ? FRAME_SHRINK + sPulse[(t - INTRO_PULSE) & 7] : FRAME_SPIN + ((((t - INTRO_PULSE) >> 3) + 1) & 3);
    return FRAME_SPIN + ((t - INTRO_FAST) & 3);
}

/* PowerOrbShotNear(); Whether one of its shots is coming at a spot (8.8, the room's): ahead of it (to its right, moving left) within
 * 72 pixels, in its row (20 up or down); its y in `shotY` (abilities/star_rod.c: the helpers dodge it now and then); */
bool32 PowerOrbShotNear(struct Object *obj, s32 x, s32 y, s32 *shotY) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct OrbShot *shot;
        s32 dx, dy;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != OrbShotMain) continue;
        shot = TaskGetStructPtr(t);
        if (shot->base.parent != obj || shot->base.roomId != obj->base.roomId || (shot->base.flags & 0x1000) || shot->vx >= 0) continue;
        dx = (shot->base.x - x) >> 8;
        dy = (shot->base.y - y) >> 8;
        if (dx < 0 || dx > 72 || dy < -20 || dy > 20) continue;
        *shotY = shot->base.y;
        return TRUE;
    }
    return FALSE;
}

// PowerOrbIntro(); The intro's frame (-1: over) (abilities/star_rod.c's StarRodIntro());
s32 PowerOrbIntro(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;

    return o->state == STATE_ENTER && o->timer ? o->timer - 1 : -1;
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
    s16 intro = 0;

    NidlUpdate(&o->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (its stars too); it takes it back;
    sub_08034E14(obj);
    Fly(obj);

    // The screen sinks (its arena's, the camera with it), once the intro's over;
    if (o->state != STATE_ENTER && o->sink < SINK_END) {
        o->sink += SINK_SPEED;
        ArenaSink(obj, o->sink);
    }
    if (o->cooldown < 0xFFFF) o->cooldown++;
    o->timer++;
    switch (o->state) {
    case STATE_ENTER:
        intro = Intro(o, o->timer - 1);
        ArenaRise(obj, o->below << 8);
        if (o->timer - 1 == INTRO_CLASH) OrbSlash(o);
        if (o->timer - 1 >= INTRO_END) {
            o->below = 0;
            ArenaRise(obj, 0);
            o->sx = SPOT_X;
            o->sy = SPOT_Y;
            o->spin = 0;
            obj->base.flags &= ~(0x200 | 0x400);
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
    // (the intro: on the screen as the camera shows it (its lock's last frame: ArenaRise()), untouchable, harmless, hidden until it
    // shows);
    if (o->state == STATE_ENTER) {
        obj->base.x = gCurLevelInfo[obj->base.unk56].viewportPosition.x + o->sx;
        obj->base.y = gCurLevelInfo[obj->base.unk56].viewportPosition.y + o->sy;
        obj->base.flags |= 0x200;
        if (intro < 0) {
            obj->base.flags |= 0x400;
            return;
        }
        obj->base.flags &= ~0x400;
        NidlSetFrame(&o->n, intro);
        return;
    }
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

/* The orb beaten (PowerOrbEnd()): NiDL's end of the fight, traced frame by frame (its object's last states, its warp star's and
 * the camera's, 0x0300215C), run by a task of its own (KatAM's explosion takes the orb away, as its bosses):
 * • 24 frames after it bursts, it's back where it was, wobbling (its shrink's frames, and two of its spin's every 8), and flies off
 *   up to the right (sFleeY, a pixel right a frame), and its warp star comes, 11 pixels under where it was (finale.c's
 *   NidlFinaleStarAt());
 * • the warp star off, every Kirby on it (its flight's first step NIDL_RIDE_FINALE: warpstar.c holds them on it, this moves them),
 *   it flies NiDL's launch up off the screen (sLaunch, then on with its last two moves till it's 32 pixels over the screen); 21
 *   frames in, the screen rises to the room's top (1.25 pixels a frame, a 16th more each frame, never more than a 16th of the way
 *   left, nor under an 8th), the star kept 34 pixels over it;
 * • 2 frames after the screen's at the top, the orb, small (a quarter of its size, smaller every 20 frames: sMoonSizes), flies into
 *   the moon (sMoonOrb, from (136, -15) on the screen), then too small, a dot (FRAME_DOT, NiDL's); 60 frames after it, the star
 *   with the Kirbys, the same way (sMoonStar; warpstar.c draws them at its size, PowerOrbRideSize()), then a dot too; 410 frames
 *   after the orb came in, the star's flight goes on to its fade to white (18 frames), then Nightmare's room;
 * the screen's held where the fight left it till then (its arena gone, the camera would go back to the fight's first screen,
 * the star left under it, and the Kirbys flying with the Star Rod, kept on the screen, with it); */
#define END_TILES 36
#define END_BACK 24
#define END_STAR_Y 11
#define END_CAMERA 21
#define END_SPEED 0x140
#define END_SPEEDUP 0x10
#define END_SLOWEST 0x20
#define END_ABOVE (-32 << 8)
#define END_HELD (-34 << 8)
#define END_OFF (-16 << 8)
#define END_ON_SCREEN (24 << 8)
#define END_TOP (8 << 8)
#define END_MOON_X (136 << 8)
#define END_MOON_Y (-15 << 8)
#define END_MOON_WAIT 2
#define END_STAR_LATE 60
#define END_FADE 410
#define END_DOT_KEY 0x6000
#define END_MATRIX 6        // (its affine matrices: 6, then 14, 22, 30 for pieces flipped; warpstar.c's riders use 7...);
#define NIDL_RIDE_FINALE 0xFC
#define NIDL_RIDE_HIDDEN 0x8000

void sub_080534D0(struct Kirby *);
void NidlShrink(u32 first, s32 x, s32 y, u32 pa, u32 matrix);

enum { END_HIDDEN, END_FLEE, END_GONE };

struct OrbEnd {
    struct EffectObject e;
    s32 camX, camY, camSpeed;   // The held screen's top left (8.8, the room's) and its speed rising;
    s32 x0, y0;                 // Where it was beaten (8.8);
    s32 orbX, orbY;             // The orb, and the ride's star (Kirby's spot on it), 8.8;
    s32 starX, starY;
    u16 t;                      // Frames since it burst;
    u16 ride;                   // Frames since the star went off (0: not yet);
    u16 top;                    // Frames since the screen got to the top (0: not yet);
    u16 moon;                   // Frames since the orb came into the moon (0: not yet);
    u16 animId;
    u8 player;
    u8 phase;
    u8 palId, dotPalId;
    bool8 above;                // The star's over the screen, kept there;
};

// The flee: its y (pixels from where it was beaten) frame by frame, x 2 pixels right of it, a pixel more each frame;
static const s16 sFleeY[] = { 10, 10, 9, 9, 8, 8, 7, 6, 5, 3, 3, 1, -1, -2, -4, -6, -8, -10, -13, -15, -17, -20, -23, -26, -29, -32, -35, -39,
                              -43, -46, -49, -54, -58, -62, -67, -70, -75, -80, -85, -90, -94, -100, -105, -111, -117, -121, -127, -134, -140,
                              -146, -152, -158, -165, -172, -179, -185, -192, -200, -207 };
// (its wobble: NiDL's frames 8, 9, 11, 10, 9, 8 as its shrink's, then two of its spin's);
static const u8 sFleeFrames[6] = { 0, 1, 3, 2, 1, 0 };
// The launch: the star's moves (pixels, down positive) frame by frame from its second;
static const s8 sLaunch[][2] = { { 1, -1 }, { 2, 0 }, { 1, -1 }, { 2, 0 }, { 1, -1 }, { 2, -2 }, { 1, -2 }, { 2, -2 }, { 1, -3 }, { 2, -4 }, { 1, -4 },
                                 { 2, -4 }, { 1, -5 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -8 },
                                 { 1, -7 }, { 2, -7 }, { 1, -7 }, { 2, -7 }, { 1, -8 } };
// Into the moon: the size (its matrix's pa: 256 / the size, 8.8) and frames, from a quarter, then a dot;
static const u16 sMoonSizes[][2] = { { 1024, 19 }, { 1092, 20 }, { 1170, 20 }, { 1260, 19 }, { 1365, 20 }, { 1489, 20 }, { 1638, 19 }, { 1820, 20 },
                                     { 2048, 20 }, { 2340, 19 }, { 2730, 20 } };
// Into the moon: the moves frame by frame, a pixel or none each way, 4 bits a frame (x + 1, (y + 1) << 2), low first: the orb's,
// then the star's (NiDL's own: it catches up with the orb in the moon's middle);
// (426 frames)
static const u8 sMoonOrb[213] = {
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84,
    0x94, 0x84, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x95, 0x94, 0x95, 0x94, 0x95, 0x94,
    0x55, 0x95, 0x55, 0x95, 0x54, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x56, 0x95, 0x55, 0x95,
    0x65, 0x95, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x65, 0x95, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
};
// (366 frames)
static const u8 sMoonStar[183] = {
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84,
    0x94, 0x84, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x95, 0x94, 0x95, 0x94, 0x95, 0x94,
    0x55, 0x95, 0x55, 0x95, 0x54, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x56, 0x95, 0x55, 0x95,
    0x65, 0x95, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55,
};

// MoonSize(); The size k frames into the moon (sMoonSizes), 0 past it (a dot);
static u32 MoonSize(u16 k) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sMoonSizes); i++) {
        if (k < sMoonSizes[i][1]) return sMoonSizes[i][0];
        k -= sMoonSizes[i][1];
    }
    return 0;
}

// MoonMove(); Frame k's move of a path into the moon (sMoonOrb, sMoonStar), none past it;
static void MoonMove(const u8 *path, u16 frames, u16 k, s32 *x, s32 *y) {
    u8 code;

    if (k >= frames) return;
    code = k & 1 ? path[k >> 1] >> 4 : path[k >> 1] & 0xF;
    *x += ((code & 3) - 1) << 8;
    *y += (((code >> 2) & 3) - 1) << 8;
}

static void OrbEndMain(void);

// OrbEndOf(); The orb's end in a room (its task), or NULL;
static struct OrbEnd *OrbEndOf(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct OrbEnd *end;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != OrbEndMain) continue;
        end = TaskGetStructPtr(t);
        if (end->e.roomId == room && !(end->e.flags & 0x1000)) return end;
    }
    return NULL;
}

/* PowerOrbRideSize(); The size the ride's star and its Kirbys are drawn at (warpstar.c: on its flight's first steps): its matrix's
 * pa, 0 their own, NIDL_RIDE_HIDDEN too small to show (the orb's end draws a dot there), or off the screen's top (kept on it,
 * hidden: KatAM would point at the helpers there); */
u32 PowerOrbRideSize(u16 room) {
    struct OrbEnd *end = OrbEndOf(room);
    u32 pa;

    if (end == NULL) return 0;
    if (end->moon <= END_STAR_LATE) return end->ride && end->starY - end->camY <= END_OFF ? NIDL_RIDE_HIDDEN : 0;
    pa = MoonSize(end->moon - 1 - END_STAR_LATE);
    return pa ? pa : NIDL_RIDE_HIDDEN;
}

// EndPalette(); A palette's slot, loaded under `key` as frame `frame` has it (as NidlLoadPalette(): the orb's gone);
static u8 EndPalette(struct OrbEnd *end, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != end->e.roomId) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

// EndDraw(); The orb's sprite at (x, y) (8.8), frame `frame`, at the size of `pa` (0 its own);
static void EndDraw(struct OrbEnd *end, s32 x, s32 y, u8 frame, u32 pa) {
    struct EffectObject *e = &end->e;
    u32 first = gUnk_030024F0;

    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = frame == FRAME_DOT ? end->dotPalId : end->palId;
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    e->flags = (e->flags & ~0x400) | 4 | 1;
    EffectObjectPostUpdate(e);
    if (pa) NidlShrink(first, e->sprite.x, e->sprite.y, pa, END_MATRIX);
}

// EndRider(); Whether a Kirby rides its star (on its flight's first step, held there: warpstar.c);
static bool32 EndRider(struct Kirby *kirby, u16 room) {
    const struct Unk_08353510 *step = kirby->unk114;

    return kirby->base.roomId == room && kirby->stateFn == sub_080534D0 && step != NULL && step->unk8 == NIDL_RIDE_FINALE && !(step->unk9 & 0x40);
}

// EndRide(); The star's ride (see above): the riders moved with it, the screen rising, the orb and the star into the moon, the fade;
static void EndRide(struct OrbEnd *end) {
    struct Kirby *lead = NULL;
    u16 k;
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (EndRider(&gKirbys[i], end->e.roomId) && lead == NULL) lead = &gKirbys[i];
    if (!end->ride) {
        if (lead == NULL) return;
        end->starX = lead->base.x;
        end->starY = lead->base.y;
    }
    k = end->ride++;
    // (its launch, up off the screen, then held over it);
    if (!end->moon && !end->above && k) {
        u16 at = k - 1 < ARRAY_COUNT(sLaunch) ? k - 1 : ARRAY_COUNT(sLaunch) - 2 + ((k - 1 - ARRAY_COUNT(sLaunch)) & 1);

        end->starX += sLaunch[at][0] << 8;
        end->starY += sLaunch[at][1] << 8;
        if (end->starY - end->camY <= END_ABOVE) end->above = TRUE;
    }
    // (the screen rising to the room's top);
    if (k >= END_CAMERA && !end->top) {
        s32 left = end->camY - END_TOP;

        end->camSpeed = end->camSpeed ? end->camSpeed + END_SPEEDUP : END_SPEED;
        if (end->camSpeed > left >> 4) end->camSpeed = left >> 4;
        if (end->camSpeed < END_SLOWEST) end->camSpeed = END_SLOWEST;
        end->camY = end->camSpeed < left ? end->camY - end->camSpeed : END_TOP;
        if (end->camY == END_TOP) end->top = 1;
    } else if (end->top) {
        end->top++;
    }
    if (end->above && !end->moon) end->starY = end->camY + END_HELD;
    // (into the moon: the orb, then the star);
    if (end->top == END_MOON_WAIT + 1) {
        end->moon = 1;
        end->orbX = end->camX + END_MOON_X;
        end->orbY = end->camY + END_MOON_Y;
    } else if (end->moon) {
        MoonMove(sMoonOrb, ARRAY_COUNT(sMoonOrb) * 2, end->moon - 1, &end->orbX, &end->orbY);
        if (end->moon - 1 == END_STAR_LATE) {
            end->starX = end->camX + END_MOON_X;
            end->starY = end->camY + END_MOON_Y;
        } else if (end->moon - 1 > END_STAR_LATE) {
            MoonMove(sMoonStar, ARRAY_COUNT(sMoonStar) * 2, end->moon - 2 - END_STAR_LATE, &end->starX, &end->starY);
        }
        end->moon++;
    }
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!EndRider(kirby, end->e.roomId)) continue;
        kirby->base.x = end->starX;
        kirby->base.y = end->starY;
        // (off the screen, hidden, they're kept on it: KatAM points at helpers off it, hidden or not);
        if (end->starY - end->camY <= END_OFF && end->moon <= END_STAR_LATE) kirby->base.y = end->camY + END_ON_SCREEN;
        // (on to the fade: its flight's next step);
        if (end->moon == END_FADE + 2) {
            kirby->unk114++;
            kirby->base.counter = ((const struct Unk_08353510 *)kirby->unk114)->unk4;
        }
    }
}

static void OrbEndMain(void) {
    struct OrbEnd *end = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &end->e;
    u16 k;
    u8 i;

    if (e->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    // (gone with the room: every Kirby left it);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys) {
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    EndRide(end);
    NidlFinaleHoldCamera(e->roomId, end->camX, end->camY);
    k = end->t++;
    if (k == END_BACK) {
        NidlFinaleStarAt(end->player, end->x0, end->y0 + (END_STAR_Y << 8));
        end->phase = END_FLEE;
    }
    // (back, wobbling, flying off up to the right);
    if (end->phase == END_FLEE) {
        k -= END_BACK;
        if (k >= ARRAY_COUNT(sFleeY)) {
            end->phase = END_GONE;
        } else {
            end->orbX = end->x0 + ((2 + k) << 8);
            end->orbY = end->y0 + (sFleeY[k] << 8);
            EndDraw(end, end->orbX, end->orbY, (k & 7) < 6 ? FRAME_SHRINK + sFleeFrames[k & 7] : FRAME_SPIN + (((k >> 3) + 1) & 3), 0);
        }
    }
    // (into the moon, smaller and smaller, then a dot; the star's dot after it, warpstar.c hiding it and its Kirbys);
    if (end->moon > 1) {
        u32 pa = MoonSize(end->moon - 2);

        EndDraw(end, end->orbX, end->orbY, pa ? FRAME_SPIN : FRAME_DOT, pa);
        if (!pa && end->moon - 2 > END_STAR_LATE && !MoonSize(end->moon - 2 - END_STAR_LATE)) EndDraw(end, end->starX, end->starY, FRAME_DOT, 0);
    }
}

// PowerOrbEnd(); The orb beaten (finale.c's PowerOrbBeaten(), as KatAM's explosion bursts it): its end (see above);
void PowerOrbEnd(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;
    struct Task *t = TaskCreate(OrbEndMain, sizeof(struct OrbEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct OrbEnd *end = TaskGetStructPtr(t);
    struct EffectObject *e = &end->e;

    ClearEffectObject(e);
    e->header.kind = 3;
    e->x = end->x0 = end->orbX = obj->base.x;
    e->y = end->y0 = end->orbY = obj->base.y;
    e->parent = NULL;
    e->roomId = obj->base.roomId;
    end->player = obj->base.unk56;
    e->flags |= 0x400;
    end->camX = ScreenLeft(obj);
    end->camY = ScreenTop(o);
    end->camSpeed = 0;
    end->starX = end->starY = 0;
    end->t = end->ride = end->top = end->moon = 0;
    end->phase = END_HIDDEN;
    end->above = FALSE;
    end->animId = NidlGraphicsOf(&o->n)->animId;
    EffectObjectInitSprite(e, &e->sprite, END_TILES, end->animId, FRAME_SPIN, 0x1A);
    end->palId = EndPalette(end, end->animId, FRAME_SPIN);
    end->dotPalId = EndPalette(end, end->animId | END_DOT_KEY, FRAME_DOT);
}

// PowerOrbStart(); The fight starts (also the engine's after a hit: it goes on where it was);
void PowerOrbStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, PowerOrbMain);
}

// PowerOrbInit(); Its boxes (40x40, through anything), its start (the intro's: hidden, over its screen's right);
void PowerOrbInit(struct NidlObject *n) {
    struct PowerOrb *o = (struct PowerOrb *)n;
    struct Object *obj = &n->obj;

    o->sx = INTRO_X;
    o->sy = INTRO_Y;
    o->sink = 0;
    o->below = INTRO_BELOW;
    o->cooldown = COOLDOWN;
    o->attack = ATTACK_STREAM;
    o->stream = o->streamTimer = 0;
    o->look = 0;
    o->turns = 0;
    Set(o, STATE_ENTER);
    o->spin = 0;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags |= 0x100 | 0x400;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -20, -20, 20, 20);
    ObjectSetBounds(&obj->base, -20, -20, 20, 20);
}
