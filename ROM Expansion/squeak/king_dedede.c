/* King Dedede (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x38 (its body 0x020E7E58, vtable 0x0217D6A4: +0x3C its
 * start 0x020E7758, +0x34 per frame 0x020E7F80, +0x2C its catch 0x020E7C5C, +0x30 its hit 0x020E79B0; no brain: 0x0203A9C4(0, 0)),
 * as SQUEAK_KING_DEDEDE, a boss on the mid-bosses' framework (mini-boss type 0x40: the camera held on its area, the HP bar, the music,
 * KatAM's defeat). Values from param/prmBB02Ml0.kgb (its HP 200: here KatAM's bosses' 60 for one player); speeds 20.12 a frame, Y up;
 * drawn with enemy/dedede (all its animations, their numbers kept; its .mca's inhale and summon; the pieces in its palette 1 a layer);
 * its boxes from dedede.rca. Recorded on the DS (its intro, idles, hammer walks and swings, the summon: the states' frames).
 * • Its fight: standing 90 frames (0x020E78E8, 0x020E7878, 0x020E7828), then (0x020E7758) it picks a move (0x020E749C): on the ground,
 *   turned toward Kirby, after 30 frames a roll of 0-99 (rolled again while it picks nothing): Kirby within 64 pixels across, 10 the
 *   hammer walk, 15 the hammer jump, 40 the inhale; else 30 a walk, 10 a jump, 20 the hammer walk, 10 the hammer jump, 15 the float,
 *   else the summon while fewer than 5 of its Waddle Dees are out (a walk if not). Each move ends standing 60 frames (0x020E73F0).
 * • Walk (0x020E72D4, 0x020E7194): 0.7 a frame till within 50 pixels of Kirby or 78 pixels from the arena's side (Squeak Squad's 256
 *   pixels from the area's left: here KatAM's camera view, 240). Jump (0x020E70E8, 0x020E6FB4, 0x020E6F08, 0x020E6E18): crouched 30
 *   frames, up 4.2 (half the time 1.2 forward), 0.12 gravity; landed, a big puff and two stars 32 pixels either side, the screen
 *   shaking, 30 frames. Off a ledge it falls (0x020E6D10, 0x020E6C28: to 2.4) and lands as from a jump.
 * • Hammer walk (0x020E6AF8, 0x020E6968, 0x020E68B8): turned to Kirby, 0.7 a frame with its hammer up till within 48 pixels of him or
 *   the arena's side, then its swing (56 frames): its 32nd frame a star rolls off 48 pixels ahead, the screen shaking. Hammer jump
 *   (0x020E680C, 0x020E6720, 0x020E6674, 0x020E6624, 0x020E6568, 0x020E650C): crouched 30 frames, up 4.2 in place, swung down from the
 *   top, landed a star 48 pixels ahead and the screen shaking, 24 frames.
 * • Float (0x020E6474, 0x020E6394, 0x020E62C8, 0x020E60EC, 0x020E6074, 0x020E5FA8, 0x020E5F54): it puffs up (110 frames), jumps 3.4
 *   up, and from the top floats after Kirby 200 frames (0.05 a frame each way, to 0.6; bouncing 2.0 off the ground); then it lets its
 *   air out (a puff 24 pixels ahead, 16 up: 0x0212EAA0, SQUEAK_DEDEDE_PUFF, 3.0 slowing 0.1, 30 frames, harmless: its animation has no
 *   box) and drops.
 * • Inhale (0x020E5D30, 0x020E5C78, 0x020E5C1C): its mouth open 40 frames, then it inhales 84 frames (its .mca's composite 0); Kirby
 *   in its inhale's box (dedede.rca's: 66 pixels ahead; 0x020E7C5C) is caught, drawn to its mouth (8 ahead, 12 up: 4.0 and 2.4 a
 *   frame; 0x020E598C), chewed 40 frames (0x020E580C), and spat out toward the arena's middle in a star (0x0212389C's kind 0: the
 *   framework's SQUEAK_STAR_SPIT, which lets him out hurt past the arena's side; 0x020E56EC), its pose 60 frames.
 * • Summon (0x020E5EB0, 0x020E5DC8): it calls (108 frames, its composite 1), and a Waddle Dee floats down with a parasol from the
 *   arena's top over Kirby (Squeak Squad's spawn 0, byte 8 4: KatAM's Waddle Dee 3), facing the arena's middle.
 * • Hit (0x020E79B0): 60 frames safe after each (KatAM's hits then undone), its hurt pose 20 frames (0x020E5654, 0x020E5604); a Kirby
 *   it holds is let out.
 * Not ported: its sounds, its own defeat (0x020E5530-0x020E52DC: KatAM's mini-boss defeat), Squeak Squad's hit flash and shakes
 * (KatAM's), its boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/object_types.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (dedede.cea's, all kept: their numbers);
enum {
    ANIM_IDLE = 1, ANIM_WALK, ANIM_CROUCH, ANIM_JUMP, ANIM_FALL, ANIM_FALLING, ANIM_LANDED, ANIM_HAMMER_WALK, ANIM_SWING, ANIM_HAMMER_CROUCH,
    ANIM_HAMMER_UP, ANIM_SWING_DOWN, ANIM_INFLATE = 15, ANIM_PUFFED, ANIM_FLOAT, ANIM_EXHALE, ANIM_FLOAT_LANDED, ANIM_PUFF, ANIM_INHALE,
    ANIM_CHEW = 29, ANIM_SPIT, ANIM_HURT = 38,
};
// Its composite animations (dedede.mca's);
enum { COMPOSITE_INHALE, COMPOSITE_SUMMON };

// prmBB02Ml0.kgb;
#define INTRO 90                        // +0x08;
#define SAFE_TIME 60                    // +0x0C;
#define WALK_SPEED 0xB33                // +0x14;
#define IDLE_TIME 60                    // +0x18;
#define FLOAT_VY 0x3666                 // +0x20..+0x3C: its float;
#define FLOAT_GRAVITY 0x19A
#define FLOAT_AX 0xCD
#define FLOAT_AY 0xCD
#define FLOAT_VX_CAP 0x99A
#define FLOAT_VY_CAP 0x99A
#define FLOAT_BOUNCE 0x2000
#define FLOAT_TIME 200
#define MOUTH_X 0x8000                  // +0x50..+0x5C: a catch drawn to its mouth;
#define MOUTH_Y 0xC000
#define PULL_X 0x4000
#define PULL_Y 0x2666
#define SPIT_X 0x8000                   // +0x60, +0x64;
#define SPIT_Y 0x10000
#define HAMMER_REACH 0x30000            // +0x6C..+0x78;
#define HAMMER_HIT 32
#define HJUMP_CROUCH 30                 // +0x80..+0x8C: its hammer jump;
#define HJUMP_VY 0x4333
#define HJUMP_GRAVITY 0x1EC
#define HJUMP_WAIT 24
#define IMPACT_X 0x30000                // +0x9C: where its hammer's star rolls off;
#define JUMP_CROUCH 30                  // +0xAC..+0xBC: its jump;
#define JUMP_VX 0x1334
#define JUMP_VY 0x4333
#define GRAVITY 0x1EC
#define JUMP_WAIT 30
#define NEAR 0x40000                    // +0xCC..+0xE8: its picks;
#define NEAR_HAMMER 10
#define NEAR_HJUMP 15
#define NEAR_INHALE 65                  // (its code's 0x41: the inhale under it);
#define FAR_WALK 30
#define FAR_JUMP 10
#define FAR_HAMMER 20
#define FAR_HJUMP 10
#define FAR_FLOAT 15
#define PUFF_SPEED 0x3000               // +0x40..+0x48: the puff;
#define PUFF_SLOW 0x19A
#define PUFF_TIME 30
// Its code's;
#define PICK_WAIT 30                    // 0x020E749C: frames facing Kirby before it picks;
#define MAX_FALL (-0x2666)              // 0x020E6C28;
#define LAND_TIME 30
#define WALK_STOP 0x32000               // 0x020E7194: within 50 pixels of Kirby;
#define SIDE 78                         // 0x020E7194, 0x020E6968: the area's left plus 78, plus 178 (its 256 less 78);
#define FACE_ZONE 0x8000                // 0x020E60EC: 8 pixels;
#define STAR_LAND_X 0x20000             // 0x02021864: the landing's stars;
#define PUFF_X 0x18000                  // 0x020E60EC: the puff;
#define PUFF_Y 0x10000
#define DEES 5                          // 0x020E749C: its Waddle Dees out at most;
#define SUMMON_DEE 3                    // KatAM's Waddle Dee with a parasol (Squeak Squad's 4);
#define SHAKE 1                         // KatAM's shortest timed shake, 14 frames (Squeak Squad's 2 for 8; KatAM's 4-6 never stop);

enum {
    BODY_INTRO, BODY_START, BODY_PICK, BODY_IDLE_ENTRY, BODY_IDLE, BODY_WALK_ENTRY, BODY_WALK, BODY_JUMP_ENTRY, BODY_JUMP_CROUCH,
    BODY_JUMP_RISE, BODY_JUMP_FALL, BODY_LANDED, BODY_FALL_ENTRY, BODY_FALL, BODY_HAMMER_ENTRY, BODY_HAMMER_WALK, BODY_HAMMER_SWING,
    BODY_HJUMP_ENTRY, BODY_HJUMP_CROUCH, BODY_HJUMP_RISE, BODY_HJUMP_SWING, BODY_HJUMP_FALL, BODY_FLOAT_ENTRY, BODY_FLOAT_INFLATE,
    BODY_FLOAT_RISE, BODY_FLOAT, BODY_FLOAT_EXHALE, BODY_FLOAT_FALL, BODY_FLOAT_LANDED, BODY_INHALE_ENTRY, BODY_INHALE_OPEN,
    BODY_INHALE, BODY_PULL, BODY_CHEW, BODY_SPIT, BODY_SUMMON_ENTRY, BODY_SUMMON, BODY_HURT_ENTRY, BODY_HURT,
};

#define COUNT(s) ((s)->vars[0])         // Squeak Squad's +0x208;
#define WAIT(s) ((s)->vars[1])          // +0x20C: its pick's wait;
#define HELD(s) ((s)->vars[2])          // The Kirbys it caught (a bit each: +0x164);
#define SAFE(s) ((s)->vars[3])          // +0x1F8: frames left safe after a hit;
#define LAST_HP(s) ((s)->vars[4])
#define HARM(s) ((s)->vars[5])
#define BOX(s) (((u8 *)&(s)->vars[6])[0])
#define SWALLOWED(s) (((u8 *)&(s)->vars[6])[1])
#define DEE(s, i) (((struct Object **)&(s)->vars[7])[i])  // Its Waddle Dees out (vars[7]-vars[11]);
#define P_COUNT(s) ((s)->vars[0])       // The puff's frames left;

// dedede.rca's boxes (boxes.py; the inhale's 14: its catch);
static const struct SqueakBox sDededeBoxes[] = {
    { { -11, -49, 17, -1 }, TRUE, 0 },
    { { -10, -37, 14, -1 }, TRUE, 0 },
    { { -17, -61, 17, -1 }, TRUE, 0 },
    { { -19, -61, 17, -1 }, TRUE, 0 },
    { { -16, -56, 17, -1 }, TRUE, 0 },
    { { -16, -66, 17, -1 }, TRUE, 0 },
    { { -16, -95, 18, -1 }, TRUE, 0 },
    { { -16, -66, 49, -1 }, TRUE, 0 },
    { { -16, -70, 17, -1 }, TRUE, 0 },
    { { -16, -95, 21, -1 }, TRUE, 0 },
    { { -16, -62, 48, -1 }, TRUE, 0 },
    { { -16, -61, 48, -1 }, TRUE, 0 },
    { { -22, -51, 24, 2 }, TRUE, 0 },
    { { -10, -37, 14, -7 }, TRUE, 0 },
    { { -10, -37, 14, -1 }, TRUE, 1, { { -10, -45, 66, -3 } } },
    { { -15, -36, 20, 1 }, FALSE, 0 },
    { { -19, -40, 14, -1 }, TRUE, 0 },
    { { -19, -40, 13, -12 }, FALSE, 0 },
};
#define BOX_INHALE 14
static const struct SqueakBoxFrame sDededeAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sDededeAnim1[] = { { 56, 1 } };
static const struct SqueakBoxFrame sDededeAnim2[] = { { 52, 1 } };
static const struct SqueakBoxFrame sDededeAnim3[] = { { 1, 1 } };
static const struct SqueakBoxFrame sDededeAnim4[] = { { 1, 2 } };
static const struct SqueakBoxFrame sDededeAnim5[] = { { 4, 3 }, { 4, 3 } };
static const struct SqueakBoxFrame sDededeAnim7[] = { { 30, 1 } };
static const struct SqueakBoxFrame sDededeAnim8[] = { { 52, 4 } };
static const struct SqueakBoxFrame sDededeAnim9[] = { { 5, 4 }, { 23, 5 }, { 2, 6 }, { 2, 7 }, { 24, 7 } };
static const struct SqueakBoxFrame sDededeAnim10[] = { { 20, 1 } };
static const struct SqueakBoxFrame sDededeAnim11[] = { { 1, 8 } };
static const struct SqueakBoxFrame sDededeAnim12[] = { { 2, 9 }, { 2, 10 }, { 1, 10 } };
static const struct SqueakBoxFrame sDededeAnim13[] = { { 1, 10 } };
static const struct SqueakBoxFrame sDededeAnim14[] = { { 24, 11 } };
static const struct SqueakBoxFrame sDededeAnim15[] = { { 110, 1 } };
static const struct SqueakBoxFrame sDededeAnim16[] = { { 4, 4 } };
static const struct SqueakBoxFrame sDededeAnim17[] = { { 8, 12 } };
static const struct SqueakBoxFrame sDededeAnim18[] = { { 12, 13 }, { 1, 1 } };
static const struct SqueakBoxFrame sDededeAnim19[] = { { 16, 1 } };
static const struct SqueakBoxFrame sDededeAnim21[] = { { 40, 1 } };
static const struct SqueakBoxFrame sDededeAnim22[] = { { 16, 14 } };
static const struct SqueakBoxFrame sDededeAnim29[] = { { 40, 15 } };
static const struct SqueakBoxFrame sDededeAnim30[] = { { 60, 15 } };
static const struct SqueakBoxFrame sDededeAnim32[] = { { 6, 1 } };
static const struct SqueakBoxFrame sDededeAnim38[] = { { 20, 16 } };
static const struct SqueakBoxFrame sDededeAnim39[] = { { 30, 17 } };
static const struct SqueakBoxFrame sDededeAnim41[] = { { 156, 17 } };
static const struct SqueakBoxAnim sDededeBoxAnims[] = {
    { sDededeAnim0, 1 }, { sDededeAnim1, 1 }, { sDededeAnim2, 1 }, { sDededeAnim3, 1 }, { sDededeAnim4, 1 }, { sDededeAnim5, 2 },
    { sDededeAnim3, 1 }, { sDededeAnim7, 1 }, { sDededeAnim8, 1 }, { sDededeAnim9, 5 }, { sDededeAnim10, 1 }, { sDededeAnim11, 1 },
    { sDededeAnim12, 3 }, { sDededeAnim13, 1 }, { sDededeAnim14, 1 }, { sDededeAnim15, 1 }, { sDededeAnim16, 1 }, { sDededeAnim17, 1 },
    { sDededeAnim18, 2 }, { sDededeAnim19, 1 }, { NULL, 0 }, { sDededeAnim21, 1 }, { sDededeAnim22, 1 }, { NULL, 0 }, { NULL, 0 },
    { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { sDededeAnim29, 1 }, { sDededeAnim30, 1 }, { sDededeAnim21, 1 },
    { sDededeAnim32, 1 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { sDededeAnim38, 1 }, { sDededeAnim39, 1 },
    { sDededeAnim39, 1 }, { sDededeAnim41, 1 },
};
// (dedede.rca's boxes flagged 0x8000 in its chewing and spitting, 29 and 30, and its hurt, 38-41: untouchable);
static const struct SqueakBoxes sDededeBoxData = {
    sDededeBoxes, sDededeBoxAnims, ARRAY_COUNT(sDededeBoxAnims),
    { SQUEAK_ANIM_LO(ANIM_CHEW) | SQUEAK_ANIM_LO(ANIM_SPIT), SQUEAK_ANIM_HI(38) | SQUEAK_ANIM_HI(39) | SQUEAK_ANIM_HI(40) | SQUEAK_ANIM_HI(41) },
};

/* The grab's scripts (bosses/dedede.c's: x ahead, y, priority, frames, Kirby's animation, flags: 0x400 held, 0x40 hidden, 0x200 left
 * where he is, 0x10 KatAM's hurt): drawn to its mouth (Pull() moves him), then hidden in it; let out hurt (a hit, its defeat). A
 * Kirby's script restarts when its holder's unk83 (its frame) changes; */
