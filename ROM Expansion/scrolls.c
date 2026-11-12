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

// The scrolls' numbers (the DS's);
enum {
    SCROLL_FIRE,
    SCROLL_ICE,
    SCROLL_SPARK,
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
 * spark is KatAM's (sub_08074AB0, its hit box and look); the bursts, the aura and the ball are SparkPieces (their hit boxes the DS's,
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
#define SPARK_FEET 0x800                    // The DS's origin, Kirby's feet: 8 pixels under KatAM's;
#define SPARK_FX_ROOM 48                    // squeak.c's SQUEAK_FX_ROOM: tiles left for KatAM's own effects;
#define SPARK_LOOKS 3
#define SPARK_NO_LOOK 0xFF
#define SPARK_D_PAD 0xF0

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

enum { SPARK_AURA, SPARK_BURST, SPARK_BALL };

// SparkLook; One of a piece's looks: an effect's animation (squeak.h's SQUEAK_FX_*), `dy` pixels under the piece (the DS's
// composite's part: its offset);
struct SparkLook {
    struct EffectObject e;
    u8 effect;                  // SPARK_NO_LOOK: none;
    u8 step;
    u8 timer;
    u8 palId;
    s8 dy;
};

// SparkPiece; An aura, a burst or a ball: its hit box (the DS's, from its origin: the base at Kirby's feet), its looks;
struct SparkPiece {
    struct ObjectBase base;
    struct SparkLook look[SPARK_LOOKS];
    struct Kirby *kirby;
    u8 kind;
    u8 level;
    u16 clock;
};

static void SparkChargeMain(void);

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
static void LookEnd(struct SparkLook *l) {
    if (l->effect != SPARK_NO_LOOK && l->e.sprite.tilesVram >= 0x6014000 && SqueakVramOk(l->e.sprite.tilesVram))
        VramFree(l->e.sprite.tilesVram);
    l->effect = SPARK_NO_LOOK;
}

// LookStart(); A look playing an effect from its first step (none when KatAM's sprite heap can't hold it, as squeak.c's effects);
static void LookStart(struct SparkPiece *p, u8 i, u8 effect, s8 dy) {
    struct SparkLook *l = &p->look[i];
    const struct SqueakEffect *d = &gSqueakData.effects[effect];
    u16 cell = d->anim.step[0].cell;

    LookEnd(l);
    if (VramGetTotalAllocatedTiles() + d->tiles + SPARK_FX_ROOM > gVramHeapMaxTileSlots) return;
    ClearEffectObject(&l->e);
    l->e.header.kind = 3;
    l->e.roomId = p->base.roomId;
    l->e.parent = NULL;
    l->step = 0;
    l->timer = d->anim.step[0].frames;
    l->dy = dy;
    l->palId = SqueakLoadPalette(d->palette.key, d->palette.variant, p->base.roomId);
    EffectObjectInitSprite(&l->e, &l->e.sprite, d->tiles, d->animId, cell == SQUEAK_FX_HIDDEN ? 0 : cell, 0xA);
    if (SqueakVramOk(l->e.sprite.tilesVram)) l->effect = effect;
}

// LookDraw(); A look's step (its animations all loop) where its piece is, facing as it;
static void LookDraw(struct SparkPiece *p, struct SparkLook *l) {
    const struct SqueakAnim *a;
    const struct SqueakStep *step;
    bool32 left = p->base.flags & 1;

    if (l->effect == SPARK_NO_LOOK) return;
    a = &gSqueakData.effects[l->effect].anim;
    step = &a->step[l->step];
    l->e.x = p->base.x;
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
    if (--l->timer == 0) {
        l->step = l->step + 1 >= a->steps ? 0 : l->step + 1;
        l->timer = a->step[l->step].frames;
    }
}

// Box(); The piece's hit box this frame: the DS's (left, top, right, bottom from its origin, Y up), its strength; 0: none;
static void Box(struct SparkPiece *p, s8 left, s8 top, s8 right, s8 bottom, s8 strength) {
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
static void Follow(struct SparkPiece *p) {
    p->base.x = p->kirby->base.x;
    p->base.y = p->kirby->base.y + SPARK_FEET;
    p->base.flags = (p->base.flags & ~1) | (p->kirby->base.flags & 1);
}

// AuraLook(); The aura's look by level (the DS's aura, 0x02087AA4: spark_ext's 30; composite 3), with its sound;
static void AuraLook(struct SparkPiece *p, u8 level) {
    u8 i;

    for (i = 0; i < SPARK_LOOKS; i++) LookEnd(&p->look[i]);
    if (level == 1) {
        LookStart(p, 0, SQUEAK_FX_SPARK_BITS, 0);
    } else {
        LookStart(p, 0, SQUEAK_FX_SPARK_BITS_B, 0);
        LookStart(p, 1, SQUEAK_FX_SPARK_BITS_A, 0);
        LookStart(p, 2, SQUEAK_FX_SPARK_BALL, 0);
    }
    p->level = level;
    p->clock = 0;
    PlaySfx(&p->kirby->base, SQUEAK_SE(level == 1 ? 0xB0 : 0xB1));
}

// SparkBallStart(); The level-2 aura flies off (0x02087AA4's: composite 4, 4 pixels up, sound 0xB2);
static void SparkBallStart(struct SparkPiece *p) {
    u8 i;

    p->kind = SPARK_BALL;
    p->base.y -= SPARK_BALL_RISE;
    for (i = 0; i < SPARK_LOOKS; i++) {
        if (p->look[i].effect == SPARK_NO_LOOK) continue;
        p->look[i].step = 0;
        p->look[i].timer = gSqueakData.effects[p->look[i].effect].anim.step[0].frames;
    }
    p->clock = 0;
    PlaySfx(&p->base, SQUEAK_SE(0xB2));
}

// The level-2 aura's and the ball's hit boxes (their .rca's 43 and 44: 5 every 3 frames, the ball's first 20; 16 frames, looping);
static void AuraBox(struct SparkPiece *p) {
    u8 frame = p->clock % 16;

    if (p->kind == SPARK_BALL && frame == 0) Box(p, -24, 29, 24, -15, 3);
    else if (frame % 3 == 0 && (p->kind == SPARK_BALL || frame < 15)) Box(p, -24, 31, 23, -15, 1);
    else Box(p, 0, 0, 0, 0, 0);
}

// AuraStep(); The aura: its Kirby's level's look, gone at level 0 or his attack (flying off at level 2 with forward held);
static bool32 AuraStep(struct SparkPiece *p) {
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
static bool32 BallStep(struct SparkPiece *p) {
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
static bool32 BurstStep(struct SparkPiece *p) {
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

static void SparkPieceMain(void) {
    struct SparkPiece *p = TaskGetStructPtr(gCurTask);
    bool32 alive;
    u8 i;

    if (ObjectPreUpdate(&p->base)) return;
    alive = p->kind == SPARK_AURA ? AuraStep(p) : p->kind == SPARK_BALL ? BallStep(p) : BurstStep(p);
    if (!alive) return;
    for (i = 0; i < SPARK_LOOKS; i++) LookDraw(p, &p->look[i]);
    p->clock++;
}

static void SparkPieceDestroy(struct Task *t) {
    struct SparkPiece *p = TaskGetStructPtr(t);
    u8 i;

    for (i = 0; i < SPARK_LOOKS; i++) LookEnd(&p->look[i]);
}

// SparkPieceCreate(); A piece at its Kirby's feet, its hit box KatAM's spark's kind (sub_08074AB0's);
static struct SparkPiece *SparkPieceCreate(struct Kirby *kirby, u8 kind, u8 level) {
    struct Task *t = TaskCreate(SparkPieceMain, sizeof(struct SparkPiece), 0x3500, TASK_USE_EWRAM, SparkPieceDestroy);
    struct SparkPiece *p;
    u8 i;

    if (t == &gEmptyTask) return NULL;
    p = TaskGetStructPtr(t);
    ClearObjectBase(&p->base);
    for (i = 0; i < SPARK_LOOKS; i++) p->look[i].effect = SPARK_NO_LOOK;
    p->kirby = kirby;
    p->kind = kind;
    p->level = level;
    p->clock = 0;
    p->base.header.kind = 2;
    p->base.parent = kirby;
    p->base.counter = 0;
    p->base.roomId = kirby->base.roomId;
    p->base.unk56 = kirby->base.unk56;
    if (Macro_0810B1F4(&p->base)) p->base.flags |= 0x2000;
    p->base.unk64 = 0xA0;
    p->base.unk66 = 0;
    p->base.unk63 = 0;
    p->base.flags |= 0xA0000100 | 0x400 | 0x200;
    p->base.unk68 = 0xA0000403;
    Follow(p);
    return p;
}

// Glint(); spark_cmn's 58 at random in his body's box (spark_cmn's .rca: -6 to 6 across, 2 to 14 up from his feet);
static void Glint(struct Kirby *kirby) {
    s32 dx = (Rand16() % 12) - 6, dy = 2 + Rand16() % 12;

    if (kirby->base.flags & 1) dx = -dx;
    SqueakEffectInRoom(kirby->base.roomId, SQUEAK_FX_SPARK_GLINT, kirby->base.x + (dx << 8), kirby->base.y + SPARK_FEET - (dy << 8));
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
                    SparkPieceCreate(kirby, SPARK_AURA, 0);
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

// ScrollKirbyInput(); sub_0803F324 (detoured): a Kirby's input read each frame, then the scrolls' that work from it;
void ScrollKirbyInput(struct Kirby *kirby) {
    gScrollInputOriginal(kirby);
    if (kirby->ability == KIRBY_ABILITY_SPARK && SqueakScrollOwned(SCROLL_SPARK)) SparkCharging(kirby);
}

/* ScrollSparkAttack(); sub_08074AB0 (detoured): KatAM's spark attack starts; with the scroll, by the level: 0 KatAM's; 1 and 2 the
 * burst (KatAM's after it, BurstStep); 2 with forward held, KatAM's and the aura flies off (AuraStep). The level and charge end; */
void ScrollSparkAttack(struct Kirby *kirby) {
    struct SparkCharge *c = kirby->ability == KIRBY_ABILITY_SPARK ? ChargeOf(kirby, FALSE) : NULL;
    struct SparkPiece *p;
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
    if (level == 0 || c->ball || (p = SparkPieceCreate(kirby, SPARK_BURST, level)) == NULL) {
        gScrollSparkOriginal(kirby);
        return;
    }
    if (level == 1) {
        LookStart(p, 0, SQUEAK_FX_SPARK_RING_1, -8);
        LookStart(p, 1, SQUEAK_FX_SPARK_BURST_1, 0);
    } else {
        LookStart(p, 0, SQUEAK_FX_SPARK_RING_2B, -8);
        LookStart(p, 1, SQUEAK_FX_SPARK_RING_2A, -8);
        LookStart(p, 2, SQUEAK_FX_SPARK_BURST_2, 0);
    }
    PlaySfx(&kirby->base, SQUEAK_SE(level == 1 ? 0x51 : 0xB2));
}

// ScrollSparkHit(); Kirby hit (abilities.c's NidlKirbyHit): his level ends (the DS's hurt);
void ScrollSparkHit(struct Kirby *kirby) {
    struct SparkCharge *c = kirby->ability == KIRBY_ABILITY_SPARK ? ChargeOf(kirby, FALSE) : NULL;

    if (c != NULL) c->level = 0;
}
