/* NiDL bosses; Object types 0x43 and 0x44 (OBJ_EMPTY_43 and 44, which KatAM already treats as a boss everywhere: the boss music, its HP bar, a hit
 * only flashes it, its defeat: it freezes, explodes and goes, then gUnk_08350E58[type - 0x43] runs, here NidlBossDefeat): creates the
 * boss the template's behavior byte asks for (high nibble = boss, NIDL_* in nidl.h; low nibble = the mirror shard it leaves, 0-7),
 * and starts the fight as the mini-bosses do (nidl_miniboss.c): it waits, untouchable, until Kirby is in the template's area (the
 * boss's screen, held by an arena, enemies/arena.c, until it's beaten), then the HP bar shows and the boss music plays. Graphics are
 * the mini-bosses' (one AnimInfo table for the type, its own palettes and anim ID, a 64x96 hurt frame); */
#include "nidl.h"
#include "abilities.h"
#include "constants/kirby.h"
#include "treasures.h"
#include "bonus.h"
#include "kirby.h"
#include "random.h"
#include "code_0806F780.h"

/* Their HP by the number of players as KatAM's bosses' (gUnk_08351530), from NiDL's 60 (a spit star takes 10: 6 stars, as KatAM's
 * Kracko and King Golem, whose 60, 80, 100 and 110 they all take; its HP bar's fill, KatAMExpansion.cs, is Kracko's); Mr. Shine and
 * Mr. Bright have NiDL's 30 each (bosses/shine_bright.c takes their hits twice over, their HP bar's fill tuned to 60); the Power Orb
 * counts its hits itself, a Star Rod star each (bosses/power_orb.c); */
#define NIDL_BOSS_HP { 60, 80, 100, 110 }

const s16 gNidlBossHp[NIDL_BOSS_COUNT][4] = {
    [NIDL_WHISPY_WOODS] = NIDL_BOSS_HP,
    [NIDL_KRACKO] = NIDL_BOSS_HP,
    [NIDL_SHINE_BRIGHT] = NIDL_BOSS_HP,
    [NIDL_META_KNIGHT] = NIDL_BOSS_HP,
    [NIDL_PAINT_ROLLER] = NIDL_BOSS_HP,
    [NIDL_HEAVY_MOLE] = NIDL_BOSS_HP,
    [NIDL_KING_DEDEDE] = NIDL_BOSS_HP,
    [NIDL_POWER_ORB] = NIDL_BOSS_HP,
    [NIDL_NIGHTMARE] = NIDL_BOSS_HP,
};

static const struct NidlBoss sNidlBosses[NIDL_BOSS_COUNT] = {
    [NIDL_WHISPY_WOODS] = { WhispyWoodsInit, WhispyWoodsStart },
    [NIDL_KRACKO] = { KrackoInit, KrackoStart },
    [NIDL_SHINE_BRIGHT] = { ShineBrightInit, ShineBrightStart },
    [NIDL_META_KNIGHT] = { MetaKnightInit, MetaKnightStart },
    [NIDL_PAINT_ROLLER] = { PaintRollerInit, PaintRollerStart },
    [NIDL_HEAVY_MOLE] = { HeavyMoleInit, HeavyMoleStart },
    [NIDL_KING_DEDEDE] = { KingDededeInit, KingDededeStart },
    [NIDL_POWER_ORB] = { PowerOrbInit, PowerOrbStart },
    [NIDL_NIGHTMARE] = { NightmareInit, NightmareStart },
};

// NidlBossHp(); A boss's HP at the start, for the number of players;
s16 NidlBossHp(u8 boss) {
    return gNidlBossHp[boss][gNumHumanPlayers - 1];
}

void sub_0809F974(struct Object *);

/* NidlBossHitMore(); A hit that took HP off a boss (KatAM's, between frames: from `hp` to its object's, not out) takes `times` halves
 * of it instead, rounded up (Heavy Mole's and Paint Roller's 3: their 60 on the HP bar's 60, as the other bosses' (its fill is theirs,
 * one for both their types), down as 40's, KatAM's bigger mini-bosses' (gUnk_08351530): a spit star's 10 takes 15, 4 of them, a
 * sword's 1 takes 2; the owner's: too much HP); out, KatAM's defeat as its own killing hit's (sub_0809AF38: frozen, sub_0809F974; its
 * explosion) and TRUE: the boss's frame ends there; */
