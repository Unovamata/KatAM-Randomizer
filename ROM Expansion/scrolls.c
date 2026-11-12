/* scrolls.c: Squeak Squad's Ability Scrolls' upgrades on KatAM's abilities. A scroll is owned in the save's scroll bits (rooms.c's
 * SqueakScrollOwned(), squeak.h), numbered as the DS's (its HasScroll, 0x02020AD8, asked with a scroll's number by each ability's
 * code: Fire 0, Ice 1, Spark 2, Beam 3, Tornado 4, Parasol 5, Hammer 6, Cupid 7, Cutter 8, Laser 9, Sword 10, Wheel 11, Hi-Jump 12,
 * UFO 13, Sleep 14, Bomb 15, Ninja 16, Fighter 17, Throw 18, Magic 19, Animal 20, Bubble 21, Metal 22). The owner: "For KatAM existing
 * abilities, the scroll will upgrade them with the DS moveset 1:1": what the DS's scroll adds to its move, KatAM's move gets, with the
 * DS's numbers (SQUEAK_SPEED: its 20.12 speeds in KatAM's 24.8). Every Kirby has them (the DS has one Kirby: the helpers share the
 * player's scrolls). KatAMExpansionScrolls.cs hooks KatAM's code into these:
 * • Fire and Ice (the DS's flame and ice breath, 0x0202FDD0 and 0x02030F6C: each puff of breath it shoots gets, with the scroll, its
 *   prmKirbyFire/Ice.kgb words 3 and 4, 1.5 pixels a frame, up with Up held, down with Down): KatAM's breath's puffs take their height
 *   speed from Kirby's base.counter as they start again (sub_080737D8, the breath's hit box, and sub_080730E0, sub_080732E4,
 *   sub_080733D8, its flames), which KatAM's fire breath state (sub_0805D268) moves up to 1 pixel a frame up and 3/4 down, a step a
 *   frame, while Up or Down is held, and keeps at 0 for ice. ScrollBreath runs that state (its pointer, set as the breath starts, is
 *   this) and then, with the scroll, puts the DS's speed in at once: KatAM's ice breath bends as the DS's, its fire breath as far and
 *   as fast as the DS's;
 * • Spark (the DS's 0x02088444, Spark Kirby's every frame, and its aura 0x02087AA4, attacks 0x020882F8-0x02087FC0 and ball
 *   0x02087A74): see SparkCharging() below; */
#include "global.h"
#include "kirby.h"
#include "constants/kirby.h"
#include "main.h"
#include "data.h"
#include "squeak.h"
#include "abilities.h"
#include "code_0806F780.h"
#include "malloc_vram.h"
#include "trig.h"

// The scrolls' numbers (the DS's);
enum {
    SCROLL_FIRE,
    SCROLL_ICE,
    SCROLL_SPARK,
    SCROLL_BEAM,
    SCROLL_TORNADO,
    SCROLL_PARASOL,
    SCROLL_CUPID = 7,
    SCROLL_CUTTER,
    SCROLL_LASER,
    SCROLL_WHEEL = 11,
    SCROLL_HI_JUMP = 12,
    SCROLL_UFO,
    SCROLL_SLEEP = 14,
};

// SCROLL_BREATH_AIM; prmKirbyFire.kgb's and prmKirbyIce.kgb's words 3 and 4 (6144, -6144): a breath's puff's height speed with Up or
// Down held;
#define SCROLL_BREATH_AIM SQUEAK_SPEED(6144)

extern void sub_0805D268(struct Kirby *kirby);

// ScrollBreath(); KatAM's fire and ice breath state (sub_0805D268), then the DS's Fire and Ice scrolls' aim (see above);
void ScrollBreath(struct Kirby *kirby) {
    sub_0805D268(kirby);
    if (kirby->stateFn != ScrollBreath) return;
    if (!(kirby->ability == KIRBY_ABILITY_FIRE ? SqueakScrollOwned(SCROLL_FIRE)
          : kirby->ability == KIRBY_ABILITY_ICE && SqueakScrollOwned(SCROLL_ICE))) return;
    if (kirby->movementState & 0x40) kirby->base.counter = SCROLL_BREATH_AIM;
    else if (kirby->movementState & 0x80) kirby->base.counter = -SCROLL_BREATH_AIM;
    else kirby->base.counter = 0;
}

/* Spark. The DS's Spark Kirby charges as the D-pad is tapped (any way, a new press: prmKirbySpark.kgb's word 3, 16, each, a sound
 * 0xB0 or 0xB1 at random, half and half; the charge falls 1 a frame): past word 0 times that (128) he's at level 1, his spark aura
 * about him (spark_ext's 30, sound 0xB0), the charge back to 16; with the scroll (HasScroll 2), past word 1 times it (128) at
 * level 2 (the aura's composite 3, sound 0xB1, a hit box of 5 every 3 frames), falling back to level 1 (charge 128) when the
 * charge runs out. His attack (B) by level: 0, his own spark; 1 and 2, a burst for 30 frames (composites 1 and 2: their
 * lightning rings and bursts, hit boxes of 25 and 40 every frame, sounds 0x51 and 0xB2), then his own spark while B is held;
 * at level 2 with forward held, his own spark and the aura flies off forward as a ball (composite 4: word 5 and 6 from where it
 * was, 4 pixels up, word 7 a frame, 6 pixels, through everything, off the screen; its hit box 20, then 5 every 3 frames; sound
 * 0xB2, 0xAF after 30 frames). An attack ends the charge and the level; a hurt, the water or no control, the level. Every 61
 * frames a glint flickers on him (spark_cmn's 58, at random in his body's box).
 * In KatAM, with the scroll (the owner: "Keep KatAM's base as it is, however, with the upgraded abilities that the scrolls give, we
 * will then implement the quirks the base ability has plus the upgraded ones"): KatAM's Spark as it is, plus all of that. His own
 * spark is KatAM's (sub_08074AB0, its hit box and look); the bursts, the aura and the ball are ScrollPieces (their hit boxes the DS's,
 * the strengths KatAM's spark's 3 for the DS's 20: 25 is 4, 40 is 6, 5 is 1), their looks the DS's (SQUEAK_FX_SPARK_*, without
 * the DS's Kirby cells: KatAM's Kirby is drawn as he is). The charge and level are a SparkCharge task's per Kirby (its task's
 * number + 1 in SPARK_SLOT, Kirby's ability byte that Mike and Hi-Jump use, abilities.h), worked from his input each frame
 * (ScrollKirbyInput: sub_0803F324, detoured); his attack (ScrollSparkAttack: sub_08074AB0, KatAM's spark's start, detoured) takes
 * the level, KatAM's attack state (sub_0805D630, anim 0x34) held for the burst's 30 frames; */
#define SPARK_SLOT(kirby) NIDL_MIKE_USES(kirby)
#define SPARK_TAP 16                        // prmKirbySpark.kgb's word 3;
#define SPARK_LEVEL_1 (8 * SPARK_TAP)       // its word 0 times word 3;
#define SPARK_LEVEL_2 (8 * SPARK_TAP)       // its word 1 times word 3;
#define SPARK_CHARGE_MOST 0x7000
#define SPARK_GLINT_WAIT 60                 // (the DS's: past 60, every 61 frames);
#define SPARK_BURST_FRAMES 30
#define SPARK_BALL_RISE SQUEAK_SPEED(16384) // its words 6 and 7: the ball's start, 4 pixels up, and its speed, 6 pixels a frame;
#define SPARK_BALL_SPEED SQUEAK_SPEED(24576)
#define SPARK_BALL_SOUND_AT 30
#define SPARK_BALL_MARGIN 48                // Pixels round the screen the ball goes out of to end;
#define SCROLL_FEET 0x800                    // The DS's origin, Kirby's feet: 8 pixels under KatAM's;
#define SCROLL_FX_ROOM 48                    // squeak.c's SQUEAK_FX_ROOM: tiles left for KatAM's own effects;
#define SCROLL_LOOKS 5
#define SCROLL_NO_LOOK 0xFF
#define SCROLL_BEHIND_KIRBYS 0x14             // A sprite's order behind every Kirby's (theirs 0x10 + unk56, KatAM's effects' 0xA);
#define SPARK_D_PAD 0xF0
#define SPARK_KNOCK 0xA0                    // KatAM's spark's hit (sub_08074AB0's unk64 and unk68);
#define SPARK_HIT 0xA0000403

extern void sub_0805D630(struct Kirby *kirby);

// gScrollInputOriginal, gScrollSparkOriginal; KatAMExpansionScrolls.cs's trampolines: sub_0803F324 and sub_08074AB0 as they were;
void (*gScrollInputOriginal)(struct Kirby *) = NULL;
void (*gScrollSparkOriginal)(struct Kirby *) = NULL;

struct SparkCharge {
    struct Kirby *kirby;
    s16 charge;                 // The DS's +0x3A0;
    u8 level;                   // +0x3A4;
    bool8 attack;               // +0x3A8: his attack started this frame (the aura goes, or flies off: `ball`);
    bool8 ball;
    u8 glint;                   // +0x3AC;
};

enum { SPARK_AURA, SPARK_BURST, SPARK_BALL, BEAM_WHIP, TORNADO_ELEMENT, PARASOL_THRUST, CUTTER_BLADE, LASER_CHARGE, LASER_BOLT, UFO_STAR, HI_JUMP_ORB, WHEEL_ELEMENT };

// ScrollLook; One of a piece's looks: an effect's animation (squeak.h's SQUEAK_FX_*), `dx` pixels forward and `dy` under the piece
// (the DS's composite's part: its offset);
struct ScrollLook {
    struct EffectObject e;
    u8 effect;                  // SCROLL_NO_LOOK: none;
    u8 step;
    u8 timer;
    u8 palId;
    s8 dx;                      // (forward);
    s8 dy;
};

// ScrollPiece; An aura, a burst or a ball: its hit box (the DS's, from its origin: the base at Kirby's feet), its looks;
struct ScrollPiece {
    struct ObjectBase base;
    struct ScrollLook look[SCROLL_LOOKS];
    struct Kirby *kirby;
    struct Task *other;         // KatAM's own attack it goes with (the tornado's);
    u8 kind;
    u8 level;
    u16 clock;
};

