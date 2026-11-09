/* The dark star (Squeak Squad's enemy 0x3E); ported from Kirby: Squeak Squad (USA): the dark star Kirby chases through a8s4s3 (a
 * corridor) and a8s4s4 (a shaft) before Dark Nebula comes, as SQUEAK_DARK_STAR. Its body 0x0211B680 (vtable 0x021828F8; its states
 * 0x0211A904-0x0211B4C8); values from param/prmLB01Ml0.kgb (3.5, 0.15, 4.0, 0.15: ML[n]) and prmLB01Pl0.kgb (120, 90, 64, 16, 4137,
 * 10, 90, 60, 30: PL[n]); speeds 20.12 a frame, Y up. Drawn with enemy/darkZERO_cmn's animation 1 (the small spinning star): its
 * .rca has only a body box there (no attack or hurt box), so it hurts nobody and nothing hits it.
 * • Its route is its stage's (the map's section 9: a zone per tile, points of u8 index, u16 x, y, size; 0x02209C7C, 0x02209CC0):
 *   a8s4s3's points 0-7 run along the corridor 8 under its floor (y 168), point 8 (size 255) is the end, 68 from the stage's right
 *   end, 20 over that line, and points 9-18 are the end's loop. KatAM's rooms have none, so here the route is the corridor's, laid
 *   on the room it's in: its line under the floor it first bounces on, its end at the room's side it heads for (DS spots from the
 *   stage's end: the end point 68 in, Kirby's camera lock at the end 276 in, the star's end 196 in).
 * • Recorded on the DS (a8s4s3 and a8s4s4 loaded by repointing the stage table; Kirby walked and floated along by a script):
 *   - It faces away from Kirby as it's made (0x0211B680) and moves backward: toward Kirby's side, past him and on (a8s4s3's starts
 *     behind him, at the stage's left end).
 *   - Start (0x0211B4C8, 0x0211B418): 1.2 a frame, 0.03 gravity, on the terrain; on a floor a bounce (3.5 up, 0.15 gravity from then
 *     on); after 120 frames its hops.
 *   - Hops (0x0211AED8, 0x0211AE38), through everything: 3.5 up, 0.15 gravity, across at Kirby's speed + 0.88 (0 while it's more
 *     than 64 pixels ahead of him: it waits); a new hop once it falls as fast as it rose, or comes within 12 pixels of the route's
 *     line, or falls below Kirby (each frame: it rises at 3.35 till it's over him).
 *   - The end (its hop starting within 128 pixels of the end point): it holds still till Kirby's camera locks on the stage's end
 *     (the camera's +0x84 bit 8, 0x0211AA50: here Kirby past the same spot), then its loop (0x0211ACBC, 0x0211AAE0): quadratic
 *     Bezier legs from where it is through points 9-10, 11-12, 13-14, 15-16 and 17-18 (90, 60, 30, 30, 30 frames), then on at a
 *     quarter of the last leg's end speed (0x0211AA74, through everything) till it's out of the camera's view (0x02209D00's
 *     margins 1), and the loop again from there (0x0211B37C, 0x0211B2EC), for as long as Kirby's in its room.
 * Checked in KatAM (USA, room 0x323, against the DS's recordings): the drift, the hops and their waits, a hop frame by frame (3.35 up
 * to -3.407 in 45 frames, as the DS's), the end and its loop both ways, Kirby touching and inhaling it (nothing).
 * Not ported: its sounds (0x1C7 as it comes, 0x1C8 each hop, 0x1C9 its first legs); a8s4s4's route (the same moves up a shaft: its
 * points zigzag, its hops turn with them, 0.2 + Kirby's speed); the route's line before its first point (a slope from the star). */
#include "../squeak.h"

enum { ANIM_STAR };

enum { STATE_START, STATE_DRIFT, STATE_HOP, STATE_HOPPING, STATE_END, STATE_LEG_START, STATE_LEG, STATE_OFF };