static const struct Kirby_110 sDrawnIn[] = {
    { 0, 0, -1, 1, 0x4A, 0x600 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sSwallowed[] = {
    { MOUTH_X >> 12, -(MOUTH_Y >> 12), -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sLetOut[] = {
    { -8, 0x30, 0, 1, 0, 0x410 },
    { 0, 0, 0, 0, 0, 0x500 },
};

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// ArenaLeft(); The arena's left (Squeak Squad's area, 0x0220A3E4: here the camera's view, which the fight holds);
static s32 ArenaLeft(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.x;
}

static bool32 FacesLeft(struct SqueakObject *s) {
    return (s->obj.base.flags & 1) != 0;
}

// FaceKirby(); Turned toward Kirby (Kirby right of it, facing left: turned round; else facing right: turned round);
static void FaceKirby(struct SqueakObject *s) {
    if (s->obj.base.x < s->obj.kirby3->base.x ? FacesLeft(s) : !FacesLeft(s)) SqueakTurnAround(s);
}

// Gravity(); 0x020E52A0: in the air GRAVITY a frame; on the ground still;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else {
        s->ay = -GRAVITY;
    }
}

// Puff(); 0x02021864: a puff where it stands (big: two stars either side too);
static void Puff(struct SqueakObject *s, bool8 big) {
    if (big) {
        SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, STAR_LAND_X, 0);
        SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_BEHIND, -STAR_LAND_X, 0);
    }
    SqueakSpawnEffect(s, big ? SQUEAK_FX_LAND_BIG : SQUEAK_FX_LAND, 0, 0, FALSE);
}

