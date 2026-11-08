/* Squeakers; ported from Kirby: Squeak Squad (USA), the Squeak Squad's foot soldiers, enemy 0x40 (its body 0x020EC504, vtable
 * 0x0217DB64; brain 0x020E9444; its bombs 0x020ED208, made by 0x020ED308), as SQUEAK_SQUEAKER and SQUEAK_SQUEAKER_BOMB. Values from
 * param/prmDE01Ml0.kgb (movement, the bombs' too), prmDE01Pl0.kgb (brain) and prmDE01Sl0.kgb (HP 10); speeds 20.12 a frame, Y up.
 * Mice in four colors (the spawn's first parameter byte, KatAM's template unk12: 0 red, 1 blue, 2 green, 3 yellow; their palettes
 * enemy/chun_blue, _green, _yellow, squeak.c's SqueakUsePalette); they hop about, throw bombs at Kirby, and run off with treasure
 * chests.
 * • Brain (0x020E9378, 0x020E931C, then 0x020E8F9C each frame): its target is the nearest treasure chest in the room (Squeak Squad's
 *   0x020CF348: the stage's chests, kind 0x2000) not held by another; with none left to take (all held) Kirby; with no chest in the
 *   room at all, or one in its hands, its hole (0x0220997C: the stage's escape spot for it; KatAM's rooms have none: here the spot
 *   it was placed at, where it came from). Its pad steers it there (0x020E8D0C): across, toward it beyond 4 pixels (on the ground
 *   or under a ceiling, whatever the distance); up or down beyond 4, up (with A when it's higher than it's away across) or down;
 *   a wall ahead, A to hop it with the target above. Kirby within 256 pixels across and 64 up or down (not yellow ones, not with a
 *   chest; blue ones at most once in 336 frames of thinking), it faces him (0x020E8F84) and waits (0x020E8F04: 60 frames half the
 *   time, none, 20, 30 or 45 a tenth, 20% 45; only on the ground, its pad off), then presses B one frame (0x020E8E7C) and thinks
 *   again. Blue and green ones only head for a chest within 80 pixels across and 48 up or down (prmDE01Pl0.kgb); else they keep
 *   still. Within 32 pixels of its hole with a chest, it presses its escape (pad 5, 5).
 * • Body (gravity 0.13 a frame, falls no faster than 2.5; on its pad held 4 frames it hops: anim 6, then anim 7, 4 frames crouched
 *   (0x020EBBFC, 0x020EBA8C, 0x020EB8AC), off at 0.70 forward and 1.78 up (up held: 3.70, straight up unless forward too), anim 8
 *   rising (0x020EB794), anim 9 falling (0x020EB6CC, 0x020EB500: A after 10 frames hops again), anim 10 landing (0x020EB458);
 *   off a ledge it falls steering (0x020EB390, 0x020EB17C: forward 0.70, none stopping it), anim 10 landing (0x020EB0D4); with a
 *   chest each has its own animation (11-15, 35 standing). B: red, green and yellow ones pull out a bomb (anim 24, 0x020E9EC0),
 *   held while B is (0x020E9D98), wind up (anim 25, 60 frames: 0x020E9C80) and throw it (anim 26, 21 frames: 0x020E9BD8); blue ones
 *   the same with a big one (anims 36-38: 0x020E9AE8, 0x020E99C0, 0x020E98A8, 0x020E9800). The bomb leaves 8 pixels ahead, 20 up.
 * • Chests (KatAM's, small and big, closed): touching one (its touch, 0x020EC168) it takes it (0x020A2BB0), stops 48 frames (anim 48,
 *   0x020EC008, 0x020EBF68) and carries it 10 pixels behind it, 2 up (its vtable +0x78, 0x020E9640: swaying behind its moves, 1.5
 *   pixels a frame, 6 at most); at its hole it walks to it a pixel a frame (0x020EAF8C, 0x020EAE0C), stops 96 frames, and goes in
 *   with it (anim 27, 64 frames: 0x020EAD78, 0x020EACF0): both gone (the chest back in the room the next time Kirby comes in, KatAM's
 *   own). Hit (KatAM's hit, its touch too, 0x020EC3C0), it drops it where it is (0x020A2A48: it falls to the ground) and can't take
 *   one again for 10 frames of touching it.
 * • Hit, one time in five it drops a treat (0x020E9528: Squeak Squad's foods 9-14 popping up; KatAM's small food, as Boxy's), not
 *   again for 40 frames.
 * • Bombs (0x020ED208): thrown at 3.4, 30 degrees up (one time in five 30, 25, 15, 20 or 30: 0x020ECF88's draw, its adjustments
 *   added up as its code does), falling 0.13 a frame (no faster than 2.4). The small one (anim 33, 0x020ECE44) bursts on the ground
 *   or a wall: unseen, its blast (anim 41's box, 30 pixels round) 9 frames (0x020ECE14), efc_cmn1's burst and a shake. The big one
 *   (anim 39, 0x020ECB48) lands and stays, its wick lit (anim 40, 192 frames: 0x020EC9F4), then blows up 9 times in 60 frames
 *   (0x020EC7DC: every 7 frames a burst and a shake, its blast moving round its spot: 14 left and up, 10 right and down, ...).
 *   Touched, they burst (0x020ED0B8): KatAM's attacks and touch beat them (KatAM's defeat).
 * • Boxes (chun.rca, per animation frame, as uja.c's).
 * Not ported: its sounds (0xF9 the hop, 0x126 the throw, 0x103 the take, 0xF8 the treat, 0x12D and 0x14F the bombs), the stages'
 * routes Squeak Squad's Squeakers follow between their parts (0x02209560's: the next spot on the way, 0x02209BFC) and their tunnels
 * (flag 0x200000: anims 18-23, 0x020EA2EC-0x020EAC30), Kirby carrying the stage's chest (0x020A2BF8: the stage's mode 2, all chase
 * Kirby), Squeak Squad's chests' carried look (0x0209FFC0's animations: KatAM's chests have one). */