// prmLB01Ml0.kgb (+0x204), prmLB01Pl0.kgb (+0x234);
#define HOP_VY 0x3800           // ML[0];
#define HOP_AY (-615)           // ML[1] (615: 0.15);
#define DRIFT_VX (-0x1333)      // 0x0211B4C8's 1.2, backward;
#define DRIFT_AY (-122)         // Its 0.03;
#define DRIFT_TIME 120          // PL[0];
#define HOP_PLUS 3604           // 0x0211AED8's 0.88;
#define AHEAD 64                // PL[2]: pixels;
#define LINE_ABOVE 12           // 0x0211A97C's 12.0;
#define LEG_FIRST 90            // PL[6], PL[7], PL[8];
#define LEG_SECOND 60
#define LEG_REST 30
// a8s4s3's route (pixels): the line 8 under the floor (13 under the star as it bounces), the end point 20 over it;
#define LINE_UNDER_BOUNCE 13
#define LINE_UNDER_SPAWN 100    // (a8s4s3's spawn: 100 over the line; the line till it first bounces);
#define END_IN 68               // From the stage's end: the end point;
#define END_REACH 196           // The star's end (zone 8 and 128 from the end point);
#define END_NEAR 128
#define LOCK_IN 276             // Kirby's camera lock on the end (Kirby at 1388 of 1664);
#define END_OVER 20

// Its loop's points 9-18, from the end point (pixels: across toward the stage's end, down);
static const s16 sLoop[10][2] = {
    { 36, -96 }, { -80, -100 }, { -164, -12 }, { -4, 68 }, { 52, -4 }, { 12, -100 }, { -108, -84 }, { -136, 40 }, { 20, 8 },
    { 44, -128 },
};

#define TIMER(s) ((s)->vars[0])
#define DIR(s) ((s)->vars[1])           // The way it heads: 1 right, -1 left;
#define LINE(s) ((s)->vars[2])          // The route's line (KatAM's y, 8.8);
#define BOUNCED(s) ((s)->vars[3])
#define EDGE(s) ((s)->vars[4])          // The room's side it heads for (KatAM's x, 8.8);
#define LOCKED(s) ((s)->vars[5])        // Kirby's reached the end (the camera's lock: latched, 0x0211AA50's +0x264);
#define POINT(s) ((s)->vars[6])         // Its loop's next point (0-9);
#define LEGS(s) ((s)->vars[7])
#define FRAMES(s) ((s)->vars[8])        // The leg's frames (+0x25C);
#define P0X(s) ((s)->vars[9])           // The leg: from, through, to (8.8);
#define P0Y(s) ((s)->vars[10])
#define P1X(s) ((s)->vars[11])
#define P1Y(s) ((s)->vars[12])
#define P2X(s) ((s)->vars[13])
#define P2Y(s) ((s)->vars[14])
#define LAST_DX(s) ((s)->vars[15])      // The last leg's end, less its middle point (8.8): the speed it flies off at;
#define LAST_DY(s) ((s)->vars[16])

/* MoveFree(); 0x02013D18 with its flags 0x100 and 0x20000: through everything, not held in the stage (out of the room too: it flies
 * out of the camera's view and back); */
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

// InView(); 0x02209D00's margins 1 (0x022275F4's: 24 pixels above, 56 below and either side): in the camera's view;
static bool32 InView(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x - (DISPLAY_WIDTH / 2 << 8);
    s32 y = s->obj.base.y - level->viewportPosition.y - (DISPLAY_HEIGHT / 2 << 8);

    return x > -((DISPLAY_WIDTH / 2 + 56) << 8) && x < (DISPLAY_WIDTH / 2 + 56) << 8 && y > -((DISPLAY_HEIGHT / 2 + 24) << 8)
           && y < (DISPLAY_HEIGHT / 2 + 56) << 8;
}

// Toward(); How far `x` is from the room's side it heads for (8.8: 0 there, less inside);
static s32 Toward(struct SqueakObject *s, s32 x) {
    return DIR(s) > 0 ? x - EDGE(s) : EDGE(s) - x;
}