static void SparkChargeMain(void);
static struct ScrollPiece *ScrollPieceCreate(struct Kirby *kirby, u8 kind, u8 level, s16 knock, u32 hitKind);
static bool32 CutterStep(struct ScrollPiece *p);
static bool32 LaserStep(struct ScrollPiece *p);
static bool32 BoltStep(struct ScrollPiece *p);
static bool32 StarStep(struct ScrollPiece *p);
static bool32 OrbStep(struct ScrollPiece *p);
static bool32 WheelStep(struct ScrollPiece *p);

// ChargeOf(); A Kirby's SparkCharge (SPARK_SLOT: its task's number + 1, checked), made with `create`; NULL: none;
static struct SparkCharge *ChargeOf(struct Kirby *kirby, bool32 create) {
    u8 slot = SPARK_SLOT(kirby);
    struct Task *t;
    struct SparkCharge *c;

    if (slot != 0 && slot <= MAX_TASK_NUM) {
        t = &gTasks[slot - 1];
        if (t->main == SparkChargeMain) {
            c = TaskGetStructPtr(t);
            if (c->kirby == kirby) return c;
        }
    }
    if (!create) return NULL;
    t = TaskCreate(SparkChargeMain, sizeof(struct SparkCharge), 0x3500, TASK_USE_EWRAM, NULL);
    if (t == &gEmptyTask) return NULL;
    c = TaskGetStructPtr(t);
    c->kirby = kirby;
    c->charge = 0;
    c->level = 0;
    c->attack = c->ball = FALSE;
    c->glint = 0;
    SPARK_SLOT(kirby) = t - gTasks + 1;
    return c;
}

// SparkChargeMain(); Gone once its Kirby isn't Spark Kirby (or his byte is another's);
static void SparkChargeMain(void) {
    struct SparkCharge *c = TaskGetStructPtr(gCurTask);

    if (c->kirby->ability != KIRBY_ABILITY_SPARK || SPARK_SLOT(c->kirby) != gCurTask - gTasks + 1) TaskDestroy(gCurTask);
}

// LookEnd(); A look gone, its VRAM given back;
static void LookEnd(struct ScrollLook *l) {
    if (l->effect != SCROLL_NO_LOOK && l->e.sprite.tilesVram >= 0x6014000 && SqueakVramOk(l->e.sprite.tilesVram))
        VramFree(l->e.sprite.tilesVram);
    l->effect = SCROLL_NO_LOOK;
}

/* LookStartAt(); A look playing an effect from its first step (none when KatAM's sprite heap can't hold it, as squeak.c's effects),
 * its sprite's order `order` (KatAM's effects' 0xA, in front of the Kirbys: SCROLL_BEHIND_KIRBYS, behind them); */
static void LookStartAt(struct ScrollPiece *p, u8 i, u8 effect, s8 dx, s8 dy, u16 order) {
    struct ScrollLook *l = &p->look[i];
    const struct SqueakEffect *d = &gSqueakData.effects[effect];
    u16 cell = d->anim.step[0].cell;

    LookEnd(l);
    if (VramGetTotalAllocatedTiles() + d->tiles + SCROLL_FX_ROOM > gVramHeapMaxTileSlots) return;
    ClearEffectObject(&l->e);
    l->e.header.kind = 3;
    l->e.roomId = p->base.roomId;
    l->e.parent = NULL;
    l->step = 0;
    l->timer = d->anim.step[0].frames;
    l->dx = dx;
    l->dy = dy;
    l->palId = SqueakLoadPalette(d->palette.key, d->palette.variant, p->base.roomId);
    EffectObjectInitSprite(&l->e, &l->e.sprite, d->tiles, d->animId, cell == SQUEAK_FX_HIDDEN ? 0 : cell, order);
    if (SqueakVramOk(l->e.sprite.tilesVram)) l->effect = effect;
}

// LookStart(); A look in front of the Kirbys (KatAM's effects' order);
static void LookStart(struct ScrollPiece *p, u8 i, u8 effect, s8 dx, s8 dy) {
    LookStartAt(p, i, effect, dx, dy, 0xA);
}

// LookDraw(); A look's step where its piece is, facing as it;
static void LookDraw(struct ScrollPiece *p, struct ScrollLook *l) {
    const struct SqueakAnim *a;
    const struct SqueakStep *step;
    bool32 left = p->base.flags & 1;

    if (l->effect == SCROLL_NO_LOOK) return;
    a = &gSqueakData.effects[l->effect].anim;
    step = &a->step[l->step];
    l->e.x = p->base.x + ((left ? -l->dx : l->dx) << 8);
    l->e.y = p->base.y + (l->dy << 8);
    l->e.roomId = p->base.roomId;
    if (step->cell == SQUEAK_FX_HIDDEN) {
        l->e.flags |= 0x400;
    } else {
        l->e.sprite.variant = step->cell;
        l->e.sprite.palId = l->palId;
        // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right);
        l->e.flags = (l->e.flags & ~0x401) | 4 | (left ? 0 : 1);
        l->e.objBase54 = left ? -step->x : step->x;
        l->e.objBase55 = step->y;
        EffectObjectPostUpdate(&l->e);
    }
    // (one that doesn't loop stays on its last step);
    if (--l->timer == 0) {
        if (l->step + 1 >= a->steps && !a->loops) {
            l->timer = 1;
            return;
        }
        l->step = l->step + 1 >= a->steps ? 0 : l->step + 1;
        l->timer = a->step[l->step].frames;
    }
}

// Box(); The piece's hit box this frame: the DS's (left, top, right, bottom from its origin, Y up), its strength; 0: none;
static void Box(struct ScrollPiece *p, s8 left, s8 top, s8 right, s8 bottom, s8 strength) {
    if (strength == 0) {
        p->base.flags |= 0x200;
        return;
    }
    p->base.flags &= ~(0x200 | 0x40000);
    p->base.unk63 = strength;
    sub_0803E2B0(&p->base, left, -top, right, -bottom);
    ObjectSetBounds(&p->base, left, -top, right, -bottom);
    SetPointerSomething(&p->base);
}

// Follow(); At its Kirby's feet, facing as he does;
static void Follow(struct ScrollPiece *p) {
    p->base.x = p->kirby->base.x;
    p->base.y = p->kirby->base.y + SCROLL_FEET;
    p->base.flags = (p->base.flags & ~1) | (p->kirby->base.flags & 1);
}

// AuraLook(); The aura's look by level (the DS's aura, 0x02087AA4: spark_ext's 30; composite 3), with its sound;
static void AuraLook(struct ScrollPiece *p, u8 level) {
    u8 i;

    for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
    if (level == 1) {
        LookStart(p, 0, SQUEAK_FX_SPARK_BITS, 0, 0);
    } else {
        LookStart(p, 0, SQUEAK_FX_SPARK_BITS_B, 0, 0);
        LookStart(p, 1, SQUEAK_FX_SPARK_BITS_A, 0, 0);
        LookStart(p, 2, SQUEAK_FX_SPARK_BALL, 0, 0);
    }
    p->level = level;
    p->clock = 0;
    PlaySfx(&p->kirby->base, SQUEAK_SE(level == 1 ? 0xB0 : 0xB1));
}

// SparkBallStart(); The level-2 aura flies off (0x02087AA4's: composite 4, 4 pixels up, sound 0xB2);
static void SparkBallStart(struct ScrollPiece *p) {
    u8 i;

    p->kind = SPARK_BALL;
    p->base.y -= SPARK_BALL_RISE;
    for (i = 0; i < SCROLL_LOOKS; i++) {
        if (p->look[i].effect == SCROLL_NO_LOOK) continue;
        p->look[i].step = 0;
        p->look[i].timer = gSqueakData.effects[p->look[i].effect].anim.step[0].frames;
    }
    p->clock = 0;
    PlaySfx(&p->base, SQUEAK_SE(0xB2));
}

// The level-2 aura's and the ball's hit boxes (their .rca's 43 and 44: 5 every 3 frames, the ball's first 20; 16 frames, looping);
static void AuraBox(struct ScrollPiece *p) {
    u8 frame = p->clock % 16;

    if (p->kind == SPARK_BALL && frame == 0) Box(p, -24, 29, 24, -15, 3);
    else if (frame % 3 == 0 && (p->kind == SPARK_BALL || frame < 15)) Box(p, -24, 31, 23, -15, 1);
    else Box(p, 0, 0, 0, 0, 0);
}

// AuraStep(); The aura: its Kirby's level's look, gone at level 0 or his attack (flying off at level 2 with forward held);
static bool32 AuraStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    struct SparkCharge *c = kirby->ability == KIRBY_ABILITY_SPARK ? ChargeOf(kirby, FALSE) : NULL;

    if (c != NULL && c->attack && c->ball && p->level == 2) {
        SparkBallStart(p);
        return TRUE;
    }
    if (c == NULL || c->attack || c->level == 0 || kirby->base.roomId != p->base.roomId) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    if (c->level != p->level) AuraLook(p, c->level);
    Follow(p);
    if (p->level == 2) AuraBox(p);
    return TRUE;
}

// BallStep(); The ball: forward 6 pixels a frame through everything, gone off the screen;
static bool32 BallStep(struct ScrollPiece *p) {
    const struct LevelInfo *level = &gCurLevelInfo[p->base.unk56];
    s32 x, y;

    p->base.x += p->base.flags & 1 ? -SPARK_BALL_SPEED : SPARK_BALL_SPEED;
    x = (p->base.x - level->viewportPosition.x) >> 8;
    y = (p->base.y - level->viewportPosition.y) >> 8;
    if (x < -SPARK_BALL_MARGIN || x > DISPLAY_WIDTH + SPARK_BALL_MARGIN || y < -SPARK_BALL_MARGIN || y > DISPLAY_HEIGHT + SPARK_BALL_MARGIN) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    if (p->clock == SPARK_BALL_SOUND_AT) PlaySfx(&p->base, SQUEAK_SE(0xAF));
    AuraBox(p);
    return TRUE;
}

/* BurstStep(); The level attack: its boxes (.rca's 41 and 42, every frame, one and the other) for 30 frames while KatAM's attack
 * lasts (anim 0x34), which it holds that long; then KatAM's spark if he's still at it (B held: the DS's base attack after it); */