#include "../squeak.h"
#include "chest.h"
#include "random.h"
#include "constants/object_types.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (chun.cea's, only those played: KatAMExpansionSqueak.cs's OnlyAnims, in this order);
enum {
    ANIM_STAND, ANIM_HOP, ANIM_CROUCH, ANIM_JUMP, ANIM_FALL, ANIM_LAND, ANIM_HOP_CARRY, ANIM_CROUCH_CARRY, ANIM_JUMP_CARRY,
    ANIM_FALL_CARRY, ANIM_LAND_CARRY, ANIM_PULL, ANIM_WIND, ANIM_THROW, ANIM_ESCAPE, ANIM_HURT, ANIM_HURT_2, ANIM_STAND_CARRY,
    ANIM_BIG_PULL, ANIM_BIG_WIND, ANIM_BIG_THROW, ANIM_TAKE,
};
// Its bombs' (chun.cea's 33, 39-41);
enum { BOMB_ANIM_SMALL, BOMB_ANIM_BIG, BOMB_ANIM_WICK, BOMB_ANIM_BLAST };

// Colors (the spawn's parameter byte);
enum { RED, BLUE, GREEN, YELLOW };
// Bombs (0x020ED208's third parameter);
enum { BOMB_SMALL, BOMB_BIG };

// prmDE01Ml0.kgb;
#define HOP_SPEED 0xB34
#define HOP_LOW 0x1C7B
#define HOP_HIGH 0x3B34
#define GRAVITY 0x215
#define BOMB_X 0x8000
#define BOMB_Y 0x14000
#define BOMB_SPEED 0x3667               // (0x1E000: 30 degrees, its draw's angles' vectors below)
#define CROUCH_TIME 4
#define FALL_TIME 10
#define TAKE_TIME 0x30
#define ESCAPE_TIME 0x60
// prmDE01Pl0.kgb;
#define CHEST_X 0x50000                 // Blue and green ones: a chest within 80 pixels across, 48 up or down;
#define CHEST_Y 0x30000
// The code's;
#define MAX_FALL -0x2800
#define BOMB_MAX_FALL -0x2666
#define NEAR 0x4000                     // Its pad's dead zone (0x020E8F9C's 4 pixels);
#define SIGHT_X 0x100000                // Kirby within 256 pixels across, 64 up or down;
#define SIGHT_Y 0x40000
#define HOLE 0x20000                    // At its hole: within 32 pixels;
#define BLUE_WAIT 0x150
#define HOLD_X -0xA000                  // Its chest 10 pixels behind it, 2 up (16 behind while it stops);
#define HOLD_Y 0x2000
#define STOP_X -0x10000
#define SWAY 0x1800
#define SWAY_BACK 0x800
#define SWAY_MAX 0x6000
#define REGRAB 10
#define TREAT_WAIT 40
#define TREAT_CHANCE 20
#define CHEST_GRAVITY 0x15              // A dropped chest falls as KatAM's objects (0x0809A990);
#define CHEST_MAX_FALL 0x300
#define WICK_FRAMES 7                   // The big bomb's blasts, every 7 frames, 60 in all;
#define BOOM_TIME 60
#define BLAST_TIME 8
#define SHAKE 1                         // KatAM's shortest screen shake (14 frames; Squeak Squad's 0x02035FC4 3, 6, 2);

enum {
    BODY_START, BODY_STAND, BODY_HOP, BODY_HOP_WAIT, BODY_CROUCH, BODY_RISE, BODY_FALL_ENTRY, BODY_FALL, BODY_LAND, BODY_DROP_ENTRY,
    BODY_DROP, BODY_DROP_LAND, BODY_PULL, BODY_HOLD, BODY_WIND, BODY_THROWN, BODY_BIG_PULL, BODY_BIG_HOLD, BODY_BIG_WIND,
    BODY_BIG_THROWN, BODY_TAKE, BODY_PAUSE, BODY_ESCAPE_ENTRY, BODY_TO_HOLE, BODY_ESCAPE, BODY_ESCAPING,
};
enum { BRAIN_START, BRAIN_INIT, BRAIN_THINK, BRAIN_ATTACK, BRAIN_WAIT, BRAIN_PRESS };
// Its target (0x020E8F9C's);
enum { AIM_NONE, AIM_CHEST, AIM_KIRBY, AIM_HOLE };
// Its escape on its pad (pad 5, 5);
#define PAD_ESCAPE 5

#define COUNT(s) ((s)->vars[0])         // 0x234: crouch and fall frames;
#define PAUSE(s) ((s)->vars[1])         // 0x238 and the state after it (0x22C);
#define PAUSE_NEXT(s) ((s)->vars[2])
#define CHEST(s) (*(struct Chest **)&(s)->vars[3])  // 0x16C (0x1F4 and 0x1EC set with it: its carrying animations);
#define SWAY_X(s) ((s)->vars[4])        // 0x240, 0x244;
#define SWAY_Y(s) ((s)->vars[5])
#define REGRAB_COUNT(s) ((s)->vars[6])  // 0x1F8;
#define TREAT_TIMER(s) ((s)->vars[7])   // 0x248;
#define WAIT(s) ((s)->vars[8])          // Its brain's 0x88 and 0x98 (blue ones' wait to attack again);
#define BLUE_TIMER(s) ((s)->vars[9])
#define HOME_X(s) ((s)->vars[10])       // Its hole: where it was placed;
#define HOME_Y(s) ((s)->vars[11])
#define HARM(s) ((s)->vars[12])         // KatAM's unk68 from InitObject: its touch's harm;
#define BOX(s) (*(u8 *)&(s)->vars[13])  // This frame's box (its boxes' index);
#define HURT(s) (*((u8 *)&(s)->vars[13] + 1))  // In KatAM's hurt state (BodyWatch: a hit's first frame);
#define TARGET_X(s) ((s)->vars[14])     // 0x130 (its hole's spot);
#define TARGET_Y(s) ((s)->vars[15])
// Bombs;
#define B_COUNT(s) ((s)->vars[0])       // 0x138;
#define B_SPOT_X(s) ((s)->vars[1])      // 0x13C, 0x140;
#define B_SPOT_Y(s) ((s)->vars[2])
#define B_HARM(s) ((s)->vars[3])
#define B_BOX(s) (*(u8 *)&(s)->vars[4])

