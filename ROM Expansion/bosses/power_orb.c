/* Nightmare's Power Orb; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x42 (spawn 0x080AE3BC), as NiDL boss
 * NIDL_POWER_ORB, the Fountain of Dreams' first (room 0x003 here: the fall through space). As NiDL's, it's fought with the Star Rod
 * (abilities/star_rod.c: every Kirby in the room flies with it, shooting stars), at the top of the room, the screen sinking through
 * space as they fight (0.2 pixels a frame, 881 pixels down to the ground: ArenaSink(), its arena's screen, the cameras on it every
 * frame as NiDL's camera, 0x0300215C, is: 8 + the sink), the orb, its shots and the Kirbys keeping their places on it; not beaten by
 * the time the ground's reached (NiDL's camera past 888), every Kirby in its room loses a life. Its frames (0x087544B4, mini-boss
 * descriptor 37; only those it uses: KatAMExpansion.PowerOrb): NiDL's 4-7, the spinning orb, 8-11 it shrinking (its look as it
 * attacks; 11 blinks as it charges), 12-13 its look when hit, then its shots' (its 20-29 and 39-42, their own tiles), from FRAME_SHOT,
 * then the intro's slash (14-19), then its end's (the dot, the stars it bursts into, the sparkle in their middle), then its shots'
 * afterimages (30-38). They're drawn where NiDL draws them: a pixel up from KatAM's enemies' (KatAMExpansion.cs builds NiDL's frames a
 * pixel lower, to stand as KatAM's), the way NiDL's face (KatAM's flag 1), its shots over it and their trails under it (NiDL's
 * layers: its shots' 9, its 0xB, their sparkles' 0xC, their afterimages' 0xD).
 * Its intro (STATE_ENTER), NiDL's, traced frame by frame from the room's first (the orb's path on the screen frame by frame,
 * sIntroMoves): Kirby comes in at the room's bottom (KatAMExpansionRooms.cs: NiDL's start there; this and its arena stand there too,
 * KatAM making objects only near the camera) and the screen rises from there to the fight's (sRiseSpeeds: 761 pixels, 4 a frame,
 * slowing to 1 as it gets there: ArenaRise(), its arena's lock under its screen, the cameras right on it); 66 frames in, every
 * Kirby flies up into it from below its left (abilities/star_rod.c's StarRodIntro(): NiDL's flight and frames) and a frame later
 * the orb comes in from its top right, spinning, down to the left; Kirby swings at it (a slash across the screen: OrbSlash()), it pulses 5 times
 * and spins fast as it loops round to its spot, and the fight starts there (319 frames in, NiDL's). Meanwhile it's untouchable and
 * harmless and shoots nothing, and the Kirbys neither move nor shoot (the riders fly them).
 * What it does, traced frame by frame in NiDL's fight (its speeds change in steps it keeps exactly), from its spot on the screen's
 * right ((192, 76) on it): its moves (sMoves: NiDL's, its hover and roam two each, 120 frames), twice through, then it flies off over
 * the screen (STATE_ESCAPE: right, rising faster and faster) and the screen sinks on to the ground:
 * • it hovers (STATE_HOVER, 240 frames), bobbing up and down;
 * • it roams (STATE_ROAM, 240 frames), a figure of eight over the screen's right;
 * • it charges (STATE_CHARGE, 120 frames), still and blinking: 40 frames in it fires 4 stars that spread into a column over the
 *   screen's right (their speeds halving every 14 frames) with a gap in its middle, wait, and 88 frames after they came out dash
 *   left at 6 pixels a frame, squashed;
 * • it zips (STATE_ZIP) up and down at 6 pixels a frame to rows above and below where it was, at each lunging to the right and back
 *   as it shoots a crescent left at 6 pixels a frame;
 * • it dashes (STATE_DASH) at the Kirby it's after, high, in the middle or low by where he is, whooshing, then back round to its spot.
 * Hovering and roaming it spins (faster as it weakens: Look()), and after 40 frames shows its attack look 1-4 times by chance, the
 * first with a timed shot if its last was 140 frames ago or more (NiDL's cooldown, 0x080AF144; its charge starts it again), in turn:
 * a stream of 3 stars left at 4 pixels a frame, 16 frames apart; a fan of 3 (left at 3, and 2.2 left 1.7 up or down); a pair (2 left,
 * 2 up or down, for 16 frames, then left at 3). Its stars spin (3, 2, 1, 0, 4 frames each) and its shots leave afterimages and
 * sparkles behind them (OrbTrail()); its colors pulse every 24 frames (Pulse()). Its shots are on the screen, as NiDL's, gone 64
 * pixels off it; they hurt (attack objects, as the enemies' stars); so does touching it.
 * Its hits, NiDL's (traced frame by frame): a Star Rod star hits it once both are 30 pixels or less apart each way (NiDL's: its box
 * +-18, 0x0874B434, the star's +-12, 0x0873BE74), whatever its look, as the star will be after its move that frame; it takes them itself
 * (KatAM's own would shake the screen, freeze it 10 frames and spark: none of that in NiDL's; its box is off, flag 0x200), 1 HP a star of
 * its 60, the star flying on 2 frames more before it bursts (abilities/star_rod.c's StarRodHits()); hit, for 24 frames it shows its
 * look in lines (NiDL's 12, 12, a spin frame, 13, 13, the next spin frame...), moving on as ever, from the start again at each hit.
 * Its last (Die()): it stops as it was, the screen flashes grey and red by turns 2 frames after (KatAM's own flashes, gUnk_08350E34:
 * NiDL's colors and frames), everything frozen but its end (KatAM's freeze, as its bosses' end); 29 frames after, it bursts
 * (PowerOrbEnd(): its shots gone, eight stars out of its middle and a sparkle there), and everything moves again 54 frames after
 * (KatAM's own end would shake the screen twice and burst into stars of its own: not in NiDL's); */
#include "../nidl.h"
#include "../abilities.h"
#include "code_0806F780.h"
#include "sprite.h"
#include "malloc_vram.h"
#include "constants/songs.h"

void ArenaSink(struct Object *, s32);
void ArenaRise(struct Object *, s32);
void sub_08033540(u8);
void sub_080335B4(u8);
void sub_080860A8(struct ObjectBase *, const struct Unk_08357260 *);
void sub_08086C48(struct Object *);
void sub_08088640(struct Kirby *, u8, u16);

#define SPOT_X (192 << 8)
#define SPOT_Y (76 << 8)
#define SINK_SPEED 0x33
#define SINK_END (881 << 8)
#define GROUND_Y 892
#define COOLDOWN 140
#define STREAM_GAP 16
#define LUNGE_TIME 8
#define ZIP_WAIT 8
#define ZIP_SPEED 0x600
#define LOOK_WAIT 40
#define LOOPS 2
#define ESCAPE_TIME 60
#define ESCAPE_SPEED 0x100
#define ESCAPE_RISE 0x20
#define OFF_SCREEN 32
#define DASH_HIGH 63
#define DASH_LOW 128
#define PULSE_TIME 24
#define PULSE_START 22       // (its first frame's: NiDL's, from the orb's first frame);
#define HIT_REACH 30
#define HURT_TIME 24
/* Its last (frames after the hit; its task runs after the others, each frame's last: what it starts shows, or runs, from the frame
 * after): everything's frozen, the flashes start (shown from then), it bursts, everything moves again; */
#define DEATH_FREEZE 1
#define DEATH_FLASH 2
#define DEATH_BURST 29
#define DEATH_THAW 53
#define DEATH_PRIORITY 0x3600
// The intro (frames into it: the orb and the Kirbys show; it moves; Kirby's swing hits it; its pulses; its fast spin; the fight);
#define INTRO_SHOW 67
#define INTRO_MOVE 95
#define INTRO_CLASH 203
#define INTRO_PULSE 205
#define INTRO_FAST 243
#define INTRO_END 319
#define INTRO_X (256 << 8)
#define INTRO_Y (-4 << 8)
#define INTRO_BELOW 761
// Its frames (KatAMExpansion.PowerOrb): the spin, the shrink, its look when hit, then the shots' (NiDL's shot frames 0-9, then 19-22,
// from FRAME_SHOT);
#define FRAME_SPIN 0
#define FRAME_SHRINK 4
#define FRAME_HURT 8
#define FRAME_SHOT 10
#define SHOT_STAR 0         // 0-3, spinning (shown 3, 2, 1, 0);
#define SHOT_DASH 4         // 4-7, squashed as it dashes (7, 6, 5, 4, 2 frames each);
#define SHOT_CRESCENT 8     // 8-9, in turn;
#define SHOT_SPARKLE 10     // 10-13 (NiDL's 19-22), the sparkles a dashing star trails;
#define SHOT_TILES 8         // (its biggest frame, a crescent: 16x32);
// The intro's slash (NiDL's 14-19, from FRAME_SLASH; its second palette): where it is on the fight's screen (NiDL's (144, 118) in
// the room, the screen still rising past it);
#define FRAME_SLASH (FRAME_SHOT + 14)
#define SLASH_TILES 16
#define SLASH_X 136
#define SLASH_Y 110
// Its end's (PowerOrbEnd()): the dot (the orb, and the star with its Kirbys, far off in the moon; NiDL's palette 15), the stars it
// bursts into (0-8) and the sparkle in their middle (0-22; those two NiDL's shared palette 15);
#define FRAME_DOT (FRAME_SLASH + 6)
#define FRAME_BURST (FRAME_DOT + 1)
#define FRAME_SPARKLE (FRAME_BURST + 9)
#define FRAME_TRAIL (FRAME_SPARKLE + 23)   // (its shots' afterimages, their tiles and palette: NiDL's 30-38, OrbTrail());
#define BURST_TILES 8
#define SPARKLE_TILES 48

enum { STATE_ENTER, STATE_HOVER, STATE_ROAM, STATE_CHARGE, STATE_ZIP, STATE_DASH, STATE_ESCAPE, STATE_GONE };
enum { LOOK_SPIN, LOOK_START, LOOK_ATTACK, LOOK_SCRIPTED };
enum { ATTACK_STREAM, ATTACK_FAN, ATTACK_PAIR, ATTACKS };
enum { KIND_STAR, KIND_PAIR, KIND_CHARGE, KIND_CRESCENT };

struct PowerOrb {
    struct NidlObject n;
    s32 sx, sy;         // Its spot on the fight's screen (8.8, from the screen's top left as it sinks);
    s32 sink;           // How far the screen has sunk (8.8);
    s32 below;          // How far under its screen the intro's screen is (pixels);
    u16 timer;          // Frames into its state;
    u16 cooldown;       // Frames since its last timed shot;
    u8 state;
    u8 step, stepTimer; // The step of its moves (a table's) and frames left of it;
    u8 attack;          // Its next timed shot;
    u8 stream, streamTimer;
    u8 move, loop;      // Where it is in its moves (sMoves) and how many times it's been through them;
    u8 row;             // ZIP: the row it's going to, and that row's phase; DASH: its way, and 0 out, 1 back;
    u8 phase;
    s16 vy;             // ESCAPE: its y speed (8.8);
    s32 zipFrom;        // ZIP: where it started (its rows are from there);
    s16 fromX, fromY;   // Its pixel as the frame began;
    s32 lag;            // How far the screen had sunk before this frame (its drawing's, and its shots');
    const u8 *seq;      // Its look (NiDL's frame sequences: SeqSet()), where it is in it, frames left of that frame, and the frame;
    u8 seqAt, seqLeft, frame;
    u8 lookState;       // Its looks between its moves' (LOOK_SPIN... : NiDL's +0x15) and frames into its spin before it shoots;
    u8 lookCount;
    s8 repeats;         // Its attack look's turns left;
    u8 pulse;           // Frames into its colors' pulse (Pulse());
    u8 hurt;            // Frames left of its look when hit;
    bool8 dying;        // Beaten: its end under way (Die());
    u8 slots;           // NiDL's objects its shots and their trails take, and those of them gone this frame (OrbTrail());
    u8 slotsGone;
};