static bool32 BurstStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;

    if (kirby->animationIndex != 0x34 || p->clock >= SPARK_BURST_FRAMES) {
        if (kirby->animationIndex == 0x34 && kirby->stateFn == sub_0805D630) gScrollSparkOriginal(kirby);
        m4aSongNumStop(SQUEAK_SE(p->level == 1 ? 0x51 : 0xB2));
        p->base.flags |= 0x1000;
        return FALSE;
    }
    // (KatAM's attack ends 16 frames on at the earliest, sub_0805D630's counter: so not before these 30);
    if (kirby->stateFn == sub_0805D630 && p->clock < SPARK_BURST_FRAMES - 17) kirby->base.counter = 0;
    Follow(p);
    if (p->level == 1) {
        if (p->clock & 1) Box(p, -46, 36, 45, -22, 4);
        else Box(p, -27, 51, 29, -36, 4);
    } else {
        if (p->clock & 1) Box(p, -30, 65, 32, -51, 6);
        else Box(p, -56, 37, 54, -24, 6);
    }
    return TRUE;
}

/* Beam. The DS's Beam Kirby's whip (0x02088B44, after its start-up): its .mca composite 0 (beam_ext's 23, 34 frames, sound 0xD3, hit
 * boxes of 15: its .rca's 27); with the scroll (HasScroll 3), composite 1: a bigger whip, sweeping from higher above him to lower in
 * front, 44 frames (beam_ext's 25, 4 pixels forward; sound 0xD4; hit boxes of 20, one of 9: .rca 28). The DS's Beam has no other move.
 * In KatAM, with the scroll: KatAM's whip (its hit box, sub_0807CFE4, and chain, sub_0807D2D0 and sub_0807D4B0, all three detoured,
 * from its attack state sub_08064094 only, not the ability-get show) is the DS's upgraded whip, a ScrollPiece (BEAM_WHIP): its look,
 * its boxes (strengths: KatAM's beam's 3 for the DS's 15, so 20 is 4, 9 is 2), its sound, its 44 frames (KatAM's whip is 35: its
 * attack, anim 52, is held to the whip's end, BeamStep); */
extern void sub_08064094(struct Kirby *kirby);
void (*gScrollBeamOriginal)(struct Kirby *) = NULL;
void (*gScrollChainOriginal)(struct Kirby *) = NULL;
void (*gScrollChain2Original)(struct Kirby *) = NULL;
#define BEAM_KNOCK 0x1C0                    // KatAM's beam's hit (sub_0807CFE4's unk64 and unk68);
#define BEAM_HIT 0x20000403
#define BEAM_WHIP_FRAMES 44

// sBeamBoxes; The upgraded whip's hit boxes (.rca 28): its frame, the box from Kirby's feet (Y up), KatAM's strength;
static const struct { u8 frame; s8 left, top, right, bottom; u8 strength; } sBeamBoxes[] = {
    { 2, 10, 33, 27, 9, 4 }, { 5, 8, 59, 26, 31, 4 }, { 8, 19, 48, 36, 19, 4 }, { 11, 32, 56, 46, 41, 4 }, { 12, 15, 31, 32, 7, 4 },
    { 14, 37, 48, 58, 18, 4 }, { 17, 26, 21, 55, 4, 4 }, { 19, 7, 15, 22, 4, 4 }, { 20, 35, 9, 69, -8, 4 },
    // (the DS's frame 23 has two: this, and frame 19's again, which KatAM's one box a frame leaves out);
    { 23, 31, 1, 62, -13, 4 }, { 26, 9, 6, 32, -10, 4 }, { 29, 24, -11, 54, -30, 4 }, { 30, 4, 14, 21, 2, 4 },
    { 32, 16, -15, 33, -31, 4 }, { 33, 6, 14, 20, -2, 4 }, { 35, 17, -23, 35, -40, 2 }, { 36, 8, 1, 19, -23, 4 },
    { 38, 6, -17, 23, -31, 4 }, { 39, 4, 16, 18, 0, 4 },
};

static bool32 BeamWhipped(struct Kirby *kirby) {
    return kirby->ability == KIRBY_ABILITY_BEAM && kirby->stateFn == sub_08064094 && SqueakScrollOwned(SCROLL_BEAM);
}

// BeamStep(); The upgraded whip: its boxes on their frames, KatAM's attack held its 44 frames, then ended;
static bool32 BeamStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    u8 i;

    if (kirby->stateFn != sub_08064094 || p->clock >= BEAM_WHIP_FRAMES) {
        // (his attack ends with it: anim 52's end);
        if (kirby->stateFn == sub_08064094) kirby->base.flags |= 2;
        p->base.flags |= 0x1000;
        return FALSE;
    }
    kirby->base.flags &= ~2;
    Follow(p);
    for (i = 0; i < ARRAY_COUNT(sBeamBoxes) && sBeamBoxes[i].frame != p->clock; i++) ;
    if (i < ARRAY_COUNT(sBeamBoxes))
        Box(p, sBeamBoxes[i].left, sBeamBoxes[i].top, sBeamBoxes[i].right, sBeamBoxes[i].bottom, sBeamBoxes[i].strength);
    else
        Box(p, 0, 0, 0, 0, 0);
    return TRUE;
}

// ScrollBeamStart(); sub_0807CFE4 (detoured): KatAM's whip's hit box starts; with the scroll, the DS's upgraded whip instead;
void ScrollBeamStart(struct Kirby *kirby) {
    struct ScrollPiece *p;

    if (!BeamWhipped(kirby) || (p = ScrollPieceCreate(kirby, BEAM_WHIP, 0, BEAM_KNOCK, BEAM_HIT)) == NULL) {
        gScrollBeamOriginal(kirby);
        return;
    }
    LookStart(p, 0, SQUEAK_FX_BEAM_WHIP, 4, 0);
    PlaySfx(&kirby->base, SQUEAK_SE(0xD4));
}

// ScrollBeamChain(), ScrollBeamChain2(); sub_0807D2D0 and sub_0807D4B0 (detoured): KatAM's whip's chain; none with the scroll;
void ScrollBeamChain(struct Kirby *kirby) {
    if (!BeamWhipped(kirby)) gScrollChainOriginal(kirby);
}

void ScrollBeamChain2(struct Kirby *kirby) {
    if (!BeamWhipped(kirby)) gScrollChain2Original(kirby);
}

/* Tornado. The DS's (start 0x020BB03C, start-up 0x020BAD4C, spin 0x020BA8F8): with the scroll (HasScroll 4), while the tornado has no
 * element yet it takes the one of what meets Kirby (0x02012E84: his +0x100, the kind of the attack box that met him: fire, water,
 * electricity), sound 0x93, and spins as that element's tornado (tornado_ext.mca's composites 8, 9 and 10: its sparks, anims 28, 32
 * and 36 at (-10, -30), over its body, 27, 31 and 35, in place of the plain one; a bigger hit box, 22 behind to 25 in front and 46 up,
 * of 25, the plain one's 20) to its end.
 * In KatAM, with the scroll: KatAM's tornado (its attack, sub_0807A10C, detoured: the piece starts with it; its task sub_0807A280)
 * takes the element of an attack or enemy that meets it whose hit kind (unk68) is fire (0x100, as Batafire's and KatAM's fire
 * breath's) or electricity (0x400, as KatAM's spark's), found in KatAM's collision lists (gUnk_02022F50, by room slot and kind,
 * counts gUnk_02022EB0) by a piece run just before KatAM's collision pass; KatAM's attack then is that element's (its kind bit, the
 * DS's bigger box, strength 3: KatAM's tornado's 2 for the DS's 20), the DS's element tornado drawn in place of KatAM's, till the spin
 * ends (anims 0x35, 0x34). Not water: KatAM's tornado stops in water (sub_08062A80); */
extern void sub_0807A280(void);
void (*gScrollTornadoOriginal)(struct Kirby *) = NULL;
#define TORNADO_FIRE 0x100
#define TORNADO_SPARK 0x400
#define TORNADO_STRENGTH 3
#define TORNADO_SPIN 0x34
#define TORNADO_START 0x35
#define TORNADO_HIDE 0x80000                // A sprite's flag (unk8) KatAM doesn't draw Kirby with (sub_0803F46C), his code never sets;

// IsKirby(); A Kirby (an attack of a Kirby's: its parent);
static bool32 IsKirby(void *p) {
    return (u32)p >= (u32)&gKirbys[0] && (u32)p < (u32)&gKirbys[4];
}

// Meets(); An object's hit box (unk38 left, unk39 top, unk3A and unk3B half its size, mirrored facing left) meets a box about (x, y);
static bool32 Meets(struct ObjectBase *o, s32 x, s32 y, s8 left, s8 top, s8 right, s8 bottom) {
    s32 w = o->unk3A * 2, h = o->unk3B * 2, l = o->flags & 1 ? (o->x >> 8) - o->unk38 - w : (o->x >> 8) + o->unk38;
    s32 t = (o->y >> 8) + o->unk39;

    return l < x + right && l + w > x + left && t < y + bottom && t + h > y + top;
}

// ElementTouch(); The element (TORNADO_FIRE, TORNADO_SPARK; fire first) of what meets a box about Kirby this frame, not a Kirby's;
// 0: none (the DS's 0x02012E84, from what met him);
static u16 ElementTouch(struct Kirby *kirby, s8 left, s8 top, s8 right, s8 bottom) {
    u8 slot = gCurLevelInfo[kirby->base.unk56].unk65E, kind, n, count;
    s32 x = kirby->base.x >> 8, y = kirby->base.y >> 8;
    u16 found = 0;

    if (slot >= 4) return 0;
    for (kind = 0; kind < 2; kind++) {
        count = gUnk_02022EB0[slot][kind];
        for (n = 0; n < count && n < 0x20; n++) {
            struct ObjectBase *o = gUnk_02022F50[slot * 0x40 + kind * 0x20 + n];

            if (o == NULL || (void *)o == (void *)kirby || IsKirby(o->parent) || o->roomId != kirby->base.roomId) continue;
            if (!(o->unk68 & (TORNADO_FIRE | TORNADO_SPARK)) || o->flags & (0x200 | 0x1000)) continue;
            if (!Meets(o, x, y, left, top, right, bottom)) continue;
            if (o->unk68 & TORNADO_FIRE) return TORNADO_FIRE;
            found = TORNADO_SPARK;
        }
    }
    return found;
}

/* TornadoStep(); With Kirby's tornado: its element taken once, KatAM's attack made that element's, the DS's element tornado drawn
 * in place of KatAM's own (its body and its sparks: KatAM's hidden); gone when the spin ends; */
