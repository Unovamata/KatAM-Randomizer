/* Yadgaine (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x3B (its body 0x0211A4E8, vtable 0x021824F0: +0x34 per frame
 * 0x0211A8C0, +0x30 its hit 0x0211A13C, +0x2C its touch 0x0211A3DC, +0x18 0x0211A098; its states 0x02117048-0x0211A038), as
 * SQUEAK_YADGAINE, a boss on the mid-bosses' framework (mini-boss type 0x40: the camera held on its area, the HP bar, the music,
 * KatAM's defeat), with its two legs (SQUEAK_YADGAINE_LEG: 0x021185FC, the back one, and 0x02117C44, the front one), what it throws
 * and knocks down (SQUEAK_YADGAINE_THING: 0x0212E248's kinds 0-6), the gordos falling from the top (SQUEAK_YADGAINE_GORDO: kind 7) and
 * Daroach bringing it in (SQUEAK_YADGAINE_DAROACH: kind 8). Values from param/PrmBB05Ml0, Sl0 (HP 400, its legs 290 and 220, 360 of
 * damage before its second form) and Pl0 (its picks); here KatAM's bosses' HP as Mrs. Moley's 300 is 60: 80, its legs 58 and 44, 72.
 * Speeds 20.12 a frame, Y up; drawn with enemy/yadgaine (a robot in a spiked shell: its .mca's composites, its body 0 and 1, its legs
 * 2-15), its boxes from yadgaine.rca (two animations behind the .cea's). Recorded on the DS in its own stage (map/a4s7s2).
 * • Its stage scrolls: Squeak Squad moves the camera's lock right every frame (0x02035C04, by +0x210: its scroll), an endless stage
 *   (a4s7s2's width set to 512000 pixels), Yadgaine chasing from the left at the scroll's speed and more or less. KatAM's rooms
 *   don't go on: here the screen stays and the scroll is taken out of everything of its fight instead (as the DS's screen leaves it
 *   behind): it and its things move at their speed less the scroll's, and the Kirbys in its room are carried back at the scroll's
 *   speed. Its spots are from the camera's view at its fight's start (its left, as Squeak Squad's: 64 and 110 pixels in), the
 *   rocks' from the view's right (Squeak Squad's 256 across, KatAM's 240).
 * • Its front is a wall (0x0222BFDC, the stage's terrain: 0x022111B4): nothing goes behind it; a Kirby there is pushed to it.
 * • Its fight: Daroach comes down over it (80 pixels up: anims 1-5, then off up and back at 2.0), it waits 120 frames, hidden; the
 *   scroll at 1.0 150 frames (the view's left carrying it along); then it shows and comes in at 2.0 from the view's left till 64
 *   pixels in (0x02119E90, 0x02119E08). Then each round
 *   (0x02119D04: the scroll 0.5, its speed the same, 90 frames) while rocks fall (0x0211983C: every 15 frames, of 100, under 20 a
 *   small one, under 28 a big one, under 30 a gordo, across the view's right 120 pixels, 80 when it's forward), it picks (of 100):
 *   with a leg left, back (at 64) under 20 its swipe, 40 its lunge, 60 its shot, else its dash forward; forward (at 110) under 45
 *   its swipe, 90 back, else a short pause; with no leg left, before its second form, back under 70 its shot, else its dash;
 *   forward under 70 back, else a short pause; in its second form, back under 70 its dash, else the long pause; forward under 70
 *   back, else the long pause. Its dash (0x02119600): the scroll 1.0, it 1.3, till 110 in (forward); back (0x02119510): the scroll
 *   1.0, it 0.8, till 64 in. Its swipe (0x021193FC): the scroll and it 0.5, its legs raised 30 frames and struck down, 120 frames;
 *   its lunge (0x02119254): the scroll and it 1.0, legs raised 30 frames, then struck down 120 frames while it goes 1.5 (forward
 *   after). Its shot (0x02119100): the scroll and it 0.5, a puff 5 ahead, 10 up, 20 frames later its capsule (21 ahead, 10 up, 3.5
 *   forward), 80 frames. Its pauses (0x021197A0, 0x02119704): 20 or 50 frames. It's held at most 110 pixels in (0x0211A098).
 * • Its legs follow it (0x021182A4, 0x021178E8): walking (.mca's 2 and 3), raised (8 then 9, 12 then 13), struck down and resting
 *   on the ground (10 then 11, 14 then 15: each time a foot touches the ground there, of 100, under 4, 7 and 12 a block is knocked
 *   up from it, 0x021179CC, 0x02118384: 2.0 or 3.0 forward, 4.0, 5.0 or 3.0 up, 0.1 gravity; landed, a solid block till it's under
 *   it, 32 pixels ahead of it), broken (4, 5). Its hit (0x0211A13C) goes to its legs as well (their own hits theirs only); a leg
 *   whose HP is out stays broken, hurting by touch, untouchable. Its body 20 frames safe after a hit (KatAM's damage undone); 72
 *   damage on it (Sl0's 360) and its second form: its cracked body (.mca's 1). Its shell's box (yadgaine.rca's 35: 197 pixels
 *   behind its middle) is behind its wall.
 * • Its size: three quarters of the DS's (the owner's pick: at the DS's, its body's own pixels beside a shell KatAM's sprite memory
 *   can't hold as big, it was too detailed beside it), drawn pixel for pixel: its frames, its shell and its animations' offsets
 *   (KatAMExpansionSqueak.cs's YadgaineScale) and here its boxes, its spots (the shell's cut left edge off the screen), how far
 *   ahead of it things start, its moves' own speed (its speed less the scroll's) and its knocked-up blocks' flight as much smaller
 *   (SIZED: each move and block as long as the DS's; at the DS's flight beside its smaller spots, blocks piled up and the fight
 *   lagged); what it throws and knocks down (Kirby's size), the scroll and the rocks' range across the view as they were.
 * • Its shell (the stage draws it on a background layer from its own tiles, 0x0220B0B4: 154 by 208 pixels, 312 tiles; KatAM's
 *   backgrounds' tiles and palettes are full in a fight): a sprite at its size (174 tiles), only what its fight shows of it
 *   (see Shell() below). Its fight is near the GBA's limits without it (KatAM's
 *   sprite heap at 428 of 512 tiles when two rock bursts are out, its 7 enemy palettes all taken, its frames near the end of the
 *   CPU's time): Squeak Squad's effects wait for room in the heap (squeak.c's SQUEAK_FX_ROOM: a rock's burst left out when it
 *   would leave too little for KatAM's own), the shell takes Daroach's palette slot once he's gone, and is drawn by hand.
 * Not ported: the floor crumbling behind it (the stage's tiles), the screen not scrolling, the DS's camera jolts as it walks, steps and
 * throws (0x02035F60, 0x02035FC4), its hit flash (yadgaine_dmg's palettes), Squeak Squad's damage less from some attacks (80 in
 * 100, 60 with its legs broken), its own defeat (0x02118E94: KatAM's mini-boss defeat), its walk-off when Kirby is beaten (+0x3C,
 * 0x02118A80), its boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "main.h"
#include "data.h"
#include "sprite.h"
#include "code_0806F780.h"
#include "constants/object_types.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Its .mca's composite animations (Squeak Squad's 0x3000000 | n): its body; its legs (back, front);
enum { COMPOSITE_BODY, COMPOSITE_CRACKED };
static const u8 sLegComposites[2][7] = {    // walk, walk (on), raise, raised, strike, rest, broken;
    { 2, 6, 8, 9, 10, 11, 4 },
    { 3, 7, 12, 13, 14, 15, 5 },
};
enum { LEG_WALK, LEG_WALK_ON, LEG_RAISE, LEG_RAISED, LEG_STRIKE, LEG_REST, LEG_BROKEN };

// PrmBB05Ml0.kgb (20.12, frames, pixels);
#define SAFE_TIME 20            // +0x0C;
#define RUN_TIME 150            // +0x14, +0x18: its intro's scroll;
#define RUN_SCROLL 0x1000
#define RUSH_EXTRA 0x1000       // +0x1C;
#define ROUND_SCROLL 0x800      // +0x24;
#define PAUSE_SHORT 20          // +0x28, +0x2C;
#define PAUSE_LONG 50
#define ROUND_TIME 90           // +0x30;
#define ROCK_RANGE_FRONT 80     // +0x34, +0x38: across the view's right, forward or back;
#define ROCK_RANGE_BACK 120
#define ROCK_EVERY 15           // +0x3C;
#define DASH_SCROLL 0x1000      // +0x44, +0x48, +0x4C;
#define DASH_EXTRA 0x4CD
#define FRONT_X 110
#define BACK_SCROLL 0x1000      // +0x50, +0x54, +0x58;
#define BACK_EXTRA (-0x334)
#define BACK_X 64
#define SWIPE_SCROLL 0x800      // +0x60, +0x64, +0x68;
#define SWIPE_RAISE 30
#define SWIPE_TIME 120
#define LUNGE_SCROLL 0x1000     // +0x98, +0x9C, +0xA0, +0xA4;
#define LUNGE_EXTRA 0x800
#define LUNGE_RAISE 30
#define LUNGE_TIME 120
#define SHOT_SCROLL 0x800       // +0xAC, +0xB0, +0xB4, +0xB8, +0xBC, +0xC4;
#define SHOT_X 5
#define SHOT_Y 10
#define SHOT_X2 16
#define SHOT_WAIT 20
#define SHOT_AFTER 80
#define INTRO_TIME 120          // 0x0211A038;
#define CLAMP_X 110             // 0x0211A098 (as logged: no further than 110 pixels in);
#define DAROACH_UP 80           // 0x0211A038: Daroach 80 pixels over it;
#define PHASE_DAMAGE_NUM 9      // PrmBB05Sl0: 360 of its 400 HP before its second form;
#define PHASE_DAMAGE_DEN 10
#define LEG_B_NUM 220           // PrmBB05Sl0: its front leg's 220 HP, its back one's 290;
#define LEG_A_DEN 290
#define LEG_HP_OFFSET 100       // A leg's HP kept over this (KatAM's defeat at 0: a leg stays, broken);
#define SCREEN_W 240
/* Its size (see above): SIZE_NUM / SIZE_DEN of the DS's (KatAMExpansionSqueak.cs's YadgaineScaleNum, YadgaineScaleDen); SIZED() a
 * length or speed of the DS's at it, rounded (halves away from 0: the same either way it faces); */
