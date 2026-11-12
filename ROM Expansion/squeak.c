/* Squeak Squad enemies; Object type 0x2C: creates the character the template's behavior byte asks for and handles what all of them
 * share (see squeak.h); */
#include "squeak.h"
#include "constants/kirby.h"
#include "constants/object_types.h"
#include "abilities.h"
#include "code_0806F780.h"
#include "malloc_vram.h"

void NidlDestroy(struct Task *task);

// HP as KatAM's enemies' (NiDL's ported ones' too): 4 for the ones that give an ability, 2 for the others;
const s16 gSqueakEnemyHp[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = 2,
    [SQUEAK_GOLD_WADDLE_DEE] = 6,   // Squeak Squad's: 30, a Waddle Dee's 10 (Perara's too);
    [SQUEAK_GUSSA] = 2,             // Squeak Squad's: 10 (prmNE12Sl0.kgb);
    [SQUEAK_CRIMP] = 2,             // Squeak Squad's: 10 (prmNE13Sl0.kgb);
    [SQUEAK_CRIMP_ROCK] = 1,        // (KatAM's: any hit breaks it);
    [SQUEAK_MAIGA] = 2,             // Squeak Squad's: 10 (prmNE16Sl0.kgb);
    [SQUEAK_CRET] = 2,              // Squeak Squad's: 10 (prmNE17Sl0.kgb);
    [SQUEAK_CRET_BALL] = 1,         // (KatAM's: any hit breaks it);
    [SQUEAK_UJA] = 8,               // Squeak Squad's: 40 (prmNE18Sl0.kgb);
    [SQUEAK_BIO_SPARK] = 4,         // Squeak Squad's: 20 (prmCE18Sl0.kgb);
    [SQUEAK_BIO_SPARK_KUNAI] = 1,   // (KatAM's: any hit breaks it);
    [SQUEAK_BIO_SPARK_LEAF] = 1,
    [SQUEAK_STARMAN] = 2,           // Squeak Squad's: 10 (prmCE20Sl0.kgb);
    [SQUEAK_BUBBLE_HEAD] = 4,       // Squeak Squad's: 20 (prmCE21Sl0.kgb);
    [SQUEAK_BUBBLE_HEAD_BUBBLE] = 1,
    [SQUEAK_GAW_GAW] = 4,           // Squeak Squad's: 20 (prmCE22Sl0.kgb);
    [SQUEAK_ACCHI] = 6,             // Squeak Squad's: 30 (prmCE27Sl0.kgb);
    [SQUEAK_ACCHI_FIRE] = 1,
    [SQUEAK_BUN] = 7,               // Squeak Squad's: 35 (prmCE28Sl0.kgb);
    [SQUEAK_METALUN] = 10,          // Squeak Squad's: 50 (prmCE29Sl0.kgb);
    [SQUEAK_BIG_METALUN] = 64,      // Squeak Squad's: 300 (prmMB10Sl0.kgb), its other mid-bosses' 140 being KatAM's Mr. Frosty's 30;
    [SQUEAK_GAO_GAO] = 30,          // Squeak Squad's: 140 (prmMB09Sl0.kgb), KatAM's Mr. Frosty's 30;
    [SQUEAK_STAR] = 1,
    [SQUEAK_TEDHAUN] = 30,          // Squeak Squad's: 140 (prmMB11Sl0.kgb);
    [SQUEAK_TEDHAUN_GHOST] = 1,     // (any hit ends it, as Squeak Squad's);
    [SQUEAK_BUBOO] = 30,            // Squeak Squad's: 140 (prmMB12Sl0.kgb);
    [SQUEAK_BUBOO_BUBBLE] = 1,      // (any hit ends it, as Squeak Squad's);
    [SQUEAK_SQUEAKER] = 2,          // Squeak Squad's: 10 (prmDE01Sl0.kgb);
    [SQUEAK_SQUEAKER_BOMB] = 1,     // (KatAM's: any hit breaks it);
    [SQUEAK_SPINNI] = 12,           // Squeak Squad's: 60 (prmDB01Sl0.kgb);
    [SQUEAK_SPINNI_THROWN] = 1,
    [SQUEAK_DOC] = 24,              // Squeak Squad's: 120 (PrmDB02Sl0.kgb);
    [SQUEAK_DOC_THING] = 1,
    [SQUEAK_STORO] = 26,            // Squeak Squad's: 130 (PrmDB03Sl0.kgb);
    [SQUEAK_DAROACH] = 1,           // (untouchable: Squeak Squad's has no HP, prmDB04 none);
    [SQUEAK_KING_DEDEDE] = 60,      // Squeak Squad's: 200 (prmBB02Ml0.kgb); KatAM's bosses' for one player (Kracko's, King Golem's);
    [SQUEAK_DEDEDE_PUFF] = 1,
    [SQUEAK_MRS_MOLEY] = 60,        // Squeak Squad's: 300 (prmBB04Sl0.kgb); KatAM's bosses' for one player;
    [SQUEAK_MOLEY_THING] = 1,       // (any hit breaks it, as Squeak Squad's: 0x021291C8);
    [SQUEAK_MOLEY_DIRT] = 1,
    [SQUEAK_MECHA_KRACKO] = 64,     // Squeak Squad's: 320, then 400 (prmBB03Sl0.kgb); KatAM's bosses' 60 for one player as Mrs. Moley's
                                    // 300 (its second form's 80, squeak/mecha_kracko.c);
    [SQUEAK_MECHA_KRACKO_EYE] = 1,
    [SQUEAK_YADGAINE] = 80,         // Squeak Squad's 400 (prmBB05Sl0.kgb); KatAM's bosses' 60 for one player as Mrs. Moley's 300 is;
    [SQUEAK_YADGAINE_LEG] = 58,     // Its back leg's 290 (its front one's 220: 44, yadgaine.c);
    [SQUEAK_YADGAINE_THING] = 1,
    [SQUEAK_YADGAINE_GORDO] = 1,
    [SQUEAK_YADGAINE_DAROACH] = 1,
    [SQUEAK_BOHBOH] = 70,           // Squeak Squad's 350 (prmBB06Sl0.kgb); KatAM's bosses' 60 for one player as Mrs. Moley's 300 is;
    [SQUEAK_BOHBOH_THING] = 1,
    [SQUEAK_DAROACH_BOSS] = 80,     // Squeak Squad's 400 (prmBB07Sl0.kgb), as Bohboh's 350 is 70;
    [SQUEAK_DAROACH_BOSS_THING] = 1,
    [SQUEAK_DARK_DAROACH] = 80,     // (the same parameters);
    [SQUEAK_DARK_DAROACH_THING] = 1,
    [SQUEAK_META_KNIGHT] = 80,      // Squeak Squad's 400 (PrmBB01Sl0.kgb), as Daroach's;
    [SQUEAK_META_KNIGHT_THING] = 1,
    [SQUEAK_META_KNIGHT_BEAM] = 1,
    [SQUEAK_META_KNIGHT_WAVE] = 1,
    [SQUEAK_META_KNIGHT_SPARKLE] = 1,
    [SQUEAK_DARK_NEBULA] = 60,      // Squeak Squad's 300 (prmLB02Sl0.kgb), as Mecha-Kracko's 320 is 64;
    [SQUEAK_DARK_NEBULA_EYE] = 1,
    [SQUEAK_DARK_NEBULA_TWINKLE] = 1,
    [SQUEAK_DARK_NEBULA_AURA] = 1,
    [SQUEAK_DARK_NEBULA_SHOT] = 1,
    [SQUEAK_DARK_NEBULA_FIREBALL] = 10, // Its 50 (prmLB02Ml0's 105th);
    [SQUEAK_DARK_NEBULA_FIRE] = 1,
    [SQUEAK_DARK_NEBULA_ICE] = 1,
    [SQUEAK_DARK_NEBULA_BOLT] = 1,
    [SQUEAK_DARK_NEBULA_BURN] = 1,
    [SQUEAK_DARK_STAR] = 2,         // Squeak Squad's 10 (prmLB01Sl0.kgb; nothing hits it);
};

static const struct SqueakCharacter sSqueakCharacters[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = { PeraraInit, PeraraStart, 0, FALSE },
    [SQUEAK_GOLD_WADDLE_DEE] = { GoldWaddleDeeInit, GoldWaddleDeeStart, 0, TRUE },
    [SQUEAK_GUSSA] = { GussaInit, GussaStart, 0, TRUE },
    [SQUEAK_CRIMP] = { CrimpInit, CrimpStart, 0, FALSE },
    [SQUEAK_CRIMP_ROCK] = { CrimpRockInit, CrimpRockStart, 0, FALSE },
    [SQUEAK_MAIGA] = { MaigaInit, MaigaStart, 0, FALSE },
    [SQUEAK_CRET] = { CretInit, CretStart, 0, FALSE },
    [SQUEAK_CRET_BALL] = { CretBallInit, CretBallStart, 0, FALSE },
    [SQUEAK_UJA] = { UjaInit, UjaStart, 0, FALSE },
    [SQUEAK_BIO_SPARK] = { BioSparkInit, BioSparkStart, SQUEAK_ABILITY_NINJA, FALSE },   // The DS's Ninja (its ability 0x13);
    [SQUEAK_BIO_SPARK_KUNAI] = { BioSparkKunaiInit, BioSparkKunaiStart, 0, FALSE },
    [SQUEAK_BIO_SPARK_LEAF] = { BioSparkLeafInit, BioSparkLeafStart, 0, FALSE },
    [SQUEAK_STARMAN] = { SqueakStarmanInit, SqueakStarmanStart, 0, FALSE },
    [SQUEAK_BUBBLE_HEAD] = { BubbleHeadInit, BubbleHeadStart, 0, FALSE },
    [SQUEAK_BUBBLE_HEAD_BUBBLE] = { BubbleHeadBubbleInit, BubbleHeadBubbleStart, 0, FALSE },
    [SQUEAK_GAW_GAW] = { GawGawInit, GawGawStart, 0, FALSE },
    [SQUEAK_ACCHI] = { AcchiInit, AcchiStart, KIRBY_ABILITY_FIRE, FALSE },
    [SQUEAK_ACCHI_FIRE] = { AcchiFireInit, AcchiFireStart, 0, FALSE },
    [SQUEAK_BUN] = { BunInit, BunStart, KIRBY_ABILITY_THROW, FALSE },
    [SQUEAK_METALUN] = { MetalunInit, MetalunStart, 0, FALSE },
    [SQUEAK_BIG_METALUN] = { BigMetalunInit, BigMetalunStart, 0, FALSE, TRUE },
    [SQUEAK_GAO_GAO] = { GaoGaoInit, GaoGaoStart, 0, FALSE, TRUE },
    [SQUEAK_STAR] = { SqueakStarInit, SqueakStarStart, 0, FALSE },
    [SQUEAK_TEDHAUN] = { TedhaunInit, TedhaunStart, 0, FALSE, TRUE },
    [SQUEAK_TEDHAUN_GHOST] = { TedhaunGhostInit, TedhaunGhostStart, 0, FALSE },
    [SQUEAK_BUBOO] = { BubooInit, BubooStart, 0, FALSE, TRUE },
    [SQUEAK_BUBOO_BUBBLE] = { BubooBubbleInit, BubooBubbleStart, 0, FALSE },
    [SQUEAK_SQUEAKER] = { SqueakerInit, SqueakerStart, 0, FALSE },
    [SQUEAK_SQUEAKER_BOMB] = { SqueakerBombInit, SqueakerBombStart, 0, FALSE },
    [SQUEAK_SPINNI] = { SpinniInit, SpinniStart, 0, FALSE, FALSE, TRUE },
    [SQUEAK_SPINNI_THROWN] = { SpinniThrownInit, SpinniThrownStart, 0, FALSE },
    [SQUEAK_DOC] = { DocInit, DocStart, 0, FALSE, FALSE, TRUE },
    [SQUEAK_DOC_THING] = { DocThingInit, DocThingStart, 0, FALSE },
    [SQUEAK_STORO] = { StoroInit, StoroStart, 0, FALSE, FALSE, TRUE },
    [SQUEAK_DAROACH] = { DaroachInit, DaroachStart, 0, FALSE },
    [SQUEAK_KING_DEDEDE] = { SqueakDededeInit, SqueakDededeStart, 0, FALSE, TRUE },
    [SQUEAK_DEDEDE_PUFF] = { DededePuffInit, DededePuffStart, 0, FALSE },
    [SQUEAK_MRS_MOLEY] = { MrsMoleyInit, MrsMoleyStart, 0, FALSE, TRUE },
    [SQUEAK_MOLEY_THING] = { MoleyThingInit, MoleyThingStart, 0, FALSE },
    [SQUEAK_MOLEY_DIRT] = { MoleyDirtInit, MoleyDirtStart, 0, FALSE },
    [SQUEAK_MECHA_KRACKO] = { MechaKrackoInit, MechaKrackoStart, 0, FALSE, TRUE },
    [SQUEAK_MECHA_KRACKO_EYE] = { MechaKrackoEyeInit, MechaKrackoEyeStart, 0, FALSE },
    [SQUEAK_YADGAINE] = { YadgaineInit, YadgaineStart, 0, FALSE, TRUE },
    [SQUEAK_YADGAINE_LEG] = { YadgaineLegInit, YadgaineLegStart, 0, FALSE },
    [SQUEAK_YADGAINE_THING] = { YadgaineThingInit, YadgaineThingStart, 0, FALSE },
    [SQUEAK_YADGAINE_GORDO] = { YadgaineGordoInit, YadgaineGordoStart, 0, FALSE },
    [SQUEAK_YADGAINE_DAROACH] = { YadgaineDaroachInit, YadgaineDaroachStart, 0, FALSE },
    [SQUEAK_BOHBOH] = { BohbohInit, BohbohStart, 0, FALSE, TRUE },
    [SQUEAK_BOHBOH_THING] = { BohbohThingInit, BohbohThingStart, 0, FALSE },
    [SQUEAK_DAROACH_BOSS] = { DaroachBossInit, DaroachBossStart, 0, FALSE, TRUE },
    [SQUEAK_DAROACH_BOSS_THING] = { DaroachBossThingInit, DaroachBossThingStart, 0, FALSE },
    [SQUEAK_DARK_DAROACH] = { DaroachBossInit, DaroachBossStart, 0, FALSE, TRUE },
    [SQUEAK_DARK_DAROACH_THING] = { DaroachBossThingInit, DaroachBossThingStart, 0, FALSE },
    [SQUEAK_META_KNIGHT] = { SqueakMetaKnightInit, SqueakMetaKnightStart, 0, FALSE, TRUE },
    [SQUEAK_META_KNIGHT_THING] = { SqueakMetaKnightThingInit, SqueakMetaKnightThingStart, 0, FALSE },
    [SQUEAK_META_KNIGHT_BEAM] = { SqueakMetaKnightThingInit, SqueakMetaKnightThingStart, 0, FALSE },
    [SQUEAK_META_KNIGHT_WAVE] = { SqueakMetaKnightThingInit, SqueakMetaKnightThingStart, 0, FALSE },
    [SQUEAK_META_KNIGHT_SPARKLE] = { SqueakMetaKnightThingInit, SqueakMetaKnightThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA] = { SqueakDarkNebulaInit, SqueakDarkNebulaStart, 0, FALSE, TRUE },
    [SQUEAK_DARK_NEBULA_EYE] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_TWINKLE] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_AURA] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_SHOT] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_FIREBALL] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_FIRE] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_ICE] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_BOLT] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_NEBULA_BURN] = { SqueakDarkNebulaThingInit, SqueakDarkNebulaThingStart, 0, FALSE },
    [SQUEAK_DARK_STAR] = { SqueakDarkStarInit, SqueakDarkStarStart, 0, FALSE },
};

