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
    Mecha-Kracko (0x39) is ported (`squeak/mecha_kracko.c`): its side spots, low line and off-view points (its own stage's,
    `a3s7s2`, from the camera's middle and the floor), the intro, waits, wave, swoop and enemy drops (a Starman, Waddle Dee or
    Waddle Doo, fewer with more out) of its first form; the pause, the cloud breaking, the shudder and the cloud gathering when
    its first form's HP runs out (its second form's HP, held back under the bar's, fills it); the dive into its electric field,
    the bolts crossing, the low sweep under the floor's line and the field swoop of its second form; its eye, a sprite of its
    own looking at Kirby in 8 ways; hit only in its cloud, its field and bolts hurting without being hurt (KatAM's level 6, as
    Gordo). Recorded on the DS in its own stage (its second form forced by a RAM write) and checked in KatAM (USA, room 0x323):
    the first form's moves and drops, the change and every second-form move with Squeak Squad's frame counts and paths, Sword
    and body hits landing only in the cloud (none in 507 field frames and 137 bolt frames with Kirby beside it), its bar taken
    back after its enemies are hit, KatAM's defeat at 0 HP with the eye gone. The change was forced (HP written down to the held
    back 80), the defeat too (HP 5). Not tested: a whole fight played through by hand, helpers' hits, EU and Japanese builds
    (built and logic-checked, not played). Fixed alongside in the framework: a part sprite takes its own frames' VRAM (rebuilt
    when another composition needs more or less) and asks again when KatAM's 512-tile heap is full (Mecha-Kracko's field and
    bolts drew other sprites' tiles), and a mid-boss keeps its bar's full HP and a held-back part of its HP of its own.
    Seen: Kirby flashing after a hit hurts a mid-boss he touches (KatAM's body touch with his level 6 over its 3; every ported
    mid-boss alike; left as it is).
    Yadgaine (0x3B) is ported (`squeak/yadgaine.c`): its chase (Squeak Squad's endless scrolling stage: here the screen stays and
    the scroll is taken out of everything of the fight, Yadgaine and its things moving at their speed less the scroll's and the
    Kirbys carried back at its speed; its front a wall that pushes Kirby), Daroach bringing it in, the intro and rush in, its
    rounds and picks (dash forward, back, swipe, lunge, shot with its puff and capsule, short and long pauses), the rocks and
    gordos falling over the view's right, its two legs (their own HP; a hit on its body goes to them too; broken for good,
    hurting by touch), the debris blocks its resting feet knock up (solid tiles once landed, stepping back as the screen would
    scroll, broken under it), its second form after 72 damage (its cracked body, no-leg picks). Recorded on the DS in its own
    stage and checked in KatAM (USA, room 0x323): the intro's 120 and 150 frames, the rush's 64, every move's speeds and frames,
    the rocks, gordo and blocks, Sword hits (the same damage on its back leg, 20 safe frames), the legs' broken look and the
    second form (HP written down by a RAM write, 63 damage at once), KatAM's defeat at 0 HP with its legs gone. Not tested: a
    whole fight played through by hand, standing on a landed block, helpers' hits on the legs, EU and Japanese builds (built
    and logic-checked, not played). Its green spiked shell (Squeak Squad's background layer, 154 by 208 pixels, 312 tiles), as
    the owner asked: cut to what its fight shows, then (the owner: half size was too coarse beside its body) at two thirds of
    its size (134 tiles) drawn half as big again by the GBA's sprite scaling, its pieces written into KatAM's OAM by hand;
    Squeak Squad's effects wait for room in KatAM's sprite heap (a rock's burst left out when it would leave too little for
    KatAM's own), the shell takes Daroach's palette slot once he's gone, goes when it's beaten (as its legs). Checked in the
    emulator (USA, room 0x323, the fight from its start, 1,500 frames, Kirby idle with helpers: a forced start, not a
    playthrough): its place and look beside the DS's and the half-size one, 131 lag frames against 133 without it, no sprite
    drawn from missing VRAM, KatAM's 7 enemy palettes holding, the GBA's sprite time per line passed only as Kirby was beaten
    (one line of the shell cut on one frame); European and Japanese builds: its look in the same fight. Not tested: a fight
    played by hand (Kirby's own attacks' effects in the tight heap).
  - **Owner's call**: Yadgaine's rock bursts with the sharper shell: its 134 tiles leave KatAM's sprite heap room for a rock's
    burst (64 tiles) only with a rock or so in the air, so most are left out (4 of 11 shown in the test; 9 with the half-size
    shell). The choices: rocks of a kind share one block of sprite memory (they'd spin in step; about 32 tiles back), a
    smaller margin kept for KatAM's own effects (48 tiles now: Kirby's attacks drawn from bad VRAM if it's too small), or left
    as it is. Not ported: the floor crumbling behind it, the DS's camera
    jolts. Fixed
    alongside in the framework: each ported object's sprite takes as many tiles as its own frames need (not its type's
    biggest), so a boss's legs and showers of things fit KatAM's heap; HP-bar names can have a letter no KatAM name has
    (Yadgaine's "Y", drawn in their style).
    Bohboh (0x3C) is ported (`squeak/bohboh.c`): its intro (far off in the background across the view, then in big to its
    point), its waits and picks (by how many of the stage's enemies are out), its leap from point to point, its charge and its
    pass dropping fires every 30 frames (the pass back far off in the background), its swoop at Kirby's feet that grabs him,
    carries him to the other point and throws him down into the lava (hurt), its dive when the swoop ends in the lava (20 of its
    350 HP: 4 of 70 here), its 120 frames in the lava while fires and the stage's
    enemies (a Waddle Dee, Pengy, Sir Kibble and Hot Head, KatAM's own, one of each at most) fall from the view's top, its rise
    and flare. Recorded on the DS in its own stage (a5s7s2, reached through a5s7s1's door; its maps are stored uncompressed, so
    the stage-loading trick needs a test ROM with them compressed) and checked in KatAM (USA, room 0x323): every move's speeds
    and frames, the grab, carry and throw (a helper and Kirby taken; Kirby hurt in the lava), the dives into the lava (4 HP each
    time), the lava phase's fires and enemies (a Sir Kibble and a Waddle Dee dropped), hits landing in the lava (the helpers'
    10), KatAM's defeat when a dive takes its last HP (HP written down to 4 by a RAM write), the HP bar's name. Not tested: a
    whole fight played through by hand, EU and Japanese builds (built and logic-checked, not played).
    Corrected since (Squeak Squad's hit test read from its code, `0x0201322C`, and checked per state on the DS): a frame with no
    hit box of its own is hit by the character's first box, and only boxes flagged 0x8000 (and grabs and holds, and the safe
    frames after a hit) make it untouchable. So Bohboh is hittable everywhere but far off and in its dive (not only in the lava
    and its dive, as first ported); Mecha-Kracko in all its poses, its field and bolts too (not only in its cloud). The framework
    now marks the flagged animations (`SqueakBoxes`' `untouchable`): King Dedede's chew, spit and hurt (and from his catch on),
    Tedhaun's grab and hold, Buboo's bubbles' pops, the Squeakers' grab of the chest and their bomb's blast, Bio Spark's kunai and
    hiding (and from its grab to its landing), Bun's and Gaw Gaw's holds. Checked in KatAM by its hit level each frame (Bohboh: 6
    in its dive, the safe frames and its holds, 3 elsewhere; a forced log, not hits landed); the others built, not replayed.
    For the owner to decide: the lava. Squeak Squad's stage draws a lava pool 128 pixels across the middle of the arena, and
    Bohboh brings in an invisible object over it that hurts whoever touches it (and makes the lava's "floor" its dives and Kirby's
    throw end in). KatAM's rooms have no lava, so here that hurting strip is invisible on the floor's middle, as ported 1:1. It can
    stay, be made harmless (kept only as the line its dives and throws end in), or be shown somehow. Also: KatAM's view is 32
    pixels shorter than the DS's, so at its two points (96 over the floor, as on the DS) the top of its flames can go past the
    view's top in rooms whose floor sits high in the view.
    Daroach and Dark Daroach (0x3D, parameter 0 and 1) are ported (`squeak/daroach_boss.c`): his idle and picks by Kirby's zone,
    his warps (and chains of three), hops, his beam (a hop to Kirby's height, the charge following him, 120 frames of beam reaching
    31 to 332 pixels ahead; Dark Daroach's taller and backing off), his bomb (Dark Daroach's lands, waits 180 frames and burns a
    column 120 frames), his three stars (bouncing, gone after 100 frames or on touching Kirby, inhaled to spit back), untouchable
    in his warp, 10 frames safe after a hit, HP 80 (Squeak Squad's 400); Dark Daroach in dancho01's darker colors with his name on
    the HP bar. Recorded on the DS in his stage (a6s7s2: his moves; his hit test per state) and checked in KatAM (USA, room 0x323):
    the picks, warps, hops, beams (Kirby hurt by the beam at 80 pixels; the beam's 16 parts drawn), bombs (Dark Daroach's whole
    sequence), stars, Dark Daroach's look. Hits checked by forced HP drops (kept and 10 safe frames while hittable, undone in his
    warp), not by an attack landed (Kirby's slide doesn't hurt mini-bosses, KatAM's 0x80; no ability attack was scripted). Not
    tested: a whole fight by hand, his defeat, EU and Japanese builds (built and logic-checked, not played).
    Meta Knight (0x37) is ported (`squeak/meta_knight_kss.c`): his intro (150 frames, the screen rumbling), his walks, back-offs,
    runs and follows, his picks by Kirby's place (ahead and close: follow, back, dash, double slash, thrust, tornado, beams; above:
    up slash, wave, jump; his zone: tornado, beams, follow, walk, run; elsewhere: walk, run, a drop through the thin floor, a jump),
    jumps, hops, falls, backflips, his dash, up slash and double slash (his sword's reach hurting on their frames), his thrust (the
    landing's stars, a Sword when inhaled), his beams (one or three), his wave, his tornado (it catches Kirby, lifts him 2 pixels a
    frame with sparkles, lets him go hurt), his dodges and counter-hops, 20 safe frames after a hit, HP 80 (Squeak Squad's 400);
    his sword drawn over his body as on the DS. Recorded on the DS in his stage (a7s7s2) and checked in KatAM (USA, room 0x323):
    each move forced by a RAM write to its state and compared to the DS recordings (beams hitting Kirby; the tornado catching Kirby
    and a helper; the thrust's stars), then two unforced fights of 3600 and 5000 frames (Kirby idle, with and without helpers):
    walks, runs, jumps, falls, dashes, double slashes, thrusts, beams (one and three) and a tornado picked on their own, Kirby hurt
    by them and once beaten, the helpers' hits taking 10 each (80 to 24; hits in the safe frames undone); his defeat (HP written
    down to 2 by a RAM write, then a helper's hit: KatAM's mini-boss defeat, his stars burst, gone). A fix found on the way, for
    every character with composite parts: a part's sprite is now made on the cell it shows (made on cell 0, his tornado's
    103-tile frame ran over his sword layer's VRAM and garbled it while the tornado was out). Not tested: a dodge or counter-hop
    in a real fight (the helpers' hits came outside the modes he dodges in), the wave picked on its own (Kirby never above him;
    tested forced), the drop through a thin floor in KatAM (room 0x323 has none), a whole fight by hand, EU and Japanese builds
    (built and logic-checked, not played).
    Dark Nebula (0x46 and 0x3F) is ported (`squeak/dark_nebula.c`): his coming (the dark star flying in, forming, his eye opening,
    looking, glinting), his start (the black star, his eye's twinkle, the screen whitening to his form and clearing), his three
    forms and their picks: fire (dashes; the fireball, its burst, fire wall, flames along the floor and the floor tiles they set
    burning), spark (waves; bolts to the view's corners and the cage), ice (orbit; the ice pillar), the four shared star moves,
    his eye's shots, the form change (3-5 moves, then 240 frames, a swoop to the top middle, the twinkle and whitening), 60 safe
    frames after a hit, HP 60 (Squeak Squad's 300). Recorded on the DS in his stage (a8s4s6) and checked against it move by move
    (each forced by a RAM write in both games from the same spot: his spots, states and things frame by frame, screenshots side by
    side), then in KatAM (USA, room 0x323): an unforced fight of 2400 frames with helpers (fire and ice forms, the form change),
    and his defeat (HP written down to 2 by a RAM write, then a helper's hit: KatAM's mini-boss defeat, gone). Approximated: the
    floor's burning time (the DS's average, 47 frames; 42-52 there, its own timing not found). Not ported: his sounds, the stage's
    background tinted by his form, the fire's heat (harmless on the DS), stars turned back by reflecting attacks (KatAM has none),
    his own defeat (KatAM's instead). Not tested: a whole fight by hand, EU and Japanese builds (built and logic-checked, not
    played).
    The dark star (0x3E, the one Kirby chases through a8s4s3 and a8s4s4 to Dark Nebula) is ported (`squeak/dark_star.c`): it
    appears facing away from Kirby and heads past him (120 frames drifting and bouncing), then hops ahead of him at his speed +
    0.88, waiting while it's more than 64 pixels ahead and keeping to his height; at the room's end it waits for him, then loops
    (five Bezier legs), flies off out of view and loops again. Harmless and untouchable, as on the DS. Recorded on the DS in both
    stages (Kirby driven by a script) and checked in KatAM (USA, room 0x323: the drift, the hops and their waits, a hop frame by
    frame against the DS's, the end and its loop both ways, Kirby touching and inhaling it). **Owner's call**: Squeak Squad's route
    is its stage's own (points and zones in the map); KatAM's rooms have none, so the corridor's route is laid on the room it's
    in: its line under the floor it first bounces on, its end and loop at the room's side it heads for (where the DS's camera
    locks on the stage's end, KatAM's camera just follows Kirby, so part of the loop can be off the screen). The shaft's route
    (a8s4s4: the same moves going up) isn't ported. Say if the star should do something else in KatAM's rooms. Not tested: EU
    and Japanese builds (built and logic-checked, not played); the randomizer doesn't place it (its route is a stage's).
  - The whole cast is ported: the Squeak Squad-only enemies, the shared enemies' Squeak Squad behaviors, its mid-bosses, the
    Squeaks, the bosses (0x37-0x3D), Dark Nebula (0x3F, 0x46) and the dark star (0x3E).
  - **Randomizer options: done** (the ROM Expansion tab's "Kirby: Squeak Squad" group, under NiDL's: Load Squeak Squad ROM, Unload,
    Include Enemies, Include Mini-Bosses, Include Behaviors; see the READMEs). A Squeak Squad ROM works with or without a NiDL ROM
    (the owner's rule): alone, only what its characters need goes in. Checked: NiDL + Squeak Squad's expansion byte for byte as
    before; NiDL-only randomizations (random behaviors and HP, two seeds) byte for byte as before; Squeak Squad-only and NiDL +
    Squeak Squad randomizations of the USA, European and Japanese releases (all 18 enemies placed by spot: flyers in flying spots,
    Gussa in water spots; the 4 mid-bosses on their mini-boss types; the natives' new behaviors; HP options on their HP table
    only); the three releases with both built and logic-checked (every room reached, every big switch, nothing trapped, with and
    without thin-floor drops). In the emulator (a new game started in the room: a forced start, not a playthrough; USA, Squeak
    Squad only): Big Metalun, Gao Gao, Tedhaun and Buboo fight in their randomized KatAM mini-boss rooms (HP bar, camera); Gussa
    swims in Cabbage Cavern's water; a room of flyers. Not tested: the window itself (it can't run here: compiled only), the
    spoiler log (compiled only), a whole playthrough.
  - **Owner's call**: Squeak Squad's bosses aren't in the pools. Tried in KatAM's mini-boss rooms, Mecha-Kracko and Mrs. Moley fit,
    but Yadgaine (at its arena's edge, a leg showing) and Dark Nebula (above the locked camera) were mostly off-screen. They could
    go in mini-boss rooms whose arena fits them, or wait for Squeak Squad's own rooms.

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