#define SIZE_NUM 3
#define SIZE_DEN 4
#define SIZED(v) (((v) * SIZE_NUM * 2 + ((v) >= 0 ? SIZE_DEN : -SIZE_DEN)) / (SIZE_DEN * 2))
#define START_X 48              // 0x0211A038: hidden where its stage has it, 48 pixels in;
// PrmBB05Pl0.kgb: its picks (of 100);
static const u8 sPickBack[] = { 20, 40, 60, 100 }, sPickFront[] = { 45, 90, 100 };
#define PICK_BARE 70
#define ROCK_SMALL 20           // +0x58, +0x5C, +0x60;
#define ROCK_BIG 28
#define ROCK_GORDO 30
#define DEBRIS_2 4              // +0x68, +0x6C, +0x70 (the legs');
#define DEBRIS_3 7
#define DEBRIS_4 12

enum {
    YG_START, YG_INTRO, YG_RUN, YG_RUSH, YG_ARRIVE, YG_ROUND, YG_IDLE, YG_DASH, YG_DASH_GO, YG_BACK, YG_BACK_GO, YG_SWIPE,
    YG_SWIPE_RAISE, YG_STRIKE, YG_LUNGE, YG_LUNGE_RAISE, YG_SHOT, YG_SHOT_WAIT, YG_SHOT_AFTER, YG_PAUSE_SHORT, YG_PAUSE_LONG,
    YG_PAUSE, YG_CRACK,
};

#define TIMER(s) ((s)->vars[0])         // +0x208;
#define PHASE_LEFT(s) ((s)->vars[1])    // +0x20C: damage before its second form;
#define SCROLL(s) ((s)->vars[2])        // +0x210: the scroll's speed;
#define FORWARD(s) ((s)->vars[3])       // +0x218: forward (at 110) or back (at 64);
#define ROCK_TIMER(s) ((s)->vars[4])    // +0x21C;
#define SAFE(s) ((s)->vars[5])          // +0x204;
#define LAST_HP(s) ((s)->vars[6])
#define HARM(s) ((s)->vars[7])
#define FLOOR(s) ((s)->vars[8])
#define LEFT(s) ((s)->vars[9])          // The camera view's left at its fight's start;
#define STEP(s) ((s)->vars[10])         // How far the scroll went this frame (24.8), taken out of everything of its fight;
#define SHOWN(s) ((s)->vars[11])
#define LEG(s, i) (*(struct SqueakObject **)&(s)->vars[12 + (i)])
#define CRACKED(s) ((s)->vars[14])
#define SCROLLS(s) ((s)->vars[15])      // This state scrolls (0x02035C04);
#define SHELL(s) (*(struct YadgaineShell **)&(s)->vars[16])
#define BOB(s) ((s)->vars[17])          // Its body's step's offset down this frame (the DS's shell follows it);
#define DAROACH(s) (*(struct SqueakObject **)&(s)->vars[18])

static void Shell(struct SqueakObject *s);

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

static bool32 Alive(struct SqueakObject *o, u8 enemy) {
    return o && o->obj.base.header.kind == 1 && !(o->obj.base.flags & 0x1000) && SqueakIsSqueak(&o->obj) && o->enemy == enemy;
}

// YadgaineOf(); The boss a leg or thing of its fight belongs to (NULL once it's gone);
static struct SqueakObject *YadgaineOf(struct SqueakObject *t) {
    struct SqueakObject *p = (struct SqueakObject *)t->obj.base.parent;

    if (p && Alive(p, SQUEAK_YADGAINE_LEG)) p = (struct SqueakObject *)p->obj.base.parent;
    return p && Alive(p, SQUEAK_YADGAINE) ? p : NULL;
}

// Back(); How far the scroll took the screen this frame (24.8; 0 once its fight is over);
static s32 Back(struct SqueakObject *t) {
    struct SqueakObject *y = YadgaineOf(t);

    return y && y->obj.unk80 > 0 ? STEP(y) : 0;
}

static bool32 LegUp(struct SqueakObject *s, u8 i) {
    return Alive(LEG(s, i), SQUEAK_YADGAINE_LEG) && LEG(s, i)->vars[0] > 0;
}

// SetLegs(); 0x0211851C, 0x02117B64: its legs' mode (a broken one stays broken);
static void SetLegs(struct SqueakObject *s, u8 mode) {
    u8 i;

    for (i = 0; i < 2; i++) {
        struct SqueakObject *l = LEG(s, i);

        if (!LegUp(s, i)) continue;
        l->vars[1] = mode;
        SqueakSetState(l, 0);
    }
}

/* Spawn(); One of its things at a spot ahead of and over it (pixels), the parent `by` (the boss or a leg): Squeak Squad's 0x0212E4A0
 * (its offsets turned with it) and 0x0212E3B4 (a spot); */
static struct SqueakObject *Spawn(struct SqueakObject *by, u8 enemy, u8 kind, s32 x, s32 y) {
    s32 dx = (x - by->obj.base.x) << 4;
    struct SqueakObject *t = SqueakSpawn(by, enemy, kind, by->obj.base.flags & 1 ? -dx : dx, (by->obj.base.y - y) << 4);

    if (t) t->obj.base.parent = &by->obj;
    return t;
}

// Rocks(); 0x0211983C: every ROCK_EVERY frames, of 100, a small rock, a big one or a gordo over the view's right;
static void Rocks(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 range, x, y;
    u32 r;

    if (ROCK_TIMER(s)-- > 0) return;
    ROCK_TIMER(s) = ROCK_EVERY;
    range = FORWARD(s) ? ROCK_RANGE_FRONT : ROCK_RANGE_BACK;
    x = level->viewportPosition.x + ((SCREEN_W - range + (s32)Random(range)) << 8);
    y = level->viewportPosition.y;
    r = Random(100);
    if (r < ROCK_SMALL) Spawn(s, SQUEAK_YADGAINE_THING, 5, x, y);
    else if (r < ROCK_BIG) Spawn(s, SQUEAK_YADGAINE_THING, 6, x, y);
    else if (r < ROCK_GORDO) Spawn(s, SQUEAK_YADGAINE_GORDO, 0, x, y);
}

// Round(); 0x02119D04: the scroll and it at ROUND_SCROLL, ROUND_TIME frames, its legs walking;
static void Round(struct SqueakObject *s) {
    SqueakStop(s);
    SCROLL(s) = ROUND_SCROLL;
    s->vx = SCROLL(s);
    TIMER(s) = ROUND_TIME;
    ROCK_TIMER(s) = ROCK_EVERY;
    SetLegs(s, LEG_WALK);
    SqueakSetState(s, YG_IDLE);
}

