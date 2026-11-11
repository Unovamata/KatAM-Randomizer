/* Dark Nebula (Squeak Squad's last boss); ported from Kirby: Squeak Squad (USA), enemies 0x46 (his coming, a8s4s5: 0x021376EC) and
 * 0x3F (his fight, a8s4s6: his body 0x02122CD0, vtable 0x02182FB8: +0x34 per frame 0x0212301C, +0x30 his hit 0x021228C4, +0x18 each
 * frame 0x02122804; his states 0x0211CD00-0x021226E8), as SQUEAK_DARK_NEBULA, a boss on the mid-bosses' framework (mini-boss type
 * 0x40: the camera held on its area, the HP bar, the music, KatAM's defeat): his coming, then his fight in the same room (Squeak
 * Squad's stage changes between them). Values from param/prmLB02Ml0.kgb (150 values: ML[n] in the comments), prmLB02Pl0.kgb (30:
 * PL[n]) and prmLB02Sl0.kgb (his HP, 300: here 60, as Mecha-Kracko's 320 is 64); speeds 20.12 a frame, Y up; drawn with
 * enemy/darkZERO_cmn and darkZERO_ext (one palette set: the files' are the same), his eye and his flames sprites of their own, as
 * Squeak Squad's (0x0211C4E0's object, his +0x1F8; 0x0211B98C's, his +0x1FC). Recorded on the DS in his stage (map/a8s4s6, loaded
 * by repointing the stage table: his moves, as below).
 * • His arena (a8s4s6: 288 by 240, the floor 80 up; Squeak Squad's spots are from the camera view, 256 by 192, its top 144 over the
 *   floor): here from the camera view's middle across (a spot `a` pixels from the DS view's left is a - 128 from the middle) and the
 *   floor where he was placed (`b` pixels under its top is 144 - b over the floor).
 * • His coming (0x46: 0x0213756C-0x02136F28): a dark star (cmn's 1) 360 pixels left of his spot (204 across, 52 down: a8s4s6's
 *   spawn), flying in at 3.0 120 frames; its spin's end, three more; it forms (ext's 22, 124 frames), his eye opens (ext's .mca 1, 96),
 *   looks (.mca 2, three times 32) and glints (.mca 3, 32), 24 frames; then his fight.
 * • His fight's start (0x021226E8, 0x021225B4): the black star (cmn's 2), a form at random (1-3), 50 frames, a shake, his eye's
 *   twinkle; at its end (0x021220D0) the screen whitens (60 frames: 0x0203C174), his form shows (fire, spark or ice: cmn's 7, 8, 6),
 *   the screen clears (60 frames); idle (0x02121CB8: 40 frames, his eye half shut) and a pick (0x02121A64):
 *   - fire (1): 35 of 100 his dashes, 40 a shared move, 25 his fireball; spark (2): 30 his wave, 30 shared, 40 his bolts; ice (3):
 *     30 his orbit, 30 shared, 40 his pillar. A shared move: one of four (0x021204DC, rand >> 30).
 *   - After each move (0x021224F0): a count down; past it (3-5 moves again, PL[0], PL[1]) the other two forms one at random
 *     (0x021222E4), 240 frames later (0x02122804, whatever he's doing) a swoop to the view's top middle (130, 33: 20 frames, 3.0 up,
 *     0.3 down), 30 frames, his eye's twinkle, the screen whitening, his new form, the screen clearing.
 *   - His dashes (0x02121994): 3-5, at 5.0 between 7 spots (a pick, never the one he's at), 6 frames between; his eye shooting.
 *   - His fireball (0x0211EBE4): to (130, 33), his eye's blink, 40 frames, the fireball (0x0212F89C: falling 21/4096 a frame, 50 HP:
 *     10 here, 5 frames safe after a hit; broken, 5 sparks): on the floor a burst (0x0212F448: cmn's 12, then the fire wall, .mca 0,
 *     50-69 frames, then cmn's 19), flames along the floor either way (2.6), each 16-pixel tile of the floor they pass burning
 *     (0x0212F194: the stage's fire tile, map/25_f.bin's, a flame's trail here, hurting; 47 frames, the DS's 42-52); the move
 *     ends when the fireball is gone.
 *   - His wave (0x02121480): 2-3 waves across at 1.0 (turned at the view's sides), his height a wave (3.0° a frame, 120 high, then 100)
 *     from 8 under the floor, rising or sinking 2.5 a frame at most; his eye shooting.
 *   - His bolts (0x0211E850): to (128, 80), his eye's blink, 40 frames, four bolts to the view's corners (72 frames), then the cage
 *     (ext's .mca 0, 40 frames): its sides, its floor and its top hurt; 60 frames.
 *   - His orbit (0x02120C54): 1-2 laps round an ellipse (128, 112; 120 across, 80 high), speeding up 0.3° a frame to 2.0°, his
 *     steps at most 6 a frame; his eye shooting.
 *   - His pillar (0x0211E290): to a top corner (224 or 32, 32), his eye's blink, 40 frames, the ice pillar on the floor under him
 *     (cmn's 20), both across at 0.8 to the other side.
 *   - Shared: a row of stars (0x02120220: from a low corner, 223 or 33 at 128, 3-8 every 12 frames, straight across at 3.0); a rain
 *     (0x0211FC28: from a top corner, 52 down, 3-8 at Kirby's floor 24 about him); four (0x0211F628: from the top middle, at the floor
 *     at 16, 96, 160 and 240); a drop (0x0211F0EC: across the top at 2.0, a star down at 32, 80, 128, 176 and 224).
 *   - His eye shooting (0x0211BEB4): each frame, 1 in 100 a star at 1.5 at Kirby's 24 pixels ahead, else 4 in 100 at Kirby at 2.0.
 * • Hit (0x021228C4): only in a form (the black star has no box), not while the screen whitens or clears, nor 60 frames after a hit
 *   (blinking); his touch hurts in a form (darkZERO_cmn.rca's 16 round him, hit 17 round).
 * Checked against the DS (his stage recorded, and each move forced there and in KatAM from the same spot, frame 200: his spots,
 * states and things frame by frame; screenshots side by side): his coming, his start, the whitening and clearing, the idle, every
 * move above, the swoop and form change, the fireball's burst, flames and burning floor, the bolts and cage, the pillar.
 * • His defeat (0x021228C4 his HP out, 0x0211DF2C-0x0211CDCC; ML[128]-ML[149]): KatAM's mini-boss defeat (sub_0809AF38's
 *   sub_0809D060) caught the frame it starts (DarkNebulaWatch, his unk7C) and his own instead, his HP left at 0 (the bar empty, as the
 *   DS's): the Kirbys' controls taken, the far view darkening to black over 240 frames, the screen whitening (2 frames) and clearing
 *   (60) three times while his forms flicker (every 5, 5 and 8 frames; the second time his black star every third, the last every
 *   second), his black star at the view's (128, 64), the far view back, the controls given back (white 11 frames, clearing 10), his
 *   eye shaking (12 offsets, 0x0211BAB0); 120 frames of sparks (one every 20 frames at 1.5: thrown at rand(180)° up, falling 0.1, one
 *   bounce at the floor, 0.6 of their fall back up, 0x0211CBAA), his flames gone, his star shaking (ext's 14), his music fading out
 *   (240 frames); 120 frames of sparks (every 4 at 3.0), his star breaking up (ext's 15, 112 frames; sparks every 6 at 2.0); 120 frames
 *   (every 10 at 2.0), his eye left still; 120 frames, its twinkle (cmn's 5); the screen whitening (120 frames, sound 0x1DA), where the
 *   DS's stage ends. Here the white then clears (30 frames) and he's gone: KatAM's mini-boss end, the finale's dance on (squeak.c's
 *   SqueakDestroy). His music fades only where it's his own (his template's unk22 bit 2, KatAMExpansionSqueakRooms.cs: KatAM's
 *   mini-boss defeat leaves the music be); the far view darkens only in Squeak Squad's rooms (its palettes are known there).
 *   Checked in KatAM (USA, Europe, Japan: his stage, KatAM's defeat's writes made by hand, as a hit would: the DS's states, timings and
 *   pictures; no Kirby's hit landed in that test, his mini-boss immunities, unk5C, keeping off the slide).
 * • His floor in his own stage (a8s4s6): the first under his spot (the dark star's, in the air), the DS's 80 over its bottom.
 * • His form's tint (0x021225B4 at his fight's start, 0x021222E4 at each form he picks: 0x02209F84's +0x3C-0x44, over ML[7] frames
 *   from where it is): the stage's background its own color (fire red, spark green, ice blue) times 1.0, the other two times 0.502
 *   (0x808); here the far view's colors (ShowTint(): the stars in it too, they're the far view's), kept through KatAM's palette
 *   effects (FarViewKept()). His HP bar on the HUD from his black star's wait's end (0x020B308C: filling, sound 0x177), not his coming
 *   (KatAM's HUD brings it in about 55 frames later, then fills it, as it does every boss's bar). His music is his rooms' own (map
 *   music 37, a8s4s5 and a8s4s6: KatAMExpansionSqueakSound.cs's SqueakRoomSong), which his code never changes (no KatAM mini-boss
 *   music: he doesn't start it), faded at his defeat.
 * His sounds: Squeak Squad's (SqueakSfx). Not ported: the screen shakes' strength (KatAM's own), stars turned back by Squeak Squad's
 * reflecting attacks (flag 0x400: KatAM has none), the fire's heat (0x0212F138: an object drawing nothing with only a grab box,
 * harmless), the stage's +0x9C bit 2 while the screen whitens and clears at a form change (0x021220D0 sets it, 0x02121D14 clears it:
 * a few of Kirby's own DS states, 0x02017B60, 0x02037DC4, 0x020BBD9C, hold a change of state while it's set; KatAM's Kirby has
 * none of them), his defeat's message handler at the white-out (0x020217D0: an effect's animation set while the screen is all white,
 * which the DS's whitening covers too); the floor's burning time is the DS's average (its own timing not found). */
#include "../squeak.h"
#include "kirby.h"
#include "main.h"
#include "gba/m4a.h"
#include "constants/object_types.h"
#include "code_0806F780.h"

void RequestScreenShake(u16, struct ObjectBase *);
u16 ArcTan2(s16 x, s16 y);
extern const s16 gSineTable[0x500];

// His animations (darkZERO_cmn's 1, 2, 6, 7, 8, ext's 22), his composites (ext's .mca 1-3);
enum { ANIM_STAR, ANIM_BLACK, ANIM_ICE, ANIM_FIRE, ANIM_SPARK, ANIM_FORMING };
// (then ext's 19-21 and 23-25, his composites' parts; then ext's 14 and 15, his defeat's black star shaking and breaking up);
#define ANIM_SHAKING 12
#define ANIM_BREAKING 13
enum { C_OPEN, C_LOOK, C_GLINT };
// His eye's (cmn's 3, ext's 17, 18); the things' (each character's own, in KatAMExpansionSqueak.cs' order);
enum { EYE_IDLE, EYE_BLINK, EYE_HALF };
enum { SHOT_STAR, SHOT_FLAME };
enum { FIRE_BURST, FIRE_FADE };
#define C_WALL 0
enum { BOLT_DOWN_LEFT, BOLT_DOWN_RIGHT, BOLT_UP_LEFT, BOLT_UP_RIGHT };
#define C_CAGE 0

enum { FORM_NONE, FORM_FIRE, FORM_SPARK, FORM_ICE };

// prmLB02Ml0.kgb (pixels as Squeak Squad's ints, speeds 20.12);
#define GRAVITY 0x19A           // ML[0];
#define BLINK_TIME 60           // ML[1];
#define IDLE_WAIT 40            // ML[2];
#define AIM_WAIT 40             // ML[3];
#define FLY_SPEED 0x2000        // ML[4];
#define START_WAIT 50           // ML[6];
#define TINT_TIME 120           // ML[7], ML[8]: the next form 240 frames after it's picked;
#define FORM_DELAY 240
#define TINT_HALF 0x808         // (0x021222E4, 0x021225B4: a form's own color times SHADE_FULL, the other two times this);
#define SWOOP_X 130             // ML[9]-ML[14];
#define SWOOP_Y 33
#define SWOOP_TIME 20
#define SWOOP_VY 0x3000
#define SWOOP_AY (-0x4CD)
#define SWOOP_WAIT 30
#define WHITEN_TIME 60          // ML[15], ML[16];
#define CLEAR_TIME 60
#define DASH_SPEED 0x5000       // ML[18], ML[19];
#define DASH_PAUSE 6
#define WAVE_VX 0x1000          // ML[35]-ML[42];
#define WAVE_STEP 0x2800
#define WAVE_UP 40
#define WAVE_TURN 0x3000
#define WAVE_HIGH 60
#define WAVE_LOW 50
#define ORBIT_MAX 0x2000        // ML[44]-ML[53] (degrees 20.12);
#define ORBIT_ACCEL 0x4CD
#define ORBIT_STEP 0x6000
#define ORBIT_LAP_RIGHT 50
#define ORBIT_LAP_LEFT 130
#define ORBIT_X 128
#define ORBIT_Y 112
#define ORBIT_RX 120
#define ORBIT_RY 80
#define EYE_AIM 0x2000          // ML[56]-ML[58];
#define EYE_LEAD 0x1800
#define EYE_AHEAD 24
#define ROW_RIGHT_X 223         // ML[60]-ML[66];
#define ROW_LEFT_X 33
#define ROW_Y 128
#define ROW_ROUND 12
#define ROW_GAP 12
#define STAR_SPEED 0x3000
#define RAIN_RIGHT_X 223        // ML[68]-ML[74];
#define RAIN_LEFT_X 33
#define RAIN_Y 52
#define RAIN_AT 140
#define RAIN_ROUND 24
#define RAIN_GAP 12
#define FOUR_X 130              // ML[77]-ML[84];
#define FOUR_Y 33
#define FOUR_AT 140
#define FOUR_SPEED 0x2000
#define DROP_VX 0x2000          // ML[86]-ML[96];
#define DROP_RIGHT_X 240
#define DROP_LEFT_X 15
#define DROP_Y 30
#define DROP_VY 0x2000
#define ORB_X 130               // ML[98]-ML[109];
#define ORB_Y 33
#define ORB_AY 21
#define WALL_TIME 50
#define WALL_RAND 20
#define ORB_SAFE 5
#define FLAME_VX 0x299A
#define BOLTS_X 128             // ML[111]-ML[118];
#define BOLTS_Y 80
#define BOLT_TIME 72
#define CAGE_X 128
#define CAGE_Y 144
#define CAGE_TIME 40
#define CAGE_AFTER 60
#define PILLAR_VX 0xCCD         // ML[120]-ML[126];
#define PILLAR_RIGHT_X 224
#define PILLAR_LEFT_X 32
#define PILLAR_Y 32
#define PILLAR_AT 144
#define SPARK_SPEED 0x2800      // 0x0212F57C;
#define ORB_HP_BASE 100         // (its HP kept over this: KatAM beats a thing at 0, before its sparks);
#define SPARK_TIME 20
/* The floor his flames set burning (0x0212F194: the stage's vtable +0x90, 0x02216BD8: the 16-pixel tile the flame is in, over the
 * floor, its fire tile, which hurts; as recorded, each burned 42-52 frames, 47 on average: the stage's own timing, not found); */