typedef char sPowerOrbFits[sizeof(struct PowerOrb) <= NIDL_OBJECT_SIZE ? 1 : -1];

struct OrbShot {
    struct ObjectBase base;
    s32 sx, sy;         // Its spot on the fight's screen (16.16, as the orb's: NiDL's shots are on the screen too);
    s32 vx, vy;         // Its speed (16.16, y down);
    u8 kind;
    u8 palId;
    u8 side;            // A charge star's: its place in the column (0 outer, 1 inner) and way (y speed's sign);
    bool8 up;
    struct OrbTrails *trails;
};

// The hover: y speeds (8.8, down) and frames, from the middle up and round twice, back to the middle;
static const s16 sHoverSpeeds[] = { -0x100, -0x80, -0x20, 0x20, 0x80, 0x100, 0x80, 0x20, -0x20, -0x80, -0x100, -0x80, -0x20, 0x20, 0x80, 0x100, 0x80,
                                    0x20, -0x20, -0x80, -0x100 };
static const u8 sHoverTimes[] = { 10, 10, 10, 10, 10, 20, 10, 10, 10, 10, 20, 10, 10, 10, 10, 20, 10, 10, 10, 10, 10 };
/* The roam: speeds (8.8, y down) of a figure of eight, 6 frames each but its crossings (12), from the middle going up left (half a
 * crossing first); round twice, back to the middle; */
static const s16 sRoamX[] = { -0x200, -0x100, 0, 0x100, 0x200, 0x200, 0x100, 0, -0x100, -0x200, -0x100, 0, 0x100, 0x200, 0x200, 0x100, 0, -0x100, -0x200 };
static const s16 sRoamY[] = { -0x200, -0x200, -0x140, -0x100, -0x80, 0x80, 0x100, 0x140, 0x200, 0x200, 0x200, 0x140, 0x100, 0x80, -0x80, -0x100, -0x140, -0x200, -0x200 };
static const u8 sRoamTimes[] = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 12, 6, 6, 6, 6, 6, 6, 6, 6, 12 };
#define ROAM_LOOP 1
// Its moves, NiDL's (0x080AE3BC: its hover and roam two of its moves each, 120 frames), through them LOOPS times, then off;
static const u8 sMoves[] = { STATE_HOVER, STATE_ROAM, STATE_CHARGE, STATE_ROAM, STATE_ZIP, STATE_HOVER, STATE_CHARGE, STATE_ROAM, STATE_DASH };
#define HALF_MOVE 120
/* The dash (NiDL's 0x080AE8E0): speeds (8.8, y down) and frames, its way by the y of the Kirby it's after (from the screen's top:
 * over DASH_HIGH, high; over DASH_LOW, low; else the middle), whooshing at its DASH_WHOOSH step; then back round to its spot, the
 * high and middle ways (0x080AEC00) over, the low (0x080AED5C) under, still for 32 frames at the end; */
struct OrbStep {
    s16 vx, vy;
    u8 time;
};
static const struct OrbStep sDashMiddle[] = { { -0x300, 0, 8 }, { 0x300, 0, 8 }, { 0x200, 0, 8 }, { 0x100, 0, 8 }, { 0x80, 0, 8 }, { 0x20, 0, 8 },
                                              { 0, 0, 4 }, { -0x580, 0, 32 }, { -0x200, 0, 8 }, { -0x100, 0, 8 }, { -0x80, 0, 8 }, { 0, 0, 32 } };
static const struct OrbStep sDashHigh[] = { { 0x200, 0x200, 8 }, { 0x100, 0x100, 8 }, { 0x80, 0x80, 8 }, { 0x20, 0x20, 8 }, { 0, 0, 16 },
                                            { -0x80, -0x400, 8 }, { -0x100, -0x300, 8 }, { -0x200, -0x200, 8 }, { -0x300, -0x100, 8 },
                                            { -0x400, -0x80, 8 }, { -0x600, -0x20, 12 }, { -0x300, 0x100, 8 }, { -0x200, 0x200, 8 }, { -0x100, 0x400, 8 } };
static const struct OrbStep sDashLow[] = { { 0x200, -0x200, 8 }, { 0x100, -0x100, 8 }, { 0x80, -0x80, 8 }, { 0x20, -0x20, 8 }, { 0, 0, 16 },
                                           { -0x80, 0x400, 8 }, { -0x100, 0x300, 8 }, { -0x200, 0x200, 8 }, { -0x300, 0x100, 8 },
                                           { -0x400, 0x80, 8 }, { -0x600, 0x20, 12 }, { -0x300, -0x100, 8 }, { -0x200, -0x200, 8 }, { -0x100, -0x400, 8 } };
static const struct OrbStep sDashBack[] = { { 0x200, 0x200, 8 }, { 0x200, 0x100, 8 }, { 0x200, -0x100, 8 }, { 0x200, -0x200, 8 }, { 0x200, -0x200, 8 },
                                            { 0x200, -0x100, 8 }, { 0x200, 0x100, 8 }, { 0x200, 0x200, 8 }, { 0x200, 0x200, 8 }, { 0x200, 0x100, 8 },
                                            { 0x400, 0, 6 }, { 0x200, -0x80, 6 }, { 0x100, -0x100, 6 }, { 0x80, -0x200, 6 }, { -0x100, -0x200, 6 },
                                            { -0x200, -0x100, 6 }, { -0x100, 0x80, 6 }, { -0x100, 0x100, 6 }, { 0, 0x100, 6 }, { 0, 0, 32 } };
enum { DASH_MIDDLE, DASH_UP, DASH_DOWN };
static const struct OrbStep *const sDashWays[3] = { sDashMiddle, sDashHigh, sDashLow };
static const u8 sDashSteps[3] = { ARRAY_COUNT(sDashMiddle), ARRAY_COUNT(sDashHigh), ARRAY_COUNT(sDashLow) };
static const u8 sDashWhoosh[3] = { 7, 5, 5 };
/* Its looks (NiDL's frame sequences, 0x0874AD74...: its frames (FRAME_SPIN...) and how long each shows), with: SEQ_LOOP, back to the
 * start; SEQ_TURN, an attack look's turn done (0x080AF178); SEQ_HP, as long as its HP / 8 + 1 (0x080AF100: it spins faster as it
 * weakens); SEQ_DASH, on to sDashSpin, from its second frame (NiDL's dash look ends calling into the middle of its code, which sets
 * 0x0874B07C going); */
#define SEQ_LOOP 0xFF
#define SEQ_TURN 0xFE
#define SEQ_HP 0xFD
#define SEQ_DASH 0xFC
#define PULSE FRAME_SHRINK, 1, FRAME_SHRINK + 1, 1, FRAME_SHRINK + 3, 1, FRAME_SHRINK + 2, 1, FRAME_SHRINK + 1, 1, FRAME_SHRINK, 1
static const u8 sSpinSeq[] = { 0, SEQ_HP, 1, SEQ_HP, 2, SEQ_HP, 3, SEQ_HP, SEQ_LOOP };
static const u8 sAttackSeq[] = { 0, 2, PULSE, SEQ_TURN, 1, 2, PULSE, SEQ_TURN, 2, 2, PULSE, SEQ_TURN, 3, 2, PULSE, SEQ_TURN, SEQ_LOOP };
static const u8 sPulseSeq[] = { 0, 2, PULSE, 1, 2, PULSE, 2, 2, PULSE, 3, 2, PULSE, SEQ_LOOP };
static const u8 sDashSeq[] = { 0, 2, PULSE, 1, 2, PULSE, 2, 2, PULSE, 3, 2, PULSE, SEQ_DASH };
static const u8 sDashSpin[] = { 0, 1, 2, 1, 1, 1, 3, 1, SEQ_LOOP };
static const u8 sDashBackSeq[] = { 0, 4, 1, 4, 2, 4, 3, 4, SEQ_LOOP };
/* Its colors: its palette (NiDL's 0x082FE0E4) blended into a darker one (0x082FE104) and back, by these 256ths, 3 frames each, all
 * through its fight (0x080AEF5C); KatAMExpansion.cs fills them in; */
const u16 gNidlPowerOrbColors[2][16] = { { 0 } };
static const u16 sPulse256[PULSE_TIME / 3] = { 0, 64, 128, 192, 256, 192, 128, 64 };
// The charge stars: their speeds (8.8) step by step, outer and inner (y up for the upper two), 14 frames each, then still;
static const s32 sChargeX[2] = { 0x8000, 0x2000 };
static const s32 sChargeY[2][4] = { { 0x20000, 0x10000, 0x8000, 0x2000 }, { 0x10000, 0x8000, 0x2000, 0x800 } };
#define CHARGE_STEP 14
#define CHARGE_DASH 88
#define CHARGE_FIRE 41
// The zip's rows (pixels down from its spot: NiDL moves it by as much, 0x080AE7EC);
static const s8 sZipRows[] = { -60, 36, -36, 60, 0 };
// The lunge: x speeds (8.8) frame by frame;
static const s16 sLunge[LUNGE_TIME] = { 0x800, 0x800, 0x400, 0x400, -0x400, -0x400, -0x800, -0x800 };
// The intro's screen rising: pixels a frame and frames;
static const u8 sRiseSpeeds[] = { 0, 8, 4, 3, 2, 1 };
static const u8 sRiseTimes[] = { 2, 1, 141, 32, 32, 29 };
/* The intro's moves on the screen frame by frame from INTRO_MOVE (x + 8, (y + 8) << 4, pixels: NiDL's spot less its camera, traced
 * frame by frame, the camera's rise in it), from (256, -4) on the screen (above its right) down to the left, past Kirby, its swing
 * knocking it back up to the right, then round to the left and back to its spot (192, 75); */