// Pick(); 0x0211983C: its next move;
static void Pick(struct SqueakObject *s) {
    u32 r = Random(100);
    u8 next;

    if (!LegUp(s, 0) && !LegUp(s, 1)) {
        if (PHASE_LEFT(s) < 1) next = !FORWARD(s) ? (r < PICK_BARE ? YG_DASH : YG_PAUSE_LONG) : (r < PICK_BARE ? YG_BACK : YG_PAUSE_LONG);
        else next = !FORWARD(s) ? (r < PICK_BARE ? YG_SHOT : YG_DASH) : (r < PICK_BARE ? YG_BACK : YG_PAUSE_SHORT);
    } else if (!FORWARD(s)) {
        next = r < sPickBack[0] ? YG_SWIPE : r < sPickBack[1] ? YG_LUNGE : r < sPickBack[2] ? YG_SHOT : YG_DASH;
    } else {
        next = r < sPickFront[0] ? YG_SWIPE : r < sPickFront[1] ? YG_BACK : YG_PAUSE_SHORT;
    }
    SqueakSetState(s, next);
}

static bool32 Yadgaine(struct SqueakObject *s) {
    s32 x = s->obj.base.x - LEFT(s);

    SCROLLS(s) = TRUE;
    switch (s->state) {
    case YG_START:          // 0x0211A038: hidden where its stage has it (48 in), Daroach over it;
        SqueakStop(s);
        s->obj.base.flags = (s->obj.base.flags & ~1) | 0x400;
        LEFT(s) = gCurLevelInfo[s->obj.base.unk56].viewportPosition.x;
        s->obj.base.x = LEFT(s) + (SIZED(START_X) << 8);
        s->obj.base.y = FLOOR(s);
        SCROLL(s) = 0;
        TIMER(s) = INTRO_TIME;
        DAROACH(s) = Spawn(s, SQUEAK_YADGAINE_DAROACH, 0, s->obj.base.x, s->obj.base.y - (SIZED(DAROACH_UP) << 8));
        SqueakPlayComposite(s, COMPOSITE_BODY);
        SqueakSetState(s, YG_INTRO);
        return TRUE;
    case YG_INTRO:          // 0x02119FCC: still (no scroll);
        SCROLLS(s) = FALSE;
        if (--TIMER(s) < 1) {
            TIMER(s) = RUN_TIME;
            SCROLL(s) = RUN_SCROLL;
            SqueakSetState(s, YG_RUN);
            return TRUE;
        }
        break;
    case YG_RUN:            // 0x02119E90: the scroll on; then it shows, its legs walking, coming in;
        if (--TIMER(s) < 1) {
            s->obj.base.flags &= ~0x400;
            SHOWN(s) = TRUE;
            Shell(s);
            SqueakPlayComposite(s, COMPOSITE_BODY);
            SetLegs(s, LEG_WALK_ON);
            SqueakStop(s);
            s->vx = SCROLL(s) + SIZED(RUSH_EXTRA);
            SqueakSetState(s, YG_RUSH);
            return TRUE;
        }
        break;
    case YG_RUSH:           // 0x02119E08;
        if (x >= SIZED(BACK_X) << 8) {
            SqueakSetState(s, YG_ARRIVE);
            return TRUE;
        }
        break;
    case YG_ARRIVE:         // 0x02119DAC;
        SqueakSfx(s, 0x19D);
        Round(s);
        return TRUE;
    case YG_ROUND:
        Round(s);
        return TRUE;
    case YG_IDLE:           // 0x0211983C;
        Rocks(s);
        if (TIMER(s)-- > 0) break;
        Pick(s);
        return TRUE;
    case YG_DASH:           // 0x02119600;
        SqueakStop(s);
        SqueakSfx(s, 0x19D);
        SCROLL(s) = DASH_SCROLL;
        s->vx = SCROLL(s) + SIZED(DASH_EXTRA);
        SetLegs(s, LEG_WALK_ON);
        SqueakSetState(s, YG_DASH_GO);
        return TRUE;
    case YG_DASH_GO:        // 0x02119570;
        if (x >= SIZED(FRONT_X) << 8) {
            FORWARD(s) = TRUE;
            SqueakSetState(s, YG_ROUND);
            return TRUE;
        }
        break;
    case YG_BACK:           // 0x02119510;
        SqueakStop(s);
        SCROLL(s) = BACK_SCROLL;
        s->vx = SCROLL(s) + SIZED(BACK_EXTRA);
        SqueakSetState(s, YG_BACK_GO);
        return TRUE;
    case YG_BACK_GO:        // 0x02119480;
        if (x <= SIZED(BACK_X) << 8) {
            FORWARD(s) = FALSE;
            SqueakSetState(s, YG_ROUND);
            return TRUE;
        }
        break;
    case YG_SWIPE:          // 0x021193FC;
        SqueakStop(s);
        SCROLL(s) = SWIPE_SCROLL;
        s->vx = SCROLL(s);
        TIMER(s) = SWIPE_RAISE;
        SetLegs(s, LEG_RAISE);
        SqueakSetState(s, YG_SWIPE_RAISE);
        return TRUE;
    case YG_SWIPE_RAISE:    // 0x0211935C;
        if (TIMER(s)-- < 1) {
            TIMER(s) = SWIPE_TIME;
            SetLegs(s, LEG_STRIKE);
            SqueakSetState(s, YG_STRIKE);
            return TRUE;
        }
        break;
    case YG_STRIKE:         // 0x021192E8;
        if (TIMER(s)-- < 1) {
            SqueakSetState(s, YG_ROUND);
            return TRUE;
        }
        break;
    case YG_LUNGE:          // 0x02119254;
        SqueakStop(s);
        SCROLL(s) = LUNGE_SCROLL;
        s->vx = SCROLL(s);
        FORWARD(s) = TRUE;
        TIMER(s) = LUNGE_RAISE;
        SetLegs(s, LEG_RAISE);
        SqueakSetState(s, YG_LUNGE_RAISE);
        return TRUE;
    case YG_LUNGE_RAISE:    // 0x0211919C;
        if (TIMER(s)-- < 1) {
            s->vx = SCROLL(s) + SIZED(LUNGE_EXTRA);
            TIMER(s) = LUNGE_TIME;
            SetLegs(s, LEG_STRIKE);
            SqueakSetState(s, YG_STRIKE);
            return TRUE;
        }
        break;
    case YG_SHOT:           // 0x02119100: a puff ahead of it;
        SqueakStop(s);
        SCROLL(s) = SHOT_SCROLL;
        s->vx = SCROLL(s);
        TIMER(s) = SHOT_WAIT;
        Spawn(s, SQUEAK_YADGAINE_THING, 0, s->obj.base.x + (SIZED(SHOT_X) << 8), s->obj.base.y - (SIZED(SHOT_Y) << 8));
        SqueakSetState(s, YG_SHOT_WAIT);
        return TRUE;
    case YG_SHOT_WAIT:      // 0x02119054: its capsule;
        if (TIMER(s)-- < 1) {
            Spawn(s, SQUEAK_YADGAINE_THING, 1, s->obj.base.x + (SIZED(SHOT_X + SHOT_X2) << 8), s->obj.base.y - (SIZED(SHOT_Y) << 8));
            TIMER(s) = SHOT_AFTER;
            SqueakSetState(s, YG_SHOT_AFTER);
            return TRUE;
        }
        break;
    case YG_SHOT_AFTER:     // 0x02118FE0;
        if (TIMER(s)-- < 1) {
            SqueakSetState(s, YG_ROUND);
            return TRUE;
        }
        break;
    case YG_PAUSE_SHORT:    // 0x021197A0, 0x02119704: still at 0.5, its legs walking, PAUSE_SHORT or PAUSE_LONG frames;
    case YG_PAUSE_LONG:
        SqueakStop(s);
        SCROLL(s) = ROUND_SCROLL;
        s->vx = SCROLL(s);
        TIMER(s) = s->state == YG_PAUSE_SHORT ? PAUSE_SHORT : PAUSE_LONG;
        SetLegs(s, LEG_WALK);
        SqueakSetState(s, YG_PAUSE);
        return TRUE;
    case YG_PAUSE:          // 0x0211968C;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, YG_ROUND);
            return TRUE;
        }
        break;
    case YG_CRACK:          // 0x02118F4C: its second form (its cracked body), a new round;
        CRACKED(s) = TRUE;
        SqueakPlayComposite(s, COMPOSITE_CRACKED);
        SqueakSetState(s, YG_ROUND);
        return TRUE;
    }
    return FALSE;
}

