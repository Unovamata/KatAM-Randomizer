/* Meta Knight; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3D (spawn 0x080A55EC), as NiDL boss
 * NIDL_META_KNIGHT, Orange Ocean's (room 0x29D here, on NiDL's spot, (232, 56): his ledge in NiDL, he drops to the floor here). NiDL's Meta Knight waits there in his
 * cape and throws Kirby a sword ("GET IT!") before the duel; here he sets down KatAM's Sword statue (ThrowSword), and the duel starts
 * once Kirby has a power (or he's hit, or after 10 seconds). Then he leaps down and fights with Galaxia: his body is a streamed frame (0x08753BB4: 0-9 his cape, 21-52 his poses: 21-22
 * stand masked, 33-35 leap, 45-48 dash; NiDL's others show his face bare, so standing, walking and slashing are 21-22 here), his
 * sword a sprite of its own (0x08753DA0's 16-23, eight ways, the descriptor's second palette, gold), which hurts (an attack object
 * following him, held as each pose holds it: sPoses). Its frames (KatAMExpansion.cs): 0-9 the cape, 10-41 the poses (NiDL's 21-52),
 * 42-49 the sword (up, then each eighth of a turn clockwise).
 * What he does (NiDL's moves, by its pictures: walks at Kirby and slashes when close; or crouches and dashes across with his sword
 * out; or leaps at Kirby, sword up, and slashes as he lands). Touching him hurts, as does his sword; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/kirby.h"
#include "constants/songs.h"

#define FRAME_CAPE 0
#define FRAME_SWORD 42
#define BODY(f) (10 + (f) - 21)     // NiDL's 21-52;
#define SWORD_TILES 8
#define SWORD_REACH 12              // From his hand to the blade's middle;

#define WALK_SPEED 0x100
#define DASH_SPEED 0x400
#define LEAP_SPEED 0x180
#define LEAP_UP 0x500
#define GRAVITY 0x30
#define MAX_FALL 0x500
#define CLOSE 40
#define WAIT_TIME 600

enum { STATE_WAIT, STATE_LEAP_DOWN, STATE_WALK, STATE_SLASH, STATE_CROUCH, STATE_DASH, STATE_LEAP, STATE_REST };
// Sword ways (facing right; mirrored facing left);
enum { UP, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN };

// Poses: his frame and how he holds his sword (way, his hand from his middle);
struct Pose {
    u8 body;
    u8 way;
    s8 x, y;
};
enum { POSE_STAND, POSE_WALK, POSE_RAISE, POSE_SWING, POSE_SLASH, POSE_CROUCH, POSE_DASH, POSE_LEAP, POSE_CAPE };
static const struct Pose sPoses[] = {
    [POSE_STAND] = { BODY(21), UP_RIGHT, 10, 2 },
    [POSE_WALK] = { BODY(21), UP_RIGHT, 10, 2 },
    [POSE_RAISE] = { BODY(22), UP, 2, -12 },
    [POSE_SWING] = { BODY(22), UP_RIGHT, 10, -8 },
    [POSE_SLASH] = { BODY(22), DOWN_RIGHT, 12, 4 },
    [POSE_CROUCH] = { BODY(22), RIGHT, 8, 6 },
    [POSE_DASH] = { BODY(45), RIGHT, 16, 2 },
    [POSE_LEAP] = { BODY(33), UP, 4, -10 },
    [POSE_CAPE] = { FRAME_CAPE, UP, 0, 0 },
};
// Way vectors (from the hand to the blade's middle, SWORD_REACH long);
static const s8 sWayX[] = { 0, 9, 12, 9, 12 };
static const s8 sWayY[] = { -12, -9, 0, 9, 12 };

struct MetaKnight {
    struct NidlObject n;
    u8 state;
    u16 timer;
    u8 pose;
    u8 anim;
    u16 starTimer;      // Until the Sword statue;
    u16 wait;
    s16 hp;             // His HP as the duel starts;
    bool8 started;
    bool8 airborne;
    bool8 armed;        // His sword is out (after his cape);
};

typedef char sMetaKnightFits[sizeof(struct MetaKnight) <= NIDL_OBJECT_SIZE ? 1 : -1];

static s32 ScreenX(struct Object *obj) {
    return gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
}

/* SwordMain(); Galaxia: an attack object (as Whispy Woods' puffs) held in his hand as his pose holds it (sPoses), hidden while he
 * wears his cape; gone when he is, as KatAM's Kracko's lightning (sub_080DE42C); */