static bool32 TornadoStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    struct ObjectBase *attack = p->other && p->other->main == sub_0807A280 ? TaskGetStructPtr(p->other) : NULL;
    u16 element;

    // (through the start-up, anim 0x35, as the DS takes it then too, and the spin, 0x34; not its end, 0x36);
    if (attack == NULL || attack->parent != kirby || attack->flags & 0x1000
        || (kirby->animationIndex != TORNADO_SPIN && kirby->animationIndex != TORNADO_START)) {
        p->base.flags |= 0x1000;
        if (p->level == 0) return FALSE;
        // (Kirby, already not drawn this frame, shown again from the next: the element tornado drawn this one last time);
        kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
        return TRUE;
    }
    Follow(p);
    p->base.roomId = kirby->base.roomId;
    // (KatAM's tornado's box, sub_0807A10C's);
    if (p->level == 0 && (element = ElementTouch(kirby, -10, -10, 10, 11)) != 0) {
        p->level = element == TORNADO_FIRE ? 1 : 3;
        attack->unk68 |= element;
        attack->unk63 = TORNADO_STRENGTH;
        // (the DS's box from his feet, Y up: -22, 46, 25, 0; KatAM's from his middle, 8 pixels up);
        sub_0803E2B0(attack, -22, -38, 25, 8);
        // (the composite's parts: its sparks first, on top, then its body at his feet);
        LookStart(p, 0, element == TORNADO_FIRE ? SQUEAK_FX_TORNADO_FIRE : SQUEAK_FX_TORNADO_SPARK, -10, -30);
        LookStart(p, 1, element == TORNADO_FIRE ? SQUEAK_FX_TORNADO_FIRE_BODY : SQUEAK_FX_TORNADO_SPARK_BODY, 0, 0);
        PlaySfx(&kirby->base, SQUEAK_SE(0x93));
    }
    // (KatAM's own tornado not drawn while the DS's body is: none when its sprite couldn't be made);
    if (p->look[1].effect != SCROLL_NO_LOOK) kirby->base.sprite.unk8 |= TORNADO_HIDE;
    Box(p, 0, 0, 0, 0, 0);
    return TRUE;
}

// ScrollTornadoStart(); sub_0807A10C (detoured): KatAM's tornado's attack starts; with the scroll, the piece that gives it its element;
void ScrollTornadoStart(struct Kirby *kirby) {
    struct ScrollPiece *p;
    u8 i;

    gScrollTornadoOriginal(kirby);
    if (kirby->ability != KIRBY_ABILITY_TORNADO || !SqueakScrollOwned(SCROLL_TORNADO)) return;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];

        if (t->main != sub_0807A280 || ((struct ObjectBase *)TaskGetStructPtr(t))->parent != kirby) continue;
        if ((p = ScrollPieceCreate(kirby, TORNADO_ELEMENT, 0, 0, 0)) != NULL) p->other = t;
        return;
    }
}

/* Parasol. The DS's (its Kirby's attacks are its parasol_ext's animations): its swing (0x020B97BC, 21 and 22, sound 0xA2) and, B
 * held after it, its twirl (0x020B92F0, 0x020B90A8: 26, then 27, the parasol held out forward spinning as KatAM's, a box of 8, .rca
 * 27, till B is let go, 0x020B8F18). With the scroll (HasScroll 5) the twirl is
 * parasol_ext.mca's composite 2 (93 frames, a loop: Kirby thrusting his parasol forward, stars bursting from its tip), sound 0xA3 at its
 * start and every 37 frames while B is held, its boxes (.rca 40) one frame in three: 13 to 33-63 pixels in front, 12 under his feet to
 * 30 over them, of 10 the first and 8 the rest (the plain twirl's 8).
 * In KatAM, with the scroll: KatAM's twirl (B held after the swing, sub_0806487C: anims 104, 105; its attack, sub_08084188, detoured:
 * strength 1, 10 to 38 in front, again every 7 frames) keeps its look, Kirby's own, and gets the DS's stars about him, its boxes in
 * place of KatAM's (strength 1: the DS's 8 and 10, the plain twirl's) and its sound, till it ends. Not in water (anims 69, 70: the DS's
 * water parasol is a file of its own, parasol_wat); */
extern void sub_08084280(void);
void (*gScrollParasolOriginal)(struct Kirby *) = NULL;
#define PARASOL_TWIRL_START 104
#define PARASOL_TWIRL 105
#define PARASOL_FRAMES 93
#define PARASOL_SOUND_EVERY 37
#define PARASOL_KNOCK 0x120                 // KatAM's twirl's hit (sub_08084188's unk64, unk66 and unk68);
#define PARASOL_KNOCK_Y 0x100
#define PARASOL_HIT 0x20000043

// sParasolReach; The thrust's boxes' right edges (.rca 40: a box on the first frame of each three, 13 in front, 30 up to 12 down);
static const u8 sParasolReach[] = { 33, 33, 39, 39, 55, 55, 55, 55, 63, 63, 63, 47, 47, 47, 35, 53, 53, 53, 61, 60, 60, 60, 40, 49, 40, 47,
                                    55, 57, 57, 57, 61 };

// ParasolStep(); With KatAM's twirl: the DS's stars and boxes, KatAM's own box off, the DS's sound; gone when the twirl ends;
static bool32 ParasolStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    struct ObjectBase *attack = p->other && p->other->main == sub_08084280 ? TaskGetStructPtr(p->other) : NULL;
    u16 time = p->clock % PARASOL_FRAMES;

    if (attack == NULL || attack->parent != kirby || attack->flags & 0x1000 || kirby->ability != KIRBY_ABILITY_PARASOL
        || (kirby->animationIndex != PARASOL_TWIRL_START && kirby->animationIndex != PARASOL_TWIRL)) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    Follow(p);
    p->base.roomId = kirby->base.roomId;
    attack->flags |= 0x200;
    if (p->clock % PARASOL_SOUND_EVERY == 0) PlaySfx(&kirby->base, SQUEAK_SE(0xA3));
    if (time % 3 == 0) Box(p, 13, 30, sParasolReach[time / 3], -12, 1);
    else Box(p, 0, 0, 0, 0, 0);
    return TRUE;
}

// ScrollParasolTwirl(); sub_08084188 (detoured): KatAM's twirl's attack starts; with the scroll, on land, the piece of the DS's;
void ScrollParasolTwirl(struct Kirby *kirby) {
    struct ScrollPiece *p;
    u8 i;

    gScrollParasolOriginal(kirby);
    if (kirby->ability != KIRBY_ABILITY_PARASOL || kirby->animationIndex != PARASOL_TWIRL_START || !SqueakScrollOwned(SCROLL_PARASOL)) return;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];

        if (t->main != sub_08084280 || ((struct ObjectBase *)TaskGetStructPtr(t))->parent != kirby) continue;
        if ((p = ScrollPieceCreate(kirby, PARASOL_THRUST, 0, PARASOL_KNOCK, PARASOL_HIT)) == NULL) return;
        p->other = t;
        p->base.unk66 = PARASOL_KNOCK_Y;
        LookStart(p, 0, SQUEAK_FX_PARASOL_STARS, 0, 0);
        return;
    }
}

static void ScrollPieceMain(void) {
    struct ScrollPiece *p = TaskGetStructPtr(gCurTask);
    bool32 alive;
    u8 i;

    if (ObjectPreUpdate(&p->base)) return;
    alive = p->kind == SPARK_AURA ? AuraStep(p) : p->kind == SPARK_BALL ? BallStep(p) : p->kind == BEAM_WHIP ? BeamStep(p)
          : p->kind == TORNADO_ELEMENT ? TornadoStep(p) : p->kind == PARASOL_THRUST ? ParasolStep(p) : p->kind == CUTTER_BLADE ? CutterStep(p)
          : p->kind == LASER_CHARGE ? LaserStep(p) : p->kind == LASER_BOLT ? BoltStep(p) : p->kind == UFO_STAR ? StarStep(p) : p->kind == HI_JUMP_ORB ? OrbStep(p) : p->kind == WHEEL_ELEMENT ? WheelStep(p)
          : BurstStep(p);
    if (!alive) return;
    for (i = 0; i < SCROLL_LOOKS; i++) LookDraw(p, &p->look[i]);
    p->clock++;
}

static void ScrollPieceDestroy(struct Task *t) {
    struct ScrollPiece *p = TaskGetStructPtr(t);
    u8 i;

    if (p->kind == TORNADO_ELEMENT || p->kind == WHEEL_ELEMENT) p->kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
    for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
}

/* ScrollPieceCreate(); A piece at its Kirby's feet, its hit's kind and knock back KatAM's own attack's (unk68, unk64: the spark's,
 * sub_08074AB0, SPARK_HIT and SPARK_KNOCK; the beam whip's, sub_0807CFE4, BEAM_HIT and BEAM_KNOCK); */
static struct ScrollPiece *ScrollPieceCreate(struct Kirby *kirby, u8 kind, u8 level, s16 knock, u32 hitKind) {
    // (the tornado's runs just before KatAM's collision pass, 0xFFFD: every object has put itself in its lists by then);
    struct Task *t = TaskCreate(ScrollPieceMain, sizeof(struct ScrollPiece), kind == TORNADO_ELEMENT ? 0xFFF0 : 0x3500, TASK_USE_EWRAM,
                                ScrollPieceDestroy);
    struct ScrollPiece *p;
    u8 i;

    if (t == &gEmptyTask) return NULL;
    p = TaskGetStructPtr(t);
    ClearObjectBase(&p->base);
    for (i = 0; i < SCROLL_LOOKS; i++) p->look[i].effect = SCROLL_NO_LOOK;
    p->kirby = kirby;
    p->other = NULL;
    p->kind = kind;
    p->level = level;
    p->clock = 0;
    p->base.header.kind = 2;
    p->base.parent = kirby;
    p->base.counter = 0;
    p->base.roomId = kirby->base.roomId;
    p->base.unk56 = kirby->base.unk56;
    if (Macro_0810B1F4(&p->base)) p->base.flags |= 0x2000;
    p->base.unk64 = knock;
    p->base.unk66 = 0;
    p->base.unk63 = 0;
    p->base.flags |= 0xA0000100 | 0x400 | 0x200;
    p->base.unk68 = hitKind;
    Follow(p);
    return p;
}