#define BURN_TIME 47
// The dark star's coming (0x46: 0x0213756C-0x02137000);
#define COME_SPEED 0x3000
#define COME_TIME 120
#define COME_X 204              // a8s4s6's spawn (220, 172: 204 from the DS view's left, 52 under its top);
#define COME_Y 52
#define GLINT_WAIT 24
/* His defeat (0x0211DF2C-0x0211CDCC: ML[128]-ML[149]): the screen's whitenings and clearings, his forms flickering, the far view
 * darkening, his black star, its sparks (0x0211CBAA: an effect at him, thrown at rand(180)°, falling ML[0] a frame, bouncing once
 * under the view's 144, its fall up 0.6 times, 0x0211C9A8), its shaking and breaking up, his eye's twinkle, the white-out; */
#define DEFEAT_CLEAR 60         // ML[128]: each clearing after a whitening (FLASH frames: 0x0203C174's 2);
#define FLASH 2
#define DEFEAT_DARK 240         // ML[129]: the far view to black (the stage's background tint, 0x02209F84's +0x1C-0x44);
#define FLICKER_1 5             // ML[130]-ML[132]: a form shown every 5, 5 and 8 frames;
#define FLICKER_2 5
#define FLICKER_3 8
#define DEFEAT_X 128            // ML[133], ML[134]: his black star's spot (the view's middle, 64 under its top);
#define DEFEAT_Y 64
#define WHITE_HOLD 10           // ML[135], ML[136];
#define WHITE_CLEAR 10
#define DEFEAT_STEP 120         // ML[137];
#define SPARKS_1 20             // ML[138]-ML[144]: a spark every 20, 4, 6 and 10 frames, at 1.5, 3.0, 2.0 and 2.0 (ML[145]'s 1.8
#define SPARK_1 0x1800          // unread);
#define SPARKS_2 4
#define SPARK_2 0x3000
#define SPARKS_3 6
#define SPARK_3 0x2000
#define SPARKS_4 10
#define SPARK_FLOOR 144         // ML[146]-ML[148];
#define SPARK_BOUNCES 1
#define SPARK_GAIN 0x99A
#define WHITE_OUT 120           // ML[149];
#define MUSIC_FADE 15           // (0x020B0678's 240 frames: KatAM's m4a fades in 16 steps);
#define WHITE_BACK 30           // (here: the white-out cleared before the finale goes on, its Kirbys' dance shown);
#define SHADE_FULL 0x1000        // (the far view's colors times this, 20.12: KatAM's BG palettes 6-13, squeak/gimmicks.c's);
#define FAR_VIEW_FIRST 96
#define FAR_VIEW_COLORS 128
extern u16 gUnk_02022120[0x100];    // (the BG palettes as loaded);

// The view (Squeak Squad's: 256 across, its top 144 over the floor);
#define VIEW_HALF 128
#define VIEW_TOP 144

static const u8 sDashSpots[7][2] = { { 128, 32 }, { 32, 40 }, { 224, 40 }, { 32, 80 }, { 96, 64 }, { 160, 64 }, { 224, 80 } };
// (ML[91]-ML[95]: 32, 80, 128, 176 and 224 across a8s4s6 itself, not its view, whose left is at 16);
static const u8 sDropMarks[5] = { 16, 64, 112, 160, 208 };
static const u8 sFourAt[4] = { 16, 96, 160, 240 };

enum {
    DN_COME, DN_COME_STOP, DN_COME_SPIN, DN_COME_FORM, DN_COME_OPEN, DN_COME_LOOK, DN_COME_GLINT, DN_COME_END,
    DN_START, DN_TWINKLE, DN_WHITEN, DN_CLEAR, DN_IDLE, DN_PICK, DN_END, DN_SWOOP, DN_SWOOP_WAIT,
    DN_DASHES, DN_DASH, DN_DASH_PAUSE, DN_ORB_FLY, DN_ORB_AIM, DN_ORB, DN_ORB_DONE, DN_WAVE, DN_WAVE_RISE, DN_WAVE_ON, DN_WAVE_LAST,
    DN_BOLTS_FLY, DN_BOLTS_AIM, DN_BOLTS, DN_BOLTS_CAGE,
    DN_BOLTS_AFTER, DN_ORBIT, DN_PILLAR_FLY, DN_PILLAR_AIM, DN_PILLAR, DN_ROW_FLY, DN_ROW_AIM, DN_ROW, DN_RAIN_FLY, DN_RAIN_AIM,
    DN_RAIN, DN_FOUR_FLY, DN_FOUR_AIM, DN_DROP_FLY, DN_DROP_AIM, DN_DROP,
    DN_DEFEAT, DN_DEFEAT_FLASH, DN_FLICKER_1, DN_FLASH_2, DN_FLICKER_2, DN_FLASH_3, DN_FLICKER_3, DN_BLACK, DN_BLACK_HOLD,
    DN_BLACK_CLEAR, DN_SPARKS_1, DN_SPARKS_2, DN_SPARKS_3, DN_SPARKS_4, DN_EYE_WAIT, DN_TWINKLE_END, DN_WHITE_OUT, DN_WHITE_BACK,
};

// Things (the variant): what he and his eye bring in, each its character (sThingEnemies);
enum {
    T_EYE, T_TWINKLE, T_AURA, T_STAR, T_SPARK, T_FLAME, T_FIREBALL, T_BURST, T_PILLAR, T_BOLT, T_CAGE, T_CAGE_SIDE, T_CAGE_FLOOR,
    T_CAGE_TOP, T_BURN,
};
static const u8 sThingEnemies[] = {
    [T_EYE] = SQUEAK_DARK_NEBULA_EYE, [T_TWINKLE] = SQUEAK_DARK_NEBULA_TWINKLE, [T_AURA] = SQUEAK_DARK_NEBULA_AURA,
    [T_STAR] = SQUEAK_DARK_NEBULA_SHOT, [T_SPARK] = SQUEAK_DARK_NEBULA_SHOT, [T_FLAME] = SQUEAK_DARK_NEBULA_SHOT,
    [T_FIREBALL] = SQUEAK_DARK_NEBULA_FIREBALL, [T_BURST] = SQUEAK_DARK_NEBULA_FIRE, [T_PILLAR] = SQUEAK_DARK_NEBULA_ICE,
    [T_BOLT] = SQUEAK_DARK_NEBULA_BOLT, [T_CAGE] = SQUEAK_DARK_NEBULA_BOLT, [T_CAGE_SIDE] = SQUEAK_DARK_NEBULA_SHOT,
    [T_CAGE_FLOOR] = SQUEAK_DARK_NEBULA_SHOT, [T_CAGE_TOP] = SQUEAK_DARK_NEBULA_SHOT, [T_BURN] = SQUEAK_DARK_NEBULA_BURN,
};

#define TIMER(s) ((s)->vars[0])         // +0x208;
#define COUNT(s) ((s)->vars[1])         // +0x20C: shots, dashes, waves, laps left;
#define SIDE(s) ((s)->vars[2])          // +0x210: 0 right, 1 left (his dashes: the spot he's at);
#define ANGLE(s) ((s)->vars[3])         // +0x21C: his wave's phase, his orbit's angle (degrees, 20.12);
#define TURN(s) ((s)->vars[4])          // +0x214: their step a frame;
#define TURN_UP(s) ((s)->vars[5])       // +0x218: his orbit's speeding up;
#define HIGH(s) ((s)->vars[6])          // +0x220: his wave's height (half of it, pixels);
#define FORM(s) ((s)->vars[7])          // +0x224;
#define NEXT(s) ((s)->vars[8])          // +0x228;
#define DELAY(s) ((s)->vars[9])         // +0x22C;
#define PICKS(s) ((s)->vars[10])        // +0x230;
#define BLINK(s) ((s)->vars[11])        // +0x200;
#define FLOOR(s) ((s)->vars[12])        // The floor where he was placed (a8s4s6's);
#define LAST_HP(s) ((s)->vars[13])
#define HARM(s) ((s)->vars[14])
#define EYE(s) ((s)->vars[15])          // His eye (+0x1F8);
#define AURA(s) ((s)->vars[16])         // His flames (+0x1FC);
#define WHITE(s) ((s)->vars[17])        // The screen's whitening: frames left, | 0x10000 whitening (else clearing), its length >> 17;
#define ORB(s) ((s)->vars[18])          // His fireball, while it's out;
#define PREV_X(s) ((s)->vars[19])
#define BURNING(s) ((s)->vars[20])      // The floor's 16-pixel tiles burning (the DS view's, from its left: bit 0 its first);
#define BURN_VRAM(s) ((s)->vars[21])    // Their VRAM: one block for all, the same still picture (KatAM's 512 tiles: his fire wall's up);
#define DEFEAT(s) ((s)->vars[22])       // His defeat under way (DarkNebulaWatch);
#define LOCKED(s) ((s)->vars[23])       // The Kirbys whose controls he took (0x02012F94);
#define KEPT_FLAGS(s) ((s)->vars[24])   // His flags as his last frame left them (KatAM's mini-boss defeat changes them: DarkNebulaWatch);
#define KEPT_C(s) ((s)->vars[25])
#define KEPT_5C(s) ((s)->vars[26])
#define TINT(s) ((s)->vars[27])         // The far view's tint (0x02209F84, ShowTint()) and its colors' keeper (FarViewKept());
// (his defeat's own, in his fight's that it doesn't use: the far view's tint from, to, its frames gone and all, his twinkle);
#define SHADE_FROM(s) ((s)->vars[3])
#define SHADE_TO(s) ((s)->vars[4])
#define SHADE_GONE(s) ((s)->vars[5])
#define SHADE_LEN(s) ((s)->vars[6])
#define TWINKLE(s) ((s)->vars[18])

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

// FxMul(); Squeak Squad's 20.12 multiply, in 32-bit pieces (KatAM has no __muldi3), as Mecha-Kracko's;
static s32 FxMul(s32 a, s32 b) {
    bool32 minus = (a < 0) != (b < 0);
    u32 ua = a < 0 ? -(u32)a : (u32)a, ub = b < 0 ? -(u32)b : (u32)b;
    u32 al = ua & 0xFFFF, ah = ua >> 16, bl = ub & 0xFFFF, bh = ub >> 16;
    u32 lo = al * bl, mid1 = ah * bl, mid2 = al * bh, hi = ah * bh, t;

    t = lo + (mid1 << 16); hi += (mid1 >> 16) + (t < lo); lo = t;
    t = lo + (mid2 << 16); hi += (mid2 >> 16) + (t < lo); lo = t;
    t = lo + (minus ? 0x7FF : 0x800); hi += t < lo; lo = t;
    t = (hi << 20) | (lo >> 12);
    return minus ? -(s32)t : (s32)t;
}

// FxSqrt(); The square root of a 20.12 value;
static s32 FxSqrt(s32 v) {
    u64 n = (u64)(v < 0 ? 0 : v) << 12, r = 0, bit = (u64)1 << 62;

    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) {
            n -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (s32)r;
}

/* Cos(), Sin(); Of an angle in degrees (20.12), 20.12 (KatAM's gSineTable: 1024 a circle, 16384 one; Squeak Squad's float
 * cosine and sine, 0x02006AD0, 0x020071E0); */
static s32 Sin(s32 degrees) {
    s32 d = degrees % (360 << 12), i, f, a, b;

    if (d < 0)
        d += 360 << 12;
    // (360 degrees, 20.12, are the table's 1024 steps of 1440; between two, the line between them);
    i = d / 1440;
    f = d - i * 1440;
    a = gSineTable[i & 0x3FF];
    b = gSineTable[(i + 1) & 0x3FF];
    return (a + (b - a) * f / 1440) >> 2;
}

static s32 Cos(s32 degrees) {
    return Sin(degrees + (90 << 12));
}

static struct LevelInfo *Level(struct SqueakObject *s) {
    return &gCurLevelInfo[s->obj.base.unk56];
}

// Middle(); The camera view's middle across (KatAM's view, 240 across), the DS view's 128 from its left;
static s32 Middle(struct SqueakObject *s) {
    return Level(s)->viewportPosition.x + (DISPLAY_WIDTH / 2 << 8);
}

// SpotX(), SpotY(); A spot `a` pixels from the DS view's left, `b` under its top (KatAM's 24.8);
static s32 SpotX(struct SqueakObject *s, s32 a) {
    return Middle(s) + ((a - VIEW_HALF) << 8);
}

static s32 SpotY(struct SqueakObject *s, s32 b) {
    return FLOOR(s) - ((VIEW_TOP - b) << 8);
}

