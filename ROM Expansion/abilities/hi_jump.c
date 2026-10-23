/* Hi-Jump; NiDL's (from Starman), from its code (0x08049F98, its script; 0x0804A258, every frame after it; 0x08059570, its effects,
 * Kirby's effect 0x2A) and checked against it frame by frame (positions, speeds, poses and effects; KatAM takes B a frame sooner than
 * NiDL, the same from there):
 * • B: Kirby crouches (frame 0xB2D, 4 frames, sinking 1, 2, 2, 3 pixels as NiDL's does), held where he is up and down, on land or in
 *   the air, going on sideways as he was; dust puffs out from under him both ways, in front of him (variant 5);
 * • the rise: up at 0x428 a frame (4.16 pixels) for 23 frames, then from 0x400, slowing by NiDL's gravity (0x26), for 23 more, then 5
 *   from 0xB0 (148 pixels in all); for its first 33 frames he glows (NiDL's bright yellow, 4 frames in 8 by the game's clock: not with
 *   "Reduce Flashing"), a star and sparkles fall under him (variants 0, 1, 2: each round from where he is, his speed sideways
 *   backwards and 2 pixels a frame down; no new round once he's done glowing), and a burst goes off where he took off (variant 4: its
 *   box at once, 4 frames, then its frames). He hurts what he touches the whole rise (NiDL's box, 12 left and right, 12 up and 14
 *   down from its spot, 3 above KatAM's);
 * • then the fall: KatAM's, its somersault first, where B does nothing till he lands (NiDL's states 5 and 8: he can still float);
 * • his speed sideways (from his 2nd rise frame, as NiDL's input takes a frame): 0.25 a frame up to 2.0 the way he's pressed (and
 *   facing it), else slowing by 0.055; his pose by it (NiDL's five sets, leaning more from 0.25, 1.0, 1.25 and 1.75), on the
 *   flapping cape's beat (a frame every 3, the first 2), whichever set;
 * • B again (from the rise's 5th frame, NiDL's countdown of 4, to the end of the rise) stops him: that frame's move as any, a puff
 *   where it ends (variant 3: its box 3 frames), then the fall from still, somersault first; a ceiling or the room's top stops him too,
 *   falling at once.
 * NiDL's guard (untouchable while he glows) isn't ported: no ported ability is a shield (README). NiDL's take-off sound is its own:
 * KatAM's closest plays. Strengths are KatAM's (its attacks hit again every few frames, NiDL's once): the rise 2 (NiDL's 10), the
 * burst and the puff 1 (its 6 and 2); */
#include "../abilities.h"
#include "code_0806F780.h"
#include "palette.h"
#include "random.h"
#include "task.h"
#include "constants/songs.h"

#define CROUCH_FRAMES 4
#define RISE_SPEED 0x428
#define RISE_FRAMES 23          // At RISE_SPEED; then from SLOW_SPEED, slowing, till END_FRAMES;
#define SLOW_SPEED 0x400
#define GLOW_FRAMES 33          // (NiDL's state 1: his glow, the star and sparkles' rounds);
#define TOP_FRAMES 46           // From TOP_SPEED;
#define TOP_SPEED 0xB0
#define END_FRAMES 51
#define CANCELLED (END_FRAMES + 1) // (B again: the fall next frame, from still);
#define CANCEL_FROM 4
#define GRAVITY 0x26
#define STEER_ACCELERATION 0x40
#define STEER_SPEED 0x200
#define STEER_SLOW 0xE
#define NIDL_Y 3                // NiDL's Kirby's spot is this far above KatAM's (KatAMExpansionMotions.cs, MotionDy);
#define BEATS 12                // The rise's poses' loop (4 frames, 3 each);

// His speed sideways (8.8) up to which each of his poses' sets shows (NiDL's 0x0804A3FA; the last above);
static const u16 sPoseSpeeds[] = { 0x40, 0x100, 0x140, 0x1C0 };
static const s8 sSink[CROUCH_FRAMES] = { 1, 2, 2, 3 };