bool32 NidlBossHitMore(struct Object *obj, s16 hp, u8 times) {
    s32 left = hp - ((hp - obj->unk80) * times + 1) / 2;

    if (obj->unk80 <= 0 || obj->unk80 >= hp) return FALSE;
    if (left > 0) {
        obj->unk80 = left;
        return FALSE;
    }
    obj->unk80 = 0;
    obj->base.unk68 &= 7;
    obj->base.unk5C |= 0x80;
    if (!(obj->objTemplate->unk22 & 4)) sub_08086C48(obj);
    PlaySfx(&obj->base, SE_MINIBOSS_PRE_EXPLODE);
    obj->base.flags |= 0x200;
    obj->unk78 = sub_0809F974;
    sub_08088528(obj);
    return TRUE;
}

// Where a boss leaves its shard: the middle of its screen (the camera's, KatAM's 240x160), a little up;
#define SHARD_X 120
#define SHARD_Y 72

void *CreateNidlBoss(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, NIDL_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, NidlDestroy);
    struct NidlObject *n = TaskGetStructPtr(task);
    struct Object *obj = &n->obj;
    u8 boss = template->subtype1 >> 4;

    if (boss >= NIDL_BOSS_COUNT || !gNidlData.bosses[boss].animId) boss = 0;
    InitObject(obj, template, playerId);
    n->enemy = boss;
    n->variant = template->subtype1 & 0xF;
    n->animBase = gNidlData.bosses[boss].animBase;
    n->hurtVram = 0;
    n->sequence = NULL;
    n->activated = FALSE;
    n->hurt = 0;
    obj->unk80 = NidlBossHp(boss);
    // As the mini-bosses: water doesn't defeat it, untouchable until the fight starts;
    obj->base.unkC |= 1;
    obj->base.flags |= 0x2000000 | 0x200 | 0x200000;
    sNidlBosses[boss].init(n);
    NidlSetFrame(n, 0);
    ObjectInitSprite(obj);
    obj->base.sprite.unk14 = 0x6C0;
    NidlInitPalette(n);
    NidlUploadHurtTiles(n);
    NidlBossStart(obj);
    return n;
}

/* NidlBossWait(); Before the fight: until Kirby is in the template's area, then the HP bar shows and the boss music plays
 * (Macro_08100F18: gUnk_08352D80[0x43 - 0x38], the boss music); the Power Orb's as he comes in (its intro flies him up into its
 * area, bosses/power_orb.c); */
static void NidlBossWait(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    NidlUpdate(n);
    obj->base.flags |= 4;
    // (the ending down from the moon in the Power Orb's room: not there meanwhile, ending.c);
    if (n->enemy == NIDL_POWER_ORB && NidlEndingOn(obj->base.roomId)) {
        obj->base.flags |= 0x400;
        return;
    }
    // (Nightmare's intro: hidden till his light shows where he appears, his fight's first appearance, bosses/nightmare.c; the Kirbys
    // still knocked down then, held by it);
    if (n->enemy == NIDL_NIGHTMARE) {
        if (NightmareIntroHolds(obj->base.roomId)) {
            obj->base.flags |= 0x400;
            return;
        }
        obj->base.flags &= ~0x400;
    }
    obj->kirby3 = FindTargetKirby(&obj->base);
    if (((obj->kirby3->base.unkC & 0x8000) && n->enemy != NIDL_NIGHTMARE) || obj->base.roomId != obj->kirby3->base.roomId) return;
    if (n->enemy != NIDL_POWER_ORB && !Macro_08039430_2(&obj->kirby3->base, obj)) return;
    Macro_081003EC(obj, &obj->kirby3->base);
    obj->base.flags &= ~0x200;
    Macro_08100F18(obj);
    n->activated = TRUE;
    sNidlBosses[n->enemy].start(obj);
}

// NidlBossStart(); The type's behavior start;
void NidlBossStart(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    obj->kirbyAbility = 0;
    // The engine's gravity off, as the enemies' (see NidlStart);
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    if (n->activated) {
        sNidlBosses[n->enemy].start(obj);
        return;
    }
    ObjectSetFunc(obj, 0, NidlBossWait);
}