/* MoveFree(); His moves and his things' (Squeak Squad's flag 0x20000 and 0x100: not held in the stage, through everything): the
 * speed, then the spot. Squeak Squad's spots are 20.12, KatAM's 24.8: the last 4 bits are kept in fx and fy (which only
 * SqueakMove's friction would use), so slow speeds go as far as Squeak Squad's (0.8 a frame is 0.8, not 0.797); */
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    s32 dx, dy;

    s->vx += s->ax;
    s->vy += s->ay;
    dx = s->vx + s->fx;
    dy = s->vy + s->fy;
    base->xspeed = dx >> 4;
    base->yspeed = dy >> 4;
    s->fx = dx & 0xF;
    s->fy = dy & 0xF;
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

static void Stop(struct SqueakObject *s) {
    s->vx = s->vy = s->ax = s->ay = 0;
}

/* Aim(); The speed at `speed` from (x, y) to (tx, ty) (KatAM's 24.8), and the frames it takes (Squeak Squad's: the angle's cosine and
 * sine times the speed, the distance over the speed); */
static s32 Aim(s32 x, s32 y, s32 tx, s32 ty, s32 speed, s32 *vx, s32 *vy) {
    s32 dx = (tx - x) << 4, dy = (y - ty) << 4, d = FxSqrt(FxMul(dx, dx) + FxMul(dy, dy)), k;

    if (d == 0) {
        *vx = *vy = 0;
        return 0;
    }
    k = (speed << 12) / d;
    *vx = FxMul(dx, k);
    *vy = FxMul(dy, k);
    return d / speed;
}

// FlyTo(); 0x0211FC28 and the others: towards a spot at FLY_SPEED (the frames into TIMER);
static void FlyTo(struct SqueakObject *s, s32 a, s32 b) {
    TIMER(s) = Aim(s->obj.base.x, s->obj.base.y, SpotX(s, a), SpotY(s, b), FLY_SPEED, &s->vx, &s->vy);
    s->ax = s->ay = 0;
}

// Arrived(); Its frames gone by: there (snapped to it), still;
static bool32 Arrived(struct SqueakObject *s, s32 a, s32 b) {
    if (TIMER(s)-- > 0) {
        MoveFree(s);
        return FALSE;
    }
    Stop(s);
    s->obj.base.x = SpotX(s, a);
    s->obj.base.y = SpotY(s, b);
    return TRUE;
}

static bool32 Alive(struct SqueakObject *t, u8 enemy) {
    return t && t->obj.base.header.kind == 1 && !(t->obj.base.flags & 0x1000) && SqueakIsSqueak(&t->obj) && t->enemy == enemy;
}

// Thing(); What he brings in, dx ahead and dy up of him (Squeak Squad's units), facing right (as he does all his fight);
static struct SqueakObject *Thing(struct SqueakObject *s, u8 kind, s32 dx, s32 dy) {
    struct SqueakObject *t = SqueakSpawn(s, sThingEnemies[kind], kind, dx, dy);

    if (t == NULL) return NULL;
    t->obj.base.parent = &s->obj;
    t->obj.base.flags &= ~1;
    return t;
}

static void StarMain(struct Object *star);

/* Shoot(); 0x0212FF80's kind 0: a star at (vx, vy), dx ahead and dy up of him; KatAM's own (Dark Mind's shot, its objects 0xCB-0xCE:
 * fire, ice, spark, mixed; the owner: "massive lag when summoning stars. You can use the game's existing stars"), its kind at random
 * as Dark Mind's (the DS's in many colors too), moved by KatAM (its speed, through everything: flag 0x100), inhaled a star to spit back
 * as the DS's (no ability: Dark Mind's give theirs), its sound the DS's; its end StarMain(); */
static void Shoot(struct SqueakObject *s, s32 dx, s32 dy, s32 vx, s32 vy) {
    u8 kind = Random(4);
    s32 x = s->obj.base.x + SQUEAK_SPEED(s->obj.base.flags & 1 ? -dx : dx), y = s->obj.base.y - SQUEAK_SPEED(dy);
    struct Object *star;

    if (gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    star = CreateObjTemplateAndObj(s->obj.base.unk56, 1, 0x24, x >> 8, y >> 8, 0, 0x1F, 0, 0, OBJ_DARK_MIND_STAR_FIRE + kind, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (star == NULL) return;
    star->base.parent = &s->obj;
    star->base.x = x;
    star->base.y = y;
    ObjectSetFunc(star, 0, StarMain);
    star->unk83 += kind;
    star->base.xspeed = SQUEAK_SPEED(vx);
    star->base.yspeed = SQUEAK_SPEED(vy);
    star->base.flags |= 0x100 | 0x2000000;
    star->kirbyAbility = 0;
    SqueakSound(star, 0x1D2);
}

// ShootAt(); A star at `speed` towards (tx, ty);
static void ShootAt(struct SqueakObject *s, s32 tx, s32 ty, s32 speed) {
    s32 vx, vy;

    Aim(s->obj.base.x, s->obj.base.y, tx, ty, speed, &vx, &vy);
    Shoot(s, 0, 0, vx, vy);
}

/* Whiten(); 0x0203C174: the screen whitening (KatAM's brightness, 16 levels) over `frames` frames (clearing: from white); the "no
 * flashing lights" option's own limit on brightness applies (calm.c); */
static void Whiten(struct SqueakObject *s, bool32 whiten, s32 frames) {
    WHITE(s) = frames | (whiten ? 0x10000 : 0) | (frames << 17);
}

static bool32 Whitening(struct SqueakObject *s) {
    return (WHITE(s) & 0xFFFF) != 0;
}

static void ShowWhite(struct SqueakObject *s) {
    s32 left = WHITE(s) & 0xFFFF, length = (u32)WHITE(s) >> 17, level;

    if (length == 0) return;
    level = 16 * (length - left) / length;
    if (!(WHITE(s) & 0x10000)) level = 16 - level;
    if (level <= 0 && !(WHITE(s) & 0x10000) && left == 0) {
        gBldRegs.bldCnt = 0;
        gBldRegs.bldY = 0;
        WHITE(s) = 0;
        return;
    }
    gBldRegs.bldCnt = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
    gBldRegs.bldY = level;
    if (left > 0) WHITE(s)--;
}

/* EyeMode(); His eye's (0x0211C35C: +0x1F8's vtable +0x38): 0 still, 1 shooting, 2 half shut (ext's 18), 3 its twinkle (cmn's 5:
 * at its end his form's whitening), 4 its blink (ext's 17, then still). It sets the eye's state, which runs in the eye's update:
 * Squeak Squad runs that before his, so the eye takes it the next frame (EyeFrame); */
enum { MODE_STILL, MODE_SHOOT, MODE_HALF, MODE_TWINKLE, MODE_BLINK, MODE_SHAKE };
#define E_MODE(e) ((e)->vars[0])
#define E_NEXT(e) ((e)->vars[1])        // The mode it takes in its next update, + 1 (0: none);
#define E_TWINKLE(e) ((e)->vars[2])     // Its twinkle (cmn's 5: here a sprite of its own over the hidden eye);
#define E_SHAKE(e) ((e)->vars[3])       // Its shaking's step (mode 6, 0x0211BAB0: his defeat);

static void EyeMode(struct SqueakObject *s, u8 mode) {
    struct SqueakObject *e = (struct SqueakObject *)EYE(s);

    if (Alive(e, SQUEAK_DARK_NEBULA_EYE)) E_NEXT(e) = mode + 1;
}

// Companions(); His eye and his flames (0x0211C4E0, 0x0211B98C: made with him), made once his fight starts, again if lost;
static void Companions(struct SqueakObject *s) {
    if (!Alive((struct SqueakObject *)EYE(s), SQUEAK_DARK_NEBULA_EYE)) EYE(s) = (s32)Thing(s, T_EYE, 0, 0);
    if (!Alive((struct SqueakObject *)AURA(s), SQUEAK_DARK_NEBULA_AURA)) AURA(s) = (s32)Thing(s, T_AURA, 0, 0);
}

/* PlayForm(); One of his forms' animations (ice, fire, spark: their palettes his .cla's 1, 3 and 2), its palette into the slot of
 * another form's if one's loaded: one form shows at a time, and KatAM's 7 enemy palette slots are full in his fight otherwise (his
 * black star's, his attacks' and stars', the burning floor's, KatAM's Dark Mind stars'), the last loaded over (his fire wall drawn in
 * the burning floor's colors); */
static void PlayForm(struct SqueakObject *s, u8 anim) {
    static const u8 sForms[3] = { ANIM_ICE, ANIM_FIRE, ANIM_SPARK };
    const struct SqueakPaletteKey *k = gSqueakData.enemies[s->enemy].palette;
    u8 i;

    for (i = 0; i < 3 && k != NULL; i++)
        if (sForms[i] != anim && SqueakTakePalette(k[anim].key, k[anim].variant, k[sForms[i]].key, s->obj.base.roomId) != 0xFF) break;
    SqueakPlay(s, anim);
}

static u8 FormAnim(s32 form) {
    switch (form) {
    case FORM_FIRE: return ANIM_FIRE;
    case FORM_SPARK: return ANIM_SPARK;
    case FORM_ICE: return ANIM_ICE;
    default: return ANIM_BLACK;
    }
}

// Pick(); 0x02121A64: his move by his form (PL[3]-PL[11]); a shared one (0x021204DC: rand >> 30, a quarter each);
static void Pick(struct SqueakObject *s) {
    static const u8 sOwn[4][3] = {
        [FORM_FIRE] = { 35, 40, 25 }, [FORM_SPARK] = { 30, 30, 40 }, [FORM_ICE] = { 30, 30, 40 },
    };
    static const u8 sOwnA[4] = { [FORM_FIRE] = DN_DASHES, [FORM_SPARK] = DN_WAVE, [FORM_ICE] = DN_ORBIT };
    static const u8 sOwnB[4] = { [FORM_FIRE] = DN_ORB_FLY, [FORM_SPARK] = DN_BOLTS_FLY, [FORM_ICE] = DN_PILLAR_FLY };
    static const u8 sShared[4] = { DN_ROW_FLY, DN_RAIN_FLY, DN_FOUR_FLY, DN_DROP_FLY };
    u32 r = Random(100);
    s32 f = FORM(s);

    if (f < FORM_FIRE || f > FORM_ICE) {
        SqueakSetState(s, DN_END);
        return;
    }
    if (r < sOwn[f][0]) SqueakSetState(s, sOwnA[f]);
    else if (r < sOwn[f][0] + sOwn[f][1]) SqueakSetState(s, sShared[Random(4)]);
    else if (r < sOwn[f][0] + sOwn[f][1] + sOwn[f][2]) SqueakSetState(s, sOwnB[f]);
    else SqueakSetState(s, DN_IDLE);
}

// Shots(); 0x02120220, 0x0211FC28: how many stars, of 100 (PL[22]-PL[25] or PL[26]-PL[29]), less one (his last is the count's 0);
static s32 Shots(const u8 *w) {
    u32 r = Random(100);

    if (r < w[0]) return 3;
    if (r < w[0] + w[1]) return 4;
    if (r < w[0] + w[1] + w[2]) return 5;
    if (r < w[0] + w[1] + w[2] + w[3]) return 8;
    return 0;
}

static void DashTo(struct SqueakObject *s) {
    TIMER(s) = Aim(s->obj.base.x, s->obj.base.y, SpotX(s, sDashSpots[SIDE(s)][0]), SpotY(s, sDashSpots[SIDE(s)][1]), DASH_SPEED,
                   &s->vx, &s->vy) + 1;
}

// OtherSpot(); 0x021215B8: one of the other six;
static void OtherSpot(struct SqueakObject *s) {
    s32 r = Random(6);

    SIDE(s) = SIDE(s) <= r ? r + 1 : r;
}

/* WaveMove(); 0x021212DC and the others: across at 1.0, turned past the view's sides; his height to his wave's (8 under the floor and
 * up to 2 HIGH more: the view's bottom + 40 + HIGH(1 + cos)), at most WAVE_STEP a frame; */
static void WaveMove(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    s32 y, h;

    // (the DS view's sides: 8 pixels past KatAM's narrower view's);
    if (base->x < SpotX(s, 0)) s->vx = WAVE_VX;
    else if (base->x > SpotX(s, 2 * VIEW_HALF)) s->vx = -WAVE_VX;
    MoveFree(s);
    h = HIGH(s) * 4096 + FxMul(HIGH(s) * 4096, Cos(ANGLE(s)));
    y = FLOOR(s) - ((WAVE_UP - 48) << 8) - (h >> 4);
    if (y < base->y - (WAVE_STEP >> 4)) y = base->y - (WAVE_STEP >> 4);
    else if (y > base->y + (WAVE_STEP >> 4)) y = base->y + (WAVE_STEP >> 4);
    base->y = y;
}

// OrbitAt(); 0x02120A04: his spot on the ellipse at his angle, his step at most ORBIT_STEP a frame;
static void OrbitAt(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    s32 x = SpotX(s, ORBIT_X) + (FxMul(ORBIT_RX << 12, Cos(ANGLE(s))) >> 4);
    s32 y = SpotY(s, ORBIT_Y) - (FxMul(ORBIT_RY << 12, Sin(ANGLE(s))) >> 4);

    if (x > base->x + (ORBIT_STEP >> 4)) x = base->x + (ORBIT_STEP >> 4);
    else if (x < base->x - (ORBIT_STEP >> 4)) x = base->x - (ORBIT_STEP >> 4);
    if (y > base->y + (ORBIT_STEP >> 4)) y = base->y + (ORBIT_STEP >> 4);
    else if (y < base->y - (ORBIT_STEP >> 4)) y = base->y - (ORBIT_STEP >> 4);
    base->x = x;
    base->y = y;
}

static s32 Wrap360(s32 a) {
    if (a < 0) return a + (360 << 12);
    if (a >= 360 << 12) return a - (360 << 12);
    return a;
}

// Thing kinds' ends with him (0x0212FB94: all but the fire's heat gone when his HP is out);
static void EndThings(struct SqueakObject *s) {
    ORB(s) = 0;
}

static void Gone(struct SqueakObject *t);
static void DarkNebulaMain(struct Object *obj);

// Lock(); 0x02012F94: every Kirby in his room (and up) without controls (the cutscenes' flag, unless one already has it);
static void Lock(struct SqueakObject *s) {
    u8 i;

    LOCKED(s) = 0;
    for (i = 0; i < 4; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != s->obj.base.roomId || kirby->hp <= 0 || (kirby->base.flags & 0x1000000)) continue;
        kirby->base.flags |= 0x1000000;
        LOCKED(s) |= 1 << i;
    }
}

// Unlock(); 0x02012F68: the controls he took given back;
static void Unlock(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < 4; i++)
        if (LOCKED(s) >> i & 1) gKirbys[i].base.flags &= ~0x1000000;
    LOCKED(s) = 0;
}

