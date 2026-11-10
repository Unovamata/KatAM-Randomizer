/* Star Rod stand; The Star Rod on its own, upright, in the NiDL hub where Central Circle has the Master Sword's stand (KatAMExpansionHub.cs),
 * there once the game is beaten, as the Master Sword's (gAIKirbyState from AI_KIRBY_STATE_UNK1 on: KatAM's ending sets it; the owner's):
 * object type 0x9A (OBJ_EMPTY_9A, which KatAM's spawn table leaves empty: KatAMExpansion.cs points it here, and
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

// Locked(); Not there before the game is beaten, as the Master Sword's stand (sub_080AA618, sub_080AAA64);
#define Locked() (gAIKirbyState < AI_KIRBY_STATE_UNK1)

// RodStandHide(); Before it runs and is drawn (unk7C, as sub_080AAA64): hidden and untouchable before the game is beaten (Locked()) and
// while the Star Rod is out (Held());
static void RodStandHide(struct Object *obj) {
    if (Locked() || Held()) obj->base.flags |= 0x600;
}

/* RodStandMain(); Each frame (unk78, as sub_080AA618): once the game is beaten (Locked()), back, with KatAM's puff (effect 0x292), once
 * the Star Rod isn't out (Held()); a player's Kirby touching it with no ability, free (not dying, nothing in his mouth...: sub_08054C0C's
 * checks), takes it; */
static void RodStandMain(struct Object *obj) {
    struct Kirby *kirby;

    if (Locked() || Held()) return;
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

/* The NiDL ability statues (the owner: "we can generate our own pedestals for NiDL exclusive abilities ... For a pedestal, it's a
 * matter of shrinking down the enemy and making sure it looks alright in game"): object 0x9A with a subtype (its template's
 * subtype1, 1-7: the ability NIDL_FIRST_ABILITY + subtype - 1, Needle to Backdrop), in the NiDL ability room (KatAMExpansionRooms.cs's
 * BuildNidlAbilityRoom()). Each is KatAM's ability statue (CreateAbilityStatue, sub_080AA4EC, sub_080AA588) with the NiDL enemy
 * that gives it, shrunk to a statue's size, for its figure (KatAMExpansionMotions.cs's BuildNidlStatues(): its type's AnimInfo 1-7,
 * one palette for the seven, as KatAM's statues share theirs), on KatAM's own stand (sub_080988B0: anim 0x384's stand, 16 pixels under
 * the figure). gNidlStandKeys: the palette keys of the rod's anim (0) and the statues' (1): the type's own is 0 (none), each loads its
 * own (StandPalette()); */
const u16 gNidlStandKeys[2] = { 0, 0 };

// The statues' box: KatAM's (CreateAbilityStatue);
#define STATUE_LEFT -4
#define STATUE_TOP -4
#define STATUE_RIGHT 4
#define STATUE_BOTTOM 8

// StandPalette(); Its palette's slot (KatAM's ObjectInitSprite's way, under its own key: the anim's first variant's palette);
static u8 StandPalette(struct Object *obj, u16 key) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != obj->base.roomId) return 0;
        sub_0803DFAC(key, 0);
        palId = sub_0803DF24(key);
    }
    return palId;
}

/* StatueMain(); Each frame (unk78, as sub_080AA588): a Kirby with no ability (a player's or a helper) touching it, free (not dying,
 * nothing in his mouth...), takes its ability, in KatAM's pose for getting one (sub_08054C0C); */
static void StatueMain(struct Object *obj) {
    struct Kirby *kirby;

    if (!(obj->base.flags & 0x40000) || !obj->base.unk6C) return;
    kirby = (struct Kirby *)obj->base.unk6C;
    if (kirby->base.header.kind != 0 || kirby->ability != KIRBY_ABILITY_NORMAL || kirby->hp <= 0 || kirby->animationIndex == 0x27
        || kirby->animationIndex > 0x7A || kirby->unk110 || (kirby->base.flags & 0x3800B00)) return;
    if ((kirby->transitioningAbility & 0x1F) == KIRBY_ABILITY_MASTER) gUnk_0203AD34 = 0;
    kirby->transitioningAbility = obj->kirbyAbility;
    sub_08054C0C(kirby);
    kirby->animationIndex = 0xF;
}

// NidlCreateRodStand(); Object 0x9A's spawn: as KatAM's ability statues and the Master Sword's stand (CreateAbilityStatue), still and harmless;
void *NidlCreateRodStand(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct Object), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct Object *obj = TaskGetStructPtr(task);
    u8 statue = template->subtype1;

    InitObject(obj, template, playerId);
    obj->base.unk68 &= ~7;
    obj->base.unk5C |= ~7;
    obj->base.unk5C &= ~0x80;
    obj->base.unkC |= 1 | 4;
    obj->base.flags |= 0x200000 | 0x10000 | 0x800 | 0x8000 | 0x40 | 0x100 | 0x400000;
    if (statue) {
        sub_0803E2B0(&obj->base, STATUE_LEFT, STATUE_TOP, STATUE_RIGHT, STATUE_BOTTOM);
        ObjectSetBounds(&obj->base, STATUE_LEFT, STATUE_TOP, STATUE_RIGHT, STATUE_BOTTOM);
    } else {
        sub_0803E2B0(&obj->base, BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM);
        ObjectSetBounds(&obj->base, BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM);
    }
    // (its AnimInfo: 0 the rod, 1-7 the statues);
    obj->unk83 = statue;
    ObjectInitSprite(obj);
    obj->base.sprite.palId = StandPalette(obj, gNidlStandKeys[statue != 0]);
    if (statue) {
        obj->base.sprite.unk14 = 0x780;
        if (template->subtype2) obj->base.flags |= 1;
        obj->kirbyAbility = NIDL_FIRST_ABILITY + statue - 1;
        obj->unk85 = 0;
        ObjectSetFunc(obj, statue, StatueMain);
        sub_080988B0(&obj->base);
        return obj;
    }
    obj->kirbyAbility = NIDL_ABILITY_STAR_ROD;
    obj->unk83 = 0;
    obj->unk85 = 0;
    obj->unk78 = RodStandMain;
    obj->unk7C = RodStandHide;
    return obj;
}