/* NidlBossDefeat(); gUnk_08350E58's entry for type 0x43, run as it explodes (as KatAM's bosses, sub_08087390...): its mirror shard
 * (the behavior's low nibble) in the middle of the screen, which Kirby takes as a boss's (the shard cutscene, then where
 * gNidlShardExits says for the room: the boss stage's way on, its room having no door out; KatAM's: Central Circle); a Maxim
 * Tomato instead, never the shard, when the shard is owned already (KatAM's bosses fought again leave one, sub_08088A38): in a
 * room with no door out (one gNidlShardExits has), eating it takes Kirby where the shard would (NidlExitTomato()). The finale's
 * bosses leave none (finale.c): the Power Orb its warp star, Nightmare the way to the Fountain of Dreams hub; */
static const struct NidlShardExit *NidlShardExitOf(u16 room);

void NidlBossDefeat(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    // (the Power Orb's: the Kirbys flying with the Star Rod stop aiming at it, abilities/star_rod.c);
    if (n->enemy == NIDL_POWER_ORB) {
        StarRodFightOver(obj);
        PowerOrbBeaten(obj);
        return;
    }
    if (n->enemy == NIDL_NIGHTMARE) {
        NightmareBeaten(obj);
        return;
    }
    // (Meta Knight's: NiDL's defeat first, his shard after it, bosses/meta_knight.c);
    if (n->enemy == NIDL_META_KNIGHT) {
        MetaKnightBeaten(obj);
        return;
    }
    // (King Dedede's: NiDL's defeat first, his shard after it, bosses/dedede.c);
    if (n->enemy == NIDL_KING_DEDEDE) {
        KingDededeBeaten(obj);
        return;
    }
    NidlBossShard(obj->base.unk56, obj->base.roomId, n->variant & 7);
}

// NidlBossShard(); A boss's mirror shard `shard` in room `room` (its Maxim Tomato, the shard owned), as NidlBossDefeat() says;
void NidlBossShard(u8 unk56, u16 room, u8 shard) {
    NidlBossShardAt(unk56, room, shard, SHARD_X, SHARD_Y);
}

/* NidlBossShardAt(); The same, at (`screenX`, `screenY`) on the camera's screen (King Dedede's: where the Fountain of Dreams behind
 * his fight doesn't hide it, bosses/dedede.c); */
