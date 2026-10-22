/* Star Rod stand; The Star Rod on its own, upright, in the NiDL hub where Central Circle has the Master Sword's stand (KatAMExpansionHub.cs),
 * there from the start: object type 0x9A (OBJ_EMPTY_9A, which KatAM's spawn table leaves empty: KatAMExpansion.cs points it here, and
 * gives it parameters: the Master Sword stand's, with the Star Rod, and the rod's anim, KatAMExpansionMotions.cs's BuildRodStand()).
 * It's the Master Sword's stand (ability_objects.c's sub_080AA618 and sub_080AAA64) with the Star Rod: a player's Kirby (not a helper)
 * with no ability who touches it takes it, in KatAM's pose for getting an ability (sub_08054C0C); it's gone while a player's Kirby has the
 * Star Rod (or is getting it) or it's out as an ability star (thrown away with Select: the Master Sword's stays away for its star too,
 * gUnk_0203AD34), and back with a puff once neither is (lost to his death, its star gone, or gone after Nightmare: finale.c's
 * StarRodEnd()). The rod taken here is his own (STAR_ROD_SAVED without STAR_ROD_FROM_FIGHT: the shard's dance after a boss doesn't
 * take it, StarRodRestore()). The rod glows and its colors cycle as on Kirby (abilities.c's NidlDisplaySprite(): gNidlAbilityData.glows[2]); */
#include "../abilities.h"
#include "code_0806F780.h"

// Its box: the rod, from its foot (its spot, on the floor) up;
#define BOX_LEFT -5
#define BOX_TOP -24
#define BOX_RIGHT 5
#define BOX_BOTTOM 0

/* Held(); Whether a player's Kirby has the Star Rod, or is getting it (transitioningAbility: KatAM clears it once he has it), or it's
 * out as an ability star (objects 0xA3 and 0xA4, CreateAbilityStar: their ability, kirbyAbility); */
static bool32 Held(void) {
    struct Task *t = &gTasks[0];
    u8 i;

    for (i = 0; i < gNumHumanPlayers; i++)
        if (gKirbys[i].ability == NIDL_ABILITY_STAR_ROD || (gKirbys[i].transitioningAbility & NIDL_ABILITY_MASK) == NIDL_ABILITY_STAR_ROD)
            return TRUE;
    for (i = 0; i < MAX_TASK_NUM && t->next; i++) {
        struct Object *obj;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != ObjectMain) continue;
        obj = TaskGetStructPtr(t);
        if ((obj->type == OBJ_ABILITY_STAR_1 || obj->type == OBJ_ABILITY_STAR_2) && obj->kirbyAbility == NIDL_ABILITY_STAR_ROD
            && !(obj->base.flags & 0x1000)) return TRUE;
    }
    return FALSE;
}

// RodStandHide(); Before it runs and is drawn (unk7C, as sub_080AAA64): hidden and untouchable while the Star Rod is out (Held());
static void RodStandHide(struct Object *obj) {
    if (Held()) obj->base.flags |= 0x600;
}

/* RodStandMain(); Each frame (unk78, as sub_080AA618): back, with KatAM's puff (effect 0x292), once the Star Rod isn't out (Held()); a
 * player's Kirby touching it with no ability, free (not dying, nothing in his mouth...: sub_08054C0C's checks), takes it; */
static void RodStandMain(struct Object *obj) {
    struct Kirby *kirby;

    if (Held()) return;
    if (obj->base.flags & 0x400) CreateEffectObject(&obj->base, 0, 0x292, 0);
    obj->base.flags &= ~0x600;
    obj->base.flags |= 4;
    if (!(obj->base.flags & 0x40000) || !obj->base.unk6C) return;
    kirby = (struct Kirby *)obj->base.unk6C;
    if (kirby->base.header.kind != 0 || kirby->base.unk56 >= gNumHumanPlayers || kirby->ability != KIRBY_ABILITY_NORMAL || kirby->hp <= 0
        || kirby->animationIndex == 0x27 || kirby->animationIndex > 0x7A || kirby->unk110 || (kirby->base.flags & 0x3800B00)) return;
    kirby->transitioningAbility = NIDL_ABILITY_STAR_ROD;
    STAR_ROD_SAVED(kirby) = 0;
    sub_08054C0C(kirby);
    kirby->animationIndex = 0xF;
    obj->base.flags |= 0x200;
}

// NidlCreateRodStand(); Object 0x9A's spawn: as KatAM's ability statues and the Master Sword's stand (CreateAbilityStatue), still and harmless;
void *NidlCreateRodStand(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct Object), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct Object *obj = TaskGetStructPtr(task);

    InitObject(obj, template, playerId);
    obj->base.unk68 &= ~7;
    obj->base.unk5C |= ~7;
    obj->base.unk5C &= ~0x80;
    obj->base.unkC |= 1 | 4;
    obj->base.flags |= 0x200000 | 0x10000 | 0x800 | 0x8000 | 0x40 | 0x100 | 0x400000;
    sub_0803E2B0(&obj->base, BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM);
    ObjectSetBounds(&obj->base, BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM);
    ObjectInitSprite(obj);
    obj->kirbyAbility = NIDL_ABILITY_STAR_ROD;
    obj->unk83 = 0;
    obj->unk85 = 0;
    obj->unk78 = RodStandMain;
    obj->unk7C = RodStandHide;
    return obj;
}
