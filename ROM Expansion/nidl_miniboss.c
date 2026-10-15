/* NiDL mini-bosses; Object type 0x40 (OBJ_EMPTY_40, which KatAM already treats as a mini-boss everywhere: HP bar, music, camera,
 * defeat): creates the mini-boss the template's behavior byte asks for (high nibble = mini-boss, NIDL_* in nidl.h; low nibble = its
 * variant), and starts the fight like KatAM's own mini-bosses (Mr. Frosty, 0x080CB7E8): it waits facing Kirby until Kirby is in the
 * template's area, then locks the camera there, shows the HP bar and plays the mini-boss music. The rest is shared with the enemies
 * (see nidl.h): one AnimInfo table for the type, each mini-boss's own palette, its hurt frame uploaded to the end of its VRAM
 * (64x64 here), its own HP bar name; */
#include "nidl.h"
#include "constants/kirby.h"
#include "abilities.h"

// The mini-bosses not ported yet stand in with Poppy Bros. Sr.'s code (and KatAMExpansion.cs gives them no graphics, see below);
static const struct NidlMiniboss sNidlMinibosses[NIDL_MINIBOSS_COUNT] = {
    [NIDL_POPPY_BROS_SR] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, KIRBY_ABILITY_CRASH },
    [NIDL_MR_TICK_TOCK] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, NIDL_ABILITY_MIKE },
    [NIDL_GRAND_WHEELIE] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, KIRBY_ABILITY_WHEEL },
    [NIDL_BUGZZY] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, NIDL_ABILITY_BACKDROP },
    [NIDL_FIRE_LION] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, KIRBY_ABILITY_BURNING },
    [NIDL_META_KNIGHTS] = { PoppyBrosSrInit, PoppyBrosSrStart, 30, 0 },
};

void NidlMinibossStart(struct Object *);

void *CreateNidlMiniboss(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, NIDL_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct NidlObject *n = TaskGetStructPtr(task);
    struct Object *obj = &n->obj;
    u8 boss = template->subtype1 >> 4;

    // Unknown ones, and ones without graphics (not ported yet), are the first;
    if (boss >= NIDL_MINIBOSS_COUNT || !gNidlData.minibosses[boss].animId) boss = 0;

    InitObject(obj, template, playerId);
    n->enemy = boss;
    n->variant = template->subtype1 & 0xF;
    n->animBase = gNidlData.minibosses[boss].animBase;
    n->hurtVram = 0;
    n->sequence = NULL;
    n->activated = FALSE;
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