// Glint(); spark_cmn's 58 at random in his body's box (spark_cmn's .rca: -6 to 6 across, 2 to 14 up from his feet);
static void Glint(struct Kirby *kirby) {
    s32 dx = (Rand16() % 12) - 6, dy = 2 + Rand16() % 12;

    if (kirby->base.flags & 1) dx = -dx;
    SqueakEffectInRoom(kirby->base.roomId, SQUEAK_FX_SPARK_GLINT, kirby->base.x + (dx << 8), kirby->base.y + SCROLL_FEET - (dy << 8));
}

// SparkCharging(); The DS's 0x02088444 (see above), each frame, after his input's read;
static void SparkCharging(struct Kirby *kirby) {
    struct SparkCharge *c = ChargeOf(kirby, TRUE);
    bool32 attacking = kirby->stateFn == sub_0805D630, blocked;

    if (c == NULL) return;
    c->attack = FALSE;
    // (the DS's: a hurt, the water, no control: KatAM's in water, or with his input off);
    blocked = kirby->base.unk58 & 2 || kirby->base.flags & 0x1000000;
    if (blocked) {
        c->level = 0;
    } else if (kirby->unk11A & SPARK_D_PAD) {
        if (!attacking && c->charge < SPARK_CHARGE_MOST) {
            c->charge += SPARK_TAP;
            if (c->level == 0) {
                if (c->charge > SPARK_LEVEL_1) {
                    ScrollPieceCreate(kirby, SPARK_AURA, 0, SPARK_KNOCK, SPARK_HIT);
                    c->charge = SPARK_TAP;
                    c->level = 1;
                }
            } else if (c->level == 1 && c->charge > SPARK_LEVEL_2) {
                c->charge = SPARK_TAP;
                c->level = 2;
            }
        }
        PlaySfx(&kirby->base, SQUEAK_SE(Rand16() % 100 < 50 ? 0xB0 : 0xB1));
    }
    if (c->charge < 1) {
        if (c->level == 2) {
            c->charge = SPARK_LEVEL_2;
            c->level = 1;
        }
    } else {
        c->charge--;
    }
    // (the DS's attack state ends them every frame, 0x02087FC0);
    if (attacking) c->level = c->charge = 0;
    if (!blocked && ++c->glint > SPARK_GLINT_WAIT) {
        c->glint = 0;
        Glint(kirby);
    }
}

/* Cupid. The DS's Cupid Kirby (an ability the DS keeps but doesn't hand out; PrmKirbyAngel.kgb): his shot (0x020BE864) is a single
 * arrow, aimed and charged while B is held (0x020BE5A4: types 0, 4 and 8, straight, up, down; their speed between words 18 and 20
 * by the charge, word 16's fall, sound 0xED); with the scroll (HasScroll 7) and an input no button sets (`+0x28`: the owner chose
 * it always on) it is a volley as the shot's start ends (0x020BE724): three arrows at once (0x020BE28C, types 12, 13 and 14 of the
 * table 0x021592B0), from words 61-62, 65-66 and 69-70 (20 forward and 16 up from his feet, 14 and 10, 8 and 4), all straight on
 * (word 57's angle, 0), at word 63's speed (6 pixels a frame) with no fall (word 59, 0), sound 0xEC; not charged, not aimed.
 * In KatAM, with the scroll: KatAM's shot (sub_080680C0, detoured: his pose, sound and arrow, sub_08081864; three when charged to
 * 0x30, KatAM's own triple, the first of strength 5 past it) is the DS's volley: three of KatAM's arrows from the DS's places,
 * straight on at the DS's speed (each kept so, CupidArrowMain, against the arrow's own slowing and fall, sub_08081EB0, till it
 * sticks or ends), of KatAM's strength (3; 5 charged past 0x30), with KatAM's sound and stuck arrows; one volley a shot. Not
 * aimed: the DS's isn't; */
extern void sub_08081864(struct Kirby *kirby, s16 charge, u8 aim);
extern void sub_08081EB0(void);
void (*gScrollCupidOriginal)(struct Kirby *) = NULL;
#define CUPID_SPEED SQUEAK_SPEED(24576)     // PrmKirbyAngel.kgb's word 63 (67, 71): 6 pixels a frame;
#define CUPID_SLOWING 6                     // sub_08081EB0's slowing and fall a frame (in water 0xC), undone each frame;
#define CUPID_FALL 0x18
#define CUPID_FALL_WATER 0xC
#define CUPID_CHARGED 0x30                  // sub_080680C0: from it, KatAM's two more arrows;
#define CUPID_ARROWS_MOST 8

// gCupidVolley; The DS's three arrows' places, forward and up from his feet (PrmKirbyAngel.kgb's words 61-62, 65-66, 69-70);
static const s8 gCupidVolley[3][2] = { { 20, 16 }, { 14, 10 }, { 8, 4 } };

struct CupidArrow {
    struct Task *arrow;
};

// CupidArrowAim(); An arrow's speed set so that, after its own slowing and fall this frame, it goes straight on at the DS's speed;
static void CupidArrowAim(struct ObjectBase *o) {
    o->xspeed = o->flags & 1 ? -(CUPID_SPEED + CUPID_SLOWING) : CUPID_SPEED + CUPID_SLOWING;
    o->yspeed = o->unkC & 1 ? CUPID_FALL_WATER : CUPID_FALL;
}

// CupidArrowMain(); Each frame while its arrow flies (sub_08081EB0): kept straight on; gone once it sticks or ends;
static void CupidArrowMain(void) {
    struct CupidArrow *c = TaskGetStructPtr(gCurTask);
    struct ObjectBase *o = TaskGetStructPtr(c->arrow);

    if (c->arrow->main != sub_08081EB0 || o->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    CupidArrowAim(o);
}

// IsKirbyArrow(); A task of a Kirby's flying arrow;
static bool32 IsKirbyArrow(struct Task *t, struct Kirby *kirby) {
    return t->main == sub_08081EB0 && ((struct ObjectBase *)TaskGetStructPtr(t))->parent == kirby;
}

// ScrollCupidShot(); sub_080680C0 (detoured): KatAM's shot; with the scroll, the DS's volley of three in its arrows' place;
void ScrollCupidShot(struct Kirby *kirby) {
    struct Task *before[CUPID_ARROWS_MOST];
    s16 charge = kirby->base.counter;
    u8 i, j, n = 0, made = 0;

    if (!SqueakScrollOwned(SCROLL_CUPID) || kirby->ability != KIRBY_ABILITY_CUPID) {
        gScrollCupidOriginal(kirby);
        return;
    }
    for (i = 0; i < MAX_TASK_NUM && n < CUPID_ARROWS_MOST; i++)
        if (IsKirbyArrow(&gTasks[i], kirby)) before[n++] = &gTasks[i];
    // (KatAM's pose, sound and first arrow, at the shot's strength; not its two more: the volley's two in their place);
    if (charge >= CUPID_CHARGED) kirby->base.counter = CUPID_CHARGED - 1;
    gScrollCupidOriginal(kirby);
    for (j = 0; j < 2; j++) sub_08081864(kirby, charge, 1);
    if (charge >= CUPID_CHARGED) kirby->base.counter = charge - 2 * 0x18;
    for (i = 0; i < MAX_TASK_NUM && made < 3; i++) {
        struct Task *t = &gTasks[i], *keep;
        struct ObjectBase *o;

        if (!IsKirbyArrow(t, kirby)) continue;
        for (j = 0; j < n && before[j] != t; j++) ;
        if (j < n) continue;
        o = TaskGetStructPtr(t);
        // (KatAM's strength by the shot's charge, sub_08081864's: past 0x30, 5);
        if (charge > CUPID_CHARGED) {
            o->unk63 = 5;
            o->unk64 = 0x200;
        }
        o->x = kirby->base.x + (o->flags & 1 ? -gCupidVolley[made][0] : gCupidVolley[made][0]) * 0x100;
        o->y = kirby->base.y + SCROLL_FEET - gCupidVolley[made][1] * 0x100;
        CupidArrowAim(o);
        keep = TaskCreate(CupidArrowMain, sizeof(struct CupidArrow), 0x3500, TASK_USE_EWRAM, NULL);
        if (keep != &gEmptyTask) ((struct CupidArrow *)TaskGetStructPtr(keep))->arrow = t;
        made++;
    }
}

/* Cutter. The DS's cutter (0x020BA1C8, as Kirby throws it): with the scroll (HasScroll 8) it is cutter_ext.mca's composite 0
 * (0x020B9F18: a big blue blade spinning, 25, sparkles about it, 27) for its 23 (a small yellow one), its box 39 x 27 (.rca: -20 to 19
 * across, -13 to 14 up) for 27 x 20 (-14 to 13, -9 to 11), the same damage (20) and flight.
 * In KatAM, with the scroll: KatAM's cutter (thrown by sub_08078EFC, detoured: its task sub_08070580, strength 4, its box 24 x 15, -12
 * to 12 across, -7 to 8) flies as it does, drawn as the DS's (KatAM's own not drawn: its flag 0x400), its box the DS's, as its look:
 * -20 to 19 across, -14 to 13 (KatAM's Y down), its strength KatAM's (the DS's damage the plain cutter's); */
extern void sub_08070580(void);
void (*gScrollCutterOriginal)(struct Kirby *) = NULL;
#define CUTTER_HIT 0x20002003               // KatAM's cutter's hit (sub_08078EFC's unk68);
#define CUTTER_MOST 4                       // (cutters of his in flight at once, looked for);

// CutterStep(); The DS's cutter drawn where KatAM's flies (its look and box KatAM's otherwise); gone with it;
static bool32 CutterStep(struct ScrollPiece *p) {
    struct ObjectBase *cutter = p->other && p->other->main == sub_08070580 ? TaskGetStructPtr(p->other) : NULL;

    if (cutter == NULL || cutter->parent != p->kirby || cutter->flags & 0x1000 || cutter->unk68 != CUTTER_HIT) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    p->base.x = cutter->x;
    p->base.y = cutter->y;
    p->base.roomId = cutter->roomId;
    p->base.flags = (p->base.flags & ~1) | (cutter->flags & 1);
    cutter->flags |= 0x400;
    Box(p, 0, 0, 0, 0, 0);
    return TRUE;
}

// IsKirbyCutter(); A task of a Kirby's thrown cutter;
static bool32 IsKirbyCutter(struct Task *t, struct Kirby *kirby) {
    struct ObjectBase *o;

    if (t->main != sub_08070580) return FALSE;
    o = TaskGetStructPtr(t);
    return o->parent == kirby && o->unk68 == CUTTER_HIT && !(o->flags & 0x1000);
}

// ScrollCutterThrow(); sub_08078EFC (detoured): KatAM's cutter thrown; with the scroll, the new one drawn as the DS's, its box grown;
void ScrollCutterThrow(struct Kirby *kirby) {
    struct Task *before[CUTTER_MOST];
    struct ScrollPiece *p;
    u8 i, j, n = 0;

    for (i = 0; i < MAX_TASK_NUM && n < CUTTER_MOST; i++)
        if (IsKirbyCutter(&gTasks[i], kirby)) before[n++] = &gTasks[i];
    gScrollCutterOriginal(kirby);
    if (kirby->ability != KIRBY_ABILITY_CUTTER || !SqueakScrollOwned(SCROLL_CUTTER)) return;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];

        if (!IsKirbyCutter(t, kirby)) continue;
        for (j = 0; j < n && before[j] != t; j++) ;
        if (j < n) continue;
        sub_0803E2B0(TaskGetStructPtr(t), -20, -14, 19, 13);
        if ((p = ScrollPieceCreate(kirby, CUTTER_BLADE, 0, 0, 0)) == NULL) return;
        p->other = t;
        LookStart(p, 0, SQUEAK_FX_CUTTER_SPARKLES, 0, 0);
        LookStart(p, 1, SQUEAK_FX_CUTTER_BLADE, 0, 0);
        CutterStep(p);
        return;
    }
}