// chun.rca's boxes (its animations from the .rca's second; boxes.py, the animations kept in their order here);
static const struct SqueakBox sSqueakerBoxes[] = {
    { { -8, -19, 8, -2 }, TRUE, 0 },
    { { -8, -28, 8, -11 }, TRUE, 0 },
    { { -8, -24, 8, -7 }, TRUE, 0 },
    { { -8, -19, 8, -2 }, FALSE, 0 },
    { { -8, -27, 8, -10 }, TRUE, 0 },
    { { -8, -25, 8, -8 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sSqueakerBody[] = { { 1, 0 } };
static const struct SqueakBoxFrame sSqueakerWind[] = { { 2, 0 }, { 6, 1 }, { 24, 0 }, { 4, 2 }, { 4, 2 }, { 20, 0 } };
static const struct SqueakBoxFrame sSqueakerHurt[] = { { 1, 3 } };
static const struct SqueakBoxFrame sSqueakerBigWind[] = { { 2, 0 }, { 6, 4 }, { 24, 0 }, { 4, 0 }, { 4, 5 }, { 20, 0 } };
static const struct SqueakBoxAnim sSqueakerBoxAnims[] = {
    [ANIM_STAND] = { sSqueakerBody, 1 },
    [ANIM_HOP] = { sSqueakerBody, 1 },
    [ANIM_CROUCH] = { sSqueakerBody, 1 },
    [ANIM_JUMP] = { sSqueakerBody, 1 },
    [ANIM_FALL] = { sSqueakerBody, 1 },
    [ANIM_LAND] = { sSqueakerBody, 1 },
    [ANIM_HOP_CARRY] = { sSqueakerBody, 1 },
    [ANIM_CROUCH_CARRY] = { sSqueakerBody, 1 },
    [ANIM_JUMP_CARRY] = { sSqueakerBody, 1 },
    [ANIM_FALL_CARRY] = { sSqueakerBody, 1 },
    [ANIM_LAND_CARRY] = { sSqueakerBody, 1 },
    [ANIM_PULL] = { sSqueakerBody, 1 },
    [ANIM_WIND] = { sSqueakerWind, 6 },
    [ANIM_THROW] = { sSqueakerBody, 1 },
    [ANIM_ESCAPE] = { sSqueakerBody, 1 },
    [ANIM_HURT] = { sSqueakerHurt, 1 },
    [ANIM_HURT_2] = { sSqueakerHurt, 1 },
    [ANIM_STAND_CARRY] = { sSqueakerBody, 1 },
    [ANIM_BIG_PULL] = { sSqueakerBody, 1 },
    [ANIM_BIG_WIND] = { sSqueakerBigWind, 6 },
    [ANIM_BIG_THROW] = { sSqueakerBody, 1 },
    [ANIM_TAKE] = { sSqueakerBody, 1 },
};
static const struct SqueakBoxes sSqueakerBoxData = { sSqueakerBoxes, sSqueakerBoxAnims, ARRAY_COUNT(sSqueakerBoxAnims) };

static const struct SqueakBox sBombBoxes[] = {
    { { -6, -6, 7, 6 }, TRUE, 0 },
    { { -12, -13, 14, 13 }, TRUE, 0 },
    { { -30, -30, 30, 30 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sBombSmall[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBombBig[] = { { 1, 1 } };
static const struct SqueakBoxFrame sBombBlast[] = { { 1, 2 } };
static const struct SqueakBoxAnim sBombBoxAnims[] = {
    [BOMB_ANIM_SMALL] = { sBombSmall, 1 },
    [BOMB_ANIM_BIG] = { sBombBig, 1 },
    [BOMB_ANIM_WICK] = { sBombBig, 1 },
    [BOMB_ANIM_BLAST] = { sBombBlast, 1 },
};
static const struct SqueakBoxes sBombBoxData = { sBombBoxes, sBombBoxAnims, ARRAY_COUNT(sBombBoxAnims) };

/* The bombs' launch (0x020ECF88, 0x020ECC60: 3.4 at 30 degrees, its draw under 20 +10, under 40 +5, under 60 -5, and under 80 -10
 * too, as its code adds them: 30, 25, 15, 20, 30 degrees), as 0x020ECD90 makes them (its sine table, 0x0215CE20); */
static const s32 sBombLaunch[5][2] = {
    { 0x2F20, 0x1B2D }, { 0x3151, 0x16F7 }, { 0x3492, 0xE07 }, { 0x3323, 0x1291 }, { 0x2F20, 0x1B2D },
};

// The big bomb's blasts (0x020EC7DC: from its spot, every 7 frames; then its spot);
static const s32 sBoom[8][2] = {
    { -0xE000, -0xE000 }, { 0xA000, 0xA000 }, { -0x12000, -0xE000 }, { 0x12000, 0xA000 },
    { -0xA000, -0x12000 }, { 0x8000, 0xC000 }, { -0x12000, -0x12000 }, { 0x12000, 0xC000 },
};

static void SqueakerMain(struct Object *obj);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

static u8 Color(struct SqueakObject *s) {
    return s->variant <= YELLOW ? s->variant : RED;
}

/* Chests: KatAM's (objects 0x80 and 0x81). One a Squeaker holds or that's falling from its hands keeps its own function in its unkDC
 * (CreateChest's 0, nothing else uses it) and runs ChestHeld or ChestDropped instead (Kirby can't open it meanwhile); */
static bool32 IsChest(struct Object *obj) {
    return obj->type == OBJ_SMALL_CHEST || obj->type == OBJ_BIG_CHEST;
}

// ChestHeld(); A chest in a Squeaker's hands: its Squeaker places it;
static void ChestHeld(struct Object *obj) {
}

/* ChestDropped(); A chest let go (0x020A2A48: where its Squeaker was): it falls to the ground (KatAM's gravity, unkA0 its speed), and
 * there it's itself again, its feet on the cell's floor as CreateChest puts them; */
static void ChestDropped(struct Object *obj) {
    struct Chest *chest = (struct Chest *)obj;
    s32 bottom = obj->base.y + obj->base.unk3F * 0x100;

    if (NidlIsFloor(obj, obj->base.x, bottom + 0x100) || obj->base.y > (gCurLevelInfo[obj->base.unk56].roomHeight << 8)) {
        obj->base.y = ((bottom + 0xFFF) & ~0xFFF) - obj->base.unk3F * 0x100 - 1;
        obj->unkA0 = 0;
        obj->unk78 = (void *)chest->unkDC;
        chest->unkDC = 0;
        return;
    }
    if (obj->unkA0 < CHEST_MAX_FALL) obj->unkA0 += CHEST_GRAVITY;
    obj->base.y += obj->unkA0;
}

// ChestOpenable(); Closed (unk83 0, or 2: a 0x63 one) and itself (not held nor falling), in the Squeaker's room;
static bool32 ChestClosed(struct SqueakObject *s, struct Object *obj) {
    return IsChest(obj) && obj->base.roomId == s->obj.base.roomId && !(obj->base.flags & 0x1000) && !(obj->unk83 & 1);
}

/* SqueakNearestChest(); 0x020CF348: the nearest closed chest no Squeak holds; `any`: there's a closed chest in the room at all (the
 * Squeaks' chest thefts: Squeakers, Spinni); */
struct Chest *SqueakNearestChest(struct SqueakObject *s, bool8 *any) {
    struct Chest *nearest = NULL;
    u32 best = 0xFFFFFFFF;
    u8 i;

    *any = FALSE;
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];
        struct Object *obj;
        s32 dx, dy;
        u32 d;

        if (t->main != ObjectMain) continue;
        obj = TaskGetStructPtr(t);
        if (!ChestClosed(s, obj)) continue;
        *any = TRUE;
        if (((struct Chest *)obj)->unkDC) continue;
        dx = (obj->base.x - s->obj.base.x) >> 8;
        dy = (obj->base.y - s->obj.base.y) >> 8;
        d = dx * dx + dy * dy;
        if (d < best) {
            best = d;
            nearest = (struct Chest *)obj;
        }
    }
    return nearest;
}

/* SqueakHeldChest(); The chest `*held`, if it's still held: still a chest, there and held (the room's end may have freed it, and its
 * memory hold another object since); else none (`*held` cleared); */
struct Chest *SqueakHeldChest(struct Chest **held) {
    struct Chest *chest = *held;

    if (chest && (!IsChest(&chest->obj) || chest->obj.unk78 != ChestHeld || (chest->obj.base.flags & 0x1000))) chest = *held = NULL;
    return chest;
}

static struct Chest *Held(struct SqueakObject *s) {
    return SqueakHeldChest(&CHEST(s));
}

// SqueakChestFeet(); A chest's spot as Squeak Squad's (its feet; 20.12, Y up);
void SqueakChestFeet(struct Chest *chest, s32 *x, s32 *y) {
    *x = chest->obj.base.x << 4;
    *y = -(chest->obj.base.y + chest->obj.base.unk3F * 0x100) << 4;
}

// HitBox(); Its hit box in the room (sub_0803E2B0's: its center's offset and half sizes, turned with its facing);
static void HitBox(struct ObjectBase *b, s32 *r) {
    s32 x = b->x + ((b->flags & 1) ? -(b->unk38 + b->unk3A) : b->unk38 + b->unk3A) * 0x100, y = b->y + (b->unk39 + b->unk3B) * 0x100;

    r[0] = x - b->unk3A * 0x100;
    r[2] = x + b->unk3A * 0x100;
    r[1] = y - b->unk3B * 0x100;
    r[3] = y + b->unk3B * 0x100;
}

// SqueakTouchedChest(); A closed chest no one holds touching its box (a chest's touch, kind 0x1001);
struct Chest *SqueakTouchedChest(struct SqueakObject *s) {
    s32 r[4];
    u8 i;

    HitBox(&s->obj.base, r);
    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];
        struct Object *obj;

        if (t->main != ObjectMain) continue;
        obj = TaskGetStructPtr(t);
        if (!ChestClosed(s, obj) || ((struct Chest *)obj)->unkDC) continue;
        if (r[2] > obj->base.x + obj->base.unk3C * 0x100 && r[0] < obj->base.x + obj->base.unk3E * 0x100
            && r[3] > obj->base.y + obj->base.unk3D * 0x100 && r[1] < obj->base.y + obj->base.unk3F * 0x100)
            return (struct Chest *)obj;
    }
    return NULL;
}

/* Carry(); Its vtable +0x78 (0x020E9640) and the held thing's +0x10 (0x02012FB0): the chest it holds at (dx, dy) from it (Squeak
 * Squad's units, x ahead: turned with it), its feet there, plus its sway: behind its moves 1.5 pixels a frame, 6 at most, back 0.5 a
 * frame once it stops (at once within 1.5); facing as it; */
// (a sway: behind the move, back once it stops, at most SWAY_MAX);
static s32 Sway(s32 sway, s32 v) {
    if (v > 0) sway -= SWAY;
    else if (v < 0) sway += SWAY;
    else if (sway > SWAY) sway -= SWAY_BACK;
    else if (sway < -SWAY) sway += SWAY_BACK;
    else sway = 0;
    if (sway > SWAY_MAX) sway = SWAY_MAX;
    if (sway < -SWAY_MAX) sway = -SWAY_MAX;
    return sway;
}

/* SqueakCarryChest(); The chest `*held` at (dx, dy) from its carrier, plus its sway (sway[2], the carrier's: Squeakers' 0x240,
 * Spinni's 0x22C); */
void SqueakCarryChest(struct SqueakObject *s, struct Chest **held, s32 *sway, s32 dx, s32 dy) {
    struct Chest *chest = SqueakHeldChest(held);
    s32 x, y;

    if (chest == NULL) return;
    sway[0] = Sway(sway[0], s->vx);
    sway[1] = Sway(sway[1], s->vy);
    x = dx + sway[0];
    y = dy + sway[1];
    chest->obj.base.x = s->obj.base.x + SQUEAK_SPEED(s->obj.base.flags & 1 ? -x : x);
    chest->obj.base.y = s->obj.base.y - SQUEAK_SPEED(y) - chest->obj.base.unk3F * 0x100;
    chest->obj.base.flags = (chest->obj.base.flags & ~1) | (s->obj.base.flags & 1);
}

static void Carry(struct SqueakObject *s, s32 dx, s32 dy) {
    SqueakCarryChest(s, &CHEST(s), &SWAY_X(s), dx, dy);
}

// SqueakTakeChest(); 0x020A2BB0: `*held` the chest (its own function kept, ChestHeld run instead), its sway still;
void SqueakTakeChest(struct Chest **held, s32 *sway, struct Chest *chest) {
    *held = chest;
    chest->unkDC = (u32)chest->obj.unk78;
    chest->obj.unk78 = ChestHeld;
    sway[0] = sway[1] = 0;
}

static void Take(struct SqueakObject *s, struct Chest *chest) {
    SqueakTakeChest(&CHEST(s), &SWAY_X(s), chest);
}

// SqueakDropChest(); 0x020A2A48: its carrier lets go of the chest `*held` where it is (Squeak Squad's: at its spot), falling (ChestDropped);
void SqueakDropChest(struct SqueakObject *s, struct Chest **held) {
    struct Chest *chest = SqueakHeldChest(held);

    if (chest == NULL) return;
    *held = NULL;
    chest->obj.base.x = s->obj.base.x;
    chest->obj.base.y = s->obj.base.y - chest->obj.base.unk3F * 0x100;
    chest->obj.unkA0 = 0;
    chest->obj.unk78 = ChestDropped;
}

static void Drop(struct SqueakObject *s) {
    SqueakDropChest(s, &CHEST(s));
}

/* Treat(); 0x020E9528 (hit: one time in five, not within 40 frames of the last): Squeak Squad's foods 9-14 popping up from it at 2.2,
 * KatAM's small food here (one of its five looks), as Boxy's (subtype2 2); */
static void Treat(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (TREAT_TIMER(s) > 0 || Random() >= TREAT_CHANCE) return;
    TREAT_TIMER(s) = TREAT_WAIT;
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, OBJ_SMALL_FOOD, RandLessThan(5),
                            0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Gravity(); Its vtable +0x40 (0x020A23B4): on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
        return;
    }
    s->ay = -GRAVITY;
    if (s->vy < MAX_FALL) {
        s->vy = MAX_FALL;
        s->ay = 0;
    }
}

// Still(); Accelerations, caps and friction off (the speeds stay);
static void Still(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// Moving(); A frame of its states: gravity (`fall`), its move, its chest after it;
static void Moving(struct SqueakObject *s, bool8 fall) {
    if (fall) Gravity(s);
    SqueakMove(s);
    Carry(s, HOLD_X, HOLD_Y);
}

// Carrying(); Its animation, with a chest the carrying one;
static void Play(struct SqueakObject *s, u8 anim, u8 carrying) {
    SqueakPlay(s, CHEST(s) ? carrying : anim);
}

// Escaping(); Its pad's escape (pad 5, 5);
static bool32 Escaping(struct SqueakObject *s) {
    if (s->padY == PAD_ESCAPE && s->padDir == PAD_ESCAPE) {
        SqueakSetState(s, BODY_ESCAPE_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// AirControl(); 0x020EC0F8: back turns it round; forward, HOP_SPEED (no cap); none, slowed HOP_SPEED a frame;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = HOP_SPEED;
        s->maxVx = 0;
    } else {
        s->ax = 0;
        s->fx = HOP_SPEED;
    }
}

// ThrowBomb(); 0x020E97DC: a bomb 8 pixels ahead, 20 up;
static void ThrowBomb(struct SqueakObject *s, u8 bomb) {
    SqueakSpawn(s, SQUEAK_SQUEAKER_BOMB, bomb, BOMB_X, BOMB_Y);
}

static bool32 Body(struct SqueakObject *s) {
    s32 x, y;

    switch (s->state) {
    case BODY_START:        // 0x020EBEA0 (vtable +0x3C);
        Play(s, ANIM_STAND, ANIM_STAND_CARRY);
        SqueakStop(s);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x020EBCB4: B throws; its pad held 4 frames, it hops; off the ground, it falls;
        if (Escaping(s)) return TRUE;
        if (s->padB) {
            SqueakSetState(s, Color(s) == BLUE ? BODY_BIG_PULL : BODY_PULL);
            return TRUE;
        }
        if (s->padDir == 0 && s->padY < 1 && s->padA == 0) {
            COUNT(s) = 0;
        } else {
            if (s->padDir < 0) SqueakTurnAround(s);
            if (COUNT(s) > 3) {
                SqueakSetState(s, BODY_HOP);
                return TRUE;
            }
            COUNT(s)++;
        }
        Moving(s, FALSE);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_DROP_ENTRY);
        break;
    case BODY_HOP:          // 0x020EBBFC;
        Play(s, ANIM_HOP, ANIM_HOP_CARRY);
        if (s->vy < 0) s->vy = 0;
        s->ax = s->ay = s->fx = s->fy = 0;
        SqueakSetState(s, BODY_HOP_WAIT);
        return TRUE;
    case BODY_HOP_WAIT:     // 0x020EBA8C;
        if (Escaping(s)) return TRUE;
        if (!s->animEnded) {
            Moving(s, TRUE);
            break;
        }
        Play(s, ANIM_CROUCH, ANIM_CROUCH_CARRY);
        COUNT(s) = CROUCH_TIME;
        SqueakSetState(s, BODY_CROUCH);
        return TRUE;
    case BODY_CROUCH:       // 0x020EB8AC: 4 frames (and one), then off: low forward; up held, high (straight up unless forward too);
        if (Escaping(s)) return TRUE;
        if (COUNT(s)-- >= 0) {
            Moving(s, TRUE);
            break;
        }
        Play(s, ANIM_JUMP, ANIM_JUMP_CARRY);
        if (s->padY < 1) {
            s->vx = HOP_SPEED;
            s->vy = HOP_LOW;
        } else {
            s->vx = s->padDir < 1 ? 0 : HOP_SPEED;
            s->vy = HOP_HIGH;
        }
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_RISE);
        return TRUE;
    case BODY_RISE:         // 0x020EB794;
        if (Escaping(s)) return TRUE;
        Moving(s, FALSE);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x020EB6CC;
        Play(s, ANIM_FALL, ANIM_FALL_CARRY);
        Still(s);
        s->ay = -GRAVITY;
        COUNT(s) = FALL_TIME;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020EB500: after 10 frames A hops again;
        if (COUNT(s) < 1) {
            if (s->padA) {
                if (s->padDir < 0) SqueakTurnAround(s);
                SqueakSetState(s, BODY_HOP);
                return TRUE;
            }
        } else {
            COUNT(s)--;
        }
        if (s->vy < MAX_FALL + 1) {
            s->vy = MAX_FALL;
            s->ay = 0;
        }
        Moving(s, FALSE);
        if (SqueakOnGround(s)) {
            Play(s, ANIM_LAND, ANIM_LAND_CARRY);
            SqueakStop(s);
            SqueakSetState(s, BODY_LAND);
        }
        break;
    case BODY_LAND:         // 0x020EB458;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_DROP_ENTRY:   // 0x020EB390: off a ledge;
        Play(s, ANIM_FALL, ANIM_FALL_CARRY);
        Still(s);
        s->ay = -GRAVITY;
        COUNT(s) = FALL_TIME;
        SqueakSetState(s, BODY_DROP);
        return TRUE;
    case BODY_DROP:         // 0x020EB17C;
        if (Escaping(s)) return TRUE;
        AirControl(s);
        if (s->vy < MAX_FALL + 1) {
            s->vy = MAX_FALL;
            s->ay = 0;
        }
        if (COUNT(s) < 1) {
            if (s->padA) {
                if (s->padDir < 0) SqueakTurnAround(s);
                SqueakSetState(s, BODY_HOP);
                return TRUE;
            }
        } else {
            COUNT(s)--;
        }
        Moving(s, FALSE);
        if (SqueakOnGround(s)) {
            Play(s, ANIM_LAND, ANIM_LAND_CARRY);
            SqueakStop(s);
            SqueakSetState(s, BODY_DROP_LAND);
        }
        break;
    case BODY_DROP_LAND:    // 0x020EB0D4;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_PULL:         // 0x020E9EC0;
        SqueakPlay(s, ANIM_PULL);
        SqueakStop(s);
        SqueakSetState(s, BODY_HOLD);
        return TRUE;
    case BODY_HOLD:         // 0x020E9D98: held while B is;
        if (!s->padB) {
            SqueakPlay(s, ANIM_WIND);
            SqueakSetState(s, BODY_WIND);
            if (s->padDir < 0) SqueakTurnAround(s);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_WIND:         // 0x020E9C80: once wound up, it throws;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_THROW);
            ThrowBomb(s, BOMB_SMALL);
            SqueakSetState(s, BODY_THROWN);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_THROWN:       // 0x020E9BD8;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_BIG_PULL:     // 0x020E9AE8;
        SqueakPlay(s, ANIM_BIG_PULL);
        SqueakStop(s);
        SqueakSetState(s, BODY_BIG_HOLD);
        return TRUE;
    case BODY_BIG_HOLD:     // 0x020E99C0;
        if (!s->padB) {
            SqueakPlay(s, ANIM_BIG_WIND);
            SqueakSetState(s, BODY_BIG_WIND);
            if (s->padDir < 0) SqueakTurnAround(s);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_BIG_WIND:     // 0x020E98A8;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_BIG_THROW);
            ThrowBomb(s, BOMB_BIG);
            SqueakSetState(s, BODY_BIG_THROWN);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_BIG_THROWN:   // 0x020E9800;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Moving(s, TRUE);
        break;
    case BODY_TAKE:         // 0x020EC008: it stops (PAUSE frames), then on (PAUSE_NEXT);
        SqueakPlay(s, ANIM_TAKE);
        SqueakStop(s);
        SqueakSetState(s, BODY_PAUSE);
        return TRUE;
    case BODY_PAUSE:        // 0x020EBF68 (its chest 16 pixels behind it meanwhile);
        if (PAUSE(s) < 0) {
            PAUSE(s) = 0;
            SqueakSetState(s, PAUSE_NEXT(s));
            return TRUE;
        }
        PAUSE(s)--;
        SqueakMove(s);
        Carry(s, HOLD_X, HOLD_Y);
        Carry(s, STOP_X, 0);
        break;
    case BODY_ESCAPE_ENTRY: // 0x020EAF8C: to its hole;
        SqueakPlay(s, ANIM_STAND_CARRY);
        SqueakStop(s);
        TARGET_X(s) = HOME_X(s);
        TARGET_Y(s) = HOME_Y(s);
        SqueakSetState(s, BODY_TO_HOLE);
        return TRUE;
    case BODY_TO_HOLE:      // 0x020EAE0C: a pixel a frame each way, till within 2 across and 1 up or down: it stops 96 frames;
        x = SQ_X(&s->obj);
        y = SQ_Y(&s->obj);
        if (x > TARGET_X(s) - 0x2000 && x < TARGET_X(s) + 0x2000 && y > TARGET_Y(s) - 0x1000 && y < TARGET_Y(s) + 0x1000) {
            SqueakStop(s);
            PAUSE(s) = ESCAPE_TIME;
            PAUSE_NEXT(s) = BODY_ESCAPE;
            SqueakSetState(s, BODY_TAKE);
            break;
        }
        x >>= 12;
        y >>= 12;
        if (TARGET_X(s) >> 12 < x) x--;
        else if (x < TARGET_X(s) >> 12) x++;
        if (TARGET_Y(s) >> 12 < y) y--;
        else if (y < TARGET_Y(s) >> 12) y++;
        s->obj.base.x = x << 8;
        s->obj.base.y = -y << 8;
        Moving(s, FALSE);
        break;
    case BODY_ESCAPE:       // 0x020EAD78;
        SqueakPlay(s, ANIM_ESCAPE);
        SqueakSetState(s, BODY_ESCAPING);
        return TRUE;
    case BODY_ESCAPING:     // 0x020EACF0: into its hole, its chest with it (0x020A2968);
        if (s->animEnded) {
            if (CHEST(s)) CHEST(s)->obj.base.flags |= 0x1000;
            CHEST(s) = NULL;
            s->obj.base.flags |= 0x1000;
            break;
        }
        Moving(s, FALSE);
        break;
    }
    return FALSE;
}

// ClearPad(); 0x020E8E1C;
static void ClearPad(struct SqueakObject *s) {
    s->padDir = s->padY = s->padA = s->padB = 0;
}

/* FaceKirby(); 0x020EC708: Kirby behind it, turn round (its pad back; KatAM's flag 1 its facing left); */
static void FaceKirby(struct SqueakObject *s) {
    s32 kirby = SQ_X(s->obj.kirby3), x = SQ_X(&s->obj);

    if ((kirby > x && (s->obj.base.flags & 1)) || (kirby < x && !(s->obj.base.flags & 1))) s->padDir = -1;
}

/* Steer(); 0x020E8D0C: its pad toward (tx, ty) from its spot (0x020E8E2C, 0x020E8E48: forward or back as it faces), beyond NEAR;
 * on the ground or under a ceiling (flags 0x4000, 0x2000), toward it across at any distance; a wall ahead (0x800), A and up to
 * a target above, down to one below; */
static void Steer(struct SqueakObject *s, s32 tx, s32 ty) {
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj), dx;
    bool8 left = s->obj.base.flags & 1;

    if (tx < x + NEAR && x - NEAR < tx) s->padDir = 0;
    else if (tx < x) s->padDir = left ? 1 : -1;
    else if (x < tx) s->padDir = left ? -1 : 1;
    if (ty < y + NEAR && y - NEAR < ty) {
        s->padY = 0;
        s->padA = 0;
    } else if (y < ty) {
        s->padY = 1;
        dx = x < tx ? tx - x : x - tx;
        if (dx < ty - y) s->padA = 1;
    } else {
        s->padA = 0;
        s->padY = -1;
    }
    if ((s->terrain & 4) || SqueakCeiling(s)) {
        if (tx < x) s->padDir = left ? 1 : -1;
        else if (x < tx) s->padDir = left ? -1 : 1;
    }
    if (SqueakWallAhead(s)) {
        if (ty < y) {
            s->padA = 0;
            s->padY = -1;
        } else if (y < ty) {
            s->padA = 1;
            s->padY = 1;
        }
    }
}

// Think(); 0x020E8F9C: its target; Kirby near, the wait to throw; its pad;
static void Think(struct SqueakObject *s) {
    struct Chest *chest = NULL;
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj), kx = SQ_X(s->obj.kirby3), ky = SQ_Y(s->obj.kirby3), tx, ty;
    bool8 any = FALSE, steer = TRUE, attack;
    u8 aim, color = Color(s);
    u32 roll;

    if (!CHEST(s)) chest = SqueakNearestChest(s, &any);
    if (CHEST(s) || !any) {
        aim = AIM_HOLE;
        tx = HOME_X(s);
        ty = HOME_Y(s);
    } else if (chest) {
        aim = AIM_CHEST;
        SqueakChestFeet(chest, &tx, &ty);
    } else {
        aim = AIM_KIRBY;
        tx = kx;
        ty = ky;
    }
    if (color != YELLOW && !CHEST(s)) {
        attack = TRUE;
        if (color == BLUE) {
            if (BLUE_TIMER(s) < 1) {
                BLUE_TIMER(s) = BLUE_WAIT;
            } else {
                attack = FALSE;
                BLUE_TIMER(s)--;
            }
        }
        if (attack && kx < x + SIGHT_X && x - SIGHT_X < kx && ky < y + SIGHT_Y && y - SIGHT_Y < ky) {
            roll = Random();
            if (roll < 50) WAIT(s) = 60;
            else if (roll < 60) WAIT(s) = 0;
            else if (roll < 70) WAIT(s) = 20;
            else if (roll < 80) WAIT(s) = 30;
            else WAIT(s) = 45;
            s->brain = BRAIN_ATTACK;
        }
    }
    // (blue and green ones: only to a chest near them);
    if (color == BLUE || color == GREEN) {
        steer = aim == AIM_CHEST && x > tx - CHEST_X && x < tx + CHEST_X && y > ty - CHEST_Y && y < ty + CHEST_Y;
    }
    if (CHEST(s)) steer = TRUE;
    if (steer) Steer(s, tx, ty);
    else ClearPad(s);
    // At its hole with a chest: its escape (0x0221AE04, the stage's leave to: always here);
    if (aim == AIM_HOLE && CHEST(s) && x > tx - HOLE && x < tx + HOLE && y > ty - HOLE && y < ty + HOLE) {
        ClearPad(s);
        s->padDir = s->padY = PAD_ESCAPE;
    }
}

static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_START:       // 0x020E9378;
        FaceKirby(s);
        s->brain = BRAIN_INIT;
        break;
    case BRAIN_INIT:        // 0x020E931C;
        s->padDir = 0;
        s->brain = BRAIN_THINK;
        break;
    case BRAIN_THINK:       // 0x020E8F9C;
        Think(s);
        break;
    case BRAIN_ATTACK:      // 0x020E8F84;
        FaceKirby(s);
        s->brain = BRAIN_WAIT;
        break;
    case BRAIN_WAIT:        // 0x020E8F04: on the ground, its wait; then B, down, facing Kirby;
        if (!SqueakOnGround(s)) {
            s->padY = s->padB = s->padA = 0;
            break;
        }
        s->padDir = 0;
        if (WAIT(s) < 1) {
            WAIT(s) = 0;
            s->padY = -1;
            FaceKirby(s);
            s->padB = 1;
            s->brain = BRAIN_PRESS;
            break;
        }
        WAIT(s)--;
        break;
    case BRAIN_PRESS:       // 0x020E8E7C: B let go (its wait is over), thinking again;
        s->padDir = 0;
        FaceKirby(s);
        if (SqueakWallAhead(s)) {
            s->padB = 0;
            s->padY = 0;
            s->brain = BRAIN_THINK;
        }
        if (WAIT(s) < 1) {
            s->padB = 0;
            s->padY = 0;
            s->brain = BRAIN_THINK;
            break;
        }
        WAIT(s)--;
        break;
    }
}