/* Shade(); The stage's background tint (0x02209F84: +0x1C its frames gone, +0x20 all of them, +0x30-0x38 from, +0x3C-0x44 to, 20.12
 * times its colors) in his defeat, one level for its three colors: from where it is to `to` over `frames` frames (his defeat starts it
 * at SHADE_FULL: DefeatStart()); in his fight, his form's tint (ShowTint()); */
static s32 ShadeNow(struct SqueakObject *s) {
    if (SHADE_LEN(s) <= 0) return SHADE_TO(s);
    return SHADE_FROM(s) + (SHADE_TO(s) - SHADE_FROM(s)) * SHADE_GONE(s) / SHADE_LEN(s);
}

static void Shade(struct SqueakObject *s, s32 to, s32 frames) {
    SHADE_FROM(s) = ShadeNow(s);
    SHADE_TO(s) = to;
    SHADE_GONE(s) = 0;
    SHADE_LEN(s) = frames;
}

/* ShowShade(); The far view (KatAM's BG palettes 6-13 in Squeak Squad's rooms: KatAMExpansionSqueakRooms.cs's SqueakBackground) its
 * colors as loaded times the tint, each frame it goes and once more at its end (in KatAM's own rooms, whose palettes are laid out
 * otherwise, none: their foreground's would darken); */
static void FarViewTimes(struct SqueakObject *s, s32 r, s32 g, s32 b);

static void ShowShade(struct SqueakObject *s) {
    s32 level;

    if (SHADE_LEN(s) <= 0) return;
    if (SHADE_GONE(s) < SHADE_LEN(s)) SHADE_GONE(s)++;
    level = ShadeNow(s);
    if (s->obj.base.roomId >= SQUEAK_FIRST_ROOM) FarViewTimes(s, level, level, level);
    // (back to its own colors: done);
    if (SHADE_GONE(s) >= SHADE_LEN(s) && level == SHADE_FULL) SHADE_LEN(s) = 0;
}

/* FarViewKept(); The far view's colors as loaded, kept while his tint or shade is on it (a task of his: SQUEAK_OBJECT_SIZE has no room;
 * its address in TINT's top half), as squeak/gimmicks.c's Light() keeps them: the colors shown go into those KatAM's palette effects
 * put back too (gUnk_02022120: the copy ability's flash, Kirby's sub_0805C11C and sub_0803CBC4, ends so: sub_0803A1F4), which then
 * keep them. Gone with him, the far view its own colors again (DarkNebulaGone()); a task freed with the room before him (its own
 * check: its main and its owner) none, `make` FALSE none made; */
struct FarView {
    struct SqueakObject *owner;
    u16 colors[FAR_VIEW_COLORS];
};

static void FarViewMain(void) {}

static struct Task *FarViewTask(struct SqueakObject *s) {
    struct Task *t = (struct Task *)(0x03000000 | (u32)TINT(s) >> 16);

    if (!((u32)TINT(s) >> 16) || t->main != FarViewMain || ((struct FarView *)TaskGetStructPtr(t))->owner != s) return NULL;
    return t;
}

static u16 *FarViewKept(struct SqueakObject *s, bool32 make) {
    struct Task *t = FarViewTask(s);
    struct FarView *f;
    u16 i;

    if (t != NULL) return ((struct FarView *)TaskGetStructPtr(t))->colors;
    if (!make) return NULL;
    t = TaskCreate(FarViewMain, sizeof(struct FarView), 0x1000, TASK_USE_EWRAM, NULL);
    if (t->main != FarViewMain) return NULL;    // (none free: KatAM's empty task);
    f = TaskGetStructPtr(t);
    f->owner = s;
    for (i = 0; i < FAR_VIEW_COLORS; i++) f->colors[i] = gUnk_02022120[FAR_VIEW_FIRST + i];
    TINT(s) = (TINT(s) & 0xFFFF) | ((u32)t & 0xFFFF) << 16;
    return f->colors;
}

/* FarViewTimes(); The far view (KatAM's BG palettes 6-13 in Squeak Squad's rooms) its colors as loaded (FarViewKept()), each of red,
 * green and blue times its own level (20.12); none in KatAM's own rooms (their palettes are laid out otherwise); */
static void FarViewTimes(struct SqueakObject *s, s32 r, s32 g, s32 b) {
    u16 i, *kept;

    if (s->obj.base.roomId < SQUEAK_FIRST_ROOM || (kept = FarViewKept(s, TRUE)) == NULL) return;
    for (i = 0; i < FAR_VIEW_COLORS; i++) {
        u16 c = kept[i];

        gBgPalette[FAR_VIEW_FIRST + i] = gUnk_02022120[FAR_VIEW_FIRST + i] =
            ((c & 31) * r >> 12) | ((c >> 5 & 31) * g >> 12) << 5 | ((c >> 10 & 31) * b >> 12) << 10;
    }
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE;
}

/* TintStart(), ShowTint(); His form's tint of the stage's background (0x021225B4 at his fight's start, 0x021222E4 at each form he
 * picks: 0x02209F84's +0x3C-0x44, the colors it goes to over ML[7] frames, TINT_TIME, from those it's at): his form's own color
 * (fire red, spark green, ice blue) times SHADE_FULL, the other two times TINT_HALF; no form, all three SHADE_FULL. Each frame of it
 * the far view's colors (FarViewTimes()), then none till the next (they keep it: FarViewKept()); TINT: its frames gone, the form it's
 * from << 8, to << 12, FarViewKept()'s task << 16; */
static s32 TintLevel(u8 form, u8 color) {
    return form == FORM_NONE || form == color + FORM_FIRE ? SHADE_FULL : TINT_HALF;
}

static void TintStart(struct SqueakObject *s, u8 from, u8 to) {
    TINT(s) = (TINT(s) & ~0xFFFF) | from << 8 | to << 12;
}

static void TintOff(struct SqueakObject *s) {
    TINT(s) = (TINT(s) & ~0xFFFF) | (TINT_TIME + 1);
}

static void ShowTint(struct SqueakObject *s) {
    s32 gone = TINT(s) & 0xFF, level[3];
    u8 from = TINT(s) >> 8 & 15, to = TINT(s) >> 12 & 15, i;

    if (gone > TINT_TIME) return;
    for (i = 0; i < 3; i++) level[i] = TintLevel(from, i) + (TintLevel(to, i) - TintLevel(from, i)) * gone / TINT_TIME;
    TINT(s)++;
    FarViewTimes(s, level[0], level[1], level[2]);
}

/* DefeatSpark(); 0x0211CBAA: a spark at him, thrown at `speed` rand(180)° up (0x0211C878), falling GRAVITY a frame, bouncing once under
 * the view's SPARK_FLOOR, its fall up SPARK_GAIN times (sound 0x7D); his sound 0x1D8; */
static void DefeatSpark(struct SqueakObject *s, s32 speed) {
    s32 a = Random(180) << 12;

    SqueakFallInRoom(s->obj.base.roomId, SQUEAK_FX_DARK_SPARK, s->obj.base.x, s->obj.base.y, FxMul(speed, Cos(a)), FxMul(speed, Sin(a)),
                     -GRAVITY, SpotY(s, SPARK_FLOOR), SPARK_BOUNCES, SPARK_GAIN, 0x7D);
    SqueakSfx(s, 0x1D8);
}

// AnyForm(); 0x0211DCB8's pick of his forms (rand % 3): 0 fire (cmn's 7), 1 spark (8), 2 ice (6);
static void AnyForm(struct SqueakObject *s) {
    static const u8 sForms[3] = { ANIM_FIRE, ANIM_SPARK, ANIM_ICE };

    PlayForm(s, sForms[Random(3)]);
}

/* Flicker(); 0x0211DCB8, 0x0211D9C4, 0x0211D6D0: every `every` frames (COUNT) one of his forms, or (`black`: SIDE counting down from
 * it) after `black` of them his black star (none: forms only); */
static void Flicker(struct SqueakObject *s, s32 every, s32 black) {
    if (COUNT(s)-- >= 1) return;
    COUNT(s) = every;
    if (black == 0) {
        AnyForm(s);
    } else if (SIDE(s) < 1) {
        SIDE(s) = black;
        SqueakPlay(s, ANIM_BLACK);
    } else {
        SIDE(s)--;
        AnyForm(s);
    }
}

/* DarkNebula(); His states. As Squeak Squad's, a state changed runs at once (its entry and its first frame) unless `chain` is cleared:
 * where Squeak Squad's returns 0 after changing it (his moves' ends into 0x021224F0, the fireball's end), the next frame; */
