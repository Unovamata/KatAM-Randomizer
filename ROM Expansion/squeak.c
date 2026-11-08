/* Squeak Squad enemies; Object type 0x2C: creates the character the template's behavior byte asks for and handles what all of them
 * share (see squeak.h); */
#include "squeak.h"
#include "constants/kirby.h"
#include "constants/object_types.h"
#include "abilities.h"
#include "code_0806F780.h"

void NidlDestroy(struct Task *task);

// HP as KatAM's enemies' (NiDL's ported ones' too): 4 for the ones that give an ability, 2 for the others;
const s16 gSqueakEnemyHp[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = 2,
    [SQUEAK_GOLD_WADDLE_DEE] = 6,   // Squeak Squad's: 30, a Waddle Dee's 10 (Perara's too);
    [SQUEAK_GUSSA] = 2,             // Squeak Squad's: 10 (prmNE12Sl0.kgb);
    [SQUEAK_CRIMP] = 2,             // Squeak Squad's: 10 (prmNE13Sl0.kgb);
    [SQUEAK_CRIMP_ROCK] = 1,        // (KatAM's: any hit breaks it);
};

static const struct SqueakCharacter sSqueakCharacters[SQUEAK_ENEMY_COUNT] = {
    [SQUEAK_PERARA] = { PeraraInit, PeraraStart, 0, FALSE },
    [SQUEAK_GOLD_WADDLE_DEE] = { GoldWaddleDeeInit, GoldWaddleDeeStart, 0, TRUE },
    [SQUEAK_GUSSA] = { GussaInit, GussaStart, 0, TRUE },
    [SQUEAK_CRIMP] = { CrimpInit, CrimpStart, 0, FALSE },
    [SQUEAK_CRIMP_ROCK] = { CrimpRockInit, CrimpRockStart, 0, FALSE },
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

static void SqueakReleaseParts(struct SqueakObject *s);
static void SqueakShowComposite(struct SqueakObject *s);

// SqueakPlay(); Starts one of the character's Squeak Squad animations, from its first step;
void SqueakPlay(struct SqueakObject *s, u8 anim) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[anim];

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
 * animation starts over, the others hold their last step. Either way animEnded tells the animation got through once (Squeak
 * Squad's flag 8, which characters wait for). The step's offset moves the drawing (Squeak Squad adds it to the cell's position when
 * it builds the OAM, 0x01FF8720, turned with the character as the cell; KatAM's objBase54/55, which the drawing clears once used); */
void SqueakAnimate(struct SqueakObject *s) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->enemy].anim[s->anim];

    if (s->composite != SQUEAK_NO_COMPOSITE) {
        s->compositeTime++;
        SqueakShowComposite(s);
        return;
    }
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
    s8 dx, dy;                  // Where it's drawn (pixels from the character, turned);
    u8 cell;                    // Its cell (the character's frame, variants SQUEAK_HURT_VARIANTS on);
    u8 index;                   // Its part (1 on);
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
    p->e.x = s->obj.base.x;
    p->e.y = s->obj.base.y;
    p->e.sprite.variant = p->cell + SQUEAK_HURT_VARIANTS;
    p->e.sprite.palId = s->obj.base.sprite.palId;
    // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right, flipped when facing left);
    p->e.flags = (p->e.flags & ~0x401) | 4 | (s->obj.base.flags & 1 ? 0 : 1);
    p->e.objBase54 = p->dx;
    p->e.objBase55 = p->dy;
    EffectObjectPostUpdate(&p->e);
}

// SqueakCreatePart(); A sprite for part `index` of the character's compositions;
static struct SqueakPartFx *SqueakCreatePart(struct SqueakObject *s, u8 index) {
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
    p->cell = 0;
    p->index = index;
    EffectObjectInitSprite(&p->e, &p->e.sprite, g->tiles, g->animId, SQUEAK_HURT_VARIANTS, 0x1A + index < 0x1F ? 0x1A + index : 0x1F);
    p->e.sprite.palId = s->obj.base.sprite.palId;
    return p;
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

/* SqueakShowComposite(); The composite animation at its time: the composition its step shows, and each part's cell and offset (the
 * first the character's own, the others its SqueakPartFx, made or let go as the composition has them); animEnded once its frames
 * went by (flag 8); */
static void SqueakShowComposite(struct SqueakObject *s) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[s->enemy];
    const struct SqueakComposite *c = &g->composite[s->composite];
    const struct SqueakComposition *composition;
    struct SqueakPartFx **link = (struct SqueakPartFx **)&s->parts, *p;
    u32 total = 0, time = s->compositeTime, local;
    u8 i;

    for (i = 0; i < c->steps; i++) total += c->step[i].frames;
    if (total && time + 1 >= total) s->animEnded = TRUE;
    local = total == 0 ? 0 : c->loops ? time % total : time < total ? time : total - 1;
    time = local;
    for (i = 0; i + 1 < c->steps && time >= c->step[i].frames; i++) time -= c->step[i].frames;
    composition = &g->composition[c->step[i].composition];
    for (i = 0; i < composition->parts; i++) {
        const struct SqueakPart *part = &composition->part[i];
        const struct SqueakStep *step = SqueakStepAt(&g->anim[part->anim], part->absolute ? s->compositeTime : local);
        s32 x = SqueakTurned(s, part->x + step->x), y = part->y + step->y;

        if (i == 0) {
            SqueakSetCell(s, step->cell);
            s->obj.base.objBase54 = x;
            s->obj.base.objBase55 = y;
            continue;
        }
        if (*link == NULL) *link = SqueakCreatePart(s, i);
        if ((p = *link) == NULL) break;
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
static u8 SqueakLoadPalette(u16 key, u8 variant, u16 roomId) {
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
    if (!(base->flags & 0x100)) sub_0809D8C8(base);
    s->terrain = base->unk62;
    s->water = base->unk58;
    s->moved = TRUE;
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
struct SqueakFx {
    struct EffectObject e;
    struct SqueakObject *owner; // Followed (0x020218C0: its position plus the offset each frame), till it's gone;
    s32 dx, dy;
    u8 effect;
    u8 step;
    u16 timer;
    bool8 left;
    u8 palId;
};

// SqueakFxDestroy(); An effect's end (its own, or the room's): its character lets go of it;
static void SqueakFxDestroy(struct Task *t) {
    struct SqueakFx *fx = TaskGetStructPtr(t);

    if (fx->owner && fx->owner->follower == fx) fx->owner->follower = NULL;
    ObjectBaseDestroy(t);
}

static void SqueakFxMain(void) {
    struct SqueakFx *fx = TaskGetStructPtr(gCurTask);
    const struct SqueakAnim *a = &gSqueakData.effects[fx->effect].anim;
    const struct SqueakStep *step;

    if (fx->e.flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (fx->timer && --fx->timer == 0) {
        if (fx->step + 1 >= a->steps && !a->loops) {
            fx->e.flags |= 0x1000;
            return;
        }
        fx->step = fx->step + 1 >= a->steps ? 0 : fx->step + 1;
        fx->timer = a->step[fx->step].frames;
    }
    if (fx->owner) {
        fx->e.x = fx->owner->obj.base.x + SQUEAK_SPEED(fx->dx);
        fx->e.y = fx->owner->obj.base.y - SQUEAK_SPEED(fx->dy);
    }
    step = &a->step[fx->step];
    fx->e.sprite.variant = step->cell;
    fx->e.sprite.palId = fx->palId;
    // (KatAM flips a frame when flag 1 is set: its frames face left, Squeak Squad's cells right, flipped when facing left);
    fx->e.flags = (fx->e.flags & ~0x401) | 4 | (fx->left ? 0 : 1);
    fx->e.objBase54 = fx->left ? -step->x : step->x;
    fx->e.objBase55 = step->y;
    EffectObjectPostUpdate(&fx->e);
}

// SqueakEffectAt(); An effect at (x, y) (KatAM's units), facing left or right, playing from its first step;
static struct SqueakFx *SqueakEffectAt(struct SqueakObject *s, u8 effect, s32 x, s32 y, bool8 left) {
    const struct SqueakEffect *d = &gSqueakData.effects[effect];
    struct Task *t;
    struct SqueakFx *fx;

    if (gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return NULL;
    t = TaskCreate(SqueakFxMain, sizeof(struct SqueakFx), 0x3500, TASK_USE_EWRAM, SqueakFxDestroy);
    if (t == NULL) return NULL;
    fx = TaskGetStructPtr(t);
    ClearEffectObject(&fx->e);
    fx->e.header.kind = 3;
    fx->e.roomId = s->obj.base.roomId;
    fx->e.parent = NULL;
    fx->e.x = x;
    fx->e.y = y;
    fx->owner = NULL;
    fx->dx = fx->dy = 0;
    fx->effect = effect;
    fx->step = 0;
    fx->timer = d->anim.step[0].frames;
    fx->left = left;
    fx->palId = SqueakLoadPalette(d->animId, d->anim.step[0].cell, s->obj.base.roomId);
    EffectObjectInitSprite(&fx->e, &fx->e.sprite, d->tiles, d->animId, d->anim.step[0].cell, 0x1A);
    return fx;
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

// SqueakDestroy(); A character's end: the effect following it stays where it is, its parts go;
static void SqueakDestroy(struct Task *t) {
    struct SqueakObject *s = TaskGetStructPtr(t);

    if (s->follower) ((struct SqueakFx *)s->follower)->owner = NULL;
    SqueakReleaseParts(s);
    NidlDestroy(t);
}

// (SqueakObject fits the room every character gets);
typedef char SqueakObjectFits[sizeof(struct SqueakObject) <= SQUEAK_OBJECT_SIZE ? 1 : -1];

void *CreateSqueakEnemy(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, SQUEAK_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, SqueakDestroy);
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
    s->brain = 0;
    s->started = FALSE;
    s->moved = FALSE;
    s->terrain = s->water = 0;
    s->follower = NULL;
    s->composite = SQUEAK_NO_COMPOSITE;
    s->parts = NULL;
    s->compositeTime = 0;
    SqueakStop(s);
    s->padDir = s->padY = s->padA = s->padB = 0;
    for (i = 0; i < ARRAY_COUNT(s->vars); i++) s->vars[i] = 0;
    s->obj.unk80 = gSqueakEnemyHp[enemy];
    sSqueakCharacters[enemy].init(s);
    if (sSqueakCharacters[enemy].swims) s->obj.base.unkC |= 1;
    // (the type's AnimInfo table has one entry: ObjectInitSprite reads it; the patched drawing shows the character's own frames);
    s->obj.unk83 = 0;
    ObjectInitSprite(&s->obj);
    s->obj.base.sprite.palId = SqueakLoadPalette(gSqueakData.enemies[enemy].animId, SQUEAK_HURT_VARIANTS, s->obj.base.roomId);
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
    s->started = TRUE;
}

// SqueakName(); A character's HP bar name (NidlCopyName shows it);
const u8 *SqueakName(struct Object *obj) {
    return gSqueakData.names + ((struct SqueakObject *)obj)->enemy * 0x100;
}

// SqueakMaxHp(); A character's HP at the start (its table's: the randomizer's HP options may have changed it);
s16 SqueakMaxHp(struct Object *obj) {
    return gSqueakEnemyHp[((struct SqueakObject *)obj)->enemy];
}