// Impact(); 0x020217F8(..., 1): its hammer's star rolling off ahead, the screen shaking;
static void Impact(struct SqueakObject *s) {
    SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_AHEAD, IMPACT_X, 0);
    RequestScreenShake(SHAKE, &s->obj.base);
}

// AtSide(); Walking into the arena's side (78 pixels from it);
static bool32 AtSide(struct SqueakObject *s) {
    s32 x = s->obj.base.x - ArenaLeft(s);

    return FacesLeft(s) ? x < SIDE << 8 : x > (DISPLAY_WIDTH - SIDE) << 8;
}

/* DeesOut(); Its Waddle Dees still about (Squeak Squad counts the stage's enemies: 0x0201B76C); Summon(): one floating down over Kirby
 * from the arena's top (KatAM's Waddle Dee with a parasol), facing the arena's middle; */
static u8 DeesOut(struct SqueakObject *s) {
    u8 i, n = 0;

    for (i = 0; i < DEES; i++) {
        struct Object *dee = DEE(s, i);

        if (dee && dee->base.header.kind && dee->type == OBJ_WADDLE_DEE_1 && dee->base.parent == &s->obj && !(dee->base.flags & 0x1000)
            && dee->unk80 > 0)
            n++;
        else
            DEE(s, i) = NULL;
    }
    return n;
}