// SqueakVramTiles(); The tiles a sprite's VRAM block holds (KatAM's heap keeps each block's segments, 2 tiles each); 0 none;
static u16 SqueakVramTiles(u32 vram) {
    if (!SqueakVramOk(vram) || vram < gVramHeapStartAddr) return 0;
    return gVramHeapState[(vram - gVramHeapStartAddr) / 0x40] * 2;
}

/* SqueakVramFits(); KatAM's VRAM heap has a free block of `tiles` (its blocks walked, each skipped by its size): VramMalloc's own
 * search tries every free segment against those after it, a fifth of a frame when there's none (Dark Daroach's beam's sprites
 * waiting for his fire column's VRAM), so a retry asks only when it fits; */
bool32 SqueakVramFits(u16 tiles) {
    u16 need = (tiles + 1) / 2, run = 0, i = 0, n = gVramHeapMaxTileSlots / 2;

    while (i < n) {
        if (gVramHeapState[i] != 0) {
            run = 0;
            i += gVramHeapState[i];
            continue;
        }
        if (++run >= need) return TRUE;
        i++;
    }
    return FALSE;
}

/* SqueakFitVram(); A character with its frames' tiles (SqueakData.cellTiles: Daroach's stars, drawn with his graphics) gets as many
 * as the frame it's about to show takes: a bigger block when it hasn't that many (its frame's tiles copied in again, KatAM copying
 * them as a frame starts), none while there's none that big (unkC 0x200: KatAM's sub_0809A630 would ask for its type's 123 every
 * frame, a long search when the heap's full), asked for again next frame; drawn by nothing (flag 8: SqueakNoSprite), none; */
static void SqueakFitVram(struct SqueakObject *s) {
    const u16 *tiles = gSqueakData.cellTiles != NULL ? gSqueakData.cellTiles[s->enemy] : NULL;
    struct Sprite *sprite = &s->obj.base.sprite;
    u16 need, have;
    u32 vram;

    if (tiles == NULL || (s->obj.base.flags & 8) || s->obj.unk83 >= tiles[0]) return;
    need = tiles[1 + s->obj.unk83];
    have = SqueakVramTiles(sprite->tilesVram);
    if (need == 0 || have >= need) return;
    if (have) VramFree(sprite->tilesVram);
    vram = SqueakVramFits(need) ? VramMalloc(need) : 0;
    if (!SqueakVramOk(vram)) {
        sprite->tilesVram = 0;
        s->obj.base.unkC |= 0x200;
        return;
    }
    sprite->tilesVram = vram;
    s->obj.base.unkC &= ~0x200;
    sprite->unk1B = 0xFF;
}

/* SqueakAnimFor(); The anim and variant an object shows (variant << 16 | anim ID), for the game's object drawing (sub_0809A7A4,
 * patched to call this where it read them from the type's AnimInfo table): a Squeak Squad character's own anim, with its frame
 * index as the variant (its VRAM fitted to it first: SqueakFitVram); the type's AnimInfo for the others, as the game does; */
u32 SqueakAnimFor(struct Object *obj) {
    const struct AnimInfo *info;

    if (SqueakIsSqueak(obj)) {
        SqueakFitVram((struct SqueakObject *)obj);
        return gSqueakData.enemies[((struct SqueakObject *)obj)->enemy].animId | obj->unk83 << 16;
    }
    // (a Squeak Squad room's gimmicks on the hammer peg's type: its switches, shutters: squeak/gimmicks.c);
    if (obj->type == OBJ_HAMMER_PEG && SqueakGimmickAnim(obj) != 0) return SqueakGimmickAnim(obj);
    info = &gUnk_08351648[obj->type].unk14[obj->unk83];
    return info->animId | info->variant << 16;
}

// SqueakSetCell(); Shows one of the character's cells (Squeak Squad's numbering);
void SqueakSetCell(struct SqueakObject *s, u8 cell) {
    s->obj.unk83 = cell + SQUEAK_HURT_VARIANTS;
}

static void SqueakReleaseParts(struct SqueakObject *s);
static void SqueakShowComposite(struct SqueakObject *s);

/* SqueakAnimPalette(); An animation's palette (its key and frame): in the character's hurt flash's colors while they're on
 * (SqueakDamageColors()), else the animation's own (a character in several palettes), else NULL: the character's one; */
static const struct SqueakPaletteKey *SqueakAnimPalette(struct SqueakObject *s, u8 anim) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];

    if (s->damage && gSqueakData.damage != NULL && gSqueakData.damage[s->enemy] != NULL) return &gSqueakData.damage[s->enemy][anim];
    return g->palette != NULL ? &g->palette[anim] : NULL;
}

/* SqueakDamageColors(); Squeak Squad's hurt flash (the DS's vtable +0x20, Daroach's 0x02126B30: his _dmg.cla's palettes, +0x33C,
 * or his own, +0x23C, into his sprite's): the character's animation in its damage colors (`on`), or its own back; a composite's
 * parts take them as its composition shows them (SqueakShowComposite()); none for a character without them; */
void SqueakDamageColors(struct SqueakObject *s, bool32 on) {
    const struct SqueakPaletteKey *k;

    on = on != 0;
    if (s->damage == on || gSqueakData.damage == NULL || gSqueakData.damage[s->enemy] == NULL) return;
    s->damage = on;
    k = SqueakAnimPalette(s, s->anim);
    if (k != NULL && s->composite == SQUEAK_NO_COMPOSITE) s->obj.base.sprite.palId = SqueakLoadPalette(k->key, k->variant, s->obj.base.roomId);
}
static void SqueakShowLayer(struct SqueakObject *s);

// SqueakPlay(); Starts one of the character's Squeak Squad animations, from its first step;
void SqueakPlay(struct SqueakObject *s, u8 anim) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];
    const struct SqueakAnim *a = &g->anim[anim];

    const struct SqueakPaletteKey *k = SqueakAnimPalette(s, anim);

    // (a character in several palettes: the animation's);
    if (k != NULL) s->obj.base.sprite.palId = SqueakLoadPalette(k->key, k->variant, s->obj.base.roomId);
    SqueakReleaseParts(s);
    s->composite = SQUEAK_NO_COMPOSITE;
    s->anim = anim;
    s->step = 0;
    s->animEnded = FALSE;
    s->timer = a->step[0].frames;
    SqueakSetCell(s, a->step[0].cell);
}

/* SqueakPlayComposite(); Starts one of the character's composite animations (.mca: the sprite object's vtable +8 with 0x3000000 |
 * the animation), from its first frame; */
void SqueakPlayComposite(struct SqueakObject *s, u8 composite) {
    // (a character without it: none played, its end at once);
    if (composite >= gSqueakData.enemies[s->enemy].composites) {
        s->animEnded = TRUE;
        return;
    }
    s->composite = composite;
    s->compositeTime = 0;
    s->animEnded = FALSE;
    SqueakShowComposite(s);
}

// SqueakTurned(); An offset across as the character faces (Squeak Squad's are as it faces right: KatAM's flag 1 unset);
static s32 SqueakTurned(struct SqueakObject *s, s32 x) {
    return s->obj.base.flags & 1 ? -x : x;
}

/* SqueakAnimate(); Steps the animation, once per frame: a step shows its cell its frames, then the next; after the last, a looping
 * animation starts over, the others hold their last step. animEnded is Squeak Squad's flag 8, which characters wait for (its body's
 * update sets it each frame from the sprite's, 0x02014590 asking vtable +0x1C, 0x01FFA8B8): a held animation's from the frame it got
 * through on; a looping one's only on the frames it starts over (a pulse: waiting on it twice waits for two loops). The step's offset moves the drawing (Squeak Squad adds it to the cell's position when
 * it builds the OAM, 0x01FF8720, turned with the character as the cell; KatAM's objBase54/55, which the drawing clears once used); */
void SqueakAnimate(struct SqueakObject *s) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[s->anim];

    if (s->composite != SQUEAK_NO_COMPOSITE) {
        s->compositeTime++;
        SqueakShowComposite(s);
        return;
    }
    if (a->loops) s->animEnded = FALSE;
    if (!(s->animEnded && !a->loops) && !(s->timer && --s->timer)) {
        if (s->step + 1 < a->steps) {
            s->step++;
            s->timer = a->step[s->step].frames;
            SqueakSetCell(s, a->step[s->step].cell);
        } else {
            s->animEnded = TRUE;
            if (a->loops) {
                s->step = 0;
                s->timer = a->step[0].frames;
                SqueakSetCell(s, a->step[0].cell);
            }
        }
    }
    s->obj.base.objBase54 = SqueakTurned(s, a->step[s->step].x);
    s->obj.base.objBase55 = a->step[s->step].y;
    SqueakShowLayer(s);
}

/* SqueakAnimTime(); The frames the animation has shown since it started (Squeak Squad's time in it, which its .rca boxes go by): its
 * steps' before this one and this one's so far; */
u32 SqueakAnimTime(struct SqueakObject *s) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[s->anim];
    u32 time = a->step[s->step].frames - s->timer;
    u8 i;

    for (i = 0; i < s->step; i++) time += a->step[i].frames;
    return time;
}