static bool32 DarkNebula(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    bool32 chain = TRUE;
    s32 x, y, a;
    u8 i;

    switch (s->state) {
    // His coming (0x46);
    case DN_COME:               // 0x0213756C;
        if (T(s) == 0) {
            SqueakPlay(s, ANIM_STAR);
            SqueakLoopSfx(s, 0x1C7);
            base->x = SpotX(s, COME_X) - (COME_TIME * COME_SPEED >> 4);
            base->y = SpotY(s, COME_Y);
            base->flags &= ~0x400;
            s->vx = COME_SPEED;
            s->vy = 0;
        }
        MoveFree(s);
        if (T(s) + 1 >= COME_TIME) {
            Stop(s);
            SqueakSetState(s, DN_COME_STOP);
        }
        break;
    case DN_COME_STOP:          // 0x021374CC, 0x02137430;
        if (!s->animEnded) break;
        COUNT(s) = 3;
        SqueakSetState(s, DN_COME_SPIN);
        break;
    case DN_COME_SPIN:          // 0x021373D4;
        if (s->animEnded && --COUNT(s) < 1) SqueakSetState(s, DN_COME_FORM);
        break;
    case DN_COME_FORM:          // 0x02137338;
        if (T(s) == 0) SqueakPlay(s, ANIM_FORMING);
        else if (s->animEnded) SqueakSetState(s, DN_COME_OPEN);
        break;
    case DN_COME_OPEN:          // 0x02137264;
        if (T(s) == 0) {
            SqueakPlayComposite(s, C_OPEN);
            SqueakStopSfx(s, 0x1C7);
            SqueakSfx(s, 0x1CA);
        }
        else if (s->animEnded) {
            COUNT(s) = 3;
            SqueakSetState(s, DN_COME_LOOK);
        }
        break;
    case DN_COME_LOOK:          // 0x02137188, 0x021370B4;
        if (T(s) == 0) {
            SqueakPlayComposite(s, C_LOOK);
            SqueakSfx(s, 0x1CA);
        }
        else if (s->animEnded && --COUNT(s) < 1) SqueakSetState(s, DN_COME_GLINT);
        break;
    case DN_COME_GLINT:         // 0x021370B4, 0x02137054;
        if (T(s) == 0) {
            SqueakPlayComposite(s, C_GLINT);
            SqueakSfx(s, 0x1CB);
        } else if (T(s) == 32) {
            SqueakSfx(s, 0x1CC);
        } else if (T(s) >= 32 + GLINT_WAIT) {
            SqueakSetState(s, DN_START);
        }
        break;
    // His fight (0x3F);
    case DN_START:              // 0x021226E8;
        if (T(s) == 0) {
            SqueakPlay(s, ANIM_BLACK);
            Stop(s);
            base->x = SpotX(s, COME_X);
            base->y = SpotY(s, COME_Y);
            FORM(s) = NEXT(s) = FORM_FIRE + Random(3);
            Companions(s);
        }
        /* (0x021225B4: ML[6] frames, then a shake and his eye's twinkle, his HP bar on the HUD (0x020B308C: filling, sound 0x177; KatAM's
         * bar fills as it shows, DarkNebulaMain), the far view tinted toward his form's color (ShowTint)); */
        if (T(s) > START_WAIT) {
            struct Object5 *bar;

            RequestScreenShake(1, base);
            EyeMode(s, MODE_TWINKLE);
            SqueakSfx(s, 0x1CD);
            TintStart(s, FORM_NONE, FORM(s));
            if ((bar = sub_08034E14(&s->obj)) != NULL) bar->unk9 = 0;
            SqueakSfx(s, 0x177);
            SqueakSetState(s, DN_TWINKLE);
        }
        break;
    case DN_TWINKLE:            // 0x021225A4, 0x02122154: his eye's twinkle's end starts his whitening (0x0211BCCC);
        if (T(s) > 60) SqueakSetState(s, DN_WHITEN);   // (no eye to twinkle: KatAM couldn't make it);
        break;
    case DN_WHITEN:             // 0x021220D0, 0x02122038;
        if (T(s) == 0) {
            Whiten(s, TRUE, WHITEN_TIME);
            SqueakSfx(s, 0x1CE);
        }
        else if (!Whitening(s)) SqueakSetState(s, DN_CLEAR);
        break;
    case DN_CLEAR:              // 0x02121E4C, 0x02121DE4;
        if (T(s) == 0) {
            if (FORM(s) == FORM_NONE) SqueakPlay(s, ANIM_BLACK);
            else PlayForm(s, FormAnim(FORM(s)));
            Whiten(s, FALSE, CLEAR_TIME);
        } else if (!Whitening(s)) {
            SqueakSetState(s, DN_IDLE);
        }
        break;
    case DN_IDLE:               // 0x02121CB8;
        if (T(s) == 0) {
            Stop(s);
            EyeMode(s, MODE_HALF);
        }
        if (T(s) >= IDLE_WAIT) SqueakSetState(s, DN_PICK);
        break;
    case DN_PICK:
        Pick(s);
        break;
    case DN_END:                // 0x021224F0, 0x021222E4;
        if (PICKS(s)-- < 0) {
            PICKS(s) = 3 + Random(3);
            if (FORM(s) >= FORM_FIRE && FORM(s) <= FORM_ICE) {
                a = Random(2);
                NEXT(s) = FORM(s) == FORM_FIRE ? (a ? FORM_ICE : FORM_SPARK) : FORM(s) == FORM_SPARK ? (a ? FORM_ICE : FORM_FIRE)
                        : (a ? FORM_SPARK : FORM_FIRE);
            } else {
                NEXT(s) = FORM_FIRE + Random(3);
            }
            DELAY(s) = FORM_DELAY;
            // (0x021222E4: the far view tinted toward the next form's color, its 240 frames' first ML[7]);
            TintStart(s, FORM(s), NEXT(s));
        }
        SqueakSetState(s, DN_IDLE);
        break;
    case DN_SWOOP:              // 0x0212222C, 0x021221C8;
        if (T(s) == 0) {
            EyeMode(s, MODE_STILL);
            Stop(s);
            s->vx = ((SpotX(s, SWOOP_X) - base->x) << 4) / SWOOP_TIME;
            s->vy = ((base->y - SpotY(s, SWOOP_Y)) << 4) / SWOOP_TIME + SWOOP_VY;
            s->ay = SWOOP_AY;
        }
        if (T(s) >= SWOOP_TIME) {
            Stop(s);
            SqueakSetState(s, DN_SWOOP_WAIT);
            break;
        }
        MoveFree(s);
        break;
    case DN_SWOOP_WAIT:         // 0x02122164;
        if (T(s) >= SWOOP_WAIT) {
            EyeMode(s, MODE_TWINKLE);
            SqueakSetState(s, DN_TWINKLE);
        }
        break;
    // Fire: his dashes (0x02121994, 0x021216DC, 0x02121648, 0x021215B8);
    case DN_DASHES:
        COUNT(s) = 3 + Random(3);
        SIDE(s) = Random(7);
        EyeMode(s, MODE_SHOOT);
        SqueakSfx(s, 0x1CF);
        SqueakSetState(s, DN_DASH);
        break;
    case DN_DASH:
        if (T(s) == 0) {
            DashTo(s);
            while (TIMER(s) <= 2) {
                OtherSpot(s);
                DashTo(s);
            }
        }
        if (TIMER(s)-- > 0) {
            MoveFree(s);
            break;
        }
        if (--COUNT(s) < 0) {
            SqueakSetState(s, DN_IDLE);
            break;
        }
        Stop(s);
        SqueakSetState(s, DN_DASH_PAUSE);
        break;
    case DN_DASH_PAUSE:
        if (T(s) >= DASH_PAUSE) {
            OtherSpot(s);
            SqueakSetState(s, DN_DASH);
        }
        break;
    // Fire: his fireball (0x0211EBE4, 0x0211EB2C, 0x0211EAC0, 0x0211EAB0);
    case DN_ORB_FLY:
        // (his own moves but his dashes: none while a form waits);
        if (T(s) == 0 && FORM(s) != NEXT(s)) {
            SqueakSetState(s, DN_IDLE);
            break;
        }
        if (T(s) == 0) FlyTo(s, ORB_X, ORB_Y);
        if (Arrived(s, ORB_X, ORB_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_ORB_AIM);
        }
        break;
    case DN_ORB_AIM:
        if (T(s) >= AIM_WAIT) {
            ORB(s) = (s32)Thing(s, T_FIREBALL, 0, 0);
            SqueakSfx(s, 0x1D3);
            SqueakSetState(s, DN_ORB);
        }
        break;
    case DN_ORB:                // (0x0211EAB0: till the fireball's end, DarkNebulaMain);
        break;
    case DN_ORB_DONE:           // 0x0211EA7C;
        SqueakSetState(s, DN_END);
        chain = FALSE;
        break;
    // Spark: his wave (0x02121480, 0x021212DC, 0x0212120C, 0x02120FF4, 0x02120E50);
    case DN_WAVE:               // 0x02121480;
        COUNT(s) = 2 + Random(2);
        Stop(s);
        s->vx = Random(2) ? -WAVE_VX : WAVE_VX;
        TURN(s) = WAVE_TURN;
        ANGLE(s) = 0;
        HIGH(s) = WAVE_HIGH;
        EyeMode(s, MODE_SHOOT);
        SqueakSfx(s, 0x1D1);
        SqueakSetState(s, DN_WAVE_RISE);
        break;
    case DN_WAVE_RISE:          // 0x021212DC: to 180°, then (0x0212120C) his next wave's height;
        ANGLE(s) += TURN(s);
        if (ANGLE(s) >= 180 << 12) {
            HIGH(s) = WAVE_LOW;
            SqueakSetState(s, DN_WAVE_ON);
            break;
        }
        WaveMove(s);
        break;
    case DN_WAVE_ON:            // 0x02120FF4: a wave each 360° (at 540°), his last back to 180° under 0;
        if (COUNT(s) < 1) {
            ANGLE(s) -= 360 << 12;
            HIGH(s) = WAVE_HIGH;
            SqueakSetState(s, DN_WAVE_LAST);
            break;
        }
        ANGLE(s) += TURN(s);
        if (ANGLE(s) >= 540 << 12) {
            ANGLE(s) -= 360 << 12;
            COUNT(s)--;
            HIGH(s) = WAVE_LOW;
            SqueakSetState(s, DN_WAVE_ON);
            break;
        }
        WaveMove(s);
        break;
    case DN_WAVE_LAST:          // 0x02120E50: to 360°;
        ANGLE(s) += TURN(s);
        if (ANGLE(s) >= 360 << 12) {
            SqueakSetState(s, DN_IDLE);
            break;
        }
        WaveMove(s);
        break;
    // Spark: his bolts (0x0211E850, 0x0211E798, 0x0211E5F4, 0x0211E56C, 0x0211E51C);
    case DN_BOLTS_FLY:
        if (T(s) == 0 && FORM(s) != NEXT(s)) {
            SqueakSetState(s, DN_IDLE);
            break;
        }
        if (T(s) == 0) FlyTo(s, BOLTS_X, BOLTS_Y);
        if (Arrived(s, BOLTS_X, BOLTS_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_BOLTS_AIM);
        }
        break;
    case DN_BOLTS_AIM:
        if (T(s) == 0) SqueakSfx(s, 0x1D6);
        if (T(s) < AIM_WAIT) break;
        SqueakSfx(s, 0x1D5);
        for (i = 0; i < 4; i++) {
            struct SqueakObject *t = Thing(s, T_BOLT, 0, 0);

            if (t == NULL) continue;
            x = SpotX(s, i & 1 ? 2 * VIEW_HALF : 0);
            y = SpotY(s, i & 2 ? CAGE_Y : 0);
            t->vx = ((x - base->x) << 4) / BOLT_TIME;
            t->vy = ((base->y - y) << 4) / BOLT_TIME;
            SqueakPlay(t, t->vy < 0 ? (t->vx < 0 ? BOLT_DOWN_LEFT : BOLT_DOWN_RIGHT) : (t->vx < 0 ? BOLT_UP_LEFT : BOLT_UP_RIGHT));
        }
        Stop(s);
        SqueakSetState(s, DN_BOLTS);
        break;
    case DN_BOLTS:
        if (T(s) < BOLT_TIME) break;
        {
            struct SqueakObject *t = Thing(s, T_CAGE, 0, 0);

            if (t) {
                t->obj.base.x = SpotX(s, CAGE_X);
                t->obj.base.y = SpotY(s, CAGE_Y);
            }
        }
        SqueakSetState(s, DN_BOLTS_AFTER);
        break;
    case DN_BOLTS_AFTER:
        if (T(s) >= CAGE_AFTER) SqueakSetState(s, DN_END);
        break;
    // Ice: his orbit (0x02120C54, 0x02120A04, 0x02120798, 0x021205CC);
    case DN_ORBIT:
        if (T(s) == 0) {
            COUNT(s) = 1 + Random(2);
            SqueakSfx(s, 0x1D0);
            SIDE(s) = 0;
            PREV_X(s) = Random(2);
            x = (base->x - SpotX(s, ORBIT_X)) / ORBIT_RX;
            y = (SpotY(s, ORBIT_Y) - base->y) / ORBIT_RY;
            // (0x0200741C's angle, degrees: KatAM's ArcTan2, 0-0xFFFF a circle);
            ANGLE(s) = (s32)((u32)ArcTan2(x, y) * 360 >> 4);
            Stop(s);
            TURN(s) = 0;
            TURN_UP(s) = Random(2) ? -ORBIT_ACCEL : ORBIT_ACCEL;
            EyeMode(s, MODE_SHOOT);
        }
        if (SIDE(s) == 0) {
            TURN(s) += TURN_UP(s);
            if (TURN(s) >= ORBIT_MAX || TURN(s) <= -ORBIT_MAX) {
                TURN(s) = TURN(s) < 0 ? -ORBIT_MAX : ORBIT_MAX;
                SIDE(s) = 1;
            }
            ANGLE(s) = Wrap360(ANGLE(s) + TURN(s));
        } else {
            s32 lap = (PREV_X(s) ? ORBIT_LAP_LEFT : ORBIT_LAP_RIGHT) << 12, from = ANGLE(s), to = from + TURN(s);

            if ((from - lap < 0) != (to - lap < 0) || to == lap) {
                if (--COUNT(s) < 1) {
                    ANGLE(s) = lap;
                    OrbitAt(s);
                    SqueakSetState(s, DN_IDLE);
                    break;
                }
            }
            ANGLE(s) = Wrap360(to);
        }
        OrbitAt(s);
        break;
    // Ice: his pillar (0x0211E290, 0x0211E1A8, 0x0211E094, 0x0211DFE4);
    case DN_PILLAR_FLY:
        if (T(s) == 0 && FORM(s) != NEXT(s)) {
            SqueakSetState(s, DN_IDLE);
            break;
        }
        if (T(s) == 0) {
            SIDE(s) = Random(2);
            FlyTo(s, SIDE(s) ? PILLAR_LEFT_X : PILLAR_RIGHT_X, PILLAR_Y);
        }
        if (Arrived(s, SIDE(s) ? PILLAR_LEFT_X : PILLAR_RIGHT_X, PILLAR_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_PILLAR_AIM);
        }
        break;
    case DN_PILLAR_AIM:
        if (T(s) < AIM_WAIT) break;
        s->vx = SIDE(s) ? PILLAR_VX : -PILLAR_VX;
        {
            struct SqueakObject *t = Thing(s, T_PILLAR, 0, 0);

            SqueakSfx(s, 0x1D7);
            if (t) {
                t->obj.base.y = SpotY(s, PILLAR_AT);
                t->vx = s->vx;
                t->vars[1] = SIDE(s);
            }
        }
        SqueakSetState(s, DN_PILLAR);
        break;
    case DN_PILLAR:             // (past the other side: the end, his move on before);
        if (SIDE(s) ? base->x > SpotX(s, PILLAR_RIGHT_X) : base->x < SpotX(s, PILLAR_LEFT_X)) {
            SqueakSetState(s, DN_END);
            chain = FALSE;
            break;
        }
        MoveFree(s);
        break;
    // Shared: a row of stars (0x02120220, 0x02120150, 0x02120100, 0x0211FF3C, 0x0211FEEC);
    case DN_ROW_FLY:
        if (T(s) == 0) {
            static const u8 sRow[4] = { 94, 1, 4, 1 };

            SIDE(s) = Random(2);
            COUNT(s) = Shots(sRow);
            FlyTo(s, SIDE(s) ? ROW_LEFT_X : ROW_RIGHT_X, ROW_Y);
        }
        if (Arrived(s, SIDE(s) ? ROW_LEFT_X : ROW_RIGHT_X, ROW_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_ROW_AIM);
        }
        break;
    case DN_ROW_AIM:
        if (T(s) >= AIM_WAIT) SqueakSetState(s, DN_ROW);
        break;
    case DN_ROW:
        if (T(s) == 0) {
            a = Random(360) << 12;
            Shoot(s, FxMul(ROW_ROUND << 12, Cos(a)), FxMul(ROW_ROUND << 12, Sin(a)), SIDE(s) ? STAR_SPEED : -STAR_SPEED, 0);
            if (COUNT(s)-- < 1) {
                SqueakSetState(s, DN_END);
                chain = FALSE;
                break;
            }
        }
        if (T(s) >= ROW_GAP) SqueakSetState(s, DN_ROW);
        break;
    // Shared: a rain (0x0211FC28, 0x0211FB50, 0x0211FB00, 0x0211F86C, 0x0211F81C);
    case DN_RAIN_FLY:
        if (T(s) == 0) {
            static const u8 sRain[4] = { 94, 4, 1, 1 };

            SIDE(s) = Random(2);
            COUNT(s) = Shots(sRain);
            FlyTo(s, SIDE(s) ? RAIN_LEFT_X : RAIN_RIGHT_X, RAIN_Y);
        }
        if (Arrived(s, SIDE(s) ? RAIN_LEFT_X : RAIN_RIGHT_X, RAIN_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_RAIN_AIM);
        }
        break;
    case DN_RAIN_AIM:
        if (T(s) >= AIM_WAIT) SqueakSetState(s, DN_RAIN);
        break;
    case DN_RAIN:
        if (T(s) == 0) {
            a = Random(360) << 12;
            ShootAt(s, s->obj.kirby3->base.x + (FxMul(RAIN_ROUND << 12, Cos(a)) >> 4), SpotY(s, RAIN_AT), STAR_SPEED);
            if (COUNT(s)-- < 1) {
                SqueakSetState(s, DN_END);
                chain = FALSE;
                break;
            }
        }
        if (T(s) >= RAIN_GAP) SqueakSetState(s, DN_RAIN);
        break;
    // Shared: four (0x0211F628, 0x0211F578, 0x0211F528, 0x0211F340);
    case DN_FOUR_FLY:
        if (T(s) == 0) FlyTo(s, FOUR_X, FOUR_Y);
        if (Arrived(s, FOUR_X, FOUR_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_FOUR_AIM);
        }
        break;
    case DN_FOUR_AIM:
        if (T(s) < AIM_WAIT) break;
        for (i = 0; i < 4; i++) ShootAt(s, SpotX(s, sFourAt[i]), SpotY(s, FOUR_AT), FOUR_SPEED);
        SqueakSetState(s, DN_END);
        chain = FALSE;
        break;
    // Shared: a drop (0x0211F0EC, 0x0211F00C, 0x0211EF88, 0x0211EE10);
    case DN_DROP_FLY:
        if (T(s) == 0) {
            SIDE(s) = Random(2);
            FlyTo(s, SIDE(s) ? DROP_LEFT_X : DROP_RIGHT_X, DROP_Y);
        }
        if (Arrived(s, SIDE(s) ? DROP_LEFT_X : DROP_RIGHT_X, DROP_Y)) {
            EyeMode(s, MODE_BLINK);
            SqueakSetState(s, DN_DROP_AIM);
        }
        break;
    case DN_DROP_AIM:
        if (T(s) < AIM_WAIT) break;
        s->vx = SIDE(s) ? DROP_VX : -DROP_VX;
        SqueakSetState(s, DN_DROP);
        break;
    case DN_DROP:               // (a star where his next step reaches a mark or crosses it, then his step);
        if (SIDE(s) ? base->x > SpotX(s, DROP_RIGHT_X) : base->x < SpotX(s, DROP_LEFT_X)) {
            SqueakSetState(s, DN_END);
            chain = FALSE;
            break;
        }
        x = base->x + SQUEAK_SPEED(s->vx);
        for (i = 0; i < ARRAY_COUNT(sDropMarks); i++) {
            s32 mark = SpotX(s, sDropMarks[i]);

            s32 from = base->x - mark, to = x - mark;

            if ((from > 0 && to <= 0) || (from < 0 && to >= 0)) Shoot(s, 0, 0, 0, -DROP_VY);
        }
        MoveFree(s);
        break;
    // His defeat (0x0211DF2C-0x0211CDCC): TIMER the DS's +0x208, COUNT its +0x22C, SIDE its +0x230;
    case DN_DEFEAT:             // 0x0211DF2C: the controls taken, the far view to black, the screen whitening, still;
        Lock(s);
        Shade(s, 0, DEFEAT_DARK);
        Whiten(s, TRUE, FLASH);
        Stop(s);
        SqueakSetState(s, DN_DEFEAT_FLASH);
        break;
    case DN_DEFEAT_FLASH:       // 0x0211DEA4;
        if (Whitening(s)) break;
        COUNT(s) = 0;
        Whiten(s, FALSE, DEFEAT_CLEAR);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_FLICKER_1);
        break;
    case DN_FLICKER_1:          // 0x0211DCB8: his forms flickering while the screen clears, then it whitens;
        Flicker(s, FLICKER_1, 0);
        if (Whitening(s)) break;
        Whiten(s, TRUE, FLASH);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_FLASH_2);
        break;
    case DN_FLASH_2:            // 0x0211DC24;
        if (Whitening(s)) break;
        COUNT(s) = 0;
        SIDE(s) = 2;
        Whiten(s, FALSE, DEFEAT_CLEAR);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_FLICKER_2);
        break;
    case DN_FLICKER_2:          // 0x0211D9C4: two forms, his black star;
        Flicker(s, FLICKER_2, 2);
        if (Whitening(s)) break;
        Whiten(s, TRUE, FLASH);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_FLASH_3);
        break;
    case DN_FLASH_3:            // 0x0211D930;
        if (Whitening(s)) break;
        COUNT(s) = 0;
        SIDE(s) = 1;
        Whiten(s, FALSE, DEFEAT_CLEAR);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_FLICKER_3);
        break;
    case DN_FLICKER_3:          // 0x0211D6D0: a form, his black star;
        Flicker(s, FLICKER_3, 1);
        if (Whitening(s)) break;
        Whiten(s, TRUE, FLASH);
        SqueakSfx(s, 0x153);
        SqueakSetState(s, DN_BLACK);
        break;
    case DN_BLACK:              // 0x0211D5A4: his black star at the view's (128, 64), the far view back to its colors;
        if (Whitening(s)) break;
        SqueakPlay(s, ANIM_BLACK);
        base->x = SpotX(s, DEFEAT_X);
        base->y = SpotY(s, DEFEAT_Y);
        Shade(s, SHADE_FULL, FLASH);
        TIMER(s) = WHITE_HOLD;
        SqueakSetState(s, DN_BLACK_HOLD);
        break;
    case DN_BLACK_HOLD:         // 0x0211D51C: white a little longer, then clearing, the controls given back;
        if (TIMER(s)-- >= 1) break;
        Whiten(s, FALSE, WHITE_CLEAR);
        SqueakSfx(s, 0x153);
        Unlock(s);
        SqueakSetState(s, DN_BLACK_CLEAR);
        break;
    case DN_BLACK_CLEAR:        // 0x0211D46C: his eye shaking;
        if (Whitening(s)) break;
        TIMER(s) = DEFEAT_STEP;
        EyeMode(s, MODE_SHAKE);
        SqueakSetState(s, DN_SPARKS_1);
        break;
    case DN_SPARKS_1:           // 0x0211D330: sparks; then his flames gone, his star shaking, his music fading out;
        if (TIMER(s) % SPARKS_1 == 0) DefeatSpark(s, SPARK_1);
        if (TIMER(s)-- >= 1) break;
        if (Alive((struct SqueakObject *)AURA(s), SQUEAK_DARK_NEBULA_AURA)) Gone((struct SqueakObject *)AURA(s));
        AURA(s) = 0;
        SqueakPlay(s, ANIM_SHAKING);
        TIMER(s) = DEFEAT_STEP;
        // (only where his music still plays: his own fight's, his template's unk22 bit 2; KatAM's mini-boss defeat put others' back);
        if (s->obj.objTemplate && (s->obj.objTemplate->unk22 & 4)) m4aMPlayFadeOut(&gMPlayInfo_0, MUSIC_FADE);
        SqueakSetState(s, DN_SPARKS_2);
        break;
    case DN_SPARKS_2:           // 0x0211D214: more sparks; then his star breaking up;
        if (TIMER(s) % SPARKS_2 == 0) DefeatSpark(s, SPARK_2);
        if (TIMER(s)-- >= 1) break;
        SqueakPlay(s, ANIM_BREAKING);
        TIMER(s) = DEFEAT_STEP;
        SqueakSetState(s, DN_SPARKS_3);
        break;
    case DN_SPARKS_3:           // 0x0211D0F4: till it's broken up (ext's 16: nothing);
        if (TIMER(s) % SPARKS_3 == 0) DefeatSpark(s, SPARK_3);
        TIMER(s)--;
        if (!s->animEnded) break;
        base->flags |= 0x400;
        TIMER(s) = DEFEAT_STEP;
        SqueakSetState(s, DN_SPARKS_4);
        break;
    case DN_SPARKS_4:           // 0x0211CFD0: his eye left, still (the DS: cmn's 3 on him, his eye hidden);
        if (TIMER(s) % SPARKS_4 == 0) DefeatSpark(s, SPARK_3);
        if (TIMER(s)-- >= 1) break;
        EyeMode(s, MODE_STILL);
        TIMER(s) = DEFEAT_STEP;
        SqueakSetState(s, DN_EYE_WAIT);
        break;
    case DN_EYE_WAIT:           // 0x0211CEF8: then its twinkle (the DS: cmn's 5 on him);
        if (TIMER(s)-- >= 1) break;
        if (Alive((struct SqueakObject *)EYE(s), SQUEAK_DARK_NEBULA_EYE)) Gone((struct SqueakObject *)EYE(s));
        EYE(s) = 0;
        TWINKLE(s) = (s32)Thing(s, T_TWINKLE, 0, 0);
        SqueakSetState(s, DN_TWINKLE_END);
        break;
    case DN_TWINKLE_END:        // 0x0211CE24: at its end, the white-out (sound 0x1DA);
        if (Alive((struct SqueakObject *)TWINKLE(s), SQUEAK_DARK_NEBULA_TWINKLE) && !((struct SqueakObject *)TWINKLE(s))->animEnded) break;
        Whiten(s, TRUE, WHITE_OUT);
        SqueakSfx(s, 0x1DA);
        SqueakSetState(s, DN_WHITE_OUT);
        break;
    case DN_WHITE_OUT:          // 0x0211CDCC: white, the stage cleared (here: the white cleared, then he's gone, the finale on);
        if (Whitening(s)) break;
        if (Alive((struct SqueakObject *)TWINKLE(s), SQUEAK_DARK_NEBULA_TWINKLE)) Gone((struct SqueakObject *)TWINKLE(s));
        TWINKLE(s) = 0;
        Whiten(s, FALSE, WHITE_BACK);
        SqueakSetState(s, DN_WHITE_BACK);
        break;
    case DN_WHITE_BACK:
        if (Whitening(s)) break;
        base->flags |= 0x1000;
        break;
    }
    return s->stateTimer == 0xFFFF && chain;
}

