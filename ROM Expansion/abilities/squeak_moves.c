/* Squeak Squad's abilities' moves (abilities 35-38, abilities.h): ported 1:1 from the DS's code (Kirby Squeak Squad (USA): each
 * ability's Kirby class, its vtable's B (+0x88) and hooks, its states; their words are its param/PrmKirby<Ability>.kgb's, at the
 * player's +0x394, after a 4-word header; its speeds 20.12 a frame, SQUEAK_SPEED converting them to KatAM's 24.8; Y up, as KatAM's
 * speeds; its damage in KatAM's strengths as the scrolls' (one for each 5)). Kirby is drawn as the DS draws him in them: his moves'
 * looks (gSqueakAbilityLooks: the DS's frames, his body in his own palette) drawn by a piece (scrolls.h's, SCROLL_MOVE) that follows
 * him while his state is one of the moves', his own sprite hidden (TORNADO_HIDE); without the Squeak Squad ROM, KatAM's Normal frames.
 *
 * Ninja (the DS's Ninja Kirby, vtable 0x0217AE0C; PrmKirbyNinja.kgb):
 * • B (0x020C9D80): in the air with Down held, the dive; else in the air, or standing on the ground, the kunai throw; on the ground
 *   moving (walking or dashing: the DS's pad +0x28 and +0x50), the dash's grab; in water, the water's own attack (KatAM's, Normal's).
 * • The kunai throw (0x020C9368, 0x020C9284, 0x020C91E0): composite 0 (9 frames, efc_cmn0's 21 at his feet), the kunai thrown (word 8,
 *   9: 14 forward, 14 up), composite 1 (14 frames), standing again. In the air he falls meanwhile. The kunai (0x020C7998, 0x020C771C,
 *   0x020C769C; sound 0xDB): ninja_ext's 48, flying ahead at word 10 (7.0) through everything for its first word 11 (5) frames, then
 *   stopped by walls: stuck there (0x020C7608: composite 3, 200 frames, blinking out; sound 0xDD), or by what it hits (+0x2C,
 *   0x020C783C: stuck in it, sound 0xDE); off the screen, gone. Its hit, ninja_ext.rca's 49: 10 (2 in KatAM's).
 * • The dash's grab (0x020C9144; sound 0xDF): composite 4 (6 frames: its grab box, .rca's 88, kind 6, -8 to 17 ahead, 0 to 29 up).
 *   Touching an enemy that can be held (+0x2C: kind 6), the grab (Izuna drop, below); else the slash (0x020C9058: composite 5, word
 *   40 (4.0) forward, slowing by word 41 (0.5) a frame; its box, .rca's 89, -18 to 21 ahead, 0 to 27 up, 20, i.e. 4), its end
 *   (0x020C8F78: composite 6, 8 frames), then standing.
 * • The grab (0x020C8E34, sound 0x113: 32; then 0x020C8CBC: 34, up at word 21 (8.0), sound 0x27, efc_cmn0's 21 at his feet, and one
 *   every 8 frames, through everything for word 24 (10) frames; then 0x020C8C1C: falling, gravity word 22 (0.5), capped at word 23
 *   (8.0); once falling, 0x020C8B38: 35, sound 0x49; through everything until 8 pixels over where he started, 0x020C89B4) holding
 *   what he grabbed word 27 (16) ahead of him; on the ground: what he holds hit for 50 (10), sound 0xE0, efc_cmn0's 22 at his feet,
 *   and he bounces back off it (word 25 (2.0) back, word 26 (3.0) up: composite 8, its box .rca's 92, 1 to 19 ahead, -9 to 9 up, 20)
 *   until he falls (0x020C97CC: under 1.0 up).
 * • The dive (0x020C88D8, 0x020C87E0, 0x020C86CC, 0x020C8618): stopped, 28 (4 frames), then down ahead at words 3 and 4 (4.0, 4.0;
 *   sound 0x129; composite 7, its box .rca's 91, -6 to 18 ahead, -9 to 25 up, 20) for at most word 5 (135) frames (then falling), on
 *   the ground 31 (6 frames) and standing.
 * • On a wall (+0x4C, 0x020C9F18: in the air, falling, a wall ahead and held toward): clinging (0x020C9C8C, 0x020C9AFC: 43, sound
 *   0x75, kept there while held toward it); A, the wall jump (0x020C984C: 44, then turned away, word 6 and 7 (3.0, 3.0) off it, sound
 *   0x76, composite 11, its box .rca's 95, -17 to 1 ahead, -10 to 8 up, 20; 0x020C97CC: until falling); B, the kunai thrown off it
 *   (0x020C9A18, 0x020C994C: composite 9, the kunai flying away from the wall, composite 10, clinging again);
 * • On water (+0x50, 0x020C9E8C, 0x020C967C, 0x020C9404: landing on its surface, he stands on it; held up by it, +0x54): standing (1)
 *   or running on it (53: word 1 (0.25) faster a frame up to word 2 (2.0), word 0 (0.25) slower without the pad), sound 0xE2; B, A as
 *   on the ground; Down, into it.
 * • The scroll (HasScroll 16): B held as the throw, the slash, the dive's landing ends (0x020C91E0, 0x020C8ED4, 0x020C8618): the art
 *   (0x020C84F0: composite 13, on water 18, sound 0xE1; 0x020C8390, 0x020C8130: held, B let go before word 43 (30) frames ends it;
 *   0x020C7F04: charged, composite 14 (19), held; let go, 0x020C7EA0: composite 15 (20), seven flames (0x020C7DD4, 0x020C7B9C:
 *   words 45-58, their way out and frames: at him after 1, 16 pixels either side after 8, 32 after 16, 48 after 24, going out at
 *   those speeds, gone in a wall or over the void; sound 0x58 each), standing after word 44 (60) frames. Each flame (0x020C7A38):
 *   composite 16, on water the geyser 21, its boxes (.rca's 64, 72: rising up to 70 pixels) hitting for 40 (8)); */
#include "../abilities.h"
#include "../scrolls.h"
#include "../squeak.h"
#include "main.h"
#include "data.h"
#include "constants/kirby.h"
#include "constants/object_types.h"

void KirbyEnterWater(struct Kirby *);
void KirbyStartWaterMovement(struct Kirby *);
void sub_0803FE74(struct Kirby *);
void sub_08044EA8(struct Kirby *);
void sub_08043360(struct Kirby *);
void sub_0805B1B8(struct Kirby *);

#define SCROLL_NINJA 16                     // The Ninja scroll's number (the DS's HasScroll 16);
#define NINJA_HIT 0x20002003                // KatAM's Cutter's hit (gUnk_082DDFC8): the kunai, the slash, the dive, the flip;
#define NINJA_FIRE_HIT 0x20000103           // KatAM's Fire's (the art's flames);
#define NINJA_WATER_HIT 0x20000043          // KatAM's Parasol's (its geysers);
#define NINJA_KNOCK 0x100
#define NINJA_THROW_FRAMES 9                // composite 0's;
#define NINJA_THROW_END_FRAMES 14           // composite 1's;
#define NINJA_DASH_FRAMES 6                 // composite 4's;
#define NINJA_GRAB_BOX_FRAMES 4             // .rca's 88: its first 4 frames;
#define NINJA_SLASH_FRAMES 6                // composite 5's (a loop's once through);
#define NINJA_SLASH_END_FRAMES 8            // composite 6's;
#define NINJA_SLASH_SPEED SQUEAK_SPEED(16384)   // word 40;
#define NINJA_SLASH_SLOW SQUEAK_SPEED(2048)     // word 41;
#define NINJA_DIVE_START_FRAMES 4           // 28's;
#define NINJA_DIVE_SPEED SQUEAK_SPEED(16384)    // words 3, 4;
#define NINJA_DIVE_MOST 135                 // word 5;
#define NINJA_DIVE_LAND_FRAMES 6            // 31's;
#define NINJA_KUNAI_X 14                    // words 8, 9;
#define NINJA_KUNAI_Y 14
#define NINJA_KUNAI_SPEED SQUEAK_SPEED(28672)   // word 10;
#define NINJA_KUNAI_THROUGH 5               // word 11;
#define NINJA_KUNAI_STUCK_FRAMES 200        // composite 3's;
#define NINJA_KUNAI_STRENGTH 2              // .rca 49's 10;
#define NINJA_KUNAI_MARGIN 16               // Pixels round the screen it goes out of to end;
#define NINJA_GRAB_FRAMES 8                 // 32's;
#define NINJA_GRAB_RISE SQUEAK_SPEED(32768)     // word 21;
#define NINJA_GRAB_GRAVITY SQUEAK_SPEED(2048)   // word 22;
#define NINJA_GRAB_FALL_CAP SQUEAK_SPEED(32768) // word 23;
#define NINJA_GRAB_RISE_FRAMES 10           // word 24;
#define NINJA_GRAB_PUFF_EVERY 8
#define NINJA_GRAB_HOLD 16                  // word 27: pixels ahead of him;
#define NINJA_GRAB_BACK SQUEAK_SPEED(8192)      // word 25;
#define NINJA_GRAB_BOUNCE SQUEAK_SPEED(12288)   // word 26;
#define NINJA_SLAM_STRENGTH 10              // the DS's 50;
#define NINJA_STRENGTH 4                    // .rca's 89, 91, 92, 95: 20;
#define NINJA_WALL_JUMP SQUEAK_SPEED(12288)     // words 6, 7;
#define NINJA_CLING_PUSH 0x100              // (the DS's 1.0 toward the wall, kept on it);
#define NINJA_CLING_JUMP_FRAMES 6           // 44's;
#define NINJA_WALL_THROW_FRAMES 9           // composite 9's;
#define NINJA_WATER_SLOW SQUEAK_SPEED(1024)     // word 0;
#define NINJA_WATER_FAST SQUEAK_SPEED(1024)     // word 1;
#define NINJA_WATER_MOST SQUEAK_SPEED(8192)     // word 2;
#define NINJA_ART_CHARGE 30                 // word 43;
#define NINJA_ART_AFTER 60                  // word 44;
#define NINJA_ART_START_FRAMES 4            // composite 13's (a loop's once through);
#define NINJA_FLAMES 7
#define NINJA_FLAME_STRENGTH 8              // .rca's 64, 72: 40;
#define NINJA_FLAME_FRAMES 34               // composites 16, 21;

static void NinjaThrow(struct Kirby *kirby);
static void NinjaThrowEnd(struct Kirby *kirby);
static void NinjaDash(struct Kirby *kirby);
static void NinjaSlash(struct Kirby *kirby);
static void NinjaSlashEnd(struct Kirby *kirby);
static void NinjaDiveStart(struct Kirby *kirby);
static void NinjaDive(struct Kirby *kirby);
static void NinjaDiveLand(struct Kirby *kirby);
static void NinjaCling(struct Kirby *kirby);
static void NinjaClingJump(struct Kirby *kirby);
static void NinjaWallJump(struct Kirby *kirby);
static void NinjaWallThrow(struct Kirby *kirby);
static void NinjaWallThrowEnd(struct Kirby *kirby);
static void NinjaGrab(struct Kirby *kirby);
static void NinjaGrabRise(struct Kirby *kirby);
static void NinjaGrabFall(struct Kirby *kirby);
static void NinjaWater(struct Kirby *kirby);
static void NinjaArt(struct Kirby *kirby);
static void NinjaArtHold(struct Kirby *kirby);
static void NinjaArtCharged(struct Kirby *kirby);
static void NinjaArtRelease(struct Kirby *kirby);
static void AnimalScratch(struct Kirby *kirby);
static void AnimalDash(struct Kirby *kirby);
static void AnimalLeap(struct Kirby *kirby);
static void AnimalFlurry(struct Kirby *kirby);
static void AnimalThrow(struct Kirby *kirby);
static void AnimalDig(struct Kirby *kirby);
static void AnimalLookUp(struct Kirby *kirby);
static void AnimalRoll(struct Kirby *kirby);
static void AnimalSpin(struct Kirby *kirby);
static void AnimalRollEnd(struct Kirby *kirby);
static void AnimalBounce(struct Kirby *kirby);
static void BubbleStart(struct Kirby *kirby);
static void BubbleBlow(struct Kirby *kirby);
static void BubbleEnd(struct Kirby *kirby);
static void BubbleBig(struct Kirby *kirby);
static void BubbleQuiet(void);
static void MetalStompStart(struct Kirby *kirby);
static void MetalStomp(struct Kirby *kirby);
static void MetalBounce(struct Kirby *kirby);
static void MetalRollStart(struct Kirby *kirby);
static void MetalRoll(struct Kirby *kirby);
static void MetalRollEnd(struct Kirby *kirby);

// (the flames' way out and frames, words 45-58: pixels, frames);
static const s8 gNinjaFlames[NINJA_FLAMES][2] = { { 0, 1 }, { 16, 8 }, { -16, 8 }, { 32, 16 }, { -32, 16 }, { 48, 24 }, { -48, 24 } };
// (the flame pillar's boxes, .rca's 64, a step a frame (its last held): left, top, right, bottom, Y up from its feet);
static const s8 gNinjaFlameBoxes[][4] = {
    { -16, 15, 16, -1 }, { -14, 32, 14, -1 }, { -11, 32, 12, 16 }, { -13, 48, 13, 21 }, { -11, 52, 10, 27 }, { -11, 55, 10, 33 },
    { -11, 59, 10, 38 }, { -10, 59, 10, 37 }, { -11, 62, 10, 41 }, { -11, 65, 10, 43 }, { -13, 68, 12, 44 }, { -14, 68, 10, 48 },
};
// (the geyser's, .rca's 72);
static const s8 gNinjaGeyserBoxes[][4] = {
    { -18, 15, 11, -3 }, { -18, 15, 11, -3 }, { -11, 32, 11, -3 }, { -9, 15, 11, -3 }, { -10, 44, 11, 26 }, { -8, 50, 9, 32 },
    { -9, 54, 9, 35 }, { -9, 54, 10, 34 }, { -9, 59, 10, 38 }, { -8, 57, 9, 37 }, { -11, 62, 9, 40 }, { -11, 61, 8, 42 },
    { -10, 65, 10, 43 }, { -9, 65, 9, 44 }, { -12, 66, 12, 44 }, { -13, 70, 11, 46 },
};

static bool32 Water(struct ObjectBase *b, s32 x, s32 y);

/* MoveGravity(); NidlAbilityGravity, and Ninja held up by the water's surface (the DS's +0x54: his flag 0x80000 on out of water, its
 * surface a floor to him): falling onto it, he's on it; */
static void MoveGravity(struct Kirby *kirby) {
    s32 feet = kirby->base.y + SCROLL_FEET;

    NidlAbilityGravity(kirby);
    if (kirby->base.unk62 & 4 || kirby->base.yspeed > 0) return;
    if (Water(&kirby->base, kirby->base.x, feet - kirby->base.yspeed + 0x100) && !Water(&kirby->base, kirby->base.x, feet - 0x800)) {
        kirby->base.y = (((feet - kirby->base.yspeed + 0x100) >> 12) << 12) - SCROLL_FEET;
        kirby->base.yspeed = 0;
        kirby->base.unk62 |= 4;
    }
}

