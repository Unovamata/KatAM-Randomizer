/* Squeak Squad enemies; Object type 0x2C: creates the character the template's behavior byte asks for and handles what all of them
 * share (see squeak.h); */
#include "squeak.h"
#include "constants/kirby.h"
#include "constants/object_types.h"
#include "abilities.h"

void NidlDestroy(struct Task *task);

// HP as KatAM's enemies' (NiDL's ported ones' too): 4 for the ones that give an ability, 2 for the others;
const s16 gSqueakEnemyHp[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = 2,
};

static const struct SqueakCharacter sSqueakCharacters[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = { PeraraInit, PeraraStart, 0 },
};

/* SqueakAnimFor(); The anim and variant an object shows (variant << 16 | anim ID), for the game's object drawing (sub_0809A7A4,
 * patched to call this where it read them from the type's AnimInfo table): a Squeak Squad character's own anim, with its frame
 * index as the variant; the type's AnimInfo for the others, as the game does; */
u32 SqueakAnimFor(struct Object *obj) {
    const struct AnimInfo *info;

    if (obj->type == SQUEAK_TYPE) return gSqueakData.enemies[((struct SqueakObject *)obj)->enemy].animId | obj->unk83 << 16;
    info = &gUnk_08351648[obj->type].unk14[obj->unk83];
    return info->animId | info->variant << 16;
}

// SqueakSetCell(); Shows one of the character's cells (Squeak Squad's numbering);
void SqueakSetCell(struct SqueakObject *s, u8 cell) {
    s->obj.unk83 = cell + SQUEAK_HURT_VARIANTS;
}

// SqueakPlay(); Starts one of the character's Squeak Squad animations, from its first step;
void SqueakPlay(struct SqueakObject *s, u8 anim) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[anim];

    s->anim = anim;
    s->step = 0;
    s->animEnded = FALSE;
    s->timer = a->step[0].frames;
    SqueakSetCell(s, a->step[0].cell);
}

/* SqueakAnimate(); Steps the animation, once per frame: a step shows its cell its frames, then the next; after the last, a looping
 * animation starts over, the others hold their last step. Either way animEnded tells the animation got through once (Squeak
 * Squad's flag 8, which characters wait for); */
void SqueakAnimate(struct SqueakObject *s) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[s->anim];

    if ((s->animEnded && !a->loops) || (s->timer && --s->timer)) return;
    if (s->step + 1 >= a->steps) {
        s->animEnded = TRUE;
        if (!a->loops) return;
        s->step = 0;
    } else {
        s->step++;
    }
    s->timer = a->step[s->step].frames;
    SqueakSetCell(s, a->step[s->step].cell);
}

// SqueakStepExtra(); The third word of the animation's step showing (Squeak Squad moves some characters with it);
s32 SqueakStepExtra(struct SqueakObject *s) {
    return gSqueakData.enemies[s->enemy].anim[s->anim].step[s->step].extra;
}

/* SqueakSetState(); A new state for the character's own code; its timer counts the times the state ran (0 the first: Squeak Squad
 * sets it to -1, and its dispatcher adds one before each run); */
void SqueakSetState(struct SqueakObject *s, u8 state) {
    s->state = state;
    s->stateTimer = 0xFFFF;
}

/* SqueakRun(); Squeak Squad's state dispatcher (its enemies' update, vtable +0x34: 0x0206E5A4 for Perara): runs the state's
 * function, and again while it returns TRUE (it changed state and the new one runs this same frame: an entry function sets things
 * up and chains to the state that runs every frame); */
void SqueakRun(struct SqueakObject *s, bool32 (*state)(struct SqueakObject *)) {
    u8 guard = 8;

    do {
        s->stateTimer++;
    } while (state(s) && --guard);
}

/* SqueakLoadPalette(); The character's palette slot: its anim ID is the key (the type has none), as NidlLoadPalette does for NiDL's
 * enemies (KatAM keeps 7 enemy palettes a room); */
static u8 SqueakLoadPalette(struct SqueakObject *s) {
    u16 key = gSqueakData.enemies[s->enemy].animId;
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId == s->obj.base.roomId) {
            sub_0803DFAC(key, SQUEAK_HURT_VARIANTS);
            palId = sub_0803DF24(key);
        } else {
            palId = 0;
        }
    }
    return palId;
}

/* SqueakStandOnFloor(); Squeak Squad's spawn records stand a character on the ground; KatAM's templates are anywhere in the cell
 * above it, or higher (its enemies fall into place): the feet go onto the first floor below the template's cell (up to 4 cells
 * down), else stay where they are; */
void SqueakStandOnFloor(struct Object *obj) {
    s32 y = obj->base.y & ~0xFFF;
    u8 i;

    for (i = 0; i < 5; i++, y += 0x1000) {
        if (NidlIsFloor(obj, obj->base.x, y + 0x1000)) {
            obj->base.unk4C = obj->base.y = y + 0x1000 - 1;
            return;
        }
    }
}

void *CreateSqueakEnemy(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, SQUEAK_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, NidlDestroy);
    struct SqueakObject *s = TaskGetStructPtr(task);
    u8 enemy = template->subtype1 < SQUEAK_ENEMY_COUNT ? template->subtype1 : 0;
    u8 i;

    InitObject(&s->obj, template, playerId);
    s->enemy = enemy;
    s->variant = template->unk12;
    s->anim = 0;
    s->step = 0;
    s->timer = 0;
    s->animEnded = TRUE;
    s->state = 0;
    s->stateTimer = 0;
    for (i = 0; i < ARRAY_COUNT(s->vars); i++) s->vars[i] = 0;
    s->obj.unk80 = gSqueakEnemyHp[enemy];
    sSqueakCharacters[enemy].init(s);
    // (the type's AnimInfo table has one entry: ObjectInitSprite reads it; the patched drawing shows the character's own frames);
    s->obj.unk83 = 0;
    ObjectInitSprite(&s->obj);
    s->obj.base.sprite.palId = SqueakLoadPalette(s);
    SqueakSetCell(s, 0);
    SqueakStart(&s->obj);
    return s;
}

// SqueakStart(); The type's behavior start (the game calls it again after a hit);
void SqueakStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirbyAbility = sSqueakCharacters[s->enemy].ability;
    // The engine's gravity off (flag 0x40, and 0x20, which keeps it on once falling): Squeak Squad's characters have their own;
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    sSqueakCharacters[s->enemy].start(obj);
}

// SqueakName(); A character's HP bar name (NidlCopyName shows it);
const u8 *SqueakName(struct Object *obj) {
    return gSqueakData.names + ((struct SqueakObject *)obj)->enemy * 0x100;
}

// SqueakMaxHp(); A character's HP at the start (its table's: the randomizer's HP options may have changed it);
s16 SqueakMaxHp(struct Object *obj) {
    return gSqueakEnemyHp[((struct SqueakObject *)obj)->enemy];
}