/* NextForm(); 0x02122804 (his +0x18, each frame): his next form, DELAY frames after it's picked: a swoop, whatever he's doing (as
 * recorded: a rain cut short); */
static void NextForm(struct SqueakObject *s) {
    if (FORM(s) == NEXT(s)) return;
    if (DELAY(s)-- > 0) return;
    FORM(s) = NEXT(s);
    SqueakSetState(s, DN_SWOOP);
}

// Hittable(); 0x021228C4: in a form, the screen not whitening, not blinking (BLINK frames after a hit);
static bool32 Hittable(struct SqueakObject *s) {
    return !DEFEAT(s) && s->state >= DN_IDLE && !Whitening(s) && BLINK(s) <= 0 && FORM(s) >= FORM_FIRE && FORM(s) <= FORM_ICE
        && s->anim >= ANIM_ICE && s->anim <= ANIM_SPARK;
}

/* Hit(); 0x021228C4 (KatAM's mini-bosses take hits without a hurt state of their own: a fall of his HP is the hit): a hit where
 * none is taken undone; a hit taken, BLINK_TIME frames blinking (his eye drawn every other two), the camera shaken; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (BLINK(s) > 0) BLINK(s)--;
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (!Hittable(s)) {
            obj->unk80 = LAST_HP(s);
        } else {
            BLINK(s) = BLINK_TIME;
            RequestScreenShake(1, &obj->base);
        }
    }
    LAST_HP(s) = obj->unk80;
}

/* Boxes(); His box in a form (darkZERO_cmn.rca's: hit 17 round him, his touch 16: KatAM's one box, the hit one), hurting; none as the
 * dark star and the black star (no attack takes him: KatAM's level 6); */
static void Boxes(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    SqueakUntouchable(s, !Hittable(s));
    // (his defeat: none, his forms flickering harmless);
    if (DEFEAT(s) || s->anim < ANIM_ICE || s->anim > ANIM_SPARK || s->composite != SQUEAK_NO_COMPOSITE) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    base->flags &= ~0x200;
    sub_0803E2B0(base, -17, -17, 17, 17);
    base->unk68 = HARM(s);
}

/* EyeFrame(); His eye's update (0x0211C4E0's states), run before his as Squeak Squad's: where he is before his move (a frame behind
 * him), the mode he gave it, then: its twinkle's end (0x0211BCCC: whatever he's doing, his whitening), its blink's (0x0211BBC4:
 * still), its shooting (0x0211BEB4: while under the view's top, 1 in 100 a star at 1.5 at Kirby's 24 ahead, and 5 in 100 at Kirby
 * at 2.0: both under 1); */
static void EyeFrame(struct SqueakObject *s) {
    struct SqueakObject *e = (struct SqueakObject *)EYE(s), *w;
    struct Kirby *kirby = s->obj.kirby3;
    u32 r;
    s32 x;

    if (!Alive(e, SQUEAK_DARK_NEBULA_EYE)) return;
    e->obj.base.x = s->obj.base.x;
    e->obj.base.y = s->obj.base.y;
    if (E_NEXT(e)) {
        E_MODE(e) = E_NEXT(e) - 1;
        E_NEXT(e) = 0;
        switch (E_MODE(e)) {
        case MODE_STILL: case MODE_SHOOT: SqueakPlay(e, EYE_IDLE); break;
        case MODE_HALF: SqueakPlay(e, EYE_HALF); break;
        case MODE_BLINK: SqueakPlay(e, EYE_BLINK); break;
        case MODE_TWINKLE: E_TWINKLE(e) = (s32)Thing(s, T_TWINKLE, 0, 0); break;
        case MODE_SHAKE: SqueakPlay(e, EYE_IDLE); E_SHAKE(e) = 0; break;
        }
    }
    switch (E_MODE(e)) {
    case MODE_SHAKE: {          // 0x0211BAB0: on him, a step of its 12 offsets each frame (the DS's Y up);
        static const s8 sShake[12][2] = {
            { 0, 0 }, { 1, -2 }, { 1, -2 }, { -2, 2 }, { 2, -1 }, { 2, -1 }, { -2, 0 }, { 2, 1 }, { 2, 1 }, { -2, -2 }, { 1, 2 }, { 1, 2 },
        };
        const s8 *d = sShake[++E_SHAKE(e) % 12];

        e->obj.base.x += d[0] << 8;
        e->obj.base.y -= d[1] << 8;
        break;
    }
    case MODE_TWINKLE:
        w = (struct SqueakObject *)E_TWINKLE(e);
        if (Alive(w, SQUEAK_DARK_NEBULA_TWINKLE)) {
            w->obj.base.x = e->obj.base.x;
            w->obj.base.y = e->obj.base.y;
            if (!w->animEnded) break;
            w->obj.base.flags |= 0x1000;
            if (s->obj.unk80 > 0) SqueakSetState(s, DN_WHITEN);
        }
        // (none made: his twinkle's wait goes on, DN_TWINKLE);
        E_TWINKLE(e) = 0;
        E_MODE(e) = MODE_STILL;
        SqueakPlay(e, EYE_IDLE);
        break;
    case MODE_BLINK:
        if (!e->animEnded) break;
        E_MODE(e) = MODE_STILL;
        SqueakPlay(e, EYE_IDLE);
        break;
    case MODE_SHOOT:
        if (e->obj.base.y <= SpotY(s, 0) || kirby == NULL) break;
        r = Random(100);
        if (r < 1) {
            x = kirby->base.x + ((kirby->base.flags & 1) ? -(EYE_AHEAD << 8) : EYE_AHEAD << 8);
            ShootAt(s, x, kirby->base.y, EYE_LEAD);
        }
        if (r < 5) ShootAt(s, kirby->base.x, kirby->base.y, EYE_AIM);
        break;
    }
}