static const u8 sIntroMoves[] = {
    0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97,
    0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97,
    0x97, 0x97, 0x87, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97,
    0x87, 0x97, 0x87, 0x97, 0x87, 0x97, 0x87, 0x97, 0x87, 0x97, 0x77, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x97, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87,
    0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x6C, 0x7C, 0x7A, 0x7A, 0x7A, 0x7A,
    0x7A, 0x7A, 0x7A, 0x7A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x79, 0x79, 0x79, 0x79, 0x79, 0x79, 0x78, 0x89, 0x78, 0x89, 0x86, 0x86,
    0x86, 0x96, 0x86, 0x96, 0x94, 0x84, 0x94, 0x94, 0x94, 0x84, 0xA4, 0x94, 0xA4, 0x94, 0xA4, 0x94, 0xA4, 0x94, 0xA4, 0x94, 0xA4, 0x97, 0xA7, 0x97,
    0xA7, 0x97, 0xA7, 0x99, 0xA9, 0x99, 0xA9, 0x99, 0xA9, 0x9A, 0xAA, 0x9A, 0xAA, 0x9A, 0xAA, 0x9C, 0x8C, 0x9C, 0x9C, 0x9C, 0x8C, 0x8A, 0x8A, 0x8A,
    0x7A, 0x8A, 0x8A, 0x78, 0x79, 0x88, 0x79, 0x78, 0x79, 0x88, 0x78, 0x78, 0x79, 0x88, 0x78,
};
// Its pulses (8 frames apart): the shrink's frames, 6 frames; the slash's frames, frame by frame;
static const u8 sPulse[6] = { 0, 1, 3, 2, 1, 0 };
static const u8 sSlash[] = { 0, 0, 1, 2, 3, 3, 4, 4, 5, 5 };

static s32 ScreenLeft(struct Object *obj) {
    return (obj->objTemplate->x + obj->objTemplate->unk1A) << 8;
}

static s32 ScreenTop(struct PowerOrb *o) {
    struct Object *obj = &o->n.obj;
    return ((obj->objTemplate->y + obj->objTemplate->unk1C) << 8) + o->sink;
}

static u16 ShotKey(struct PowerOrb *o) {
    return NidlGraphicsOf(&o->n)->animId | NIDL_SECOND_PALETTE;
}

// StarSpin(); A star's spin's frame (from FRAME_SHOT + SHOT_STAR) t frames on: 3 down to 0, 4 frames each;
static u8 StarSpin(u16 t) {
    return 3 - ((t >> 2) & 3);
}

/* OrbTrail(); What a shot leaves behind (NiDL's objects 0xD2-0xD5, 0x080AFDF0...), 16 frames, from its pixel as its frame begins: its
 * afterimage (TRAIL_AFTER, 0xD5) or a sparkle (TRAIL_SPARKLE, 0xD2: off it by (dx, dy)), going as the shot does but slower by its way's
 * 16ths (its x speed's share of both speeds, and its y's: x to the right, y toward still), once for the afterimage, twice for the
 * sparkle; a dashing star's other sparkles (TRAIL_UP, TRAIL_DOWN: 0xD3, 0xD4; it's flat) a pixel a frame slower again, and half a pixel
 * a frame up or down. The afterimage's frames: a star's spin's, on from the star's (NiDL's 30-33, 4 frames each), a dashing star's (37
 * down to 34, 2 each) or a crescent's (38); the sparkles' 39-42, 2 each, round. None if NiDL's objects would be full (SLOTS: the orb,
 * its shots and their trails; the ones going this frame still there as the shots make theirs: PowerOrbMain() frees them the frame
 * after); on the screen as the shots are; */
enum { TRAIL_AFTER, TRAIL_SPARKLE, TRAIL_UP, TRAIL_DOWN };
#define TRAIL_LIFE 16
#define SLOTS 31
/* Their frames' sprites, each frame's tiles loaded once for them all (OrbTrails: the afterimages' 9, FRAME_TRAIL on, then the sparkles'
 * 4), and each frame's one OAM piece, made once as DisplaySprite() would, a trail adding its spot to it (OrbTrailDraw()): KatAM drawing
 * the dashing stars' dozens each from its own sprite, its tiles copied as its frame changes, lags; */
#define TRAIL_SPARKLES 9
#define TRAIL_SPRITES (TRAIL_SPARKLES + 4)

struct TrailOam {
    u8 layer;           // (under the orb, as NiDL's: its sparkles', then its afterimages': Shoot());
    u16 attr0, attr1, attr2;
    u16 x, y;           // (the piece's spot in the frame);
    s16 left, top;      // (the frame's, from its spot; off the screen by its size, not drawn);
    u16 width, height;
    bool8 one;          // (one piece, not flipped upside down: else DisplaySprite());
};
// (an OAM piece's width by its shape and size, KatAM's gUnk_08D6084C);
static const u8 sPieceWidth[12] = { 8, 16, 32, 64, 16, 32, 32, 64, 8, 8, 16, 32 };
// A star's sparkle off it (pixels), by its way (flat, up, down) and in turn;
static const s8 sSparkleDx[3][2] = { { 0, 0 }, { 1, -1 }, { -1, 1 } };
static const s8 sSparkleDy[3][2] = { { 2, -2 }, { -1, 1 }, { -1, 1 } };

struct OrbTrails {
    struct PowerOrb *o;
    struct Sprite sprites[TRAIL_SPRITES];
    struct TrailOam oams[TRAIL_SPRITES];
    bool8 gone;
};

struct OrbTrail {
    struct EffectObject e;
    s32 sx, sy;         // (16.16, as a shot's);
    s32 vx, vy;
    u8 kind;
    u8 shot;            // Its shot's kind, and a star's spin's frame as it was left;
    u8 spin;
    u8 palId;
    struct OrbTrails *trails;
};

static void OrbTrailsMain(void) {
    struct OrbTrails *ts = TaskGetStructPtr(gCurTask);

    if (ts->gone || ts->o->n.obj.base.flags & 0x1000) TaskDestroy(gCurTask);
}

static void OrbTrailsDestroy(struct Task *t) {
    struct OrbTrails *ts = TaskGetStructPtr(t);
    u8 i;

    for (i = 0; i < TRAIL_SPRITES; i++)
        if (ts->sprites[i].tilesVram) VramFree(ts->sprites[i].tilesVram);
}

// OrbTrails(); Its trails' sprites (the orb's first shot makes them);
static struct OrbTrails *OrbTrails(struct PowerOrb *o) {
    struct Task *t = &gTasks[0];
    struct OrbTrails *ts;
    u8 i;

    for (i = 0; i < MAX_TASK_NUM && t->next; i++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == OrbTrailsMain && ((struct OrbTrails *)TaskGetStructPtr(t))->o == o) return TaskGetStructPtr(t);
    }
    t = TaskCreate(OrbTrailsMain, sizeof(struct OrbTrails), 0x3500, TASK_USE_EWRAM, OrbTrailsDestroy);
    ts = TaskGetStructPtr(t);
    ts->o = o;
    ts->gone = FALSE;
    for (i = 0; i < TRAIL_SPRITES; i++) {
        struct Sprite *sprite = &ts->sprites[i];

        struct TrailOam *oam = &ts->oams[i];
        const struct SpriteAttributes_Sub *sub;
        const u16 *piece;

        // (as EffectObjectInitSprite() would, still: its frame's tiles once);
        oam->layer = (o->n.obj.base.sprite.unk14 >> 6) + (i < TRAIL_SPARKLES ? 2 : 1);
        sprite->unk20[0].unk0 = -1;
        SpriteInit(sprite, VramMalloc(i < TRAIL_SPARKLES ? SHOT_TILES : 4), oam->layer * 0x40, NidlGraphicsOf(&o->n)->animId,
                   i < TRAIL_SPARKLES ? FRAME_TRAIL + i : FRAME_SHOT + SHOT_SPARKLE + i - TRAIL_SPARKLES, 0, 0xFF, 0, 0xF, 0, 0, 0x40000 | 0x2000 | 0x400);
        // (DisplaySprite()'s piece, mirrored (0x400: as NiDL's, see OrbBurstMain()) unless the frame is, without its palette and spot);
        sub = !(sprite->unk4 >> 28) ? &gSpriteTables->attrs[sprite->animId].sub[sprite->unk4] : &gSpriteTables->attrs[sprite->animId].full[sprite->unk4].sub;
        piece = &gSpriteTables->oamData[sprite->animId][3 * (sub->bitfield & 0x3FFF)];
        oam->one = sub->numSubframes == 1 && !(sub->bitfield & 0x8000);
        oam->attr0 = (piece[0] & 0xFE00) | (sprite->unk8 & 0x180) * 8;
        oam->attr1 = piece[1] & 0xFE00;
        oam->x = piece[1] & 0x1FF;
        if (!(sub->bitfield & 0x4000)) {
            oam->attr1 ^= 0x1000;
            oam->x = sub->width - sPieceWidth[(piece[0] & 0xC000) >> 12 | (piece[1] & 0xC000) >> 14] - oam->x;
        }
        oam->attr2 = piece[2] | (sprite->unk8 & 0x3000) >> 2;
        if (oam->attr0 & 0x2000) oam->attr2 += oam->attr2 & 0x3FF;
        oam->attr2 += (sprite->tilesVram - (u32)OBJ_VRAM0) >> 5;
        oam->y = piece[0] & 0xFF;
        oam->left = sub->width - sub->offsetX;
        oam->top = sub->offsetY;
        oam->width = sub->width;
        oam->height = sub->height;
    }
    return ts;
}

// OrbTrailDraw(); Its frame this frame (see above), from its sprites';
static void OrbTrailDraw(struct OrbTrail *tr) {
    struct EffectObject *e = &tr->e;
    struct Sprite sprite;
    const struct TrailOam *piece;
    OamData *oam;
    s32 x, y;
    u8 i = e->unk4 - 1;

    if (e->flags & 0x400 || gKirbys[gLocalPlayerId].base.roomId != e->roomId) return;
    if (tr->kind != TRAIL_AFTER) i = TRAIL_SPARKLES + ((i >> 1) & 3);
    else if (tr->shot == KIND_CRESCENT) i = 8;
    else if (tr->shot == KIND_CHARGE) i = 4 + ((3 - (i >> 1)) & 3);
    else i = (tr->spin - (i >> 2)) & 3;
    // (its own sprite unused: its frame kept there as it's drawn);
    e->sprite.variant = tr->trails->sprites[i].variant;
    // (a pixel up, as the orb: PowerOrbMain());
    x = (e->x >> 8) - (gCurLevelInfo[gLocalPlayerId].viewportPosition.x >> 8) + gUnk_0203AD18[0];
    y = (e->y >> 8) - (gCurLevelInfo[gLocalPlayerId].viewportPosition.y >> 8) + gUnk_0203AD18[1] - 1;
    piece = &tr->trails->oams[i];
    if (!piece->one) {
        sprite = tr->trails->sprites[i];
        sprite.x = x;
        sprite.y = y;
        sprite.palId = tr->palId;
        DisplaySprite(&sprite);
        return;
    }
    x -= piece->left;
    y -= piece->top;
    if (piece->width && (x + piece->width < 0 || x > DISPLAY_WIDTH || y + piece->height < 0 || y > DISPLAY_HEIGHT)) return;
    if ((oam = sub_08156D84(piece->layer)) == iwram_end) return;
    oam->all.attr0 = piece->attr0 | ((y + piece->y) & 0xFF);
    oam->all.attr1 = piece->attr1 | ((x + piece->x) & 0x1FF);
    oam->all.attr2 = piece->attr2 | tr->palId << 12;
}