// Facing(); As KatAM's attacks start: facing the way he's held;
static void Facing(struct Kirby *kirby) {
    if (kirby->movementState & 0x20) kirby->base.flags |= 1;
    if (kirby->movementState & 0x10) kirby->base.flags &= ~1;
}

// Dir(); 1 facing right, -1 left;
#define Dir(kirby) ((kirby)->base.flags & 1 ? -1 : 1)

// StartState(); A move's state from its first frame;
static void StartState(struct Kirby *kirby, void (*state)(struct Kirby *)) {
    kirby->stateFn = state;
    kirby->base.counter = 0;
}

// Moving(); His state is one of the moves' (the piece drawing him stays);
static bool32 Moving(struct Kirby *kirby) {
    void (*s)(struct Kirby *) = kirby->stateFn;

    return s == NinjaThrow || s == NinjaThrowEnd || s == NinjaDash || s == NinjaSlash || s == NinjaSlashEnd || s == NinjaDiveStart
        || s == NinjaDive || s == NinjaDiveLand || s == NinjaCling || s == NinjaClingJump || s == NinjaWallJump || s == NinjaWallThrow
        || s == NinjaWallThrowEnd || s == NinjaGrab || s == NinjaGrabRise || s == NinjaGrabFall || s == NinjaWater || s == NinjaArt
        || s == NinjaArtHold || s == NinjaArtCharged || s == NinjaArtRelease
        || s == AnimalScratch || s == AnimalDash || s == AnimalLeap || s == AnimalFlurry || s == AnimalThrow || s == AnimalDig
        || s == AnimalLookUp || s == AnimalRoll || s == AnimalSpin || s == AnimalRollEnd || s == AnimalBounce
        || s == BubbleStart || s == BubbleBlow || s == BubbleEnd || s == BubbleBig
        || s == MetalStompStart || s == MetalStomp || s == MetalBounce || s == MetalRollStart || s == MetalRoll || s == MetalRollEnd;
}

/* The held enemy (the grab's): its own state stopped (KatAM's objects' state, unk78, as an inhale's sub_0809C48C), carried by the
 * engine nowhere (flags 0x800: no speed, 0x100: no terrain, 0x200: no hit box), put where he holds it each frame; let go, its state
 * back. Kept on his moves' piece (in RAM: the code's own statics would be in ROM), its task's main kept to see it's still the same
 * object; */
static void HeldState(struct Object *obj) {
}

static struct ScrollPiece *FindPiece(struct Kirby *kirby);

// HeldOf(); What the Kirby holds (NULL: none, or gone);
static struct Object *HeldOf(struct Kirby *kirby) {
    struct ScrollPiece *p = FindPiece(kirby);
    struct Object *obj;

    if (p == NULL || (obj = p->held) == NULL) return NULL;
    if (p->heldTask->main != p->heldMain || TaskGetStructPtr(p->heldTask) != obj || obj->base.flags & 0x1000 || obj->unk78 != HeldState) {
        p->held = NULL;
        return NULL;
    }
    return obj;
}

// LetGo(); What he holds let go: its state and flags back;
static void LetGo(struct Kirby *kirby) {
    struct ScrollPiece *p = FindPiece(kirby);
    struct Object *obj = HeldOf(kirby);

    if (obj != NULL) {
        obj->unk78 = p->heldState;
        obj->base.flags = (obj->base.flags & ~(0x800 | 0x100 | 0x200)) | (p->heldFlags & (0x800 | 0x100 | 0x200));
        obj->base.xspeed = obj->base.yspeed = 0;
    }
    if (p != NULL) p->held = NULL;
}

// HoldAt(); What he holds at his hand: `ahead` pixels ahead of him, `down` below his middle;
static void HoldAt(struct Kirby *kirby, s32 ahead, s32 down) {
    struct Object *obj = HeldOf(kirby);

    if (obj == NULL) return;
    obj->base.x = kirby->base.x + Dir(kirby) * (ahead << 8);
    obj->base.y = kirby->base.y + (down << 8);
    obj->base.roomId = kirby->base.roomId;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags = (obj->base.flags & ~1) | (kirby->base.flags & 1 ? 0 : 1);
}

// Hold(); Ninja's: word 27 ahead of him;
static void Hold(struct Kirby *kirby) {
    HoldAt(kirby, NINJA_GRAB_HOLD, 0);
}

// Grabbable(); An enemy he can hold (the DS's 0x020193FC refuses bosses and what no attack takes): KatAM's enemies, not untouchable;
static bool32 Grabbable(struct Object *obj, struct Kirby *kirby) {
    if (obj->base.header.kind != 1 || obj->base.flags & 0x1000 || obj->base.roomId != kirby->base.roomId) return FALSE;
    if (!ObjType0To37(obj) || obj->type == OBJ_MIRRA || (obj->base.unk5C & 7) >= 6 || obj->unk78 == HeldState) return FALSE;
    return obj->unk80 > 0;
}

// HitBox(); An object's hit box (sub_0803E2B0's: its center's offset and half sizes, turned with its facing) in the room;
static void HitBox(struct ObjectBase *b, s32 *r) {
    s32 x = b->x + ((b->flags & 1) ? -(b->unk38 + b->unk3A) : b->unk38 + b->unk3A) * 0x100, y = b->y + (b->unk39 + b->unk3B) * 0x100;

    r[0] = x - b->unk3A * 0x100;
    r[2] = x + b->unk3A * 0x100;
    r[1] = y - b->unk3B * 0x100;
    r[3] = y + b->unk3B * 0x100;
}

/* GrabTouch(); The grab box (.rca's 88: -8 to 17 ahead, 0 to 29 up from his feet) on an enemy he can hold: it's held (the first found,
 * walking the task list as squeak.c's SqueakBossAlive()); */
static struct ScrollPiece *PieceOf(struct Kirby *kirby);

static struct Object *GrabTouchBox(struct Kirby *kirby, s8 left, s8 top, s8 right, s8 bottom) {
    struct Task *t = &gTasks[0];
    struct ScrollPiece *p = PieceOf(kirby);
    s32 feet = kirby->base.y + SCROLL_FEET, box[4], r[4];
    u8 n;

    if (Dir(kirby) > 0) {
        box[0] = kirby->base.x + (left << 8);
        box[2] = kirby->base.x + (right << 8);
    } else {
        box[0] = kirby->base.x - (right << 8);
        box[2] = kirby->base.x - (left << 8);
    }
    box[1] = feet - (top << 8);
    box[3] = feet - (bottom << 8);
    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *obj;

        t = (struct Task *)(IWRAM_START + t->next);
        obj = TaskGetStructPtr(t);
        if (!Grabbable(obj, kirby)) continue;
        HitBox(&obj->base, r);
        if (r[2] < box[0] || r[0] > box[2] || r[3] < box[1] || r[1] > box[3]) continue;
        if (p == NULL) return NULL;
        p->held = obj;
        p->heldTask = t;
        p->heldMain = t->main;
        p->heldState = obj->unk78;
        p->heldFlags = obj->base.flags;
        obj->unk78 = HeldState;
        obj->base.flags |= 0x800 | 0x100 | 0x200;
        return obj;
    }
    return NULL;
}

static struct Object *GrabTouch(struct Kirby *kirby) {
    struct Object *obj = GrabTouchBox(kirby, -8, 29, 17, 0);

    if (obj != NULL) Hold(kirby);
    return obj;
}

// MoveBox(); The move's hit this frame (the DS's box from his feet, Y up; strength 0: none), on his piece;
static void MoveBox(struct Kirby *kirby, s8 left, s8 top, s8 right, s8 bottom, s8 strength);

// MoveStep(); His piece: at his feet, his looks (his body in his own colors), his move's box; gone with his moves (what he holds let go);
static bool32 MoveStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;

    if (!Moving(kirby) || kirby->ability < SQUEAK_FIRST_ABILITY) {
        // (Bubble's blowing sounds stopped with its moves, however they end: a hurt, a door);
        if (p->shown >= BUBBLE_LOOK_START && p->shown <= BUBBLE_LOOK_BIG) BubbleQuiet();
        LetGo(kirby);
        p->base.flags |= 0x1000;
        kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
        p->hides = FALSE;
        return FALSE;
    }
    Follow(p);
    p->base.roomId = kirby->base.roomId;
    // (his body in his own palette: his spray's; Metal's all metal, in its own, as the DS draws him);
    if (kirby->ability != SQUEAK_ABILITY_METAL) p->look[0].palId = kirby->base.sprite.palId;
    if (p->look[0].effect != SCROLL_NO_LOOK) {
        kirby->base.sprite.unk8 |= TORNADO_HIDE;
        p->hides = TRUE;
    } else if (p->hides) {
        // (no look: Kirby's own sprite, his ability's motions: Animal's bounce);
        kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
        p->hides = FALSE;
    }
    return TRUE;
}

// FindPiece(); His moves' piece (NULL: none);
static struct ScrollPiece *FindPiece(struct Kirby *kirby) {
    u8 i;
    struct ScrollPiece *p;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        if (!ScrollPieceIs(&gTasks[i])) continue;
        p = TaskGetStructPtr(&gTasks[i]);
        if (p->step == MoveStep && p->kirby == kirby && !(p->base.flags & 0x1000)) return p;
    }
    return NULL;
}

// PieceOf(); His moves' piece, found or made (NULL: none could be);
static struct ScrollPiece *PieceOf(struct Kirby *kirby) {
    struct ScrollPiece *p = FindPiece(kirby);

    if (p != NULL) return p;
    p = ScrollPieceCreate(kirby, SCROLL_MOVE, 0, NINJA_KNOCK, NINJA_HIT);
    if (p == NULL) return NULL;
    p->step = MoveStep;
    return p;
}

// SetLook(); A look of the piece's (`restart`: from its start again, as the DS sets an animation again);
static void SetLook(struct ScrollPiece *p, u8 look, bool32 restart) {
    u8 i;

    if (p == NULL || (p->shown == look && !restart)) return;
    p->shown = look;
    for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
    for (i = 0; i < SQUEAK_LOOK_LAYERS; i++)
        if (gSqueakAbilityLooks[look][i] != SQUEAK_LOOK_NONE) LookStart(p, i, gSqueakAbilityLooks[look][i], 0, 0);
}

// Look(); His look (his moves' piece's);
static void Look(struct Kirby *kirby, u8 look, bool32 restart) {
    struct ScrollPiece *p = PieceOf(kirby);

    SetLook(p, look, restart);
    if (p != NULL) MoveStep(p);
}

static void MoveBox(struct Kirby *kirby, s8 left, s8 top, s8 right, s8 bottom, s8 strength) {
    struct ScrollPiece *p = PieceOf(kirby);

    if (p == NULL) return;
    p->base.unk68 = NINJA_HIT;
    Box(p, left, top, right, bottom, strength);
}

// Puff(); efc_cmn0's 21 at his feet (the DS's 0x02018C24, its types 5 and 11), or 22 (its type 6: the slam's);
static void Puff(struct Kirby *kirby, bool32 big) {
    SqueakEffectInRoom(kirby->base.roomId, big ? SQUEAK_FX_LAND_BIG : SQUEAK_FX_KIRBY_PUFF, kirby->base.x, kirby->base.y + SCROLL_FEET);
}

// ArtNext(); A move's end (the DS's 0x020C91E0's and others'): with B held and the scroll, the art; else standing (or falling);
static void ArtNext(struct Kirby *kirby) {
    MoveBox(kirby, 0, 0, 0, 0, 0);
    if (kirby->movementState & 2 && SqueakScrollOwned(SCROLL_NINJA)) {
        StartState(kirby, NinjaArt);
        return;
    }
    NidlAbilityEnd(kirby);
}

/* Kunai. A piece of its own (SCROLL_MOVE, its step KunaiStep): ahead at word 10 through everything for word 11 frames, then stopped by a
 * wall (KatAM's solid tiles: attribute 0x200) where it sticks (StuckStep: composite 3, 200 frames), or by what it hits (its hit's
 * flag 0x40000) where it sticks too; off the screen, gone; */
#define KUNAI_STUCK 1
static bool32 StuckStep(struct ScrollPiece *p);

// Solid(); A wall's tile (KatAM's collision attribute 0x200) where (x, y) is, in the room;
static bool32 Solid(struct ObjectBase *b, s32 x, s32 y) {
    struct LevelInfo *level = &gCurLevelInfo[b->unk56];

    if (x < level->levelMinPosition.x || y < level->levelMinPosition.y || x > level->levelMaxPosition.x || y > level->levelMaxPosition.y)
        return FALSE;
    return (gCollisionAttributes[GetCollisionTile(b->unk56, x >> 12, y >> 12)] & 0x200) != 0;
}

// Water(); The water's tile (KatAM's collision attribute 2) where (x, y) is;
static bool32 Water(struct ObjectBase *b, s32 x, s32 y) {
    struct LevelInfo *level = &gCurLevelInfo[b->unk56];

    if (x < level->levelMinPosition.x || y < level->levelMinPosition.y || x > level->levelMaxPosition.x || y > level->levelMaxPosition.y)
        return FALSE;
    return (gCollisionAttributes[GetCollisionTile(b->unk56, x >> 12, y >> 12)] & 2) != 0;
}

// OffScreen(); Out of the camera's view by more than `margin` pixels;
static bool32 OffScreen(struct ObjectBase *b, s32 margin) {
    const struct LevelInfo *level = &gCurLevelInfo[b->unk56];
    s32 x = (b->x - level->viewportPosition.x) >> 8, y = (b->y - level->viewportPosition.y) >> 8;

    return x < -margin || x > DISPLAY_WIDTH + margin || y < -margin || y > DISPLAY_HEIGHT + margin;
}

static void KunaiStick(struct ScrollPiece *p, u16 sound) {
    PlaySfx(&p->base, SQUEAK_SE(sound));
    p->step = StuckStep;
    p->clock = 0;
    p->base.xspeed = 0;
    Box(p, 0, 0, 0, 0, 0);
    SetLook(p, NINJA_LOOK_KUNAI_STUCK, TRUE);
}

static bool32 KunaiStep(struct ScrollPiece *p) {
    if (p->base.flags & 0x40000) {
        KunaiStick(p, 0xDE);
        return TRUE;
    }
    p->base.x += p->base.xspeed;
    if (OffScreen(&p->base, NINJA_KUNAI_MARGIN)) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    if (p->clock >= NINJA_KUNAI_THROUGH && Solid(&p->base, p->base.x, p->base.y)) {
        KunaiStick(p, 0xDD);
        return TRUE;
    }
    // (.rca's 49: -10 to 0 ahead, -6 to 7 up from its middle);
    Box(p, -10, 7, 0, -6, NINJA_KUNAI_STRENGTH);
    return TRUE;
}