// SqueakerMain(); Its brain, its body, then a closed chest it touches (its touch, 0x020EC168: taken unless it just dropped one);
static void SqueakerMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Chest *chest;

    if (TREAT_TIMER(s) > 0) TREAT_TIMER(s)--;
    Held(s);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    if (!CHEST(s) && s->state != BODY_ESCAPING && (chest = SqueakTouchedChest(s)) != NULL) {
        if (REGRAB_COUNT(s) > 0) {
            REGRAB_COUNT(s)--;
        } else {
            Take(s, chest);
            PAUSE(s) = TAKE_TIME;
            PAUSE_NEXT(s) = BODY_START;
            SqueakSetState(s, BODY_TAKE);
        }
    }
    SqueakSetBoxes(s, &sSqueakerBoxData, HARM(s), &BOX(s));
}

/* SqueakerWatch(); Its pre-callback: KatAM's hit put it out of its own state (unk78 not SqueakerMain), or it's beaten or gone: it
 * drops its chest (0x020EC168, 0x020EC3C0; then 10 touches before it takes one again), and a hit's first frame, maybe a treat; */
static void SqueakerWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (obj->unk78 != SqueakerMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000)) {
        if (CHEST(s)) {
            Drop(s);
            REGRAB_COUNT(s) = REGRAB;
        }
        if (!HURT(s) && !(obj->base.flags & 0x1000)) Treat(s);
        HURT(s) = TRUE;
    } else {
        HURT(s) = FALSE;
    }
}