static void OrbTrailMain(void) {
    struct OrbTrail *tr = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &tr->e;
    struct PowerOrb *o = e->parent;
    u8 i;

    if (e->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    e->flags |= 4;
    // (still while everything's frozen at the orb's end, gone as it bursts: OrbShotsGone());
    if (o->dying) {
        OrbTrailDraw(tr);
        return;
    }
    if (++e->unk4 > TRAIL_LIFE) {
        o->slotsGone++;
        e->flags |= 0x1000;
        return;
    }
    tr->sx += tr->vx;
    tr->sy += tr->vy;
    e->x = ScreenLeft(&o->n.obj) + ((tr->sx >> 16) << 8);
    e->y = gCurLevelInfo[gLocalPlayerId].viewportPosition.y + (((tr->sy >> 16) + ((e->unk4 > 1 ? o->lag : o->sink) >> 8) - (o->sink >> 8)) << 8);
    OrbTrailDraw(tr);
}

static void OrbTrail(struct OrbShot *shot, u8 kind, s8 dx, s8 dy) {
    struct PowerOrb *o = shot->base.parent;
    s32 ax = shot->vx < 0 ? -shot->vx : shot->vx, ay = shot->vy < 0 ? -shot->vy : shot->vy;
    s32 ex = ((ax << 4) / (ax + ay)) << 12, ey = ((ay << 4) / (ax + ay)) << 12;
    struct Task *t;
    struct OrbTrail *tr;
    struct EffectObject *e;

    if (1 + o->slots >= SLOTS) return;
    o->slots++;
    t = TaskCreate(OrbTrailMain, sizeof(struct OrbTrail), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    tr = TaskGetStructPtr(t);
    e = &tr->e;
    ClearEffectObject(e);
    tr->sx = ((shot->sx >> 16) + dx) << 16;
    tr->sy = ((shot->sy >> 16) + dy) << 16;
    if (kind != TRAIL_AFTER) {
        ex <<= 1;
        ey <<= 1;
    }
    tr->vx = shot->vx + ex;
    tr->vy = shot->vy >= 0 ? shot->vy - ey : shot->vy + ey;
    if (kind == TRAIL_UP || kind == TRAIL_DOWN) {
        tr->vx -= 0x10000;
        tr->vy = kind == TRAIL_UP ? -0x8000 : 0x8000;
    }
    tr->kind = kind;
    tr->shot = shot->kind;
    // (the star's frame this frame: OrbShotMain()'s counter is on already);
    tr->spin = StarSpin(shot->base.counter - 1);
    e->header.kind = 3;
    e->x = shot->base.x;
    e->y = shot->base.y;
    e->parent = o;
    e->roomId = shot->base.roomId;
    e->unk4 = 0;
    tr->palId = shot->palId;
    tr->trails = shot->trails;
}

// OrbSlash(); The slash Kirby's swing makes in the intro (NiDL's object 0xDC): growing across the screen from its spot, 10 frames;
static void OrbSlashMain(void) {
    struct EffectObject *slash = TaskGetStructPtr(gCurTask);

    if (slash->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (slash->unk4 >= ARRAY_COUNT(sSlash)) {
        slash->flags |= 0x1000;
        return;
    }
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    slash->flags |= 4 | 1;
    slash->sprite.variant = FRAME_SLASH + sSlash[slash->unk4++];
    EffectObjectPostUpdate(slash);
}

static void OrbSlash(struct PowerOrb *o) {
    struct Task *t = TaskCreate(OrbSlashMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *slash = TaskGetStructPtr(t);

    ClearEffectObject(slash);
    slash->header.kind = 3;
    slash->x = ScreenLeft(&o->n.obj) + (SLASH_X << 8);
    slash->y = ScreenTop(o) + (SLASH_Y << 8);
    slash->parent = o;
    slash->roomId = o->n.obj.base.roomId;
    slash->unk4 = 0;
    EffectObjectInitSprite(slash, &slash->sprite, SLASH_TILES, NidlGraphicsOf(&o->n)->animId, FRAME_SLASH, 0x1A);
    slash->sprite.palId = NidlLoadPalette(&o->n, NidlGraphicsOf(&o->n)->animId | NIDL_THIRD_PALETTE, FRAME_SLASH);
}

/* OrbShotMain(); A star or crescent: an attack object like the enemies' stars (as Mr. Shine's crescents), moving with the screen as
 * it sinks; gone off it, or as the orb bursts (OrbShotsGone(): frozen till then); */
static void OrbShotMain(void) {
    struct Sprite sprite;
    struct OrbShot *shot = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &shot->base;
    struct PowerOrb *o = base->parent;
    struct LevelInfo *level = &gCurLevelInfo[base->unk56];
    s32 sx, sy;
    u16 t;
    u8 i;

    if (ObjectPreUpdate(base)) return;
    base->flags |= 4;
    if (o->n.obj.base.flags & 0x1000) {
        base->flags |= 0x1000;
        return;
    }
    SetPointerSomething(base);
    t = base->counter++;
    switch (shot->kind) {
    case KIND_PAIR:
        // (2 left, 2 up or down, then flat at 3);
        if (t == 16) {
            shot->vx = -0x30000;
            shot->vy = 0;
        }
        break;
    case KIND_CHARGE:
        if (t < CHARGE_STEP * 4) {
            u8 step = t >= CHARGE_STEP * 3 ? 3 : t >= CHARGE_STEP * 2 ? 2 : t >= CHARGE_STEP ? 1 : 0;
            shot->vx = sChargeX[shot->side];
            shot->vy = shot->up ? -sChargeY[shot->side][step] : sChargeY[shot->side][step];
        } else if (t < CHARGE_DASH) {
            shot->vx = shot->vy = 0;
        } else {
            shot->vx = -0x60000;
            shot->vy = 0;
        }
        break;
    }
    /* Its trail (OrbTrail(), NiDL's 0x080AF938...): a star's afterimage and a sparkle every 8 frames, from its first (the pair's from
     * its turn), the sparkle 2 pixels under it then over it (a fan's slanting stars', a pixel each way across them); a crescent's
     * afterimage every 16; a dashing star's afterimage and a sparkle going up every 8, one going down 4 frames on, one going on 2 more; */
    switch (shot->kind) {
    case KIND_PAIR:
        if (t < 16) break;
        // (on as a star's);
    case KIND_STAR:
        if (!(t & 7)) {
            u8 way = shot->vy < 0 ? 1 : shot->vy > 0 ? 2 : 0, turn = (t >> 3) & 1;

            OrbTrail(shot, TRAIL_AFTER, 0, 0);
            OrbTrail(shot, TRAIL_SPARKLE, sSparkleDx[way][turn], sSparkleDy[way][turn]);
        }
        break;
    case KIND_CRESCENT:
        if (!(t & 15)) OrbTrail(shot, TRAIL_AFTER, 0, 0);
        break;
    case KIND_CHARGE:
        if (t < CHARGE_DASH) break;
        switch ((t - CHARGE_DASH) & 7) {
        case 0:
            OrbTrail(shot, TRAIL_AFTER, 0, 0);
            OrbTrail(shot, TRAIL_UP, 0, 0);
            break;
        case 4:
            OrbTrail(shot, TRAIL_DOWN, 0, 0);
            break;
        case 6:
            OrbTrail(shot, TRAIL_SPARKLE, 0, 0);
            break;
        }
        break;
    }
    // (moving on the screen, drawn as the orb is: its pixel, the screen's sink a frame late, but its first frame's);
    shot->sx += shot->vx;
    shot->sy += shot->vy;
    base->x = ScreenLeft(&o->n.obj) + ((shot->sx >> 16) << 8);
    base->y = level->viewportPosition.y + (((shot->sy >> 16) + ((t ? o->lag : o->sink) >> 8) - (o->sink >> 8)) << 8);
    // (it hits the flying Kirbys itself: KatAM's damage is kept off them, abilities/star_rod.c);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == base->roomId
            && StarRodTouches(&gKirbys[i], base->x, base->y, 6, shot->kind == KIND_CRESCENT ? 12 : 6)) StarRodHit(&gKirbys[i], base->x);
    // (gone 64 pixels off the screen where it's drawn, NiDL's: 0x0806523C);
    sx = shot->sx >> 16;
    sy = (base->y - level->viewportPosition.y) >> 8;
    if (sx <= -64 || sx >= DISPLAY_WIDTH + 64 || sy <= -64 || sy >= DISPLAY_HEIGHT + 64) {
        o->slotsGone++;
        base->flags |= 0x1000;
        return;
    }
    if (shot->kind == KIND_CRESCENT) base->sprite.variant = FRAME_SHOT + SHOT_CRESCENT + (t & 1);
    else if (shot->kind == KIND_CHARGE && t >= CHARGE_DASH) base->sprite.variant = FRAME_SHOT + SHOT_DASH + 3 - (((t - CHARGE_DASH) >> 1) & 3);
    else base->sprite.variant = FRAME_SHOT + SHOT_STAR + StarSpin(t);
    // (a pixel up, as the orb: PowerOrbMain());
    base->objBase55 = -1;
    Macro_08107BA8_4(base, &base->sprite, &sprite, SHOT_TILES, &base->sprite);
    base->sprite.palId = shot->palId;
    sub_0806F8BC(base);
}

/* Shoot(); A shot from (x, y) on the fight's screen (8.8; NiDL's from the orb's pixel: where it is after its move that frame for its
 * timed shots, before it for its stream's later stars and its crescents), moving that frame already (vx, vy: 16.16); */
static struct OrbShot *Shoot(struct PowerOrb *o, u8 kind, s32 x, s32 y, s32 vx, s32 vy) {
    struct Object *obj = &o->n.obj;
    struct Task *t = TaskCreate(OrbShotMain, sizeof(struct OrbShot), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct OrbShot *shot = TaskGetStructPtr(t);
    struct ObjectBase *base = &shot->base;

    o->slots++;
    ClearObjectBase(base);
    base->header.kind = 2;
    shot->sx = (x >> 8) << 16;
    shot->sy = (y >> 8) << 16;
    base->x = obj->base.x;
    base->y = obj->base.y;
    base->parent = o;
    base->counter = 0;
    base->roomId = obj->base.roomId;
    base->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(base)) base->flags |= 0x2000;
    base->unk63 = 1;
    // (gone 64 pixels off the screen, NiDL's, not KatAM's 48: 0x2000000, OrbShotMain(); facing right: drawn as NiDL's, OrbBurstMain());
    base->flags |= 0x10000140 | 0x2000000 | 1;
    base->unk68 = 0x20000043;
    base->unk5C |= 0x80000;
    shot->vx = vx;
    shot->vy = vy;
    shot->kind = kind;
    shot->side = 0;
    shot->up = FALSE;
    shot->palId = NidlLoadPalette(&o->n, ShotKey(o), FRAME_SHOT);
    shot->trails = OrbTrails(o);
    if (kind == KIND_CRESCENT) sub_0803E2B0(base, -6, -12, 6, 12);
    else sub_0803E2B0(base, -6, -6, 6, 6);
    ObjectSetBounds(base, 0, 0, 0, 0);
    // (over the orb, as NiDL's: its shots' layer 9, the orb's 0xB, their trails' 0xC and 0xD; KatAM's first layers on top);
    ObjectBaseInitSprite(base, &base->sprite, SHOT_TILES, NidlGraphicsOf(&o->n)->animId, FRAME_SHOT + (kind == KIND_CRESCENT ? SHOT_CRESCENT : SHOT_STAR + 3),
                         (obj->base.sprite.unk14 >> 6) - 1);
    base->sprite.palId = shot->palId;
    return shot;
}

// TimedShot(); A timed shot, if the last was COOLDOWN frames ago or more: the next of the stream, the fan and the pair;
static void TimedShot(struct PowerOrb *o) {
    if (o->cooldown < COOLDOWN) return;
    o->cooldown = 0;
    switch (o->attack) {
    case ATTACK_STREAM:
        o->stream = 3;
        o->streamTimer = 0;
        break;
    case ATTACK_FAN:
        Shoot(o, KIND_STAR, o->sx, o->sy, -0x23333, -0x1B333);
        Shoot(o, KIND_STAR, o->sx, o->sy, -0x30000, 0);
        Shoot(o, KIND_STAR, o->sx, o->sy, -0x23333, 0x1B333);
        break;
    default:
        Shoot(o, KIND_PAIR, o->sx, o->sy, -0x20000, -0x20000);
        Shoot(o, KIND_PAIR, o->sx, o->sy, -0x20000, 0x20000);
        break;
    }
    PlaySfx(&o->n.obj.base, SE_KIRBY_AIRSHOT);
    if (++o->attack >= ATTACKS) o->attack = 0;
}

// SeqRun(); On through its look's sequence to its next frame (see sSpinSeq);
static void SeqRun(struct PowerOrb *o) {
    for (;;) {
        u8 at = o->seq[o->seqAt];

        if (at == SEQ_LOOP) {
            o->seqAt = 0;
        } else if (at == SEQ_TURN) {
            o->repeats--;
            o->seqAt++;
        } else if (at == SEQ_DASH) {
            o->seq = sDashSpin;
            o->seqAt = 2;
        } else {
            o->frame = at;
            o->seqLeft = o->seq[o->seqAt + 1] == SEQ_HP ? (o->n.obj.unk80 >> 3) + 1 : o->seq[o->seqAt + 1];
            o->seqAt += 2;
            return;
        }
    }
}

// SeqSet(); A look's sequence from its start (its first frame at once);
static void SeqSet(struct PowerOrb *o, const u8 *seq) {
    o->seq = seq;
    o->seqAt = 0;
    SeqRun(o);
}

static void SeqTick(struct PowerOrb *o) {
    if (!--o->seqLeft) SeqRun(o);
}

/* Look(); Its look this frame, after its move's (NiDL's 0x080AF114...): spinning, LOOK_WAIT frames, then its attack look (shooting,
 * TimedShot()) 1-4 times by chance, then spinning again...; or its move's own (LOOK_SCRIPTED); */
static void Look(struct PowerOrb *o) {
    switch (o->lookState) {
    case LOOK_SPIN:
        SeqTick(o);
        if (++o->lookCount >= LOOK_WAIT) {
            SeqSet(o, sAttackSeq);
            o->lookState = LOOK_START;
        }
        break;
    case LOOK_START:
        o->repeats = (Rand16() & 3) + 1;
        o->lookState = LOOK_ATTACK;
        TimedShot(o);
        break;
    case LOOK_ATTACK:
        SeqTick(o);
        if (o->repeats <= 0) {
            SeqSet(o, sSpinSeq);
            o->lookCount = 0;
            o->lookState = LOOK_SPIN;
        }
        break;
    default:
        SeqTick(o);
        break;
    }
}

/* HalfMove(); The start of each of NiDL's moves (its hover and roam two each): hovering, it spins (its attack look LOOK_WAIT frames on);
 * roaming, its attack look at once; the others, their own look; */
static void HalfMove(struct PowerOrb *o) {
    switch (o->state) {
    case STATE_HOVER:
        SeqSet(o, sSpinSeq);
        o->lookCount = 0;
        o->lookState = LOOK_SPIN;
        break;
    case STATE_ROAM:
        SeqSet(o, sAttackSeq);
        o->lookState = LOOK_START;
        break;
    case STATE_ZIP:
        SeqSet(o, sAttackSeq);
        o->lookState = LOOK_SCRIPTED;
        o->zipFrom = o->sy;
        break;
    case STATE_DASH:
        SeqSet(o, sDashSeq);
        o->lookState = LOOK_SCRIPTED;
        break;
    case STATE_CHARGE:
    case STATE_ESCAPE:
        SeqSet(o, sPulseSeq);
        o->lookState = LOOK_SCRIPTED;
        break;
    }
}

static void Set(struct PowerOrb *o, u8 state) {
    o->state = state;
    o->timer = 0;
    o->step = 0;
    o->stepTimer = 0;
    o->row = 0;
    o->phase = 0;
    o->vy = 0;
}

// Next(); Its next move (sMoves), through them LOOPS times; then it flies off (STATE_ESCAPE);
static void Next(struct PowerOrb *o) {
    if (o->state == STATE_ENTER) {
        o->move = o->loop = 0;
    } else if (++o->move >= ARRAY_COUNT(sMoves)) {
        o->move = 0;
        if (++o->loop >= LOOPS) {
            Set(o, STATE_ESCAPE);
            return;
        }
    }
    Set(o, sMoves[o->move]);
}

/* Dash(); Its way (the Kirby it's after: see sDashMiddle), then back round to its spot, its look the dash's then spinning (NiDL's
 * 0x0874B0A0); then on to hovering; */
static void Dash(struct PowerOrb *o) {
    const struct OrbStep *step;

    if (o->timer == 1) {
        struct Kirby *kirby = o->n.obj.kirby3;
        s32 y = kirby ? (kirby->base.y - ScreenTop(o)) >> 8 : DASH_LOW;

        o->row = y <= DASH_HIGH ? DASH_UP : y <= DASH_LOW ? DASH_MIDDLE : DASH_DOWN;
    }
    if (o->phase == 0 && o->step >= sDashSteps[o->row]) {
        o->phase = 1;
        o->step = 0;
        SeqSet(o, sDashBackSeq);
    }
    step = o->phase ? &sDashBack[o->step] : &sDashWays[o->row][o->step];
    if (!o->stepTimer) {
        o->stepTimer = step->time;
        if (!o->phase && o->step == sDashWhoosh[o->row]) PlaySfx(&o->n.obj.base, SE_WIZ_DASH);
    }
    o->sx += step->vx;
    // (the low way's back under: the same steps, upside down);
    o->sy += o->row == DASH_DOWN && o->phase ? -step->vy : step->vy;
    if (--o->stepTimer) return;
    // (back where it was, but for the high and low ways' half a pixel, which NiDL keeps);
    if (++o->step >= ARRAY_COUNT(sDashBack) && o->phase) Next(o);
}

/* Pulse(); Its colors this frame (see gNidlPowerOrbColors), in gObjPalette and the copy KatAM puts back after a flash (as Nightmare's,
 * bosses/nightmare.c), when they change (every 3 frames: KatAM's busiest frames here are near too long); */
static void Pulse(struct PowerOrb *o) {
    const u16 *a = gNidlPowerOrbColors[0], *b = gNidlPowerOrbColors[1];
    struct Object *obj = &o->n.obj;
    s32 amount = sPulse256[o->pulse / 3], i;
    u16 *colors;
    bool8 change = o->pulse % 3 == 0;

    if (++o->pulse >= PULSE_TIME) o->pulse = 0;
    if (!change || gKirbys[gLocalPlayerId].base.roomId != obj->base.roomId || !obj->base.sprite.palId) return;
    colors = &gObjPalette[obj->base.sprite.palId * 16];
    for (i = 0; i < 16; i++) {
        s32 r = a[i] & 0x1F, g = a[i] & 0x3E0, bl = a[i] & 0x7C00;

        r += ((b[i] & 0x1F) - r) * amount >> 8;
        g += ((b[i] & 0x3E0) - g) * amount >> 8;
        bl += ((b[i] & 0x7C00) - bl) * amount >> 8;
        colors[i] = (r & 0x1F) | (g & 0x3E0) | (bl & 0x7C00);
    }
    sub_0803D280(obj->base.sprite.palId * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

// Zip(); To each row at 6 pixels a frame (after a wait, ZIP_WAIT frames), a wait, a lunge as it shoots a crescent; then on to hovering;
static void Zip(struct PowerOrb *o) {
    s32 target;

    switch (o->phase) {
    case 0:
        // (the wait before it goes);
        if (++o->stepTimer <= ZIP_WAIT) return;
        o->stepTimer = 0;
        o->phase = 1;
        // fallthrough
    case 1:
        target = o->zipFrom + (sZipRows[o->row] << 8);
        if (o->sy < target - ZIP_SPEED) o->sy += ZIP_SPEED;
        else if (o->sy > target + ZIP_SPEED) o->sy -= ZIP_SPEED;
        else {
            o->sy = target;
            o->phase = 2;
        }
        return;
    case 2:
        if (++o->stepTimer <= ZIP_WAIT) return;
        o->stepTimer = 0;
        o->phase = 3;
        Shoot(o, KIND_CRESCENT, o->sx, o->sy, -0x60000, 0);
        PlaySfx(&o->n.obj.base, SE_KIRBY_AIRSHOT);
        // fallthrough
    case 3:
        o->sx += sLunge[o->stepTimer];
        if (++o->stepTimer < LUNGE_TIME) return;
        o->stepTimer = 0;
        if (++o->row >= ARRAY_COUNT(sZipRows)) {
            o->phase = 4;
            return;
        }
        o->phase = 0;
        return;
    default:
        if (++o->stepTimer >= ZIP_WAIT) Next(o);
        return;
    }
}

/* Intro(); The intro's frame t (see above): the screen rising, the orb's place and look (hidden until INTRO_SHOW; spinning, 8 frames
 * a frame; the pulses, the spin going on between them; spinning fast, 1 frame a frame); its frame, or -1 (hidden); */
static s16 Intro(struct PowerOrb *o, u16 t) {
    s32 n, moves;
    u8 i;

    for (n = t, i = 0, o->below = INTRO_BELOW; i < ARRAY_COUNT(sRiseTimes); n -= sRiseTimes[i++]) {
        o->below -= sRiseSpeeds[i] * (n < sRiseTimes[i] ? n + 1 : sRiseTimes[i]);
        if (n < sRiseTimes[i]) break;
    }
    o->sx = INTRO_X;
    o->sy = INTRO_Y;
    for (moves = t - INTRO_MOVE + 1, n = 0; n < moves && n < ARRAY_COUNT(sIntroMoves); n++) {
        o->sx += ((sIntroMoves[n] & 0xF) - 8) << 8;
        o->sy += ((sIntroMoves[n] >> 4) - 8) << 8;
    }
    if (t < INTRO_SHOW) return -1;
    if (t < INTRO_PULSE) return FRAME_SPIN + ((((t < INTRO_CLASH ? t : INTRO_CLASH - 1) - INTRO_SHOW) >> 3) & 3);
    if (t < INTRO_FAST) return ((t - INTRO_PULSE) & 7) < 6 ? FRAME_SHRINK + sPulse[(t - INTRO_PULSE) & 7] : FRAME_SPIN + ((((t - INTRO_PULSE) >> 3) + 1) & 3);
    return FRAME_SPIN + ((t - INTRO_FAST) & 3);
}

/* PowerOrbShotNear(); Whether one of its shots is coming at a spot (8.8, the room's): ahead of it (to its right, moving left) within
 * 72 pixels, in its row (20 up or down); its y in `shotY` (abilities/star_rod.c: the helpers dodge it now and then); */
bool32 PowerOrbShotNear(struct Object *obj, s32 x, s32 y, s32 *shotY) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct OrbShot *shot;
        s32 dx, dy;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != OrbShotMain) continue;
        shot = TaskGetStructPtr(t);
        if (shot->base.parent != obj || shot->base.roomId != obj->base.roomId || (shot->base.flags & 0x1000) || shot->vx >= 0) continue;
        dx = (shot->base.x - x) >> 8;
        dy = (shot->base.y - y) >> 8;
        if (dx < 0 || dx > 72 || dy < -20 || dy > 20) continue;
        *shotY = shot->base.y;
        return TRUE;
    }
    return FALSE;
}

/* PowerOrbGround(); The lowest a flying Kirby may be on the screen (abilities/star_rod.c): over the ground, as it rises into view at
 * the fall's end (NiDL's: GROUND_Y less how far the screen's sunk); */
s32 PowerOrbGround(struct Object *obj) {
    return GROUND_Y - (((struct PowerOrb *)obj)->sink >> 8);
}

// PowerOrbIntro(); The intro's frame (-1: over) (abilities/star_rod.c's StarRodIntro());
s32 PowerOrbIntro(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;

    return o->state == STATE_ENTER && o->timer ? o->timer - 1 : -1;
}

// OrbShotsGone(); Its shots and their sparkles gone (as it bursts: frozen till then);
static void OrbShotsGone(struct PowerOrb *o) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == OrbShotMain && ((struct OrbShot *)TaskGetStructPtr(t))->base.parent == o)
            ((struct OrbShot *)TaskGetStructPtr(t))->base.flags |= 0x1000;
        else if (t->main == OrbTrailMain && ((struct EffectObject *)TaskGetStructPtr(t))->parent == o)
            ((struct EffectObject *)TaskGetStructPtr(t))->flags |= 0x1000;
        else if (t->main == OrbTrailsMain && ((struct OrbTrails *)TaskGetStructPtr(t))->o == o)
            ((struct OrbTrails *)TaskGetStructPtr(t))->gone = TRUE;
    }
}

