/* NiDL mini-bosses; Object types 0x40-0x42 (OBJ_EMPTY_40-42, which KatAM already treats as mini-bosses everywhere: HP bar, music,
 * camera, defeat): creates the mini-boss the template's behavior byte asks for (high nibble = mini-boss, NIDL_* in nidl.h; low nibble = its
 * variant), and starts the fight like KatAM's own mini-bosses (Mr. Frosty, 0x080CB7E8): it waits facing Kirby until Kirby is in the
 * template's area, then locks the camera there, shows the HP bar and plays the mini-boss music. The rest is shared with the enemies
 * (see nidl.h): one AnimInfo table for the type, each mini-boss's own palette, its hurt frame uploaded to the end of its VRAM
 * (64x96 here; 32x64 for type 0x42's), its own HP bar name. The Meta-Knights are several objects of type 0x42: Meta Knight (the
 * template's), who calls in his knights (see minibosses/meta_knights.c); */
#include "nidl.h"
#include "constants/kirby.h"
#include "abilities.h"

static const struct NidlMiniboss sNidlMinibosses[NIDL_MINIBOSS_COUNT] = {
    [NIDL_POPPY_BROS_SR] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, KIRBY_ABILITY_CRASH, 0, NIDL_MINIBOSS_TYPE },
    [NIDL_MR_TICK_TOCK] = { MrTickTockInit, MrTickTockStart, 30, NIDL_ABILITY_MIKE, 0, NIDL_MINIBOSS_TYPE },
    [NIDL_GRAND_WHEELIE] = { GrandWheelieInit, GrandWheelieStart, 30, KIRBY_ABILITY_WHEEL, 0, NIDL_MINIBOSS_TYPE },
    [NIDL_BUGZZY] = { BugzzyInit, BugzzyStart, 30, NIDL_ABILITY_BACKDROP, 0, NIDL_MINIBOSS_TYPE_2 },
    [NIDL_FIRE_LION] = { FireLionInit, FireLionStart, 30, KIRBY_ABILITY_BURNING, 12, NIDL_MINIBOSS_TYPE_2 },
    [NIDL_META_KNIGHTS] = { MetaKnightsInit, MetaKnightsStart, 30, 0, 0, NIDL_MINIBOSS_TYPE_3 },
};

void NidlMinibossStart(struct Object *);

void *CreateNidlMiniboss(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task;
    struct NidlObject *n;
    struct Object *obj;
    u8 boss = template->subtype1 >> 4;

    // Unknown ones, ones without graphics and ones of another type are the first of this type;
    if (boss >= NIDL_MINIBOSS_COUNT || !gNidlData.minibosses[boss].animId || sNidlMinibosses[boss].type != template->type)
        for (boss = 0; boss < NIDL_MINIBOSS_COUNT - 1; boss++)
            if (sNidlMinibosses[boss].type == template->type && gNidlData.minibosses[boss].animId) break;
    // The Meta-Knights know when one of them goes (MetaKnightsDestroy);
    task = TaskCreate(ObjectMain, NIDL_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, boss == NIDL_META_KNIGHTS ? MetaKnightsDestroy : NidlDestroy);
    n = TaskGetStructPtr(task);
    obj = &n->obj;

    InitObject(obj, template, playerId);
    n->enemy = boss;
    n->variant = template->subtype1 & 0xF;
    n->animBase = gNidlData.minibosses[boss].animBase;
    n->hurtVram = 0;
    n->sequence = NULL;
    n->activated = FALSE;
    n->hurt = 0;
    obj->unk80 = sNidlMinibosses[boss].hp;
    // As Mr. Frosty (0x080CB6E8): water doesn't defeat it, untouchable until the fight starts, its touch hurts like a mini-boss's;
    obj->base.unkC |= 1;
    obj->base.flags |= 0x2000000 | 0x200 | 0x200000;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    NidlFaceKirby(obj);
    sNidlMinibosses[boss].init(n);
    NidlSetFrame(n, 0);
    ObjectInitSprite(obj);
    obj->base.sprite.unk14 = 0x6C0;
    // A mini-boss whose frames are drawn lower (hurtShift, see KatAMExpansion.cs) has its boxes as much lower (KatAM resets objBase55
    // after every frame it draws, so the sprite can't be moved back up instead);
    NidlInitPalette(n);
    NidlUploadHurtTiles(n);
    NidlMinibossStart(obj);
    return n;
}

/* NidlMinibossWait(); Before the fight (Mr. Frosty's 0x080CB7E8): facing Kirby until Kirby is in the template's area, then the
 * camera stays there, the HP bar shows and the mini-boss music plays; */
static void NidlMinibossWait(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    NidlUpdate(n);
    obj->base.flags |= 4;
    NidlFall(obj);
    obj->kirby3 = FindTargetKirby(&obj->base);
    if ((obj->kirby3->base.unkC & 0x8000) || obj->base.roomId != obj->kirby3->base.roomId) return;
    NidlFaceKirby(obj);
    if (!Macro_08039430_2(&obj->kirby3->base, obj)) return;
    Macro_081003EC(obj, &obj->kirby3->base);
    obj->base.flags &= ~0x200;
    Macro_08100F18(obj);
    n->activated = TRUE;
    sNidlMinibosses[n->enemy].start(obj);
}

// NidlMinibossStart(); The type's behavior start (the game calls it again after a hit);
void NidlMinibossStart(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    obj->kirbyAbility = sNidlMinibosses[n->enemy].ability;
    // The engine's gravity off, as the enemies' (see NidlStart);
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    if (n->activated) {
        sNidlMinibosses[n->enemy].start(obj);
        return;
    }
    ObjectSetFunc(obj, 0, NidlMinibossWait);
    NidlSetFrame(n, 0);
    obj->base.xspeed = 0;
}