// SqueakLength(); An animation's frames in all;
static u32 SqueakLength(const struct SqueakAnim *a) {
    u32 total = 0;
    u8 i;

    for (i = 0; i < a->steps; i++) total += a->step[i].frames;
    return total;
}

// SqueakStepAt(); The step an animation shows `time` frames in (a looping one starts over, the others hold their last step);
static const struct SqueakStep *SqueakStepAt(const struct SqueakAnim *a, u32 time) {
    u32 total = SqueakLength(a);
    u8 i;

    if (total == 0) return &a->step[0];
    if (a->loops) time %= total;
    for (i = 0; i < a->steps; i++) {
        if (time < a->step[i].frames) return &a->step[i];
        time -= a->step[i].frames;
    }
    return &a->step[a->steps - 1];
}

/* Composite animations (.mca): a composition's parts are the character's own animations, played at once where the composition puts
 * them (0x01FFA640: they turn with the character), each at the time the part asks for (0x01FFA6CC: the frames since the composite
 * animation started, or its time within it), the first on top. The character's own sprite shows the first part, a SqueakPartFx
 * each of the others, drawn behind it in their order (KatAM draws a lower order in front: 0x1A, the enemies', then 0x1B, ...); */
struct SqueakPartFx {
    struct EffectObject e;
    struct SqueakObject *owner; // Placed by it (SqueakShowComposite) each frame; gone with it or its composite animation;
    struct SqueakPartFx *next;
    s16 dx, dy;                 // Where it's drawn (pixels from the character, turned: Daroach's beam's parts reach 248);
    u8 cell;                    // Its cell (the character's frame, variants SQUEAK_HURT_VARIANTS on);
    u8 index;                   // Its part (1 on);
    u8 palId;                   // Its palette: its animation's (a character in several palettes: Dedede's sparkles), else 0xFF: its character's;
    u16 tiles;                  // Its VRAM's tiles (its part's frames');
};

// SqueakPartDestroy(); A part's end (its own, or the room's): its character lets go of it;
static void SqueakPartDestroy(struct Task *t) {
    struct SqueakPartFx *p = TaskGetStructPtr(t), **link;

    if (p->owner) {
        for (link = (struct SqueakPartFx **)&p->owner->parts; *link; link = &(*link)->next) {
            if (*link == p) {
                *link = p->next;
                break;
            }
        }
    }
    ObjectBaseDestroy(t);
}

/* SqueakPartMain(); Draws a part where its character placed it, facing as it, with its palette; none while the character shows
 * KatAM's hurt frames (they're its whole), or isn't drawn; */
static void SqueakPartMain(void) {
    struct SqueakPartFx *p = TaskGetStructPtr(gCurTask);
    struct SqueakObject *s = p->owner;

    if ((p->e.flags & 0x1000) || s == NULL) {
        TaskDestroy(gCurTask);
        return;
    }
    if (s->composite == SQUEAK_NO_COMPOSITE || s->obj.unk83 < SQUEAK_HURT_VARIANTS || (s->obj.base.flags & 0x400)) return;
    // (VRAM full when it was made, other sprites' let go of a frame later: asked for again, not drawn till it's got);
    if (!SqueakVramOk(p->e.sprite.tilesVram)) {
        if (!SqueakVramFits(p->tiles)) return;
        p->e.sprite.tilesVram = VramMalloc(p->tiles);
        if (!SqueakVramOk(p->e.sprite.tilesVram)) return;
        /* (its frame's tiles copied in again: KatAM copies them as a frame starts (sub_08155370), its frame started when it had
         * none, the VRAM showing what was there: Dark Daroach's beam in bits); */
        p->e.sprite.unk1B = 0xFF;
    }
    // (its offset added to its spot, past the drawing offset's reach of 127 pixels);
    p->e.x = s->obj.base.x + (p->dx << 8);
    p->e.y = s->obj.base.y + (p->dy << 8);
    p->e.sprite.variant = p->cell + SQUEAK_HURT_VARIANTS;
    p->e.sprite.palId = p->palId != 0xFF ? p->palId : s->obj.base.sprite.palId;
    // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right, flipped when facing left);
    p->e.flags = (p->e.flags & ~0x401) | 4 | (s->obj.base.flags & 1 ? 0 : 1);
    p->e.objBase54 = 0;
    p->e.objBase55 = 0;
    EffectObjectPostUpdate(&p->e);
}

/* SqueakCreatePart(); A sprite for part `index` of the character's compositions, `tiles` of VRAM (its part's frames', not the
 * character's biggest: King Dedede's sparkles), made on `cell`, the one it shows (KatAM's sprite copies its first frame's tiles as
 * it's made: another cell's, bigger than its VRAM, would write over the sprites after it, Meta Knight's sword under his tornado's); */
static struct SqueakPartFx *SqueakCreatePart(struct SqueakObject *s, u8 index, u8 tiles, u8 cell) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];
    struct Task *t;
    struct SqueakPartFx *p;

    if (gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return NULL;
    t = TaskCreate(SqueakPartMain, sizeof(struct SqueakPartFx), 0x3500, TASK_USE_EWRAM, SqueakPartDestroy);
    if (t == NULL) return NULL;
    p = TaskGetStructPtr(t);
    ClearEffectObject(&p->e);
    p->e.header.kind = 3;
    p->e.roomId = s->obj.base.roomId;
    p->e.parent = NULL;
    p->e.x = s->obj.base.x;
    p->e.y = s->obj.base.y;
    p->owner = s;
    p->next = NULL;
    p->dx = p->dy = 0;
    p->cell = cell;
    p->index = index;
    p->palId = 0xFF;
    p->tiles = tiles ? tiles : g->tiles;
    EffectObjectInitSprite(&p->e, &p->e.sprite, p->tiles, g->animId, cell + SQUEAK_HURT_VARIANTS, 0x1A + index < 0x1F ? 0x1A + index : 0x1F);
    p->e.sprite.palId = s->obj.base.sprite.palId;
    return p;
}

void SqueakNoSprite(struct SqueakObject *s) {
    /* (as the NiDL bosses' own objects, bosses/dedede.c: unkC 0x200 keeps KatAM from giving it VRAM again, sub_0809A630 asking for
     * its type's every frame a sprite has none, VramMalloc's 0 when it's full; flag 8 from stepping its frames); */
    s->obj.base.unkC |= 0x200;
    if (SqueakVramOk(s->obj.base.sprite.tilesVram)) VramFree(s->obj.base.sprite.tilesVram);
    s->obj.base.sprite.tilesVram = 0;
    s->obj.base.flags |= 8 | 0x400;
}

// SqueakReleaseParts(); Lets go of the parts (each ends itself on its next frame);
static void SqueakReleaseParts(struct SqueakObject *s) {
    struct SqueakPartFx *p, *next;

    for (p = s->parts; p != NULL; p = next) {
        next = p->next;
        p->owner = NULL;
        p->next = NULL;
    }
    s->parts = NULL;
}

/* Layers: a character's cell with pieces in other palettes (KatAMExpansionSqueak.cs's LayerPalettes) has them drawn by a sprite of
 * their own (its anim, its own palette), where the character is, with its step's offset, facing as it, in front of it or behind as the
 * DS draws them; */
struct SqueakLayerFx {
    struct EffectObject e;
    struct SqueakObject *owner;
    u16 frame;                  // Its layer frame + 1 (0: none this frame);
    u8 index;                   // Which of the character's layers (0, 1);
    bool8 front;
};

// SqueakLayerDestroy(); A layer's end (its own, or the room's): its character lets go of it;
static void SqueakLayerDestroy(struct Task *t) {
    struct SqueakLayerFx *l = TaskGetStructPtr(t);

    if (l->owner && l->owner->layer[l->index] == l) l->owner->layer[l->index] = NULL;
    ObjectBaseDestroy(t);
}

// SqueakLayerMain(); Draws the layer's frame where its character is (none while it has none, or the character isn't drawn);
static void SqueakLayerMain(void) {
    struct SqueakLayerFx *l = TaskGetStructPtr(gCurTask);
    struct SqueakObject *s = l->owner;

    if ((l->e.flags & 0x1000) || s == NULL) {
        TaskDestroy(gCurTask);
        return;
    }
    if (l->frame == 0 || s->obj.unk83 < SQUEAK_HURT_VARIANTS || (s->obj.base.flags & 0x400)) return;
    l->e.x = s->obj.base.x;
    l->e.y = s->obj.base.y;
    l->e.sprite.variant = l->frame - 1;
    // (KatAM draws a lower order in front: the character's own, one less or more);
    l->e.sprite.unk14 = l->front ? s->obj.base.sprite.unk14 - 0x40 : s->obj.base.sprite.unk14 + 0x40;
    // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right, flipped when facing left);
    l->e.flags = (l->e.flags & ~0x401) | 4 | (s->obj.base.flags & 1 ? 0 : 1);
    l->e.objBase54 = s->obj.base.objBase54;
    l->e.objBase55 = s->obj.base.objBase55;
    EffectObjectPostUpdate(&l->e);
}

// SqueakShowLayer(); Its cell's frame of each layer this frame (a layer's sprite made the first time one shows, its own palette loaded);
static void SqueakShowLayer(struct SqueakObject *s) {
    const struct SqueakLayer *layer;
    struct SqueakLayerFx *l;
    u16 frame;
    u8 k;
    struct Task *t;

    if (gSqueakData.layers == NULL) return;
    for (k = 0; k < SQUEAK_LAYERS; k++) {
        layer = &gSqueakData.layers[s->enemy * SQUEAK_LAYERS + k];
        l = s->layer[k];
        frame = 0;
        if (layer->animId == 0) return;
        if (s->obj.unk83 >= SQUEAK_HURT_VARIANTS && s->obj.unk83 - SQUEAK_HURT_VARIANTS < gSqueakData.enemies[s->enemy].cells)
            frame = layer->cell[s->obj.unk83 - SQUEAK_HURT_VARIANTS];
        if (l == NULL) {
            if ((frame & 0x7FFF) == 0 || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) continue;
            t = TaskCreate(SqueakLayerMain, sizeof(struct SqueakLayerFx), 0x3500, TASK_USE_EWRAM, SqueakLayerDestroy);
            if (t == NULL) continue;
            l = TaskGetStructPtr(t);
            ClearEffectObject(&l->e);
            l->e.header.kind = 3;
            l->e.roomId = s->obj.base.roomId;
            l->e.parent = NULL;
            l->e.x = s->obj.base.x;
            l->e.y = s->obj.base.y;
            l->owner = s;
            l->index = k;
            EffectObjectInitSprite(&l->e, &l->e.sprite, layer->tiles, layer->animId, 0, 0x1A);
            l->e.sprite.palId = SqueakLoadPalette(layer->palette.key, layer->palette.variant, s->obj.base.roomId);
            s->layer[k] = l;
        }
        l->frame = frame & 0x7FFF;
        l->front = (frame & 0x8000) != 0;
    }
}

/* SqueakShowComposite(); The composite animation at its time: the composition its step shows, and each part's cell and offset (the
 * first the character's own, the others its SqueakPartFx, made or let go as the composition has them); animEnded (flag 8) once its
 * frames went by (as a plain animation's: a composite of 4 frames started at frame t is done at t + 4, as recorded: Bubble Head's),
 * a looping one's on each frame it starts over; */