/* OrbDeath(); Its end's task (as KatAM's bosses', sub_08086E50: frames since its last hit): the freeze, the flashes, its burst (gone, its
 * end: NidlBossDefeat()'s PowerOrbEnd()), everything moving again (see above); */
struct OrbDeath {
    struct PowerOrb *o;
    u16 k;
    u8 player;
};

static void OrbDeathMain(void) {
    struct OrbDeath *d = TaskGetStructPtr(gCurTask);
    struct Object *obj = &d->o->n.obj;
    u16 k = ++d->k;

    if (k == DEATH_FLASH) sub_080860A8(&obj->base, gUnk_08350E34);
    if (k == DEATH_FREEZE) sub_08033540(d->player);
    if (k == DEATH_BURST) {
        OrbShotsGone(d->o);
        obj->base.flags |= 0x400;
        NidlBossDefeat(obj);
        obj->base.flags |= 0x1000;
    }
    if (k == DEATH_THAW) {
        sub_080335B4(d->player);
        TaskDestroy(gCurTask);
    }
}

// OrbDead(); Its state once beaten: still, as it was (KatAM's sub_0809F974);
static void OrbDead(struct Object *obj) {
    obj->base.flags |= 4;
    obj->base.objBase55 = -1;
}

/* Die(); Its last hit (KatAM's own for its bosses, sub_0809AF38's: the boss music's end, its sound, no more hits; but its end NiDL's,
 * OrbDeath(), not KatAM's sub_08088528); */