/* Hit(); 0x0211A13C (KatAM's mini-bosses take hits without a hurt state of their own: a fall of its HP is the hit): SAFE_TIME frames
 * safe after one (KatAM's damage undone), the hit given to its legs too, its second form after PHASE_LEFT of damage; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    s32 damage = LAST_HP(s) - obj->unk80;
    u8 i;

    if (SAFE(s) > 0) SAFE(s)--;
    if (damage > 0 && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (SAFE(s) > 0 || !SHOWN(s)) {
            obj->unk80 = LAST_HP(s);
        } else {
            SAFE(s) = SAFE_TIME;
            for (i = 0; i < 2; i++)
                if (LegUp(s, i)) LEG(s, i)->obj.unk80 -= damage;
            if (PHASE_LEFT(s) > 0 && (PHASE_LEFT(s) -= damage) < 1) SqueakSetState(s, YG_CRACK);
        }
    }
    LAST_HP(s) = obj->unk80;
}

/* Carry(); The Kirbys in its room carried back as far as the scroll went (the DS's screen leaving them behind), held off its front
 * (0x022111B4: its wall, its x while it shows); */
static void Carry(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];

        if (k->base.roomId != s->obj.base.roomId || (k->base.unkC & 0x8000)) continue;
        k->base.x -= STEP(s);
        if (SHOWN(s) && k->base.x < s->obj.base.x) k->base.x = s->obj.base.x;
    }
}

// Boxes(); Its body's box (yadgaine.rca's 1 and 10: where it's hit and where its touch hurts in one; SIZED), none while hidden;
static void Boxes(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    if (!SHOWN(s)) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    base->flags &= ~0x200;
    if (CRACKED(s)) sub_0803E2B0(base, SIZED(-29), SIZED(-87), SIZED(26), SIZED(-14));
    else sub_0803E2B0(base, SIZED(-29), SIZED(-97), SIZED(27), SIZED(-1));
    base->unk68 = HARM(s);
}

/* Its shell: the stage's own picture on the DS (a background layer from its own tiles, 0x0220B0B4: KatAM's backgrounds' tiles and
 * palettes are full in a fight), here KatAMExpansionSqueak.cs's SqueakShell, a frame at three quarters of its size, as its body
 * (174 tiles of the 512 of KatAM's sprite heap in a stage, not 288 as the DS's), drawn pixel for pixel. Its tiles and palette
 * are loaded once (KatAM's sprite running its anim's first step, its colors as KatAM loads a slot's); after, it's drawn by hand:
 * its pieces (read once from its frame) written into KatAM's OAM buffer together, no sprite drawing of KatAM's each frame
 * (DisplaySprite, sub_08156D84 a piece: over twice its cost; the fight is near the end of the GBA's frame already). Where the DS has
 * it: the picture's column 128, row 171 at its feet, bobbing with its body (BOB); behind it and its legs (the last of KatAM's sprite
 * layers) and the room's foreground (OAM priority 3, a background layer's: as the DS's); shown with it till it's beaten, from its
 * coming in (Daroach gone by then: his palette slot it takes, as KatAM's 7 enemy palettes are full in its fight, its own 4 of them);
 * never turned (it never does); */
#define SHELL_LAYER 0x1F
#define SHELL_PIECES 48

struct ShellPiece {
    s16 dx, dy;             // Its top left from the frame's spot (pixels);
    u16 attr0, attr1;       // Its shape, size;
    u16 tile;               // Its first tile in the frame's;
    u8 w, h;                // Its size (pixels);
};

struct YadgaineShell {
    struct EffectObject e;
    struct SqueakObject *owner;     // (NULL once it's gone: SqueakDestroy's YadgaineGone);
    u8 pieces;
    struct ShellPiece piece[SHELL_PIECES];
};