/* The effects: their anim's frames (gNidlAbilityData.hiJumpFx, KatAMExpansion.cs's HiJumpEffects: NiDL's) a frame each, as NiDL plays
 * them (the burst's first frames, -1, hidden); */
enum { FX_STAR, FX_SPARKLE, FX_SPARKLE_2, FX_PUFF, FX_BURST, FX_DUST_LEFT, FX_DUST_RIGHT };
#define FX_FRAME 0              // NiDL's effect frame 16 (0x08751F0C's), its star;
#define FX_PUFF_FRAME 14        // 0x0874C804's 0, the puff;
#define FX_DUST_FRAME 23        // Its 9 (0x0874C828's 0), the dust;
#define FX_BURST_FRAME 29       // Its 25 (0x0874C828's 16), the burst;
#define FX_HIDDEN 0xFF
#define FX_TILES 16             // (VRAM per effect: the burst's widest frame);
#define FX_FRONT -1             // In front of Kirby (NiDL's depth 5, his 7) or behind him (8): next to his sprites' order (KatAM's
#define FX_BACK 1               // Macro_0803E920: 15 for the player's, 16 + his number for the others);
#define FX_FALL 0x200           // The star and sparkles: down 2 pixels a frame;
#define DUST_FAST 0x400         // The dust: out 4 pixels a frame for 4 frames, then 1, slowing by 1/32 a frame (NiDL's 0x0805999C);
#define DUST_SLOW 0x100
#define DUST_EASE 8
#define DUST_Y 3                // (below NiDL's Kirby's spot, where he crouched);

static const u8 sStar[] = { FX_FRAME, FX_FRAME + 1, FX_FRAME + 2, FX_FRAME + 3, FX_FRAME + 4, FX_FRAME + 5 };
static const u8 sSparkle[] = { FX_FRAME + 10, FX_FRAME + 11, FX_FRAME + 11, FX_FRAME + 12, FX_FRAME + 12, FX_FRAME + 13, FX_FRAME + 13,
                               FX_FRAME + 12, FX_FRAME + 12, FX_FRAME + 11, FX_FRAME + 11 };
static const u8 sSparkle2[] = { FX_FRAME + 6, FX_FRAME + 7, FX_FRAME + 7, FX_FRAME + 8, FX_FRAME + 8, FX_FRAME + 9, FX_FRAME + 9,
                                FX_FRAME + 10, FX_FRAME + 10, FX_FRAME + 11, FX_FRAME + 11, FX_FRAME + 12, FX_FRAME + 12, FX_FRAME + 13 };
static const u8 sPuff[] = { FX_PUFF_FRAME, FX_PUFF_FRAME + 1, FX_PUFF_FRAME + 2, FX_PUFF_FRAME + 3, FX_PUFF_FRAME + 4, FX_PUFF_FRAME + 5,
                            FX_PUFF_FRAME + 6, FX_PUFF_FRAME + 6, FX_PUFF_FRAME + 7, FX_PUFF_FRAME + 7, FX_PUFF_FRAME + 8 };
static const u8 sBurst[] = { FX_HIDDEN, FX_HIDDEN, FX_HIDDEN, FX_HIDDEN, FX_BURST_FRAME, FX_BURST_FRAME + 8, FX_BURST_FRAME + 1,
                             FX_BURST_FRAME + 2, FX_BURST_FRAME + 3, FX_BURST_FRAME + 4, FX_BURST_FRAME + 5, FX_BURST_FRAME + 6,
                             FX_BURST_FRAME + 7 };
static const u8 sDust[] = { FX_DUST_FRAME, FX_DUST_FRAME, FX_DUST_FRAME + 2, FX_DUST_FRAME + 2, FX_DUST_FRAME + 4, FX_DUST_FRAME + 4,
                            FX_DUST_FRAME + 4, FX_DUST_FRAME + 4, FX_DUST_FRAME + 4, FX_DUST_FRAME + 2, FX_DUST_FRAME + 2, FX_DUST_FRAME + 2,
                            FX_HIDDEN, FX_DUST_FRAME + 2, FX_HIDDEN, FX_DUST_FRAME + 2, FX_HIDDEN, FX_DUST_FRAME, FX_HIDDEN, FX_DUST_FRAME };