static void Die(struct PowerOrb *o) {
    struct Object *obj = &o->n.obj;
    struct Task *t = TaskCreate(OrbDeathMain, sizeof(struct OrbDeath), DEATH_PRIORITY, TASK_USE_EWRAM, NULL);
    struct OrbDeath *d = TaskGetStructPtr(t);

    d->o = o;
    d->k = 0;
    d->player = obj->base.unk56;
    // (its screen sinks no more: the cameras on it, PowerOrbMain());
    ArenaSink(obj, o->sink);
    o->dying = TRUE;
    o->hurt = 0;
    obj->unk80 = 0;
    obj->base.unk68 &= 7;
    obj->base.unk5C |= 0x80;
    obj->base.flags |= 0x200 | 0x40;
    obj->base.flags &= ~0x20;
    obj->unk78 = OrbDead;
    if (!(obj->objTemplate->unk22 & 4)) sub_08086C48(obj);
    PlaySfx(&obj->base, SE_MINIBOSS_PRE_EXPLODE);
}

// Hit(); A star's hit: 1 HP (KatAM's boss hit sound), its look when hit; the last, its end;
static void Hit(struct PowerOrb *o) {
    struct Object *obj = &o->n.obj;

    if (--obj->unk80 > 0) {
        o->hurt = HURT_TIME;
        PlaySfx(&obj->base, SE_08D58D6C);
        return;
    }
    Die(o);
}

// Fly(); Every Kirby free in its room takes the Star Rod (again, coming back, or after a hit KatAM's states took him);
static void Fly(struct Object *obj) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == obj->base.roomId && StarRodCanFly(&gKirbys[i])) StarRodStart(&gKirbys[i], obj);
}

static void PowerOrbMain(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;
    u8 i, frame;
    s32 camera;
    s16 intro = 0;

    NidlUpdate(&o->n);
    // (no box: it takes its hits itself, NiDL's, without KatAM's shake, freeze and sparks);
    obj->base.flags |= 4 | 0x200;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (its stars too); it takes it back;
    sub_08034E14(obj);
    Fly(obj);

    o->slots -= o->slotsGone;
    o->slotsGone = 0;
    // The screen sinks (its arena's, the camera with it), once the intro's over;
    o->fromX = o->sx >> 8;
    o->fromY = o->sy >> 8;
    o->lag = o->sink;
    if (o->state != STATE_ENTER && o->sink < SINK_END) {
        o->sink += SINK_SPEED;
        // (the cameras where NiDL's are, 8 + its sink, each frame: KatAM sets them as its frame starts, before its objects move, from
        // the arena's lock: held on next frame's screen);
        ArenaSink(obj, o->sink < SINK_END ? o->sink + SINK_SPEED : o->sink);
        // (down at the ground (NiDL's camera past 888), not beaten in time: every Kirby in its room loses a life, as in KatAM's pits);
        if (o->sink >= SINK_END && !o->dying) {
            for (i = 0; i < gNumKirbys; i++) {
                if (gKirbys[i].base.roomId != obj->base.roomId || gKirbys[i].hp <= 0) continue;
                gKirbys[i].hp = 0;
                sub_08088640(&gKirbys[i], 0x1C, 0x20);
            }
        }
    }
    if (o->cooldown < 0xFFFF) o->cooldown++;
    o->timer++;
    Pulse(o);
    if (o->timer == 1 || (o->timer == HALF_MOVE + 1 && (o->state == STATE_HOVER || o->state == STATE_ROAM))) HalfMove(o);
    switch (o->state) {
    case STATE_ENTER:
        intro = Intro(o, o->timer - 1);
        ArenaRise(obj, o->below << 8);
        if (o->timer - 1 == INTRO_CLASH) OrbSlash(o);
        if (o->timer - 1 >= INTRO_END) {
            o->below = 0;
            ArenaRise(obj, 0);
            o->sx = SPOT_X;
            o->sy = SPOT_Y;
            // (shown this frame where the intro left it, a pixel over its spot: NiDL's fight moves it from the next);
            o->lag = -0x100;
            obj->base.flags &= ~0x400;
            Next(o);
        }
        break;
    case STATE_HOVER:
        if (!o->stepTimer) o->stepTimer = sHoverTimes[o->step];
        o->sy += sHoverSpeeds[o->step];
        if (!--o->stepTimer && ++o->step >= ARRAY_COUNT(sHoverTimes)) Next(o);
        break;
    case STATE_ROAM:
        if (!o->stepTimer) o->stepTimer = sRoamTimes[o->step];
        o->sx += sRoamX[o->step];
        o->sy += sRoamY[o->step];
        if (!--o->stepTimer && ++o->step >= ARRAY_COUNT(sRoamTimes)) o->step = ROAM_LOOP;
        if (o->timer >= HALF_MOVE * 2) Next(o);
        break;
    case STATE_CHARGE:
        if (o->timer == CHARGE_FIRE) {
            // (all four from its middle, NiDL's);
            for (i = 0; i < 4; i++) {
                struct OrbShot *shot = Shoot(o, KIND_CHARGE, o->sx, o->sy, 0, 0);
                shot->side = i == 0 || i == 3 ? 0 : 1;
                shot->up = i < 2;
            }
            PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
            // (its timed shots wait for it too: 0x080AE7D0);
            o->cooldown = 0;
        }
        if (o->timer >= HALF_MOVE) Next(o);
        break;
    case STATE_ZIP:
        Zip(o);
        break;
    case STATE_DASH:
        Dash(o);
        break;
    case STATE_ESCAPE:
        // (NiDL's 0x080AEEF8: right at 1 pixel a frame, rising faster and faster, till it's gone over the screen; then still there);
        o->vy -= ESCAPE_RISE;
        o->sx += ESCAPE_SPEED;
        o->sy += o->vy;
        if (o->timer >= ESCAPE_TIME) Set(o, STATE_GONE);
        break;
    }
    if (o->state != STATE_ENTER && o->state != STATE_GONE) Look(o);
    // A stream's stars, 16 frames apart (the first from where it is now, the others from where it was as the frame began: NiDL's
    // come from a thing of their own, which goes where it is before it moves);
    if (o->stream && !o->streamTimer--) {
        if (o->stream == 3) Shoot(o, KIND_STAR, o->sx, o->sy, -0x40000, 0);
        else Shoot(o, KIND_STAR, o->fromX << 8, o->fromY << 8, -0x40000, 0);
        o->streamTimer = STREAM_GAP - 1;
        o->stream--;
    }

    obj->base.x = ScreenLeft(obj) + o->sx;
    obj->base.y = ScreenTop(o) + o->sy;
    obj->base.xspeed = obj->base.yspeed = 0;
    // (a pixel up from KatAM's enemies': KatAMExpansion.cs builds NiDL's frames a pixel lower, to stand as KatAM's; it floats where
    // NiDL's is, as bosses/nightmare.c's Nightmare);
    obj->base.objBase55 = -1;
    // (drawn as NiDL draws it: its pixel on the screen, the screen's sink a frame late, so it shows a pixel higher the frames the
    // screen steps down);
    camera = gCurLevelInfo[gLocalPlayerId].viewportPosition.y;
    obj->base.y = camera + (((o->sy >> 8) + (o->lag >> 8) - (o->sink >> 8)) << 8);
    // (flown off over the screen, hidden just above it: KatAM's HUD lets go of a boss further off, NiDL keeps its bar);
    if ((o->state == STATE_ESCAPE || o->state == STATE_GONE) && obj->base.y < camera - (OFF_SCREEN << 8)) {
        obj->base.y = camera - (OFF_SCREEN << 8);
        obj->base.flags |= 0x400;
    }
    // (the intro: on the screen as the camera shows it (its lock's last frame: ArenaRise()), harmless, hidden until it shows);
    if (o->state == STATE_ENTER) {
        obj->base.x = gCurLevelInfo[obj->base.unk56].viewportPosition.x + o->sx;
        obj->base.y = gCurLevelInfo[obj->base.unk56].viewportPosition.y + o->sy;
        if (intro < 0) {
            obj->base.flags |= 0x400;
            return;
        }
        obj->base.flags &= ~0x400;
        NidlSetFrame(&o->n, intro);
        return;
    }
    // Touching it hurts (the flying Kirbys: it hits them itself); gone, it's neither there nor hit (NiDL's 0x080AEF50);
    if (o->state == STATE_GONE) return;
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == obj->base.roomId && StarRodTouches(&gKirbys[i], obj->base.x, obj->base.y, 18, 18)) StarRodHit(&gKirbys[i], obj->base.x);
    // Its look (Look());
    frame = o->frame;
    // Its hits (the stars where they'll be this frame), the last its end (that look kept); hit, its look in lines over all that, every
    // third frame its spin's next (NiDL's 12, 12, 4, 13, 13, 5, 12...);
    for (i = StarRodHits(obj->base.roomId, obj->base.x, obj->base.y, HIT_REACH); i && !o->dying; i--) Hit(o);
    if (o->hurt) {
        u8 k = HURT_TIME - o->hurt--;

        frame = k % 3 == 2 ? FRAME_SPIN + ((k / 3) & 3) : FRAME_HURT + (k % 6 > 2);
    }
    NidlSetFrame(&o->n, frame);
}