static void SqueakShowComposite(struct SqueakObject *s) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];
    const struct SqueakComposite *c = &g->composite[s->composite];
    const struct SqueakComposition *composition;
    struct SqueakPartFx **link = (struct SqueakPartFx **)&s->parts, *p;
    u32 total = 0, time = s->compositeTime, local;
    u8 i;

    for (i = 0; i < c->steps; i++) total += c->step[i].frames;
    if (c->loops) s->animEnded = total && time && time % total == 0;
    else if (total && time >= total) s->animEnded = TRUE;
    local = total == 0 ? 0 : c->loops ? time % total : time < total ? time : total - 1;
    time = local;
    for (i = 0; i + 1 < c->steps && time >= c->step[i].frames; i++) time -= c->step[i].frames;
    composition = &g->composition[c->step[i].composition];
    for (i = 0; i < composition->parts; i++) {
        const struct SqueakPart *part = &composition->part[i];
        const struct SqueakStep *step = SqueakStepAt(&g->anim[part->anim], part->absolute ? s->compositeTime : local);
        s32 x = SqueakTurned(s, part->x + step->x), y = part->y + step->y;

        const struct SqueakPaletteKey *k = SqueakAnimPalette(s, part->anim);

        if (i == 0) {
            if (k != NULL) s->obj.base.sprite.palId = SqueakLoadPalette(k->key, k->variant, s->obj.base.roomId);
            SqueakSetCell(s, step->cell);
            s->obj.base.objBase54 = x;
            s->obj.base.objBase55 = y;
            // (its layers, as its cell's: Meta Knight's sword over his body in his composites);
            SqueakShowLayer(s);
            continue;
        }
        /* (a part whose VRAM isn't this composition's part's frames' size: a new one, the old let go of, its VRAM free a frame
         * later; KatAM's VRAM heap in a stage is 512 tiles, Mecha-Kracko's bolts take seven parts); */
        if ((p = *link) != NULL && p->tiles != part->tiles) {
            *link = p->next;
            p->owner = NULL;
            p->next = NULL;
            p = SqueakCreatePart(s, i, part->tiles, step->cell);
            if (p == NULL) break;
            p->next = *link;
            *link = p;
        }
        if (*link == NULL) *link = SqueakCreatePart(s, i, part->tiles, step->cell);
        if ((p = *link) == NULL) break;
        // (a character in several palettes: the part's animation's own);
        if (k != NULL) p->palId = SqueakLoadPalette(k->key, k->variant, s->obj.base.roomId);
        p->cell = step->cell;
        p->dx = x;
        p->dy = y;
        link = &p->next;
    }
    // (the parts this composition doesn't have);
    for (p = *link; p != NULL; p = *link) {
        *link = p->next;
        p->owner = NULL;
        p->next = NULL;
    }
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

/* SqueakLoadPalette(); A character's or effect's palette slot: its anim ID is the key (the type has none), as NidlLoadPalette does
 * for NiDL's enemies (KatAM keeps 7 enemy palettes a room); the palette is the one of the anim's frame `variant`; */
u8 SqueakLoadPalette(u16 key, u8 variant, u16 roomId) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId == roomId) {
            sub_0803DFAC(key, variant);
            palId = sub_0803DF24(key);
        } else {
            palId = 0;
        }
    }
    return palId;
}

extern u16 gUnk_02022E60[];     // The keys of the palettes in KatAM's enemy slots 8-14 (sub_0803DF24, sub_0803DFAC), and how many;
extern u8 gUnk_02022EA0;

/* SqueakTakePalette(); Palette `key` (its anim's frame `variant`) into the slot of palette `take`, when `take` is loaded and `key`
 * isn't: a character's palettes never shown together sharing one slot (Dark Nebula's forms: KatAM keeps 7 a room, and loads past
 * them into the last, over the palette there); its slot, else 0xFF (`take` not loaded: none taken, SqueakLoadPalette's to load it); */
u8 SqueakTakePalette(u16 key, u8 variant, u16 take, u16 roomId) {
    struct Sprite sprite;
    u8 i;

    if (gKirbys[gLocalPlayerId].base.roomId != roomId) return 0xFF;
    for (i = 0; i < gUnk_02022EA0; i++)
        if (gUnk_02022E60[i] == key) return i + 8;
    for (i = 0; i < gUnk_02022EA0; i++) {
        if (gUnk_02022E60[i] != take) continue;
        gUnk_02022E60[i] = key;
        SpriteSomething(&sprite, 0x6000000, key & 0xFFF, variant, 0xFF, 0, 0, 0, 0, 0x10, i + 8, 0x80000);
        return i + 8;
    }
    return 0xFF;
}

/* SqueakUsePalette(); One of the character's other palettes (KatAMExpansionSqueak.cs's Palettes: `alternate` 1 the first; 0 its own),
 * the one of its frame after its cells, under a key of its own (its anim ID | alternate << 13: sub_0803DFAC loads anim ID & 0xFFF); */
void SqueakUsePalette(struct SqueakObject *s, u8 alternate) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];

    if (alternate == 0) return;
    s->obj.base.sprite.palId = SqueakLoadPalette(g->animId | alternate << 13, SQUEAK_HURT_VARIANTS + g->cells + alternate - 1,
                                                 s->obj.base.roomId);
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
            ((struct SqueakObject *)obj)->terrain = 4;
            return;
        }
    }
}

/* Squeak Squad's engine: the functions its enemies share, on SqueakObject's fields (speeds 20.12 a frame, Y up, x forward: facing
 * left, KatAM's flag 1, turns it round); */

/* SqueakHoldInStage(); Squeak Squad's flag 0x200 (0x0201355C, KatAM's 0x100 here: SqueakThroughTerrain()): the terrain is met but
 * doesn't stop the character (the push out of it isn't added to its move, nor the snap to the floor); the stage's edges still hold
 * its spot (unless flag 0x20000); */
static void SqueakHoldInStage(struct ObjectBase *base) {
    struct LevelInfo *level = &gCurLevelInfo[base->unk56];

    if (base->x < level->levelMinPosition.x) base->x = level->levelMinPosition.x;
    if (base->x > level->levelMaxPosition.x) base->x = level->levelMaxPosition.x;
    if (base->y < level->levelMinPosition.y) base->y = level->levelMinPosition.y;
    if (base->y > level->levelMaxPosition.y) base->y = level->levelMaxPosition.y;
    base->unk62 = 0;
}

// SqueakThroughTerrain(); Its flag 0x200 set (`through`) or cleared: SqueakMove() takes it through the terrain;
void SqueakThroughTerrain(struct SqueakObject *s, bool32 through) {
    if (through) s->obj.base.flags |= 0x100;
    else s->obj.base.flags &= ~0x100;
}

/* SqueakDrowns(); The characters with the DS enemy base class's shared state in water (their vtable +0x80 0x02021408, every state
 * checking it first; +0x6C its start, 0x02021D5C, +0x70 its frames, 0x02021D30): the enemies that don't swim, not the mid-bosses nor
 * the things characters make (no base class: KatAM's engine drowns those as before, sub_0809D710); */
static bool32 SqueakDrowns(u8 enemy) {
    switch (enemy) {
    case SQUEAK_PERARA: case SQUEAK_CRIMP: case SQUEAK_MAIGA: case SQUEAK_CRET: case SQUEAK_UJA: case SQUEAK_BIO_SPARK:
    case SQUEAK_STARMAN: case SQUEAK_BUBBLE_HEAD: case SQUEAK_GAW_GAW: case SQUEAK_ACCHI: case SQUEAK_BUN: case SQUEAK_METALUN:
    case SQUEAK_SQUEAKER: case SQUEAK_SPINNI: case SQUEAK_DOC: case SQUEAK_STORO: case SQUEAK_DAROACH:
        return !sSqueakCharacters[enemy].swims;
    default:
        return FALSE;
    }
}

/* SqueakDrown(); The DS enemy base class's shared state in water (0x02021D5C, then 0x02021D30 each frame), as KatAM's own drowning
 * (sub_0809D710, sub_0809F988: one of its two hurt looks at random, still, sinking slowly, then defeated with KatAM's poof, its splash
 * KatAM's on going in) with the DS's numbers: it sinks at 0.4 a frame (its +0x90, -0x666: KatAM's 0.25, -0x40) for 42 frames and is
 * defeated on the next (KatAM's 41); the sound it kept stops, as a hit's (0x02013D88). KatAM's engine doesn't drown these (unkC 1,
 * SqueakCreate()), this does, from SqueakMove(): the DS checks at each state's start, the port after each move; */
#define DROWN_FRAMES 42
#define DROWN_SINK (-0x66)
static void SqueakDrownMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    // (each frame: the frame it went in, the rest of its own frame may have shown its own look or stopped it, SqueakFinishFrame());
    obj->unk83 = s->drownLook;
    obj->base.xspeed = 0;
    obj->base.yspeed = DROWN_SINK;
    if (--s->drown == 0) {
        obj->base.xspeed = obj->base.yspeed = 0;
        sub_0809DA30(obj);
        obj->base.flags |= 0x1000;
    }
}

static void SqueakDrown(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (s->loopSound) {
        m4aSongNumStop(SQUEAK_SE(s->loopSound));
        s->loopSound = 0;
    }
    SqueakStop(s);
    s->drown = DROWN_FRAMES + 1;
    obj->base.counter = 0;
    obj->unk83 = s->drownLook = Rand16() & 1 ? gUnk_08351648[obj->type].unk0 : gUnk_08351648[obj->type].unk1;
    obj->unk78 = SqueakDrownMain;
    obj->base.flags &= ~(0x100 | 0x20);
    obj->base.flags |= 0x40;
    obj->base.flags &= ~(0x40000 | 0x10000000 | 0x20000000 | 0x40000000 | 0x80000000);
    obj->base.flags |= 0x8000;
    obj->base.unk5C = obj->base.unk5C & 0x20 ? 32 : 0;
    obj->base.xspeed = 0;
    obj->base.yspeed = DROWN_SINK;
}

/* SqueakMove(); 0x02013D18: acceleration and the caps (0x02013460), friction (0x02013370), then the move on the terrain (0x0201355C),
 * there and then as Squeak Squad's (its states check the ground right after moving): KatAM's own, ObjectMain's after the object's
 * function (position, then sub_0809D8C8's terrain: unk62's floor and walls, unk58's water); SqueakFinishFrame() leaves it nothing
 * to move again; */
void SqueakMove(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    if (s->maxVx > 0) {
        if (s->vx > s->maxVx) s->vx = s->maxVx;
        if (s->vx < -s->maxVx) s->vx = -s->maxVx;
    }
    if (s->maxVy > 0) {
        if (s->vy > s->maxVy) s->vy = s->maxVy;
        if (s->vy < -s->maxVy) s->vy = -s->maxVy;
    }
    if (s->fx > 0) {
        if (s->vx > 0) {
            if ((s->vx -= s->fx) < 0) s->vx = 0;
        } else if (s->vx < 0 && (s->vx += s->fx) > 0) {
            s->vx = 0;
        }
    }
    if (s->fy > 0) {
        if (s->vy > 0) {
            if ((s->vy -= s->fy) < 0) s->vy = 0;
        } else if (s->vy < 0 && (s->vy += s->fy) > 0) {
            s->vy = 0;
        }
    }
    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    if (!(base->flags & 0x800)) {
        base->unk48 = base->x;
        base->unk4C = base->y;
        base->x += base->xspeed;
        base->y -= base->yspeed;
    }
    if (!(base->flags & 0x100)) {
        sub_0809D8C8(base);
        // (the moving platforms and thunder clouds are floors too: squeak/gimmicks.c, SqueakPlatformLanding());
        if (s->vy <= 0 && !(base->unk62 & 4) && SqueakPlatformLanding(base, base->unk4C)) s->vy = 0;
    } else {
        SqueakHoldInStage(base);
    }
    s->terrain = base->unk62;
    s->water = base->unk58;
    s->moved = TRUE;
    if ((s->water & 2) && !s->drown && SqueakDrowns(s->enemy)) SqueakDrown(s);
}

/* SqueakFinishFrame(); After the character's frame: moved already (SqueakMove), ObjectMain's move after its function moves it no more
 * (only by what pushes it, unk98/unk9A); */
void SqueakFinishFrame(struct SqueakObject *s) {
    if (s->moved) s->obj.base.xspeed = s->obj.base.yspeed = 0;
    s->moved = FALSE;
}

// SqueakStop(); 0x02023468: no speed, acceleration, caps or friction;
void SqueakStop(struct SqueakObject *s) {
    s->vx = s->vy = s->ax = s->ay = s->maxVx = s->maxVy = s->fx = s->fy = 0;
    s->obj.base.xspeed = s->obj.base.yspeed = 0;
}

// SqueakCapFall(); 0x02023410: falls no faster than `cap` (negative: down); TRUE once there;
bool32 SqueakCapFall(struct SqueakObject *s, s32 cap) {
    if (s->vy <= cap) {
        s->vy = cap;
        s->ay = 0;
        return TRUE;
    }
    return FALSE;
}

// SqueakSinkTo(); 0x020233EC: sinks at `accel` down to `cap`; TRUE once there;
bool32 SqueakSinkTo(struct SqueakObject *s, s32 accel, s32 cap) {
    if (s->vy <= cap) {
        s->vy = cap;
        s->ay = 0;
        return TRUE;
    }
    s->ay = accel;
    return FALSE;
}

/* SqueakTurnAround(); 0x020132B4: faces the other way, its speed and acceleration across with it; the brain's direction too (its
 * vtable +0x10, 0x02013368: a turn asked for, -1, is done), and the walls ahead and behind (flags 0x800 and 0x1000) swap; */