static const struct { const u8 *frames; u8 count; s8 order; } sFx[] = {
    [FX_STAR] = { sStar, ARRAY_COUNT(sStar), FX_BACK },
    [FX_SPARKLE] = { sSparkle, ARRAY_COUNT(sSparkle), FX_FRONT },
    [FX_SPARKLE_2] = { sSparkle2, ARRAY_COUNT(sSparkle2), FX_FRONT },
    [FX_PUFF] = { sPuff, ARRAY_COUNT(sPuff), FX_BACK },
    [FX_BURST] = { sBurst, ARRAY_COUNT(sBurst), FX_FRONT },
    [FX_DUST_LEFT] = { sDust, ARRAY_COUNT(sDust), FX_FRONT },
    [FX_DUST_RIGHT] = { sDust, ARRAY_COUNT(sDust), FX_FRONT },
};

struct HiJumpFx {
    struct EffectObject e;
    struct Kirby *kirby;
    u8 kind, t;
    s32 anchor, x, vx;          // The dust: where he crouched, how far out (8.8), how fast;
};

// The fall's state (KatAM's sub_08044FD4, else its long fall's, sub_08045A34) in Mike's byte (Kirby has Hi-Jump, not Mike);
#define HI_JUMP_LONG_FALL(kirby) NIDL_MIKE_USES(kirby)

s32 sub_08153E6C(struct Sprite *, s32);
void sub_08044FD4(struct Kirby *);
void sub_08045A34(struct Kirby *);
extern const struct NidlCalm { void (*updateScreenDma)(void); void (*updateScreenCpuSet)(void); } gNidlCalm;

static void HiJumpCrouch(struct Kirby *);
static void HiJumpRise(struct Kirby *);
static void HiJumpFallen(struct Kirby *);

static bool32 HiJumpRising(struct Kirby *kirby) {
    return kirby->stateFn == HiJumpRise;
}

static bool32 HiJumpGlowing(struct Kirby *kirby) {
    return kirby->stateFn == HiJumpRise && kirby->base.counter < GLOW_FRAMES;
}

// HiJumpFxMain(); An effect: its frames, the star's and sparkles' rounds again under Kirby while he glows (NiDL's 0x080595C4-0x08059750);
static void HiJumpFxMain(void) {
    struct HiJumpFx *fx = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = fx->kirby;
    u8 frame;

    if (fx->e.flags & 0x1000 || kirby->base.roomId != fx->e.roomId) {
        TaskDestroy(gCurTask);
        return;
    }
    if (fx->t >= sFx[fx->kind].count) {
        if (fx->kind > FX_SPARKLE_2 || !HiJumpGlowing(kirby)) {
            fx->e.flags |= 0x1000;
            return;
        }
        // A new round, from where he is (NiDL's star right under him, its sparkles 8 either way at random);
        fx->t = 0;
        fx->e.x = kirby->base.x;
        fx->e.y = kirby->base.y + ((16 - NIDL_Y) << 8);
        if (fx->kind != FX_STAR) {
            fx->e.x += ((Rand16() & 15) - 8) << 8;
            fx->e.y += ((Rand16() & 15) - 8) << 8;
        }
        fx->e.unk3C = -kirby->base.xspeed;
    }
    if (fx->kind <= FX_SPARKLE_2) {
        fx->e.x += fx->e.unk3C;
        fx->e.y += FX_FALL;
    } else if (fx->kind >= FX_DUST_LEFT) {
        if (fx->t == CROUCH_FRAMES) fx->vx = -DUST_SLOW;
        if (fx->t >= CROUCH_FRAMES) fx->vx += DUST_EASE;
        fx->x += fx->vx;
        fx->e.x = fx->anchor + (fx->kind == FX_DUST_LEFT ? fx->x : -fx->x);
    }
    frame = sFx[fx->kind].frames[fx->t++];
    if (frame == FX_HIDDEN) fx->e.flags |= 0x400;
    else {
        fx->e.flags &= ~0x400;
        fx->e.sprite.variant = frame + (fx->kind == FX_DUST_RIGHT);
    }
    fx->e.flags |= 4;
    EffectObjectPostUpdate(&fx->e);
}

