/* NiDL enemies; Object type 0x31: creates the enemy the template's behavior byte asks for and handles what all of them share
 * (see nidl.h); */
#include "nidl.h"

extern const u8 *const gUnk_08D60EE4[];  // HP bar names per language: 64x8 (0x100 bytes) per object type;

static const struct NidlEnemy sNidlEnemies[NIDL_ENEMY_COUNT] = {
    [NIDL_BROOM_HATTER] = { BroomHatterInit, BroomHatterStart, 2, 0 },
    [NIDL_CAPPY] = { CappyInit, CappyStart, 2, 0 },
    [NIDL_TWIZZY] = { TwizzyInit, TwizzyStart, 1, 0 },
    [NIDL_BUBBLES] = { BubblesInit, BubblesStart, 2, 0 },
    [NIDL_SLIPPY] = { SlippyInit, SlippyStart, 2, 0 },
    [NIDL_KABU] = { KabuInit, KabuStart, 2, 0 },
    [NIDL_COCONUT] = { CoconutInit, CoconutStart, 1, 0 },
    [NIDL_CONER] = { ConerInit, ConerStart, 2, 0 },
};

// NidlSetFrame(); Shows one of the enemy's frames (its own numbering);
void NidlSetFrame(struct NidlObject *n, u8 frame) {
    n->obj.unk83 = n->animBase + frame;
}

// NidlPlay(); Starts a frame sequence (see nidl.h), played by NidlAnimate;
void NidlPlay(struct NidlObject *n, const s16 *sequence) {
    n->sequence = sequence;
    n->sequenceStep = 0;
    n->sequenceTimer = 0;
    NidlAnimate(n);
}

// NidlAnimate(); Steps the frame sequence, once per frame;
void NidlAnimate(struct NidlObject *n) {
    const s16 *entry;

    if (!n->sequence || (n->sequenceTimer && --n->sequenceTimer)) return;

    entry = &n->sequence[n->sequenceStep * 2];
    if (entry[0] == NIDL_LOOP) {
        n->sequenceStep = 0;
        entry = n->sequence;
    } else if (entry[0] == NIDL_END) {
        n->sequence = NULL;
        return;
    }
    NidlSetFrame(n, entry[0]);
    n->sequenceTimer = entry[1];
    n->sequenceStep++;
}

/* NidlUploadHurtTiles(); Copies the enemy's hurt frame into the last NIDL_HURT_TILES tiles of its VRAM, where the type's
 * hurt animation draws from. Done again whenever the object gets new VRAM; */
static void NidlUploadHurtTiles(struct NidlObject *n) {
    u32 vram = n->obj.base.sprite.tilesVram;

    if (!vram || vram == n->hurtVram) return;
    n->hurtVram = vram;
    CpuSet(gNidlData.enemies[n->enemy].hurtTiles, (void *)(vram + (gUnk_08351648[NIDL_TYPE].numTiles - NIDL_HURT_TILES) * 32),
           NIDL_HURT_TILES * 32 / 2);
}

/* NidlInitPalette(); The type's palette key is 0 (no palette), so each enemy loads its own palette with its anim ID as the key,
 * like ObjectInitSprite does for the other enemies; */
static void NidlInitPalette(struct NidlObject *n) {
    u16 key = gNidlData.enemies[n->enemy].animId;
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId == n->obj.base.roomId) {
            sub_0803DFAC(key, n->obj.objTemplate->unkF);
            palId = sub_0803DF24(key);
        } else {
            palId = 0;
        }
    }
    n->obj.base.sprite.palId = palId;
}

// NidlUpdate(); Called by every enemy each frame;
void NidlUpdate(struct NidlObject *n) {
    NidlUploadHurtTiles(n);
}

// NidlFall(); The engine's gravity (0x0809A990), for enemies that only fall when there's no floor (NiDL's is the same);
void NidlFall(struct Object *obj) {
    if (obj->base.unk62 & 4) return;
    obj->base.yspeed -= 0x15;
    if (obj->base.yspeed < -0x300) obj->base.yspeed = -0x300;
}

void NidlStart(struct Object *);

void *CreateNidlEnemy(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, NIDL_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct NidlObject *n = TaskGetStructPtr(task);
    u8 enemy = template->subtype1 >> 4;

    if (enemy >= NIDL_ENEMY_COUNT) enemy = 0;

    InitObject(&n->obj, template, playerId);
    n->enemy = enemy;
    n->variant = template->subtype1 & 0xF;
    n->animBase = gNidlData.enemies[enemy].animBase;
    n->hurtVram = 0;
    n->sequence = NULL;
    n->obj.unk80 = sNidlEnemies[enemy].hp;
    sNidlEnemies[enemy].init(n);
    NidlSetFrame(n, 0);
    ObjectInitSprite(&n->obj);
    NidlInitPalette(n);
    NidlUploadHurtTiles(n);
    NidlStart(&n->obj);
    return n;
}

// NidlStart(); The type's behavior start (the game calls it again after a hit);
void NidlStart(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    obj->kirbyAbility = sNidlEnemies[n->enemy].ability;
    // The engine's gravity off (flag 0x40, and 0x20, which keeps it on once falling): it's turned back on while the enemy is
    // knocked back by a hit, and NiDL's enemies have their own;
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    sNidlEnemies[n->enemy].start(obj);
}

/* NidlCopyName(); The HP bar's name, for any object: the game's copies of it (0x08034ED2, 0x08035B34) are patched to call
 * this, so ported enemies show their own name; */
void NidlCopyName(struct Object *obj) {
    const u8 *name;

    if (obj->type == NIDL_TYPE)
        name = gNidlData.names + ((struct NidlObject *)obj)->enemy * 0x100;
    else
        name = gUnk_08D60EE4[gLanguage] + obj->type * 0x100;
    CpuSet(name, (void *)0x060077A0, 0x80);
}

// NidlFaceKirby(); 0x08063E14: faces the closest Kirby;
void NidlFaceKirby(struct Object *obj) {
    if (obj->base.x > obj->kirby3->base.x)
        obj->base.flags |= 1;
    else
        obj->base.flags &= ~1;
}

// NidlIsFloor(); Anything but an empty tile (gCollisionAttributes bit 0 marks empty tiles);
bool32 NidlIsFloor(struct Object *obj, s32 x, s32 y) {
    return (gCollisionAttributes[GetCollisionTile(obj->base.unk56, x >> 12, y >> 12)] & 1) == 0;
}

// NidlStopAtLedge(); Stops walking when the floor ends ahead (walls already stop it). A slope going down continues one tile lower;
void NidlStopAtLedge(struct Object *obj) {
    s32 x, y;

    if (!obj->base.xspeed || !(obj->base.unk62 & 4)) return;
    if (obj->base.x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x || obj->base.x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x
        || obj->base.y > gCurLevelInfo[obj->base.unk56].levelMaxPosition.y || obj->base.y < gCurLevelInfo[obj->base.unk56].levelMinPosition.y)
        return;
    x = obj->base.x + obj->base.xspeed;
    y = obj->base.y + (obj->base.unk3F + 9) * 0x100;
    if (!NidlIsFloor(obj, x, y) && !NidlIsFloor(obj, x, y + 0x1000))
        obj->base.xspeed = 0;
}