void SqueakTurnAround(struct SqueakObject *s) {
    u8 walls = s->terrain;

    s->obj.base.flags ^= 1;
    s->padDir = -s->padDir;
    s->vx = -s->vx;
    s->ax = -s->ax;
    s->terrain = (walls & ~3) | (walls & 1) << 1 | (walls & 2) >> 1;
}

// SqueakOnGround(); 0x84 (its distance down to the floor, from its last move) under 1;
bool32 SqueakOnGround(struct SqueakObject *s) {
    return (s->terrain & 4) && s->vy <= 0;
}

// SqueakWallAhead(); Squeak Squad's flag 0x800 (0x0201355C): its last move met a wall in front of it (KatAM's unk62 1);
bool32 SqueakWallAhead(struct SqueakObject *s) {
    return s->terrain & 1;
}

// SqueakCeiling(); Squeak Squad's flag 0x2000 (0x0201355C): its last move met a ceiling (KatAM's unk62 8);
bool32 SqueakCeiling(struct SqueakObject *s) {
    return s->terrain & 8;
}

// SqueakInWater(); Squeak Squad's flag 0x200000: in water at its last move (KatAM's unk58 2, as its Blipper reads it);
bool32 SqueakInWater(struct SqueakObject *s) {
    return s->water & 2;
}

/* SqueakFaceKirby(); 0x0206FF08 (each enemy has its own copy: Gold Waddle Dee's 0x020B6918): Kirby more than `distance` behind it,
 * its pad turns it round; within `distance` across, no direction; ahead, the direction stays as it was; */
void SqueakFaceKirby(struct SqueakObject *s, s32 distance) {
    struct Object *obj = &s->obj;
    s32 kirby = SQ_X(obj->kirby3), x = SQ_X(obj);

    if (kirby > x + distance) {
        if (obj->base.flags & 1) s->padDir = -1;
    } else if (kirby < x - distance) {
        if (!(obj->base.flags & 1)) s->padDir = -1;
    } else {
        s->padDir = 0;
    }
}

// SqueakKirbyWithin(); 0x0206FF94 / 0x0206FF64: the closest Kirby within (dx, dy) of it (dy 0: any height);
bool32 SqueakKirbyWithin(struct SqueakObject *s, s32 dx, s32 dy) {
    struct Object *obj = &s->obj;
    s32 x = SQ_X(obj->kirby3) - SQ_X(obj), y = SQ_Y(obj->kirby3) - SQ_Y(obj);

    if (x <= -dx || x >= dx) return FALSE;
    return dy == 0 || (y > -dy && y < dy);
}

/* Squeak Squad's effects (0x02021900, its effect manager 0x02039B28): an effect object playing an effect's animation once where it
 * was spawned, facing as its character, then gone; */
#define SQUEAK_FX_ROOM 48     // Tiles of KatAM's sprite heap an effect leaves free (KatAM's own effects: its stars, its abilities');

#define SQUEAK_DRIFT_MARGIN 16
#define SQUEAK_DRIFT_LIFE 255
struct SqueakFx {
    struct EffectObject e;
    struct SqueakObject *owner; // Followed (0x020218C0: its position plus the offset each frame), till it's gone;
    s32 dx, dy;
    u8 effect;
    u8 step;
    u16 timer;
    bool8 left;
    u8 palId;
    bool8 held;                 // Kept on its last frame (or looping) while its character holds it (SqueakHoldEffect);
    bool8 seen;                 // A drifting one (SqueakDriftInRoom: no owner, dx and dy its speed): it's been on the screen;
    u8 life;                    // and the frames it has left;
    /* A falling one (SqueakFallInRoom, Squeak Squad's 0x0211C9A8): its speed (Squeak Squad's 20.12, Y up; fracX and fracY the bits
     * KatAM's 24.8 spot drops) and its fall a frame; under `floor` it bounces (its speed up, `gain` times it, 20.12) while it has
     * bounces left, each sounding bounceSfx; */
    bool8 falling;
    u8 bounces;
    u8 fracX, fracY;
    u16 bounceSfx;
    s32 vx, vy, ay, gain, floor;
};

// SqueakFxDestroy(); An effect's end (its own, or the room's): its character lets go of it;
static void SqueakFxDestroy(struct Task *t) {
    struct SqueakFx *fx = TaskGetStructPtr(t);

    if (fx->owner && fx->owner->follower == fx) fx->owner->follower = NULL;
    ObjectBaseDestroy(t);
}

// FallStep(); A falling effect's step: its speed (20.12, Y up), the 4 bits KatAM's 24.8 spot drops kept;
static void FallStep(struct SqueakFx *fx) {
    s32 dx = fx->vx + fx->fracX, dy = fx->vy + fx->fracY;

    fx->e.x += dx >> 4;
    fx->e.y -= dy >> 4;
    fx->fracX = dx & 0xF;
    fx->fracY = dy & 0xF;
}

static void SqueakFxMain(void) {
    struct SqueakFx *fx = TaskGetStructPtr(gCurTask);
    const struct SqueakAnim *a = &gSqueakData.effects[fx->effect].anim;
    const struct SqueakStep *step;

    if (fx->e.flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (fx->held && fx->owner == NULL) {
        fx->e.flags |= 0x1000;
        return;
    }
    if (fx->timer && --fx->timer == 0) {
        if (fx->step + 1 >= a->steps && !a->loops) {
            if (fx->held) {
                fx->timer = 1;
                goto shown;
            }
            fx->e.flags |= 0x1000;
            return;
        }
        fx->step = fx->step + 1 >= a->steps ? 0 : fx->step + 1;
        fx->timer = a->step[fx->step].frames;
    }
    if (fx->owner) {
        fx->e.x = fx->owner->obj.base.x + SQUEAK_SPEED(fx->dx);
        fx->e.y = fx->owner->obj.base.y - SQUEAK_SPEED(fx->dy);
    } else if (fx->falling || fx->dx || fx->dy) {
        const struct LevelInfo *level = &gCurLevelInfo[gLocalPlayerId];
        s32 x, y;
        bool32 shownNow;

        if (fx->falling) {
            // (Squeak Squad's order: its step, its fall, then under the floor its bounce and a step more);
            FallStep(fx);
            fx->vy += fx->ay;
            if (fx->bounces && fx->e.y > fx->floor) {
                fx->bounces--;
                fx->vy = (-fx->vy * fx->gain + 0x800) >> 12;      // (its speeds a few pixels a frame: no overflow);
                FallStep(fx);
                if (gKirbys[gLocalPlayerId].base.roomId == fx->e.roomId) m4aSongNumStart(SQUEAK_SE(fx->bounceSfx));
            }
        } else {
            fx->e.x += fx->dx;
            fx->e.y += fx->dy;
        }
        x = (fx->e.x - level->viewportPosition.x) >> 8;
        y = (fx->e.y - level->viewportPosition.y) >> 8;
        shownNow = x > -SQUEAK_DRIFT_MARGIN && x < DISPLAY_WIDTH + SQUEAK_DRIFT_MARGIN && y > -SQUEAK_DRIFT_MARGIN && y < DISPLAY_HEIGHT + SQUEAK_DRIFT_MARGIN;
        if (shownNow) fx->seen = TRUE;
        if (fx->life == 0 || --fx->life == 0 || (fx->seen && !shownNow)) {
            fx->e.flags |= 0x1000;
            return;
        }
    }
shown:
    step = &a->step[fx->step];
    // (a step showing nothing: SQUEAK_FX_HIDDEN);
    if (step->cell == SQUEAK_FX_HIDDEN) {
        fx->e.flags |= 0x400;
        return;
    }
    fx->e.sprite.variant = step->cell;
    fx->e.sprite.palId = fx->palId;
    // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right, flipped when facing left);
    fx->e.flags = (fx->e.flags & ~0x401) | 4 | (fx->left ? 0 : 1);
    fx->e.objBase54 = fx->left ? -step->x : step->x;
    fx->e.objBase55 = step->y;
    EffectObjectPostUpdate(&fx->e);
}

// SqueakEffectAt(); An effect at (x, y) (KatAM's units) in a room, facing left or right, playing from its first step;
static struct SqueakFx *SqueakEffectAtIn(u16 roomId, u8 effect, s32 x, s32 y, bool8 left) {
    const struct SqueakEffect *d = &gSqueakData.effects[effect];
    struct Task *t;
    struct SqueakFx *fx;

    if (gKirbys[gLocalPlayerId].base.roomId != roomId) return NULL;
    /* (none when KatAM's sprite heap couldn't hold it and still SQUEAK_FX_ROOM tiles for KatAM's own effects, which draw whatever is
     * in VRAM when they get none: Yadgaine's fight, its shell in, two of its rocks' bursts at once fill it); */
    if (VramGetTotalAllocatedTiles() + d->tiles + SQUEAK_FX_ROOM > gVramHeapMaxTileSlots) return NULL;
    t = TaskCreate(SqueakFxMain, sizeof(struct SqueakFx), 0x3500, TASK_USE_EWRAM, SqueakFxDestroy);
    if (t == NULL) return NULL;
    fx = TaskGetStructPtr(t);
    ClearEffectObject(&fx->e);
    fx->e.header.kind = 3;
    fx->e.roomId = roomId;
    fx->e.parent = NULL;
    fx->e.x = x;
    fx->e.y = y;
    fx->owner = NULL;
    fx->dx = fx->dy = 0;
    fx->effect = effect;
    fx->step = 0;
    fx->timer = d->anim.step[0].frames;
    fx->left = left;
    fx->held = FALSE;
    fx->seen = FALSE;
    fx->life = 0;
    fx->falling = FALSE;
    fx->palId = SqueakLoadPalette(d->palette.key, d->palette.variant, roomId);
    EffectObjectInitSprite(&fx->e, &fx->e.sprite, d->tiles, d->animId,
                           d->anim.step[0].cell == SQUEAK_FX_HIDDEN ? 0 : d->anim.step[0].cell, 0x1A);
    // (none drawn when its VRAM couldn't be got: gone at its first run);
    if (!SqueakVramOk(fx->e.sprite.tilesVram)) fx->e.flags |= 0x1000 | 0x400;
    return fx;
}

static struct SqueakFx *SqueakEffectAt(struct SqueakObject *s, u8 effect, s32 x, s32 y, bool8 left) {
    return SqueakEffectAtIn(s->obj.base.roomId, effect, x, y, left);
}

// SqueakEffectInRoom(); An effect at (x, y) (KatAM's units) in a room, facing right, owned by nothing (a gimmick's: squeak/gimmicks.c);
void SqueakEffectInRoom(u16 roomId, u8 effect, s32 x, s32 y) {
    SqueakEffectAtIn(roomId, effect, x, y, FALSE);
}

/* SqueakDriftInRoom(); An effect at (x, y) in a room moving (dx, dy) a frame (KatAM's units), owned by nothing (gimmick 0x18's streaks:
 * squeak/gimmicks.c's Streaks): it ends once it's been on the screen (SQUEAK_DRIFT_MARGIN pixels round it) and left it, or after
 * SQUEAK_DRIFT_LIFE frames; */
void SqueakDriftInRoom(u16 roomId, u8 effect, s32 x, s32 y, s32 dx, s32 dy) {
    struct SqueakFx *fx = SqueakEffectAtIn(roomId, effect, x, y, FALSE);

    if (fx == NULL) return;
    fx->dx = dx;
    fx->dy = dy;
    fx->life = SQUEAK_DRIFT_LIFE;
}

/* SqueakFallInRoom(); An effect at (x, y) in a room (KatAM's units) thrown at (vx, vy) and falling ay a frame (Squeak Squad's units:
 * 20.12, Y up; 0x0211C9A8's), bouncing `bounces` times under `floor` (its speed up `gain` times its fall, 20.12; sound `sfx`, the DS's
 * id), owned by nothing: it ends as a drifting one (SqueakDriftInRoom), once it's been on the screen and left it; */
void SqueakFallInRoom(u16 roomId, u8 effect, s32 x, s32 y, s32 vx, s32 vy, s32 ay, s32 floor, u8 bounces, s32 gain, u16 sfx) {
    struct SqueakFx *fx = SqueakEffectAtIn(roomId, effect, x, y, FALSE);

    if (fx == NULL) return;
    fx->falling = TRUE;
    fx->vx = vx;
    fx->vy = vy;
    fx->ay = ay;
    fx->fracX = fx->fracY = 0;
    fx->floor = floor;
    fx->bounces = bounces;
    fx->gain = gain;
    fx->bounceSfx = sfx;
    fx->life = SQUEAK_DRIFT_LIFE;
}

/* SqueakSpawnEffect(); 0x02021900: an effect at the character's position plus (dx, dy) (Squeak Squad's units: 20.12, Y up; not
 * turned with the character, as Squeak Squad's), facing as the character; `follow`: it stays there as the character moves (its last
 * parameter, 0x020218C0); */
void SqueakSpawnEffect(struct SqueakObject *s, u8 effect, s32 dx, s32 dy, bool8 follow) {
    struct SqueakFx *fx = SqueakEffectAt(s, effect, s->obj.base.x + SQUEAK_SPEED(dx), s->obj.base.y - SQUEAK_SPEED(dy), s->obj.base.flags & 1);

    if (fx == NULL) return;
    fx->dx = dx;
    fx->dy = dy;
    if (follow) {
        if (s->follower) ((struct SqueakFx *)s->follower)->owner = NULL;
        fx->owner = s;
        s->follower = fx;
    }
}

/* SqueakHoldEffect(); An effect following the character at (dx, dy) as SqueakSpawnEffect's, kept (on its last frame, or looping)
 * till SqueakEndHeldEffect() or the character's end: a composite animation's part held after its frames (Doc's); */
void SqueakHoldEffect(struct SqueakObject *s, u8 effect, s32 dx, s32 dy) {
    SqueakSpawnEffect(s, effect, dx, dy, TRUE);
    if (s->follower && ((struct SqueakFx *)s->follower)->effect == effect) ((struct SqueakFx *)s->follower)->held = TRUE;
}

// SqueakEndHeldEffect(); The effect the character holds, gone;
void SqueakEndHeldEffect(struct SqueakObject *s) {
    struct SqueakFx *fx = s->follower;

    if (fx == NULL) return;
    fx->owner = NULL;
    fx->e.flags |= 0x1000;
    s->follower = NULL;
}

// SqueakWaterTile(); A tile (16 pixels square) of the character's room is water (KatAM's collision attribute 2), inside the level;
static bool32 SqueakWaterTile(struct Object *obj, s32 tx, s32 ty) {
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];

    if (tx < 0 || ty < 0 || tx << 12 > level->levelMaxPosition.x || ty << 12 > level->levelMaxPosition.y) return FALSE;
    return (gCollisionAttributes[GetCollisionTile(obj->base.unk56, tx, ty)] & 2) != 0;
}

