/* NiDL enemies; Object type 0x31: creates the enemy the template's behavior byte asks for and handles what all of them share
 * (see nidl.h); */
#include "nidl.h"
#include "constants/kirby.h"
#include "abilities.h"

extern const u8 *const gUnk_08D60EE4[];  // HP bar names per language: 64x8 (0x100 bytes) per object type;

static const struct NidlEnemy sNidlEnemies[NIDL_ENEMY_COUNT] = {
    [NIDL_BROOM_HATTER] = { BroomHatterInit, BroomHatterStart, 2, 0 },
    [NIDL_CAPPY] = { CappyInit, CappyStart, 2, 0 },
    [NIDL_TWIZZY] = { TwizzyInit, TwizzyStart, 1, 0 },
    [NIDL_BUBBLES] = { BubblesInit, BubblesStart, 2, NIDL_ABILITY_BALL },
    [NIDL_SLIPPY] = { SlippyInit, SlippyStart, 2, 0 },
    [NIDL_KABU] = { KabuInit, KabuStart, 2, 0 },
    [NIDL_COCONUT] = { CoconutInit, CoconutStart, 1, 0 },
    [NIDL_CONER] = { ConerInit, ConerStart, 2, 0 },
    [NIDL_STARMAN] = { StarmanInit, StarmanStart, 2, NIDL_ABILITY_HI_JUMP },
    [NIDL_NEEDLOUS] = { NeedlousInit, NeedlousStart, 2, NIDL_ABILITY_NEEDLE },
    [NIDL_BLADE_KNIGHT] = { BladeKnightInit, BladeKnightStart, 2, KIRBY_ABILITY_SWORD },
    [NIDL_CHILLY] = { ChillyInit, ChillyStart, 2, NIDL_ABILITY_FREEZE },
    [NIDL_COOL_SPOOK] = { CoolSpookInit, CoolSpookStart, 1, NIDL_ABILITY_LIGHT },
    [NIDL_POPPY_BROS_JR] = { PoppyBrosJrInit, PoppyBrosJrStart, 1, 0 },
    [NIDL_GIP] = { GipInit, GipStart, 2, 0 },
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

// NidlHurtTiles(); The tiles of an enemy's or mini-boss's hurt frame (its type's);
u32 NidlHurtTiles(struct Object *obj) {
    if (obj->type == NIDL_MINIBOSS_TYPE_3) return NIDL_KNIGHT_HURT_TILES;
    return NidlIsMiniboss(obj) ? NIDL_MINIBOSS_HURT_TILES : NIDL_HURT_TILES;
}

/* NidlUploadHurtTiles(); Copies the enemy's hurt frame (its hurt one, for the mini-bosses with several) into the last NidlHurtTiles
 * tiles of its VRAM, where the type's hurt animation draws from. Done again whenever the object gets new VRAM; */
void NidlUploadHurtTiles(struct NidlObject *n) {
    u32 vram = n->obj.base.sprite.tilesVram;
    u32 tiles = NidlHurtTiles(&n->obj);

    if (!vram || vram == n->hurtVram) return;
    n->hurtVram = vram;
    CpuSet(NidlGraphicsOf(n)->hurtTiles + n->hurt * tiles * 32, (void *)(vram + (gUnk_08351648[n->obj.type].numTiles - tiles) * 32), tiles * 32 / 2);
}

// NidlGraphicsOf(); The graphics of an enemy or mini-boss;
const struct NidlGraphics *NidlGraphicsOf(struct NidlObject *n) {
    return NidlIsMiniboss(&n->obj) ? &gNidlData.minibosses[n->enemy] : &gNidlData.enemies[n->enemy];
}

/* NidlLoadPalette(); The palette slot of one of the enemy's palettes: the one its frame `frame` loads (the anim's GetPalette), under
 * `key`. The type's palette key is 0 (no palette), so each enemy loads its own palettes with its anim ID as the key (| 0x2000 for a
 * second one), like ObjectInitSprite does for the other enemies; */
u8 NidlLoadPalette(struct NidlObject *n, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId == n->obj.base.roomId) {
            sub_0803DFAC(key, frame);
            palId = sub_0803DF24(key);
        } else {
            palId = 0;
        }
    }
    return palId;
}