/* DefeatFrame(); His frame in his defeat (his HP out: DarkNebulaWatch): his eye's, his states', the far view's tint and the screen's
 * whiteness; no box, no HP bar taken back (it shows none left); */
static void DefeatFrame(struct SqueakObject *s) {
    EyeFrame(s);
    SqueakRun(s, DarkNebula);
    ShowShade(s);
    ShowWhite(s);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
}

/* DefeatStart(); 0x021228C4, his HP out: his eye still (its mode 5), his things gone (Owner()), his sounds stopped, the far view at
 * its colors (SHADE_FULL), then his defeat (0x0211DF2C); */
static void DefeatStart(struct SqueakObject *s) {
    DEFEAT(s) = TRUE;
    BLINK(s) = 0;
    EndThings(s);
    if (s->loopSound) SqueakStopSfx(s, s->loopSound);
    SHADE_FROM(s) = SHADE_TO(s) = SHADE_FULL;
    SHADE_GONE(s) = SHADE_LEN(s) = 0;
    // (his form's tint gone: the far view its own colors);
    TintOff(s);
    FarViewTimes(s, SHADE_FULL, SHADE_FULL, SHADE_FULL);
    TWINKLE(s) = 0;
    LOCKED(s) = 0;
    EyeMode(s, MODE_STILL);
    SqueakSetState(s, DN_DEFEAT);
}

/* DarkNebulaWatch(); KatAM's object's own before its Main (unk7C, every frame, after its hits): his HP out, KatAM's mini-boss defeat
 * (sub_0809AF38's sub_0809D060: unk78 its falling body's, his flags, unkC and unk5C changed, his HP 0) put back as his last frame left
 * them (KEPT_*), his HP kept at 0 (KatAM's HP bar empty, as the DS's), and his own defeat instead (DefeatStart); */
static void DarkNebulaWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    // (his HP out however it went, KatAM's defeat started or not: his own defeat, once);
    if (DEFEAT(s) || obj->unk80 > 0 || (obj->base.flags & 0x1000)) return;
    obj->unk78 = DarkNebulaMain;
    obj->base.flags = KEPT_FLAGS(s);
    obj->base.unkC = KEPT_C(s);
    obj->base.unk5C = KEPT_5C(s);
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->unk80 = 0;
    DefeatStart(s);
}

static void DarkNebulaMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (DEFEAT(s)) {
        DefeatFrame(s);
        return;
    }
    Hit(s);
    if (obj->unk80 <= 0) {
        if (WHITE(s)) {
            gBldRegs.bldCnt = 0;
            gBldRegs.bldY = 0;
            WHITE(s) = 0;
        }
        EndThings(s);
        return;
    }
    EyeFrame(s);
    // (his fireball's end, 0x0212FBBC: whatever he's doing, he ends his move: 0x0211EA7C);
    if (ORB(s) && !Alive((struct SqueakObject *)ORB(s), SQUEAK_DARK_NEBULA_FIREBALL)) {
        ORB(s) = 0;
        SqueakSetState(s, DN_ORB_DONE);
    }
    NextForm(s);
    SqueakRun(s, DarkNebula);
    if (s->state >= DN_START && obj->unk80 > 0) Companions(s);
    /* (his HP bar Squeak Squad's boss's, which his things' hits don't take: KatAM's goes to whatever is hit, taken back; from his black
     * star's wait's end on, as the DS's HUD takes him then: DN_START); */
    if (s->state > DN_START) sub_08034E14(obj);
    ShowTint(s);
    ShowWhite(s);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
    KEPT_FLAGS(s) = obj->base.flags;
    KEPT_C(s) = obj->base.unkC;
    KEPT_5C(s) = obj->base.unk5C;
}

// DarkNebulaGone(); He's gone (squeak.c's SqueakDestroy): his burning floor's VRAM let go of;
void DarkNebulaGone(struct SqueakObject *s) {
    struct Task *t;
    u16 i;

    if (BURN_VRAM(s)) VramFree(BURN_VRAM(s));
    BURN_VRAM(s) = 0;
    // (gone in his defeat: the controls he took given back);
    if (DEFEAT(s)) Unlock(s);
    /* (his far view's colors' keeper let go of (FarViewKept()), the far view its own colors again while his room is the one shown
     * (a room loaded after him has its own); */
    if ((t = FarViewTask(s)) != NULL) {
        for (i = 0; i < gNumKirbys; i++)
            if (gCurLevelInfo[i].currentRoom == s->obj.base.roomId) break;
        if (i < gNumKirbys) FarViewTimes(s, SHADE_FULL, SHADE_FULL, SHADE_FULL);
        TaskDestroy(t);
    }
    TINT(s) &= 0xFFFF;
}

/* SqueakDarkNebulaStart(); His coming and fight's start (and after a hit: it goes on, as KatAM's mini-bosses do). He isn't held in
 * the camera's area (Squeak Squad's flag 0x20000), through everything (0x100); */
void SqueakDarkNebulaStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, DarkNebulaMain);
    obj->unk7C = DarkNebulaWatch;
    if (s->started) return;
    LAST_HP(s) = obj->unk80;
    FORM(s) = NEXT(s) = FORM_NONE;
    DELAY(s) = PICKS(s) = BLINK(s) = WHITE(s) = ORB(s) = BURNING(s) = BURN_VRAM(s) = 0;
    EYE(s) = AURA(s) = DEFEAT(s) = LOCKED(s) = 0;
    TINT(s) = TINT_TIME + 1;
    KEPT_FLAGS(s) = obj->base.flags;
    KEPT_C(s) = obj->base.unkC;
    KEPT_5C(s) = obj->base.unk5C;
    SqueakSetState(s, DN_COME);
}

// SqueakDarkNebulaInit(); His box, his bounds, the floor where he's placed (a8s4s6's); hidden, still, till his fight;
void SqueakDarkNebulaInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    sub_0803E2B0(&obj->base, -17, -17, 17, 17);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
    HARM(s) = obj->base.unk68;
    DEFEAT(s) = LOCKED(s) = 0;
    SqueakStandOnFloor(obj);
    FLOOR(s) = SqueakBossFloor(s);
    /* (in his own stage, a8s4s6, his spot is the dark star's, in the air: 92 over the floor, COME_Y under the view's top, too high for
     * SqueakStandOnFloor's 5 cells; the floor his moves are measured from is the first one under him, his DS one: 80 over the stage's
     * bottom); */
    if (obj->base.roomId >= SQUEAK_FIRST_ROOM && !(obj->objTemplate && (obj->objTemplate->unk22 & SQUEAK_LAIR_FIGHT))) {
        s32 y = obj->base.y & ~0xFFF, bottom = gCurLevelInfo[obj->base.unk56].levelMaxPosition.y;

        for (; y <= bottom; y += 0x1000) {
            if (!NidlIsFloor(obj, obj->base.x, y + 0x1000)) continue;
            FLOOR(s) = y + 0x1000 - 1;
            break;
        }
    }
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk62 |= 4;
    SqueakPlay(s, ANIM_STAR);
}

/* What he brings in (the variant): his eye, its twinkle, his flames, his stars, sparks and flames, his fireball, its burst, his ice
 * pillar, his bolts, their cage (and its sides, floor and top: KatAM's one box an object); all gone when he's beaten (0x0212FB94); */
#define T_HARM(t) ((t)->vars[0])
#define T_SIDE(t) ((t)->vars[1])        // The pillar's side (moving right: 1);
#define T_TIME(t) ((t)->vars[2])
#define T_LAST_HP(t) ((t)->vars[3])
#define T_SAFE(t) ((t)->vars[4])
#define T_PREV_Y(t) ((t)->vars[5])

/* A flame's trail (T_BURN): the floor's tiles it set burning (0x0212F194: the stage's fire tile, each BURN_TIME frames), one thing
 * for them all, hidden, drawing each tile itself and hurting with one box across them (a thing a tile, sixteen with his fire wall up,
 * took a sixth of a frame more than KatAM has: a lag every other frame); */
#define T_TRAIL(t) ((t)->vars[6])       // A flame's trail;
#define B_FLAME(t) ((t)->vars[6])       // A trail's flame, its first and last tiles burning (16 pixels each, from the DS view's left),
#define B_FIRST(t) ((t)->vars[7])       // each tile's frames left (none: not burning);
#define B_LAST(t) ((t)->vars[8])
#define B_LEFT(t, c) ((t)->vars[9 + (c)])

static struct SqueakObject *Owner(struct SqueakObject *t) {
    struct SqueakObject *s = (struct SqueakObject *)t->obj.base.parent;

    if (!Alive(s, SQUEAK_DARK_NEBULA)) return NULL;
    // (his HP out, 0x0212FB94: all gone but his eye, his flames and his twinkle, his defeat's);
    if (s->obj.unk80 <= 0 && !(DEFEAT(s) && (t->variant == T_EYE || t->variant == T_AURA || t->variant == T_TWINKLE))) return NULL;
    return s;
}

/* OutOfView(); Out of Squeak Squad's view (0x02209D00: across 0-256 of it, down 8-200, as recorded: its bolts gone 8 under its top,
 * its flames past either side), its margins 1 (24 over, 56 under and either side: his stars, sparks, fireball and pillar) or none (4:
 * his bolts and flames); */
enum { MARGINS_1, MARGINS_NONE };

static bool32 OutOfView(struct SqueakObject *t, struct SqueakObject *s, u8 margins) {
    s32 side = margins == MARGINS_1 ? 56 : 0, over = margins == MARGINS_1 ? 24 : 0;
    s32 x = t->obj.base.x, y = t->obj.base.y;

    return x < SpotX(s, -side) || x > SpotX(s, 2 * VIEW_HALF + side) || y < SpotY(s, 8 - over) || y > SpotY(s, 200 + side);
}

static void Gone(struct SqueakObject *t) {
    t->obj.base.flags |= 0x1000;
}

// Ignite(); Tile `c` of the floor set burning by flame `t`, its trail's (made with its first tile);
static void Ignite(struct SqueakObject *t, struct SqueakObject *s, s32 c) {
    struct SqueakObject *b = (struct SqueakObject *)T_TRAIL(t);
    u8 i;

    if (!Alive(b, SQUEAK_DARK_NEBULA_BURN) || B_FLAME(b) != (s32)t) {
        b = SqueakSpawn(t, SQUEAK_DARK_NEBULA_BURN, T_BURN, 0, 0);
        T_TRAIL(t) = (s32)b;
        if (b == NULL) return;
        if (!BURN_VRAM(s)) BURN_VRAM(s) = VramMalloc(gSqueakData.enemies[SQUEAK_DARK_NEBULA_BURN].tiles);
        VramFree(b->obj.base.sprite.tilesVram);
        b->obj.base.sprite.tilesVram = BURN_VRAM(s);
        b->obj.base.parent = &s->obj;
        // (the stage's tile as it is: Squeak Squad's pictures face right, KatAM's frames left, turned facing right; hidden: Burn());
        b->obj.base.flags |= 1 | 0x400;
        B_FLAME(b) = (s32)t;
        B_FIRST(b) = B_LAST(b) = c;
        for (i = 0; i < 16; i++) B_LEFT(b, i) = 0;
    }
    if (c < B_FIRST(b)) B_FIRST(b) = c;
    if (c > B_LAST(b)) B_LAST(b) = c;
    B_LEFT(b, c) = BURN_TIME;
    BURNING(s) |= 1 << c;
    b->obj.base.x = SpotX(s, (B_FIRST(b) + B_LAST(b) + 1) * 8);
    b->obj.base.y = FLOOR(s);
}

/* Burn(); A trail's tiles burning out, each drawn (KatAM's drawing of an object, sub_0809A7A4's, the thing's sprite at each tile:
 * its frame from its second, KatAM's stepping after its first), its box across them (ThingBoxes()); gone with its last, and with the
 * floor's last their VRAM block; */
static void Burn(struct SqueakObject *b, struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[gLocalPlayerId];
    struct Sprite *sprite = &b->obj.base.sprite;
    s32 c, first = -1, last = -1;
    bool32 drawn = T(b) > 0 && gKirbys[gLocalPlayerId].base.roomId == b->obj.base.roomId;

    for (c = B_FIRST(b); c <= B_LAST(b); c++) {
        if (B_LEFT(b, c) <= 0) continue;
        if (--B_LEFT(b, c) == 0) {
            BURNING(s) &= ~(1 << c);
            continue;
        }
        if (first < 0) first = c;
        last = c;
        if (!drawn) continue;
        sprite->x = (SpotX(s, c * 16 + 8) >> 8) - (level->viewportPosition.x >> 8) + gUnk_0203AD18[0];
        sprite->y = (FLOOR(s) >> 8) - (level->viewportPosition.y >> 8) + gUnk_0203AD18[1];
        DisplaySprite(sprite);
    }
    if (first < 0) {
        if (!BURNING(s) && BURN_VRAM(s)) {
            VramFree(BURN_VRAM(s));
            BURN_VRAM(s) = 0;
        }
        Gone(b);
        return;
    }
    B_FIRST(b) = first;
    B_LAST(b) = last;
    b->obj.base.x = SpotX(s, (first + last + 1) * 8);
    b->obj.base.y = FLOOR(s);
}