/* The orb beaten (PowerOrbEnd()): NiDL's end of the fight, traced frame by frame (its object's last states, its warp star's and
 * the camera's, 0x0300215C), run by a task of its own (its object gone as it bursts, OrbDeath()):
 * • it bursts into eight stars out of its middle, one each way of the compass from up (NiDL's objects 0x9B, frames 0x0874CC60's 0-8,
 *   3 frames each, the first 2; 6 frames at 6 pixels a frame, then 3, 1.5 and 0.625, the diagonals' 4, 2, 1 and 0.5 each way: 24
 *   frames) and a sparkle in their middle (NiDL's 0x94, 0x0874CA78's frames by turns from 0 and from 12, 23 frames): OrbBurst();
 * • 24 frames after it bursts, it's back (a frame a pixel right and up of where it was, its spin's first frame), then wobbling (its
 *   shrink's frames, and two of its spin's every 8) it flies off up to the right (sFleeY, a pixel right a frame), and its warp star
 *   comes, 11 pixels under where it was (finale.c's NidlFinaleStarAt());
 * • the warp star off, every Kirby on it (its flight's first step NIDL_RIDE_FINALE: warpstar.c holds them on it, this moves them),
 *   it flies NiDL's launch up off the screen (sLaunch, then on with its last two moves till it's 32 pixels over the screen); 21
 *   frames in, the screen rises to the room's top (1.25 pixels a frame, a 16th more each frame, never more than a 16th of the way
 *   left, nor under an 8th), the star kept 34 pixels over it;
 * • 2 frames after the screen's at the top, the orb, small (a quarter of its size, smaller every 20 frames: sMoonSizes), flies into
 *   the moon (sMoonOrb, from (136, -15) on the screen), then too small, a dot (FRAME_DOT, NiDL's); 60 frames after it, the star
 *   with the Kirbys, the same way (sMoonStar; warpstar.c draws them at its size, PowerOrbRideSize()), then a dot too; 410 frames
 *   after the orb came in, the star's flight goes on to its fade to white (18 frames), then Nightmare's room;
 * the screen's held where the fight left it till then (its arena gone, the camera would go back to the fight's first screen,
 * the star left under it, and the Kirbys flying with the Star Rod, kept on the screen, with it); */
#define END_TILES 36
#define END_BACK 24
#define END_STAR_Y 11
#define END_CAMERA 21
#define END_SPEED 0x140
#define END_SPEEDUP 0x10
#define END_SLOWEST 0x20
#define END_ABOVE (-32 << 8)
#define END_HELD (-34 << 8)
#define END_OFF (-16 << 8)
#define END_ON_SCREEN (24 << 8)
#define END_TOP (8 << 8)
#define END_MOON_X (136 << 8)
#define END_MOON_Y (-15 << 8)
#define END_MOON_WAIT 2
#define END_STAR_LATE 60
#define END_FADE 410
#define END_DOT_KEY 0x6000
#define END_MATRIX 6        // (its affine matrices: 6, then 14, 22, 30 for pieces flipped; warpstar.c's riders use 7...);
#define NIDL_RIDE_FINALE 0xFC
#define NIDL_RIDE_HIDDEN 0x8000

void sub_080534D0(struct Kirby *);
void NidlShrink(u32 first, s32 x, s32 y, u32 pa, u32 matrix);

enum { END_HIDDEN, END_FLEE, END_GONE };

struct OrbEnd {
    struct EffectObject e;
    s32 camX, camY, camSpeed;   // The held screen's top left (8.8, the room's) and its speed rising;
    s32 x0, y0;                 // Where it was beaten (8.8);
    s32 orbX, orbY;             // The orb, and the ride's star (Kirby's spot on it), 8.8;
    s32 starX, starY;
    u16 t;                      // Frames since it burst;
    u16 ride;                   // Frames since the star went off (0: not yet);
    u16 top;                    // Frames since the screen got to the top (0: not yet);
    u16 moon;                   // Frames since the orb came into the moon (0: not yet);
    u16 animId;
    u8 player;
    u8 phase;
    u8 palId, dotPalId;
    bool8 above;                // The star's over the screen, kept there;
};

// The flee: its y (pixels from where it was beaten) frame by frame, x 2 pixels right of it, a pixel more each frame;
static const s16 sFleeY[] = { 10, 10, 9, 9, 8, 8, 7, 6, 5, 3, 3, 1, -1, -2, -4, -6, -8, -10, -13, -15, -17, -20, -23, -26, -29, -32, -35, -39,
                              -43, -46, -49, -54, -58, -62, -67, -70, -75, -80, -85, -90, -94, -100, -105, -111, -117, -121, -127, -134, -140,
                              -146, -152, -158, -165, -172, -179, -185, -192, -200, -207 };
// (its wobble: NiDL's frames 8, 9, 11, 10, 9, 8 as its shrink's, then two of its spin's);
static const u8 sFleeFrames[6] = { 0, 1, 3, 2, 1, 0 };
// The launch: the star's moves (pixels, down positive) frame by frame from its second;
static const s8 sLaunch[][2] = { { 1, -1 }, { 2, 0 }, { 1, -1 }, { 2, 0 }, { 1, -1 }, { 2, -2 }, { 1, -2 }, { 2, -2 }, { 1, -3 }, { 2, -4 }, { 1, -4 },
                                 { 2, -4 }, { 1, -5 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -6 }, { 1, -6 }, { 2, -8 },
                                 { 1, -7 }, { 2, -7 }, { 1, -7 }, { 2, -7 }, { 1, -8 } };
// Into the moon: the size (its matrix's pa: 256 / the size, 8.8) and frames, from a quarter, then a dot;
static const u16 sMoonSizes[][2] = { { 1024, 19 }, { 1092, 20 }, { 1170, 20 }, { 1260, 19 }, { 1365, 20 }, { 1489, 20 }, { 1638, 19 }, { 1820, 20 },
                                     { 2048, 20 }, { 2340, 19 }, { 2730, 20 } };
// Into the moon: the moves frame by frame, a pixel or none each way, 4 bits a frame (x + 1, (y + 1) << 2), low first: the orb's,
// then the star's (NiDL's own: it catches up with the orb in the moon's middle);
// (426 frames)
static const u8 sMoonOrb[213] = {
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84,
    0x94, 0x84, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x95, 0x94, 0x95, 0x94, 0x95, 0x94,
    0x55, 0x95, 0x55, 0x95, 0x54, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x56, 0x95, 0x55, 0x95,
    0x65, 0x95, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x55, 0xA5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x65, 0x95, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
};
// (366 frames)
static const u8 sMoonStar[183] = {
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84, 0x94, 0x84,
    0x94, 0x84, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x95, 0x94, 0x95, 0x94, 0x95, 0x94,
    0x55, 0x95, 0x55, 0x95, 0x54, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x55, 0x95, 0x56, 0x95, 0x55, 0x95,
    0x65, 0x95, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55,
    0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55, 0x55, 0x65, 0x55, 0x55, 0x95, 0x65, 0x55, 0x55,
};

// MoonSize(); The size k frames into the moon (sMoonSizes), 0 past it (a dot);
static u32 MoonSize(u16 k) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sMoonSizes); i++) {
        if (k < sMoonSizes[i][1]) return sMoonSizes[i][0];
        k -= sMoonSizes[i][1];
    }
    return 0;
}

// MoonMove(); Frame k's move of a path into the moon (sMoonOrb, sMoonStar), none past it;
static void MoonMove(const u8 *path, u16 frames, u16 k, s32 *x, s32 *y) {
    u8 code;

    if (k >= frames) return;
    code = k & 1 ? path[k >> 1] >> 4 : path[k >> 1] & 0xF;
    *x += ((code & 3) - 1) << 8;
    *y += (((code >> 2) & 3) - 1) << 8;
}

static void OrbEndMain(void);

// OrbEndOf(); The orb's end in a room (its task), or NULL;
static struct OrbEnd *OrbEndOf(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct OrbEnd *end;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != OrbEndMain) continue;
        end = TaskGetStructPtr(t);
        if (end->e.roomId == room && !(end->e.flags & 0x1000)) return end;
    }
    return NULL;
}

/* PowerOrbRideSize(); The size the ride's star and its Kirbys are drawn at (warpstar.c: on its flight's first steps): its matrix's
 * pa, 0 their own, NIDL_RIDE_HIDDEN too small to show (the orb's end draws a dot there), or off the screen's top (kept on it,
 * hidden: KatAM would point at the helpers there); */
u32 PowerOrbRideSize(u16 room) {
    struct OrbEnd *end = OrbEndOf(room);
    u32 pa;

    if (end == NULL) return 0;
    if (end->moon <= END_STAR_LATE) return end->ride && end->starY - end->camY <= END_OFF ? NIDL_RIDE_HIDDEN : 0;
    pa = MoonSize(end->moon - 1 - END_STAR_LATE);
    return pa ? pa : NIDL_RIDE_HIDDEN;
}

// EndPalette(); A palette's slot, loaded under `key` as frame `frame` has it (as NidlLoadPalette(): the orb's gone);
static u8 EndPalette(struct OrbEnd *end, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != end->e.roomId) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

// EndDraw(); The orb's sprite at (x, y) (8.8), frame `frame`, at the size of `pa` (0 its own);
static void EndDraw(struct OrbEnd *end, s32 x, s32 y, u8 frame, u32 pa) {
    struct EffectObject *e = &end->e;
    u32 first = gUnk_030024F0;

    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = frame == FRAME_DOT ? end->dotPalId : end->palId;
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    e->flags = (e->flags & ~0x400) | 4 | 1;
    EffectObjectPostUpdate(e);
    if (pa) NidlShrink(first, e->sprite.x, e->sprite.y, pa, END_MATRIX);
}