/* SqueakSplash(); The stage's splash where the character is (its vtable +0xB4, 0x02215A10: type 0 into the water, 1 out of it): only
 * at the surface (0x02215860: its tile water and one beside, above or below it not, or the other way round); placed on it
 * (0x02215954: 6 pixels into the top water tile; the bottom of a water tile with no water below; the top of a dry tile under water),
 * across where the character is; facing right (the effect manager, 0x02039B28, isn't told a facing). Not ported: Squeak Squad
 * spawns one at most every 6 frames among all splashes (its counter in the stage's effects, 0x0221BCA0), Kirby's too; */
void SqueakSplash(struct SqueakObject *s, bool8 out) {
    struct Object *obj = &s->obj;
    s32 tx = obj->base.x >> 12, ty = obj->base.y >> 12, y = obj->base.y;
    bool32 here = SqueakWaterTile(obj, tx, ty), up = SqueakWaterTile(obj, tx, ty - 1), down = SqueakWaterTile(obj, tx, ty + 1),
           side = SqueakWaterTile(obj, tx - 1, ty) + SqueakWaterTile(obj, tx + 1, ty);

    if (here ? up && down && side == 2 : !up && !down && side == 0) return;
    if (here) {
        if (!up) y = (ty * 16 + 6) << 8;
        else if (!down) y = (ty * 16 + 16) << 8;
    } else if (up) {
        y = ty * 16 << 8;
    }
    SqueakEffectAt(s, out ? SQUEAK_FX_SPLASH_OUT : SQUEAK_FX_SPLASH_IN, obj->base.x, y, FALSE);
}

/* SqueakTouches(); The stage's vtable +0x18 (0x0221022C): the character's box (KatAM's bounds), `dy` up from where it is (Squeak
 * Squad's units), touches something solid on that side: a line just past the box's edge (up: one unit above it; down: its bottom
 * edge; left, right: one unit beside it, its bottom raised a unit when the character's spot is solid and its box starts at its feet),
 * either end solid (`both`: both ends, its mode 2); */
bool32 SqueakTouches(struct SqueakObject *s, u8 side, s32 dy, bool8 both) {
    struct ObjectBase *b = &s->obj.base;
    s32 x = b->x, y = b->y - SQUEAK_SPEED(dy);
    s32 left = x + b->unk3C * 0x100, top = y + b->unk3D * 0x100, right = x + b->unk3E * 0x100, bottom = y + b->unk3F * 0x100;
    s32 x0 = left, y0 = bottom, x1 = right, y1 = bottom;
    bool32 a, c;

    switch (side) {
    case SQUEAK_UP:
        y0 = y1 = top - 1;
        break;
    case SQUEAK_DOWN:
        break;
    case SQUEAK_LEFT:
    case SQUEAK_RIGHT:
        x0 = x1 = side == SQUEAK_LEFT ? left - 1 : right + 1;
        y1 = top;
        if (b->unk3F == 0 && NidlIsFloor(&s->obj, x, y)) y0--;
        break;
    }
    a = NidlIsFloor(&s->obj, x0, y0);
    c = NidlIsFloor(&s->obj, x1, y1);
    return both ? a && c : a || c;
}

/* SqueakSpawn(); Another character (`enemy`, `variant`) at (dx, dy) from this one (Squeak Squad's units, dx forward), facing as it
 * (Squeak Squad's turn the new one round when the thrower faces left), with its own template copy: its room slot none (31), so it
 * doesn't count as the room's; */
struct SqueakObject *SqueakSpawn(struct SqueakObject *s, u8 enemy, u8 variant, s32 dx, s32 dy) {
    struct ObjectTemplate template;
    struct SqueakObject *spawned;

    if (!s->obj.objTemplate || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return NULL;
    template = *s->obj.objTemplate;
    template.x = (s->obj.base.x + SQUEAK_SPEED(s->obj.base.flags & 1 ? -dx : dx)) >> 8;
    template.y = (s->obj.base.y - SQUEAK_SPEED(dy)) >> 8;
    template.type = SQUEAK_TYPE;
    template.subtype1 = enemy;
    template.unk12 = variant;
    template.unk2 = 0;
    template.unk3 = 31;
    template.unk11 = 0;     // (not a boss's shard: SqueakBossShard);
    spawned = CreateSqueakEnemy(&template, s->obj.base.unk56);
    if (spawned == NULL) return NULL;
    spawned->spawn = template;
    spawned->obj.objTemplate = &spawned->spawn;
    spawned->obj.base.x = s->obj.base.x + SQUEAK_SPEED(s->obj.base.flags & 1 ? -dx : dx);
    spawned->obj.base.y = s->obj.base.y - SQUEAK_SPEED(dy);
    spawned->obj.base.flags = (spawned->obj.base.flags & ~1) | (s->obj.base.flags & 1);
    return spawned;
}

// SqueakInLevel(); 0x02209D00: the character is inside the stage (KatAM's level bounds);
bool32 SqueakInLevel(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];

    return s->obj.base.x >= level->levelMinPosition.x && s->obj.base.x <= level->levelMaxPosition.x
           && s->obj.base.y >= level->levelMinPosition.y && s->obj.base.y <= level->levelMaxPosition.y;
}

// SqueakBoxAt(); A box of the character's (KatAM's: x ahead, y down, pixels from its spot) in the room, as it faces;
static void SqueakBoxAt(struct SqueakObject *s, const s8 *b, s32 *r) {
    s32 x = s->obj.base.x, y = s->obj.base.y;

    if (s->obj.base.flags & 1) {
        r[0] = x - b[2] * 0x100;
        r[2] = x - b[0] * 0x100;
    } else {
        r[0] = x + b[0] * 0x100;
        r[2] = x + b[2] * 0x100;
    }
    r[1] = y + b[1] * 0x100;
    r[3] = y + b[3] * 0x100;
}

// SqueakHitBoxAt(); An object's hit box (sub_0803E2B0's: its center's offset and half sizes, turned with its facing) in the room;
static void SqueakHitBoxAt(struct ObjectBase *b, s32 *r) {
    s32 x = b->x + ((b->flags & 1) ? -(b->unk38 + b->unk3A) : b->unk38 + b->unk3A) * 0x100, y = b->y + (b->unk39 + b->unk3B) * 0x100;

    r[0] = x - b->unk3A * 0x100;
    r[2] = x + b->unk3A * 0x100;
    r[1] = y - b->unk3B * 0x100;
    r[3] = y + b->unk3B * 0x100;
}

/* SqueakUntouchable(); Squeak Squad's 0x0201322C (an object no attack takes: its hit box flagged 0x8000, as the .rca's in some
 * animations): KatAM's hit level held at 6 (as Gordo's: no attack hurts it, nor inhales it; its touch still hurts), or given back; */
void SqueakUntouchable(struct SqueakObject *s, bool32 untouchable) {
    struct ObjectBase *base = &s->obj.base;

    if (untouchable) {
        if (!s->level) s->level = (base->unk5C & 7) | 0x80;
        base->unk5C = (base->unk5C & ~7) | 6;
    } else if (s->level) {
        base->unk5C = (base->unk5C & ~7) | (s->level & 7);
        s->level = 0;
    }
}

/* SqueakSetBoxes(); This frame's boxes (the .rca's, at its animation's time; Squeak Squad's LoadBoxes, 0x02014010): KatAM's hit box
 * (sub_0803E2B0's: where it's hit, where its touch hurts; KatAM turns it with its facing), its touch's harm on (`harm`, KatAM's unk68
 * from InitObject) or off, `index` the box's; an animation without boxes leaves them as they were; untouchable in the animations
 * whose boxes are flagged (SqueakUntouchable()); */
void SqueakSetBoxes(struct SqueakObject *s, const struct SqueakBoxes *boxes, s16 harm, u8 *index) {
    const struct SqueakBoxAnim *frames;
    const struct SqueakBox *box;
    u32 time = SqueakAnimTime(s), total = 0;
    u8 i;

    SqueakUntouchable(s, s->composite == SQUEAK_NO_COMPOSITE && s->anim < 64 && ((boxes->untouchable[s->anim >> 5] >> (s->anim & 31)) & 1));
    if (s->anim >= boxes->anims) return;
    frames = &boxes->anim[s->anim];
    if (frames->count == 0) return;
    for (i = 0; i < frames->count; i++) total += frames->frame[i].frames;
    if (gSqueakData.enemies[s->enemy].anim[s->anim].loops) time %= total;
    for (i = 0; i < frames->count - 1 && time >= frames->frame[i].frames; i++) time -= frames->frame[i].frames;
    *index = frames->frame[i].box;
    box = &boxes->box[*index];
    sub_0803E2B0(&s->obj.base, box->box[0], box->box[1], box->box[2], box->box[3]);
    s->obj.base.unk68 = box->harms ? harm : 0;
}

/* SqueakKirbyFree(); A Kirby KatAM's grabbers can take (Box Boxer's sub_080C8548, Gobbler's), and not flashing from a hurt (unk5C's
 * 6); */
bool32 SqueakKirbyFree(struct Kirby *kirby) {
    return !kirby->base.header.kind && kirby->hp > 0 && kirby->animationIndex != 0x27 && kirby->animationIndex <= 0x7A
           && !kirby->unk110 && !(kirby->base.flags & 0x3800B00) && (kirby->base.unk5C & 7) < 6;
}

/* SqueakGrabbing(); Squeak Squad's touch reaction of kind 6 (its grabbing boxes touching Kirby): a Kirby free to grab (in its room)
 * whose hit box touches one of `box`'s grabbing boxes; */
struct Kirby *SqueakGrabbing(struct SqueakObject *s, const struct SqueakBox *box) {
    struct Kirby *kirby;
    s32 r[4], h[4];
    u8 i, k;

