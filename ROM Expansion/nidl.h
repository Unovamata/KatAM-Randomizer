/* NiDL enemies; Enemies ported from Kirby: Nightmare in Dream Land (USA) all share object type 0x31 (the only free regular
 * enemy type: the game treats types 0x00-0x37 as enemies all over), and the template's behavior byte picks which one:
 * high nibble = enemy (NIDL_*), low nibble = its variant. What the game reads per type is shared, so:
 * • Animations: one AnimInfo table for the type; every enemy has its own range in it (animBase + frame), and its own anim ID,
 *   which is also its palette key, so each enemy gets its own palette slot.
 * • Hurt frames: the game picks the type's hurt indexes (0 and 1), which show a generic 32x32 frame drawn from the last
 *   16 tiles of the object's VRAM; each enemy uploads its own hurt frame there (see NidlUploadHurtTiles).
 * • HP bar names: the game's name copies are patched to call NidlCopyName, which looks up the enemy's own name.
 * The tables filled by KatAMExpansion.cs (animations, hurt tiles, names) are in gNidlData; */
#ifndef GUARD_NIDL_H
#define GUARD_NIDL_H

#include "object.h"
#include "kirby.h"
#include "random.h"
#include "functions.h"
#include "task.h"

#define NIDL_TYPE OBJ_EMPTY_31

// Enemies (the behavior byte's high nibble);
enum {
    NIDL_BROOM_HATTER,
    NIDL_CAPPY,
    NIDL_ENEMY_COUNT,
};

// Room every ported enemy gets (NidlObject and its own fields);
#define NIDL_OBJECT_SIZE 0x100

#define NIDL_HURT_FRAME 0
#define NIDL_HURT_TILES 16

// NidlObject; Every ported enemy starts with it;
struct NidlObject {
    struct Object obj;
    u8 enemy;
    u8 variant;
    u16 animBase;
    u32 hurtVram;   // VRAM the hurt frame was uploaded to;
};

// NidlEnemy; What the code knows about each enemy;
struct NidlEnemy {
    void (*init)(struct NidlObject *);  // Bounds and position, after InitObject;
    void (*start)(struct Object *);     // Behavior start, also after recovering from a hit;
    s16 hp;
    u16 ability;
};

// NidlData; Filled in by KatAMExpansion.cs once the graphics are built;
struct NidlData {
    const u8 *names;                    // One 64x8 HP bar name (0x100 bytes) per enemy;
    struct {
        u16 animBase;                   // First AnimInfo of the enemy in the type's table;
        u16 animId;                     // Its anim ID (and palette key);
        const u8 *hurtTiles;            // Its 32x32 hurt frame (16 tiles);
    } enemies[NIDL_ENEMY_COUNT];
};

extern const struct NidlData gNidlData;

void NidlSetFrame(struct NidlObject *, u8 frame);
void NidlUpdate(struct NidlObject *);
void NidlFaceKirby(struct Object *);
bool32 NidlIsFloor(struct Object *, s32 x, s32 y);
void NidlStopAtLedge(struct Object *);

// Enemies;
void BroomHatterInit(struct NidlObject *);
void BroomHatterStart(struct Object *);
void CappyInit(struct NidlObject *);
void CappyStart(struct Object *);

#endif