// EndRider(); Whether a Kirby rides its star (on its flight's first step, held there: warpstar.c);
static bool32 EndRider(struct Kirby *kirby, u16 room) {
    const struct Unk_08353510 *step = kirby->unk114;

    return kirby->base.roomId == room && kirby->stateFn == sub_080534D0 && step != NULL && step->unk8 == NIDL_RIDE_FINALE && !(step->unk9 & 0x40);
}

// EndRide(); The star's ride (see above): the riders moved with it, the screen rising, the orb and the star into the moon, the fade;
static void EndRide(struct OrbEnd *end) {
    struct Kirby *lead = NULL;
    u16 k;
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (EndRider(&gKirbys[i], end->e.roomId) && lead == NULL) lead = &gKirbys[i];
    if (!end->ride) {
        if (lead == NULL) return;
        end->starX = lead->base.x;
        end->starY = lead->base.y;
    }
    k = end->ride++;
    // (its launch, up off the screen, then held over it);
    if (!end->moon && !end->above && k) {
        u16 at = k - 1 < ARRAY_COUNT(sLaunch) ? k - 1 : ARRAY_COUNT(sLaunch) - 2 + ((k - 1 - ARRAY_COUNT(sLaunch)) & 1);

        end->starX += sLaunch[at][0] << 8;
        end->starY += sLaunch[at][1] << 8;
        if (end->starY - end->camY <= END_ABOVE) end->above = TRUE;
    }
    // (the screen rising to the room's top);
    if (k >= END_CAMERA && !end->top) {
        s32 left = end->camY - END_TOP;

        end->camSpeed = end->camSpeed ? end->camSpeed + END_SPEEDUP : END_SPEED;
        if (end->camSpeed > left >> 4) end->camSpeed = left >> 4;
        if (end->camSpeed < END_SLOWEST) end->camSpeed = END_SLOWEST;
        end->camY = end->camSpeed < left ? end->camY - end->camSpeed : END_TOP;
        if (end->camY == END_TOP) end->top = 1;
    } else if (end->top) {
        end->top++;
    }
    if (end->above && !end->moon) end->starY = end->camY + END_HELD;
    // (into the moon: the orb, then the star);
    if (end->top == END_MOON_WAIT + 1) {
        end->moon = 1;
        end->orbX = end->camX + END_MOON_X;
        end->orbY = end->camY + END_MOON_Y;
    } else if (end->moon) {
        MoonMove(sMoonOrb, ARRAY_COUNT(sMoonOrb) * 2, end->moon - 1, &end->orbX, &end->orbY);
        if (end->moon - 1 == END_STAR_LATE) {
            end->starX = end->camX + END_MOON_X;
            end->starY = end->camY + END_MOON_Y;
        } else if (end->moon - 1 > END_STAR_LATE) {
            MoonMove(sMoonStar, ARRAY_COUNT(sMoonStar) * 2, end->moon - 2 - END_STAR_LATE, &end->starX, &end->starY);
        }
        end->moon++;
    }
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!EndRider(kirby, end->e.roomId)) continue;
        kirby->base.x = end->starX;
        kirby->base.y = end->starY;
        // (off the screen, hidden, they're kept on it: KatAM points at helpers off it, hidden or not);
        if (end->starY - end->camY <= END_OFF && end->moon <= END_STAR_LATE) kirby->base.y = end->camY + END_ON_SCREEN;
        // (on to the fade: its flight's next step);
        if (end->moon == END_FADE + 2) {
            kirby->unk114++;
            kirby->base.counter = ((const struct Unk_08353510 *)kirby->unk114)->unk4;
        }
    }
}

static void OrbEndMain(void) {
    struct OrbEnd *end = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &end->e;
    u16 k;
    u8 i;

    if (e->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    // (gone with the room: every Kirby left it);
    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == e->roomId) break;
    if (i >= gNumKirbys) {
        e->flags |= 0x1000;
        return;
    }
    e->flags |= 4 | 0x400;
    EndRide(end);
    NidlFinaleHoldCamera(e->roomId, end->camX, end->camY);
    k = end->t++;
    if (k == END_BACK) {
        NidlFinaleStarAt(end->player, end->x0, end->y0 + (END_STAR_Y << 8));
        end->phase = END_FLEE;
    }
    // (back, wobbling, flying off up to the right);
    if (end->phase == END_FLEE) {
        k -= END_BACK;
        if (!k) {
            // (back a frame first, a pixel right and up of where it was, its spin's first frame);
            EndDraw(end, end->x0 + 0x100, end->y0 - 0x100, FRAME_SPIN, 0);
        } else if (--k >= ARRAY_COUNT(sFleeY)) {
            end->phase = END_GONE;
        } else {
            end->orbX = end->x0 + ((2 + k) << 8);
            end->orbY = end->y0 + (sFleeY[k] << 8);
            EndDraw(end, end->orbX, end->orbY, (k & 7) < 6 ? FRAME_SHRINK + sFleeFrames[k & 7] : FRAME_SPIN + (((k >> 3) + 1) & 3), 0);
        }
    }
    // (into the moon, smaller and smaller, then a dot; the star's dot after it, warpstar.c hiding it and its Kirbys);
    if (end->moon > 1) {
        u32 pa = MoonSize(end->moon - 2);

        EndDraw(end, end->orbX, end->orbY, pa ? FRAME_SPIN : FRAME_DOT, pa);
        if (!pa && end->moon - 2 > END_STAR_LATE && !MoonSize(end->moon - 2 - END_STAR_LATE)) EndDraw(end, end->starX, end->starY, FRAME_DOT, 0);
    }
}

// OrbBurst(); The burst (see above): a star (way 0-7, from up round to the right), or the sparkle in their middle (way 8);
#define BURST_TIME 24
#define BURST_PHASE 6
#define SPARKLE_TIME 23
#define SPARKLE_WAY 8
static const s16 sBurstSpeeds[2][4] = { { 0x600, 0x300, 0x180, 0xA0 }, { 0x400, 0x200, 0x100, 0x80 } };
static const s8 sBurstWays[8][2] = { { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 } };

struct OrbBurst {
    struct EffectObject e;
    u16 animId;
    u8 t;
    u8 way;
    u8 palId;
};

static void OrbBurstMain(void) {
    struct OrbBurst *b = TaskGetStructPtr(gCurTask);
    struct EffectObject *e = &b->e;

    if (e->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (b->t >= (b->way == SPARKLE_WAY ? SPARKLE_TIME : BURST_TIME)) {
        e->flags |= 0x1000;
        return;
    }
    // (its sprite on its first frame: the trails' tiles are free by then, their sprites gone, OrbShotsGone(); the frame it was made
    // they still showed);
    if (!b->t)
        EffectObjectInitSprite(e, &e->sprite, b->way == SPARKLE_WAY ? SPARKLE_TILES : BURST_TILES, b->animId,
                               b->way == SPARKLE_WAY ? FRAME_SPARKLE : FRAME_BURST, 0x1A);
    if (b->way == SPARKLE_WAY) {
        e->sprite.variant = FRAME_SPARKLE + (b->t & 1 ? 12 + b->t / 2 : b->t / 2);
    } else {
        const s8 *way = sBurstWays[b->way];
        s32 speed = sBurstSpeeds[way[0] && way[1]][b->t / BURST_PHASE];

        e->x += way[0] * speed;
        e->y += way[1] * speed;
        e->sprite.variant = FRAME_BURST + (b->t + 1) / 3;
    }
    b->t++;
    // (facing right: KatAM draws effects mirrored otherwise, NiDL's as they are);
    e->flags |= 4 | 1;
    e->sprite.palId = b->palId;
    EffectObjectPostUpdate(e);
}

static void OrbBurst(struct OrbEnd *end, u8 way) {
    struct Task *t = TaskCreate(OrbBurstMain, sizeof(struct OrbBurst), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct OrbBurst *b = TaskGetStructPtr(t);
    struct EffectObject *e = &b->e;

    ClearEffectObject(e);
    e->header.kind = 3;
    e->x = end->x0;
    e->y = end->y0;
    e->parent = NULL;
    e->roomId = end->e.roomId;
    b->t = 0;
    b->way = way;
    b->palId = EndPalette(end, gNidlData.enemies[NIDL_SPIT_STAR].animId, 0);
    b->animId = end->animId;
    e->sprite.tilesVram = 0;
}

// PowerOrbEnd(); The orb beaten (finale.c's PowerOrbBeaten(), as it bursts: OrbDeath()): its end (see above);
void PowerOrbEnd(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;
    struct Task *t = TaskCreate(OrbEndMain, sizeof(struct OrbEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct OrbEnd *end = TaskGetStructPtr(t);
    struct EffectObject *e = &end->e;
    u8 i;

    ClearEffectObject(e);
    e->header.kind = 3;
    e->x = end->x0 = end->orbX = obj->base.x;
    e->y = end->y0 = end->orbY = obj->base.y;
    e->parent = NULL;
    e->roomId = obj->base.roomId;
    end->player = obj->base.unk56;
    e->flags |= 0x400;
    end->camX = ScreenLeft(obj);
    end->camY = ScreenTop(o);
    end->camSpeed = 0;
    end->starX = end->starY = 0;
    end->t = end->ride = end->top = end->moon = 0;
    end->phase = END_HIDDEN;
    end->above = FALSE;
    end->animId = NidlGraphicsOf(&o->n)->animId;
    EffectObjectInitSprite(e, &e->sprite, END_TILES, end->animId, FRAME_SPIN, 0x1A);
    end->palId = EndPalette(end, end->animId, FRAME_SPIN);
    end->dotPalId = EndPalette(end, end->animId | END_DOT_KEY, FRAME_DOT);
    for (i = 0; i <= SPARKLE_WAY; i++) OrbBurst(end, i);
}

// PowerOrbStart(); The fight starts (also the engine's after a hit: it goes on where it was);
void PowerOrbStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, PowerOrbMain);
}

// PowerOrbInit(); Its bounds (40x40, through anything; no box: it takes its hits itself), its start (the intro's: hidden, over its
// screen's right);
void PowerOrbInit(struct NidlObject *n) {
    struct PowerOrb *o = (struct PowerOrb *)n;
    struct Object *obj = &n->obj;

    o->sx = INTRO_X;
    o->sy = INTRO_Y;
    o->sink = o->lag = 0;
    o->below = INTRO_BELOW;
    o->cooldown = COOLDOWN;
    o->attack = ATTACK_STREAM;
    o->stream = o->streamTimer = 0;
    o->hurt = 0;
    o->dying = FALSE;
    o->slots = o->slotsGone = 0;
    o->move = o->loop = 0;
    o->pulse = PULSE_START;
    o->frame = FRAME_SPIN;
    o->lookState = LOOK_SCRIPTED;
    o->seq = sSpinSeq;
    o->seqAt = 0;
    o->seqLeft = 1;
    Set(o, STATE_ENTER);
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags |= 0x100 | 0x400;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    ObjectSetBounds(&obj->base, -20, -20, 20, 20);
}