// The GBA's OAM sizes in pixels, by shape and size;
static const u8 sShellPieces[3][4][2] = {
    { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
    { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
    { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

// ShellFar(); How far a piece's middle is from its body's (SHELL_BODY_Y over its feet, the DS's 48), across and up;
#define SHELL_BODY_Y SIZED(48)
static s32 ShellFar(const struct ShellPiece *p) {
    s32 x = p->dx + p->w / 2, y = p->dy + p->h / 2 + SHELL_BODY_Y;

    return (x < 0 ? -x : x) + (y < 0 ? -y : y);
}

static void ShellDestroy(struct Task *t) {
    struct YadgaineShell *sh = TaskGetStructPtr(t);

    if (sh->owner && SHELL(sh->owner) == sh) SHELL(sh->owner) = NULL;
    ObjectBaseDestroy(t);
}

static void ShellMain(void) {
    struct YadgaineShell *sh = TaskGetStructPtr(gCurTask);
    struct SqueakObject *s = sh->owner;
    const struct ShellPiece *p;
    u32 tiles, first, n;
    s32 ox, oy;
    u8 i;

    if (sh->e.flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (s == NULL) {
        sh->e.flags |= 0x1000;
        return;
    }
    /* (not drawn once it's beaten, as its legs: KatAM's defeat's stars would take the GBA's sprite time on its lines, the shell,
     * drawn last, cut in bands); */
    if (!SHOWN(s) || (s->obj.base.flags & 0x400) || s->obj.unk80 <= 0 || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    // (its spot on the screen, as KatAM's sprites': the camera's view and the screen's shake);
    ox = (s->obj.base.x >> 8) - (gCurLevelInfo[gLocalPlayerId].viewportPosition.x >> 8) + gUnk_0203AD18[0];
    oy = (s->obj.base.y >> 8) + BOB(s) - (gCurLevelInfo[gLocalPlayerId].viewportPosition.y >> 8) + gUnk_0203AD18[1];
    tiles = ((sh->e.sprite.tilesVram - 0x6010000) >> 5) | sh->e.sprite.palId << 12 | 3 << 10;
    /* (its pieces in KatAM's OAM buffer at once, as sub_08156D84 adds one at a time: the next free entries, each linked to the next,
     * the last to SHELL_LAYER's list's end; 128 at most); */
    first = n = gUnk_030024F0;
    for (i = 0, p = sh->piece; i < sh->pieces && n < 128; i++, p++) {
        s32 x = ox + p->dx, y = oy + p->dy;
        OamData *oam;

        // (one off the screen left out: the GBA wraps its spot, 8 bits down, 9 across);
        if (x <= -p->w || x >= DISPLAY_WIDTH || y <= -p->h || y >= DISPLAY_HEIGHT) continue;
        oam = &gUnk_030031C0[n++];
        oam->all.attr0 = p->attr0 | (y & 0xFF);
        oam->all.attr1 = p->attr1 | (x & 0x1FF);
        oam->all.attr2 = tiles + p->tile;
        oam->split.fractional = n;
    }
    if (n > first) {
        gUnk_030031C0[n - 1].split.fractional = 0xFF;
        if (gUnk_03002450[SHELL_LAYER] == 0xFF) gUnk_03002450[SHELL_LAYER] = first;
        else gUnk_030031C0[gUnk_03006080[SHELL_LAYER]].split.fractional = first;
        gUnk_03006080[SHELL_LAYER] = n - 1;
        gUnk_030024F0 = n;
    }
}

/* Shell(); Its shell (see above), made as it comes in: its VRAM, its palette (Daroach's slot, his key given its own, when he's gone:
 * else a slot of its own), its tiles and colors loaded by its anim's first step, its pieces read from its frame; */
static void Shell(struct SqueakObject *s) {
    const struct SqueakEffect *d = &gSqueakData.effects[SQUEAK_FX_YADGAINE_SHELL];
    const struct SpriteAttributes_Sub *frame;
    const u16 *oams;
    struct YadgaineShell *sh;
    struct Task *t;
    u16 daroach = gSqueakData.enemies[SQUEAK_YADGAINE_DAROACH].animId;
    u8 i, palId = 0xFF;

    if (SHELL(s) || d->tiles == 0 || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    t = TaskCreate(ShellMain, sizeof(struct YadgaineShell), 0x3500, TASK_USE_EWRAM, ShellDestroy);
    if (t == NULL) return;
    sh = TaskGetStructPtr(t);
    ClearEffectObject(&sh->e);
    sh->e.header.kind = 3;
    sh->e.roomId = s->obj.base.roomId;
    sh->e.parent = NULL;
    sh->e.x = s->obj.base.x;
    sh->e.y = s->obj.base.y;
    sh->owner = s;
    sh->pieces = 0;
    SHELL(s) = sh;
    EffectObjectInitSprite(&sh->e, &sh->e.sprite, d->tiles, d->animId, d->anim.step[0].cell, SHELL_LAYER);
    if (!SqueakVramOk(sh->e.sprite.tilesVram)) {
        sh->e.flags |= 0x1000;
        return;
    }
    // (Daroach's slot its key, its colors loaded into it as sub_0803DFAC loads a slot's);
    if (!Alive(DAROACH(s), SQUEAK_YADGAINE_DAROACH) && sub_0803DF24(d->palette.key) == 0xFF)
        for (i = 0; i < gUnk_02022EA0; i++)
            if (gUnk_02022E60[i] == daroach) {
                struct Sprite colors;

                gUnk_02022E60[i] = d->palette.key;
                palId = i + 8;
                SpriteSomething(&colors, 0x6000000, d->palette.key & 0xFFF, d->palette.variant, 0xFF, 0, 0, 0, 0, 0x10, palId, 0x80000);
                break;
            }
    sh->e.sprite.palId = palId != 0xFF ? palId : SqueakLoadPalette(d->palette.key, d->palette.variant, s->obj.base.roomId);
    // (its tiles: queued by its anim's first step, which EffectObjectInitSprite ran, its frame's shown);
    if (sh->e.sprite.unk4 >> 28) return;
    frame = &gSpriteTables->attrs[d->animId].sub[sh->e.sprite.unk4];
    oams = gSpriteTables->oamData[d->animId] + (frame->bitfield & 0x3FFF) * 3;
    for (i = 0; i < frame->numSubframes && i < SHELL_PIECES; i++, oams += 3) {
        struct ShellPiece *p = &sh->piece[i];
        u8 shape = oams[0] >> 14, size = oams[1] >> 14;

        if (shape > 2) continue;
        p->w = sShellPieces[shape][size][0];
        p->h = sShellPieces[shape][size][1];
        p->dx = (oams[1] & 0x1FF) - frame->offsetX;
        p->dy = (oams[0] & 0xFF) - frame->offsetY;
        p->attr0 = oams[0] & 0xC000;
        p->attr1 = oams[1] & 0xC000;
        p->tile = oams[2] & 0x3FF;
        sh->pieces = i + 1;
    }
    /* (the pieces farthest from its body first, those behind it last: on a line with more sprites than the GBA has time for, the
     * last ones in KatAM's OAM, the shell's last pieces, aren't drawn; behind its body that doesn't show); */
    for (i = 1; i < sh->pieces; i++) {
        struct ShellPiece key = sh->piece[i];
        u8 j = i;

        while (j > 0 && ShellFar(&sh->piece[j - 1]) < ShellFar(&key)) {
            sh->piece[j] = sh->piece[j - 1];
            j--;
        }
        sh->piece[j] = key;
    }
}

// YadgaineGone(); It's gone (squeak.c's SqueakDestroy): its shell goes too;
void YadgaineGone(struct SqueakObject *s) {
    if (SHELL(s)) SHELL(s)->owner = NULL;
    SHELL(s) = NULL;
}

// Legs(); Its legs, made at its fight's start, behind it (its back one) and in front (its front one);
static void Legs(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < 2; i++) {
        struct SqueakObject *l;

        if (LEG(s, i)) continue;
        l = SqueakSpawn(s, SQUEAK_YADGAINE_LEG, i, 0, 0);
        LEG(s, i) = l;
        if (l) l->obj.base.parent = &s->obj;
    }
}

static void YadgaineMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    s32 before;

    obj->kirby3 = FindTargetKirby(&obj->base);
    Hit(s);
    Legs(s);
    before = obj->base.x;
    SqueakRun(s, Yadgaine);
    if (obj->unk80 <= 0) return;
    /* (its moves: at its speed less the scroll's; held no further than CLAMP_X in, 0x0211A098, and not out of the view's left: the
     * camera's lock carries it along while it's hidden, as logged: its rush in starts from the view's left); */
    STEP(s) = SCROLLS(s) ? SQUEAK_SPEED(SCROLL(s)) : 0;
    obj->base.x += SQUEAK_SPEED(s->vx) - STEP(s);
    if (obj->base.x > LEFT(s) + (SIZED(CLAMP_X) << 8)) obj->base.x = LEFT(s) + (SIZED(CLAMP_X) << 8);
    if (obj->base.x < LEFT(s)) obj->base.x = LEFT(s);
    obj->base.unk48 = before;
    obj->base.xspeed = obj->base.x - before;
    s->moved = TRUE;
    Carry(s);
    sub_08034E14(obj);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    BOB(s) = obj->base.objBase55;
    Boxes(s);
}

/* YadgaineStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do). Through the terrain (KatAM's flag
 * 0x100: Squeak Squad's walks the stage's floor), not held in the camera's area (it starts out of view); */
void YadgaineStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, YadgaineMain);
    if (s->started) return;
    obj->base.flags = (obj->base.flags & ~1) | 0x100;
    LAST_HP(s) = obj->unk80;
    SAFE(s) = 0;
    SHOWN(s) = FALSE;
    CRACKED(s) = FALSE;
    FORWARD(s) = FALSE;
    PHASE_LEFT(s) = obj->unk80 * PHASE_DAMAGE_NUM / PHASE_DAMAGE_DEN;
    SqueakSetState(s, YG_START);
}

// YadgaineInit(); Its box, bounds, the floor where it's placed; standing still till its fight;
void YadgaineInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    sub_0803E2B0(&obj->base, SIZED(-29), SIZED(-97), SIZED(27), SIZED(-1));
    ObjectSetBounds(&obj->base, SIZED(-32), SIZED(-96), SIZED(32), 0);
    HARM(s) = obj->base.unk68;
    SqueakStandOnFloor(obj);
    FLOOR(s) = obj->base.y;
    LEG(s, 0) = LEG(s, 1) = NULL;
    SHELL(s) = NULL;
    DAROACH(s) = NULL;
    BOB(s) = 0;
    obj->base.flags |= 0x400;
    obj->base.unk62 |= 4;
    SqueakPlayComposite(s, COMPOSITE_BODY);
}

/* Its legs (0x021182A4 and on, 0x021178E8 and on), SQUEAK_YADGAINE_LEG (the variant 0 its back leg, 1 its front one): where it is,
 * in their mode (vars[1]); their HP (vars[0]: Squeak Squad's, KatAM's kept LEG_HP_OFFSET over it); out, broken (.mca's 4, 5) for good,
 * hurting by touch, untouchable (KatAM's level 6). Each time a foot touches the ground (yadgaine.rca's 0x201 box: walking, anims 8
 * and 9's frames 5 and 27; resting after a strike, 18 and 22's 3), resting, of 100: under 4, 7, 12 a block (0x021179CC, 0x02118384:
 * its kinds 2-4) from its foot (the front leg's 40 ahead, the back one's 12 behind, 1 up); */
#define L_HP(l) ((l)->vars[0])
#define L_MODE(l) ((l)->vars[1])
#define L_LAST(l) ((l)->vars[2])
#define L_HARM(l) ((l)->vars[3])
#define L_SHOWN(l) ((l)->vars[4])
#define L_PENDING(l) ((l)->vars[5])     // Raised: its held composite next frame (0x021180E4's state change, 0x02118050's next);
#define L_BACK(l) ((l)->variant == 0)

// yadgaine.rca's boxes (KatAM's: x ahead, y down; frames, left, top, right, bottom; the DS's, SIZED), its legs' composites' first parts;
struct LegBox {
    u8 frames;
    s8 box[4];
};
static const struct LegBox sWalkBack[] = {      // anim 8 (44 frames);
    { 4, { -50, -80, -21, -28 } }, { 1, { -40, -63, -11, -11 } }, { 1, { -35, -45, -6, 7 } }, { 1, { -36, -32, -7, 20 } },
    { 1, { -35, -35, -6, 17 } }, { 1, { -37, -41, -8, 11 } }, { 1, { -37, -42, -8, 10 } }, { 1, { -37, -37, -8, 15 } },
    { 1, { -37, -38, -8, 14 } }, { 1, { -38, -35, -9, 17 } }, { 1, { -38, -33, -9, 19 } }, { 1, { -39, -37, -10, 15 } },
    { 1, { -41, -33, -12, 19 } }, { 1, { -47, -33, -18, 19 } }, { 1, { -56, -39, -27, 13 } }, { 1, { -63, -42, -34, 10 } },
    { 1, { -63, -40, -34, 12 } }, { 1, { -62, -42, -33, 10 } }, { 1, { -63, -41, -34, 11 } }, { 1, { -64, -44, -35, 8 } },
    { 1, { -64, -43, -35, 9 } }, { 1, { -64, -42, -35, 10 } }, { 1, { -65, -46, -36, 6 } }, { 1, { -65, -48, -36, 4 } },
    { 1, { -64, -53, -35, -1 } }, { 1, { -65, -53, -36, -1 } }, { 1, { -59, -58, -30, -6 } }, { 1, { -60, -58, -31, -6 } },
    { 1, { -53, -62, -24, -10 } }, { 1, { -53, -61, -24, -9 } }, { 1, { -52, -62, -23, -10 } }, { 1, { -49, -66, -20, -14 } },
    { 1, { -49, -68, -20, -16 } }, { 1, { -50, -67, -21, -15 } }, { 1, { -49, -67, -20, -15 } }, { 1, { -44, -71, -15, -19 } },
    { 1, { -44, -70, -15, -18 } }, { 1, { -45, -70, -16, -18 } }, { 1, { -44, -69, -15, -17 } }, { 1, { -45, -69, -16, -17 } },
    { 1, { -44, -70, -15, -18 } },
};
static const struct LegBox sWalkFront[] = {     // anim 9 (44);
    { 6, { -11, -48, 18, 4 } }, { 1, { -9, -55, 20, -3 } }, { 1, { -4, -59, 25, -7 } }, { 1, { -4, -60, 25, -8 } },
    { 1, { 3, -66, 32, -14 } }, { 1, { 2, -63, 31, -11 } }, { 1, { 3, -64, 32, -12 } }, { 2, { 5, -67, 34, -15 } },
    { 1, { 6, -68, 35, -16 } }, { 1, { 7, -70, 36, -18 } }, { 1, { 11, -71, 40, -19 } }, { 1, { 12, -71, 41, -19 } },
    { 2, { 11, -70, 40, -18 } }, { 1, { 14, -69, 40, -17 } }, { 1, { 10, -69, 39, -17 } }, { 1, { 13, -67, 42, -15 } },
    { 1, { 12, -66, 41, -14 } }, { 1, { 13, -67, 42, -15 } }, { 1, { 12, -66, 41, -14 } }, { 1, { 14, -60, 43, -8 } },
    { 1, { 22, -42, 51, 10 } }, { 2, { 20, -32, 49, 20 } }, { 2, { 18, -38, 47, 14 } }, { 1, { 17, -31, 46, 21 } },
    { 2, { 18, -32, 47, 20 } }, { 1, { 18, -33, 47, 19 } }, { 1, { 17, -32, 46, 20 } }, { 1, { 16, -29, 45, 23 } },
    { 1, { 9, -29, 38, 23 } }, { 1, { 1, -28, 30, 24 } }, { 1, { -7, -31, 22, 21 } }, { 1, { -6, -32, 23, 20 } },
    { 1, { -8, -30, 21, 22 } }, { 1, { -6, -30, 23, 22 } },
};
static const struct LegBox sRaiseBack[] = { { 3, { -52, -60, -23, -5 } }, { 3, { -49, -72, 5, -41 } }, { 1, { -47, -96, 7, -65 } } };
static const struct LegBox sRaiseFront[] = { { 3, { 3, -59, 34, -6 } }, { 3, { 3, -94, 66, -66 } }, { 1, { 3, -114, 61, -88 } } };
static const struct LegBox sRaisedBack[] = { { 2, { -46, -90, 1, -62 } } };
static const struct LegBox sRaisedFront[] = { { 2, { 7, -113, 53, -85 } } };
static const struct LegBox sStrikeBack[] = { { 2, { -53, -75, -6, -47 } } };
static const struct LegBox sStrikeFront[] = { { 7, { 20, -93, 66, -65 } } };
static const struct LegBox sRestBack[] = {      // anim 18 (10);
    { 2, { -50, -70, -19, -17 } }, { 1, { -37, -65, -6, -12 } }, { 1, { -30, -48, 1, 5 } }, { 1, { -31, -34, 0, 19 } },
    { 1, { -32, -39, -1, 14 } }, { 1, { -34, -36, -3, 17 } }, { 1, { -33, -33, -2, 20 } }, { 1, { -32, -34, -1, 19 } },
    { 1, { -34, -30, -3, 23 } },
};
static const struct LegBox sRestFront[] = {     // anim 22 (10);
    { 2, { 10, -83, 43, -22 } }, { 1, { 17, -70, 50, -9 } }, { 1, { 22, -52, 55, 9 } }, { 1, { 21, -40, 54, 21 } },
    { 1, { 21, -48, 54, 13 } }, { 1, { 21, -40, 54, 21 } }, { 1, { 21, -41, 54, 20 } }, { 1, { 20, -40, 53, 21 } },
    { 1, { 19, -37, 52, 24 } },
};
static const struct LegBox sBrokenBack[] = { { 8, { -59, -69, -30, -33 } } };
static const struct LegBox sBrokenFront[] = { { 8, { 7, -69, 36, -42 } } };
// Their foot's touch (the 0x201 box's frame), resting and walking;
#define CONTACT_REST 3
#define CONTACT_WALK_BACK 5
#define CONTACT_WALK_FRONT 27

static const struct LegBox *LegBoxAt(const struct LegBox *b, u8 n, u32 time, bool32 loops) {
    u32 total = 0;
    u8 i;

    for (i = 0; i < n; i++) total += b[i].frames;
    if (loops) time %= total;
    for (i = 0; i < n; i++) {
        if (time < b[i].frames) return &b[i];
        time -= b[i].frames;
    }
    return &b[n - 1];
}

static void LegBoxes(struct SqueakObject *l) {
    struct ObjectBase *base = &l->obj.base;
    bool32 back = L_BACK(l);
    const struct LegBox *b;
    u32 t = l->compositeTime;

    switch (L_MODE(l)) {
    case LEG_WALK: case LEG_WALK_ON:
        b = back ? LegBoxAt(sWalkBack, ARRAY_COUNT(sWalkBack), t, TRUE) : LegBoxAt(sWalkFront, ARRAY_COUNT(sWalkFront), t, TRUE);
        break;
    case LEG_RAISE: b = back ? LegBoxAt(sRaiseBack, 3, t, FALSE) : LegBoxAt(sRaiseFront, 3, t, FALSE); break;
    case LEG_RAISED: b = back ? sRaisedBack : sRaisedFront; break;
    case LEG_STRIKE: b = back ? sStrikeBack : sStrikeFront; break;
    case LEG_REST:
        b = back ? LegBoxAt(sRestBack, ARRAY_COUNT(sRestBack), t, TRUE) : LegBoxAt(sRestFront, ARRAY_COUNT(sRestFront), t, TRUE);
        break;
    default: b = back ? sBrokenBack : sBrokenFront; break;
    }
    if (!L_SHOWN(l)) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    base->flags &= ~0x200;
    sub_0803E2B0(base, SIZED(b->box[0]), SIZED(b->box[1]), SIZED(b->box[2]), SIZED(b->box[3]));
    base->unk68 = L_HARM(l);
    base->unk5C = (base->unk5C & ~7) | (L_HP(l) > 0 ? 3 : 6) | 0x20 | 0x80;
}

// Debris(); A foot resting on the ground (0x021179CC, 0x02118384): of 100, a block knocked up from it;
static void Debris(struct SqueakObject *l) {
    u32 r = Random(100);
    s32 x = l->obj.base.x + (SIZED(L_BACK(l) ? -12 : 40) << 8), y = l->obj.base.y - (SIZED(1) << 8);

    if (r < DEBRIS_2) Spawn(l, SQUEAK_YADGAINE_THING, 2, x, y);
    else if (r < DEBRIS_3) Spawn(l, SQUEAK_YADGAINE_THING, 3, x, y);
    else if (r < DEBRIS_4) Spawn(l, SQUEAK_YADGAINE_THING, 4, x, y);
}

static bool32 Leg(struct SqueakObject *l) {
    const u8 *c = sLegComposites[L_BACK(l) ? 0 : 1];

    switch (l->state) {
    case 0:                 // Its mode (0x0211851C);
        SqueakStop(l);
        L_PENDING(l) = FALSE;
        if (L_HP(l) <= 0) L_MODE(l) = LEG_BROKEN;
        SqueakPlayComposite(l, c[L_MODE(l)]);
        SqueakSetState(l, 1);
        return TRUE;
    case 1:
        // (raised and struck down: held, then on to the next, when its composite's over: 0x021180E4, 0x02117EE4);
        if (L_PENDING(l)) {
            L_PENDING(l) = FALSE;
            L_MODE(l)++;
            SqueakPlayComposite(l, c[L_MODE(l)]);
            SqueakSfx(l, 0x123);
        } else if (L_MODE(l) == LEG_RAISE && l->animEnded) {
            L_PENDING(l) = TRUE;
        } else if (L_MODE(l) == LEG_STRIKE && l->animEnded) {
            L_MODE(l)++;
            SqueakPlayComposite(l, c[L_MODE(l)]);
        }
        if (L_MODE(l) == LEG_REST && l->compositeTime % 10 == CONTACT_REST) Debris(l);
        break;
    }
    return FALSE;
}

/* LegHit(); 0x02118454, 0x02117A9C: its HP its own (KatAM's kept over LEG_HP_OFFSET: a leg isn't beaten by KatAM), broken at 0 for
 * good (0x02117E20, 0x02117460); the boss's hits come here too (YadgaineMain's Hit()); */
static void LegHit(struct SqueakObject *l) {
    struct Object *obj = &l->obj;
    s32 damage = L_LAST(l) - obj->unk80;

    if (damage > 0 && L_HP(l) > 0) {
        L_HP(l) -= damage;
        if (L_HP(l) <= 0) {
            L_HP(l) = 0;
            L_MODE(l) = LEG_BROKEN;
            SqueakSetState(l, 0);
        }
    }
    obj->unk80 = LEG_HP_OFFSET + (L_HP(l) > 0 ? L_HP(l) : 1);
    L_LAST(l) = obj->unk80;
}

static void LegMain(struct Object *obj) {
    struct SqueakObject *l = (struct SqueakObject *)obj, *y = (struct SqueakObject *)obj->base.parent;

    if (y == NULL || !Alive(y, SQUEAK_YADGAINE) || y->obj.unk80 <= 0) {
        obj->base.flags |= 0x1000;
        return;
    }
    LegHit(l);
    obj->base.x = y->obj.base.x;
    obj->base.y = y->obj.base.y;
    obj->base.flags &= ~1;
    L_SHOWN(l) = y->vars[11];
    if (L_SHOWN(l)) obj->base.flags &= ~0x400;
    else obj->base.flags |= 0x400;
    obj->base.sprite.unk14 = y->obj.base.sprite.unk14 + (L_BACK(l) ? 0x40 : -0x40);
    SqueakRun(l, Leg);
    SqueakAnimate(l);
    LegBoxes(l);
}

void YadgaineLegStart(struct Object *obj) {
    struct SqueakObject *l = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, LegMain);
    if (l->started) return;
    SqueakSetState(l, 0);
}

// YadgaineLegInit(); Its HP (its table's for its back leg, its front one's by PrmBB05Sl0's 220 to 290), walking, through everything;
void YadgaineLegInit(struct SqueakObject *l) {
    struct Object *obj = &l->obj;

    L_HP(l) = l->variant == 0 ? obj->unk80 : obj->unk80 * LEG_B_NUM / LEG_A_DEN;
    obj->unk80 = LEG_HP_OFFSET + L_HP(l);
    L_LAST(l) = obj->unk80;
    L_MODE(l) = LEG_WALK;
    L_HARM(l) = obj->base.unk68;
    L_SHOWN(l) = FALSE;
    sub_0803E2B0(&obj->base, SIZED(-53), SIZED(-59), SIZED(-24), SIZED(-7));
    ObjectSetBounds(&obj->base, SIZED(-64), SIZED(-112), SIZED(64), SIZED(24));
    obj->base.flags |= 0x100 | 0x400 | 0x1000000 | 0x2000000;
    // (Kirby's body doesn't hurt it, unk5C 0x80: KatAM's touch of a flashing Kirby beats an enemy outright; nor is it swallowed, 0x20);
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0x20 | 0x80;
    SqueakPlayComposite(l, sLegComposites[l->variant == 0 ? 0 : 1][LEG_WALK]);
}

/* Its things (0x0212E248, 0x020B49D8: thrown things), SQUEAK_YADGAINE_THING (the variant Squeak Squad's kind, the parent the boss or a
 * leg), drawn with yadgaine's animations 32-36 (OnlyAnims 0-4): 0 its shot's puff (35: 0.5 forward, gone at its end), 1 its capsule
 * (36: 3.5 forward, through everything), 2-4 blocks (34: thrown 2.0, 2.0 or 3.0 forward, 4.0, 5.0 or 3.0 up, 0.1 gravity; landed, a
 * solid tile of KatAM's (16 pixels: its cell, stepping a cell back each 16 pixels the screen would scroll), knocked down off an edge,
 * broken under the boss 32 pixels ahead of its leg, by a hit or swallowed), 5 a small rock (33), 6 a big one (32: falling from the view's
 * top at the boss's speed, 0.1 gravity, breaking on the ground). Everything of the boss's fight: the scroll taken out (Back()); gone out
 * of the room. Kirby's body doesn't hurt them (unk5C 0x80); no HP bar (flag 0x1000000); */
enum { THING_PUFF, THING_CAPSULE, THING_BLOCK, THING_BLOCK2, THING_BLOCK3, THING_ROCK_SMALL, THING_ROCK_BIG };
static const u8 sThingAnims[] = { 3, 4, 2, 2, 2, 1, 0 };
static const s32 sBlockLaunch[3][2] = { { 0x2000, 0x4000 }, { 0x2000, 0x5000 }, { 0x3000, 0x3000 } };    // Ml0's +0x7C..+0x90;
#define THING_GRAVITY 0x19A     // Ml0's +0x08 (410);
#define PUFF_SPEED 0x800        // +0xAC;
#define CAPSULE_EXTRA 0x3000    // +0xC0;
#define CRUSH_AHEAD 32          // 0x0212D8F0;
#define T_HARM(t) ((t)->vars[0])
#define T_CELL_X(t) ((t)->vars[1])
#define T_CELL_Y(t) ((t)->vars[2])
#define T_OLD(t) ((t)->vars[3])         // The cell's own code (-1: none taken);
#define T_REAL(t) ((t)->vars[4])        // Where Squeak Squad's block would be (the scroll taken out), its cell's;
#define T_BOX(t) ((t)->vars[5])

static bool32 Block(struct SqueakObject *t) {
    return t->variant >= THING_BLOCK && t->variant <= THING_BLOCK3;
}

// FreeCell(), TakeCell(); A landed block's tile (KatAM's collision, sub_0800255C: the solid floor's code under it) given back or taken;
static void FreeCell(struct SqueakObject *t) {
    if (T_OLD(t) < 0) return;
    sub_0800255C(t->obj.base.unk56, T_CELL_X(t), T_CELL_Y(t), T_OLD(t));
    T_OLD(t) = -1;
}

static bool32 TakeCell(struct SqueakObject *t, s32 x) {
    u8 p = t->obj.base.unk56;
    s32 cx = x >> 12, cy = (t->obj.base.y - 1) >> 12;
    u8 own = GetCollisionTile(p, cx, cy), floor = GetCollisionTile(p, cx, cy + 1);

    FreeCell(t);
    if (!(gCollisionAttributes[floor] & 1) || (gCollisionAttributes[own] & 1)) return FALSE;
    T_CELL_X(t) = cx;
    T_CELL_Y(t) = cy;
    T_OLD(t) = own;
    sub_0800255C(p, cx, cy, floor);
    t->obj.base.x = (cx << 12) + 0x800;
    return TRUE;
}

static void ThingGone(struct SqueakObject *t, bool32 burst) {
    FreeCell(t);
    if (burst) SqueakSpawnEffect(t, SQUEAK_FX_BURST, 0, 0, FALSE);
    t->obj.base.flags |= 0x1000;
}

static bool32 Thing(struct SqueakObject *t) {
    struct SqueakObject *by = (struct SqueakObject *)t->obj.base.parent;

    switch (t->state) {
    case 0:
        SqueakPlay(t, sThingAnims[t->variant]);
        SqueakStop(t);
        if (t->variant == THING_PUFF) {
            t->vx = PUFF_SPEED;
        } else if (t->variant == THING_CAPSULE) {
            t->vx = PUFF_SPEED + CAPSULE_EXTRA;
        } else if (Block(t)) {
            t->vx = SIZED(sBlockLaunch[t->variant - THING_BLOCK][0]);
            t->vy = SIZED(sBlockLaunch[t->variant - THING_BLOCK][1]);
            t->ay = -SIZED(THING_GRAVITY);
        } else {
            struct SqueakObject *y = YadgaineOf(t);

            t->vx = y ? y->vx : 0;
            t->ay = -THING_GRAVITY;
        }
        SqueakSetState(t, 1);
        return TRUE;
    case 1:                 // Flying (0x0212DF30, 0x0212DE54, 0x0212D9C8, 0x0212D7BC);
        if (t->variant == THING_PUFF || t->variant == THING_CAPSULE) {
            t->obj.base.x += SQUEAK_SPEED(t->vx);
            if (t->variant == THING_PUFF && t->animEnded) ThingGone(t, FALSE);
            break;
        }
        SqueakMove(t);
        if (Block(t)) {
            if (t->vy < 0 && by && Alive(by, SQUEAK_YADGAINE_LEG) && t->obj.base.x < by->obj.base.x + (SIZED(CRUSH_AHEAD) << 8)) {
                ThingGone(t, TRUE);
                break;
            }
            if (t->vy <= 0 && SqueakOnGround(t)) {
                SqueakStop(t);
                T_REAL(t) = t->obj.base.x;
                SqueakSfx(t, 0x51);
                if (TakeCell(t, T_REAL(t))) SqueakSetState(t, 2);
                else t->ay = -SIZED(THING_GRAVITY);
            }
        } else if (SqueakOnGround(t)) {
            ThingGone(t, TRUE);
        }
        break;
    case 2:                 // A block landed (0x0212D8F0): a solid tile till it's under the boss (32 ahead of its leg);
        T_REAL(t) -= Back(t);
        if (YadgaineOf(t) == NULL || (by && Alive(by, SQUEAK_YADGAINE_LEG) && T_REAL(t) < by->obj.base.x + (SIZED(CRUSH_AHEAD) << 8))) {
            ThingGone(t, TRUE);
            break;
        }
        if ((T_REAL(t) >> 12) != T_CELL_X(t) && !TakeCell(t, T_REAL(t))) {
            t->obj.base.x = T_REAL(t);
            t->ay = -SIZED(THING_GRAVITY);
            SqueakSetState(t, 1);
        }
        return FALSE;
    }
    t->obj.base.x -= Back(t);
    return FALSE;
}

static void ThingMain(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;
    static const s8 sBoxes[][4] = {
        [THING_PUFF] = { 0, 0, 0, 0 }, [THING_CAPSULE] = { -21, -8, 16, 7 }, [THING_BLOCK] = { -4, -12, 4, -4 },
        [THING_BLOCK2] = { -4, -12, 4, -4 }, [THING_BLOCK3] = { -4, -12, 4, -4 }, [THING_ROCK_SMALL] = { -9, -13, 9, 5 },
        [THING_ROCK_BIG] = { -10, -15, 12, 7 },
    };

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (obj->unk80 <= 0 || !SqueakInLevel(t)) {
        ThingGone(t, FALSE);
        return;
    }
    SqueakRun(t, Thing);
    if (obj->base.flags & 0x1000) return;
    SqueakFinishFrame(t);
    SqueakAnimate(t);
    if (t->variant != THING_PUFF) sub_0803E2B0(&obj->base, sBoxes[t->variant][0], sBoxes[t->variant][1], sBoxes[t->variant][2],
                                               sBoxes[t->variant][3]);
}

void YadgaineThingStart(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, ThingMain);
    if (t->started) return;
    SqueakSetState(t, 0);
}

