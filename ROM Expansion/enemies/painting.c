/* Painting; Paint Roller's paintings come to life (bosses/paint_roller.c), ported from Kirby: Nightmare in Dream Land (USA), object type
 * 0x74's eight (its scripts 0x0874B540; each painting its own frame table, drawn from his descriptor's tiles and palettes), as
 * NIDL_PAINTING (the descriptor's first palette: the Kirby, the bomb, the cloud, the parasol) and NIDL_PAINTING_2 (its second: the car,
 * the Waddle Dee, the microphone, the baseball; one palette an enemy). No room has them: Paint Roller's fight makes them
 * (PaintingCreate()) and moves them as NiDL's. Enemies, so Kirby inhales one for its ability (NiDL's, each painting's: 0x0874B96C on, its
 * byte 12: the car Wheel, the microphone Mike, the baseball Ball, the bomb Crash, the cloud Spark, the parasol Parasol; the Kirby and
 * the Waddle Dee none), any attack beats one (NiDL's die to any) and touching one hurts Kirby, as KatAM's enemies; its hurt frame its
 * painting's (NiDL's 0: NidlObject.hurt). Let go (the fight over), gone; */
#include "../nidl.h"
#include "../abilities.h"
#include "constants/kirby.h"

// Each of NiDL's paintings (0-7): its enemy, its slot in it (its hurt frame), its first frame there (NiDL's 4) and its ability;
static const u8 sEnemies[8] = { NIDL_PAINTING_2, NIDL_PAINTING, NIDL_PAINTING_2, NIDL_PAINTING_2, NIDL_PAINTING_2, NIDL_PAINTING, NIDL_PAINTING,
                                NIDL_PAINTING };
static const u8 sSlots[8] = { 0, 0, 1, 2, 3, 1, 2, 3 };
static const u8 sAbilities[2][4] = {
    { KIRBY_ABILITY_NORMAL, KIRBY_ABILITY_CRASH, KIRBY_ABILITY_SPARK, KIRBY_ABILITY_PARASOL },
    { KIRBY_ABILITY_WHEEL, KIRBY_ABILITY_NORMAL, NIDL_ABILITY_MIKE, NIDL_ABILITY_BALL },
};

static void PaintingMain(struct Object *obj) {
    NidlUpdate((struct NidlObject *)obj);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
}

// PaintingStart(); Behavior start (the fight moves it): its painting's ability;
void PaintingStart(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    obj->kirbyAbility = sAbilities[n->enemy == NIDL_PAINTING_2][n->hurt & 3];
    ObjectSetFunc(obj, -1, PaintingMain);
}

// PaintingInit(); Its boxes (NiDL's: 6 either way, 0x0874BB7C), through anything (the fight moves it);
void PaintingInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    obj->base.flags |= 0x100;
    sub_0803E2B0(&obj->base, -6, -6, 6, 6);
    ObjectSetBounds(&obj->base, -6, -6, 6, 6);
}

/* PaintingCreate(); NiDL's painting `painting` (0-7) at (x, y), as its enemy: its hurt frame and ability its painting's; KatAM's
 * despawn off the screen not its (the fight's: NiDL's); */
struct Object *PaintingCreate(u8 unk56, s32 x, s32 y, u8 painting) {
    u8 enemy = sEnemies[painting & 7];
    struct Object *obj = CreateObjTemplateAndObj(unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, NIDL_TYPE,
                                                 NIDL_SPECIAL << 4 | (enemy == NIDL_PAINTING ? 4 : 5), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    struct NidlObject *n = (struct NidlObject *)obj;

    if (obj == NULL) return NULL;
    n->hurt = sSlots[painting & 7];
    n->hurtVram = 0;
    NidlUploadHurtTiles(n);
    obj->kirbyAbility = sAbilities[enemy == NIDL_PAINTING_2][n->hurt];
    obj->base.flags |= 0x2000000;
    return obj;
}

// PaintingDriven(); Still the fight's: not beaten (KatAM's defeat takes it), not inhaled;
bool32 PaintingDriven(struct Object *obj) {
    return obj->unk78 == PaintingMain && !(obj->base.flags & 0x1000) && !(obj->base.unkC & 0x400);
}