/* Laser. The DS's Laser Kirby's attack (0x0209A0C0: laser_ext's 25, then 0x02099FB0: the shot, the object 0x020373F8, laser_ext's
 * 21-23, its .rca's boxes of 20, sound 0x95, and Kirby's composite 0, 15 frames, 0x02099E94); with the scroll (HasScroll 9), B still
 * held as composite 0 ends: the charge (0x02099D00: composite 1, laser_ext's 33-36, sparkles closing in on the gun at (6, -11), and
 * the DS's Kirby, flat, 30 and 31; sound 0x138 every 5 frames), charged past 29 frames (composite 2: and 32, an orb flashing blue and
 * gold at (0, -8)). B let go: not charged, the end; charged, composite 3 (0x02099BF0: 38 and 39, sparkles at (5, -18), and his 37:
 * 6 frames), then the charged shot (the same object, its type 1: laser_ext's 42-44, a thicker bolt, its boxes of 30 for 20, as big;
 * sound 0x95; through enemies: its hit 0x02037280 ends only the plain shot, type 0) and composite 4 (0x02099B78: 41, a flash, 28, a ring bursting, and his 40: 20 frames), then the end.
 * In KatAM, with the scroll: KatAM's laser (sub_0806A520: the start-up sub_08063E50, its shot sub_0807BCE0, then sub_08063F74, which
 * ends past its counter's 8), B still held as it ends, is held there (the counter) while he charges, KatAM's Kirby drawn as he is
 * (his laser pose), the DS's sparkles about him, its sound, its orb once charged; let go charged, its sparkles for 6 frames, then
 * KatAM's shot (its flight, its turns on slopes, its box, its sound), its strength the DS's charged one (KatAM's laser's 2 for the DS's
 * 20: 30 is 3), through enemies (its hit, unk7C, ChargedShotHit: KatAM's own burst and end only on terrain), drawn as the DS's charged
 * bolt (KatAM's own not drawn), the flash and ring about him for 20 frames; then the end. The
 * charge is a piece (LASER_CHARGE: LASER_SLOT, his ability's byte as Spark's) worked from his input each frame (ScrollKirbyInput); */
extern void sub_08063F74(struct Kirby *kirby);
#define LASER_SLOT(kirby) NIDL_MIKE_USES(kirby)
#define LASER_HIT 0x20000103                // KatAM's laser's hit (sub_0807BCE0's unk68);
#define LASER_SHOT_END 8                    // sub_08063F74 ends past its counter's 8;
#define LASER_CHARGED 30                    // 0x02099D00: past 29 frames (its timer from 0);
#define LASER_HUM 5                         // its sound 0x138, each 5 frames from the first;
#define LASER_LET_GO_FRAMES 6               // composite 3's (laser_ext's 37: 6 frames);
#define LASER_RECOIL_FRAMES 20              // composite 4's (40: 20 frames);
#define LASER_STRENGTH 3                    // The charged shot's (sub_0807BCE0's unk63, 2, for the DS's 20: its 30);
#define LASER_SHOTS_MOST 4                  // (his shots in flight at once, looked for);
#define B_HELD 2                            // (movementState's B);
enum { LASER_CHARGING, LASER_LET_GO, LASER_RECOIL };

// IsKirbyShot(); A task of a Kirby's thrown cutter or laser shot (KatAM's sub_08070580 objects, by their hit);
static bool32 IsKirbyShot(struct Task *t, struct Kirby *kirby, u32 hit) {
    struct ObjectBase *o;

    if (t->main != sub_08070580) return FALSE;
    o = TaskGetStructPtr(t);
    return o->parent == kirby && o->unk68 == hit && !(o->flags & 0x1000);
}

// TaskNumberOf(); A piece's task's number + 1 (0: none);
static u8 TaskNumberOf(struct ScrollPiece *p) {
    u8 i;

    for (i = 0; i < MAX_TASK_NUM; i++)
        if (gTasks[i].main == ScrollPieceMain && TaskGetStructPtr(&gTasks[i]) == p) return i + 1;
    return 0;
}

// LaserOf(); A Kirby's charge (LASER_SLOT: its task's number + 1, checked); NULL: none;
static struct ScrollPiece *LaserOf(struct Kirby *kirby) {
    u8 slot = LASER_SLOT(kirby);
    struct ScrollPiece *p;

    if (slot == 0 || slot > MAX_TASK_NUM || gTasks[slot - 1].main != ScrollPieceMain) return NULL;
    p = TaskGetStructPtr(&gTasks[slot - 1]);
    return p->kind == LASER_CHARGE && p->kirby == kirby && !(p->base.flags & 0x1000) ? p : NULL;
}

// LaserLooks(); The charge's looks for a phase (the DS's composites' parts but its Kirby's);
static void LaserLooks(struct ScrollPiece *p, u8 phase) {
    u8 i;

    for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
    if (phase == LASER_CHARGING) {
        for (i = 0; i < 4; i++) LookStart(p, i, SQUEAK_FX_LASER_SPARKLE_A + i, 6, -11);
    } else if (phase == LASER_LET_GO) {
        LookStart(p, 0, SQUEAK_FX_LASER_GLINT_A, 5, -18);
        LookStart(p, 1, SQUEAK_FX_LASER_GLINT_B, 5, -18);
    } else {
        LookStart(p, 0, SQUEAK_FX_LASER_FLASH, 0, 0);
        LookStart(p, 1, SQUEAK_FX_LASER_RING, 0, 0);
    }
    p->level = phase;
    p->base.counter = 0;
}

// LaserStep(); The charge: at his feet, facing as he does; gone once he's out of his laser's state (hurt, the water...);
static bool32 LaserStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;

    if (kirby->ability != KIRBY_ABILITY_LASER || kirby->stateFn != sub_08063F74 || kirby->base.roomId != p->base.roomId) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    Follow(p);
    return TRUE;
}

// BoltStep(); The DS's charged bolt drawn where KatAM's shot flies, across, up or down as it goes (KatAM's not drawn); gone with it;
static bool32 BoltStep(struct ScrollPiece *p) {
    struct ObjectBase *shot = p->other && p->other->main == sub_08070580 ? TaskGetStructPtr(p->other) : NULL;
    u8 look;

    if (shot == NULL || shot->parent != p->kirby || shot->flags & 0x1000 || shot->unk68 != LASER_HIT) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    p->base.x = shot->x;
    p->base.y = shot->y;
    p->base.roomId = shot->roomId;
    p->base.flags = (p->base.flags & ~1) | (shot->flags & 1);
    shot->flags |= 0x400;
    // (KatAM's speeds: y up);
    look = shot->xspeed != 0 ? SQUEAK_FX_LASER_BOLT : shot->yspeed > 0 ? SQUEAK_FX_LASER_BOLT_UP : SQUEAK_FX_LASER_BOLT_DOWN;
    if (p->look[0].effect != look) LookStart(p, 0, look, 0, 0);
    return TRUE;
}

/* ChargedShotHit(); The charged shot's hit (its unk7C, sub_08070580's on its flags 0x40000 or 0x80000): an enemy hit (0x40000 without
 * the terrain's 0x800, sub_0807C1A0's) goes on through, as KatAM's own sub_08084C34 lets a shot; else KatAM's laser's (sub_08084BD8:
 * its burst, 0x28C, and the end); */
static bool8 ChargedShotHit(struct Unk_080C4EDC *shot) {
    if ((shot->base.flags & (0x40000 | 0x80000 | 0x800)) == 0x40000) {
        shot->base.flags &= ~0x40000;
        return FALSE;
    }
    CreateEffectObject(&shot->base, 0, 0x28C, 2);
    shot->base.flags |= 0x1000;
    return TRUE;
}

// LaserShoot(); KatAM's shot, charged: its strength, through enemies, the DS's bolt following it;
static void LaserShoot(struct Kirby *kirby) {
    struct Task *before[LASER_SHOTS_MOST];
    struct ScrollPiece *b;
    u8 i, j, n = 0;

    for (i = 0; i < MAX_TASK_NUM && n < LASER_SHOTS_MOST; i++)
        if (IsKirbyShot(&gTasks[i], kirby, LASER_HIT)) before[n++] = &gTasks[i];
    sub_0807BCE0(kirby);
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];

        if (!IsKirbyShot(t, kirby, LASER_HIT)) continue;
        for (j = 0; j < n && before[j] != t; j++) ;
        if (j < n) continue;
        ((struct ObjectBase *)TaskGetStructPtr(t))->unk63 = LASER_STRENGTH;
        ((struct Unk_080C4EDC *)TaskGetStructPtr(t))->unk7C = ChargedShotHit;
        if ((b = ScrollPieceCreate(kirby, LASER_BOLT, 0, 0, 0)) == NULL) return;
        b->other = t;
        BoltStep(b);
        return;
    }
}