void YadgaineThingInit(struct SqueakObject *t) {
    struct Object *obj = &t->obj;

    T_HARM(t) = obj->base.unk68;
    T_OLD(t) = -1;
    sub_0803E2B0(&obj->base, -6, -6, 6, 6);
    ObjectSetBounds(&obj->base, -8, -16, 8, 8);
    obj->base.unk5C |= 0x80;
    obj->base.flags |= 0x1000000;
    if (t->variant == THING_PUFF) {
        obj->base.unk5C |= 0x20;
        obj->base.flags |= 0x200;
        obj->base.unk68 = 0;
    }
    if (t->variant == THING_CAPSULE || t->variant == THING_PUFF) obj->base.flags |= 0x100;
}

/* A gordo (0x0212E198's kind 7: enemy/gordo's animation 3), SQUEAK_YADGAINE_GORDO: falling from the view's top at the boss's speed
 * (0.1 gravity); on the ground it bounces once (2.5 up, 0x0212D59C) and falls through everything out of the room; nothing hurts it
 * (KatAM's level 6, as its Gordo), nor is it swallowed; its touch hurts; */
#define GORDO_BOUNCE 0x2800     // Ml0's +0x04;

static bool32 Gordo(struct SqueakObject *g) {
    switch (g->state) {
    case 0: {
        struct SqueakObject *y = YadgaineOf(g);

        SqueakPlay(g, 0);
        SqueakStop(g);
        g->vx = y ? y->vx : 0;
        g->ay = -THING_GRAVITY;
        SqueakSetState(g, 1);
        return TRUE;
    }
    case 1:                 // 0x0212D59C;
        SqueakMove(g);
        if (SqueakOnGround(g)) {
            g->vx = 0;
            g->vy = GORDO_BOUNCE;
            g->ay = -THING_GRAVITY;
            g->obj.base.flags |= 0x100;
            SqueakSetState(g, 2);
        }
        break;
    case 2:                 // 0x0212D570;
        SqueakMove(g);
        break;
    }
    g->obj.base.x -= Back(g);
    return FALSE;
}