void NidlBossShardAt(u8 unk56, u16 room, u8 shard, s32 screenX, s32 screenY) {
    struct LevelInfo *level = &gCurLevelInfo[unk56];
    s32 x = (level->viewportPosition.x >> 8) + screenX, y = (level->viewportPosition.y >> 8) + screenY;

    if (HasShard(shard)) {
        struct Object *tomato = CreateObjTemplateAndObj(unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, OBJ_TOMATO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0);
        struct ObjectTemplate *template = (struct ObjectTemplate *)tomato->objTemplate;

        // (BonusCreateTomatoAt()'s; its gone bit is NIDL_EXIT_KIND's, unk3 its template's slot, unk12 the room: rooms.c's
        // BigSub_08001678 calls NidlExitTomato);
        tomato->base.flags |= 0x2000000;
        if (NidlShardExitOf(room)) {
            template->unk2 = NIDL_EXIT_KIND;
            template->unk3 = template - gUnk_020229E0;
            template->unk12 = room;
        }
        return;
    }
    CreateObjTemplateAndObj(unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/* NidlShake(); NiDL's screen shake (0x080261D4) as KatAM's (RequestScreenShake): NiDL's levels 1-4 are timed (14, 18, 24 and 24
 * frames, its tables at 0x08732884), 5-7 the same steps looping until stopped, 0 stops it, more than 7 does nothing; KatAM's 1-3 are
 * NiDL's 1-3 (its 3 NiDL's 4's steps, a pixel smaller) and its 4 the loop of 2, so NiDL's 4 is KatAM's 3, 5-7 its 4 (asked for 4,
 * KatAM shook until the fight's end: Mr. Shine & Mr. Bright's wall hits); */
void NidlShake(u16 level, struct ObjectBase *obj) {
    if (level > 7) return;
    if (!level) sub_080700D8(obj);
    else RequestScreenShake(level >= 5 ? 4 : level == 4 ? 3 : level, obj);
}

/* NidlBossHitShake(); A hit on a ported boss that it lives through shakes the screen a little, as one on KatAM's own bosses does
 * (sub_0809AF38's RequestScreenShake(3): 24 frames, a pixel or three each way; the owner's: the small vibration on a boss hit,
 * missing). KatAM's own hit gives it when it can (asked again, the shake starts over: the same); it gives none to an object with
 * unkC 0x800 (Mr. Shine and Mr. Bright) or to an attack with unk68 0x800000. Not the Power Orb's: its hits shake its foreground,
 * not the screen (bosses/power_orb.c, the owner's); */
void NidlBossHitShake(struct Object *obj) {
    if (obj->unk80 > 0) RequestScreenShake(3, &obj->base);
}

/* NidlExitTomato(); A rematch's Maxim Tomato (NidlBossDefeat()) gone, from template slot `slot` (rooms.c's BigSub_08001678: its
 * gone bit): its template slot freed (the game frees only those with unk2 0); eaten, every Kirby in its room dances and leaves
 * (NidlDanceOut()); */
void NidlExitTomato(u16 slot, u32 eaten) {
    gUnk_020229D4 &= ~(1 << slot);
    if (eaten) NidlDanceOut(gUnk_020229E0[slot].unk12);
}

/* NidlDanceOut(); Every Kirby in a room dances and leaves, as after a boss's shard (sub_0804BD98, as a warp star's end does it: where
 * he stands; then sub_0804C614 and NidlShardExit(): where the shard would take him); */
void NidlDanceOut(u16 room) {
    u16 i;
    u8 count = 0, index = 0, dance = (Rand16() & 1) + 2;

    for (i = 0; i < 4; i++)
        if (gKirbys[i].base.roomId == room) count++;
    for (i = 0; i < 4; i++)
        if (gKirbys[i].base.roomId == room) sub_0804BD98(&gKirbys[i], index++, count, dance, 0, gKirbys[i].base.y >> 12);
}

/* gNidlShardExits; Where Kirby goes once a boss's shard cutscene ends (KatAM: Central Circle, sub_0804C614's `roomId = 0x321`, which
 * KatAMExpansion.cs replaces with a call to NidlShardExit()): from a ported boss's room (each part), the NiDL hub, onto the level's
 * door there (the Power Orb's: Nightmare's room, the rest of its fight): a boss room has no door out, the shard is the way out
 * (KatAMExpansionRooms.cs' bossExitsOf);
 * filled in by KatAMExpansion.cs, ended by a room 0; */
#define NIDL_SHARD_EXITS 24
struct NidlShardExit {
    u16 room, exitRoom;
    u8 x, y;    // the landing cell + 1, as a door's;
    u8 left, right; // the cells of its ground either side, up to 2 (the NiDL hub's; 0 elsewhere): a Kirby on each (NidlShardExit());
};

const struct NidlShardExit gNidlShardExits[NIDL_SHARD_EXITS] = { { 0 } };

// NidlShardExitOf(); A room's entry in gNidlShardExits, or NULL;
static const struct NidlShardExit *NidlShardExitOf(u16 room) {
    const struct NidlShardExit *exit;

    for (exit = gNidlShardExits; exit < gNidlShardExits + NIDL_SHARD_EXITS && exit->room; exit++)
        if (exit->room == room) return exit;
    return NULL;
}

/* NidlShardExit(); The room and spot Kirby comes back to after the shard cutscene (sub_0804C614); his ability before the Star Rod, back;
 * the first Kirby (the player) on the landing, the others on its ground either side, each on his own cell, the second left of it, the
 * third right, the fourth further (or with the third, the ground ending): KatAM put them all on the one cell and they pushed each other
 * apart, the first furthest left, off a pillar's top in the NiDL hub (Kracko's, Grape Garden's door, with helpers: falling as if
 * through it, the owner's); */
void NidlShardExit(struct Kirby *kirby) {
    static const s8 sides[] = { 0, -1, 1, -2, 2 };
    const struct NidlShardExit *exit = NidlShardExitOf(kirby->base.roomId);
    u8 i, n = 0, which = kirby - gKirbys;
    s8 side = 0;

    StarRodRestore(kirby);

    if (exit) {
        for (i = 0; i < sizeof(sides); i++) {
            if (sides[i] < -exit->left || sides[i] > exit->right) continue;
            side = sides[i];
            if (n++ == which) break;
        }
        kirby->roomId = exit->exitRoom;
        kirby->spawnLocation.x = exit->x + side;
        kirby->spawnLocation.y = exit->y;
        return;
    }
    kirby->roomId = 0x321;
    kirby->spawnLocation.x = kirby->unkF2;
    kirby->spawnLocation.y = kirby->unkF3;
}