static bool32 StuckStep(struct ScrollPiece *p) {
    if (p->clock >= NINJA_KUNAI_STUCK_FRAMES) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    return TRUE;
}

// Kunai(); One thrown from his hand (words 8, 9 from his feet), ahead (`away`: the other way, off a wall);
static void Kunai(struct Kirby *kirby, bool32 away) {
    struct ScrollPiece *p = ScrollPieceCreate(kirby, SCROLL_MOVE, 0, NINJA_KNOCK, NINJA_HIT);
    s32 dir = away ? -Dir(kirby) : Dir(kirby);

    if (p == NULL) return;
    PlaySfx(&kirby->base, SQUEAK_SE(0xDB));
    p->step = KunaiStep;
    p->base.x = kirby->base.x + dir * (NINJA_KUNAI_X << 8);
    p->base.y = kirby->base.y + SCROLL_FEET - (NINJA_KUNAI_Y << 8);
    p->base.xspeed = dir * NINJA_KUNAI_SPEED;
    p->base.flags = (p->base.flags & ~1) | (dir < 0 ? 1 : 0);
    SetLook(p, NINJA_LOOK_KUNAI, TRUE);
    KunaiStep(p);
}

/* NinjaB(); The B button (gNidlAbilityTables' landB; the DS's 0x020C9D80): in the air, Down held, the dive, else the throw; on the
 * ground, moving, the dash's grab, else the throw; */
void NinjaB(struct Kirby *kirby) {
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    Facing(kirby);
    // (in the air: no floor under him, KatAM's unk62 4; its flag 0x20 is set as a jump or fall starts, not kept);
    if (!(kirby->base.unk62 & 4)) {
        StartState(kirby, kirby->movementState & 0x80 ? NinjaDiveStart : NinjaThrow);
    } else if (kirby->base.flags & 0x10 || kirby->movementState & 0x30) {
        PlaySfx(&kirby->base, SQUEAK_SE(0xDF));
        StartState(kirby, NinjaDash);
    } else {
        StartState(kirby, NinjaThrow);
    }
    kirby->animationIndex = NIDL_MOTION_SLOT;
    kirby->stateFn(kirby);
}

// The throw (0x020C9284): composite 0, the kunai, composite 1 (0x020C91E0), then standing or the art;
static void NinjaThrow(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) {
        Look(kirby, NINJA_LOOK_THROW, TRUE);
        Puff(kirby, FALSE);
    }
    MoveGravity(kirby);
    if (++kirby->base.counter < NINJA_THROW_FRAMES) return;
    Kunai(kirby, FALSE);
    StartState(kirby, NinjaThrowEnd);
    Look(kirby, NINJA_LOOK_THROW_END, TRUE);
}

static void NinjaThrowEnd(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    MoveGravity(kirby);
    if (++kirby->base.counter >= NINJA_THROW_END_FRAMES) ArtNext(kirby);
}

// The dash's grab (0x020C9144): composite 4, its grab box on an enemy he can hold: the grab; else the slash;
static void NinjaDash(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) Look(kirby, NINJA_LOOK_DASH, TRUE);
    MoveGravity(kirby);
    if (kirby->base.counter < NINJA_GRAB_BOX_FRAMES && GrabTouch(kirby) != NULL) {
        PlaySfx(&kirby->base, SQUEAK_SE(0x113));
        kirby->base.xspeed = kirby->base.yspeed = 0;
        StartState(kirby, NinjaGrab);
        Look(kirby, NINJA_LOOK_GRAB, TRUE);
        return;
    }
    if (++kirby->base.counter < NINJA_DASH_FRAMES) return;
    kirby->base.xspeed = Dir(kirby) * NINJA_SLASH_SPEED;
    StartState(kirby, NinjaSlash);
    Look(kirby, NINJA_LOOK_SLASH, TRUE);
    MoveBox(kirby, -18, 27, 21, 0, NINJA_STRENGTH);
}

// The slash (0x020C9058, 0x020C8F78): word 40 ahead, slowing by word 41;
static void NinjaSlash(struct Kirby *kirby) {
    s16 v = kirby->base.xspeed;

    if (NidlAbilityInWater(kirby)) return;
    if (!(kirby->base.unk62 & 4)) {
        kirby->base.yspeed -= 38;
        if (kirby->base.yspeed < -0x280) kirby->base.yspeed = -0x280;
    }
    v = v > 0 ? (v > NINJA_SLASH_SLOW ? v - NINJA_SLASH_SLOW : 0) : (v < -NINJA_SLASH_SLOW ? v + NINJA_SLASH_SLOW : 0);
    kirby->base.xspeed = kirby->base.unk62 & 1 ? 0 : v;
    MoveBox(kirby, -18, 27, 21, 0, NINJA_STRENGTH);
    if (++kirby->base.counter < NINJA_SLASH_FRAMES) return;
    MoveBox(kirby, 0, 0, 0, 0, 0);
    StartState(kirby, NinjaSlashEnd);
    Look(kirby, NINJA_LOOK_SLASH_END, TRUE);
}

static void NinjaSlashEnd(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    MoveGravity(kirby);
    if (++kirby->base.counter >= NINJA_SLASH_END_FRAMES) ArtNext(kirby);
}

/* The grab (0x020C8E34: 32, then 0x020C8CBC: up, 34), the rise through everything (0x020C8C1C), the fall (0x020C8B38: 35; 0x020C89B4)
 * holding it, the slam and the bounce (composite 8, 0x020C97CC: NinjaWallJump's); his start's height kept per player; */
#define GRAB_START_Y(kirby) (PieceOf(kirby)->startY)
static void NinjaGrab(struct Kirby *kirby) {
    kirby->base.xspeed = kirby->base.yspeed = 0;
    Hold(kirby);
    if (++kirby->base.counter < NINJA_GRAB_FRAMES) return;
    if (HeldOf(kirby) == NULL) {
        NidlAbilityEnd(kirby);
        return;
    }
    GRAB_START_Y(kirby) = kirby->base.y;
    kirby->base.yspeed = NINJA_GRAB_RISE;
    PlaySfx(&kirby->base, SQUEAK_SE(0x27));
    Puff(kirby, FALSE);
    kirby->base.flags |= 0x20;
    StartState(kirby, NinjaGrabRise);
    Look(kirby, NINJA_LOOK_GRAB_RISE, TRUE);
}

static void NinjaGrabRise(struct Kirby *kirby) {
    s16 n = kirby->base.counter++;

    // (through everything: KatAM's terrain off, as the DS's flags 0x100 and 0x20000);
    kirby->base.flags |= 0x100;
    kirby->base.unk62 &= ~4;
    if (n >= NINJA_GRAB_RISE_FRAMES) {
        kirby->base.yspeed -= NINJA_GRAB_GRAVITY;
        if (kirby->base.yspeed < -NINJA_GRAB_FALL_CAP) kirby->base.yspeed = -NINJA_GRAB_FALL_CAP;
    } else if (n % NINJA_GRAB_PUFF_EVERY == 0 && n) {
        Puff(kirby, FALSE);
    }
    Hold(kirby);
    if (n >= NINJA_GRAB_RISE_FRAMES && kirby->base.yspeed < 0) {
        PlaySfx(&kirby->base, SQUEAK_SE(0x49));
        StartState(kirby, NinjaGrabFall);
        Look(kirby, NINJA_LOOK_GRAB_FALL, TRUE);
    }
}

static void NinjaGrabFall(struct Kirby *kirby) {
    struct Object *obj;

    kirby->base.yspeed -= NINJA_GRAB_GRAVITY;
    if (kirby->base.yspeed < -NINJA_GRAB_FALL_CAP) kirby->base.yspeed = -NINJA_GRAB_FALL_CAP;
    // (the terrain again once 8 pixels over where he started: KatAM's floor flag his again from the next frame);
    if (kirby->base.y >= GRAB_START_Y(kirby) - 0x800) {
        kirby->base.flags &= ~0x100;
    } else {
        kirby->base.flags |= 0x100;
        kirby->base.unk62 &= ~4;
    }
    Hold(kirby);
    if (!(kirby->base.unk62 & 4) || kirby->base.flags & 0x100) return;
    // The slam: what he holds hit (his piece's box on it, this frame), the bounce back off it;
    obj = HeldOf(kirby);
    PlaySfx(&kirby->base, SQUEAK_SE(0xE0));
    Puff(kirby, TRUE);
    LetGo(kirby);
    if (obj != NULL) MoveBox(kirby, NINJA_GRAB_HOLD - 12, 24, NINJA_GRAB_HOLD + 12, -8, NINJA_SLAM_STRENGTH);
    kirby->base.xspeed = -Dir(kirby) * NINJA_GRAB_BACK;
    kirby->base.yspeed = NINJA_GRAB_BOUNCE;
    kirby->base.flags |= 0x20;
    StartState(kirby, NinjaWallJump);
    kirby->unkD9 = 1;
    Look(kirby, NINJA_LOOK_GRAB_BOUNCE, TRUE);
}

// The dive (0x020C88D8, 0x020C87E0, 0x020C86CC, 0x020C8618): stopped, 28, down ahead (composite 7), on the ground 31;
static void NinjaDiveStart(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) Look(kirby, NINJA_LOOK_DIVE_START, TRUE);
    kirby->base.xspeed = kirby->base.yspeed = 0;
    if (++kirby->base.counter < NINJA_DIVE_START_FRAMES) return;
    kirby->base.xspeed = Dir(kirby) * NINJA_DIVE_SPEED;
    kirby->base.yspeed = -NINJA_DIVE_SPEED;
    PlaySfx(&kirby->base, SQUEAK_SE(0x129));
    StartState(kirby, NinjaDive);
    Look(kirby, NINJA_LOOK_DIVE, TRUE);
    MoveBox(kirby, -6, 25, 18, -9, NINJA_STRENGTH);
}

static void NinjaDive(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (++kirby->base.counter >= NINJA_DIVE_MOST) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        NidlAbilityEnd(kirby);
        return;
    }
    if (kirby->base.unk62 & 1) kirby->base.xspeed = 0;
    else kirby->base.xspeed = Dir(kirby) * NINJA_DIVE_SPEED;
    kirby->base.yspeed = -NINJA_DIVE_SPEED;
    MoveBox(kirby, -6, 25, 18, -9, NINJA_STRENGTH);
    if (!(kirby->base.unk62 & 4)) return;
    kirby->base.xspeed = kirby->base.yspeed = 0;
    MoveBox(kirby, 0, 0, 0, 0, 0);
    StartState(kirby, NinjaDiveLand);
    Look(kirby, NINJA_LOOK_DIVE_LAND, TRUE);
}

static void NinjaDiveLand(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    MoveGravity(kirby);
    if (++kirby->base.counter >= NINJA_DIVE_LAND_FRAMES) ArtNext(kirby);
}

/* On a wall (0x020C9C8C, 0x020C9AFC): kept on it (the DS's 1.0 toward it, no fall) while held toward it, a wall ahead; A the jump off,
 * B the kunai off it; else falling; */
static bool32 HeldToward(struct Kirby *kirby) {
    return kirby->movementState & (kirby->base.flags & 1 ? 0x20 : 0x10);
}

static void NinjaCling(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter++ == 0) Look(kirby, NINJA_LOOK_CLING, TRUE);
    kirby->base.xspeed = Dir(kirby) * NINJA_CLING_PUSH;
    kirby->base.yspeed = 0;
    if (kirby->unk11A & 1) {
        StartState(kirby, NinjaClingJump);
        Look(kirby, NINJA_LOOK_CLING_JUMP, TRUE);
        return;
    }
    if (kirby->unk11A & 2) {
        StartState(kirby, NinjaWallThrow);
        Look(kirby, NINJA_LOOK_WALL_THROW, TRUE);
        return;
    }
    if (kirby->base.counter > 1 && (!HeldToward(kirby) || !(kirby->base.unk62 & 1))) NidlAbilityEnd(kirby);
}

// The jump off it (0x020C984C, 0x020C97CC): 44, then turned away, off it at words 6 and 7, composite 11 till he falls;
static void NinjaClingJump(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.xspeed = Dir(kirby) * NINJA_CLING_PUSH;
    kirby->base.yspeed = 0;
    if (++kirby->base.counter < NINJA_CLING_JUMP_FRAMES) return;
    kirby->base.flags ^= 1;
    kirby->base.xspeed = Dir(kirby) * NINJA_WALL_JUMP;
    kirby->base.yspeed = NINJA_WALL_JUMP;
    PlaySfx(&kirby->base, SQUEAK_SE(0x76));
    StartState(kirby, NinjaWallJump);
    kirby->unkD9 = 0;
    Look(kirby, NINJA_LOOK_WALL_JUMP, TRUE);
}

// (the wall jump's flip and the slam's bounce: unkD9 1 for the bounce, its box .rca's 92, else the flip's, .rca's 95; till falling);
static void NinjaWallJump(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->unkD9) MoveBox(kirby, 1, 9, 19, -9, NINJA_STRENGTH);
    else MoveBox(kirby, -17, 8, 1, -10, NINJA_STRENGTH);
    kirby->base.yspeed -= 38;
    if (kirby->base.yspeed < -0x280) kirby->base.yspeed = -0x280;
    kirby->base.counter++;
    // (the floor he left only after his first frames off it: KatAM's floor flag is last frame's);
    if (kirby->base.yspeed < 0x100 || (kirby->base.unk62 & 4 && kirby->base.counter > 2)) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        NidlAbilityEnd(kirby);
    }
}

// The kunai off the wall (0x020C9A18, 0x020C994C): composite 9, the kunai away from it, composite 10, on it again;
static void NinjaWallThrow(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.xspeed = Dir(kirby) * NINJA_CLING_PUSH;
    kirby->base.yspeed = 0;
    if (++kirby->base.counter < NINJA_WALL_THROW_FRAMES) return;
    Kunai(kirby, TRUE);
    StartState(kirby, NinjaWallThrowEnd);
    Look(kirby, NINJA_LOOK_WALL_THROW_END, TRUE);
}

static void NinjaWallThrowEnd(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.xspeed = Dir(kirby) * NINJA_CLING_PUSH;
    kirby->base.yspeed = 0;
    if (++kirby->base.counter < NINJA_THROW_END_FRAMES) return;
    StartState(kirby, NinjaCling);
    kirby->base.counter = 1;
    Look(kirby, NINJA_LOOK_CLING, TRUE);
}

/* On water (0x020C967C, 0x020C9404): standing on its surface (KatAM's: the top of a water tile under his feet, none over it), running
 * on it (word 1 a frame up to word 2, slowing by word 0 without the pad); A the jump, B as on the ground, Down into it; */