static void GordoMain(struct Object *obj) {
    struct SqueakObject *g = (struct SqueakObject *)obj;

    if (!SqueakInLevel(g)) {
        obj->base.flags |= 0x1000;
        return;
    }
    SqueakRun(g, Gordo);
    SqueakFinishFrame(g);
    SqueakAnimate(g);
}

void YadgaineGordoStart(struct Object *obj) {
    struct SqueakObject *g = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, GordoMain);
    if (g->started) return;
    SqueakSetState(g, 0);
}

void YadgaineGordoInit(struct SqueakObject *g) {
    sub_0803E2B0(&g->obj.base, -10, -18, 10, 1);
    ObjectSetBounds(&g->obj.base, -10, -18, 10, 1);
    g->obj.base.unk5C = (g->obj.base.unk5C & ~7) | 6 | 0x20 | 0x80;
    g->obj.base.flags |= 0x1000000;
}

/* Daroach bringing it in (0x0212E198's kind 8: demo_dancho's animations 1-5), SQUEAK_YADGAINE_DAROACH: over it, his animations 1 (its
 * loop once), 2, 3 (20 frames), 4, then 5 flying off back and up at 2.0 (0x0212D1DC) out of the room; harmless, untouchable; */
#define DAROACH_HOLD 20
#define DAROACH_FLY 0x2000