/* StarMain(); His star (Shoot()) till the floor, a wall or out of view (0x0212FAF0, 0x0212F9F8: its margins 1), or on a Kirby (a
 * puff, as Dark Mind's: 0x08109E98); gone with him, his HP out (Owner()); */
static void StarMain(struct Object *star) {
    struct SqueakObject *s = (struct SqueakObject *)star->base.parent;
    s32 x = star->base.x, y = star->base.y;

    star->base.flags |= 4;
    if (!Alive(s, SQUEAK_DARK_NEBULA) || s->obj.unk80 <= 0 || y >= FLOOR(s) || x < SpotX(s, -56) || x > SpotX(s, 2 * VIEW_HALF + 56)
        || y < SpotY(s, 8 - 24)) {
        star->base.flags |= 0x1000;
        return;
    }
    if (star->base.unk58 & 0x200) {
        CreateEffectObject(&star->base, 0, 0x298, 0);
        star->base.flags |= 0x1000;
    }
}

// OnFloor(); Its spot down to the floor where he was placed (the stage's ground: Squeak Squad's ground distance under 1);
static bool32 OnFloor(struct SqueakObject *t, struct SqueakObject *s) {
    return t->obj.base.y >= FLOOR(s);
}

static void Spark(struct SqueakObject *t) {
    struct SqueakObject *s = (struct SqueakObject *)t->obj.base.parent, *k;
    s32 a = Random(360) << 12;

    k = SqueakSpawn(t, SQUEAK_DARK_NEBULA_SHOT, T_SPARK, 0, 0);
    if (k == NULL) return;
    k->obj.base.parent = &s->obj;
    k->vx = FxMul(SPARK_SPEED, Cos(a));
    k->vy = FxMul(SPARK_SPEED, Sin(a));
}

static bool32 DarkNebulaThing(struct SqueakObject *t) {
    struct SqueakObject *s = Owner(t), *e;
    struct ObjectBase *base = &t->obj.base;
    s32 x;
    u8 i;

    if (s == NULL) {
        Gone(t);
        return FALSE;
    }
    switch (t->variant) {
    case T_EYE:                 // (its update: EyeFrame) in front of him, hidden while it twinkles, and every other frame of his
        base->sprite.unk14 = s->obj.base.sprite.unk14 - 0x40;  // blink after a hit (0x02122804: his +0x200 odd);
        base->flags = (base->flags & ~0x400) | (E_MODE(t) == MODE_TWINKLE || (BLINK(s) & 1) ? 0x400 : 0);
        break;
    case T_TWINKLE:             // (its update: EyeFrame);
        base->sprite.unk14 = s->obj.base.sprite.unk14 - 0x40;
        break;
    case T_AURA:                // 0x0211B8B4: on him, behind;
        base->x = s->obj.base.x;
        base->y = s->obj.base.y;
        base->sprite.unk14 = s->obj.base.sprite.unk14 + 0x40;
        break;
    case T_STAR:                // 0x0212FAF0, 0x0212F9F8: till the floor, a wall or out of view;
        if (T(t) == 0) SqueakSfx(t, 0x1D2);
        MoveFree(t);
        if (OnFloor(t, s) || OutOfView(t, s, MARGINS_1)) Gone(t);
        break;
    case T_SPARK:               // 0x0212F57C, 0x0212F4F4: blinking, 20 frames;
        t->ay = -GRAVITY;
        MoveFree(t);
        base->flags = (base->flags & ~0x400) | (T(t) & 1 ? 0x400 : 0);
        if (T(t) > SPARK_TIME || OnFloor(t, s) || OutOfView(t, s, MARGINS_1)) Gone(t);
        break;
    case T_FLAME:               // 0x0212F1D4, 0x0212F194: along the floor, setting it burning;
        MoveFree(t);
        if (OutOfView(t, s, MARGINS_NONE)) {
            Gone(t);
            break;
        }
        x = (base->x - SpotX(s, 0)) >> 8;
        if (x >= 0 && x < 2 * VIEW_HALF && !(BURNING(s) & (1 << (x / 16)))) Ignite(t, s, x / 16);
        break;
    case T_BURN:
        Burn(t, s);
        break;
    /* (0x0212F89C, 0x0212F770; its hits 0x0212FC08: none for 5 frames after one, a camera shake; while those 5 go, a spark every
     * third; its HP out, 0x0212F72C: its move, 5 sparks, gone; the floor: the burst, and 1 up the flames either way); */
    case T_FIREBALL:
        if (T(t) == 0) {
            t->vy = 0;
            t->ay = -ORB_AY;
            T_LAST_HP(t) = t->obj.unk80;
        }
        if (t->obj.unk80 <= 0) t->obj.unk80 = ORB_HP_BASE;
        if (t->obj.unk80 < T_LAST_HP(t)) {
            if (T_SAFE(t) > 0) {
                t->obj.unk80 = T_LAST_HP(t);
            } else {
                T_SAFE(t) = ORB_SAFE;
                RequestScreenShake(1, base);
                SqueakSfx(t, 0x1D8);
            }
        }
        T_LAST_HP(t) = t->obj.unk80;
        MoveFree(t);
        if (t->obj.unk80 <= ORB_HP_BASE) {
            for (i = 0; i < 5; i++) Spark(t);
            Gone(t);
            break;
        }
        if (T_SAFE(t) > 0 && --T_SAFE(t) % 3 == 0) Spark(t);
        if (OnFloor(t, s)) {
            struct SqueakObject *b;

            base->y = FLOOR(s);
            b = SqueakSpawn(t, SQUEAK_DARK_NEBULA_FIRE, T_BURST, 0, 0);
            if (b) b->obj.base.parent = &s->obj;
            for (i = 0; i < 2; i++) {
                b = SqueakSpawn(t, SQUEAK_DARK_NEBULA_SHOT, T_FLAME, 0, 1 << 12);
                if (b == NULL) continue;
                b->obj.base.parent = &s->obj;
                b->vx = i ? -FLAME_VX : FLAME_VX;
                SqueakPlay(b, SHOT_FLAME);
            }
            Gone(t);
        } else if (OutOfView(t, s, MARGINS_1)) {
            Gone(t);
        }
        break;
    case T_BURST:               // 0x0212F448, 0x0212F348, 0x0212F298, 0x0212F274;
        switch (T_SIDE(t)) {
        case 0:
            if (!t->animEnded) break;
            SqueakPlayComposite(t, C_WALL);
            T_TIME(t) = WALL_TIME + Random(WALL_RAND + 1);
            T_SIDE(t) = 1;
            break;
        case 1:
            if (T_TIME(t)-- > 0) break;
            SqueakPlay(t, FIRE_FADE);
            T_SIDE(t) = 2;
            break;
        default:
            if (t->animEnded) Gone(t);
            break;
        }
        break;
    case T_PILLAR:              // 0x0212EC6C, 0x0212EBC4: past the other side, gone;
        MoveFree(t);
        if (T_SIDE(t) ? base->x > SpotX(s, PILLAR_RIGHT_X) : base->x < SpotX(s, PILLAR_LEFT_X)) Gone(t);
        if (OutOfView(t, s, MARGINS_1)) Gone(t);
        break;
    case T_BOLT:                // 0x0212EE84, 0x0212EE58;
        MoveFree(t);
        if (OutOfView(t, s, MARGINS_NONE)) Gone(t);
        break;
    case T_CAGE:                // 0x0212ED54, 0x0212ED28: 40 frames; its sides, floor and top hurting (ext.rca's composite 0);
        if (T(t) == 0) {
            static const u8 sCage[] = { T_CAGE_SIDE, T_CAGE_SIDE, T_CAGE_FLOOR, T_CAGE_TOP };
            static const s16 sAt[][2] = { { -111, 68 }, { 112, 68 }, { 1, -2 }, { 1, 137 } };

            SqueakPlayComposite(t, C_CAGE);
            SqueakSfx(t, 0x1D5);
            for (i = 0; i < ARRAY_COUNT(sCage); i++) {
                e = SqueakSpawn(t, SQUEAK_DARK_NEBULA_SHOT, sCage[i], sAt[i][0] << 12, sAt[i][1] << 12);
                if (e == NULL) continue;
                e->obj.base.parent = &t->obj;
            }
        }
        if (T(t) >= CAGE_TIME) Gone(t);
        break;
    case T_CAGE_SIDE: case T_CAGE_FLOOR: case T_CAGE_TOP:   // (gone with the cage);
        e = (struct SqueakObject *)base->parent;
        if (!(e && e->obj.base.header.kind == 1 && !(e->obj.base.flags & 0x1000) && SqueakIsSqueak(&e->obj)
              && e->enemy == SQUEAK_DARK_NEBULA_BOLT && e->variant == T_CAGE))
            Gone(t);
        break;
    }
    return FALSE;
}

/* ThingBoxes(); A thing's box (darkZERO_cmn.rca's, darkZERO_ext.rca's: KatAM's units), hurting; his eye, its twinkle and his flames
 * none; */
static void ThingBoxes(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;
    u32 time = SqueakAnimTime(t);

    base->unk68 = T_HARM(t);
    switch (t->variant) {
    case T_STAR: sub_0803E2B0(base, -4, -4, 4, 4); break;
    case T_FIREBALL: sub_0803E2B0(base, -9, -8, 9, 10); break;
    case T_BURST:
        if (t->composite == C_WALL) {
            // (cmn.rca's composite 0: 32 or 37 either side, 122 up: KatAM's reach 127);
            if (t->compositeTime % 4 < 2) sub_0803E2B0(base, -31, -122, 32, 0);
            else sub_0803E2B0(base, -37, -122, 37, 0);
        } else {
            // (cmn.rca's 12, 19 the other way round: 2 frames each);
            static const s8 sBurst[3][4] = { { -9, -15, 5, -1 }, { -10, -21, 6, -1 }, { -9, -102, 7, 0 } };
            u8 f = time / 2 > 2 ? 2 : time / 2;
            const s8 *b = sBurst[t->anim == FIRE_BURST ? f : 2 - f];

            sub_0803E2B0(base, b[0], b[1], b[2], b[3]);
        }
        break;
    case T_PILLAR:
        if (time % 8 < 2) sub_0803E2B0(base, -8, -108, 8, -14);
        else if (time % 8 < 6) sub_0803E2B0(base, -5, -110, 5, -4);
        else sub_0803E2B0(base, -6, -110, 6, -4);
        break;
    case T_BOLT:
        if (time % 6 < 2 || time % 6 >= 4) sub_0803E2B0(base, -21, -11, 17, 9);
        else sub_0803E2B0(base, -13, -18, 11, 14);
        break;
    case T_CAGE_SIDE: sub_0803E2B0(base, -8, -77, 8, 77); break;
    case T_CAGE_FLOOR: sub_0803E2B0(base, -108, -8, 108, 8); break;
    case T_CAGE_TOP: sub_0803E2B0(base, -108, -8, 108, 8); break;
    case T_BURN: {
        // (its tiles burning: 16 pixels each, its spot their middle; KatAM's box reaches 127);
        s32 half = (B_LAST(t) - B_FIRST(t) + 1) * 8;

        sub_0803E2B0(base, -half, -16, half > 127 ? 127 : half, 0);
        break;
    }
    default:
        sub_0803E2B0(base, -1, -1, 1, 1);
        base->unk68 = 0;
        break;
    }
}

static void DarkNebulaThingMain(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(t, DarkNebulaThing);
    if (obj->base.flags & 0x1000) return;
    SqueakFinishFrame(t);
    if (t->variant < T_CAGE_SIDE || t->variant > T_CAGE_TOP) SqueakAnimate(t);
    ThingBoxes(t);
}

void SqueakDarkNebulaThingStart(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, DarkNebulaThingMain);
    obj->kirbyAbility = 0;
    if (t->started) return;
    SqueakSetState(t, 0);
}

/* SqueakDarkNebulaThingInit(); Its box and bounds; free of the stage's hold, through everything; his stars, sparks and flames as
 * Squeak Squad's thrown things (inhaled, KatAM's attacks breaking them), his fireball hit (50 HP: 10 here) but not inhaled, the rest
 * no attack hurts, not inhaled; Kirby's body no harm to any; */
void SqueakDarkNebulaThingInit(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    T_HARM(t) = base->unk68;
    base->flags |= 0x1000000 | 0x2000000 | 0x100;
    base->unk5C |= 0x80;
    ObjectSetBounds(base, -4, -4, 4, 4);
    switch (t->variant) {
    case T_STAR: SqueakPlay(t, SHOT_STAR); break;
    case T_SPARK: case T_FLAME: SqueakPlay(t, SHOT_FLAME); break;
    case T_FIREBALL:
        base->unk5C = (base->unk5C & ~7) | 3 | 0x20 | 0x80;
        t->obj.unk80 += ORB_HP_BASE;
        break;
    case T_TWINKLE: SqueakPlay(t, 0); break;
    case T_BURST: SqueakPlay(t, FIRE_BURST); break;
    }
    // (all but his shots and his fireball: not hit, nor inhaled);
    if (t->variant != T_STAR && t->variant != T_SPARK && t->variant != T_FLAME && t->variant != T_FIREBALL)
        base->unk5C = (base->unk5C & ~7) | 6 | 0x20 | 0x80;
    if (t->variant >= T_CAGE_SIDE && t->variant <= T_CAGE_TOP) base->flags |= 0x400;
    ThingBoxes(t);
}