#define WATER_RUNNING(kirby) ((kirby)->unkD9)
static void NinjaWater(struct Kirby *kirby) {
    s32 feet = kirby->base.y + SCROLL_FEET;
    s16 v = kirby->base.xspeed;
    bool32 moving;

    if (kirby->base.counter++ == 0) {
        WATER_RUNNING(kirby) = 0xFF;
        PlaySfx(&kirby->base, SQUEAK_SE(0xE2));
    }
    // (held at the surface: his feet on the water tile's top);
    kirby->base.y = (((feet + 0x400) >> 12) << 12) - SCROLL_FEET;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~0x20;
    kirby->base.unk62 |= 4;
    if (!Water(&kirby->base, kirby->base.x, kirby->base.y + SCROLL_FEET + 0x100) || Water(&kirby->base, kirby->base.x, kirby->base.y)) {
        NidlAbilityEnd(kirby);
        return;
    }
    if (kirby->movementState & 0x80) {
        // (into the water: under its surface);
        kirby->base.y += 0x800;
        kirby->base.flags &= ~0x40;
        KirbyEnterWater(kirby);
        KirbyStartWaterMovement(kirby);
        return;
    }
    if (kirby->unk11A & 1) {
        sub_08043360(kirby);
        return;
    }
    if (kirby->unk11A & 2) {
        NinjaB(kirby);
        return;
    }
    if (kirby->movementState & 0x30) {
        Facing(kirby);
        v += Dir(kirby) * NINJA_WATER_FAST;
        if (v > NINJA_WATER_MOST) v = NINJA_WATER_MOST;
        if (v < -NINJA_WATER_MOST) v = -NINJA_WATER_MOST;
    } else {
        v = v > 0 ? (v > NINJA_WATER_SLOW ? v - NINJA_WATER_SLOW : 0) : (v < -NINJA_WATER_SLOW ? v + NINJA_WATER_SLOW : 0);
    }
    kirby->base.xspeed = kirby->base.unk62 & 1 ? 0 : v;
    moving = kirby->base.xspeed != 0;
    if (WATER_RUNNING(kirby) != moving) {
        WATER_RUNNING(kirby) = moving;
        if (moving) {
            Look(kirby, NINJA_LOOK_WATER_RUN, TRUE);
        } else {
            struct ScrollPiece *p = PieceOf(kirby);
            u8 i;

            // (standing: his own sprite's, KatAM's standing index, shown);
            if (p != NULL) {
                for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
                p->shown = 0xFF;
                kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
            }
            kirby->animationIndex = 0;
        }
    }
}

/* The art (the scroll's: 0x020C84F0, 0x020C8390, 0x020C8130, 0x020C7F04, 0x020C7EA0, 0x020C7E40): its start once through, held
 * charging (let go before word 43, standing), charged, let go, the flames (pieces of their own), standing after word 44 frames; on
 * water (his water state's surface under him) the water's looks; */
static bool32 OnWater(struct Kirby *kirby) {
    return Water(&kirby->base, kirby->base.x, kirby->base.y + SCROLL_FEET + 0x100) && !Water(&kirby->base, kirby->base.x, kirby->base.y);
}

static void NinjaArt(struct Kirby *kirby) {
    bool32 water = OnWater(kirby);

    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) {
        Look(kirby, water ? NINJA_LOOK_ART_WATER : NINJA_LOOK_ART, TRUE);
        PlaySfx(&kirby->base, SQUEAK_SE(0xE1));
        kirby->base.xspeed = 0;
    }
    if (!water) MoveGravity(kirby);
    if (++kirby->base.counter < NINJA_ART_START_FRAMES) return;
    StartState(kirby, NinjaArtHold);
    Look(kirby, water ? NINJA_LOOK_ART_WATER : NINJA_LOOK_ART, TRUE);
}

static void NinjaArtHold(struct Kirby *kirby) {
    bool32 water = OnWater(kirby);

    if (NidlAbilityInWater(kirby)) return;
    Look(kirby, water ? NINJA_LOOK_ART_WATER : NINJA_LOOK_ART, FALSE);
    if (++kirby->base.counter >= NINJA_ART_CHARGE) {
        StartState(kirby, NinjaArtCharged);
        Look(kirby, water ? NINJA_LOOK_ART_WATER_CHARGED : NINJA_LOOK_ART_CHARGED, TRUE);
        return;
    }
    if (!water) MoveGravity(kirby);
    if (!(kirby->movementState & 2)) NidlAbilityEnd(kirby);
}

static bool32 FlameStep(struct ScrollPiece *p);

// Flames(); The seven flames (0x020C7DD4) from his feet;
static void Flames(struct Kirby *kirby) {
    u8 i;

    for (i = 0; i < NINJA_FLAMES; i++) {
        struct ScrollPiece *p = ScrollPieceCreate(kirby, SCROLL_MOVE, i, NINJA_KNOCK, NINJA_FIRE_HIT);

        if (p == NULL) return;
        p->step = FlameStep;
        p->base.x = kirby->base.x;
        p->base.y = kirby->base.y + SCROLL_FEET;
        // (its way out: word 45 + 2i pixels over word 46 + 2i frames, in KatAM's 24.8);
        p->base.xspeed = Dir(kirby) * ((gNinjaFlames[i][0] << 8) / gNinjaFlames[i][1]);
        p->vars[0] = gNinjaFlames[i][1];
    }
}

/* FlameStep(); A flame going out (gone in a wall, or with no floor under it), then the pillar (composite 16; on water, the geyser 21) and
 * its boxes; */
static bool32 FlameStep(struct ScrollPiece *p) {
    s32 out = p->vars[0];

    if (p->clock < out) {
        p->base.x += p->base.xspeed;
        if (Solid(&p->base, p->base.x, p->base.y - 0x800)
            || (!Solid(&p->base, p->base.x, p->base.y + 0x100) && !(gCollisionAttributes[GetCollisionTile(p->base.unk56, p->base.x >> 12, (p->base.y + 0x100) >> 12)] & 1)
                && !(Water(&p->base, p->base.x, p->base.y + 0x100) && !Water(&p->base, p->base.x, p->base.y - 0x100)))) {
            p->base.flags |= 0x1000;
            return FALSE;
        }
        Box(p, 0, 0, 0, 0, 0);
        return TRUE;
    }
    if (p->clock == out) {
        bool32 water = Water(&p->base, p->base.x, p->base.y + 0x100) && !Water(&p->base, p->base.x, p->base.y - 0x100);

        p->vars[1] = water;
        p->base.unk68 = water ? NINJA_WATER_HIT : NINJA_FIRE_HIT;
        PlaySfx(&p->base, SQUEAK_SE(0x58));
        SetLook(p, water ? NINJA_LOOK_ART_GEYSER : NINJA_LOOK_ART_FLAME, TRUE);
    }
    if (p->clock - out >= NINJA_FLAME_FRAMES) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    {
        u16 t = p->clock - out;
        const s8 *b;

        if (p->vars[1]) b = gNinjaGeyserBoxes[t < ARRAY_COUNT(gNinjaGeyserBoxes) ? t : ARRAY_COUNT(gNinjaGeyserBoxes) - 1];
        else b = gNinjaFlameBoxes[t < ARRAY_COUNT(gNinjaFlameBoxes) ? t : ARRAY_COUNT(gNinjaFlameBoxes) - 1];
        Box(p, b[0], b[1], b[2], b[3], NINJA_FLAME_STRENGTH);
    }
    return TRUE;
}

static void NinjaArtCharged(struct Kirby *kirby) {
    bool32 water = OnWater(kirby);

    if (NidlAbilityInWater(kirby)) return;
    Look(kirby, water ? NINJA_LOOK_ART_WATER_CHARGED : NINJA_LOOK_ART_CHARGED, FALSE);
    if (!water) MoveGravity(kirby);
    if (kirby->movementState & 2) return;
    StartState(kirby, NinjaArtRelease);
    Look(kirby, water ? NINJA_LOOK_ART_WATER_RELEASE : NINJA_LOOK_ART_RELEASE, TRUE);
    Flames(kirby);
}

static void NinjaArtRelease(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (!OnWater(kirby)) MoveGravity(kirby);
    if (++kirby->base.counter >= NINJA_ART_AFTER) NidlAbilityEnd(kirby);
}

/* Animal (the DS's Animal Kirby, vtable 0x021703E4; PrmKirbyAnimal.kgb; Gaw Gaw gives it, as the DS's):
 * • B (0x0208B7F0): on the ground with Down held over a dig cell, digging down; else a dig cell ahead (16 pixels ahead, 8 up from his
 *   feet), digging ahead; else dashing, the dash's crouch and leap, else the scratch. In the air, with the scroll (HasScroll 20) and
 *   not dashing: Down held, the spin; moving, the roll. In water, the water's own attack (KatAM's).
 * • The scratch (0x0208B754, 0x0208B6DC): animal_ext's 34 (22 frames, sound 0xE4), its box (.rca's 35: 13 to 53 ahead, 0 to 34 up, 20)
 *   in its frames 4-6; then standing.
 * • The dash's crouch (0x0208B62C, 0x0208B4F4: 21, sound 0xE6, slowing 0.1 a frame) and leap (0x0208B45C: 22, word 2 (2.0) ahead,
 *   word 3 (1.5) up, word 5 (0.1) less a frame; its grab box, .rca's 23, kind 6: -17 to 16 ahead, 4 to 27 up) till he lands or word
 *   1 (53) frames. An enemy it touches is held (+0x2C, 0x0208BAD0): the flurry (0x0208B378, 0x0208B224: 24, looped, for 67 frames;
 *   what he holds word 6 (16) ahead and word 7 (4) down, 4 more in its frames 17, 18, 21 and 22; each loop's frame 14, sound 0xE4 and
 *   its hit, 10), then the throw (0x0208B19C: 25, its box .rca's 26, -5 to 23 ahead, 2 to 37 up, 10; at its end what he holds let go
 *   with a hit of 10, sound 0xE5).
 * • Digging (the stage's +0x9C, 0x02216528: a dig cell, KatAMExpansionSqueakRooms.cs's SqueakDigCells(), cleared): down (0x0208A9D4:
 *   the cell 4 pixels under his feet, with word 0 the cells 16 pixels either side, 8 up too; centred on his cell, down 16 pixels in word
 *   8 (20) frames: 31), ahead (0x0208AC58: the cells 16 ahead at 8 and 24 up; his feet on the nearest row, ahead 16 pixels in 20
 *   frames: 30), up (Up held under a dig cell 17 pixels over his feet, 0x0208B114: looking up, 33, 0x0208AFA0; B, 0x0208ADE0: that
 *   cell and the side ones cleared, up 16 pixels in 20 frames: 32); sound 0xE3; each with its box (.rca's 31-33, 20); crouching, B
 *   over a dig cell digs down too (0x0208AB1C);
 * • The scroll (HasScroll 20), in the air: the roll (0x0208A868: 35, word 10 (4.0) back, word 11 (0.3) more ahead a frame up to word
 *   12 (4.0), level; sound 0x13E; 0x0208A59C: 36, its box .rca's 37 every 4 frames, 10) for word 9 (48) frames, or the spin (0x0208A3A0:
 *   38, word 17 (4.0) up, word 18 (0.3) less a frame, falling at word 19 (4.0) at most; 0x0208A10C: 39, its box .rca's 40 every 4
 *   frames) for word 16 (48) frames; then 37 (0x0208A094) and standing. Into a wall (the roll) or onto the ground (the spin), the
 *   bounce: word 13 (1.2) back, word 14 (3.0) up, a quake (0x02035FC4(7, 5, 2): KatAM's shortest), sound 0x66, falling. */
#define SCROLL_ANIMAL 20
#define ANIMAL_SCRATCH_FRAMES 22            // animal_ext's 34;
#define ANIMAL_SCRATCH_STRENGTH 4           // .rca's 35: 20;
#define ANIMAL_DASH_FRAMES 7                // 21's;
#define ANIMAL_DASH_SLOW SQUEAK_SPEED(410)  // (logged: 0.1 a frame);
#define ANIMAL_LEAP_X SQUEAK_SPEED(8192)    // word 2;
#define ANIMAL_LEAP_Y SQUEAK_SPEED(6144)    // word 3;
#define ANIMAL_LEAP_FALL SQUEAK_SPEED(410)  // word 5;
#define ANIMAL_LEAP_MOST 53                 // word 1;
#define ANIMAL_FLURRY_FRAMES 67             // (0x42 frames on);
#define ANIMAL_FLURRY_LOOP 22               // 24's;
#define ANIMAL_FLURRY_HIT 14
#define ANIMAL_HOLD_AHEAD 16                // word 6;
#define ANIMAL_HOLD_DOWN 4                  // word 7;
#define ANIMAL_THROW_FRAMES 12              // 25's;
#define ANIMAL_HIT_STRENGTH 2               // .rca's 25, 26, 37, 40: 10;
#define ANIMAL_DIG_STRENGTH 4               // .rca's 31-33: 20;
#define ANIMAL_DIG_FRAMES 20                // word 8;
#define ANIMAL_DIG_SPEED 0xCC               // (16 pixels in word 8's frames);
#define ANIMAL_ROLL_BACK SQUEAK_SPEED(16384)    // word 10;
#define ANIMAL_ROLL_ADD SQUEAK_SPEED(1229)      // word 11;
#define ANIMAL_ROLL_MOST SQUEAK_SPEED(16384)    // word 12;
#define ANIMAL_ROLL_START_FRAMES 18         // 35's;
#define ANIMAL_ROLL_FRAMES 48               // word 9;
#define ANIMAL_SPIN_UP SQUEAK_SPEED(16384)      // word 17;
#define ANIMAL_SPIN_LESS SQUEAK_SPEED(1229)     // word 18;
#define ANIMAL_SPIN_FALL SQUEAK_SPEED(16384)    // word 19;
#define ANIMAL_SPIN_START_FRAMES 18         // 38's;
#define ANIMAL_SPIN_FRAMES 48               // word 16;
#define ANIMAL_ROLL_END_FRAMES 18           // 37's;
#define ANIMAL_BOUNCE_BACK SQUEAK_SPEED(4916)   // word 13;
#define ANIMAL_BOUNCE_UP SQUEAK_SPEED(12288)    // word 14;
#define ANIMAL_DIG_SOLID 0x0D               // (KatAMExpansionSqueakRooms.cs' KatAMSolid: the dig cells' value till dug);
void RequestScreenShake(u16, struct ObjectBase *);

/* gSqueakDigRooms; The Squeak Squad rooms' dig cells (KatAMExpansionSqueakRooms.cs' SqueakDigCells()): per room from SQUEAK_FIRST_ROOM,
 * a list (NULL none): u16 how many, then a block record each (KatAM's, sub_08001408's: its cell, what it's drawn and left as dug); */
struct SqueakDigRooms {
    const u8 *const *list;
    u32 count;
};
const struct SqueakDigRooms gSqueakDigRooms = { NULL, 0 };

