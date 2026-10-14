/* Slippy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x22 (spawn 0x08089C58), as NiDL enemy NIDL_SLIPPY.
 * NiDL scripts on land (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • 0x08089DA8: sits facing Kirby (frame 4) for 60 frames, then decides (0x0808A7F4, p = 15).
 * • 0x08089EC8: looks around 3 times (frame 4 for 4 frames, then turning around, or blinking with frame 5, at random for the
 *   3 of them, for 16), then decides (p = 11 or 15, every other time).
 * • Deciding (0x0808A84C): r = random 0-p; the choice is the first of 6 thresholds r doesn't pass, from one table one time,
 *   the other the next (0x08742856: 3, 4, 8, 9, 11, 16; 0x0874285C: 1, 2, 9, 13, 15, 16). Choices: jump low, jump high, jump
 *   low forward (1.0), jump high forward, jump low forward turning around, look around.
 * • Low jump 0x0808A048: -5.0, gravity 0.5. High jump 0x0808A0AC: -6.0, gravity 0.25. Frame 8 going up, 6 for 2 frames at the
 *   top, then 5; landing it sits again.
 * NiDL's Slippy also swims (states 4-10, when it starts in water); that isn't ported; */
#include "../nidl.h"

// Frames: NiDL's 4 (sitting), 5 (blinking, falling), 6 (top of a jump), 8 (jumping). Its hurt frame is NiDL's 3;
enum { FRAME_SIT, FRAME_BLINK, FRAME_TOP, FRAME_JUMP };

static const u8 sThresholds[2][6] = { { 3, 4, 8, 9, 11, 16 }, { 1, 2, 9, 13, 15, 16 } };  // 0x08742856, 0x0874285C;
static const u8 sLookP[2] = { 11, 15 };                                                     // 0x08742854;

enum { CHOICE_JUMP, CHOICE_JUMP_HIGH, CHOICE_JUMP_FORWARD, CHOICE_JUMP_HIGH_FORWARD, CHOICE_JUMP_BACK, CHOICE_LOOK };
enum { STATE_SIT, STATE_LOOK, STATE_JUMP };

#define SPEED 0x100
#define MAX_FALL 0x400
#define SIT_TIME 60
#define LOOKS 3

struct Slippy {
    struct NidlObject n;
    s16 gravity;
    u8 state;
    u8 timer;
    u8 decisions;
    u8 looks;
    u8 lookAround;  // Turning around (else blinking) this time;
    u8 airborne;
};

// Decide(); 0x0808A7F4: one of the 6 choices (p = 0 takes it from 0x08742854);
static u8 Decide(struct Slippy *s, u8 p) {
    u8 table, r, choice;

    s->decisions++;
    if (!p) p = sLookP[s->decisions & 1];
    table = (s->decisions & 1) ? 1 : 0;
    r = Rand16() % (p + 1);
    for (choice = 0; sThresholds[table][choice] < r; choice++);
    return choice;
}

static void Sit(struct Slippy *s) {
    s->state = STATE_SIT;
    s->timer = SIT_TIME;
    s->n.obj.base.xspeed = 0;
    NidlFaceKirby(&s->n.obj);
    NidlSetFrame(&s->n, FRAME_SIT);
}

static void Look(struct Slippy *s) {
    s->state = STATE_LOOK;
    s->looks = 0;
    s->lookAround = Rand16() % 3 != 0;
    s->timer = 4;
    NidlSetFrame(&s->n, FRAME_SIT);
}

static void Jump(struct Slippy *s, u8 choice) {
    struct Object *obj = &s->n.obj;
    bool8 high = choice == CHOICE_JUMP_HIGH || choice == CHOICE_JUMP_HIGH_FORWARD;

    s->state = STATE_JUMP;
    s->airborne = FALSE;
    obj->base.yspeed = high ? 0x600 : 0x500;
    s->gravity = high ? 0x40 : 0x80;
    if (choice == CHOICE_JUMP_BACK) obj->base.flags ^= 1;
    if (choice >= CHOICE_JUMP_FORWARD) obj->base.xspeed = (obj->base.flags & 1) ? -SPEED : SPEED;
    NidlSetFrame(&s->n, FRAME_JUMP);
}

static void Act(struct Slippy *s, u8 choice) {
    if (choice == CHOICE_LOOK)
        Look(s);
    else
        Jump(s, choice);
}

static void SlippyMain(struct Object *obj) {
    struct Slippy *s = (struct Slippy *)obj;

    NidlUpdate(&s->n);
    obj->base.flags |= 4;

    switch (s->state) {
    case STATE_SIT:
        if (!--s->timer) Act(s, Decide(s, 15));
        break;
    case STATE_LOOK:
        // 4 frames sitting, then 16 turned around or blinking, 3 times;
        if (--s->timer) break;
        if (++s->looks == LOOKS * 2) {
            Act(s, Decide(s, 0));
        } else if (s->looks & 1) {
            if (s->lookAround)
                obj->base.flags ^= 1;
            else
                NidlSetFrame(&s->n, FRAME_BLINK);
            s->timer = 16;
        } else {
            NidlSetFrame(&s->n, FRAME_SIT);
            s->timer = 4;
        }
        break;
    case STATE_JUMP:
        obj->base.yspeed -= s->gravity;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (!(obj->base.unk62 & 4)) s->airborne = TRUE;
        if (obj->base.yspeed <= 0 && s->n.obj.unk83 == s->n.animBase + FRAME_JUMP) {
            NidlSetFrame(&s->n, FRAME_TOP);
            s->timer = 2;
        } else if (s->timer && !--s->timer) {
            NidlSetFrame(&s->n, FRAME_BLINK);
        }
        if (s->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            Sit(s);
        }
        break;
    }
}

// SlippyStart(); Behavior start, also called by the engine after Slippy recovers from a hit;
void SlippyStart(struct Object *obj) {
    struct Slippy *s = (struct Slippy *)obj;

    ObjectSetFunc(obj, 0, SlippyMain);
    obj->base.yspeed = 0;
    Sit(s);
}

// SlippyInit(); Its box, on the floor;
void SlippyInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -5, 6, 7);
    ObjectSetBounds(&obj->base, -7, -6, 7, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