static bool32 Daroach(struct SqueakObject *d) {
    switch (d->state) {
    case 0:
        SqueakPlay(d, 0);
        SqueakSetState(d, 1);
        return TRUE;
    case 1:                 // 0x0212D434, 0x0212D384;
        if (d->animEnded) {
            SqueakPlay(d, d->anim + 1);
            if (d->anim == 2) {
                d->vars[0] = DAROACH_HOLD;
                SqueakSetState(d, 2);
            }
        }
        break;
    case 2:                 // 0x0212D2D4;
        if (d->vars[0]-- < 1) {
            SqueakPlay(d, 3);
            SqueakSetState(d, 3);
        }
        break;
    case 3:                 // 0x0212D20C;
        if (d->animEnded) {
            SqueakPlay(d, 4);
            d->vx = -DAROACH_FLY;
            d->vy = DAROACH_FLY;
            SqueakSetState(d, 4);
        }
        break;
    case 4:
        d->obj.base.x += SQUEAK_SPEED(d->vx);
        d->obj.base.y -= SQUEAK_SPEED(d->vy);
        break;
    }
    d->obj.base.x -= Back(d);
    return FALSE;
}

static void DaroachMain(struct Object *obj) {
    struct SqueakObject *d = (struct SqueakObject *)obj;

    if (!SqueakInLevel(d)) {
        obj->base.flags |= 0x1000;
        return;
    }
    SqueakRun(d, Daroach);
    d->moved = TRUE;
    SqueakFinishFrame(d);
    SqueakAnimate(d);
}

void YadgaineDaroachStart(struct Object *obj) {
    struct SqueakObject *d = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, DaroachMain);
    obj->base.flags |= 0x200;
    obj->base.unk68 = 0;
    if (d->started) return;
    SqueakSetState(d, 0);
}

void YadgaineDaroachInit(struct SqueakObject *d) {
    sub_0803E2B0(&d->obj.base, -4, -4, 4, 4);
    ObjectSetBounds(&d->obj.base, -16, -32, 16, 0);
    d->obj.base.unk5C |= 0x20 | 0x80;
    d->obj.base.flags |= 0x100 | 0x200 | 0x1000000 | 0x2000000;
}