// SqueakerGone(); Its end (squeak.c's SqueakDestroy: KatAM takes it out of the room off camera, or the room's over): it drops its chest;
void SqueakerGone(struct SqueakObject *s) {
    Drop(s);
}

void SqueakerStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, SqueakerMain);
    obj->unk7C = SqueakerWatch;
    if (!s->started) {
        SqueakUsePalette(s, Color(s));
        HOME_X(s) = SQ_X(obj);
        HOME_Y(s) = SQ_Y(obj);
        s->brain = BRAIN_START;
    }
    SqueakSetState(s, BODY_START);
}

/* SqueakerInit(); Its box (chun.rca's: 8 pixels either side, 2 to 19 up; set each frame, SqueakSetBoxes()), facing left (0x020EC504's
 * flag 4); */
void SqueakerInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -19, 8, -2);
    ObjectSetBounds(&s->obj.base, -6, -17, 6, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}

/* Its bombs (0x020ED208): thrown (0x020ECF88, 0x020ECC60), flying (0x020ECE44, 0x020ECB48); the small one blasts where it lands or
 * hits a wall (0x020ECE14); the big one sits lit (0x020EC9F4), then blows up round its spot (0x020EC7DC); out of the stage, gone; */
enum { BOMB_START, BOMB_FLY, BOMB_BLAST, BOMB_BIG_FLY, BOMB_WICK, BOMB_BOOM };