static void SwordMain(void) {
    struct Sprite sprite;
    struct ObjectBase *sword = TaskGetStructPtr(gCurTask);
    struct MetaKnight *m = sword->parent;
    struct Object *obj = &m->n.obj;
    const struct Pose *pose;
    bool8 left;

    Macro_08107BA8_4(sword, &sword->sprite, &sprite, SWORD_TILES, &sword->sprite);
    if (sword->roomId != 0xFFFF && (obj->base.flags & 0x1000)) {
        sword->roomId = 0xFFFF;
        sword->flags |= 0x1000;
        return;
    }
    sword->sprite.palId = sword->filler6[0];
    if (ObjectPreUpdate(sword)) return;
    pose = &sPoses[m->pose];
    left = obj->base.flags & 1;
    sword->x = obj->base.x + (left ? -(pose->x + sWayX[pose->way]) : pose->x + sWayX[pose->way]) * 0x100;
    sword->y = obj->base.y + (pose->y + sWayY[pose->way]) * 0x100;
    sword->flags = (sword->flags & ~0x401) | (left ? 1 : 0) | (m->armed ? 0 : 0x400);
    sword->sprite.variant = FRAME_SWORD + pose->way;
    // Unarmed (his cape), harmless;
    if (m->armed) sub_0803E2B0(sword, -8, -8, 8, 8);
    else sub_0803E2B0(sword, 0, 0, 0, 0);
    SetPointerSomething(sword);
    sub_0806F8BC(sword);
}

static void Sword(struct MetaKnight *m) {
    struct Object *obj = &m->n.obj;
    struct Task *t = TaskCreate(SwordMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *sword = TaskGetStructPtr(t);
    u16 key = NidlGraphicsOf(&m->n)->animId | NIDL_SECOND_PALETTE;

    ClearObjectBase(sword);
    sword->header.kind = 2;
    sword->x = obj->base.x;
    sword->y = obj->base.y;
    sword->parent = obj;
    sword->counter = 0;
    sword->roomId = obj->base.roomId;
    sword->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(sword)) sword->flags |= 0x2000;
    sword->unk63 = 2;
    sword->flags |= 0x10000140 | 0x2000000;
    sword->unk68 = 0x20000043;
    sword->unk5C |= 0x80000;
    sword->filler6[0] = NidlLoadPalette(&m->n, key, FRAME_SWORD);   // Its palette (gold), kept in the padding;
    sub_0803E2B0(sword, 0, 0, 0, 0);
    ObjectSetBounds(sword, 0, 0, 0, 0);
    // In front of him (a smaller priority is drawn over: his is 0x6C0 >> 6);
    ObjectBaseInitSprite(sword, &sword->sprite, SWORD_TILES, NidlGraphicsOf(&m->n)->animId, FRAME_SWORD, 0x1A);
    sword->sprite.palId = sword->filler6[0];
}

/* ThrowSword(); NiDL's "GET IT!": KatAM's Sword statue (object 0x95, subtype 0, as the NiDL switch rooms' and KatAM's hub rooms'),
 * which Kirby inhales for Sword, as often as he likes, set down on the floor beside him toward Kirby; */
#define OBJ_SWORD_STATUE 0x95
#define STATUE_X 56
static void ThrowSword(struct MetaKnight *m) {
    struct Object *obj = &m->n.obj;
    s32 x = (obj->base.x >> 8) + ((obj->base.flags & 1) ? -STATUE_X : STATUE_X);

    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, (obj->base.y >> 8) - 2, 0, 0x1F, 0, 0, OBJ_SWORD_STATUE, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0);
    PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
}

static void Set(struct MetaKnight *m, u8 state) {
    m->state = state;
    m->timer = 0;
}

static void Leap(struct MetaKnight *m, u8 state, s16 speed) {
    struct Object *obj = &m->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
    obj->base.yspeed = LEAP_UP;
    m->airborne = TRUE;
    Set(m, state);
    PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
}

// Fall(); In the air until he lands;
static bool32 Fall(struct MetaKnight *m) {
    struct Object *obj = &m->n.obj;

    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
    if (obj->base.yspeed > 0 || !(obj->base.unk62 & 4)) return FALSE;
    obj->base.xspeed = obj->base.yspeed = 0;
    m->airborne = FALSE;
    return TRUE;
}

// Next(); After walking a while: a dash or a leap;
static void Next(struct MetaKnight *m) {
    if (Rand16() & 1) Set(m, STATE_CROUCH);
    else Leap(m, STATE_LEAP, LEAP_SPEED);
}