static void Summon(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.kirby3->base.x;
    struct Object *dee;
    u8 i;

    DeesOut(s);
    for (i = 0; i < DEES && DEE(s, i); i++) ;
    if (i >= DEES || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    dee = DEE(s, i) = CreateObjTemplateAndObj(s->obj.base.unk56, 1, 0x24, x >> 8, level->viewportPosition.y >> 8, 0, 0x1F, 0, 0,
                                              OBJ_WADDLE_DEE_1, SUMMON_DEE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (dee == NULL) return;
    dee->base.parent = &s->obj;
    if (x < ArenaLeft(s) + (DISPLAY_WIDTH / 2 << 8)) dee->base.flags &= ~1;
    else dee->base.flags |= 1;
}

// Inside(); A Kirby it caught still its own (held: not let go by KatAM, hurt or beaten);
static bool32 Inside(struct SqueakObject *s, u8 i) {
    return (HELD(s) >> i & 1) && gKirbys[i].unk110 && gKirbys[i].base.unk6C == &s->obj;
}

// LetOut(); 0x020E56EC: the Kirbys it holds let out, hurt (KatAM's grab script);
static void LetOut(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (Inside(s, i)) gKirbys[i].unk110 = sLetOut;
    HELD(s) = 0;
    SWALLOWED(s) = FALSE;
}

/* Catch(); 0x020E7C5C: a Kirby in its inhale's box (dedede.rca's, 66 pixels ahead) caught: into KatAM's grab, held where he is (the
 * pull moves him); */
static bool32 Catch(struct SqueakObject *s) {
    struct Kirby *kirby = SqueakGrabbing(s, &sDededeBoxes[BOX_INHALE]);

    if (kirby == NULL) return FALSE;
    kirby->unk110 = sDrawnIn;
    kirby->base.unk6C = &s->obj;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    if (!kirby->unk110) return FALSE;
    HELD(s) = 1 << (kirby - gKirbys);
    SWALLOWED(s) = FALSE;
    return TRUE;
}

/* Pull(); 0x020E598C: the Kirbys it caught drawn to its mouth (MOUTH_X ahead, MOUTH_Y up), PULL_X and PULL_Y a frame at most; there,
 * swallowed (hidden in it); whether they are; */
static bool32 Pull(struct SqueakObject *s) {
    s32 mx = s->obj.base.x + SQUEAK_SPEED(FacesLeft(s) ? -MOUTH_X : MOUTH_X), my = s->obj.base.y - SQUEAK_SPEED(MOUTH_Y);
    bool32 there = TRUE;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];
        s32 *x = &kirby->base.x, *y = &kirby->base.y;

        if (!Inside(s, i)) continue;
        if (*x < mx) *x = *x + SQUEAK_SPEED(PULL_X) < mx ? *x + SQUEAK_SPEED(PULL_X) : mx;
        else if (*x > mx) *x = *x - SQUEAK_SPEED(PULL_X) > mx ? *x - SQUEAK_SPEED(PULL_X) : mx;
        if (*y < my) *y = *y + SQUEAK_SPEED(PULL_Y) < my ? *y + SQUEAK_SPEED(PULL_Y) : my;
        else if (*y > my) *y = *y - SQUEAK_SPEED(PULL_Y) > my ? *y - SQUEAK_SPEED(PULL_Y) : my;
        if (*x != mx || *y != my) there = FALSE;
    }
    if (!HELD(s) || !there) return FALSE;
    for (i = 0; i < gNumKirbys; i++) {
        if (!Inside(s, i)) continue;
        gKirbys[i].unk110 = sSwallowed;
        gKirbys[i].base.counter = ~s->obj.unk83;
    }
    SWALLOWED(s) = TRUE;
    return TRUE;
}