    if (box->grabs == 0) return NULL;
    for (k = 0; k < gNumKirbys; k++) {
        kirby = &gKirbys[k];
        if (kirby->base.roomId != s->obj.base.roomId || !SqueakKirbyFree(kirby)) continue;
        SqueakHitBoxAt(&kirby->base, h);
        for (i = 0; i < box->grabs; i++) {
            SqueakBoxAt(s, box->grab[i], r);
            if (h[2] >= r[0] && h[0] <= r[2] && h[3] >= r[1] && h[1] <= r[3]) return kirby;
        }
    }
    return NULL;
}

/* SqueakTouchingKirby(); A Kirby (in its room, not beaten) whose hit box touches the character's (sub_0803E2B0's, in whole pixels,
 * edges touching: where KatAM's touch hurts him, as recorded): Squeak Squad's touch reaction to his body's attack (Bubble Head's
 * bubbles pop on it, 0x02096E68's flag 0x400); */
struct Kirby *SqueakTouchingKirby(struct SqueakObject *s) {
    struct Kirby *kirby;
    s32 r[4], h[4];
    u8 k;

    SqueakHitBoxAt(&s->obj.base, r);
    for (k = 0; k < gNumKirbys; k++) {
        kirby = &gKirbys[k];
        if (kirby->base.roomId != s->obj.base.roomId || kirby->hp <= 0) continue;
        SqueakHitBoxAt(&kirby->base, h);
        if (h[2] >> 8 >= r[0] >> 8 && h[0] >> 8 <= r[2] >> 8 && h[3] >> 8 >= r[1] >> 8 && h[1] >> 8 <= r[3] >> 8) return kirby;
    }
    return NULL;
}

/* SqueakBossAlive(); Whether a world's boss (SqueakBossShard) is in the room, not gone yet (beaten, it's still there till it goes);
 * walking the task list as arena.c's MinibossAlive() (objects are ObjectBase kind 1); */
bool32 SqueakBossAlive(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || !ObjType38To52(other) || !SqueakIsMidboss(other)) continue;
        if (other->base.roomId == room && !(other->base.flags & 0x1000) && other->objTemplate && SqueakBossShard(other)) return TRUE;
    }
    return FALSE;
}

void SqueakLoopSfx(struct SqueakObject *s, u16 n) {
    if (s->loopSound) m4aSongNumStop(SQUEAK_SE(s->loopSound));
    s->loopSound = n;
    SqueakSfx(s, n);
}

void SqueakStopSfx(struct SqueakObject *s, u16 n) {
    if (s->loopSound != n) return;
    m4aSongNumStop(SQUEAK_SE(n));
    s->loopSound = 0;
}

/* SqueakDestroy(); A character's end: the effect following it stays where it is, its parts go; a world's boss beaten (no HP left, the
 * player still in its room: not the room's end) leaves its shard (SqueakBossShard), the boss room's way out, as it goes after its
 * defeat, as NiDL's bosses as they explode; the finale's (SQUEAK_FINALE_SHARD) dances every Kirby out on to the finale's next part; */
static void SqueakDestroy(struct Task *t) {
    struct SqueakObject *s = TaskGetStructPtr(t);

    if (s->obj.objTemplate && SqueakBossShard(&s->obj) == SQUEAK_FINALE_SHARD && s->obj.unk80 <= 0 && gKirbys[0].base.roomId == s->obj.base.roomId)
        NidlDanceOut(s->obj.base.roomId);
    else if (s->obj.objTemplate && SqueakBossShard(&s->obj) && s->obj.unk80 <= 0 && gKirbys[0].base.roomId == s->obj.base.roomId) {
        s32 x = s->obj.objTemplate->unk14 ? s->obj.objTemplate->unk14 - (gCurLevelInfo[s->obj.base.unk56].viewportPosition.x >> 8)
                                          : DISPLAY_WIDTH / 2;
        NidlBossShardAt(s->obj.base.unk56, s->obj.base.roomId, SqueakBossShard(&s->obj) - 1, x, SQUEAK_SHARD_Y);
    }

    if (s->loopSound) m4aSongNumStop(SQUEAK_SE(s->loopSound));
    if (s->follower) ((struct SqueakFx *)s->follower)->owner = NULL;
    if (s->layer[0]) ((struct SqueakLayerFx *)s->layer[0])->owner = NULL;
    if (s->layer[1]) ((struct SqueakLayerFx *)s->layer[1])->owner = NULL;
    SqueakReleaseParts(s);
    if (s->enemy == SQUEAK_SQUEAKER) SqueakerGone(s);
    if (s->enemy == SQUEAK_SPINNI) SpinniGone(s);
    if (s->enemy == SQUEAK_DOC) DocGone(s);
    if (s->enemy == SQUEAK_STORO) StoroGone(s);
    if (s->enemy == SQUEAK_DAROACH) DaroachGone(s);
    if (s->enemy == SQUEAK_KING_DEDEDE) SqueakDededeGone(s);
    if (s->enemy == SQUEAK_DARK_NEBULA) DarkNebulaGone(s);
    if (s->enemy == SQUEAK_DARK_NEBULA_AURA) DarkNebulaAuraGone(s);
    if (s->enemy == SQUEAK_YADGAINE) YadgaineGone(s);
    /* (his burning floor's tiles share his one VRAM block: not theirs to let go of, dark_nebula.c; KatAM's mark of a sprite with
     * none, which VramFree leaves be, is what VramMalloc gives when the tiles asked for are more than the heap's: ewram_end); */
    if (s->enemy == SQUEAK_DARK_NEBULA_BURN) s->obj.base.sprite.tilesVram = VramMalloc(0x2000);
    NidlDestroy(t);
}

// (SqueakObject fits the room every character gets);
typedef char SqueakObjectFits[sizeof(struct SqueakObject) <= SQUEAK_OBJECT_SIZE ? 1 : -1];

// SqueakCreate(); A character's object (its start not run yet): InitObject, its fields, HP, init, sprite, own palette, first cell;
static struct SqueakObject *SqueakCreate(struct ObjectTemplate *template, u8 playerId, u8 enemy) {
    struct Task *task = TaskCreate(ObjectMain, SQUEAK_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, SqueakDestroy);
    struct SqueakObject *s = TaskGetStructPtr(task);
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
    s->brain = 0;
    s->started = FALSE;
    s->activated = FALSE;
    s->level = 0;
    s->moved = FALSE;
    s->terrain = s->water = 0;
    s->follower = NULL;
    s->layer[0] = s->layer[1] = NULL;
    s->composite = SQUEAK_NO_COMPOSITE;
    s->parts = NULL;
    s->compositeTime = 0;
    SqueakStop(s);
    s->padDir = s->padY = s->padA = s->padB = 0;
    for (i = 0; i < ARRAY_COUNT(s->vars); i++) s->vars[i] = 0;
    s->obj.unk80 = gSqueakEnemyHp[enemy];
    s->maxHp = s->hpBase = 0;
    s->loopSound = 0;
    s->damage = FALSE;
    s->drown = 0;
    s->chase = (template->unk12 & SQUEAK_CHASE_SPAWN) != 0;
    s->safeX = s->safeY = 0;
    sSqueakCharacters[enemy].init(s);
    // (KatAM's engine drowns what doesn't swim, sub_0809D710; the ones with the DS's own water state do it the DS's way, SqueakDrown());
    if (sSqueakCharacters[enemy].swims || SqueakDrowns(enemy)) s->obj.base.unkC |= 1;
    // (the type's AnimInfo table has one entry: ObjectInitSprite reads it; the patched drawing shows the character's own frames);
    s->obj.unk83 = 0;
    /* (its type's tiles not asked for, ObjectInitSprite made as for a sprite in another room, then its own: VramMalloc's search
     * failing is slow, about a sixth of a frame, KatAM's heap seldom with the type's 123 in one piece in a boss's fight, his
     * things spawned by the dozen: Dark Nebula's burning floor, a lag every other frame); */
    if (gSqueakData.enemies[enemy].tiles > 0 && gKirbys[gLocalPlayerId].base.roomId == s->obj.base.roomId) {
        u16 room = s->obj.base.roomId;

        s->obj.base.roomId = ~room;
        ObjectInitSprite(&s->obj);
        s->obj.base.roomId = room;
        s->obj.base.sprite.unk8 = 0x42000;
        s->obj.base.sprite.unk1B = 0xFF;
        s->obj.base.sprite.unk20[0].unk0 = -1;
    } else {
        ObjectInitSprite(&s->obj);
    }
    /* (its sprite as many tiles as its own frames take, not its type's, the most any character's take: a boss's legs and things
     * spawned by the dozen would fill KatAM's 512-tile heap; the drawing copies a frame's tiles into it, its first part's too. Its
     * own asked for when its type's couldn't be got: the heap without that many in one piece, Yadgaine's rocks with its shell in); */
    if (gSqueakData.enemies[enemy].tiles > 0) {
        if (SqueakVramOk(s->obj.base.sprite.tilesVram)) VramFree(s->obj.base.sprite.tilesVram);
        // (one with its frames' tiles: none till it's drawn, then its frame's, SqueakFitVram);
        if (gSqueakData.cellTiles != NULL && gSqueakData.cellTiles[enemy] != NULL) {
            s->obj.base.sprite.tilesVram = 0;
            s->obj.base.unkC |= 0x200;
        } else {
            s->obj.base.sprite.tilesVram = VramMalloc(gSqueakData.enemies[enemy].tiles);
        }
    }
    if (gSqueakData.enemies[enemy].palette != NULL)
        s->obj.base.sprite.palId = SqueakLoadPalette(gSqueakData.enemies[enemy].palette[0].key, gSqueakData.enemies[enemy].palette[0].variant,
                                                     s->obj.base.roomId);
    else
        s->obj.base.sprite.palId = SqueakLoadPalette(gSqueakData.enemies[enemy].animId, SQUEAK_HURT_VARIANTS, s->obj.base.roomId);
    SqueakSetCell(s, 0);
    // (a chase's Squeaks go on after Kirby off the screen: not taken away out of view, as KatAM's enemies are);
    if (s->chase) s->obj.base.flags |= 0x2000000;
    return s;
}

// CreateSqueakEnemy(); Type 0x2C's spawn: the character the behavior byte asks for (an unknown one, or a mid-boss, the first);
void *CreateSqueakEnemy(struct ObjectTemplate *template, u8 playerId) {
    u8 enemy = template->subtype1;
    struct SqueakObject *s;

    if (enemy >= SQUEAK_ENEMY_COUNT || sSqueakCharacters[enemy].midboss) enemy = 0;
    s = SqueakCreate(template, playerId, enemy);
    SqueakStart(&s->obj);
    return s;
}

/* CreateSqueakMidboss(); A mid-boss (a mini-boss type's spawn, CreateNidlMiniboss, hands its SQUEAK_MIDBOSS behavior bytes over): as
 * NiDL's and Mr. Frosty (0x080CB6E8), water doesn't defeat it, it's untouchable till the fight starts, its touch hurts as a
 * mini-boss's, it faces Kirby; an unknown character, the first mid-boss; */
void *CreateSqueakMidboss(struct ObjectTemplate *template, u8 playerId) {
    u8 enemy = template->subtype1 & ~SQUEAK_MIDBOSS;
    struct SqueakObject *s;

    if (enemy >= SQUEAK_ENEMY_COUNT || !(sSqueakCharacters[enemy].midboss || sSqueakCharacters[enemy].squeak)) enemy = SQUEAK_BIG_METALUN;
    s = SqueakCreate(template, playerId, enemy);
    // (Spinni, Doc and Storo as mini-bosses: their fight with Kirby, as the hideouts' on the DS, in their chase's mood);
    if (sSqueakCharacters[enemy].squeak) s->chase = TRUE;
    s->obj.base.unkC |= 1;
    s->obj.base.flags |= 0x2000000 | 0x200 | 0x200000;
    s->obj.base.unk5C = (s->obj.base.unk5C & ~7) | 3 | 0xA0;
    s->obj.base.sprite.unk14 = 0x6C0;
    NidlFaceKirby(&s->obj);
    SqueakStart(&s->obj);
    return s;
}

// SqueakStart(); The type's behavior start (the game calls it again after a hit);
/* SqueakBossMusic(); A world's boss's fight start (Macro_08100F18 but its music): its HP bar shows, the room's music is saved as
 * KatAM's mini-boss music saves it (restored after the fight), and the arena's own music plays on: Squeak Squad's boss theme, the
 * arena's map's (KatAMExpansionSqueakSound.cs's SqueakRoomSong: 21, 33 or 34), not KatAM's mini-boss music; */