/* Blast(); Unseen, its blast box (anim 41's), a burst there and a shake (0x020B48E0's type 6, 0x02035FC4); Kirby's body doesn't beat
 * the blast (KatAM's unk5C 0x80): it goes on, as Squeak Squad's; */
static void Blast(struct SqueakObject *s) {
    s->obj.base.unk5C |= 0x80;
    SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
    RequestScreenShake(SHAKE, &s->obj.base);
}

static bool32 Bomb(struct SqueakObject *s) {
    const s32 *launch;
    s32 i;

    switch (s->state) {
    case BOMB_START:
        launch = sBombLaunch[Random() / 20];
        s->vx = launch[0];
        s->vy = launch[1];
        s->ay = -GRAVITY;
        SqueakPlay(s, s->variant == BOMB_BIG ? BOMB_ANIM_BIG : BOMB_ANIM_SMALL);
        B_COUNT(s) = 0;
        SqueakSetState(s, s->variant == BOMB_BIG ? BOMB_BIG_FLY : BOMB_FLY);
        return TRUE;
    case BOMB_FLY:          // 0x020ECE44: on the ground or any terrain met, it blasts;
        if (s->vy < BOMB_MAX_FALL + 1) {
            s->vy = BOMB_MAX_FALL;
            s->ay = 0;
        }
        SqueakMove(s);
        if (!s->terrain && !SqueakOnGround(s)) {
            if (!SqueakInLevel(s)) s->obj.base.flags |= 0x1000;
            break;
        }
        SqueakStop(s);
        SqueakPlay(s, BOMB_ANIM_BLAST);
        B_COUNT(s) = 0;
        s->obj.base.flags |= 0x400;
        Blast(s);
        SqueakSetState(s, BOMB_BLAST);
        break;
    case BOMB_BLAST:        // 0x020ECE14: 9 frames;
        SqueakMove(s);
        if (B_COUNT(s) < BLAST_TIME) B_COUNT(s)++;
        else s->obj.base.flags |= 0x1000;
        break;
    case BOMB_BIG_FLY:      // 0x020ECB48: landed, it sits lit;
        if (s->vy < BOMB_MAX_FALL + 1) {
            s->vy = BOMB_MAX_FALL;
            s->ay = 0;
        }
        SqueakMove(s);
        if (SqueakWallAhead(s)) s->vx = 0;
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, BOMB_ANIM_WICK);
            SqueakSetState(s, BOMB_WICK);
        } else if (!SqueakInLevel(s)) {
            s->obj.base.flags |= 0x1000;
        }
        break;
    case BOMB_WICK:         // 0x020EC9F4: its wick burnt down (anim 40's 192 frames), it blows up;
        if (s->vy < BOMB_MAX_FALL + 1) {
            s->vy = BOMB_MAX_FALL;
            s->ay = 0;
        }
        if (SqueakWallAhead(s)) s->vx = 0;
        if (s->animEnded) {
            SqueakStop(s);
            SqueakPlay(s, BOMB_ANIM_BLAST);
            B_COUNT(s) = 0;
            B_SPOT_X(s) = s->obj.base.x;
            B_SPOT_Y(s) = s->obj.base.y;
            s->obj.base.flags |= 0x400;
            s->obj.base.unk5C |= 0x80;
            SqueakSetState(s, BOMB_BOOM);
            return TRUE;
        }
        SqueakMove(s);
        if (!SqueakInLevel(s)) s->obj.base.flags |= 0x1000;
        break;
    case BOMB_BOOM:         // 0x020EC7DC: its blast round its spot, a burst and a shake every 7 frames (9 in all), 60 frames;
        i = B_COUNT(s) / WICK_FRAMES;
        s->obj.base.x = B_SPOT_X(s) + (i < 8 ? SQUEAK_SPEED(sBoom[i][0]) : 0);
        s->obj.base.y = B_SPOT_Y(s) - (i < 8 ? SQUEAK_SPEED(sBoom[i][1]) : 0);
        if (B_COUNT(s) % WICK_FRAMES == 0) Blast(s);
        SqueakMove(s);
        if (B_COUNT(s) < BOOM_TIME) {
            B_COUNT(s)++;
        } else {
            SqueakStop(s);
            s->obj.base.flags |= 0x1000;
        }
        break;
    }
    return FALSE;
}

static void SqueakerBombMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Bomb);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sBombBoxData, B_HARM(s), &B_BOX(s));
}

void SqueakerBombStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, SqueakerBombMain);
    if (!s->started) SqueakSetState(s, BOMB_START);
}

/* SqueakerBombInit(); Its box (chun.rca's: the small one 6 pixels round its middle, the big one 13; set each frame), its terrain box
 * as big; its spot its middle (0x020ED208's flags 0x20000, 0x40); */
void SqueakerBombInit(struct SqueakObject *s) {
    if (s->variant == BOMB_BIG) {
        sub_0803E2B0(&s->obj.base, -12, -13, 14, 13);
        ObjectSetBounds(&s->obj.base, -12, -13, 12, 13);
    } else {
        sub_0803E2B0(&s->obj.base, -6, -6, 7, 6);
        ObjectSetBounds(&s->obj.base, -5, -6, 5, 6);
    }
    B_HARM(s) = s->obj.base.unk68;
}