// Ahead(); How far it is ahead of Kirby, the way it heads (8.8);
static s32 Ahead(struct SqueakObject *s) {
    return (s->obj.base.x - s->obj.kirby3->base.x) * DIR(s);
}

/* Hop(); 0x0211AED8: 3.5 up, 0.15 gravity, across at Kirby's speed + 0.88 backward (0 more than 64 pixels ahead of him); at its end
 * point, its end instead; */
static bool32 Hop(struct SqueakObject *s) {
    s32 kx = s->obj.kirby3->base.xspeed;
    s32 endY = LINE(s) - (END_OVER << 8);

    if (Toward(s, s->obj.base.x) > -(END_REACH << 8) && s->obj.base.y > endY - (END_NEAR << 8)
        && s->obj.base.y < endY + (END_NEAR << 8)) {
        SqueakSetState(s, STATE_END);
        return FALSE;
    }
    SqueakStop(s);
    s->vy = HOP_VY;
    s->ay = HOP_AY;
    s->vx = -((kx < 0 ? -kx : kx) * 16 + HOP_PLUS);
    if (Ahead(s) > AHEAD << 8) s->vx = 0;
    SqueakSetState(s, STATE_HOPPING);
    return TRUE;
}

// LegStart(); 0x0211ACBC: the next leg of its loop, from where it is; past the last, on at the last leg's end speed;
static bool32 LegStart(struct SqueakObject *s) {
    s32 x = EDGE(s) - DIR(s) * (END_IN << 8), y = LINE(s) - (END_OVER << 8);
    u8 p = POINT(s);

    TIMER(s) = 0;
    if (p < 10) {
        FRAMES(s) = LEGS(s) == 0 ? LEG_FIRST : LEGS(s) == 1 ? LEG_SECOND : LEG_REST;
        LEGS(s)++;
        P0X(s) = s->obj.base.x;
        P0Y(s) = s->obj.base.y;
        P1X(s) = x + DIR(s) * (sLoop[p][0] << 8);
        P1Y(s) = y + (sLoop[p][1] << 8);
        P2X(s) = x + DIR(s) * (sLoop[p + 1][0] << 8);
        P2Y(s) = y + (sLoop[p + 1][1] << 8);
        LAST_DX(s) = P2X(s) - P1X(s);
        LAST_DY(s) = P2Y(s) - P1Y(s);
        POINT(s) = p + 2;
        SqueakSetState(s, STATE_LEG);
        return TRUE;
    }
    /* (Squeak Squad's: its end over twice its frames, a quarter of a Bezier's end speed; across in the frame of its facing, so it
     * isn't turned round with it: 8.8 to 20.12, Y up); */
    SqueakStop(s);
    s->vx = (LAST_DX(s) << 4) / (FRAMES(s) * 2);
    s->vy = -(LAST_DY(s) << 4) / (FRAMES(s) * 2);
    SqueakSetState(s, STATE_OFF);
    return FALSE;
}

// Leg(); 0x0211AAE0: along the quadratic Bezier curve, t = frames / its frames; at its end, the next leg the frame after;
static void Leg(struct SqueakObject *s) {
    s32 t, a, c1, c2;

    if (++TIMER(s) >= FRAMES(s)) {
        s->obj.base.x = P2X(s);
        s->obj.base.y = P2Y(s);
        SqueakSetState(s, STATE_LEG_START);
        return;
    }
    t = (TIMER(s) << 12) / FRAMES(s);
    a = 0x1000 - t;
    c1 = 2 * t * a >> 12;
    c2 = t * t >> 12;
    s->obj.base.x = P0X(s) + ((c1 * (P1X(s) - P0X(s)) + c2 * (P2X(s) - P0X(s))) >> 12);
    s->obj.base.y = P0Y(s) + ((c1 * (P1Y(s) - P0Y(s)) + c2 * (P2Y(s) - P0Y(s))) >> 12);
}

