# Open requests

What the owner has asked for that is still to do. When an item is finished, take it out. The design rules for doors, mirrors and
hubs are in [Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 52. Kirby: Squeak Squad in Amazing Mirror
- First step, the world map: a door per Squeak Squad level, as NiDL's 8. Mock-up proposed, waiting for the owner's choice:
  1 Prism Plains (green fields by the castle) to Moonlight Mansion; 2 Nature Notch (underground, by the caves) to Rainbow Route
  South; 3 Cushy Cloud (the cloud over the hills) to Rainbow Route East; 4 Jam Jungle (the dark forest) to Rainbow Route West;
  5 Vocal Volcano (Mustard Mountain's slope) to Rainbow Route North; 6 Ice Island (the ice tower) to Peppermint Palace East;
  7 Secret Sea (out at sea past Olive Ocean, clear of the QUIT text: the owner's sketch, as its own level map is open sea) to
  Olive Ocean; 8 Gamble Galaxy (space) to Candy Constellation.
- The owner's rules for it: never connected to a NiDL hub (either game can be on alone), and no 3 exits in a chain. Read as: each
  Squeak Squad door's path leads to a KatAM door, never to a NiDL door or another Squeak Squad door; all but Secret Sea's lead
  to one that goes straight to Central Circle. Its own hub is reached from Central Circle, not through the NiDL hub.
- The Fountain of Dreams' world map door comes off the map (the owner: end-game only, its path never opens), with the Squeak
  Squad doors.
- **The whole cast, 1:1** (the owner: "I don't want you to finish until you complete the entire cast of enemies and their
  behaviors. If there's a behavior that's not in KatAM, port it 1:1."): every regular enemy, every mid-boss, the Squeaks and
  every boss of the USA release (the owner chose "everything"). Enemies KatAM already has keep KatAM's own object, as NiDL's
  natives do, and only the Squeak Squad behaviors KatAM lacks are ported (the Waddle Dee's patrol and jumps...);
  the ones KatAM lacks get a full port with Squeak Squad's graphics. Research notes: [Squeak Squad Notes](Squeak%20Squad%20Notes.md).
  - Ported so far, checked against the DS game frame by frame: Perara, Gold Waddle Dee (all six variants' code; tested in KatAM:
    variants 0 and 3, the ledge and water).
  - Gussa: ported from its code (both brains), with Squeak Squad's composite animations (`.mca`, now supported: each part its own
    sprite) and the water's splashes. Tested in KatAM (room 0x83's water, USA): line-up, dash, bonk, return, leaping out and
    falling back in with splashes, its parts drawn and cleaned up, both variants; checked against the DS only for its drawing on
    land (no DS save state in water yet).
  - Crimp: ported from its code with its rock (the framework now has box probes and spawned characters). Tested in KatAM (room
    0x323, USA): grabbing and climbing the star block's side, the throw, the rock and its burst; not against a DS recording, nor on
    a tall wall.
  - Maiga: ported from its code; Kirby's inhale makes it bite his face and hold him till he mashes free or 120 frames pass,
    then it throws him (KatAM's own thrown-Kirby state, hurt on landing). Checked against the DS by logging both (hops, pull,
    bite, bob, release, throw) and tested in KatAM (room 0x323, USA): the hops, the pull, the bite on Kirby and on a helper,
    mashing free (42 frames with A every 2 frames), the throw and the hurt landing. Not tested: a hit on it while it bites, a
    helper's mashing.
  - Cret: ported from its code with its ball (all four brains, its three loops, the drop, the blow-back and pop). Checked against
    the DS by logging (drift, bob, the inhale's timing, the pop's spawns) and tested in KatAM (room 0x323, USA): variant 0's drift,
    variant 1's loop and drop, variant 2 inhaled to the pop. Not tested: loops A and B, the fly-away, the ball's burst on screen,
    what the pop leaves in KatAM.
  - Uja: ported from its code (both forms: the sword and its lunges, the glove and its grab, smother and throw; the puff, its bob
    and flight). Checked against the DS by logging (timing, distances, the grab, the throw's damage and direction, the inhale, its
    boxes per frame) and tested in KatAM (room 0x323, USA; the Kirby input lock left by the test's room start cleared by RAM write):
    the glove's grab, close, smother (120 frames), throw (1 HP) and flight; the sword's stop at 40 pixels, lunges every 44 frames,
    pointing up and the upward lunge hurting Kirby; KatAM's inhale; the glove's hurt frames. Not tested: the downward forms, a hit
    while it holds Kirby, Kirby mashing free.
  - Bio Spark: ported from its code with its kunai and leaf (all three brains, the hops and jumps, the kunai, the slash, hiding as a
    leaf, the izuna drop). Checked against the DS by logging (its pad and moves, every move's timing, the kunai, the izuna's grab,
    rise, hang, drop and throw with its damage on Kirby's landing, the screen shake) and tested in KatAM (room 0x323, USA): hops,
    jumps and high jumps frame for frame with the DS, the throw (16/40/32 frames) and its kunai hitting Kirby and bursting on a wall,
    the slash (16/120/30), variant 1 hiding, waking and leaving its leaf, variant 2 standing; the izuna (grab, rise, hang, drop,
    landing, throw, 1 HP on landing) with Kirby put in its hands by RAM writes (his post-door and hurt flags cleared: KatAM's
    grabbers refuse him then). Not tested: the izuna from a natural jump, a hit while it holds Kirby, the izuna over a pit, KatAM's
    inhale of it and its kunai. Uja's per-frame boxes now use the same shared code (same tables; its glove's grab checked again).
  - Starman: ported from its code (all four brains; walk, lift-off, flapping, rocket, dives and bounce, glide, skid, the climb
    away). Checked against the DS by logging (variant 2's walk, lift-off, skid and glide frame for frame; variant 1's lift-off and
    flapping; variant 0's hover) and tested in KatAM (room 0x323, USA; the helpers and Kirby held aside by RAM writes): the same,
    and variant 1's climb to the screen's top and dive onto Kirby. Not tested: the hard dive at a ceiling, the climb away, the
    bounce. Its ability (Squeak Squad's copy enemy; KatAM's Hi-Jump is NiDL's Starman's) is the owner's call.
  - Bubble Head: ported from its code with its bubbles (its brains, walk, the big blow's stream, the single bubble, the jump its
    brains never use). Checked against the DS by logging (every timing frame for frame: the shot, the big blow, the bubble's flight,
    first frame, pop and star burst; kind 8's damage, an ordinary hurt) and tested in KatAM (room 0x323, USA; helpers and Kirby
    held by RAM writes): the same timings, a bubble hurting Kirby and popping, the wall pop, the stage's edge, the instant pop inside
    a cloud mound, the stream hurting Kirby, variants 1 and 2; screenshots. Not tested: the ceiling and time-out pops, Kirby's
    attacks on its bubbles. Its ability (Squeak Squad's Bubble; none in KatAM) is the owner's call.
  - Gaw Gaw: ported from its code (its four brains; hops, the crouch, charge and pounce, the jaws' grab, bite, shake, chew and
    throws, the spring back; the walk; the wall cling, climbs, jump and pounce off a wall; the bites its brains never use).
    Checked against the DS by logging (variant 0's hops frame for frame; variant 1's charge, leap, grab, shake, throw, spring back
    and Kirby's damage) and tested in KatAM (room 0x323, USA; the same timings and distances; the still grab's chew and toss and
    its brain's restart; the wall cling from its spawn and the jump off it; screenshots; Kirby put in its jaws by RAM writes).
    Not tested: the wall climbs and the pounce off a wall, a hit while it holds Kirby.
  - Acchi: ported from its code with its fire (its three brains; the charge, low spit and lob; the fire's flight through the
    terrain, landing, burn and going out; the creep, back-off and lunges; the jump its brains never use). Checked against the DS
    by logging (variant 0's brain frame for frame, its first rest skipped by leftover memory; the charge, spit and fire frames;
    the low fire's and the lob's paths, landings and 121-frame burn; variant 1's waits, aims, back-off and creep) and tested in
    KatAM (room 0x323, USA; the same timings, paths and distances; the fire burning Kirby and flying on; a fire spat into the
    cloud mound's side going out at once; the inhale passing it by; variants 1 and 2; swallowed, it gives Fire, as on the DS;
    screenshots). The fire keeps Squeak Squad's position precision (KatAM's had a lob land a frame early). Not tested: a ceiling
    over the fire.
  - Bun: ported from its code (its three brains; walk, run, the reach with its ear, the grab, carry, overhead hold, three
    swings and the throw; the jump and fall). Checked against the DS by logging (variants 0 and 1: every state's frame, the grab
    19 frames into the reach, Kirby's positions through the hold, the throw's path, his 6 HP lost on landing; it gives Throw) and
    tested in KatAM (room 0x323, USA; the same frames and positions, the throw's path frame for frame, Kirby hurt on landing,
    variants 1 and 2; screenshots of every held pose against the DS's: KatAM's own turning Kirby, drawn in front of it as the
    DS's). Not tested: a hit while it holds Kirby, its walk's wall turn.
  - Metalun: ported from its code (its three brains; stand, lurching walk, crouch, the two rolling charges with their stomps and
    screen shakes, the stand-up; the fall). Checked against the DS by logging (variants 0, 1 and 2: every state's frame, every
    lurch's frame and distance; inhaled, it gives Metal) and tested in KatAM (room 0x323, USA; the same frames and positions but
    for KatAM's own one-frame stalls; the charge's frames against DS screenshots; inhaled and swallowed: no ability, as KatAM has
    no Metal). Not tested: its fall and landing, a wall or a ledge in the charge, its stomps' shake length (KatAM's shortest, 14
    frames, against the DS's 4).
  - Parasol: KatAM has it already (object 0xA2, held by its Waddle Dees, Waddle Doos and Shotzos; KatAM's Shotzo has a parasol
    too, so "Parasol Shotzo" isn't one KatAM lacks, as these notes once said): its code does what Squeak Squad's does (held while
    floating, let go on landing or a hit, flying off, or kept), so it stays KatAM's.
  - Waddle Dee (KatAM's own, `squeak/natives.c`): Squeak Squad's three brains KatAM's lacks are its variants 5-7 (patrol 24 pixels
    either side, jump once at Kirby within 48, jump every 122 frames), on KatAM's own object (its walk, jump and hurt). Checked
    against the DS by logging and tested in KatAM (room 0x323, USA: turn points, counts, the jump trigger and timer, the resume
    after a bump; screenshots). Not tested: a ledge or wall in their walk, a hit.
  - Framework: a looping animation's end is now a pulse on its loop frames, and a composite's ends a frame later, as on the DS
    (logged: Bubble Head's composites and bubble). Bio Spark changed with it (its brain's three waits in the slash now take a loop
    each, 6 frames, as on the DS: checked in KatAM), and Cret's ball now drops after its drop composite's 20 frames, a frame later
    (by the same rule; not logged again: its drop didn't come up on the DS in the test); the others wait on animations they just
    started.
  - Cret and Uja pass through the terrain, held in the stage (Squeak Squad's flag 0x200, pinned down with Uja: Cret had been stopped
    by KatAM's terrain). Checked in KatAM: Cret drifts across room 0x323 and stops at its left edge.
  - Still to do for those: their sounds (Squeak Squad's are the DS's own: KatAM's nearest, or the owner's call); the enemy base
    class's own states (bumping into Kirby, the shared state in water of the ones that don't swim).
  - Left: the other Squeak Squad-only enemies, the Squeak Squad behaviors of shared enemies, the mid-bosses, the Squeaks and the
    bosses; the randomizer options (a Squeak Squad ROM in the ROM Expansion tab, the enemy pool).

## 5. Bosses as faithful 1:1 recreations of NiDL
- **The other bosses**: check each against NiDL and fix what differs.
- **Differences left in the recreated fights** (KatAM's own engine, not changed): KatAM sees a hit a frame after NiDL; each boss's
  defeat starts with KatAM's explosion instead of NiDL's flash and burst of stars (Meta Knight's knockout too, where NiDL holds him
  53 frames on a dark screen); KatAM's nearest sounds; the flashes, the Kracko eye's spin and the Star Rod's shine count from the
  fight's start (NiDL's from its power-on clock, so they differ from game to game there too); KatAM's Kirby's box reaches 2 pixels
  lower than NiDL's; KatAM's inhale reach and pull and Kirby's knock-back when hurt.
- **Meta Knight**: the thrust's screen shake as he lands (NiDL's `0x080261D4`; the owner: keep it, to be done later); NiDL's hit
  kinds 4 and 6 (states 16 and 23: inhaled, a hit on his box 0), which none of KatAM's attacks are, not ported.
- **Mr. Shine & Mr. Bright**: when one is beaten, KatAM's hit stars stand in for NiDL's effects (`0x95`, `0xA2`), and shots still in
  the air fall on instead of popping; NiDL's 31 frames without hits after a hurt and Kirby's box for their shots while he floats
  aren't modelled. Ask the owner: at their fight's camera the room's two back layers sit about 4 pixels further right than NiDL's,
  and its foreground colours show a step brighter (KatAM's fade in from white stops a step short).
- **King Dedede**: not traced: NiDL's Kirby's own box, whether NiDL takes a hit on the 31st frame after one, whether a blinking
  Kirby is caught. Ask the owner: KatAM's screen in his room sits 8 pixels left of and 16 above NiDL's (24, 168).
- **Paint Roller**: ask the owner whether NiDL's painting touch (knocked back 20 frames, a white burst, `0x98`) should be ported:
  they die KatAM's way at once now.
- **Heavy Mole**: its hatch's sound ends differently from NiDL's. Found while testing, ask the owner: when the mole digs up high,
  the camera goes up with it (NiDL's), and a Kirby who stays on the dirt floor below can walk along there under the screen, out
  of sight (the fight keeps him up to 39 pixels below it rather than beating him, as NiDL's bottom does).
- **Poppy Bros. Jr.**: the tomato or apple comes off a frame after KatAM's inhale takes the rider (NiDL's the same frame), at 0.5 a
  frame (NiDL's 0.496 to 0.498, from a small random angle); KatAM's Maxim Tomato, not NiDL's blinking one.

## 37. Mini-boss cameras and Mike
- **Mini-boss cameras in rooms bigger than a screen** (the owner: cameras always exactly NiDL's): the fight's screen is NiDL's,
  but the way there isn't yet: NiDL's camera follows Kirby with no easing, slides to the fight's screen 2 pixels a frame once the
  room's trigger object (type 4) comes on, and in the tall rooms 0x09B and 0x27C goes on following him up and down (the port holds
  it); 0x244 has two holds.
- **Mike's flash**: NiDL's screen goes yellow, the port's brightens to white.

## 34. The Fountain of Dreams finale: differences from NiDL
- **The ending**: the Kirbys ride in KatAM's pose (NiDL's Kirby holds the Star Rod up); KatAM's HUD stays on (in the Power Orb's
  room it covers the cloud band's bottom); the room's own music plays, not NiDL's ending music; the ending's background is NiDL's
  mirrored left to right; at Nightmare's defeat his flight off starts after KatAM's burst, not 52 frames after the hit as in NiDL.
- **The ride into the moon**: the star and the Kirbys are KatAM's warp star sprite (NiDL's star is four 16x16 pieces, with Kirby
  in his rod pose); the star's dot is 1-2 pixels off NiDL's.
- **Nightmare's intro**: Kirby's pose (NiDL's holds the Star Rod up), KatAM's HUD, NiDL's random twinkling stars.
- **Nightmare's defeat**: may show a wrong palette. Not reproduced (NiDL's defeat and five of ours compared frame by frame, all
  three versions). Needs the owner's details: when in the defeat, what it looked like (a screenshot if possible), which version.

## 45. The Star Rod: questions for the owner
- Its stars fly through walls (as in the finale's fights); outside the fights the HUD card shows the rod's picture, as for any
  ability.
- NiDL's Kirby with the rod (Nightmare's fight) neither floats nor slides: A in the air keeps him spinning, Down + A stands him up.
  Ours still floats and slides as KatAM's Kirby does, with the rod held (his jump is NiDL's spin now, till he lands). Say if he
  should do as NiDL's.
- Only the Star Rod's hat is drawn behind Kirby where NiDL draws it behind him; the other new abilities' hats have 1-4 such
  frames each and are left as they were. Say if they should follow too.
- Its stand unlocks with KatAM's ending (as the Master Sword's). Should beating Nightmare unlock it instead, and should the
  "Unlock the Master Sword" option unlock the rod too?

## 43. Kracko's stage's lag
- Not reproduced (1 lag frame in 3400 alone, 3 with the three helpers). Ask the owner when it lags (the climb, the cloud, the
  lightning), on which emulator or console, with helpers out or not, and on which seed.

## 51. NiDL rooms' backgrounds at a tall room's top
- The NiDL rooms' background rates come from the formula the NiDL hub's did (`BackgroundRates()`), which leaves out the 40 pixels
  KatAM's camera keeps above a room's bottom, so at a tall room's very top its background can start a little above its first row
  (the hub's garbled band). Only the hub was reported and changed: say if the rooms should be checked and changed as well.

## 10. Screen shake as KatAM's
- The owner: the screen shakes far too much. Mr. Shine & Mr. Bright's and King Dedede's endless shakes are fixed; anything else
  that shakes too much: tell where.