// Spit(); 0x0212389C(0, ...): the Kirbys it swallowed spat out in a star (SPIT_X ahead, SPIT_Y up), its way;
static void Spit(struct SqueakObject *s) {
    struct SqueakObject *star;
    u8 i, kirbys = 0;

    for (i = 0; i < gNumKirbys; i++)
        if (Inside(s, i)) kirbys |= 1 << i;
    HELD(s) = 0;
    SWALLOWED(s) = FALSE;
    if (!kirbys) return;
    star = SqueakSpawn(s, SQUEAK_STAR, SQUEAK_STAR_SPIT, SPIT_X, SPIT_Y);
    if (star == NULL) {
        HELD(s) = kirbys;
        LetOut(s);
        return;
    }
    SqueakStarCarry(star, kirbys);
}

/* Done(); A move over: standing (0x020E73F0), this frame (`now`: its state returned 1) or from the next (Squeak Squad's states that
 * set it and return 0); */
static bool32 Done(struct SqueakObject *s, bool32 now) {
    SqueakSetState(s, BODY_IDLE_ENTRY);
    return now;
}

static bool32 Body(struct SqueakObject *s) {
    s32 dx;
    u32 r;

    switch (s->state) {
    case BODY_INTRO:        // 0x020E7828: INTRO frames standing;
        if (COUNT(s)-- < 1) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_START:        // 0x020E7758;
        SqueakPlay(s, ANIM_IDLE);
        SqueakStop(s);
        WAIT(s) = 0;
        SqueakSetState(s, BODY_PICK);
        return TRUE;
    case BODY_PICK:         // 0x020E749C;
        if (!SqueakOnGround(s)) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            SqueakMove(s);
            break;
        }
        if (s->obj.base.x < s->obj.kirby3->base.x ? FacesLeft(s) : !FacesLeft(s)) {
            SqueakTurnAround(s);
        } else if (WAIT(s) < PICK_WAIT) {
            WAIT(s)++;
        } else {
            dx = s->obj.kirby3->base.x - s->obj.base.x;
            if (dx < 0) dx = -dx;
            // (Squeak Squad's returns 1 with nothing picked: it runs again, a new roll, till one is);
            for (;;) {
                r = Random();
                if (dx < SQUEAK_SPEED(NEAR)) {
                    if (r < NEAR_HAMMER) SqueakSetState(s, BODY_HAMMER_ENTRY);
                    else if (r < NEAR_HAMMER + NEAR_HJUMP) SqueakSetState(s, BODY_HJUMP_ENTRY);
                    else if (r < NEAR_INHALE) SqueakSetState(s, BODY_INHALE_ENTRY);
                    else continue;
                } else if (r < FAR_WALK) {
                    SqueakSetState(s, BODY_WALK_ENTRY);
                } else if (r < FAR_WALK + FAR_JUMP) {
                    SqueakSetState(s, BODY_JUMP_ENTRY);
                } else if (r < FAR_WALK + FAR_JUMP + FAR_HAMMER) {
                    SqueakSetState(s, BODY_HAMMER_ENTRY);
                } else if (r < FAR_WALK + FAR_JUMP + FAR_HAMMER + FAR_HJUMP) {
                    SqueakSetState(s, BODY_HJUMP_ENTRY);
                } else if (r < FAR_WALK + FAR_JUMP + FAR_HAMMER + FAR_HJUMP + FAR_FLOAT) {
                    SqueakSetState(s, BODY_FLOAT_ENTRY);
                } else {
                    SqueakSetState(s, DeesOut(s) < DEES ? BODY_SUMMON_ENTRY : BODY_WALK_ENTRY);
                }
                return TRUE;
            }
        }
        SqueakMove(s);
        break;
    case BODY_IDLE_ENTRY:   // 0x020E73F0;
        SqueakPlay(s, ANIM_IDLE);
        SqueakStop(s);
        COUNT(s) = IDLE_TIME;
        SqueakSetState(s, BODY_IDLE);
        return TRUE;
    case BODY_IDLE:         // 0x020E736C;
        SqueakMove(s);
        if (COUNT(s) < 1) {
            SqueakSetState(s, BODY_START);
        } else {
            if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
            COUNT(s)--;
        }
        break;
    case BODY_WALK_ENTRY:   // 0x020E72D4;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x020E7194: to within 50 pixels of Kirby or the arena's side;
        s->vx = WALK_SPEED;
        SqueakMove(s);
        dx = s->obj.kirby3->base.x;
        if (AtSide(s) || (FacesLeft(s) ? s->obj.base.x < dx + SQUEAK_SPEED(WALK_STOP) : s->obj.base.x > dx - SQUEAK_SPEED(WALK_STOP)))
            return Done(s, FALSE);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_JUMP_ENTRY:   // 0x020E70E8;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakStop(s);
        COUNT(s) = JUMP_CROUCH;
        SqueakSetState(s, BODY_JUMP_CROUCH);
        return TRUE;
    case BODY_JUMP_CROUCH:  // 0x020E6FB4;
        if (COUNT(s) < 1) {
            s->vx = Random() < 50 ? JUMP_VX : 0;
            s->vy = JUMP_VY;
            s->ay = -GRAVITY;
            SqueakPlay(s, ANIM_JUMP);
            SqueakSetState(s, BODY_JUMP_RISE);
        } else {
            COUNT(s)--;
        }
        SqueakMove(s);
        break;
    case BODY_JUMP_RISE:    // 0x020E6F08;
        SqueakMove(s);
        if (s->vy < 0) {
            SqueakPlay(s, ANIM_FALL);
            SqueakSetState(s, BODY_JUMP_FALL);
        }
        break;
    case BODY_JUMP_FALL:    // 0x020E6E18: landed, a big puff, the screen shaking;
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            s->vx = 0;
            s->vy = s->ay = 0;
            SqueakPlay(s, ANIM_LANDED);
            COUNT(s) = JUMP_WAIT;
            Puff(s, TRUE);
            RequestScreenShake(SHAKE, &s->obj.base);
            SqueakSetState(s, BODY_LANDED);
        }
        break;
    case BODY_LANDED:       // 0x020E6DBC, 0x020E650C (0x020E6BD0, after a fall, from the next frame: alike enough);
        if (COUNT(s) < 1) return Done(s, TRUE);
        COUNT(s)--;
        SqueakMove(s);
        break;
    case BODY_FALL_ENTRY:   // 0x020E6D10;
        SqueakPlay(s, ANIM_FALLING);
        SqueakStop(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020E6C28: to MAX_FALL; landed, a big puff;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_LANDED);
            Puff(s, TRUE);
            COUNT(s) = LAND_TIME;
            SqueakSetState(s, BODY_LANDED);
        }
        break;
    case BODY_HAMMER_ENTRY: // 0x020E6AF8: turned to Kirby, its hammer up;
        SqueakPlay(s, ANIM_HAMMER_WALK);
        SqueakStop(s);
        FaceKirby(s);
        SqueakSetState(s, BODY_HAMMER_WALK);
        return TRUE;
    case BODY_HAMMER_WALK:  // 0x020E6968: within HAMMER_REACH of Kirby or the arena's side, its swing;
        s->vx = WALK_SPEED;
        SqueakMove(s);
        dx = s->obj.kirby3->base.x - s->obj.base.x;
        if (dx < 0) dx = -dx;
        if (dx < SQUEAK_SPEED(HAMMER_REACH) || AtSide(s)) {
            COUNT(s) = 0;
            s->vx = 0;
            SqueakPlay(s, ANIM_SWING);
            SqueakSetState(s, BODY_HAMMER_SWING);
            return TRUE;
        }
        break;
    case BODY_HAMMER_SWING: // 0x020E68B8: its HAMMER_HIT-th frame, the impact;
        if (COUNT(s) == HAMMER_HIT) Impact(s);
        COUNT(s)++;
        SqueakMove(s);
        if (s->animEnded) return Done(s, FALSE);
        break;
    case BODY_HJUMP_ENTRY:  // 0x020E680C;
        SqueakPlay(s, ANIM_HAMMER_CROUCH);
        SqueakStop(s);
        COUNT(s) = HJUMP_CROUCH;
        SqueakSetState(s, BODY_HJUMP_CROUCH);
        return TRUE;
    case BODY_HJUMP_CROUCH: // 0x020E6720;
        if (COUNT(s) < 1) {
            s->vy = HJUMP_VY;
            s->ay = -HJUMP_GRAVITY;
            SqueakPlay(s, ANIM_HAMMER_UP);
            SqueakSetState(s, BODY_HJUMP_RISE);
        } else {
            COUNT(s)--;
        }
        SqueakMove(s);
        break;
    case BODY_HJUMP_RISE:   // 0x020E6674: from the top, swung down;
        SqueakMove(s);
        if (s->vy < 0) {
            SqueakPlay(s, ANIM_SWING_DOWN);
            SqueakSetState(s, BODY_HJUMP_SWING);
        }
        break;
    case BODY_HJUMP_SWING:  // 0x020E6624;
        SqueakMove(s);
        if (s->animEnded) SqueakSetState(s, BODY_HJUMP_FALL);
        break;
    case BODY_HJUMP_FALL:   // 0x020E6568: landed, the impact;
        Gravity(s);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            COUNT(s) = HJUMP_WAIT;
            Impact(s);
            SqueakSetState(s, BODY_LANDED);
        }
        break;
    case BODY_FLOAT_ENTRY:  // 0x020E6474: puffing up;
        SqueakPlay(s, ANIM_INFLATE);
        SqueakStop(s);
        SqueakSetState(s, BODY_FLOAT_INFLATE);
        return TRUE;
    case BODY_FLOAT_INFLATE: // 0x020E6394: its jump;
        SqueakMove(s);
        if (s->animEnded) {
            s->vy = FLOAT_VY;
            s->ay = -FLOAT_GRAVITY;
            SqueakPlay(s, ANIM_PUFFED);
            SqueakSetState(s, BODY_FLOAT_RISE);
        }
        break;
    case BODY_FLOAT_RISE:   // 0x020E62C8: from the top, floating;
        SqueakMove(s);
        if (s->vy < 0) {
            COUNT(s) = FLOAT_TIME;
            SqueakPlay(s, ANIM_FLOAT);
            SqueakSetState(s, BODY_FLOAT);
        }
        break;
    case BODY_FLOAT:        // 0x020E60EC: after Kirby; FLOAT_TIME frames, then its air out;
        dx = s->obj.kirby3->base.x;
        if (s->obj.base.x > dx + SQUEAK_SPEED(FACE_ZONE) ? !FacesLeft(s) : s->obj.base.x < dx - SQUEAK_SPEED(FACE_ZONE) && FacesLeft(s))
            SqueakTurnAround(s);
        s->ax = FLOAT_AX;
        if (s->vx > FLOAT_VX_CAP) s->vx = FLOAT_VX_CAP;
        if (s->obj.kirby3->base.y > s->obj.base.y) {
            s->ay = -FLOAT_AY;
            if (s->vy < -FLOAT_VY_CAP) s->vy = -FLOAT_VY_CAP;
        } else {
            s->ay = FLOAT_AY;
            if (s->vy > FLOAT_VY_CAP) s->vy = FLOAT_VY_CAP;
        }
        if (SqueakOnGround(s)) {
            s->vy = FLOAT_BOUNCE;
            Puff(s, FALSE);
        }
        SqueakMove(s);
        if (COUNT(s) < 0) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_EXHALE);
            SqueakSpawn(s, SQUEAK_DEDEDE_PUFF, 0, PUFF_X, PUFF_Y);
            SqueakSetState(s, BODY_FLOAT_EXHALE);
        } else {
            COUNT(s)--;
        }
        break;
    case BODY_FLOAT_EXHALE: // 0x020E6074;
        SqueakMove(s);
        if (s->animEnded) {
            SqueakStop(s);
            s->ay = -FLOAT_GRAVITY;
            SqueakSetState(s, BODY_FLOAT_FALL);
        }
        break;
    case BODY_FLOAT_FALL:   // 0x020E5FA8: landed, a puff;
        Gravity(s);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_FLOAT_LANDED);
            Puff(s, FALSE);
            SqueakSetState(s, BODY_FLOAT_LANDED);
        }
        break;
    case BODY_FLOAT_LANDED: // 0x020E5F54;
        SqueakMove(s);
        if (s->animEnded) return Done(s, TRUE);
        break;
    case BODY_INHALE_ENTRY: // 0x020E5D30: its mouth open;
        SqueakPlay(s, ANIM_INHALE);
        SqueakStop(s);
        SqueakSetState(s, BODY_INHALE_OPEN);
        return TRUE;
    case BODY_INHALE_OPEN:  // 0x020E5C78: inhaling;
        SqueakMove(s);
        if (s->animEnded) {
            SqueakPlayComposite(s, COMPOSITE_INHALE);
            SqueakSetState(s, BODY_INHALE);
        }
        break;
    case BODY_INHALE:       // 0x020E5C1C (its catch: 0x020E7C5C);
        if (Catch(s)) {
            SqueakStop(s);
            SqueakSetState(s, BODY_PULL);
            return TRUE;
        }
        SqueakMove(s);
        if (s->animEnded) return Done(s, FALSE);
        break;
    case BODY_PULL:         // 0x020E598C: drawn in, chewed;
        SqueakMove(s);
        if (!HELD(s)) return Done(s, FALSE);
        if (Pull(s)) {
            SqueakPlay(s, ANIM_CHEW);
            SqueakSetState(s, BODY_CHEW);
        }
        break;
    case BODY_CHEW:         // 0x020E580C: turned to the arena's middle, spat out;
        SqueakMove(s);
        if (!s->animEnded) break;
        if (!HELD(s)) return Done(s, FALSE);
        if (s->obj.base.x > ArenaLeft(s) + (DISPLAY_WIDTH / 2 << 8) ? !FacesLeft(s) : FacesLeft(s)) SqueakTurnAround(s);
        Spit(s);
        SqueakPlay(s, ANIM_SPIT);
        SqueakSetState(s, BODY_SPIT);
        break;
    case BODY_SPIT:         // 0x020E57A0;
        SqueakMove(s);
        if (s->animEnded) return Done(s, FALSE);
        break;
    case BODY_SUMMON_ENTRY: // 0x020E5EB0: its call;
        SqueakPlayComposite(s, COMPOSITE_SUMMON);
        SqueakStop(s);
        SqueakSetState(s, BODY_SUMMON);
        return TRUE;
    case BODY_SUMMON:       // 0x020E5DC8: a Waddle Dee;
        SqueakMove(s);
        if (s->animEnded) {
            Summon(s);
            return Done(s, FALSE);
        }
        break;
    case BODY_HURT_ENTRY:   // 0x020E5654;
        SqueakPlay(s, ANIM_HURT);
        SqueakStop(s);
        SqueakSetState(s, BODY_HURT);
        return TRUE;
    case BODY_HURT:         // 0x020E5604;
        SqueakMove(s);
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* Hit(); 0x020E79B0 (KatAM's mini-bosses take hits without a hurt state of their own: a fall of its HP is the hit): SAFE_TIME frames
 * safe after one (KatAM's damage undone, as while it's untouchable: SqueakUntouchable), its hurt pose, a Kirby it holds let out; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (SAFE(s) > 0) SAFE(s)--;
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (SAFE(s) > 0 || s->level) {
            obj->unk80 = LAST_HP(s);
        } else {
            SAFE(s) = SAFE_TIME;
            if (HELD(s)) LetOut(s);
            SqueakSetState(s, BODY_HURT_ENTRY);
        }
    }
    LAST_HP(s) = obj->unk80;
}

