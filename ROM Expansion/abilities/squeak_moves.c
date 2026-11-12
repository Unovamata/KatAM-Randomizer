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
        || s == NinjaArtHold || s == NinjaArtCharged || s == NinjaArtRelease;
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

// Hold(); What he holds at his hand: word 27 ahead of him;
static void Hold(struct Kirby *kirby) {
    struct Object *obj = HeldOf(kirby);

    if (obj == NULL) return;
    obj->base.x = kirby->base.x + Dir(kirby) * (NINJA_GRAB_HOLD << 8);
    obj->base.y = kirby->base.y;
    obj->base.roomId = kirby->base.roomId;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags = (obj->base.flags & ~1) | (kirby->base.flags & 1 ? 0 : 1);
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

static struct Object *GrabTouch(struct Kirby *kirby) {
    struct Task *t = &gTasks[0];
    struct ScrollPiece *p = PieceOf(kirby);
    s32 feet = kirby->base.y + SCROLL_FEET, box[4], r[4];
    u8 n;

    if (Dir(kirby) > 0) {
        box[0] = kirby->base.x - (8 << 8);
        box[2] = kirby->base.x + (17 << 8);
    } else {
        box[0] = kirby->base.x - (17 << 8);
        box[2] = kirby->base.x + (8 << 8);
    }
    box[1] = feet - (29 << 8);
    box[3] = feet;
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
        Hold(kirby);
        return obj;
    }
    return NULL;
}

// MoveBox(); The move's hit this frame (the DS's box from his feet, Y up; strength 0: none), on his piece;
static void MoveBox(struct Kirby *kirby, s8 left, s8 top, s8 right, s8 bottom, s8 strength);

// MoveStep(); His piece: at his feet, his looks (his body in his own colors), his move's box; gone with his moves (what he holds let go);
static bool32 MoveStep(struct ScrollPiece *p) {
    struct Kirby *kirby = p->kirby;

    if (!Moving(kirby) || kirby->ability != SQUEAK_ABILITY_NINJA) {
        LetGo(kirby);
        p->base.flags |= 0x1000;
        kirby->base.sprite.unk8 &= ~TORNADO_HIDE;
        p->hides = FALSE;
        return FALSE;
    }
    Follow(p);
    p->base.roomId = kirby->base.roomId;
    p->look[0].palId = kirby->base.sprite.palId;
    if (p->look[0].effect != SCROLL_NO_LOOK) {
        kirby->base.sprite.unk8 |= TORNADO_HIDE;
        p->hides = TRUE;
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

/* SqueakAbilityInput(); Each Kirby's input read (scrolls.c's ScrollKirbyInput): Ninja's moves no button starts: onto a wall (the DS's
 * +0x4C: in the air, falling, held toward a wall ahead: his jump's and fall's animations (15-17, 22 turning, 24), not puffed up), onto water (+0x50: falling onto
 * its surface, the water tile's top under his feet); */
void SqueakAbilityInput(struct Kirby *kirby) {
    u16 a = kirby->animationIndex;
    s32 feet;

    if (kirby->ability != SQUEAK_ABILITY_NINJA || kirby->base.unk58 & 2 || Moving(kirby) || kirby->hp <= 0) return;
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
