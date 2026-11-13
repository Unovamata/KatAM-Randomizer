/* scrolls.h; The Ability Scrolls' pieces (scrolls.c): what they draw of the DS's frames (looks: Squeak Squad's effects, squeak.h's
 * SQUEAK_FX_*, KatAMExpansionSqueak.cs) and hit with, each following its Kirby or flying off on its own. The Squeak Squad abilities'
 * moves ("abilities/squeak_moves.c") are drawn by them too; */
#ifndef GUARD_SCROLLS_H
#define GUARD_SCROLLS_H

#include "global.h"
#include "kirby.h"
#include "squeak.h"

#define SCROLL_FEET 0x800                    // The DS's origin, Kirby's feet: 8 pixels under KatAM's;
#define SCROLL_LOOKS 8                        // (the Magic scroll's jack-in-the-box: 7);
#define SCROLL_NO_LOOK 0xFFFF
#define SCROLL_BEHIND_KIRBYS 0x14             // A sprite's order behind every Kirby's (theirs 0x10 + unk56, KatAM's effects' 0xA);
#define TORNADO_HIDE 0x80000                // A sprite's flag (unk8) KatAM doesn't draw Kirby with (sub_0803F46C), his code never sets;
// (the pieces' kind of the Squeak Squad abilities' moves: their own step);
#define SCROLL_MOVE 0xFE

// ScrollLook; One of a piece's looks: an effect's animation (squeak.h's SQUEAK_FX_*), `dx` pixels forward and `dy` under the piece
// (the DS's composite's part: its offset);
struct ScrollLook {
    struct EffectObject e;
    u16 effect;                 // SCROLL_NO_LOOK: none;
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
    struct Task *other;         // KatAM's own attack it goes with (the tornado's; the thrown one's);
    struct ThrowAbilityObject *thrown;  // (THROW_FLASH: what Throw Kirby holds, other's);
    u8 kind;
    u8 level;
    u16 clock;
    s16 accel;                  // (MAGIC_DOVE: its speed forward gained a frame, the DS's 20.12 in KatAM's 24.8);
    u8 magic;                   // (MAGIC_BODY: MAGIC_FROZE, MAGIC_DIMMED: what the roulette holds till its end);
    /* A piece of the Squeak Squad abilities' ("abilities/squeak_moves.c", SCROLL_MOVE): its step (FALSE: gone), whether it hid its Kirby's
     * own sprite (shown again as it goes), its own data; */
    bool32 (*step)(struct ScrollPiece *);
    bool8 hides;
    u8 shown;                   // (its look shown: gSqueakAbilityLooks's, 0xFF none);
    s16 vars[6];
    void *held;                 // (Ninja's grab: what he holds, its task and its main then, its state and flags before; his start's height);
    struct Task *heldTask;
    void (*heldMain)(void);
    void *heldState;
    u32 heldFlags;
    s32 startY;
    u8 slot;                    // (its task's number + 1, for a Kirby's byte that keeps it: abilities/squeak_moves.c's MOVE_SLOT);
};
/* (a SCROLL_MOVE piece's Kirby's MOVE_SLOT (abilities.h's NIDL_MIKE_USES) once his moves' piece is gone: none, till one's made; set as
 * it goes, ScrollPieceDestroy(), if that byte still holds its slot); */
#define SCROLL_MOVE_NONE 0xFF

struct ScrollPiece *ScrollPieceCreate(struct Kirby *kirby, u8 kind, u8 level, s16 knock, u32 hitKind);
void LookEnd(struct ScrollLook *l);
void LookStartAt(struct ScrollPiece *p, u8 i, u16 effect, s8 dx, s8 dy, u16 order);
void LookStart(struct ScrollPiece *p, u8 i, u16 effect, s8 dx, s8 dy);
void Box(struct ScrollPiece *p, s8 left, s8 top, s8 right, s8 bottom, s8 strength);
void Follow(struct ScrollPiece *p);
bool32 ScrollPieceIs(struct Task *t);

#endif // GUARD_SCROLLS_H