static void SqueakDededeMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakMidbossUpdate(s);
    Hit(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sDededeBoxData, HARM(s), &BOX(s));
    // (from its catch on, its +0x18 bit 2 takes it out of every attack's reach: 0x020E7C5C, 0x0201322C, untouchable);
    if (HELD(s)) SqueakUntouchable(s, TRUE);
}

// SqueakDededeGone(); Its end (squeak.c's SqueakDestroy: beaten, or its room left): a Kirby it holds let out;
void SqueakDededeGone(struct SqueakObject *s) {
    LetOut(s);
}

/* SqueakDededeStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do): the first time, INTRO frames standing,
 * then its picks; */
void SqueakDededeStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    u8 i;

    ObjectSetFunc(obj, -1, SqueakDededeMain);
    if (s->started) return;
    SqueakPlay(s, ANIM_IDLE);
    SqueakStop(s);
    COUNT(s) = INTRO;
    HELD(s) = 0;
    SAFE(s) = 0;
    LAST_HP(s) = obj->unk80;
    for (i = 0; i < DEES; i++) DEE(s, i) = NULL;
    SqueakSetState(s, BODY_INTRO);
}

// SqueakDededeInit(); Its box (dedede.rca's standing one), its bounds (KatAM's), facing left, standing;
void SqueakDededeInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -10, -37, 14, -1);
    ObjectSetBounds(&s->obj.base, -16, -40, 16, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
    SqueakPlay(s, ANIM_IDLE);
}