static void Update(struct MetaKnight *m) {
    struct Object *obj = &m->n.obj;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, x = (obj->base.x >> 8) - ScreenX(obj);
    u16 t;

    switch (m->state) {
    case STATE_WAIT:
        // On his ledge in his cape: a sword for Kirby, the duel once he has a power;
        m->pose = POSE_CAPE;
        NidlFaceKirby(obj);
        // (or once he's hit, or after WAIT_TIME: Kirby's helpers fight him too);
        if (obj->kirby3->ability != KIRBY_ABILITY_NORMAL || obj->unk80 < m->hp || ++m->wait > WAIT_TIME) {
            if (++m->timer > 30) Leap(m, STATE_LEAP_DOWN, LEAP_SPEED);
            break;
        }
        m->timer = 0;
        // Once he stands on the floor (his spot is in the air here: NiDL's ledge isn't), so the statue stands there too;
        if ((obj->base.unk62 & 4) && m->starTimer && !--m->starTimer) ThrowSword(m);
        break;
    case STATE_LEAP_DOWN:
        // He casts off his cape as he leaps (NiDL's 4-9, flapping);
        m->pose = POSE_CAPE;
        NidlSetFrame(&m->n, 4 + ((m->anim >> 2) % 6));
        if (Fall(m)) {
            m->armed = TRUE;
            Set(m, STATE_REST);
        }
        return;
    case STATE_WALK:
        m->pose = POSE_WALK;
        NidlFaceKirby(obj);
        obj->base.xspeed = (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
        if (dx > -CLOSE && dx < CLOSE) {
            obj->base.xspeed = 0;
            Set(m, STATE_SLASH);
        } else if (++m->timer > 60) {
            obj->base.xspeed = 0;
            Next(m);
        }
        break;
    case STATE_SLASH:
        t = ++m->timer;
        m->pose = t < 10 ? POSE_RAISE : t < 14 ? POSE_SWING : t < 26 ? POSE_SLASH : POSE_STAND;
        if (t == 10) PlaySfx(&obj->base, SE_ABILITY_SPARK_ATTACK);
        if (t >= 40) Set(m, STATE_WALK);
        break;
    case STATE_CROUCH:
        m->pose = POSE_CROUCH;
        NidlFaceKirby(obj);
        if (++m->timer >= 16) {
            Set(m, STATE_DASH);
            PlaySfx(&obj->base, SE_KIRBY_DASH);
        }
        break;
    case STATE_DASH:
        m->pose = POSE_DASH;
        obj->base.xspeed = (obj->base.flags & 1) ? -DASH_SPEED : DASH_SPEED;
        if ((obj->base.unk62 & 3) || ++m->timer > 50 || (obj->base.flags & 1 ? x < 24 : x > 216)) {
            obj->base.xspeed = 0;
            Set(m, STATE_REST);
        }
        break;
    case STATE_LEAP:
        m->pose = POSE_LEAP;
        if (Fall(m)) {
            Set(m, STATE_SLASH);
            m->timer = 12;  // The slash as he lands;
        }
        break;
    case STATE_REST:
        m->pose = POSE_STAND;
        if (++m->timer >= 20) Set(m, STATE_WALK);
        break;
    }
    // His walk steps (NiDL's 21 and 22, the ones with his mask: its 24-32 show his face bare, his mask is gone in NiDL's other frames too);
    if (m->pose == POSE_WALK) NidlSetFrame(&m->n, BODY(21) + ((m->anim >> 3) & 1));
    else if (m->pose == POSE_DASH) NidlSetFrame(&m->n, BODY(45) + ((m->anim >> 2) & 3));
    else if (m->pose == POSE_LEAP) NidlSetFrame(&m->n, BODY(33) + ((m->anim >> 3) % 3));
    else NidlSetFrame(&m->n, sPoses[m->pose].body);
}

static void MetaKnightMain(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;

    NidlUpdate(&m->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; he takes it back;
    sub_08034E14(obj);
    m->anim++;
    // Off a ledge, he falls (as the enemies do: NidlFall); his leaps have their own gravity;
    if (!m->airborne) NidlFall(obj);
    Update(m);
}

// MetaKnightStart(); The duel starts: on his ledge, a sword for Kirby first;
void MetaKnightStart(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;

    ObjectSetFunc(obj, -1, MetaKnightMain);
    if (m->started) return;
    m->started = TRUE;
    m->starTimer = 30;
    m->wait = 0;
    m->hp = obj->unk80;
    Set(m, STATE_WAIT);
    Sword(m);
}

// MetaKnightInit(); His boxes (his body, 14 pixels down to his feet), on his ledge, his palette (his body's, a streamed frame's);
void MetaKnightInit(struct NidlObject *n) {
    struct MetaKnight *m = (struct MetaKnight *)n;
    struct Object *obj = &n->obj;

    m->started = FALSE;
    m->airborne = FALSE;
    m->armed = FALSE;
    m->pose = POSE_CAPE;
    m->anim = 0;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -12, -10, 12, 14);
    ObjectSetBounds(&obj->base, -10, -10, 10, 14);
}