// DigCell(); A dig cell where (x, y) is (the room's, still undug: KatAM's solid there), and with `dig` dug (its record applied);
static bool32 DigCell(struct Kirby *kirby, s32 x, s32 y, bool32 dig) {
    u8 p = kirby->base.unk56, cx, cy;
    u16 room = gCurLevelInfo[p].currentRoom, n, i;
    const u8 *list, *r;

    if (room < SQUEAK_FIRST_ROOM || room - SQUEAK_FIRST_ROOM >= gSqueakDigRooms.count || x < 0 || y < 0) return FALSE;
    if ((list = gSqueakDigRooms.list[room - SQUEAK_FIRST_ROOM]) == NULL) return FALSE;
    cx = x >> 12;
    cy = y >> 12;
    if (GetCollisionTile(p, cx, cy) != ANIMAL_DIG_SOLID) return FALSE;
    n = list[0] | list[1] << 8;
    for (i = 0, r = list + 2; i < n; i++, r += 0x12) {
        if (r[2] != cx || r[3] != cy) continue;
        if (dig) sub_08001408(p, (struct Unk_0888562C *)r, NULL, NULL);
        return TRUE;
    }
    return FALSE;
}

// DigAt(); The dig cell at (ahead, up) pixels from his feet (the DS's offsets, Y up), dug with `dig`;
static bool32 DigAt(struct Kirby *kirby, s32 ahead, s32 up, bool32 dig) {
    return DigCell(kirby, kirby->base.x + Dir(kirby) * (ahead << 8), kirby->base.y + SCROLL_FEET - (up << 8), dig);
}

// NoLook(); His moves' piece drawing nothing: Kirby's own sprite (his motions);
static void NoLook(struct Kirby *kirby) {
    struct ScrollPiece *p = PieceOf(kirby);
    u8 i;

    if (p == NULL) return;
    p->shown = 0xFF;
    for (i = 0; i < SCROLL_LOOKS; i++) LookEnd(&p->look[i]);
    MoveStep(p);
}

/* The dig's way: AnimalDig's unkD9 (0 down, 1 ahead, 2 up); the dig cells cleared as it starts (with word 0, the side ones too); */
#define DIG_DOWN 0
#define DIG_AHEAD 1
#define DIG_UP 2
static void StartDig(struct Kirby *kirby, u8 way) {
    kirby->base.xspeed = kirby->base.yspeed = 0;
    kirby->unkD9 = way;
    if (way == DIG_AHEAD) {
        s32 feet = kirby->base.y + SCROLL_FEET;
        DigAt(kirby, 16, 24, TRUE);
        // (his feet on the nearest row);
        kirby->base.y = (((feet + 0x800) >> 12) << 12) - SCROLL_FEET;
        kirby->base.xspeed = Dir(kirby) * ANIMAL_DIG_SPEED;
    } else {
        DigAt(kirby, 16, 8, TRUE);
        DigAt(kirby, -16, 8, TRUE);
        // (centred on his cell);
        kirby->base.x = ((kirby->base.x >> 12) << 12) + 0x800;
        kirby->base.yspeed = way == DIG_UP ? ANIMAL_DIG_SPEED : -ANIMAL_DIG_SPEED;
    }
    PlaySfx(&kirby->base, SQUEAK_SE(0xE3));
    kirby->animationIndex = NIDL_MOTION_SLOT;
    StartState(kirby, AnimalDig);
    Look(kirby, way == DIG_DOWN ? ANIMAL_LOOK_DIG_DOWN : way == DIG_AHEAD ? ANIMAL_LOOK_DIG_SIDE : ANIMAL_LOOK_DIG_UP, TRUE);
}

static bool32 Dashing(struct Kirby *kirby) {
    return (kirby->base.flags & 0x10) != 0;
}

void AnimalB(struct Kirby *kirby) {
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    Facing(kirby);
    kirby->animationIndex = NIDL_MOTION_SLOT;
    if (kirby->base.unk62 & 4) {
        // (Down held over a dig cell: the cell under his feet dug, 4 pixels down);
        if (kirby->movementState & 0x80 && DigAt(kirby, 0, -4, TRUE)) {
            StartDig(kirby, DIG_DOWN);
            return;
        }
    } else if (!Dashing(kirby) && SqueakScrollOwned(SCROLL_ANIMAL)) {
        if (kirby->movementState & 0x80) {
            kirby->base.xspeed = 0;
            kirby->base.yspeed = ANIMAL_SPIN_UP;
            PlaySfx(&kirby->base, SQUEAK_SE(0x13E));
            StartState(kirby, AnimalSpin);
            kirby->stateFn(kirby);
            return;
        }
        if (kirby->movementState & 0x30) {
            kirby->base.xspeed = -Dir(kirby) * ANIMAL_ROLL_BACK;
            kirby->base.yspeed = 0;
            kirby->base.flags |= 0x20;
            Puff(kirby, FALSE);
            PlaySfx(&kirby->base, SQUEAK_SE(0x13E));
            StartState(kirby, AnimalRoll);
            kirby->stateFn(kirby);
            return;
        }
    }
    if (DigAt(kirby, 16, 8, TRUE)) {
        StartDig(kirby, DIG_AHEAD);
        return;
    }
    if (Dashing(kirby)) {
        PlaySfx(&kirby->base, SQUEAK_SE(0xE6));
        StartState(kirby, AnimalDash);
    } else {
        PlaySfx(&kirby->base, SQUEAK_SE(0xE4));
        StartState(kirby, AnimalScratch);
    }
    kirby->stateFn(kirby);
}

// The scratch (0x0208B754, 0x0208B6DC): 34, its box in frames 4-6;
static void AnimalScratch(struct Kirby *kirby) {
    s16 n = kirby->base.counter;

    if (NidlAbilityInWater(kirby)) return;
    if (n == 0) Look(kirby, ANIMAL_LOOK_SCRATCH, TRUE);
    if (kirby->base.unk62 & 4) kirby->base.xspeed = 0;
    MoveGravity(kirby);
    if (n >= 4 && n <= 6) MoveBox(kirby, 13, 34, 53, 0, ANIMAL_SCRATCH_STRENGTH);
    else MoveBox(kirby, 0, 0, 0, 0, 0);
    if (++kirby->base.counter >= ANIMAL_SCRATCH_FRAMES) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        NidlAbilityEnd(kirby);
    }
}

// The dash's crouch (0x0208B62C, 0x0208B4F4): 21, slowing; then the leap;
static void AnimalDash(struct Kirby *kirby) {
    s16 v = kirby->base.xspeed;

    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) Look(kirby, ANIMAL_LOOK_DASH, TRUE);
    v = v > 0 ? (v > ANIMAL_DASH_SLOW ? v - ANIMAL_DASH_SLOW : 0) : (v < -ANIMAL_DASH_SLOW ? v + ANIMAL_DASH_SLOW : 0);
    kirby->base.xspeed = v;
    MoveGravity(kirby);
    if (++kirby->base.counter < ANIMAL_DASH_FRAMES) return;
    kirby->base.xspeed = Dir(kirby) * ANIMAL_LEAP_X;
    kirby->base.yspeed = ANIMAL_LEAP_Y;
    kirby->base.flags |= 0x20;
    Puff(kirby, FALSE);
    StartState(kirby, AnimalLeap);
    Look(kirby, ANIMAL_LOOK_LEAP, TRUE);
}

// The leap (0x0208B45C): its grab box on an enemy he can hold, the flurry; landed (or word 1's frames), standing;
static void AnimalLeap(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (GrabTouchBox(kirby, -17, 27, 16, 4) != NULL) {
        kirby->base.xspeed = kirby->base.yspeed = 0;
        HoldAt(kirby, ANIMAL_HOLD_AHEAD, ANIMAL_HOLD_DOWN);
        StartState(kirby, AnimalFlurry);
        Look(kirby, ANIMAL_LOOK_FLURRY, TRUE);
        return;
    }
    kirby->base.yspeed -= ANIMAL_LEAP_FALL;
    if (kirby->base.unk62 & 1) kirby->base.xspeed = 0;
    else kirby->base.xspeed = Dir(kirby) * ANIMAL_LEAP_X;
    kirby->base.counter++;
    if ((kirby->base.unk62 & 4 && kirby->base.counter > 2) || kirby->base.counter >= ANIMAL_LEAP_MOST) NidlAbilityEnd(kirby);
}

// The flurry (0x0208B378, 0x0208B224): 24, held, for 67 frames, each loop's hit; then the throw;
static void AnimalFlurry(struct Kirby *kirby) {
    s16 n = kirby->base.counter++, f = n % ANIMAL_FLURRY_LOOP;
    struct Object *obj = HeldOf(kirby);

    kirby->base.xspeed = kirby->base.yspeed = 0;
    if (obj == NULL) {
        NidlAbilityEnd(kirby);
        return;
    }
    HoldAt(kirby, ANIMAL_HOLD_AHEAD, ANIMAL_HOLD_DOWN + (f == 17 || f == 18 || f == 21 || (f == 0 && n) ? 4 : 0));
    // (its hit on what he holds: its HP less, never its last: the throw's hit takes it, as KatAM's hit can't on what's held);
    if (f == ANIMAL_FLURRY_HIT) {
        PlaySfx(&kirby->base, SQUEAK_SE(0xE4));
        if (obj->unk80 > ANIMAL_HIT_STRENGTH) obj->unk80 -= ANIMAL_HIT_STRENGTH;
        else if (obj->unk80 > 1) obj->unk80 = 1;
    }
    if (n < ANIMAL_FLURRY_FRAMES) return;
    StartState(kirby, AnimalThrow);
    Look(kirby, ANIMAL_LOOK_THROW, TRUE);
}

// The throw (0x0208B19C): 25, its box; at its end what he holds let go with its hit (sound 0xE5);
static void AnimalThrow(struct Kirby *kirby) {
    s16 n = kirby->base.counter++;

    kirby->base.xspeed = kirby->base.yspeed = 0;
    HoldAt(kirby, ANIMAL_HOLD_AHEAD, ANIMAL_HOLD_DOWN);
    if (n < 4) MoveBox(kirby, -5, 37, 23, 2, ANIMAL_HIT_STRENGTH);
    else MoveBox(kirby, 0, 0, 0, 0, 0);
    if (n + 1 < ANIMAL_THROW_FRAMES) return;
    if (HeldOf(kirby) != NULL) {
        LetGo(kirby);
        PlaySfx(&kirby->base, SQUEAK_SE(0xE5));
        MoveBox(kirby, ANIMAL_HOLD_AHEAD - 12, 20, ANIMAL_HOLD_AHEAD + 12, -8, ANIMAL_HIT_STRENGTH);
    }
    StartState(kirby, AnimalRollEnd);
    kirby->unkD9 = 1;
}

// Digging (0x0208A988, 0x0208AC0C, 0x0208AD94): his way at 16 pixels in 20 frames, its box; then standing;
static void AnimalDig(struct Kirby *kirby) {
    if (kirby->unkD9 == DIG_AHEAD) {
        kirby->base.yspeed = 0;
        kirby->base.xspeed = Dir(kirby) * ANIMAL_DIG_SPEED;
        MoveBox(kirby, -2, 30, 27, 2, ANIMAL_DIG_STRENGTH);
    } else {
        kirby->base.xspeed = 0;
        kirby->base.yspeed = kirby->unkD9 == DIG_UP ? ANIMAL_DIG_SPEED : -ANIMAL_DIG_SPEED;
        if (kirby->unkD9 == DIG_UP) MoveBox(kirby, -15, 30, 15, 8, ANIMAL_DIG_STRENGTH);
        else MoveBox(kirby, -2, 30, 27, 2, ANIMAL_DIG_STRENGTH);
    }
    kirby->base.flags &= ~0x40;
    if (++kirby->base.counter <= ANIMAL_DIG_FRAMES) return;
    MoveBox(kirby, 0, 0, 0, 0, 0);
    kirby->base.xspeed = kirby->base.yspeed = 0;
    NidlAbilityEnd(kirby);
}

// Looking up (0x0208B014, 0x0208AFA0): 33 while Up is held; B, digging up;
static void AnimalLookUp(struct Kirby *kirby) {
    if (kirby->base.counter++ == 0) {
        kirby->base.x = ((kirby->base.x >> 12) << 12) + 0x800;
        Look(kirby, ANIMAL_LOOK_UP, TRUE);
    }
    kirby->base.xspeed = 0;
    MoveGravity(kirby);
    if (!(kirby->movementState & 0x40)) {
        NidlAbilityEnd(kirby);
        return;
    }
    if (kirby->unk11A & 2 && DigAt(kirby, 0, 17, TRUE)) StartDig(kirby, DIG_UP);
}

// The bounce (0x0208A10C's, 0x0208A59C's; 0x0208A024): back and up, KatAM's flip (his own motions), till falling;
static void Bounce(struct Kirby *kirby) {
    kirby->base.xspeed = -Dir(kirby) * ANIMAL_BOUNCE_BACK;
    kirby->base.yspeed = ANIMAL_BOUNCE_UP;
    kirby->base.flags |= 0x20;
    RequestScreenShake(1, &kirby->base);
    PlaySfx(&kirby->base, SQUEAK_SE(0x66));
    MoveBox(kirby, 0, 0, 0, 0, 0);
    kirby->animationIndex = KIRBY_JUMP_FLIP;
    StartState(kirby, AnimalBounce);
    NoLook(kirby);
}

static void AnimalBounce(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.counter++;
    NidlAbilityGravity(kirby);
    if (kirby->base.yspeed < 0) NidlAbilityEnd(kirby);
}

// The roll (0x0208A868, 0x0208A78C, 0x0208A59C): 35 then 36, level, back then ahead faster to word 12; into a wall the bounce;
static void AnimalRoll(struct Kirby *kirby) {
    s16 n = kirby->base.counter++;
    s16 v = kirby->base.xspeed + Dir(kirby) * ANIMAL_ROLL_ADD;

    if (NidlAbilityInWater(kirby)) return;
    if (n == 0) Look(kirby, ANIMAL_LOOK_ROLL_START, TRUE);
    if (n == ANIMAL_ROLL_START_FRAMES) Look(kirby, ANIMAL_LOOK_ROLL, TRUE);
    if (v > ANIMAL_ROLL_MOST) v = ANIMAL_ROLL_MOST;
    if (v < -ANIMAL_ROLL_MOST) v = -ANIMAL_ROLL_MOST;
    kirby->base.xspeed = v;
    kirby->base.yspeed = 0;
    if (n >= ANIMAL_ROLL_START_FRAMES && kirby->base.unk62 & 1 && kirby->base.xspeed * Dir(kirby) > 0) {
        Bounce(kirby);
        return;
    }
    // (.rca's 36: its last frame; 37: every 4 frames, -11 to 27 ahead, -3 to 21 up);
    if (n == ANIMAL_ROLL_START_FRAMES - 1 || (n >= ANIMAL_ROLL_START_FRAMES && (n - ANIMAL_ROLL_START_FRAMES) % 4 == 0))
        MoveBox(kirby, -11, 21, 27, -3, ANIMAL_HIT_STRENGTH);
    else MoveBox(kirby, 0, 0, 0, 0, 0);
    if (n >= ANIMAL_ROLL_START_FRAMES + ANIMAL_ROLL_FRAMES) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        StartState(kirby, AnimalRollEnd);
        kirby->unkD9 = 0;
    }
}