/* The puff (0x0212EAA0, 0x0212EA1C, 0x0212E8CC, 0x0212E888: Dedede's animation 20), as SQUEAK_DEDEDE_PUFF: forward PUFF_SPEED,
 * slowing PUFF_SLOW a frame, through everything (its flag 0x200), PUFF_TIME frames or till it's out of the camera's view; harmless
 * (dedede.rca has no box for it), untouchable; */
static void PuffMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 x = obj->base.x - level->viewportPosition.x;

    if (P_COUNT(s) < 1 || x < -(56 << 8) || x > (DISPLAY_WIDTH + 56) << 8) {
        obj->base.flags |= 0x1000;
        return;
    }
    P_COUNT(s)--;
    s->vx = s->vx > PUFF_SLOW ? s->vx - PUFF_SLOW : 0;
    SqueakMove(s);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void DededePuffStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, PuffMain);
    SqueakThroughTerrain(s, TRUE);
    obj->base.flags |= 0x200;
    obj->base.unk68 = 0;
    if (s->started) return;
    s->vx = PUFF_SPEED;
    P_COUNT(s) = PUFF_TIME;
    SqueakPlay(s, ANIM_PUFF);
}

void DededePuffInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -8, 8, 8);
    ObjectSetBounds(&s->obj.base, -8, -8, 8, 8);
    s->obj.base.unk5C |= 0x20 | 0x80;
    s->obj.base.flags |= 0x200 | 0x1000000;
}