static bool32 DarkStar(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];

    obj->kirby3 = FindTargetKirby(&obj->base);
    // (0x0211AA50, each frame: Kirby past where the stage's end locks his camera);
    if (DIR(s) != 0 && Toward(s, obj->kirby3->base.x) >= -(LOCK_IN << 8)) LOCKED(s) = TRUE;
    switch (s->state) {
    case STATE_START:       // 0x0211B680, 0x0211B4C8: facing away from Kirby, 1.2 backward, 0.03 gravity;
        DIR(s) = obj->kirby3->base.x - obj->base.x >= 0x100 ? 1 : -1;
        if (DIR(s) > 0) obj->base.flags |= 1;
        else obj->base.flags &= ~1;
        EDGE(s) = DIR(s) > 0 ? level->levelMaxPosition.x : level->levelMinPosition.x;
        LINE(s) = obj->base.y + (LINE_UNDER_SPAWN << 8);
        SqueakPlay(s, ANIM_STAR);
        SqueakStop(s);
        s->vx = DRIFT_VX;
        s->ay = DRIFT_AY;
        TIMER(s) = 0;
        SqueakSetState(s, STATE_DRIFT);
        return TRUE;
    case STATE_DRIFT:       // 0x0211B418: on the terrain, 120 frames; on a floor, a bounce;
        if (++TIMER(s) > DRIFT_TIME) SqueakSetState(s, STATE_HOP);
        SqueakMove(s);
        if (s->terrain & 4) {
            if (!BOUNCED(s)) LINE(s) = obj->base.y + (LINE_UNDER_BOUNCE << 8);
            BOUNCED(s) = TRUE;
            s->vy = HOP_VY;
            s->ay = HOP_AY;
        }
        break;
    case STATE_HOP:
        return Hop(s);
    /* 0x0211AE38: a new hop when it's fallen as fast as it rose, near the line, or under Kirby's feet (Squeak Squad's Kirby's spot;
     * KatAM's is over them by his box's bottom); */
    case STATE_HOPPING:
        MoveFree(s);
        if (s->vy <= -HOP_VY || obj->base.y > LINE(s) - (LINE_ABOVE << 8)
            || obj->base.y > obj->kirby3->base.y + (obj->kirby3->base.unk3F << 8))
            SqueakSetState(s, STATE_HOP);
        break;
    case STATE_END:         // 0x0211B37C, 0x0211B2EC: still, till Kirby's at the end; its loop from its first point;
        SqueakStop(s);
        if (LOCKED(s)) {
            POINT(s) = LEGS(s) = 0;
            SqueakSetState(s, STATE_LEG_START);
        }
        break;
    case STATE_LEG_START:
        return LegStart(s);
    case STATE_LEG:
        Leg(s);
        break;
    case STATE_OFF:         // 0x0211AA74: on till it's out of the camera's view, then the loop again;
        if (!InView(s)) {
            SqueakStop(s);
            SqueakSetState(s, STATE_END);
        }
        MoveFree(s);
        break;
    }
    return FALSE;
}

static void DarkStarMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, DarkStar);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    obj->base.flags |= 0x200;
}

void SqueakDarkStarStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, DarkStarMain);
    if (!s->started) SqueakSetState(s, STATE_START);
}

/* SqueakDarkStarInit(); Its body box (darkZERO_cmn.rca's, animation 1: 4 left, 5 right, 4 up, 5 down) on the terrain; no attack or
 * hurt box: untouchable (KatAM's flag 0x200), Kirby's body and inhale take nothing from it (unk5C 0x80, 0x20), its touch hurts
 * nobody; kept out of the camera's view (flag 0x2000000: its loop goes out and back); */
void SqueakDarkStarInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -4, -4, 5, 5);
    ObjectSetBounds(&s->obj.base, -4, -4, 5, 5);
    s->obj.base.flags |= 0x200 | 0x2000000;
    s->obj.base.unk5C |= 0x20 | 0x80;
    s->obj.base.unk68 = 0;
}