// NidlInitPalette(); Its own palette (frame 0's; not the template's palette variant, the frames are all the enemy's);
void NidlInitPalette(struct NidlObject *n) {
    n->obj.base.sprite.palId = NidlLoadPalette(n, NidlGraphicsOf(n)->animId, 0);
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
    n->activated = FALSE;
    n->hurt = 0;
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
 * this, so ported enemies and mini-bosses show their own name; */
void NidlCopyName(struct Object *obj) {
    const u8 *name;

    if (obj->type == NIDL_TYPE)
        name = gNidlData.names + ((struct NidlObject *)obj)->enemy * 0x100;
    else if (NidlIsMiniboss(obj))
        name = gNidlData.minibossNames + ((struct NidlObject *)obj)->enemy * 0x100;
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

// NidlIsWater(); Water dy pixels below the object's position (NiDL looks 8 above it: -8; 0x08021C14);
bool32 NidlIsWater(struct Object *obj, s32 dy) {
    s32 y = obj->base.y + dy * 0x100;

    if (obj->base.x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x || obj->base.x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x
        || y < gCurLevelInfo[obj->base.unk56].levelMinPosition.y || y > gCurLevelInfo[obj->base.unk56].levelMaxPosition.y)
        return FALSE;
    return (gCollisionAttributes[GetCollisionTile(obj->base.unk56, obj->base.x >> 12, y >> 12)] & 2) != 0;
}

/* NidlKeepInLevel(); Keeps a swimmer's bottom (bottom pixels below its position) inside the level: the engine has no terrain there,
 * and below it nothing is water; */
void NidlKeepInLevel(struct Object *obj, s32 bottom) {
    s32 max = gCurLevelInfo[obj->base.unk56].levelMaxPosition.y - bottom * 0x100;

    if (obj->base.y - obj->base.yspeed > max && obj->base.yspeed < 0) {
        obj->base.y = max;
        obj->base.yspeed = 0;
    }
}

// NidlAtLevelBottom(); A swimmer's bottom is at the level's (where NidlKeepInLevel stops it), the floor there;
bool32 NidlAtLevelBottom(struct Object *obj, s32 bottom) {
    return obj->base.y >= gCurLevelInfo[obj->base.unk56].levelMaxPosition.y - bottom * 0x100;
}

/* NidlSetHeading(); 0x0806421C: the speed towards a NiDL angle (512 a circle, 0 right, 128 down; only multiples of 64 are used),
 * turning the sprite with it like NiDL's swimmers (0x0808A880); */
void NidlSetHeading(struct Object *obj, u16 angle, s16 speed) {
    static const s16 sCos[8] = { 256, 181, 0, -181, -256, -181, 0, 181 };
    u8 i = (angle >> 6) & 7;

    obj->base.xspeed = sCos[i] * speed / 256;
    obj->base.yspeed = -sCos[(i + 6) & 7] * speed / 256;    // sin, Y up;
    if (sCos[i] > 0) obj->base.flags &= ~1;
    else if (sCos[i] < 0) obj->base.flags |= 1;
}

/* NidlKirbyOctant(); 0x08064314: where Kirby is: 0 below right (more below), 1 below right (more to the right), 2 below left
 * (more to the left), 3 below left (more below), 4 above left (more above), 5 above left (more to the left), 6 above right (more
 * to the right), 7 above right (more above); */
u8 NidlKirbyOctant(struct Object *obj) {
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) >> 8;
    bool8 more = abs(dx) > abs(dy);

    if (dy >= 0) return dx >= 0 ? (more ? 1 : 0) : (more ? 2 : 3);
    return dx < 0 ? (more ? 5 : 4) : (more ? 6 : 7);
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