// The spin (0x0208A3A0, 0x0208A2D4, 0x0208A10C): 38 then 39, up and falling; onto the ground the bounce;
static void AnimalSpin(struct Kirby *kirby) {
    s16 n = kirby->base.counter++;

    if (NidlAbilityInWater(kirby)) return;
    if (n == 0) Look(kirby, ANIMAL_LOOK_SPIN_START, TRUE);
    if (n == ANIMAL_SPIN_START_FRAMES) Look(kirby, ANIMAL_LOOK_SPIN, TRUE);
    kirby->base.xspeed = 0;
    kirby->base.yspeed -= ANIMAL_SPIN_LESS;
    if (kirby->base.yspeed < -ANIMAL_SPIN_FALL) kirby->base.yspeed = -ANIMAL_SPIN_FALL;
    if (n > 2 && kirby->base.unk62 & 4) {
        Bounce(kirby);
        return;
    }
    // (.rca's 40: every 4 frames, -11 to 13 ahead, -18 to 21 up);
    if (n >= ANIMAL_SPIN_START_FRAMES && (n - ANIMAL_SPIN_START_FRAMES) % 4 == 0) MoveBox(kirby, -11, 21, 13, -18, ANIMAL_HIT_STRENGTH);
    else MoveBox(kirby, 0, 0, 0, 0, 0);
    if (n >= ANIMAL_SPIN_START_FRAMES + ANIMAL_SPIN_FRAMES) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        StartState(kirby, AnimalRollEnd);
        kirby->unkD9 = 0;
    }
}

/* The roll's and spin's end (0x0208A094, 0x0208A524: 37), and the throw's (unkD9 1: its last look, the let go enemy's hit kept on it
 * ANIMAL_THROW_HIT frames: KatAM's hits come a frame or more after the box), then standing; */
#define ANIMAL_THROW_HIT 4
static void AnimalRollEnd(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0 && kirby->unkD9 == 0) Look(kirby, ANIMAL_LOOK_ROLL_END, TRUE);
    MoveGravity(kirby);
    if (kirby->unkD9) MoveBox(kirby, ANIMAL_HOLD_AHEAD - 12, 20, ANIMAL_HOLD_AHEAD + 12, -8, ANIMAL_HIT_STRENGTH);
    if (++kirby->base.counter < (kirby->unkD9 ? ANIMAL_THROW_HIT : ANIMAL_ROLL_END_FRAMES)) return;
    MoveBox(kirby, 0, 0, 0, 0, 0);
    NidlAbilityEnd(kirby);
}

/* Bubble (the DS's Bubble Kirby, vtable 0x0216FDC0; PrmKirbyBubble.kgb; Bubble Head gives it, as the DS's):
 * • B (0x02089C64): moving (walking or dashing: the DS's pad +0x50) with the scroll (HasScroll 21), the big bubble; else blowing. In
 *   water, the water's own attack (KatAM's).
 * • Blowing (0x02089B90: bubble_ext's 22, 9 frames, efc_cmn0's 21 at his feet, sound 0x9C, with the scroll 0x9D; 0x02089AB0), then
 *   while B is held (0x02089700): composite 16 ahead, 14 with Up, 18 with Down (the pad read each frame, the look set again when it
 *   changes); a bubble every word 6 (9) frames from the spot of his way the frame before (words 0-5: 32 ahead and 10 up, 32 and 18 with
 *   Up, 32 and 2 with Down), kinds 0 and 1 by turns (word 12: 2); efc_cmn0's 19 at his feet every 10 frames; the sound again every 60.
 *   His box (bubble_ext.rca's: ahead 11 to 52 ahead, 3 to 19 up; down 11 to 47, -15 to 9; up none: logged on the DS, no attack of his
 *   while blowing up), 10. He stays where he is (the DS's vtable +0x38(1): on the ground its friction, 0.1 a frame; in the air his fall,
 *   KatAM's here; he doesn't turn). B let go: composite 17 (15 up, 19 down), 4 frames (0x02089680, the sounds stopped), then standing.
 * • A bubble (0x02089254, 0x0208906C; through everything: word 43 is 0): kind k's words 13 + 5k on: its look (bubble_ext's 35 on), its
 *   speed ahead and up, its change a frame ahead and up; with Up held word 7 (1.7) more up, with Down word 8 (1.7 less); a random
 *   change of up to 0.01 either way on each change (words 10, 11). Kind 0: 2.03 ahead, changing -0.01 and 0.01 a frame; kind 1: 0.4
 *   ahead, -0.1 up, changing 0.16 and -0.05; the scroll's kind 5: 1.7, 0.4, 0.12, 0.04 (logged on the DS: never capped). Its animation
 *   once through (17, 18, 36 frames), it pops (0x0208904C: 38, 11 frames, going on at its speed, no box); hitting something, it's gone
 *   at once (vtable +0x2C, 0x02089174). Its box (.rca's, from its middle; logged on the DS as its attack while it flies): kind 0 -11 to
 *   9 ahead, -12 to 11 up, kind 1 -15 to 9, -12 to 13, 20; kind 5 -15 to 14, -15 to 15, 40 (its hit's flag 0x800, not known).
 * • The scroll's big bubble (0x020895C4, 0x020894CC): composite 20 (29 frames; sound 0x150, efc_cmn0's 21 at his feet), then the kind
 *   5 bubble from the spot ahead (sound 0x151), composite 17 and standing. The scroll's only other change: the blowing sound (0x9D). */
#define SCROLL_BUBBLE 21
#define BUBBLE_START_FRAMES 9               // bubble_ext's 22;
#define BUBBLE_END_FRAMES 4                 // composites 15, 17, 19;
#define BUBBLE_BIG_FRAMES 29                // composite 20;
#define BUBBLE_EVERY 9                      // word 6;
#define BUBBLE_KINDS 2                      // word 12;
#define BUBBLE_DUST_EVERY 10
#define BUBBLE_SOUND_EVERY 60
#define BUBBLE_FRICTION SQUEAK_SPEED(410)   // (the DS's Kirby's ground friction: 0.1);
#define BUBBLE_UP 6964                      // words 7, 8 (20.12);
#define BUBBLE_JITTER 41                    // words 10, 11 (20.12);
#define BUBBLE_SPRAY_STRENGTH 2             // .rca's 10;
#define BUBBLE_POP_FRAMES 11                // bubble_ext's 38;
#define BUBBLE_KIND_BIG 2                   // (gBubbleKinds' 2: the DS's kind 5);
/* Their hit: KatAM's kind 0x80 (Magic's and Sleep's, level 3), the one an enemy's hurt state shuts out each frame and keeps out once it's
 * beaten (object.c's sub_0809B6A8: unk5C |= 0x80): with a kind it doesn't shut out (Cutter's), the spray's box every frame and a bubble
 * every 9 hit a beaten enemy again before its 11 hurt frames end, holding it there till Kirby stops (logged: HP 4 to -36); the DS's
 * hit-stop keeps them off it, and a beaten one is gone; */
#define BUBBLE_HIT 0x20000083

enum { BUBBLE_AHEAD, BUBBLE_WAY_UP, BUBBLE_WAY_DOWN };

// The bubbles: their look and frames, their speed and its change a frame (20.12: ahead, up), their box (from their middle, Y up), strength;
struct BubbleKind {
    u8 look, frames;
    s16 speedX, speedY, addX, addY;
    s8 box[4];
    u8 strength;
};
static const struct BubbleKind gBubbleKinds[] = {
    { BUBBLE_LOOK_BUBBLE, 17, 8315, 0, -41, 41, { -11, 11, 9, -12 }, 4 },              // kind 0 (words 13-17);
    { BUBBLE_LOOK_BUBBLE_2, 18, 1639, -410, 656, -205, { -15, 13, 9, -12 }, 4 },       // kind 1 (words 18-22);
    { BUBBLE_LOOK_BIG_BUBBLE, 36, 6964, 1639, 492, 164, { -15, 15, 14, -15 }, 8 },     // kind 5 (words 38-42);
};
// (words 0-5: the spot a bubble starts from, ahead and up from his feet, by his way);
static const s8 gBubbleSpots[3][2] = { { 32, 10 }, { 32, 18 }, { 32, 2 } };

#define BubbleSound() (SqueakScrollOwned(SCROLL_BUBBLE) ? 0x9D : 0x9C)

// BubbleQuiet(); The blowing's sounds stopped (the DS's 0x02013D88);
static void BubbleQuiet(void) {
    m4aSongNumStop(SQUEAK_SE(0x9C));
    m4aSongNumStop(SQUEAK_SE(0x9D));
}

// BubbleJitter(); The random change (the DS's: up to word 10's either way);
static s32 BubbleJitter(void) {
    return (s32)(Rand16() % (2 * BUBBLE_JITTER)) - BUBBLE_JITTER;
}

// A bubble's piece: vars[0] its kind, vars[1] its frames left, vars[2] and vars[3] its speed's change a frame (KatAM's, across and up);
static bool32 BubblePop(struct ScrollPiece *p) {
    if (p->vars[1] == 0) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    p->vars[1]--;
    p->base.x += p->base.xspeed;
    p->base.y -= p->base.yspeed;
    return TRUE;
}

static bool32 BubbleStep(struct ScrollPiece *p) {
    const struct BubbleKind *k = &gBubbleKinds[p->vars[0]];

    // (what it hits, it's gone);
    if (p->base.flags & 0x40000) {
        p->base.flags |= 0x1000;
        return FALSE;
    }
    if (p->vars[1] == 0) {
        p->step = BubblePop;
        p->vars[1] = BUBBLE_POP_FRAMES;
        Box(p, 0, 0, 0, 0, 0);
        SetLook(p, BUBBLE_LOOK_POP, TRUE);
        return BubblePop(p);
    }
    p->vars[1]--;
    p->base.xspeed += p->vars[2];
    p->base.yspeed += p->vars[3];
    p->base.x += p->base.xspeed;
    p->base.y -= p->base.yspeed;
    p->base.unk68 = BUBBLE_HIT;
    Box(p, k->box[0], k->box[1], k->box[2], k->box[3], k->strength);
    return TRUE;
}

// Bubble(); One blown (the DS's 0x02089254): kind `kind` from the spot of `way`, Up and Down held changing its speed up;
static void Bubble(struct Kirby *kirby, u8 kind, u8 way) {
    const struct BubbleKind *k = &gBubbleKinds[kind];
    struct ScrollPiece *p = ScrollPieceCreate(kirby, SCROLL_MOVE, 0, NINJA_KNOCK, BUBBLE_HIT);
    s32 dir = Dir(kirby), up = k->speedY;

    if (p == NULL) return;
    if (kirby->movementState & 0x40) up += BUBBLE_UP;
    else if (kirby->movementState & 0x80) up -= BUBBLE_UP;
    p->step = BubbleStep;
    p->vars[0] = kind;
    p->vars[1] = k->frames;
    p->vars[2] = dir * SQUEAK_SPEED(k->addX + BubbleJitter());
    p->vars[3] = SQUEAK_SPEED(k->addY + BubbleJitter());
    p->base.x = kirby->base.x + dir * (gBubbleSpots[way][0] << 8);
    p->base.y = kirby->base.y + SCROLL_FEET - (gBubbleSpots[way][1] << 8);
    p->base.xspeed = dir * SQUEAK_SPEED(k->speedX);
    p->base.yspeed = SQUEAK_SPEED(up);
    p->base.flags = (p->base.flags & ~1) | (dir < 0 ? 1 : 0);
    SetLook(p, k->look, TRUE);
    BubbleStep(p);
}

// BubbleStand(); Where he is (the DS's vtable +0x38(1)): on the ground slowing by its friction, in the air falling;
static void BubbleStand(struct Kirby *kirby) {
    s16 v = kirby->base.xspeed;

    if (kirby->base.unk62 & 4)
        kirby->base.xspeed = v > 0 ? (v > BUBBLE_FRICTION ? v - BUBBLE_FRICTION : 0) : (v < -BUBBLE_FRICTION ? v + BUBBLE_FRICTION : 0);
    MoveGravity(kirby);
}

/* BubbleB(); The B button (gNidlAbilityTables' landB; the DS's 0x02089C64): moving with the scroll, the big bubble; else blowing; */
void BubbleB(struct Kirby *kirby) {
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    Facing(kirby);
    kirby->animationIndex = NIDL_MOTION_SLOT;
    BubbleQuiet();
    if ((kirby->base.flags & 0x10 || kirby->movementState & 0x30) && SqueakScrollOwned(SCROLL_BUBBLE)) {
        PlaySfx(&kirby->base, SQUEAK_SE(0x150));
        StartState(kirby, BubbleBig);
    } else {
        PlaySfx(&kirby->base, SQUEAK_SE(BubbleSound()));
        StartState(kirby, BubbleStart);
    }
    Puff(kirby, FALSE);
    kirby->stateFn(kirby);
}

// The start (0x02089AB0): 22, then blowing ahead (its first frame the same frame, as the DS's state returning 1);
static void BubbleStart(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter >= BUBBLE_START_FRAMES) {
        kirby->unkD9 = BUBBLE_AHEAD;
        StartState(kirby, BubbleBlow);
        Look(kirby, BUBBLE_LOOK_BLOW, TRUE);
        BubbleBlow(kirby);
        return;
    }
    if (kirby->base.counter == 0) Look(kirby, BUBBLE_LOOK_START, TRUE);
    BubbleStand(kirby);
    kirby->base.counter++;
}

// Blowing (0x02089700): a bubble every 9 frames, his way from the pad, the dust, the sound again, his box; B let go, the end;
static void BubbleBlow(struct Kirby *kirby) {
    s16 n = kirby->base.counter;
    u8 way;

    if (NidlAbilityInWater(kirby)) return;
    if (n % BUBBLE_EVERY == 0) Bubble(kirby, (n / BUBBLE_EVERY) % BUBBLE_KINDS, kirby->unkD9);
    BubbleStand(kirby);
    way = kirby->movementState & 0x40 ? BUBBLE_WAY_UP : kirby->movementState & 0x80 ? BUBBLE_WAY_DOWN : BUBBLE_AHEAD;
    if (way != kirby->unkD9) {
        kirby->unkD9 = way;
        Look(kirby, way == BUBBLE_WAY_UP ? BUBBLE_LOOK_BLOW_UP : way == BUBBLE_WAY_DOWN ? BUBBLE_LOOK_BLOW_DOWN : BUBBLE_LOOK_BLOW, TRUE);
    }
    if (n % BUBBLE_DUST_EVERY == 0) SqueakEffectInRoom(kirby->base.roomId, SQUEAK_FX_DUST, kirby->base.x, kirby->base.y + SCROLL_FEET);
    if (n % BUBBLE_SOUND_EVERY == BUBBLE_SOUND_EVERY - 1) {
        BubbleQuiet();
        PlaySfx(&kirby->base, SQUEAK_SE(BubbleSound()));
    }
    if (way == BUBBLE_AHEAD) MoveBox(kirby, 11, 19, 52, 3, BUBBLE_SPRAY_STRENGTH);
    else if (way == BUBBLE_WAY_DOWN) MoveBox(kirby, 11, 9, 47, -15, BUBBLE_SPRAY_STRENGTH);
    else MoveBox(kirby, 0, 0, 0, 0, 0);
    if (FindPiece(kirby) != NULL) FindPiece(kirby)->base.unk68 = BUBBLE_HIT;
    kirby->base.counter++;
    if (kirby->movementState & 2) return;
    MoveBox(kirby, 0, 0, 0, 0, 0);
    StartState(kirby, BubbleEnd);
    Look(kirby, way == BUBBLE_WAY_UP ? BUBBLE_LOOK_END_UP : way == BUBBLE_WAY_DOWN ? BUBBLE_LOOK_END_DOWN : BUBBLE_LOOK_END, TRUE);
}