// LaserCharging(); The DS's 0x02099E94-0x02099B78 (see above), each frame, after his input's read, before his state;
static void LaserCharging(struct Kirby *kirby) {
    struct ScrollPiece *p = LaserOf(kirby);
    bool32 held = (kirby->movementState & B_HELD) != 0;

    if (p == NULL) {
        if (kirby->stateFn != sub_08063F74 || kirby->base.counter <= LASER_SHOT_END || !held) return;
        if ((p = ScrollPieceCreate(kirby, LASER_CHARGE, LASER_CHARGING, 0, 0)) == NULL) return;
        LASER_SLOT(kirby) = TaskNumberOf(p);
        LaserLooks(p, LASER_CHARGING);
    }
    if (kirby->stateFn != sub_08063F74) {
        p->base.flags |= 0x1000;
        return;
    }
    if (p->level == LASER_CHARGING) {
        if (p->base.counter == LASER_CHARGED) {
            // (the DS's composite 2 draws it under its Kirby: its last part);
            LookStartAt(p, 4, SQUEAK_FX_LASER_ORB, 0, -8, SCROLL_BEHIND_KIRBYS);
        }
        if (!held) {
            if (p->base.counter < LASER_CHARGED) {
                // (let go before it's charged: his laser ends, this frame);
                p->base.flags |= 0x1000;
                return;
            }
            LaserLooks(p, LASER_LET_GO);
        } else if (p->base.counter % LASER_HUM == 0) {
            PlaySfx(&kirby->base, SQUEAK_SE(0x138));
        }
    } else if (p->level == LASER_LET_GO && p->base.counter >= LASER_LET_GO_FRAMES) {
        LaserShoot(kirby);
        LaserLooks(p, LASER_RECOIL);
    } else if (p->level == LASER_RECOIL && p->base.counter >= LASER_RECOIL_FRAMES) {
        p->base.flags |= 0x1000;
        return;
    }
    // (his laser's state held: it ends past LASER_SHOT_END);
    kirby->base.counter = LASER_SHOT_END;
    p->base.counter++;
}

/* UFO. The DS's UFO Kirby (each frame, 0x020BD560): with the scroll (HasScroll 13), while he has none, a star (0x020BB85C; its state
 * 0x020BB488: ufo_ext's 36, a sparkling star; its .rca's box, -6 to 5 across, -5 to 5 up, of 10, every other frame) circles him: two angles from 0, prmKirbyUfo.kgb's words 21 (4 degrees) and 22 (3) on each frame; at the first, word 20 (22
 * pixels) from his feet, its height times word 23 (1), drawn in front of him while its sine is at most 0, behind him else; turned
 * about his feet by the second. It goes (0x020BB7A8: 0x020B48E0's star burst, efc_cmn0's 7) once he's gone or out of play (his
 * flags 0x80000000, 0x2000000), a new one coming the next frame.
 * In KatAM, with the scroll: UFO Kirby has the DS's star circling him, a piece (UFO_STAR: UFO_SLOT, his ability's byte as Spark's)
 * made from his input each frame while he has none (ScrollKirbyInput), gone with the star burst once he isn't UFO Kirby, leaves its
 * room or is out of play (his flags 0x2000000); its box every other frame, strength 2 (KatAM's UFO laser's 4, sub_0807BF2C, for the
 * DS's 20: 10 is 2), its hit's kind and knock the UFO laser's (KatAM's attacks hit by their kind); */
#define UFO_SLOT(kirby) NIDL_MIKE_USES(kirby)
#define UFO_RADIUS 22                       // prmKirbyUfo.kgb's word 20 (90112);
#define UFO_SPIN_1 (4 << 12)                // its words 21 and 22 (16384, 12288: 20.12 degrees a frame);
#define UFO_SPIN_2 (3 << 12)
#define UFO_TURN (360 << 12)
#define UFO_LAP 360
#define UFO_STAR_HIT 0x20000103             // KatAM's UFO laser's kind (sub_0807BF2C's unk68: an attack needs a kind to hit);
#define UFO_STAR_KNOCK 0x1C0                // (sub_0807BF2C's unk64);
#define UFO_STAR_STRENGTH 2

// Sine(), Cosine(); Squeak Squad's sine table (0x0215CE20, 4.12) by KatAM's (gSineTable: 1024 a turn, 2.14) at an angle (20.12 degrees);
static s32 Sine(s32 angle) {
    return gSineTable[(angle * 2 / 45 >> 6) & 0x3FF] >> 2;
}

static s32 Cosine(s32 angle) {
    return gSineTable[((angle * 2 / 45 >> 6) & 0x3FF) + 0x100] >> 2;
}

// StarOf(); A Kirby's star (UFO_SLOT: its task's number + 1, checked); NULL: none;
static struct ScrollPiece *StarOf(struct Kirby *kirby) {
    u8 slot = UFO_SLOT(kirby);
    struct ScrollPiece *p;

    if (slot == 0 || slot > MAX_TASK_NUM || gTasks[slot - 1].main != ScrollPieceMain) return NULL;
    p = TaskGetStructPtr(&gTasks[slot - 1]);
    return p->kind == UFO_STAR && p->kirby == kirby && !(p->base.flags & 0x1000) ? p : NULL;
}

// StarStep(); The star circling its Kirby (0x020BB488), in front of him or behind; gone with a star burst (0x020BB7A8);
static bool32 StarStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    s32 a1, a2, x, y, sn, cs;

    // (both angles back where they started every 360 frames: 90 and 120 frames a turn);
    if (p->clock >= UFO_LAP) p->clock -= UFO_LAP;
    if (kirby->ability != KIRBY_ABILITY_UFO || kirby->base.roomId != p->base.roomId || kirby->base.flags & 0x2000000
        || !SqueakScrollOwned(SCROLL_UFO)) {
        SqueakEffectInRoom(p->base.roomId, SQUEAK_FX_STARS, p->base.x, p->base.y);
        p->base.flags |= 0x1000;
        return FALSE;
    }
    a1 = (s32)((p->clock + 1) * (u32)UFO_SPIN_1 % UFO_TURN);
    a2 = (s32)((p->clock + 1) * (u32)UFO_SPIN_2 % UFO_TURN);
    // (4.12: pixels times 4096; Y up);
    x = UFO_RADIUS * Cosine(a1);
    y = UFO_RADIUS * Sine(a1);
    if (p->look[0].effect != SCROLL_NO_LOOK) p->look[0].e.sprite.unk14 = (y <= 0 ? 0xA : SCROLL_BEHIND_KIRBYS) << 6;
    sn = Sine(a2);
    cs = Cosine(a2);
    p->base.x = kirby->base.x + ((x * cs - y * sn) >> 16);
    p->base.y = kirby->base.y + SCROLL_FEET - ((x * sn + y * cs) >> 16);
    p->base.flags &= ~1;
    if (p->clock & 1) Box(p, 0, 0, 0, 0, 0);
    else Box(p, -6, 5, 5, -5, UFO_STAR_STRENGTH);
    return TRUE;
}

// UfoStar(); The DS's 0x020BD560 (see above), each frame, after his input's read: his star made while he has none;
static void UfoStar(struct Kirby *kirby) {
    struct ScrollPiece *p;

    if (StarOf(kirby) != NULL || (p = ScrollPieceCreate(kirby, UFO_STAR, 0, UFO_STAR_KNOCK, UFO_STAR_HIT)) == NULL) return;
    UFO_SLOT(kirby) = TaskNumberOf(p);
    LookStart(p, 0, SQUEAK_FX_UFO_STAR, 0, 0);
    StarStep(p);
}

/* Hi-Jump. The DS's Hi-Jump Kirby's rise (0x020BF8E8, each frame: highjump_ext.mca's anims 12-16, composites 9-13, by how fast he
 * goes sideways; their boxes, .rca 26-30, his body's, of 20, stopping at what he hits (the body record's flags 0x10); with the scroll
 * (HasScroll 12) anims 17-21, composites 16-20: the same, and part 37 under him (its last part) at (0, -9), an orange glow 48 pixels
 * across about him, flickering every other frame (anim 37); their boxes, .rca 31-35, the same of 20, but going on through what he hits
 * (flags 0x30: the 0x20 the Laser scroll's charged shot has too, whose hit, 0x02037280, ends only the plain shot).
 * In KatAM, with the scroll: KatAM's Hi-Jump (NiDL's, abilities/hi_jump.c) already goes on through what it hits the whole rise; the
 * scroll's glow is drawn about him behind the Kirbys while he rises (HiJumpRiseNow), a piece (HI_JUMP_ORB) made from his input; */
#define HI_JUMP_ORB_Y (-9)

// OrbOf(); A Kirby's glow (the pieces looked through: Hi-Jump Kirby's ability byte is his fall's, HI_JUMP_LONG_FALL); NULL: none;
static struct ScrollPiece *OrbOf(struct Kirby *kirby) {
    u8 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct ScrollPiece *p;

        if (gTasks[i].main != ScrollPieceMain) continue;
        p = TaskGetStructPtr(&gTasks[i]);
        if (p->kind == HI_JUMP_ORB && p->kirby == kirby && !(p->base.flags & 0x1000)) return p;
    }
    return NULL;
}

// OrbStep(); The glow about him while he rises, gone after;
static bool32 OrbStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;

    if (kirby->ability != NIDL_ABILITY_HI_JUMP || !HiJumpRiseNow(kirby) || kirby->base.roomId != p->base.roomId) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    Follow(p);
    return TRUE;
}

// HiJumpOrb(); Each frame, after his input's read: the glow made as his rise starts;
static void HiJumpOrb(struct Kirby *kirby) {
    struct ScrollPiece *p;

    if (!HiJumpRiseNow(kirby) || OrbOf(kirby) != NULL || (p = ScrollPieceCreate(kirby, HI_JUMP_ORB, 0, 0, 0)) == NULL) return;
    LookStartAt(p, 0, SQUEAK_FX_HI_JUMP_ORB, 0, HI_JUMP_ORB_Y, SCROLL_BEHIND_KIRBYS);
}