static void SqueakBossMusic(struct Object *obj) {
    struct Object5 *bar = sub_08034E14(obj);
    u8 slot = gCurLevelInfo[obj->base.unk56].unk65E;

    if (bar) bar->unk9 = 0;
    sub_08002A44(slot, sub_08002A2C(slot, 0), 1);
    sub_08002A44(slot, GetRoomMusicId(slot), 0);
}

/* SqueakMidbossWait(); A mid-boss before its fight (Mr. Frosty's 0x080CB7E8, as NiDL's: NidlMinibossWait): its animation going,
 * falling as the engine would, facing Kirby till he's in the template's area; then the camera stays there, the HP bar shows, the
 * mini-boss music plays (a world's boss: its arena's own, SqueakBossMusic) and it starts; */
static void SqueakMidbossWait(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->base.flags |= 4;
    SqueakAnimate(s);
    NidlFall(obj);
    obj->kirby3 = FindTargetKirby(&obj->base);
    if ((obj->kirby3->base.unkC & 0x8000) || obj->base.roomId != obj->kirby3->base.roomId) return;
    NidlFaceKirby(obj);
    if (!Macro_08039430_2(&obj->kirby3->base, obj)) return;
    Macro_081003EC(obj, &obj->kirby3->base);
    obj->base.flags &= ~0x200;
    if (obj->objTemplate && SqueakBossShard(obj)) SqueakBossMusic(obj);
    else Macro_08100F18(obj);
    s->activated = TRUE;
    SqueakStart(obj);
}

/* SqueakMidbossUpdate(); A mid-boss in its fight stays in the area the camera is locked to (as NiDL's, NidlUpdate, and KatAM's own:
 * ObjXSomething), called first each frame; */
void SqueakMidbossUpdate(struct SqueakObject *s) {
    if (s->activated && s->obj.unkA8 > s->obj.unkA4) ObjXSomething(&s->obj);
}

/* SqueakBossFloor(); A world's boss's floor, the height its moves are measured from (each one's FLOOR: Kirby's floor in its own stage):
 * where it's placed in its own boss room; on a mini-boss spot of KatAM's or NiDL's rooms (the mini-boss randomization's), 16 pixels up
 * from the bottom of its area (its template's: the screen the fight is held to), as its own arena's floor: KatAM's mini-bosses stand
 * anywhere in theirs (the Meta-Knights' high up), and the flying ones' moves from there left them above the screen; in a Squeak Lair
 * (SQUEAK_LAIR_FIGHT), the floor Kirby lands on, LAIR_UNDER_FLOOR over its area's bottom (its spot is its Squeak's, maybe a ledge). In
 * its own room, the first floor under its spot, however far: the DS places the flying ones in the air (Dark Daroach 92 pixels over
 * a8s4s2's floor, Dark Nebula's star too), past SqueakStandOnFloor()'s 5 cells, and their moves measured from the spot itself flew
 * them off the screen's top (the owner: "The 1st phase of the Squeak Squad boss goes out of the screen"); */
#define BOSS_FLOOR_UP 16
s32 SqueakBossFloor(struct SqueakObject *s) {
    const struct ObjectTemplate *t = s->obj.objTemplate;

    if (t == NULL || t->unk20 == 0) return s->obj.base.y;
    if (t->unk22 & SQUEAK_LAIR_FIGHT) return (t->y + t->unk1C + t->unk20 - LAIR_UNDER_FLOOR) << 8;
    if (s->obj.base.roomId >= SQUEAK_FIRST_ROOM) {
        s32 y = s->obj.base.y & ~0xFFF, bottom = gCurLevelInfo[s->obj.base.unk56].levelMaxPosition.y;

        for (; y <= bottom; y += 0x1000)
            if (NidlIsFloor(&s->obj, s->obj.base.x, y + 0x1000)) return y + 0x1000 - 1;
        return s->obj.base.y;
    }
    return (t->y + t->unk1C + t->unk20 - BOSS_FLOOR_UP) << 8;
}

/* FloorBelow(); A floor in the column at x from y down, `cells` cells (0: down to the room's bottom); */
#define PIT_DEPTH 6
static bool32 FloorBelow(struct Object *obj, s32 x, s32 y, u8 cells) {
    s32 bottom = gCurLevelInfo[obj->base.unk56].levelMaxPosition.y;
    u8 d;

    for (d = 0; (cells == 0 || d < cells) && y <= bottom; d++, y += 0x1000)
        if (NidlIsFloor(obj, x, y)) return TRUE;
    return FALSE;
}

/* SqueakPitAhead(); On the ground, no floor under one of the next `cells` cells ahead of its feet down to PIT_DEPTH cells: a pit (the
 * void, or a drop it couldn't climb back from) its next steps go into; */
bool32 SqueakPitAhead(struct SqueakObject *s, u8 cells) {
    struct Object *obj = &s->obj;
    s32 ahead = obj->base.flags & 1 ? -0x1000 : 0x1000, x;
    u8 c;

    if (!SqueakOnGround(s)) return FALSE;
    for (c = 1; c <= cells; c++) {
        x = obj->base.x + ahead * c;
        if (x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x || x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x) return FALSE;
        if (!FloorBelow(obj, x, obj->base.y + 0x100, PIT_DEPTH)) return TRUE;
    }
    return FALSE;
}

/* SqueakJumpLands(); Its jump from where it stands, forward vx and up vy, gravity g (Squeak Squad's units a frame, its fall capped at
 * JUMP_FALL_CAP), followed through the room (stopped across by walls): coming down, it meets a floor no more than PIT_DEPTH cells
 * under its feet (TRUE), or not (the void, a drop too deep, the room's edge: FALSE); */
#define JUMP_FALL_CAP -0x2800
#define JUMP_FRAMES 120
#define JUMP_WALL 0x200                 // (gCollisionAttributes' solid bit: a wall stops it across, a thin floor doesn't);
#define JUMP_MIDDLE 0x800               // (its body's middle, 8 pixels over its feet: what meets a wall);
bool32 SqueakJumpLands(struct SqueakObject *s, s32 vx, s32 vy, s32 g) {
    struct Object *obj = &s->obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 x = obj->base.x, y = obj->base.y, was, deepest = obj->base.y + PIT_DEPTH * 0x1000, t;

    if (obj->base.flags & 1) vx = -vx;
    for (t = 0; t < JUMP_FRAMES; t++) {
        was = y;
        // (against a wall it stops across, falling down it as the engine has it);
        if (vx != 0 && x + (vx >> 4) >= level->levelMinPosition.x && x + (vx >> 4) <= level->levelMaxPosition.x
            && (gCollisionAttributes[GetCollisionTile(obj->base.unk56, (x + (vx >> 4)) >> 12, (y - JUMP_MIDDLE) >> 12)] & JUMP_WALL))
            vx = 0;
        x += vx >> 4;
        y -= vy >> 4;
        vy -= g;
        if (vy < JUMP_FALL_CAP) vy = JUMP_FALL_CAP;
        if (x < level->levelMinPosition.x || x > level->levelMaxPosition.x || y > deepest || y > level->levelMaxPosition.y) return FALSE;
        // (coming down onto a floor from the open: its feet were clear the frame before; into a wall's side isn't a landing);
        if (vy < 0 && NidlIsFloor(obj, x, y + 0x100) && !NidlIsFloor(obj, x, was)) return TRUE;
    }
    return FALSE;
}

/* SqueakPit(); What a Squeak does at a pit within `cells` cells ahead (SqueakPitAhead; 0: none looked for): its jump (forward vx, up
 * vy, gravity g) lands past it, a jump (SQUEAK_PIT_LEAP); it doesn't, but the pit has a floor down in it and Kirby is lower, down to
 * him (SQUEAK_PIT_DROP); else it waits on its side (SQUEAK_PIT_WAIT: the void, or a drop away from Kirby, too wide to cross); */
u8 SqueakPit(struct SqueakObject *s, u8 cells, s32 vx, s32 vy, s32 g) {
    if (cells == 0 || !SqueakPitAhead(s, cells)) return SQUEAK_PIT_NONE;
    if (SqueakJumpLands(s, vx, vy, g)) return SQUEAK_PIT_LEAP;
    if (SqueakDropToKirby(s, cells)) return SQUEAK_PIT_DROP;
    return SQUEAK_PIT_WAIT;
}

/* SqueakDropToKirby(); Kirby lower than it (two cells or more) and a floor at the bottom of the pit within `cells` cells ahead: going
 * down to him; */
bool32 SqueakDropToKirby(struct SqueakObject *s, u8 cells) {
    struct Object *obj = &s->obj;
    s32 ahead = obj->base.flags & 1 ? -0x1000 : 0x1000, x;
    u8 c;

    if (obj->kirby3 == NULL || obj->kirby3->base.y <= obj->base.y + 0x2000) return FALSE;
    for (c = 1; c <= cells; c++) {
        x = obj->base.x + ahead * c;
        if (x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x || x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x) return FALSE;
        if (!FloorBelow(obj, x, obj->base.y + 0x100, PIT_DEPTH)) return FloorBelow(obj, x, obj->base.y + 0x100, 0);
    }
    return FALSE;
}

/* SqueakChaseSteer(); A Squeak in a chase (SqueakChasing), after its brain set its pad: on the ground going ahead with a pit within
 * `cells` cells (SqueakPit, its jump's vx, vy, g), it jumps (A with up), goes down to Kirby, or waits on its side (0 cells: the
 * character steers itself); the last floor it stood on kept, and fallen to the room's bottom (the void: the stage's edge holds it
 * there), it's put back on it, dropping onto it (its fall's own landing: a fall's speed capped, its gravity off, would hold it in the
 * air); */
void SqueakChaseSteer(struct SqueakObject *s, u8 cells, s32 vx, s32 vy, s32 g) {
    struct ObjectBase *base = &s->obj.base;

    if (!s->chase) return;
    if (SqueakOnGround(s) && !SqueakPitAhead(s, 1)) {
        s->safeX = base->x >> 8;
        s->safeY = base->y >> 8;
    }
    if (s->padDir == 1 && s->padA == 0) {
        switch (SqueakPit(s, cells, vx, vy, g)) {
        case SQUEAK_PIT_LEAP:
            s->padA = 1;
            s->padY = 1;
            break;
        case SQUEAK_PIT_WAIT:
            s->padDir = 0;
            s->padY = 0;
            break;
        }
    }
    if (s->safeX && base->y >= gCurLevelInfo[base->unk56].levelMaxPosition.y - 0x1000 && !SqueakOnGround(s)) {
        base->x = s->safeX << 8;
        base->y = s->safeY << 8;
        s->vx = s->ax = s->ay = 0;
        s->vy = -0x1000;
    }
}

void SqueakStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    // (a hit starts it over: the sound it kept stops with the move it was in, as the DS's hurt stops them, 0x02013D88);
    if (s->started && s->loopSound) {
        m4aSongNumStop(SQUEAK_SE(s->loopSound));
        s->loopSound = 0;
    }
    obj->kirbyAbility = sSqueakCharacters[s->enemy].ability;
    // (Squeak Squad's abilities KatAM lacks need NiDL's ability code and their looks: without them, none, gSqueakAbilityBodies 0);
    if (obj->kirbyAbility >= SQUEAK_FIRST_ABILITY && gSqueakAbilityBodies[obj->kirbyAbility - SQUEAK_FIRST_ABILITY] == NULL)
        obj->kirbyAbility = KIRBY_ABILITY_NORMAL;
    // The engine's gravity off (flag 0x40, and 0x20, which keeps it on once falling): Squeak Squad's characters have their own;
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    if (sSqueakCharacters[s->enemy].midboss && !s->activated) {
        ObjectSetFunc(obj, -1, SqueakMidbossWait);
        obj->base.xspeed = 0;
        return;
    }
    sSqueakCharacters[s->enemy].start(obj);
    s->started = TRUE;
}

// SqueakName(); A character's HP bar name (NidlCopyName shows it);
const u8 *SqueakName(struct Object *obj) {
    return gSqueakData.names + ((struct SqueakObject *)obj)->enemy * 0x100;
}

// SqueakMaxHp(); A character's full HP bar (its table's HP at the start, the randomizer's HP options may have changed; or its own);
s16 SqueakMaxHp(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    return s->maxHp > 0 ? s->maxHp : gSqueakEnemyHp[s->enemy];
}

// SqueakHpBase(); The HP under its bar's empty (a next form's, held back);
s16 SqueakHpBase(struct Object *obj) {
    return ((struct SqueakObject *)obj)->hpBase;
}