// The end (0x02089680): the sounds stopped, 17 (15, 19), then standing;
static void BubbleEnd(struct Kirby *kirby) {
    BubbleQuiet();
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter >= BUBBLE_END_FRAMES) {
        NidlAbilityEnd(kirby);
        return;
    }
    BubbleStand(kirby);
    kirby->base.counter++;
}

// The scroll's big bubble (0x020894CC): 20, then the kind 5 bubble (sound 0x151) and the end;
static void BubbleBig(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter >= BUBBLE_BIG_FRAMES) {
        Bubble(kirby, BUBBLE_KIND_BIG, BUBBLE_AHEAD);
        PlaySfx(&kirby->base, SQUEAK_SE(0x151));
        StartState(kirby, BubbleEnd);
        Look(kirby, BUBBLE_LOOK_END, TRUE);
        BubbleEnd(kirby);
        return;
    }
    if (kirby->base.counter == 0) Look(kirby, BUBBLE_LOOK_BIG, TRUE);
    BubbleStand(kirby);
    kirby->base.counter++;
}

/* Metal (the DS's Metal Kirby: its class's constructor 0x020CB160 loads kirby/metal_cmn, metal_ext and metal_wat, PrmKirbyMetal.kgb and
 * PrmKirbyMetalKirbyBase.kgb, his movement's words in place of Kirby's own, prmKirbyBase.kgb; Metalun gives it, as the DS's):
 * • Heavy: the movement words Metal's file changes from Kirby's (logged the same in the DS's emulator), Metal takes here, KatAM's own
 *   for the rest (KatAM's rules: when he jumps, floats, lands, swims). Walking 0.03 faster a frame up to 0.5 (word 2, word 3), 0.15
 *   slower a frame with the pad let go (word 1); no dash (the DS's Metal Kirby has none: KatAM's dash, its double tap, starts a walk);
 *   the jump 2.68 up (word 6), the same with A held or not (its cut, word 7, 2.5, is under it), its gravity 0.17 (word 8, Kirby's own
 *   too on the DS) and falling at most 4.5 (word 9) through KatAM's jump's top and fall (MetalRise, MetalFall); in the air 0.05 faster
 *   a frame up to 1.23 (words 10, 11); floating 0.03 faster a frame up to 0.35 sideways (13, 14), flapping 0.27 a frame up to 0.3 (16,
 *   17), falling at most 2.03 (19). His steps shake the screen and thud (0x020CAFFC: sound 0xAC, the quake (3, 5, 2), at the walk
 *   animation's frames 16 and 46 of each 60: its steps, every 30 frames).
 * • Hard: the DS's hurt boxes of his (metal_cmn.rca's 0x102 records: 0x034B, Kirby's own 0x000A) shut out the attack kinds 0 (touch),
 *   6 (grabs), 8 and 9 besides Kirby's own: here his unk5C shuts out KatAM's enemies' own kind (0x80: an enemy's body and the attacks
 *   that keep InitObject's 0x82), so touching them doesn't hurt him; their other kinds (fire 0x100...) and the stage's spikes do. His
 *   body doesn't hurt them either (logged on the DS: a Waddle Dee pressed against him, neither hurt; KatAM's touch hurts both: his
 *   unk68's 0x80 off). The currents don't move him (the DS's flag 0x800000: squeak/gimmicks.c's Currents).
 * • B (0x020CAF44): in the air with Down held, the stomp; else with the scroll (HasScroll 22), the roll; else nothing (no inhale: the
 *   DS's 0x02017DE4). In water, the water's own attack (KatAM's).
 * • The stomp (0x020CADD0, 0x020CAC54, 0x020CAA64): stopped where he is, metal_ext's 26 (24 frames; sound 0x16D), then straight down
 *   at word 14 (6.0): 27, his body's box (metal_ext.rca's 28: -9 to 9 ahead, 0 to 19 up, 20); on the ground the quake (4, 10, 2),
 *   efc_cmn0's 22 at his feet, sound 0xAD, and he bounces off it (28, 16 frames: the movement's word 3 ahead, 0.5, half its jump up,
 *   1.34, its gravity: the DS's jump, state 0x7C, the pad as in the air);
 * • The scroll's roll (0x020CA9B8, 0x020CA838, 0x020CA4E0, 0x020CA3F8): 23 (12 frames; sound 0xAB) at the speed he had, then
 *   efc_cmn0's 18 at his feet and the quake (3, 5, 2); rolling (no pad, no jump): on flat ground slowing by word 2 (0.04), 24 over word
 *   12 (0.5) else 34; down a slope faster by word 8 (0.16; steep, word 10: 0.22) up to words 9, 11 (3.0), up one slower by word 4
 *   (0.09; steep, word 6: 0.14), turned back once he rolls back (24 on slopes); in the air 35, the movement's air words (at most 1.23,
 *   slowing by 0.01, its gravity); landing, the quake (3, 10, 2) and sound 0xAC; into a wall, stopped, the quake (6, 10, 2) and the
 *   sound; his body's box (.rca's 25 and 36) while rolling fast and in the air; B, sound 0xAC, 25 (4 frames), standing;
 * The quakes: KatAM's shake (RequestScreenShake 1, 14 frames, its shortest, for the DS's 5 and 10 frames; 2, 18 frames, for its
 * strength 4 and 6). In water he swims as KatAM's Kirby (the DS's sinks: its words 40-51, metal_wat's frames, not ported). */
#define SCROLL_METAL 22
#define METAL_HIT 0x30005003                    // KatAM's Stone's (gUnk_082DDFC8): the stomp's and the roll's box;
#define METAL_STRENGTH 4                        // .rca's 25, 28, 36: 20;
#define METAL_GUARD 0x80                        // KatAM's enemies' own hit kind (InitObject's unk68 0x82), shut out by his unk5C;
#define METAL_FRICTION SQUEAK_SPEED(0x267)      // PrmKirbyMetalKirbyBase.kgb's word 1;
#define METAL_WALK_ADD SQUEAK_SPEED(0x7B)       // word 2;
#define METAL_WALK_MOST SQUEAK_SPEED(0x800)     // word 3;
#define METAL_JUMP SQUEAK_SPEED(0x2AE2)         // word 6;
#define METAL_GRAVITY SQUEAK_SPEED(0x2B9)       // word 8;
#define METAL_FALL_MOST SQUEAK_SPEED(0x4800)    // word 9;
#define METAL_AIR_ADD SQUEAK_SPEED(0xCD)        // word 10;
#define METAL_AIR_MOST SQUEAK_SPEED(0x13AF)     // word 11;
#define METAL_AIR_SLOW SQUEAK_SPEED(0x29)       // word 12;
#define METAL_FLOAT_ADD SQUEAK_SPEED(0x7B)      // word 13;
#define METAL_FLOAT_MOST SQUEAK_SPEED(0x59A)    // word 14;
#define METAL_FLAP_ADD SQUEAK_SPEED(0x452)      // word 16;
#define METAL_FLAP_MOST SQUEAK_SPEED(0x4CD)     // word 17;
#define METAL_SINK_MOST SQUEAK_SPEED(0x207B)    // word 19;
#define KATAM_GRAVITY 38                        // KatAM's jump's and fall's (sub_080435F8's macro), and its float's fall;
#define KATAM_FALL_MOST 0x280
#define KATAM_SINK_ADD 0x16
#define METAL_STEP_FIRST 16
#define METAL_STEP_EVERY 30
#define METAL_STOMP_START_FRAMES 24             // metal_ext's 26;
#define METAL_STOMP_FALL SQUEAK_SPEED(0x6000)   // PrmKirbyMetal.kgb's word 14;
#define METAL_BOUNCE_FRAMES 16                  // 28's;
#define METAL_ROLL_START_FRAMES 12              // 23's;
#define METAL_ROLL_END_FRAMES 4                 // 25's;
#define METAL_ROLL_FLAT SQUEAK_SPEED(164)       // word 2 (its -0.04);
#define METAL_ROLL_UP SQUEAK_SPEED(369)         // word 4 (-0.09);
#define METAL_ROLL_UP_STEEP SQUEAK_SPEED(574)   // word 6 (-0.14);
#define METAL_ROLL_DOWN SQUEAK_SPEED(656)       // word 8;
#define METAL_ROLL_DOWN_STEEP SQUEAK_SPEED(902) // word 10;
#define METAL_ROLL_MOST SQUEAK_SPEED(12288)     // words 9, 11;
#define METAL_ROLL_FAST SQUEAK_SPEED(2048)      // word 12;
void sub_0804464C(struct Kirby *);
void sub_08044FD4(struct Kirby *);
void sub_080411E8(struct Kirby *);
void sub_080424BC(struct Kirby *);

// MetalGravity(); Falling in his moves (the movement's gravity, at most its fall);
static void MetalGravity(struct Kirby *kirby) {
    if (kirby->base.unk62 & 4) return;
    kirby->base.yspeed -= METAL_GRAVITY;
    if (kirby->base.yspeed < -METAL_FALL_MOST) kirby->base.yspeed = -METAL_FALL_MOST;
}

// MetalBody(); His body's box (.rca's 25, 28, 36), or none;
static void MetalBody(struct Kirby *kirby, bool32 on) {
    struct ScrollPiece *p;

    if (!on) {
        MoveBox(kirby, 0, 0, 0, 0, 0);
        return;
    }
    MoveBox(kirby, -9, 19, 9, 0, METAL_STRENGTH);
    if ((p = FindPiece(kirby)) != NULL) p->base.unk68 = METAL_HIT;
}

// MetalSlow(); Slower by `by` a frame, to standing still;
static s16 MetalSlow(s16 v, s16 by) {
    return v > by ? v - by : v < -by ? v + by : 0;
}

// MetalSteer(); The pad in the air (KatAM's sub_0805B3A0 with the movement's air words; let go, KatAM's own slowing, or in his own
// moves, `own`, the movement's: word 12);
static void MetalSteer(struct Kirby *kirby, bool32 own) {
    s16 v = kirby->base.xspeed;

    if (kirby->movementState & 0x20) {
        if (!(kirby->base.flags & 1) && kirby->animationIndex == 22) kirby->animationIndex = 23;
        kirby->base.flags |= 1;
        v -= METAL_AIR_ADD;
        if (v < -METAL_AIR_MOST) v = -METAL_AIR_MOST;
    } else if (kirby->movementState & 0x10) {
        if (kirby->base.flags & 1 && kirby->animationIndex == 22) kirby->animationIndex = 23;
        kirby->base.flags &= ~1;
        v += METAL_AIR_ADD;
        if (v > METAL_AIR_MOST) v = METAL_AIR_MOST;
    } else if (own) {
        v = MetalSlow(v, METAL_AIR_SLOW);
    } else {
        sub_0805B1B8(kirby);
        return;
    }
    kirby->base.xspeed = v;
}

/* MetalWalk(), MetalAirMove(), MetalFloatMove(), MetalJump(), MetalDash() (KatAM's sub_0805B2FC, sub_0805B3A0, sub_0805B450, sub_08043360,
 * sub_08041B08, detoured: his walk, his steering in the air and floating, his jump's start, his dash's start); KatAM's own for the others;
 */
void (*gMetalWalkOriginal)(struct Kirby *) = NULL;
void (*gMetalAirOriginal)(struct Kirby *) = NULL;
void (*gMetalFloatOriginal)(struct Kirby *) = NULL;
void (*gMetalJumpOriginal)(struct Kirby *) = NULL;
void (*gMetalDashOriginal)(struct Kirby *) = NULL;
void (*gMetalFrameOriginal)(struct Kirby *) = NULL;
static void MetalRise(struct Kirby *kirby);

void MetalWalk(struct Kirby *kirby) {
    s16 v = kirby->base.xspeed;

    if (kirby->ability != SQUEAK_ABILITY_METAL) {
        gMetalWalkOriginal(kirby);
        return;
    }
    if (kirby->movementState & 0x20) {
        if (!(kirby->base.flags & 1)) {
            sub_080424BC(kirby);
            return;
        }
        v -= METAL_WALK_ADD;
        if (v < -METAL_WALK_MOST) v = -METAL_WALK_MOST;
    } else if (kirby->movementState & 0x10) {
        if (kirby->base.flags & 1) {
            sub_080424BC(kirby);
            return;
        }
        v += METAL_WALK_ADD;
        if (v > METAL_WALK_MOST) v = METAL_WALK_MOST;
    } else if (kirby->base.unk58 & 0x10 || kirby->base.flags & 0x60) {
        // (on ice, and in KatAM's own slides, KatAM's slowing);
        sub_0805B1B8(kirby);
        return;
    } else {
        v = MetalSlow(v, METAL_FRICTION);
    }
    kirby->base.xspeed = v;
}

void MetalAirMove(struct Kirby *kirby) {
    if (kirby->ability != SQUEAK_ABILITY_METAL) gMetalAirOriginal(kirby);
    else MetalSteer(kirby, FALSE);
}

void MetalFloatMove(struct Kirby *kirby) {
    s16 v = kirby->base.xspeed;

    if (kirby->ability != SQUEAK_ABILITY_METAL) {
        gMetalFloatOriginal(kirby);
        return;
    }
    if (kirby->movementState & 0x20) {
        kirby->base.flags |= 1;
        v -= METAL_FLOAT_ADD;
        if (v < -METAL_FLOAT_MOST) v = -METAL_FLOAT_MOST;
        kirby->base.xspeed = v;
    } else if (kirby->movementState & 0x10) {
        kirby->base.flags &= ~1;
        v += METAL_FLOAT_ADD;
        if (v > METAL_FLOAT_MOST) v = METAL_FLOAT_MOST;
        kirby->base.xspeed = v;
    } else {
        sub_0805B1B8(kirby);
    }
    if (kirby->movementState & 0x41) {
        kirby->base.yspeed += METAL_FLAP_ADD;
        if (kirby->base.yspeed > METAL_FLAP_MOST) kirby->base.yspeed = METAL_FLAP_MOST;
    } else {
        kirby->base.yspeed -= KATAM_SINK_ADD;
        if (kirby->base.yspeed < -METAL_SINK_MOST) kirby->base.yspeed = -METAL_SINK_MOST;
    }
}