/* Wheel. The DS's Wheel Kirby rolling (0x02092B24, 0x0209326C, each frame; wheel_ext.mca's composite 1, its .rca's box of 20, 8
 * behind to 10 in front and 21 up): with the scroll (HasScroll 11), while the wheel has no element yet, it takes the one of what meets
 * him (0x02012E84, as the Tornado scroll's: fire, water, electricity) and rolls on as that element's wheel (0x02093028: composites 5, 9
 * and 13; fire: the wheel, 31, in its colors, its glow, 37, flickering, and its flames, 38, behind; electric: 33, 41, 42) to its end;
 * its box the same, of the same 20, with the element (the attack record's element 1, 2, 4).
 * In KatAM, with the scroll: KatAM's wheel (its attack, sub_0807A3E4, its task sub_0807A4F4, while he rolls: anims 0x34, 0x68) takes
 * the element of a fire or electric attack or enemy that meets him (ElementTouch, as the Tornado scroll's, his wheel's box), its hit
 * that element's (KatAM's kind bit, strength as it was: the DS's damage the plain wheel's), the DS's element wheel drawn in place of
 * KatAM's (his sprite hidden, TORNADO_HIDE, and his wheel's disc, WheelDiscHide) to the end of the roll; a piece (WHEEL_ELEMENT)
 * found or made from his input each frame.
 * Not water: KatAM's wheel stops rolling in it (sub_080630B0: he swims); */
extern void sub_0807A4F4(void);
extern void sub_0809104C(void);
#define WHEEL_ROLL 0x34                     // KatAM's wheel's anims while its attack is on (sub_0807A4F4);
#define WHEEL_ROLL_2 0x68

// WheelDiscHide(); KatAM's wheel's disc about Kirby as he rolls (sub_08090F68's effect, its task sub_0809104C: anim 0x129 in his
// colors) not drawn (0x400) under the DS's element wheel;
static void WheelDiscHide(struct Kirby *kirby) {
    u8 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct EffectObject *e;

        if (gTasks[i].main != sub_0809104C) continue;
        e = TaskGetStructPtr(&gTasks[i]);
        if (e->parent == kirby) e->flags |= 0x400;
    }
}

// WheelOf(); The element piece of a Kirby's wheel, NULL: none;
static struct ScrollPiece *WheelOf(struct Kirby *kirby) {
    u8 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct ScrollPiece *p;

        if (gTasks[i].main != ScrollPieceMain) continue;
        p = TaskGetStructPtr(&gTasks[i]);
        if (p->kind == WHEEL_ELEMENT && p->kirby == kirby && !(p->base.flags & 0x1000)) return p;
    }
    return NULL;
}

/* WheelStep(); With Kirby's rolling wheel: its element taken once, KatAM's attack made that element's, the DS's element wheel drawn
 * in place of KatAM's (the wheel, its glow and its trail: his own hidden); gone when the roll ends; */
static bool32 WheelStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;
    struct ObjectBase *attack = p->other && p->other->main == sub_0807A4F4 ? TaskGetStructPtr(p->other) : NULL;
    u16 element;

    if (attack == NULL || attack->parent != kirby || attack->flags & 0x1000 || kirby->ability != KIRBY_ABILITY_WHEEL
        || (kirby->animationIndex != WHEEL_ROLL && kirby->animationIndex != WHEEL_ROLL_2)) {
        p->base.flags |= 0x1000;
        if (p->level == 0) return FALSE;
        kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
        return TRUE;
    }
    Follow(p);
    p->base.roomId = kirby->base.roomId;
    // (KatAM's wheel's box, sub_0807A3E4's);
    if (p->level == 0 && (element = ElementTouch(kirby, -9, -9, 12, 12)) != 0) {
        bool32 fire = element == TORNADO_FIRE;

        p->level = fire ? 1 : 3;
        attack->unk68 |= element;
        // (the composite's parts: the wheel first, on top, its glow, its trail);
        LookStart(p, 0, fire ? SQUEAK_FX_WHEEL_FIRE : SQUEAK_FX_WHEEL_SPARK, 0, 0);
        LookStart(p, 1, fire ? SQUEAK_FX_WHEEL_FIRE_GLOW : SQUEAK_FX_WHEEL_SPARK_GLOW, 0, 0);
        LookStart(p, 2, fire ? SQUEAK_FX_WHEEL_FIRE_TRAIL : SQUEAK_FX_WHEEL_SPARK_TRAIL, 0, 0);
    }
    if (p->look[0].effect != SCROLL_NO_LOOK) {
        kirby->base.sprite.unk8 |= TORNADO_HIDE;
        WheelDiscHide(kirby);
    }
    Box(p, 0, 0, 0, 0, 0);
    return TRUE;
}

// WheelElement(); Each frame, after his input's read: the element piece found or made for his rolling wheel's attack;
static void WheelElement(struct Kirby *kirby) {
    struct ScrollPiece *p;
    u8 i;

    if ((kirby->animationIndex != WHEEL_ROLL && kirby->animationIndex != WHEEL_ROLL_2) || WheelOf(kirby) != NULL) return;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];
        struct ObjectBase *o;

        if (t->main != sub_0807A4F4) continue;
        o = TaskGetStructPtr(t);
        if (o->parent != kirby || o->flags & 0x1000) continue;
        if ((p = ScrollPieceCreate(kirby, WHEEL_ELEMENT, 0, 0, 0)) != NULL) p->other = t;
        return;
    }
}

/* Sleep. The DS's (0x020BF294, each frame of his sleep): with the scroll (HasScroll 14) he gets better while he sleeps,
 * prmKirbySleep.kgb's word 1 (10) added a frame and a hit point each time it makes word 0 (180): one each 18 frames; A or B pressed
 * cuts his sleep by 2 frames, sound 0xA9 (the DS's own, scroll or not: KatAM's sleep has no such thing, so it comes with the scroll).
 * In KatAM, with the scroll: while he sleeps (sub_08064370's anim 52: his sleep lasts that animation), a hit point each 18 frames up to
 * his most; A or B pressed plays his sleep 2 frames further (its speed 3 frames that frame), sound 0xA9; */
extern void sub_080642B4(struct Kirby *);
extern void sub_08064370(struct Kirby *);
#define SLEEP_HEAL(kirby) NIDL_MIKE_USES(kirby)  // (his ability byte for Mike's uses and Spark's charge: neither is his while he sleeps);
#define SLEEP_HEAL_STEP 10
#define SLEEP_HEAL_AT 180
#define SLEEP_ASLEEP 52
#define SLEEP_SPEED 0x10                    // A sprite's animation speed (unk1C): a frame a frame;
#define SLEEP_CUT 2

// SleepHealing(); The upgraded sleep, after his input's read: his healing and his presses' cutting it short;
static void SleepHealing(struct Kirby *kirby) {
    if (kirby->stateFn == sub_080642B4) {
        SLEEP_HEAL(kirby) = 0;
        return;
    }
    if (kirby->stateFn != sub_08064370 || kirby->animationIndex != SLEEP_ASLEEP) return;
    SLEEP_HEAL(kirby) += SLEEP_HEAL_STEP;
    if (SLEEP_HEAL(kirby) >= SLEEP_HEAL_AT) {
        SLEEP_HEAL(kirby) -= SLEEP_HEAL_AT;
        if (kirby->hp < kirby->maxHp) kirby->hp++;
    }
    kirby->base.sprite.unk1C = kirby->sprites[1].unk1C = SLEEP_SPEED;
    if (kirby->unk11A & 3) {
        kirby->base.sprite.unk1C = kirby->sprites[1].unk1C = SLEEP_SPEED * (1 + SLEEP_CUT);
        PlaySfx(&kirby->base, SQUEAK_SE(0xA9));
    }
}

// ScrollKirbyInput(); sub_0803F324 (detoured): a Kirby's input read each frame, then the scrolls' that work from it;
void ScrollKirbyInput(struct Kirby *kirby) {
    gScrollInputOriginal(kirby);
    if (kirby->ability == KIRBY_ABILITY_SPARK && SqueakScrollOwned(SCROLL_SPARK)) SparkCharging(kirby);
    if (kirby->ability == KIRBY_ABILITY_SLEEP && SqueakScrollOwned(SCROLL_SLEEP)) SleepHealing(kirby);
    if (kirby->ability == KIRBY_ABILITY_LASER && SqueakScrollOwned(SCROLL_LASER)) LaserCharging(kirby);
    if (kirby->ability == KIRBY_ABILITY_UFO && SqueakScrollOwned(SCROLL_UFO)) UfoStar(kirby);
    if (kirby->ability == NIDL_ABILITY_HI_JUMP && SqueakScrollOwned(SCROLL_HI_JUMP)) HiJumpOrb(kirby);
    if (kirby->ability == KIRBY_ABILITY_WHEEL && SqueakScrollOwned(SCROLL_WHEEL)) WheelElement(kirby);
}

/* ScrollSparkAttack(); sub_08074AB0 (detoured): KatAM's spark attack starts; with the scroll, by the level: 0 KatAM's; 1 and 2 the
 * burst (KatAM's after it, BurstStep); 2 with forward held, KatAM's and the aura flies off (AuraStep). The level and charge end; */
void ScrollSparkAttack(struct Kirby *kirby) {
    struct SparkCharge *c = kirby->ability == KIRBY_ABILITY_SPARK ? ChargeOf(kirby, FALSE) : NULL;
    struct ScrollPiece *p;
    u8 level;

    if (c == NULL) {
        gScrollSparkOriginal(kirby);
        return;
    }
    level = c->level;
    c->attack = TRUE;
    c->ball = level == 2 && kirby->movementState & (kirby->base.flags & 1 ? 0x20 : 0x10);
    c->level = 0;
    c->charge = 0;
    if (level == 0 || c->ball || (p = ScrollPieceCreate(kirby, SPARK_BURST, level, SPARK_KNOCK, SPARK_HIT)) == NULL) {
        gScrollSparkOriginal(kirby);
        return;
    }
    if (level == 1) {
        LookStart(p, 0, SQUEAK_FX_SPARK_RING_1, 0, -8);
        LookStart(p, 1, SQUEAK_FX_SPARK_BURST_1, 0, 0);
    } else {
        LookStart(p, 0, SQUEAK_FX_SPARK_RING_2B, 0, -8);
        LookStart(p, 1, SQUEAK_FX_SPARK_RING_2A, 0, -8);
        LookStart(p, 2, SQUEAK_FX_SPARK_BURST_2, 0, 0);
    }
    PlaySfx(&kirby->base, SQUEAK_SE(level == 1 ? 0x51 : 0xB2));
}

// ScrollSparkHit(); Kirby hit (abilities.c's NidlKirbyHit): his level ends (the DS's hurt);
void ScrollSparkHit(struct Kirby *kirby) {
    struct SparkCharge *c = kirby->ability == KIRBY_ABILITY_SPARK ? ChargeOf(kirby, FALSE) : NULL;

    if (c != NULL) c->level = 0;
}