static void HiJumpFx(struct Kirby *kirby, u8 kind, s32 x, s32 y) {
    struct Task *task = TaskCreate(HiJumpFxMain, sizeof(struct HiJumpFx), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct HiJumpFx *fx = TaskGetStructPtr(task);

    ClearEffectObject(&fx->e);
    fx->e.header.kind = 3;
    fx->e.x = x;
    fx->e.y = y;
    fx->e.roomId = kirby->base.roomId;
    fx->e.unk3C = -kirby->base.xspeed;
    fx->kirby = kirby;
    fx->anchor = x;
    fx->kind = kind;
    fx->t = 0;
    fx->x = 0;
    fx->vx = -DUST_FAST;
    if (kind == FX_STAR || kind == FX_SPARKLE || kind == FX_SPARKLE_2) fx->t = sFx[kind].count;   // (its round placed on its first frame);
    EffectObjectInitSprite(&fx->e, &fx->e.sprite, FX_TILES, gNidlAbilityData.hiJumpFx, sFx[kind].frames[0] == FX_HIDDEN
                           ? FX_BURST_FRAME : sFx[kind].frames[0], (kirby->base.sprite.unk14 >> 6) + sFx[kind].order);
    fx->e.flags |= 0x400;
}

/* HiJumpGlow; Kirby's glow as he rises (NiDL's 0x0804A29A: its colors 4 frames in 8 by its clock, his own between), the body's and the
 * hat's palettes; his own put back as he stops glowing, whatever stops him; */
struct HiJumpGlow {
    struct Kirby *kirby;
    u16 own[2][16];
};

static void HiJumpGlowColors(u8 palId, const u16 *colors) {
    if (gMainFlags & MAIN_FLAG_OBJ_PALETTE_TRANSFORMATION_ENABLE) LoadObjPaletteWithTransformation(colors, palId * 16, 16);
    else {
        CpuCopy16(colors, gObjPalette + palId * 16, 16 * sizeof(u16));
        gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
    }
}

static void HiJumpGlowMain(void) {
    struct HiJumpGlow *glow = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = glow->kirby;
    u8 body = kirby->base.sprite.palId, hat = kirby->sprites[1].palId;

    if (!HiJumpGlowing(kirby) || kirby->ability != NIDL_ABILITY_HI_JUMP) {
        HiJumpGlowColors(body, glow->own[0]);
        HiJumpGlowColors(hat, glow->own[1]);
        TaskDestroy(gCurTask);
        return;
    }
    if (!(gUnk_0203AD40 & 4)) {
        u16 colors[16];
        const u16 *flash = gNidlAbilityData.hiJumpFlash;

        CpuCopy16(glow->own[0], colors, sizeof(colors));
        CpuCopy16(flash + 1, colors + 1, 11 * sizeof(u16));
        HiJumpGlowColors(body, colors);
        colors[0] = glow->own[1][0];
        CpuCopy16(flash + 17, colors + 1, 15 * sizeof(u16));
        HiJumpGlowColors(hat, colors);
    } else {
        HiJumpGlowColors(body, glow->own[0]);
        HiJumpGlowColors(hat, glow->own[1]);
    }
}

static void HiJumpGlow(struct Kirby *kirby) {
    struct Task *task;
    struct HiJumpGlow *glow;

    if (gNidlCalm.updateScreenDma) return;
    task = TaskCreate(HiJumpGlowMain, sizeof(struct HiJumpGlow), 0x3500, TASK_USE_EWRAM, NULL);
    glow = TaskGetStructPtr(task);
    glow->kirby = kirby;
    CpuCopy16(gObjPalette + kirby->base.sprite.palId * 16, glow->own[0], sizeof(glow->own[0]));
    CpuCopy16(gObjPalette + kirby->sprites[1].palId * 16, glow->own[1], sizeof(glow->own[1]));
}

/* HiJumpPose(); His rise's pose by his speed sideways (NiDL's sets), `beat` frames into its loop: the body's and hat's anims set and
 * started there (KatAM's sub_08153E6C) before the game plays his animation index, so a change of set keeps the cape's beat; */
static void HiJumpPose(struct Kirby *kirby, u32 beat) {
    u32 speed = kirby->base.xspeed < 0 ? -kirby->base.xspeed : kirby->base.xspeed;
    const struct AnimInfo *body;
    u8 set;

    for (set = 0; set < ARRAY_COUNT(sPoseSpeeds) && speed > sPoseSpeeds[set]; set++);
    if (kirby->animationIndex == NIDL_MOTION_SLOT + HI_JUMP_MOTION_RISE + set) return;
    NidlMotion(kirby, HI_JUMP_MOTION_RISE + set);
    body = &gNidlAbilityData.motions[NIDL_ABILITY_HI_JUMP - NIDL_FIRST_ABILITY][HI_JUMP_MOTION_RISE + set];
    kirby->base.sprite.animId = body->animId;
    kirby->base.sprite.variant = body->variant;
    kirby->sprites[1].animId = gNidlAbilityData.glows[1].anim;  // (the motions' hat anim, its variant the motion's, as the body's);
    kirby->sprites[1].variant = body->variant;
    sub_08153E6C(&kirby->base.sprite, beat % BEATS);
    sub_08153E6C(&kirby->sprites[1], beat % BEATS);
}

// HiJumpStart(); The B button;
void HiJumpStart(struct Kirby *kirby) {
    NidlMotion(kirby, HI_JUMP_MOTION_CROUCH);
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags |= 0x40;     // No gravity;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    kirby->stateFn = HiJumpCrouch;     // (the game runs a new state the same frame);
}

static void HiJumpCrouch(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.yspeed = 0;
    if (kirby->base.counter < CROUCH_FRAMES) {
        if (!kirby->base.counter) {
            HiJumpFx(kirby, FX_DUST_LEFT, kirby->base.x, kirby->base.y + ((DUST_Y - NIDL_Y) << 8));
            HiJumpFx(kirby, FX_DUST_RIGHT, kirby->base.x, kirby->base.y + ((DUST_Y - NIDL_Y) << 8));
        }
        kirby->base.objBase55 = sSink[kirby->base.counter++];
        return;
    }

    // The take-off;
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.flags |= 0x20;
    kirby->base.unk62 &= ~(8 | 4);
    PlaySfx(&kirby->base, SE_ABILITY_SWORD_UPSLASH);
    NidlAttackCreate(kirby, HiJumpRising, 0x30000043, 2, FALSE, -12, -12 - NIDL_Y, 12, 14 - NIDL_Y);
    NidlAttackCreate(kirby, NULL, 0x30000043, 1, FALSE, -24, -6 + 4 - NIDL_Y, 24, 9 + 4 - NIDL_Y)->frames = 4;
    HiJumpFx(kirby, FX_BURST, kirby->base.x, kirby->base.y + ((4 - NIDL_Y) << 8));
    HiJumpFx(kirby, FX_STAR, kirby->base.x, kirby->base.y);
    HiJumpFx(kirby, FX_SPARKLE, kirby->base.x, kirby->base.y);
    HiJumpFx(kirby, FX_SPARKLE_2, kirby->base.x, kirby->base.y);
    HiJumpGlow(kirby);
    kirby->stateFn = HiJumpRise;
}

// HiJumpFall(); The fall (KatAM's, its somersault first, `somersault`), where B does nothing till he lands;
static void HiJumpFall(struct Kirby *kirby, bool8 somersault) {
    kirby->base.flags &= ~0x40;
    kirby->idleTimer = 0;
    kirby->animationIndex = somersault ? 15 : 17;
    sub_08044EA8(kirby);
    HI_JUMP_LONG_FALL(kirby) = FALSE;
    kirby->stateFn = HiJumpFallen;
}

static void HiJumpFallen(struct Kirby *kirby) {
    kirby->unk11A &= ~2;
    (HI_JUMP_LONG_FALL(kirby) ? sub_08045A34 : sub_08044FD4)(kirby);
    if (kirby->stateFn == sub_08044FD4 || kirby->stateFn == sub_08045A34) {
        HI_JUMP_LONG_FALL(kirby) = kirby->stateFn == sub_08045A34;
        kirby->stateFn = HiJumpFallen;
    }
}

static void HiJumpRise(struct Kirby *kirby) {
    u16 t = kirby->base.counter;
    bool8 cancel;

    if (NidlAbilityInWater(kirby)) return;
    // The rise's end, or B the frame before (NiDL's state 3: falling from still);
    if (t >= END_FRAMES) {
        if (t == CANCELLED) kirby->base.yspeed = 0;
        HiJumpFall(kirby, TRUE);
        return;
    }
    if (kirby->base.unk62 & 4 && kirby->base.yspeed <= 0) {
        kirby->idleTimer = 0;
        NidlAbilityEnd(kirby);
        return;
    }
    // A ceiling or the room's top (he rose less than his speed, moved last frame; NiDL's clamp there stops him too), rising: the fall
    // at once;
    if (kirby->base.yspeed > 0 && (kirby->base.unk62 & 8
                                   || (!(kirby->base.flags & 0x800) && kirby->base.unk4C - kirby->base.y < kirby->base.yspeed))) {
        kirby->base.unk62 &= ~8;
        kirby->base.yspeed = 0;
        HiJumpFall(kirby, FALSE);
        return;
    }
    // B again: this frame as any other (NiDL moves Kirby before it reads B), then the fall;
    cancel = t >= CANCEL_FROM && kirby->unk11A & 2;

    // Sideways: last frame's input (NiDL's takes a frame; none on his first), then this frame's facing and input kept for the next;
    if (t) {
        if (kirby->idleTimer & 0x30) {
            kirby->base.xspeed += kirby->idleTimer & 0x20 ? -STEER_ACCELERATION : STEER_ACCELERATION;
            if (kirby->base.xspeed < -STEER_SPEED) kirby->base.xspeed = -STEER_SPEED;
            if (kirby->base.xspeed > STEER_SPEED) kirby->base.xspeed = STEER_SPEED;
        } else if (kirby->base.xspeed > 0) {
            kirby->base.xspeed -= STEER_SLOW;
            if (kirby->base.xspeed < 0) kirby->base.xspeed = 0;
        } else if (kirby->base.xspeed < 0) {
            kirby->base.xspeed += STEER_SLOW;
            if (kirby->base.xspeed > 0) kirby->base.xspeed = 0;
        }
    }
    if (kirby->movementState & 0x20) kirby->base.flags |= 1;
    else if (kirby->movementState & 0x10) kirby->base.flags &= ~1;
    if (kirby->base.xspeed < 0) kirby->base.flags |= 1;
    else if (kirby->base.xspeed > 0) kirby->base.flags &= ~1;
    kirby->idleTimer = kirby->movementState & 0x30;

    // Up: NiDL's speeds (set, then its gravity the same frame);
    if (t < RISE_FRAMES) kirby->base.yspeed = RISE_SPEED;
    else if (t == RISE_FRAMES) kirby->base.yspeed = SLOW_SPEED - GRAVITY;
    else if (t == TOP_FRAMES) kirby->base.yspeed = TOP_SPEED - GRAVITY;
    else kirby->base.yspeed -= GRAVITY;
    kirby->base.flags |= 0x40 | 0x20;
    HiJumpPose(kirby, t + 1);
    kirby->base.counter = cancel ? CANCELLED : t + 1;

    // The cancel's puff and its box, where he'll be after this frame's move (NiDL's: made after its move);
    if (cancel) {
        s32 x = kirby->base.x + kirby->base.xspeed, y = kirby->base.y - kirby->base.yspeed;
        struct NidlAttack *puff = NidlAttackCreate(kirby, NULL, 0x30000043, 1, FALSE, -18, -18 - NIDL_Y, 18, 18 - NIDL_Y);

        puff->frames = 3;
        puff->base.x = x;
        puff->base.y = y;
        HiJumpFx(kirby, FX_PUFF, x, y - (NIDL_Y << 8));
    }
}