void MetalJump(struct Kirby *kirby) {
    gMetalJumpOriginal(kirby);
    // (in water, KatAM's own jump out of it: he swims as KatAM's Kirby);
    if (kirby->ability != SQUEAK_ABILITY_METAL || kirby->base.unk58 & 2) return;
    kirby->base.yspeed = METAL_JUMP;
    kirby->stateFn = MetalRise;
}

void MetalDash(struct Kirby *kirby) {
    if (kirby->ability == SQUEAK_ABILITY_METAL) sub_080411E8(kirby);
    else gMetalDashOriginal(kirby);
}

/* MetalRise(), MetalFall(); KatAM's jump's top and its fall (sub_0804464C, sub_08044FD4: its rules, the pad, doors, floating, B,
 * landing), his speed down the movement's (where KatAM's plain gravity was all that changed it: not a ceiling's, a landing's); */
static void MetalFall(struct Kirby *kirby);
static void MetalAirState(struct Kirby *kirby, void (*katam)(struct Kirby *)) {
    void (*self)(struct Kirby *) = kirby->stateFn;
    s16 v = kirby->base.yspeed, plain = v - KATAM_GRAVITY < -KATAM_FALL_MOST ? -KATAM_FALL_MOST : v - KATAM_GRAVITY;

    katam(kirby);
    // (still in it: KatAM's state doesn't set itself again; into the other one: ours of it);
    if (kirby->stateFn == sub_0804464C) kirby->stateFn = MetalRise;
    else if (kirby->stateFn == sub_08044FD4) kirby->stateFn = MetalFall;
    else if (kirby->stateFn != self) return;
    if (kirby->base.unk62 & 4 || kirby->base.yspeed != plain) return;
    v -= METAL_GRAVITY;
    kirby->base.yspeed = v < -METAL_FALL_MOST ? -METAL_FALL_MOST : v;
}

static void MetalRise(struct Kirby *kirby) {
    MetalAirState(kirby, sub_0804464C);
}

static void MetalFall(struct Kirby *kirby) {
    MetalAirState(kirby, sub_08044FD4);
}

/* MetalKirbyFrame(); KatAM's Kirby's sprite update (sub_0803F790, detoured: each frame, his state run, before he moves): Metal's guard
 * and his body harmless (his unk68's enemy kind off: KatAM's touch hurts both), his fall (KatAM's, from a ledge or a float's end, is
 * Metal's), his steps; both back as they were once he's lost Metal; */
void MetalKirbyFrame(struct Kirby *kirby) {
    u8 *step = &NIDL_MIKE_USES(kirby);

    if (kirby->ability != SQUEAK_ABILITY_METAL) {
        // (Metal gone: his guard, and his body's touch back, KatAM's 0x82; the guard KatAM never sets on a Kirby marks it);
        if (kirby->base.unk5C & METAL_GUARD) {
            kirby->base.unk5C &= ~METAL_GUARD;
            kirby->base.unk68 |= METAL_GUARD;
        }
        gMetalFrameOriginal(kirby);
        return;
    }
    // (neither hurts the other by touch, as the DS's: logged, a Waddle Dee pressed against him, both unhurt);
    kirby->base.unk5C |= METAL_GUARD;
    kirby->base.unk68 &= ~METAL_GUARD;
    // (into water, KatAM's fall at most: he swims as KatAM's Kirby, its slowing down from 2.5, not his 4.5 into a pit under it);
    if (kirby->base.unk58 & 2 && kirby->base.yspeed < -KATAM_FALL_MOST) kirby->base.yspeed = -KATAM_FALL_MOST;
    if (kirby->stateFn == sub_08044FD4) kirby->stateFn = MetalFall;
    else if (kirby->stateFn == sub_0804464C) kirby->stateFn = MetalRise;
    // (his steps: walking, KatAM's walk's animation, 11; the byte his ability's own, Mike's uses);
    if (!(kirby->base.unk62 & 4) || kirby->animationIndex != 11 || kirby->base.xspeed == 0 || kirby->base.unk58 & 2) {
        *step = 0;
    } else {
        if (++*step >= METAL_STEP_FIRST + METAL_STEP_EVERY) *step = METAL_STEP_FIRST;
        if (*step == METAL_STEP_FIRST) {
            RequestScreenShake(1, &kirby->base);
            PlaySfx(&kirby->base, SQUEAK_SE(0xAC));
        }
    }
    gMetalFrameOriginal(kirby);
}

static void MetalStompStart(struct Kirby *kirby);
static void MetalStomp(struct Kirby *kirby);
static void MetalBounce(struct Kirby *kirby);
static void MetalRollStart(struct Kirby *kirby);
static void MetalRoll(struct Kirby *kirby);
static void MetalRollEnd(struct Kirby *kirby);

/* MetalB(); The B button (gNidlAbilityTables' landB; the DS's 0x020CAF44): in the air with Down held, the stomp; else with the scroll,
 * the roll; else nothing; */
void MetalB(struct Kirby *kirby) {
    bool32 stomp = !(kirby->base.unk62 & 4) && kirby->movementState & 0x80;

    if (!stomp && !SqueakScrollOwned(SCROLL_METAL)) return;
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    Facing(kirby);
    kirby->animationIndex = NIDL_MOTION_SLOT;
    if (stomp) {
        PlaySfx(&kirby->base, SQUEAK_SE(0x16D));
        StartState(kirby, MetalStompStart);
    } else {
        PlaySfx(&kirby->base, SQUEAK_SE(0xAB));
        StartState(kirby, MetalRollStart);
    }
    kirby->stateFn(kirby);
}

// The stomp's start (0x020CADD0, 0x020CAC54): stopped, 26, then straight down;
static void MetalStompStart(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) Look(kirby, METAL_LOOK_STOMP_START, TRUE);
    kirby->base.xspeed = kirby->base.yspeed = 0;
    if (++kirby->base.counter < METAL_STOMP_START_FRAMES) return;
    kirby->base.yspeed = -METAL_STOMP_FALL;
    StartState(kirby, MetalStomp);
    Look(kirby, METAL_LOOK_STOMP, TRUE);
    MetalBody(kirby, TRUE);
}

// The stomp (0x020CAA64): 27, down at word 14 till the ground: the quake, the puff, the sound, the bounce;
static void MetalStomp(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.counter++;
    kirby->base.xspeed = 0;
    if (!(kirby->base.unk62 & 4)) {
        kirby->base.yspeed = -METAL_STOMP_FALL;
        MetalBody(kirby, TRUE);
        return;
    }
    MetalBody(kirby, FALSE);
    RequestScreenShake(2, &kirby->base);
    Puff(kirby, TRUE);
    PlaySfx(&kirby->base, SQUEAK_SE(0xAD));
    kirby->base.xspeed = Dir(kirby) * METAL_WALK_MOST;
    kirby->base.yspeed = METAL_JUMP / 2;
    kirby->base.flags |= 0x20;
    StartState(kirby, MetalBounce);
    Look(kirby, METAL_LOOK_BOUNCE, TRUE);
}

// The bounce (the DS's jump, 0x7C, 28 shown): its gravity, the pad as in the air; 28 played, KatAM's fall (or standing);
static void MetalBounce(struct Kirby *kirby) {
    s16 n = kirby->base.counter++;

    if (NidlAbilityInWater(kirby)) return;
    if (n > 0 && kirby->base.unk62 & 4 && kirby->base.yspeed <= 0) {
        kirby->base.flags &= ~0x20;
        NidlAbilityEnd(kirby);
        return;
    }
    MetalSteer(kirby, TRUE);
    MetalGravity(kirby);
    if (n + 1 >= METAL_BOUNCE_FRAMES) NidlAbilityEnd(kirby);
}

// The roll's start (0x020CA9B8, 0x020CA838): 23 at the speed he had, then efc_cmn0's 18 at his feet and the quake;
static void MetalRollStart(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.counter == 0) Look(kirby, METAL_LOOK_ROLL_START, TRUE);
    MetalGravity(kirby);
    if (++kirby->base.counter < METAL_ROLL_START_FRAMES) return;
    SqueakEffectInRoom(kirby->base.roomId, SQUEAK_FX_KIRBY_DUST, kirby->base.x, kirby->base.y + SCROLL_FEET);
    RequestScreenShake(1, &kirby->base);
    StartState(kirby, MetalRoll);
    Look(kirby, METAL_LOOK_ROLL_SLOW, TRUE);
}

/* MetalSlope(); The ground under him the way he rolls (KatAM's slope tiles, its collision attribute's top bits: 1-3 rising to the right,
 * 4-6 falling, 3 and 4 the steep ones, as Roly-Poly's roll reads them, roly_poly.c): 2 steeply down, 1 down, 0 flat, -1 up, -2 steeply up;
 */
static s32 MetalSlope(struct Kirby *kirby) {
    u32 s = gCollisionAttributes[kirby->base.unk57] >> 28;
    s32 way;

    if (s == 0 || s > 6) return 0;
    way = (s <= 3) == (Dir(kirby) < 0) ? 1 : -1;
    return s == 3 || s == 4 ? 2 * way : way;
}

// The roll (0x020CA4E0): its speed ahead by the ground under him (see above), the air's, a landing's and a wall's; B, its end;
static void MetalRoll(struct Kirby *kirby) {
    s16 s = kirby->base.xspeed * Dir(kirby);
    s32 slope = 0;
    u8 look;

    if (NidlAbilityInWater(kirby)) return;
    kirby->base.counter++;
    if (!(kirby->base.unk62 & 4)) {
        s = MetalSlow(s, METAL_AIR_SLOW);
        if (s > METAL_AIR_MOST) s = METAL_AIR_MOST;
        if (s < -METAL_AIR_MOST) s = -METAL_AIR_MOST;
        MetalGravity(kirby);
        look = METAL_LOOK_ROLL_AIR;
    } else {
        if (kirby->base.yspeed < 0) {
            kirby->base.yspeed = 0;
            RequestScreenShake(1, &kirby->base);
            PlaySfx(&kirby->base, SQUEAK_SE(0xAC));
        }
        if (kirby->base.unk62 & 1 && s > 0) {
            s = 0;
            RequestScreenShake(2, &kirby->base);
            PlaySfx(&kirby->base, SQUEAK_SE(0xAC));
        }
        slope = MetalSlope(kirby);
        if (slope > 0) {
            s += slope > 1 ? METAL_ROLL_DOWN_STEEP : METAL_ROLL_DOWN;
            if (s > METAL_ROLL_MOST) s = METAL_ROLL_MOST;
        } else if (slope < 0) {
            s -= slope < -1 ? METAL_ROLL_UP_STEEP : METAL_ROLL_UP;
        } else {
            s = MetalSlow(s, METAL_ROLL_FLAT);
        }
        look = slope != 0 || s > METAL_ROLL_FAST ? METAL_LOOK_ROLL : METAL_LOOK_ROLL_SLOW;
    }
    // (rolling back: turned round, the DS's 0x020CA4E0's TurnAround);
    if (s < 0) {
        kirby->base.flags ^= 1;
        s = -s;
    }
    kirby->base.xspeed = s * Dir(kirby);
    Look(kirby, look, FALSE);
    MetalBody(kirby, look != METAL_LOOK_ROLL_SLOW);
    if (!(kirby->unk11A & 2)) return;
    PlaySfx(&kirby->base, SQUEAK_SE(0xAC));
    MetalBody(kirby, FALSE);
    StartState(kirby, MetalRollEnd);
    Look(kirby, METAL_LOOK_ROLL_END, TRUE);
}

// The roll's end (0x020CA3F8): 25, slowing as on flat ground, then standing (or falling);
static void MetalRollEnd(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.unk62 & 4) kirby->base.xspeed = MetalSlow(kirby->base.xspeed, METAL_ROLL_FLAT);
    MetalGravity(kirby);
    if (++kirby->base.counter < METAL_ROLL_END_FRAMES) return;
    if (kirby->base.unk62 & 4) kirby->base.flags &= ~0x20;
    else kirby->base.flags |= 0x20;
    NidlAbilityEnd(kirby);
}

/* SqueakAbilityInput(); Each Kirby's input read (scrolls.c's ScrollKirbyInput): Ninja's moves no button starts: onto a wall (the DS's
 * +0x4C: in the air, falling, held toward a wall ahead: his jump's and fall's animations (15-17, 22 turning, 24), not puffed up), onto water (+0x50: falling onto
 * its surface, the water tile's top under his feet); */
void SqueakAbilityInput(struct Kirby *kirby) {
    u16 a = kirby->animationIndex;
    s32 feet;

    if (kirby->base.unk58 & 2 || Moving(kirby) || kirby->hp <= 0) return;
    /* (Animal on the ground: Up pressed under a dig cell 17 pixels over his feet, looking up, the DS's +0x80 (0x0208B114), not puffing
     * up; crouching, B over one under his feet, digging down, +0x8C (0x0208AB1C), not sliding); */
    if (kirby->ability == SQUEAK_ABILITY_ANIMAL) {
        if (!(kirby->base.unk62 & 4)) return;
        if (kirby->unk11A & 0x40 && DigAt(kirby, 0, 17, FALSE)) {
            kirby->base.flags &= ~0x40;
            kirby->animationIndex = NIDL_MOTION_SLOT;
            StartState(kirby, AnimalLookUp);
        } else if (kirby->movementState & 0x80 && kirby->unk11A & 2 && DigAt(kirby, 0, -4, TRUE)) {
            kirby->base.flags &= ~0x40;
            StartDig(kirby, DIG_DOWN);
        }
        return;
    }
    if (kirby->ability != SQUEAK_ABILITY_NINJA) return;
    if (kirby->base.unk62 & 4 || kirby->base.yspeed > 0 || (a != KIRBY_JUMP_RISE && a != KIRBY_JUMP_FLIP && a != 17 && a != 22 && a != 24)) return;
    if (kirby->base.unk62 & 1 && HeldToward(kirby)) {
        kirby->base.flags &= ~0x40;
        kirby->base.xspeed = kirby->base.yspeed = 0;
        PlaySfx(&kirby->base, SQUEAK_SE(0x75));
        kirby->animationIndex = NIDL_MOTION_SLOT;
        StartState(kirby, NinjaCling);
        return;
    }
    feet = kirby->base.y + SCROLL_FEET;
    if (Water(&kirby->base, kirby->base.x, feet - kirby->base.yspeed + 0x100) && !Water(&kirby->base, kirby->base.x, feet - 0x800)) {
        kirby->base.flags &= ~0x40;
        kirby->base.xspeed = 0;
        kirby->animationIndex = 0;
        StartState(kirby, NinjaWater);
    }
}
