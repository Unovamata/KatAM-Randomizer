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
  - Glunk (KatAM's own, `squeak/natives.c`): Squeak Squad's beat is its variants 5 (a volley every 242 frames) and 6 (every 122),
    on KatAM's own Glunk (its sway, pose and bullets), with its hop out of water. Checked against the DS by logging (both beats:
    the volleys' frames, shots and 61-frame length) and in KatAM (room 0x323, USA: the same frames; KatAM's own variants
    unchanged); the hop tested in forced states (RAM writes: room 0xCA's underwater Glunk lifted out of the water, falling back
    in; a land Glunk set to "in water", landing on land). Not tested: a real exit from the water, B in the air, a hit.
  - Gordo (KatAM's own, `squeak/natives.c`): Squeak Squad's circle (its Gordo 4) is its variant 5 (a 19-pixel radius, 120 frames
    a lap, through everything); its other four are KatAM's. Checked against the DS by logging and in KatAM (room 0x323, USA: 251
    frames of its path within 1/100 of a pixel of the DS's; KatAM's own variants unchanged). Not tested: a hit or bump.
  - Snooter (KatAM's own, `squeak/natives.c`): Squeak Squad's standing brain is its variant 5 (faces Kirby, stands; KatAM's
    swallow, spit and rush kept, then it stands again). Checked against the DS by logging and in KatAM (room 0x323, USA: standing,
    a swallow and spit, a Sword hit's rush, the rush's end forced by RAM writes). Not tested: a real rush to its end, eating an object.
  - Scarfy: Squeak Squad's one brain is KatAM's own (inhaled, it turns, backs off and dives at Kirby, bursting on him: logged on
    the DS with Kirby inhaling it).
  - The other enemies KatAM has: their Squeak Squad brains are listed in the notes' table, each to be compared with KatAM's own
    variants (logged on both, Kirby pinned away) and the ones KatAM lacks ported as the Waddle Dee's. Done: Bronto Burt (all five
    are KatAM's own: two waves, the straight flight, the perch, the drop from the screen's top), Glunk, Scarfy, Gordo, Snooter, Chip, Batty, Blockin, Big Waddle Dee, Shotzo, Waddle Doo, Hot Head, Flamer, Pengy, Sparky, Sword Knight, Sir Kibble, Twister, Cupie, Laser Ball, Foley, Wheelie, UFO, Noddy, Boxin, Heavy Knight and Blipper (by its code: no DS save state in water) (KatAM's own but for the ported Glunk, Gordo and Snooter
    variants). Every shared enemy is compared, and the shared mid-bosses (Mr. Frosty, Bonkers, Box Boxer, Boxy: their moves logged
    on the DS are KatAM's own mid-bosses').
  - Mid-bosses (Squeak Squad's 0x33-0x36: Gao Gao, Big Metalun, Tedhaun, Buboo; its other four, Mr. Frosty, Bonkers, Box Boxer and
    Boxy, are KatAM's own): their framework is in (on the mini-boss types 0x40-0x42 NiDL's use; tested in KatAM with Big Metalun:
    the wait, the camera lock, the HP bar and its name). Big Metalun is ported (`squeak/big_metalun.c`): its brain and body checked
    against the DS by logging and in KatAM (room 0x323, USA: the same frames, the bonk at its area's edge and at a wall, Sword hits,
    the defeat with its HP lowered by a RAM write; screenshots); not tested: a fall, a real mini-boss room's camera lock, swallowing
    it. Gao Gao is ported (`squeak/gao_gao.c`, with Squeak Squad's inhalable star, `squeak/squeak_star.c`): checked against the DS
    by logging (jumps, hop, spin, pant, the brain) and in KatAM (room 0x323, USA: the same frames, the drill, a star inhaled, Sword
    hits, the defeat with its HP lowered by a RAM write; screenshots); not tested: the drill on the DS past its test stage's top, the
    landing's stars on screen, a real mini-boss room. Tedhaun is ported (`squeak/tedhaun.c`, its three ghosts with it): checked in KatAM
    (room 0x323, USA: the circle, the grab, carry, hold and throw, the ghosts' spots and sweeps, the call, Sword hits, the defeat);
    not checked against a DS recording (its DS test stage is too short for its moves: its code followed), nor its ghosts' bursts
    after its defeat. Buboo is ported (`squeak/buboo.c`, its bubbles with it, the big one holding KatAM's Waddle Dee): checked
    against the DS by logging (the brain, the weave, the little bubbles' swell, shots and shakes, the circle) and in KatAM (room
    0x323, USA: the weave, the circle, the little and big bubbles (screenshots), an inhale popping the big one, its Waddle Dee
    walking on, the five hits, the crab and its bubbles, the new cluster, the defeat by a real hit; the hits before it by RAM
    writes); not tested: a real mini-boss room, Sword hits on the cluster. All four Squeak Squad-only mid-bosses are now ported.
  - The Squeaks (Squeakers, Spinni, Doc, Storo, Daroach): the Squeakers are ported (`squeak/squeakers.c`, their bombs with them, in
    their four colors: the framework's new palette variants), with their treasure-chest thefts on KatAM's chests (each heads for
    the room's nearest closed chest, takes it, carries it to the spot it was placed at, Squeak Squad's escape hole having no KatAM
    equivalent, and goes in with it; hit, it drops it). Checked against the DS (logs and screenshots of the brain's waits, both
    bombs, the blasts) and in KatAM (USA: room 0x323, red, blue and green ones throwing; room 0x23A, a yellow one taking the chest,
    escaping with it, and inhaled while carrying it: the chest falls back and opens again). Not tested: a treat dropped (one hit
    in five), a big chest taken, the stolen chest back in a later visit, a Squeaker's way over walls; EU and Japanese builds built
    and logic-checked but not played. Not ported: its sounds, the stages' routes and tunnels the DS Squeakers follow. Spinni is
    ported (`squeak/spinni.c`, its shurikens, slash and claws with it; drawn from its two sprites in one character, its weapon a
    layer in its own colors: the framework's new `ExtraFile` and `LayerPalettes`): its brain (chests, Kirby, its spot; its pace
    every 60 frames), runs, jumps and long hops, shuriken throw and dive kick, the inhale it runs from, the knockback at 90% HP
    with its dizzy spell (hits then do no damage, only a treat roll), its chest theft. Checked in KatAM (USA: room 0x323, its waits,
    throws, dive kicks and slash, the knockback, dizzy and get-up, the inhale, Kirby's slide into it while dizzy (no damage, still
    dizzy); screenshots of the layer, shuriken and claws; the claws (a 50% roll) with the random seed set by a RAM write, gone after
    300 frames; room 0x23A, the chest taken mid-dive, carried home, gone with it). Not tested:
    a treat (hit while dizzy, one in five), its pace off the screen, variant 1, against a DS recording; EU and Japanese builds
    built and logic-checked, not played. Not ported: its sounds, light-hit flinch and its own defeat (KatAM's), the dizzy stars,
    the stages' routes and tunnels, its rage when Kirby carries the stage's chest. Its thrown things now go through Kirby as on
    the DS (his body doesn't hurt them nor shows their HP bar: they share Doc's things' hit handler `0x020B4964`). Doc is ported
    (`squeak/doc.c`, its things with it: bolts, bomb, missile, lid, empty saucer; drawn from its two sprites in one character, its
    saucer and robot arm two layers, its composite attacks' other parts held effects; the framework's new `Layers` groups,
    `SqueakHoldEffect`, and palette slots shared by color, `SqueakPaletteKey`, since KatAM keeps only 7 a room and Doc's sprites
    would take 10): its flights along curves and hovers, its routine (chests first, else beside Kirby or a corner, then 5% hover,
    40% gun, 35% claw, 20% cannon), its pace, the inhale brace, the flinch (KatAM's hurt held on the spot in the place of its
    shake, then the hop), the knockout at 90% HP (untouchable till it lies dizzy: the lid over Kirby, tiny Doc popping out, dizzy
    120 frames, the saucer coming back), its chest theft. Checked in KatAM (USA: room 0x323, flights, hovers, gun, claw bomb
    hitting Kirby without an HP bar, cannon smoke, colors against the DS files, the inhale brace, a slide hit's flinch, the
    knockout with its HP set to 22 by a RAM write before a real slide hit, the saucer's return; room 0x23A, the chest taken and
    gone with it; the hover against a DS recording). Not tested: the low bolt, the missile, a treat, a hit while dizzy, its pace
    off the screen; EU and Japanese builds built and logic-checked, not played. Not ported: its sounds, screen shakes, elemental
    hurt poses, its own defeat (KatAM's), dizzy stars, anger when Kirby carries the stage's chest, the stages' routes. Storo is
    ported (`squeak/storo.c`; drawn from its three sprites in one character: the framework's `ExtraFile` now takes several files
    and `Canvas` a bigger frame for its raised mallet; its shockwaves Gao Gao's stars, which now go through Kirby too): its legs
    toward its chest, Kirby or home, the climbing jump, its mallet and body slam, its pace, the inhale curl, the flinch, the
    knockout at 20% HP with its dizzy spell, its chest theft. Kirby's body doesn't hurt it: checked on the DS (Kirby held inside
    it 120 frames, its HP stayed). Checked in KatAM (USA: room 0x323 legs, mallet, slam, flinch, the knockout with its HP set to 6
    by a RAM write before a real slide hit; room 0x23A the chest theft). Not tested: the inhale, a treat, a hit while dizzy, the
    get-up branch, variants 1 and 2; EU and Japanese builds built and logic-checked, not played. To check: whether Kirby's body
    hurts Spinni and Doc on the DS (Storo's doesn't; theirs still take KatAM's touch damage); knocked or hopping Squeaks can pass
    a room's open edge (KatAM rooms without a wall there), walking back after. Daroach in a stage (enemy 0x45, `squeak/daroach.c`;
    drawn with `demo_dancho`, which has no boxes) is ported: his scene takes Kirby's controls (KatAM's cutscene flag), hovers 60
    frames, swirls his cape, holds it 20, points, gives the controls back and flies off backward and up, gone for the rest of the
    visit. Checked against a DS recording (timings, Kirby frozen while the scene plays, the flight and when he's gone) and in KatAM
    (USA: room 0x23A, Kirby holding right stays put till the flight starts at the DS's frame, then walks on; variant 1 faces right
    and flies the other way; the room's spawned bits unchanged after he's gone, so he doesn't come back on that visit). Not
    tested: helpers in his room, Kirby hurt during the scene; EU and Japanese builds built and logic-checked, not played. His boss
    fight (enemy 0x3D: `dancho00`, `dancho01`, `prmBB07`) is with the bosses below.
  - Framework: a looping animation's end is now a pulse on its loop frames, and a composite's ends a frame later, as on the DS
    (logged: Bubble Head's composites and bubble). Bio Spark changed with it (its brain's three waits in the slash now take a loop
    each, 6 frames, as on the DS: checked in KatAM), and Cret's ball now drops after its drop composite's 20 frames, a frame later
    (by the same rule; not logged again: its drop didn't come up on the DS in the test); the others wait on animations they just
    started.
  - Cret and Uja pass through the terrain, held in the stage (Squeak Squad's flag 0x200, pinned down with Uja: Cret had been stopped
    by KatAM's terrain). Checked in KatAM: Cret drifts across room 0x323 and stops at its left edge.
  - Still to do for those: their sounds (Squeak Squad's are the DS's own: KatAM's nearest, or the owner's call); the enemy base
    class's own states (bumping into Kirby, the shared state in water of the ones that don't swim).
  - Bosses (0x37-0x3F, 0x46), on the mid-bosses' framework (mini-boss type 0x40: camera, HP bar, music, KatAM's defeat; where the
    randomizer puts them is with its options, below). King Dedede (0x38) is ported (`squeak/king_dedede.c`): his picks, walk, jump,
    hammer walk and swing, hammer jump, float and air puff, inhale (Kirby caught, chewed and spat out in Squeak Squad's star, let
    out hurt past the arena's side), summon (a parasol Waddle Dee over Kirby), hurt pose and 60 safe frames. The framework now
    sizes each composite part's VRAM by its own frames, gives each part its animation's palette, keeps the `.mca` through layers
    (Dedede's inhale crashed the game without it) and can move a composition's body part first (`BodyLast`). Checked against a DS
    recording (intro, idles, picks, hammer walks and swings, the summon's length) and in KatAM (USA, room 0x323: every move, the
    inhale from a natural pick, the others' first frames set by RAM writes; Sword hits; the defeat from HP set to 1 by a RAM write).
    Not tested: a jump forward, a fall off a ledge's landing stars against the DS, the summon from a natural pick, helpers caught,
    EU and Japanese builds (built and logic-checked, not played).
    Mrs. Moley (0x3A) is ported (`squeak/mrs_moley.c`): her front and back holes and the middle one (her own stage's, `a2s7s2`,
    laid on KatAM's camera view), her pop-ups and looks, 2-4 throws (rocks, a stone, a pebble, a bolt, a ring for Wheel, a bomb for
    Bomb, a can for Fire, a saw for Cutter; grit out of the holes), hiding and coming up by Kirby's side, her dive from the back
    (the leap out of sight, petals, her shadow on Kirby, the upside-down plunge: into a hole she sinks, into the middle she's stuck,
    hurt, struggles and hops into the hole away from Kirby), her last stand at 4 HP (the middle hole, always diving), hits only in
    front (20 safe frames, two petals knocked off). Recorded on the DS in her own stage (loaded into the emulator by repointing
    the stage's files) and checked in KatAM (USA, room 0x323): the pops, throws (Kirby hurt by them), hides, back holes, both
    dive endings with Squeak Squad's frame counts, the shadow, Sword hits and the safe frames, swallowing a ring (Wheel), her last
    stand and the crash taking her last HP into KatAM's defeat (HP set to 3 by a RAM write). Not tested: the bomb's, can's and
    saw's abilities (the ring's path, the same code), a helper's hit's petals, EU and Japanese builds (built and logic-checked,
    not played). Fixed alongside: King Dedede, Big Metalun, Gao Gao and Tedhaun asked for KatAM's screen shake 5, which never
    stops (shook the screen till the fight's end: KatAM's timed 14-frame one now; checked with Dedede's landing).
    Next: Mecha-Kracko (0x39), Yadgaine (0x3B), Bohboh (0x3C), Daroach (0x3D), Meta Knight (0x37), Dark Daroach and Dark Nebula
    (0x3E, 0x3F, 0x46).
  - Left: those bosses; the randomizer options (a Squeak Squad ROM in the ROM Expansion tab, the enemy pool). The Squeak Squad-only
    enemies, the shared enemies' Squeak Squad behaviors, its mid-bosses and the Squeaks are all done.

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
