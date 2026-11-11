# Open requests

What the owner has asked for that is still to do. When an item is finished, take it out. The design rules for doors, mirrors and
hubs are in [Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 52. Kirby: Squeak Squad in Amazing Mirror
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
    and logic-checked but not played (its sounds: section 9's Squeak Squad's sounds). Not ported: the stages' routes and tunnels the DS Squeakers follow. Spinni is
    ported (`squeak/spinni.c`, its shurikens, slash and claws with it; drawn from its two sprites in one character, its weapon a
    layer in its own colors: the framework's new `ExtraFile` and `LayerPalettes`): its brain (chests, Kirby, its spot; its pace
    every 60 frames), runs, jumps and long hops, shuriken throw and dive kick, the inhale it runs from, the knockback at 90% HP
    with its dizzy spell (hits then do no damage, only a treat roll), its chest theft. Checked in KatAM (USA: room 0x323, its waits,
    throws, dive kicks and slash, the knockback, dizzy and get-up, the inhale, Kirby's slide into it while dizzy (no damage, still
    dizzy); screenshots of the layer, shuriken and claws; the claws (a 50% roll) with the random seed set by a RAM write, gone after
    300 frames; room 0x23A, the chest taken mid-dive, carried home, gone with it). Not tested:
    a treat (hit while dizzy, one in five), its pace off the screen, variant 1, against a DS recording; EU and Japanese builds
    built and logic-checked, not played (its sounds: section 9). Not ported: its light-hit flinch and its own defeat (KatAM's), the dizzy stars,
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
    off the screen; EU and Japanese builds built and logic-checked, not played (its sounds: section 9). Not ported: screen shakes, elemental
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
  - The enemy base class's shared moves (the owner: "Add the enemy base class shared moves"):
    - In water, for the ones that don't swim: done, the DS's (its base class's +0x80 check, `0x02021408`, at every state's start;
      +0x6C, `0x02021D5C`, and +0x70, `0x02021D30`): splashing in, one of its two hurt looks at random, still and sinking 0.4 a
      frame for 42 frames, then defeated. KatAM's own drowning is the same move (sinking 0.25 a frame, 41 frames), so the port
      keeps KatAM's look and poof and takes the DS's numbers (`squeak.c`'s SqueakDrown: Perara, Crimp, Maiga, Cret, Uja, Bio Spark,
      Starman, Bubble Head, Gaw Gaw, Acchi, Bun, Metalun, the Squeaks; the mid-bosses and bosses never drown, as before; what
      characters throw drowns as KatAM's). Checked in the GBA emulator (USA, room 0x461, its Acchi moved into the water by a RAM
      write): it flips over, sinks 0x66 (0.4) a frame for exactly 42 frames, then the poof. Europe and Japan (`sub_0809DA30`
      added to their symbol lists): the same test, the same numbers. Not tested: an enemy walking into water by itself.
    - Bumping into Kirby: the DS runs its hurt pair for it (+0x48, `0x02022CB4`: a hit-stop shake from a table, `0x02185E04`,
      then the knock-back, +0x4C/+0x50, `0x02022B4C`/`0x020229C0`: speeds and frames from a shared parameter block at the body's
      +0x1B4, then on landing back to its moves, or defeated). KatAM does the same on a bump (its hurt shake and knock-back, then
      the character starts over), and that's what the port still uses: the DS's own numbers weren't found (that block isn't
      the enemies' `Sl0` files, and no enemy was caught live in the DS emulator to read it). Left as is, KatAM's.
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
    the owner asked: cut to what its fight shows; at half its size drawn twice as big, then two thirds drawn half as big again,
    it was too coarse beside its body's own pixels (the owner: the body too detailed beside it), so (the owner's pick of four
    mockups) the whole boss is at three quarters of the DS's size, drawn pixel for pixel: its body, legs and shell
    (`KatAMExpansionSqueak.cs`'s `YadgaineScale`, 174 tiles for the shell), its pixels shrunk so outlines stay one pixel wide and
    unbroken (the owner: jaggies; each DS pixel to the pixel its middle lands in, outlines winning, stairs' doubled corners out) and its boxes, its spots (36, 48 and 83 pixels
    in: the shell's cut left edge off the screen), how far ahead things start, its moves' own speed and its knocked-up blocks'
    flight (each move and block as long as the DS's); what it throws, the rocks and the scroll as they were. Its pieces written
    into KatAM's OAM by hand; Squeak Squad's effects wait for room in KatAM's sprite heap (a rock's burst left out when it would
    leave too little for KatAM's own), the shell takes Daroach's palette slot once he's gone, goes when it's beaten (as its
    legs). Checked in the emulator (USA, room 0x323, the fight from its start, 1,500 frames, Kirby idle with helpers: a forced
    start, not a playthrough): its look and place, the shell's top and left edge, its body's parts together, its outlines one
    pixel wide (the emulator's own pixels, before and after); 86 lag frames
    against 131 with the scaled shell before and 133 without a shell (its spots shrunk, the blocks at the DS's flight piled up
    and it lagged at 271: their flight shrunk too; at the old spots 122, the shell's edge showing); no sprite drawn from bad
    VRAM, the 7 palettes holding, the GBA's sprite time per line never passed (1,016 of 1,210 cycles at worst), 9 or 10 rock bursts
    shown (4 with the scaled shell, about 11 without one). European and Japanese builds: its look in the same fight. Not
    tested: a fight played by hand (Kirby's own attacks, its boxes at their new size against him). Not ported: the floor
    crumbling behind it, the DS's camera jolts. Fixed
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
    floor's burning time (the DS's average, 47 frames; 42-52 there, its own timing not found). His sounds: section 9. Not ported: the stage's
    background tinted by his form, the fire's heat (harmless on the DS), stars turned back by reflecting attacks (KatAM has none),
    his own defeat (KatAM's instead). Not tested: a whole fight by hand, EU and Japanese builds (built and logic-checked, not
    played).
    The dark star (0x3E, the one Kirby chases through a8s4s3 and a8s4s4 to Dark Nebula) is ported (`squeak/dark_star.c`): it
    appears facing away from Kirby and heads past him (120 frames drifting and bouncing), then hops ahead of him at his speed +
    0.88, waiting while it's more than 64 pixels ahead and keeping to his height; at the room's end it waits for him, then loops
    (five Bezier legs), flies off out of view and loops again. Harmless and untouchable, as on the DS. Recorded on the DS in both
    stages (Kirby driven by a script) and checked in KatAM (USA, room 0x323: the drift, the hops and their waits, a hop frame by
    frame against the DS's, the end and its loop both ways, Kirby touching and inhaling it). Its route (the owner: "Is the dark
    star route ported? If not, port it."; it wasn't: the corridor's route was laid on whatever room the star was in, and the
    shaft's wasn't there at all): in Squeak Squad's own converted rooms (8-4-3, the corridor, and 8-4-4, the shaft) the star now
    goes by the room's own route as the DS does (the map's section 9: a zone per tile, the points; `gSqueakDarkStarRoutes`,
    `SqueakDarkStarRoute()`): hopping from its zone's point toward the next, backward at Kirby's speed + 0.88 and forward at his
    speed + 0.2 where the route turns back, waiting while he's more than 64 pixels behind or 16 below, ending within 128 of the end
    point and looping from there once Kirby's camera is held by the room's camera area. It's made at its own spot by the stage's
    first door (the door rule left it out of 8-4-3 before). Checked in KatAM (USA; Kirby put each frame where and as fast as the DS
    recordings had him, the star free): the corridor within a few pixels of the DS's star at first and about 25 later, its end and
    loop on the same frames; the shaft up the same zigzag to the same end (14 pixels off) and held there as on the DS, but off the
    DS's path for a while half-way (a re-hop under Kirby falling on the other side of a zone's edge by a pixel). In KatAM's own
    rooms (no route) it keeps the corridor's, laid on the room. Not tested: a real playthrough of 8-4-3 and 8-4-4 by hand (the
    checks moved Kirby by RAM writes), the corridor again after its spot moved 8 pixels down, EU and Japanese builds in the
    emulator (built and logic-checked); the randomizer doesn't place it elsewhere (its route is a stage's).
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
  - **Squeak Squad's bosses and the Squeaks in the pools: done** (the owner: "Add Squeak Squad bosses to randomizer pools,
    including the 'chase sequence' mouses as minibosses"). The window's "Include Bosses" (beside Include Mini-Bosses) adds the
    9 bosses (King Dedede, Mrs. Moley, Mecha-Kracko, Yadgaine, Bohboh, Daroach, Dark Daroach, Meta Knight, Dark Nebula) to the
    mini-boss pool, on KatAM's mini-boss spots; Spinni, Doc and Storo (the Squeaks that run off with the chests) are in the
    mini-boss pool with the 4 mid-bosses. Both go on mini-boss type 0x40 (`KatAMExpansionSqueak.cs`'s `SqueakAsMidboss`,
    `SqueakBossPool`, `SqueakMidbossPool`); the HP options touch the 4 mid-bosses' HP only, as before. Outside their own rooms
    the bosses that stand on the DS's floor height (Dark Nebula, Daroach, Dark Daroach, Bohboh, Mecha-Kracko) take their floor
    from the bottom of KatAM's locked arena (`squeak.c`'s `SqueakBossFloor()`), so they're on screen; a Squeak as a mini-boss
    gets the mini-boss's HP bar and camera (`SqueakMidbossUpdate`).
    - Checked: two randomized seeds (USA, Squeak Squad only so the pool is all Squeak's): the 9 bosses and the 3 Squeaks all
      placed on KatAM's mini-boss spots with the mid-bosses. In the GBA emulator (USA; KatAM's mini-boss template patched to
      each one, a new game started in the room: a forced start, not a playthrough; Kirby walked in with the buttons): Dark
      Nebula, Daroach, Dark Daroach and Bohboh on screen and fighting in rooms 0xE6, 0xF5 and 0xD2 (Dark Nebula was above the
      locked camera before); King Dedede, Mrs. Moley, Mecha-Kracko and Meta Knight in 0xE6 and 0xF5, Meta Knight also in 0x7C
      (he closes in, swings, Kirby is hit); Yadgaine in 0xD2 (in 0x7C, 0xE6 and 0xF5 Kirby went down from its hits: that's the
      fight, Kirby's HP ran out, the HUD showed it); Spinni, Doc and Storo fight with the HP bar in 0x7C, 0xE6 and 0xF5. The
      three versions built and logic-checked (unchanged). Not tested: room 0x94 (the forced start put Kirby in a bad spot), every boss in every KatAM
      mini-boss room, beating each one, Europe and Japan in the emulator for the bosses, the window (compiled against stubs only),
      the spoiler log (compiled only).
- **Squeak Squad's rooms** (the owner: "port all Squeak Squad rooms along with their backgrounds, and tilesets. Some Squeak
  Squad rooms are bigger than the limit allowed by KatAM, so make sure that these fit in the game without segmenting them.
  Reference the map guide we generated for NiDL's maps. Replace all bubbles with their respective item counterparts. Likewise,
  replace the enemies with the already ported ones. You can compress tilesets or backgrounds so they don't lag the GBA out."):
  - Scope (the owner's pick): the 8 worlds' stages only, about 480 rooms (the stages list 579; left out, as NiDL's were: the
    test rooms of flat placeholder tiles, Prism Plains' stages 8 and 9 and the like, and area 9, the Arena's copies of the boss
    rooms). Every room whole: the biggest is 4,032 blocks (Cushy Cloud 1's first, 96 by 42), over the 3,584 KatAM's rooms were
    raised to for NiDL (`rooms.c`'s BIG_BLOCKS), so that's raised again rather than splitting any.
  - Joined to the game (the owner's pick): a hub of its own as NiDL's, reached from Central Circle, never from the NiDL hub;
    each world a level laid out by the rulebook; the bosses' shards back to that hub. The owner, later: "The way in is a copy
    of the main game's central circle. We will later decide on position and visuals, the entrance door to the area is a grey
    door like the pink door for NiDL to the hub area." and "The world map should get pathing like NiDL did yes. Follow the
    rules we defined for map structure, we have rules in place for a reason." (so the world-map item above is to be built).
  - Chests' looks (the owner): "Platinum chests in Squeak Squad can use the same open/closed graphics in Squeak Squad."
  - Gimmicks (the owner): "Continue with the gimmicks and don't leave these out." Warp stars and cannons: "Ensure Star and
    Cannon Warps are in the rooms and work."
  - Squeak Lair doors (the owner, on the hidden doors to the gold-pile rooms): they open only when Spinni, Storo or Doc steals
    a big treasure chest in the stage and carries it into a temporary door; following quickly leads to the hideout (the gold
    pile) and a mini-boss fight with that Squeak to win the chest back; too slow, and the Squeaks board the door up.
    The 37 hidden doors of the ported rooms (of the DS's 46 with flag 4) are invisible doors Up opens, nothing drawn (the DS draws
    nothing there either); 34 lead into their gold-pile room; Up does nothing at three: two in 2-8-7 (rooms the DS lists but never
    had) and 4-6-3's (its cell in mid-air). Checked in the GBA emulator (USA, Kirby held on each door's cell by RAM writes, then
    Up), sent as two sheets. The owner then asked for the lairs as mini-boss interludes: see **The Squeak Lairs** below (the
    hidden doors to the gold-pile rooms stay as they are).
  - Hidden keys and Star Seals are dropped (the owner: "We are dropping hidden keys and star seals so an room that's unlocked
    via that method has to be revealed. Only the Squeak Squad ability room can be unlocked via the hidden map.").
  - Bubbles (the floating `yakumono/bubble` gimmicks): food ones become KatAM's matching food; ability ones become the enemy
    that gives that ability in KatAM (the owner, changing the earlier pick of food: "Convert the floating bubbles holding
    abilities into their respective enemy in-game. That makes the most sense.").
  - Chests (the owner): Squeak Squad's levels have their own, so none are added. Those holding ability scrolls, Squeak Squad
    spray paints, the secret map that unlocks Squeak Squad's ability room, and vitality halves hold those items, added from
    Squeak Squad (part of this request, after the rooms); the other chests hold consumables.
  - Done only when every room is ported to KatAM's engine 1:1 along with those items and the map items below (the owner: "You
    will not finish until all rooms are ported to KatAM's engine 1:1 along with the items requested.").
  - Enemies: the ported ones (and KatAM's own where Squeak Squad's are KatAM's).
  - Squeak Squad's far backgrounds are 3D textures on the DS (`map/d_viw_*.nsbtx`, 40 of them), not tiles: they're drawn
    into KatAM's background layer, reduced to fit (the owner allows compressing tilesets and backgrounds).
  - Progress (`KatAMExpansionSqueakRooms.cs`; the formats in "Squeak Squad Notes.md", "Stage rooms, for porting them"):
    - Decoded: the rooms' layers, tilesets, collision, doors, gimmicks, tile animations; the objects files' enemies and items
      (food, chests, ability bubbles and stands); the far views. Checked by screenshots in the DS emulator (Kirby moved beside
      each gimmick and item).
    - Converted, checked offline (not in the game yet): every room's three layers as KatAM's one foreground (401 of 493 rooms
      exactly; the rest within near-identical shades; the 3 busiest rooms share a few tiles to fit KatAM's 768); all 39 far views
      as KatAM backgrounds (3/4 size, 8 palettes, at most 900 tiles); the collision (slopes, thin floors, ladders, spikes, water,
      star blocks), overlaid on converted rooms to check it lines up.
    - In the ROM (the e2e harness's `KSSROOMS=1`; no option in the window yet): all 493 rooms as rooms 0x400 on, the room tables
      grown and moved, rooms of up to 4032 blocks loaded whole (rooms.c's buffers re-laid, the EWRAM heap smaller: measured), the
      camera down to the room's bottom. Checked in the GBA emulator (forced starts, not a playthrough): Prism Plains 1's first
      two rooms (drawing, background panning, slopes, star blocks inhaled away), Cushy Cloud 1's first (the biggest), Secret Sea
      1's third (water), a ladder in Cushy Cloud 6's second; NiDL's mirrors all swept again (no change), 3 versions built and
      the logic checks run (with and without drops).
    - Objects in the ROM (`SqueakObjects()`): every room's enemies (KatAM's own with Squeak Squad's brains as their behaviors,
      the ported characters with their variants), mid-bosses and bosses (each fight held on its screen by NiDL's arena object),
      food and 1UPs (taken once per visit, as KatAM's), and the ability spots and floating bubbles as the enemy giving that
      ability (on the floor under the bubble). Checked in the GBA emulator (forced starts, not a playthrough): Prism Plains 1's
      Waddle Dee, cherry and 1UP, its tutorial's Sword spot as a Sword Knight, Gao Gao in its arena room (5-3) and in a wide room
      (2-3: the camera held on its screen), King Dedede's fight (its screen held from the start); all 493 rooms started at
      Squeak Squad's own Kirby start and run 720 frames walking and inhaling both ways (no stall, Kirby stays in the room);
      Europe and Japan built and 11 rooms run the same way.
    - Chests: KatAM's small and big chests where Squeak Squad's are, their opened bits saved of their own (KatAM's 128 are all
      used), mapped in their rooms' 9ROM lists (unmapped, opening one hangs the game: the owner's warning too). Each holds a
      consumable for now. Which treasure each held is known (by its number's range: the notes' "The treasures"). The big ones are
      Squeak Squad's platinum chests, drawn with its own closed and open graphics (the owner's), each on the floor under its spot.
      Checked in the GBA emulator (forced: Kirby held standing on each chest pressing Up, its saved opened bit read after): 101 of
      the 103 open (that first sweep only checked the game ran on: a big chest set a cell too low had sunk into the floor and fallen
      out of Prism Plains 3's fifth room, and 26 more were out of Kirby's reach there); the other two have an enemy on them (a
      Sparky in Nature Notch 8's seventh room, a Flamer in Secret Sea 2's sixth) that keeps knocking Kirby off; a platinum chest
      shown closed, opened (its lid up, its 1UP taken: lives 2 to 3), and in its rock alcove; KatAM's own chest (room 0x6C) still
      opens.
      The tutorial signs (Prism Plains 2's "Tap to pop!", "Find treasure and move on.", "Mix bubbles!") are left out (the owner:
      "no need to bring" them; checked in its three rooms). The room sweep now also opens the pause screen in each room it
      can't tell is running (a VBlank counter keeps counting through a hang): all 493 live. Europe and Japan: 10 chests opened
      and 13 rooms run the same way; the 3 versions built and the logic checks unchanged (with and without drops).
    - Doors: every door leading to another room of its stage (562) is a KatAM door with its 8ROM record and 9ROM entry, Kirby
      landing on the door it names (Squeak Squad's doors are all entered with Up: checked on the DS, one of each kind). Its
      hidden ways (flag 4, to the gold-pile treasure rooms) are doors drawn as on the DS: none. Checked in the GBA emulator
      (forced: Kirby held on each door's cell pressing Up, not a playthrough): all 562 in the USA ROM land in the right room on
      the right door, none hangs; every tenth in the European and Japanese ROMs the same. The goals (the stage's end) and how
      Kirby gets into a stage wait for the levels' structure.
    - Tile animations (section 12: grass, water, flames, glows in 45 rooms) as KatAM's foreground animations, every frame drawn
      through the three layers. Checked in the GBA emulator (forced starts): shots a few frames apart in 4 rooms (USA) and 3
      (Europe, Japan), the 42 animated rooms the sweep starts in run live.
    - Collision read as the DS reads it (the blocks grid first; the notes' "Collision, the blocks grid first"): doors behind star
      blocks are behind them, as on the DS (checked in the GBA emulator, a real Sword cutting the blocks: 0x423, 0x48B, 0x5A9 lead
      where the DS's do; 0x5CB's is in a wall of them too; those two are 0x5A7 and 0x5C9 since 5-7 was left out). The door sweep's other failures are the old ones (enemies or a ledge
      where Kirby is held). All 493 rooms run live again.
    - Warp stars (gimmick 0xD): all 12 stars to get on are KatAM's warp stars flying Squeak Squad's own rides (the DS's paths,
      through the rooms a ride passes, into the room it ends in), landing where the DS lands Kirby. Checked in the GBA emulator,
      each ride from its star (Kirby walked or jumped onto 1-4-1's and 8-4-3's; the others started on the star): every one lands
      within 4 pixels of the DS's spot, found by riding each on the DS with the stage's room table rebuilt.
    - Cannons: the eight-direction cannons (gimmick 0xB: 47 in 6-3-5/6, 4-3-6, 2-1-3, 3-6-2, 4-4-6, 6-4-1, 8-1-5) as KatAM's,
      fixed ones firing on their own after the DS's wait, turning ones with A, and every shot carried on to the next cannon or a
      wall as the DS's (`chest.c`); the fuse cannons (gimmick 0xC: 3-4-2, 5-1-1, 6-2-3) as KatAM's fuse cannons with their fuses.
      The bricks and metal blocks in their way break under the shots, as on the DS. Checked in the GBA emulator (Kirby put in the
      first cannon, then left alone): 4-3-6's chain goes up, right, down, left, down, right as the DS's; 3-4-2's fuse, lit with Fire,
      burns to the cannon, which shoots Kirby into 3-4-3 at its door 0. 8-1-5's turning cannon (eight doors round it, each behind
      metal blocks): Kirby comes in through its middle door into the cannon, fired with A in each of the 8 ways (one run each), the
      shot breaks that way's metal blocks and reaches the door's cell; then Kirby put on that door with Up goes to its room, all 8
      (the up door after a second shot out of the cannon: going straight up he falls back into it). The cannon's spot is its foot
      on the DS, the shot leaving 8 pixels up: placed there, the 8 ways go through the doors' middles (8 pixels lower, the sideways
      and lower shots stopped a block short); 4-3-6's chain checked again after it, the same route. 5-1-1's and 6-2-3's fuses
      (Kirby held on the floor beside the fuse's end, given Fire, breathing on it, then put in the cannon): both burn to their
      cannon, which shoots him into the room and door the DS's names (5-1-6's door 0, 6-2-4's door 1), where he lands and stands
      (5-1-1's first ran him into a pit: KatAM's bounces forward after a cannon's landing, now left out in these rooms; its fuse
      object is a cell over the floor, which Fire's breath from the floor reaches). 5-1-1's ride checked the same way in the
      European and Japanese ROMs too (landing on 5-1-6's ledge, standing). For the Squeak Squad levels' logic (not wired yet): 8-3-8's door at (128, 7) is behind a band of metal blocks
      in a pillar with no floor in front (a Metalun beside it: the DS's Metal ability, which KatAM's Metalun doesn't give); KatAM's
      metal blocks break under Hammer (checked: Normal, Sword, Fighter, Smash and Fire don't) and cannon shots.
    - The levels and the hub (`KatAMExpansionSqueakLevels.cs`; the rulebook's rules 4 and 5, their Squeak Squad bullets): each
      world's stages laid out as a tree by the rulebook (branch doors and their mirrors back, the goals: the deepest to the boss
      stage, the others into goal games or the next world, shortcuts, a hidden two-way door into the next world's first stage),
      136 KatAM mirrors in all (the DS's own doors as they are). The hub: a copy of Central Circle (room 0x5E9, after the worlds'
      rooms), reached by a new mirror on Central Circle's floor (11, 18) and back (both provisional, the owner's to place and
      color); its Rainbow Route mirror leads to Prism Plains 1's start. 5-7 (a copy of 5-6) left out. Checked: the rooms' door
      graph (the DS's doors, rides and the new mirrors, terrain not counted) reached 359 of the 491 rooms from Prism Plains 1; the
      rest are the stages' Squeak rooms (after each goal: treasure rooms and hideouts, for the Squeak Lair doors) and 1-1's fifth
      room; in the GBA emulator (Kirby held on each door, Up): Central Circle's new door to the hub and back, the hub's way in, and
      every planned door, 146: 144 lead where they should; 1-2-1's (17, 8) works once the Sword Knight from the ability bubble
      four cells off is moved away (it walks onto Kirby held there: by the rulebook's rule 2, 3 cells, it stays); 8-4-2's is in
      Daroach's arena, shut till he's beaten (not fought in the test). No enemy now stands within 3 cells across, 2 up or down of
      a door or a landing in these rooms (the rulebook's rule 2, as the NiDL rooms': `SqueakObjects()` leaves it out), which
      fixed 4 of the first sweep's 7 failures; 8-4's rooms between its fights take the DS's warp star rides (3 to 4 to 5), no
      mirror (one dropped Kirby under 8-4-4's floor).
    - The bosses' shards (the rulebook's rule 8): each world's boss, beaten in its boss room, leaves a shard (KatAM's 8, one a
      world; the Bosses Give Random Shards option shuffles them as NiDL's: checked, all 8 found and logged), whose cutscene takes
      Kirby to the Squeak Squad hub, at the world's spot (Central Circle's door of the area of its number), standing; with or
      without the NiDL ROM (the exits' hook, `HookShardExit()`). 8-4-8 and 8-4-9, copies of Dark Nebula's room and the one before
      it, are left out (his fight once, in 8-4-6, the shard there); 2-8-7's goal door, the only one naming a room (one its stage
      doesn't have), is a goal again (2-8's last rooms had no way out). The room graph now: 365 of the 489 rooms reached, none of
      them without a way out. Checked in the GBA emulator (USA): 1-7's boss room, King Dedede beaten (forced: his HP set to 1, the
      rest of the fight played), the shard appears; Kirby put on it (forced), the cutscene plays, he dances and lands in the hub
      (0x5E9) on the first world's spot; the same in the European ROM. The owner, on 1-7's shard looking off centre: it comes
      over the boss room's chest (the DS drops its reward there, centred on the room's art: King Dedede's machine is 12 pixels
      right of the room's middle on the DS too, its camera centred), and that chest (KatAM's consumables, dealt in turn as the
      NiDL rooms': the DS's hold Boss Battle Badges) shows only once the boss is gone, as the DS drops it then. Checked in the
      GBA emulator (USA, forced as above): the chest hidden through the fight, shown after, the shard over the machine's middle. Not checked yet: the other 7 bosses' rooms (Dark Nebula's:
      his HP written to 0 skips KatAM's defeat, which only a hit starts, so that run proved nothing; the scripted Sword didn't reach
      him), a real fight, the tomato when the shard is owned, Japan in the emulator. Dark Nebula's room (8-4-6) showed a black band
      on its right: its far view is one of the five 256 pixels wide, which 3/4 made 192 (the screen 240); those now shrink to 240
      wide only (checked in the GBA emulator: the band's gone).
    - Goals (the owner: "Goal doors reach to the next stage correct, however keep in mind the Goal minigame doors from KatAM as well
      for path ending sections, 1 or 2 per area stage"): every goal but the deepest stage's (into the boss stage) leads on into the
      world's next stage; 2 a world, at paths' ends, into KatAM's goal games. Checked in the GBA emulator: all 46 goal doors lead
      where they should.
    - The switch rooms, world doors and world map (`KatAMExpansionSqueakHubs.cs`; the rulebook's rules 4, 5, 6): a switch room per
      world after the hub (0x5EA-0x5F1), NiDL's two layouts, its look the world's tiles (each cell the look its source room gives
      the same kind of cell with the same around it), its far view behind it (the owner's picks of the mock-ups, the pick circled
      in red: Prism Plains A, the far view 00; Cushy Cloud as built; Jam Jungle B, 22; Vocal Volcano A, 25; Ice Island B, 16;
      Secret Sea A, 18; `SqueakSwitchDesigns`): each its own background, the far view's middle 256 columns at full size as the
      mock-ups showed them, covering the room (the stage rooms' conversion, at 3/4, fell short of the screen and smeared its last
      rows); its open air a back wall fills, or touching no terrain, left clear, so the far view shows (Vocal Volcano's lines over
      its floor mirrors gone). Nature Notch and Gamble Galaxy redone (the owner: "those backgrounds are beautiful and you did a
      disservice to that area"): Nature Notch in 2-6-1's tiles (its leafy ledges, its earth) before its own misty forest (06),
      Gamble Galaxy in 8-3-1's starry rock before its shooting stars (26; its galaxy, 27, was the other candidate). The camera
      (the owner: "Why have been the position of the Squeak Squad hubs been off for the longest time? It's even showing out of
      bounds areas of the room"): the switch rooms were 12 rows, where KatAM's camera, hiding 8 pixels at a room's top and 40 at
      its bottom, needs 13 for a screen; it had 144 pixels for its 160 and showed past the room's edge. Now padded to 13 rows as
      the NiDL switch rooms (`Padded()`, the last row the ground's again). Checked in the GBA emulator (USA all 8, Europe Nature
      Notch, Japan Gamble Galaxy): each a screen, the ceiling at the top and the floor at the bottom, nothing past the room, the
      camera still with Kirby high and low (Ice Island). Dressed by eye as the NiDL switch rooms (the owner: "make them prettier
      like we did with NiDL's"; `SqueakSwitchDesigns`' blocks of the source room's cells, previewed over emulator shots, then
      checked in the GBA emulator, USA, all 8): Prism Plains a tree from 1-3-5 leaning in from the top left (its hollow trunk
      behind the floor mirror), flowers by the pedestal; Nature Notch a clearing in 2-6-1's woods (its canopy the ceiling, a tree
      on each side floor, bushes, the pedestal of its leafy grass); Vocal Volcano its castle's balustrade along the side floors;
      Ice Island snowy shrubs in the pit and a sprout on each side floor; Gamble Galaxy gold star dust and a crater in each side
      wall. A solid cell the source drew mostly clear now takes the nearest whole one's look (Nature Notch's and Ice Island's
      walls showed the far view through them). Cushy Cloud, Jam Jungle and Secret Sea as picked: Jam Jungle's room (4-1-5) has
      its vines and flowers on its back wall (opaque, they'd bring the wall with them), the see-through canopies are 4-2-1's,
      whose tiles are its wooden walkways (a switch room draws from one room's tiles): the owner's call. First round: the owner's
      to judge. Its big switch on its
      pedestal, two ability statues (provisional picks), its floor mirrors into the world (the stage before the boss's and another
      branch's), its way in a Mirra hidden in the world's second half. Its big switch sets a world slot of the expansion's save
      (0x20-0x27: the game's free ones are 6; rooms.c's `BigGetStateSlot`, KatAM's `GetStateSlot` replaced, the slots saved with
      the visited doors), which shows its two hub doors, the world's door in the Squeak Squad hub (Central Circle's door there, as
      Central Circle shows its own) and the world's door and path on the world map (the owner's mock-up: 8 doors after NiDL's, the
      Fountain of Dreams' off the map, the end game's), Kirby drawn on his world's door, on Central Circle in the hub. Its hub doors
      lead to the hub and to the KatAM room its path leads to, which gets a door back (by NiDL's rule for those rooms: Central
      Circle's door moved where one room's layout takes another's; provisional, the owner's to place); those KatAM rooms are edited
      after the randomizer, after NiDL's own edits (as NiDL's: before, Central Circle lost its golden NiDL mirror whenever both
      ROMs were in, found and fixed here). Checked: in the GBA emulator, USA (with NiDL's rooms and without), Europe, Japan (forced:
      Kirby held on each door, Up; the slots set): the switch rooms' 32 doors, the hub's 8, the KatAM rooms' 8, Central Circle's
      grey door, its golden NiDL mirror, NiDL's doors in the rooms shared with it, and the moved doors and the doors landing on them:
      all lead where they should (Japan's two needed a longer wait for its intro); the 8 Mirras' doors, forced open (the cell
      written as the Mirra's record makes it when it's beaten: the Mirra isn't fought in the test), lead onto each switch room's
      arrival, all three versions; a big switch pressed (Kirby dropped onto it, a real press, USA, Europe, Japan): the world map's
      animation plays the world's path, the slot is set in the save's area, its hub doors show; the pause screen's world map from a
      world's room, its switch room and the hub (USA). The room graph: every switch room reached, none without a way out; 364 of
      the 497 rooms reached, the rest the Squeak Lair rooms, as before. The 3 versions built and the NiDL logic checks unchanged
      (with and without drops). Not tested: beating a Mirra in a fight (KatAM's own Mirra code opens the same records), a save
      written and loaded with the new slots (they're in the area the save already copies), a real playthrough.
    - The window's option: done ("Include Rooms" under the Squeak Squad ROM, its own commit).
    - Logic checks with terrain (in progress; `ksslogic.py` in the session scratchpad, rooms/logic.py's rules on the Squeak Squad
      rooms, the hub, the switch rooms and Central Circle, the tables the built ROM uses): it found these, now fixed in the
      converter, each checked in the emulator:
      - Bomb chains: the blocks grid's cells flagged 0x1000 with a bomb block (or a stake) among or beside them were plain walls
        (1-1-4's grey wall walled the stage's first door in: 43 rooms of 498 reached). Now KatAM's chains (bomb block 0xF1, the
        blocks 0xF2, links 0xF3), cleared to the third layer when broken. Checked in the GBA emulator (USA): Kirby with an ability
        hits 1-1-4's bomb block, the whole wall goes, the grass in front stays. In the DS emulator: a slide into that wall away
        from its bomb doesn't set it off, into the bomb does; 3-2-2's and 3-6-2's bombs face only the inside of what they wall in
        (an attack from under 3-2-2's bomb, through the block below it, sets it off on the DS): there the chain's blocks facing
        the outside are bomb blocks too.
      - Switch blocks: gimmick 0x10 groups (the same flag, no bomb) a switch (gimmick 0, the small star switch) clears: now a
        KatAM object with Squeak Squad's switch_small (object 0x8D, chest.c's SqueakCreateSwitch), pressed by a Kirby touching it
        (as the DS's: a body moving through it), which breaks its groups as KatAM's lever walls break theirs. Checked in the GBA
        emulator, USA, Europe, Japan: 3-4-8's switch by the tower, Kirby walks onto it, it goes down, the beam's blocks over it
        go (collision 0xF2 to 0), the way up to the goal opens. Not tested: the DS's switches rise again and toggle their blocks
        back when pressed again (KatAM's stay down until Kirby comes back to the room); shutters (gimmick 2) stay open.
      - Thin floors flagged 0x300 were floors Kirby can't drop through: on the DS, Down drops him through (2-2-3's, checked in
        the DS emulator); they shut him in the pocket 2-2-5's door lands him in. Now dropped through as the others.
      - Before the fixes below ("Rooms reached": now 357, every room with a door into it): 334 of the 498 rooms reached counting metal blocks as broken
        (359 reachable without terrain), 285 without, 285 with metal broken but thin floors never dropped through; world 4's switch
        room not reached. What's left, all Squeak Squad gimmicks or abilities KatAM
        doesn't have: metal blocks on the way (2-1-3's shaft under its start, 8-3-8's band: the DS's Metal from Metalun; KatAM's
        metal blocks break only under Hammer and cannon shots), walls fire burns (flag 0x800: 4-2-3's start box and its towers'
        log bridges, 6-2-5's: Fire on the DS; tried as bomb chains, they opened 4-2-3's sealed tower parts to the levels' planner,
        whose route then ran where Kirby can't go, so they stay walls), 4-3-2's door inside blocks a scripted gimmick clears
        (gimmicks 0x13 and 0x1A, not identified yet, maybe the Squeaks' events), 6-1-4's and 7-3-2's paths (gimmick 4) and 6-3-3's
        groups with no switch. 7-3-1's door to 7-3-5 sits in a pocket of clouds the DS seals too (pushed into the wall at its
        height on the DS, Kirby stops), and 4-4-10's upper door is a twin of its lower one, never entered: Squeak Squad's own.
    - The map viewer (`KatAM Map Tracker`, `RomMap.ReadSqueakRooms()`; the ROM's map table points at the rooms' tables and each
      room's world and stage): the Squeak Squad worlds laid out as the NiDL levels, around KatAM's map on an unrandomized ROM (KatAM's
      own pause-screen layout kept, the NiDL levels and the worlds beside it), as tight clusters on a randomized one; the Squeak Squad
      hub a hub square beside Central Circle; the rooms drawn with their tiles. Checked with the tracker's own layout code headless
      (not the window, which needs Windows to build): USA ROM, unrandomized and with mirrors randomized, every room placed (the 498
      in their 9 clusters), no lines crossing rooms, the rooms' previews drawn (1-1-1, a world 4 room, the hub, a switch room).
    - The fire walls and the walls unported gimmicks open are star blocks now (below, "What only Fire or an unported gimmick
      opens"), so every room with a door into it is reached; metal blocks on the way (2-1-3's, 8-3-8's) are still counted as
      broken (KatAM's break only under Hammer and cannon shots). The owner: "Metal blocks on the way (2-1-3, 8-3-8) should only
      be breakable with path unlock abilities, so that is correct": they stay as they are.
    - Gimmicks (section 10): counted and read (the notes: 26 classes, their graphics, the barrel's states decoded); warp stars,
      cannons and the switches with their block groups in KatAM (above).
    - Gimmicks that talk to each other (`squeak/gimmicks.c`; the notes' "The gimmicks talk to each other"): the switches, trigger
      areas (0x15), delay relays (0x16), switch block groups (0x10), shutters (2: the stone pillars, upright and turned, and the
      spikes) and fire columns (7) as the DS's, each by its index in the room's list. Checked in the GBA emulator (USA, Kirby put
      in the room, then walked): 1-5-1's shut pillar stops Kirby; its switch opens it (it crumbles up), he walks through, and it
      shuts again 180 frames on as the DS's; 3-6-6's raised spikes hurt him and knock him back, its switch (through two relays)
      lowers them and he walks across; 4-3-6's trigger area, under the hole its turned pillar closes, shuts it over Kirby once
      he's dropped through (first tried with the area's box upwards: it shut under him as he stood by the hole; the DS's world's
      y goes up); 5-2-1's fire columns burn and die down in turn with their pilot flame between, and one burns Kirby (charred,
      knocked back). The logic checks count them: a shutter starting shut blocks its cells until a switch or trigger area Kirby
      gets to opens it (through relays too), and block groups any of them reach break (the waits and the shutters shutting again
      aren't counted). Europe and Japan: 1-5-1's switch, pillar and its shutting again checked the same way. Not tested yet: the
      others in Europe and Japan; a relay's waits (4-3-2's boulders: their gimmick isn't
      ported); a fire column a message lights (mode 0: 1 in the rooms).
    - The item holders (`squeak/gimmicks.c`; the notes' "What a grass tuft or a barrel holds"): grass tufts (0xE, all six looks)
      and barrels (0xF). What one holds is a record of its room's own item or enemy list, hidden till it's cut or burst: the
      converter leaves those records out of the room and the holder lets the KatAM object out (food, an enemy, or KatAM's ability
      star for an ability). Checked in the GBA emulator (USA, Kirby put in the room, Cutter written into his ability): 1-2-2's
      grass, walked through, rustles and stays (first tried with KatAM's hit mark alone: a plain touch cut it; now his body's hit is
      masked out), an attack cuts it, its blades fly, its stub goes and its Cherry comes out (first the Cherry stood there from the
      start: the room's item list holds it, the grass hides it); 1-5-1's top barrel, hit, bursts and lets out its item. Not tested:
      a regrown tuft, an enemy or ability star let out, Europe and Japan in the emulator. A chest a holder hides (7 of them: 2-1-3's,
      4-1-6's and 7-4-5's Ability Scrolls, 1-5-1's, 4-3-8's and 5-5-4's Copy Palettes, 2-5-5's Graphic Piece) came out as a Cherry;
      now the holder lets out the chest itself, standing where the barrel or tuft stood, opened and saved as the room's other chests
      (a tuft that held one doesn't grow back). Checked in the GBA emulator (USA; Sword written into Kirby's ability, Kirby moved
      onto the chest by a RAM write): 2-1-3's barrel burst into its big chest, which gave scroll 20 and marked itself opened; 4-1-6's
      grass cut into its small chest, scroll 21.
    - The icicles (9: `squeak/gimmicks.c`, the notes' icicle bullet). Checked in the GBA emulator (USA, 6-1-4, Kirby walked under
      them): one shakes, falls and shatters on the floor; on Kirby it hurts him (OUCH, a bar of health) and knocks him back (first
      it shattered at once: KatAM's camera limit was taken for the room's bottom; then in mid-air: its spot is its tip, not its
      top; then it burnt Kirby: the fire column's harm kind; now a plain hit). Not tested: its hanging again after 180 frames,
      Europe and Japan in the emulator.
    - The falling rocks (8) and the random relay (0x17) that rains them (the notes' falling rocks bullet). Checked in the GBA
      emulator (USA, 5-3-4): the rocks appear, fall and break on the floor as the relay sends. Not tested: a rock hurting Kirby,
      2-5-5, Europe and Japan in the emulator. 8-1-8's are started by its rope (gimmick 1, below) through a relay.
    - The moving platforms (4: `squeak/gimmicks.c`'s SqueakCreatePlatform, the notes' moving platforms bullet): done (the owner:
      "Write a carrier and make it smooth"). Each moves leg by leg at the DS's speed (1, 2 or 4 pixels a frame), starting at once,
      when Kirby lands on it or on a switch's message, and at its end stops, goes round, goes back and forth, or shakes and falls
      as the DS's; it carries the Kirbys standing on it every frame (its task runs before theirs, his feet put on its top and he's
      marked on the floor, so he walks and jumps from it as from the ground). Checked in the GBA emulator (USA, Kirby put in the
      room): 8-1-1's waiting one starts as he lands and carries him up 18 cells at exactly 2.00 pixels a frame, his feet on its top
      all the way; 6-1-4's goes back and forth at exactly 1.00 a frame with him on it; walking on it as it moves and jumping off.
      Not tested: helpers riding, a breaking one (7-5-1's, 1-9-17's), one a switch starts (6-2-1's, 8-1-2's), Europe and Japan
      in the emulator. The logic checks don't model them (the flood treats their paths as flown over).
    - The thunder clouds (5: the notes' thunder clouds bullet; 19, in 3-5-1 to 3-5-3, 6-3-9 and 7-3-1): moving platforms with the
      cloud's look; their zap now exactly as the DS draws it (the owner: "the thunder clouds' zap exactly as the DS draws it"). Read
      from the DS's code: the cloud plays only its 4 composite animations (its .mca: resting with sparkles, the zap's green burst
      and sparks, the shake before it breaks, its puffs), flattened here into plain animations; it has no timer: an attack on it
      starts a zap at the end of the resting loop it's in, the zap lasting 5 of its 12-frame loops (60 frames, thunder on the 2nd
      and 4th), hurting in its burst's ±40-pixel box (the DS's attack boxes), riders too; a moving cloud's break is its shake then
      its puffs. Earlier the port zapped on two 30-frame timers with the break's flashing look: both wrong. Checked in the GBA
      emulator (USA, Europe, Japan, 3-5-1, Kirby on the first cloud): no zap in 300 frames untouched; Kirby given the Sword by a RAM
      write swings it, both clouds zap with the burst and sparks, he's hurt and knocked off, and they rest again after. Not
      checked: the zap against the DS running (the DS emulator crashed loading 3-5 in 1-1's place), a moving cloud's break, the
      thunder's sound against the DS's (now Squeak Squad's own, 0x11F: section 9).
    - The ropes (1) and rope floors (3: the notes' ropes bullet; 25 ropes, 16 floors): an attack cuts a rope and messages its
      targets (the floor it holds, 8-1-8's rock relay); a rope floor is a platform that carries Kirby, then shakes, falls and
      breaks on the first solid cell under it, when its rope is cut or (6-2-1's) when Kirby stands on it. Checked in the GBA
      emulator (USA, Europe and Japan, 3-3-7, Kirby with Sword): the Sword cuts the left rope, its plank falls and breaks, the
      right plank stays with him on it; (USA, 6-2-1) standing on its floor drops it onto the spikes. Not tested: 8-1-8's rope
      starting its rocks, 3-3-6's. Logic checks: 359 rooms reached both ways, 8 switches, no traps, all three versions.
    - The rolling boulders (6: the notes' boulders bullet; 18, in 2-4-1, 2-4-2, 4-3-2, 4-6-7, 8-2-4 and the boss stages): roll down
      their slopes at the DS's ramped speed (2, 3 or 4 pixels a frame), turning as they go, hurt Kirby and burst on the first wall;
      the waiting ones start on their trigger's message (2-4-1's quake triggers, 4-6-7's trigger areas, 4-3-2's relays); the ones in
      pockets stay there. Checked in the GBA emulator: USA 2-4-1 and 4-6-7 (Kirby walks into the trigger, the boulder rolls down
      at him, hits him, bursts on the wall), Europe 2-4-1. Their quakes are in now (the owner: "I want them"): landing after a fall
      KatAM's 14-frame shake, bursting on a wall its 24-frame one (the DS's 16-frame and bigger quakes). Checked in the GBA emulator
      (USA, 2-4-1, Kirby walking right from the start): the screen shakes for 24 frames as he reaches the quake trigger, 14 as the
      boulder lands, 24 as it bursts (the view's shake offsets read each 4 frames).
      Not tested: 4-3-2's, 8-2-4's, Japan's boulders. Logic checks: 359 rooms both ways, 8 switches, no traps, all three versions.
    - The candles (0xA: the notes' candles bullet; 8, in 4-3-7, 4-3-8, 8-1-6): Fire or Burning lights one for its frames (400-540),
      then it goes out. Checked in the GBA emulator (USA and Japan, 4-3-7, Kirby with Fire). The dark room and Light: the candles and Light
      request above.
    - The camera areas (0x14): done. 58, nearly all holding the view to one DS screen (the goal games' rooms, two-screen rooms like
      2-3-8, the treasure rooms): a Kirby entering the area's box has his view held inside its bounds (their y is the DS's, up from
      the room's bottom: converted), the GBA's smaller screen moving inside them; let go once he's out of the bounds (the DS keeps
      them, its rooms closing behind him; here he'd be off the screen). Rooms whose fight's arena holds the camera already skip them.
      Checked in the GBA emulator (USA, 2-3-8): the view stays on the top screen while Kirby is there (x 16-32, y 48, not the room's
      wider range) and moves to the bottom screen (y 304) when he's in the bottom area.
    - The effect emitters (0x19): done. They're the lava sparks (120, Vocal Volcano and 2-5, 3-2, 5-x, 7-x) and water bubbles (51):
      while its spot is on the screen, every 100, 200, 300 or 400 frames (random) it plays yakumono/spark's or yakumono/bubble's
      animation once there. Checked in the GBA emulator (USA, 5-5-5): sparks pop on the lava now and then. Bubbles not checked.
    - The snow and wind streaks (0x18, 33 in 18 rooms: Ice Island's snow, 3-4-1's and 7-2-3's wind, 8-1-9's and 8-2-5's falling
      streams): ported (the owner: "Snow and wind streaks"; `squeak/gimmicks.c`'s Streaks). Every 10 frames, its box near the
      view, a streak at a random cell along the side of its box its parameter names, one of its look's 3 animations at random, moving
      away from that side at the DS's speed (its constructor's table: the wind 4, 6 or 8 pixels a frame, the snow always 1 pixel
      down). The DS hands them to its shared effect system, which ends them (both looks' animations loop); that wasn't read, so here
      a streak ends once it's been on the screen and left it, or after 255 frames. Checked in the GBA emulator (USA): 6-1-1's snow
      falling, 3-4-1's wind rising, 7-2-3's blowing left. Not compared against the DS running.
    - The sound gimmick (0x1A, one, 4-3-2): plays its sound (the DS's 0x32) every so often, its count and wait its parameter's,
      heard while Kirby is in its room. Checked in the GBA emulator (USA, 4-3-2: 26 plays in 1,800 frames).
    - The quake triggers (0x13: the notes' quake bullet): ported as a trigger area that messages its targets once (a Kirby within
      64 pixels of its box, or a switch's message, sets it off), with its screen quake now (KatAM's timed shake nearest its length:
      the rooms' are 60 frames, so KatAM's longest, 24). On the DS 4-3-2's is set off by a rolling boulder; here a Kirby nearing it
      opens its way as before. Checked in the GBA emulator (USA, 2-4-1: above). The stone shutters' quake stays out (the owner's
      earlier complaint: the screen shakes too much).
    - What only Fire or an unported gimmick opens became star blocks (the owner: every room reached; the notes' bullet on it): the
      cells Fire burns (4-2-3's start box and log bridges, 6-2-5's), the frozen blocks (6-1-4's), the hidden breakable blocks
      (block 3: 4-4-4's shafts down to its doors, small floors in 35 rooms) and the flagged groups nothing ported sets off (4-3-2's
      wall, 4-6-7's, 8-1-10's, 8-2-4's). Checked in the GBA emulator (USA, 4-2-3, plain Kirby): he inhales the burnable cell by the
      start, spits it at the log column, it breaks and he walks out. Not tested in the emulator: the frozen and hidden blocks,
      Europe and Japan.
    - DS doors landing on the right door: a door names the door it lands on by its place in that room's list (7 rooms number
      theirs out of order); 3-1-4's way into 3-1-5 was lost, now kept.
    - Rooms reached (the owner: "Check why the rooms can't be reached and fix it"): the levels' planner now follows the DS's own
      doors from every door Kirby lands on, not only the stage's arrival (7-3-5 and others were left off), through slopes too, and
      never down through a thin floor (rule 9: without Down and B); a room numbered after the goal counts as the stage's lair only
      if no DS door from the stage leads to it; a room it doesn't reach gets a throw where the DS lands Kirby there (its arrival,
      else the door a DS door lands him on: 7-2-8, under 7-2-7's thin floor); and its way-out pass (rule 9, as NiDL's) gives a
      place with no way back out a one-way door onto the start of the first stage of its world that has one (7-3-2's top, where
      the DS's way on is under a thin floor: a door by its floor, to 7-1-1). Logic checks (USA, Europe, Japan the same): 359 of
      the 498 rooms reached, the 8 switches pressed, nowhere trapped, with and without dropping through thin floors; the 139 left
      have no door into them at all (the Squeak Lair rooms and chase spawns, on hold). NiDL: 280 of 280 in all six checks.
      Checked in the GBA emulator (USA, Kirby put on 7-3-2's top, where 7-3-1's door lands him): he walks off its blocks, drops
      to the floor, takes the one-way door and lands at 7-1-1's start. Not checked in Europe and Japan in the emulator.
    - Gimmicks left (the notes have what's decoded). The owner, of the ones left out on purpose: "The left on purpose, I want
      them yes": the camera areas (0x14), the ambience 0x18 (snow, wind), 0x19 (lava sparks) and 0x1A (sound), gimmick 0x12 (a
      color fade), the boulders' quakes when they land and burst, and the thunder clouds' zap exactly as the DS draws it. Done: the
      camera areas, 0x19, 0x18 (above), the boulders' (and quake triggers') quakes, the thunder zap (above), and 0x12 with the DS's
      dim light (the owner: "Port the DS Dim light"): on the DS 0x12 fades the far view (the stage's 3D background object, not the
      room), whose 17 pictures blend a start palette into its own in 16 steps; its three rooms (2-3-1, 2-8-1, 7-3-1, where Kirby
      starts those stages) show the first till a body enters its box, then step to its own over 120 or 240 frames. Here the far
      view (BG palettes 6-13) starts at the DS's start and steps up the same way (`squeak/gimmicks.c`'s Light). The starting
      brightness (the owner: "Element the Dim Light's starting brightness"): traced on the DS, it is full. The 17-picture fade is
      only built from a room's palette file (`PL`, the boss rooms'), and these three rooms have none; the gimmick also fades the far
      view's light, from the room's own light color (read in the DS emulator: 255, 255, 255 in 2-3-1 and 7-3-1) to the gimmick's
      (255, 255, 255 in all three). So on the DS nothing dims in these rooms, and the port now starts at full too (it started at
      half, a stand-in). What does dim on the DS is the candle rooms' light (4-3-7, 4-3-8, 8-1-6): to a quarter over 30 frames
      when the candles go out, below with the candles. Checked: built (USA, Europe, Japan); the half-brightness start is gone (the
      fade code is kept, starting at full). Not checked on screen in the GBA emulator, as nothing changes. 0x1A plays its sound
      now (above). Also:
      - The candles and the Light ability (the owner: "Add to the pending list candle and light ability interaction", then:
        "the light ability should light up Squeak Squad sections that require the candle if NiDL expansion is present"): with
        the NiDL ROM loaded, the Light ability (NiDL's, from Cool Spook) lights the Squeak Squad dark rooms' parts the candles
        light, as Fire does on the DS.
    - Vitality halves: done (the owner: "Two halves make one Vitality"). Each Vitality chest holds half: KatAM's Vitality,
      taken back on a pair's first half, so max HP grows on every second one (`chest.c`, `gSqueakVitalityChests`). Tested
      in the emulator in all three versions on 1-6-7's chest (Kirby walks onto it and presses Up): as the first half, max HP
      stays 6; with another half marked found by a RAM write, Vitality 1 and max HP 7. Not tested by opening two real chests
      in a row. Three of the eight are placed (1-6-7, 2-6-7, 3-6-2); the other five are chase-spawn chests (on hold).
    - Left: what an Ability Scroll does (they're owned and saved: below), the chase's chests (its Squeaks are done: **The
      chases**, below; the chests but the 5 scrolls' aren't placed), the arenas without the NiDL ROM (NiDL's type holds the camera), the
      gimmicks, the areas.
  - Ability bubbles whose ability no regular enemy gives (Hammer, Bonkers' in KatAM; Squeak Squad's ability numbers 12, 18 and
    26) become food (the owner: "In the case of bubbles without no regular enemy, you can swap them out for food.").
- **Squeak Squad's spray paints**: done (the owner: "The Spray Paints should be the spray colors from Squeak Squad as new items in
  the collection screen", then: "remove any duplicate spray color, it would be pointless to have multiple if these are already in
  KatAM"). Squeak Squad has 18 sprays; 13 are KatAM's (YELLOW to CHALK by name, 7 of them in slightly other shades), so only its
  5 new ones are added: SHADOW, IVORY, CITRUS, WHITE, LAVENDER. They are Kirby colors 16-20 (the UFO's too), owned in the spray
  field's free bits 14-18 (the completion stays KatAM's), each in its Squeak Squad chest (treasures 83-87; the 13 others' chests
  hold items), shown in the Collection Room's grid (the first row after KatAM's columns, below) as KatAM's spray can in their colors, with "SPRAY PAINT : NAME" in
  KatAM's letters after each language's own "SPRAY PAINT :" (the Japanese ROM has no English letters: Squeak Squad's name graphic
  there). A chest shows the closest KatAM can rising (it gives the spray, not KatAM's). The chest shuffle leaves these chests
  alone; the map viewer names them. `KatAMExpansionSprays.cs`, `collection.c`.
  - Tested in the emulator, sprays owned by a RAM write (a forced state, not a playthrough): the 5 cans and texts
    (USA, Europe in its 5 languages, Japan), choosing a color (the previews), the color worn in the game (USA, the owner's save:
    Lavender Kirby in Central Circle, also with the Master Sword), and a real chest opened (room 0x498, Lavender: its bit set,
    KatAM's Grape bit, whose can it shows, given back). A full randomization with the chest shuffle and with random chests keeps
    the 5 spray chests.
  - Not tested: the UFO ability in a new color, the screen that reads the colors' third table (the continue palettes,
    `gUnk_0834BB20`: its data checked only), multiplayer with a new color, the other 4 chests opened. Which chest holds which
    spray takes treasures 70-87 as Squeak Squad's sprays 1-18 in order: not checked against the DS's chests.
  - The names are English in every European language and Squeak Squad's in the Japanese ROM: Squeak Squad's European and
    Japanese names aren't in the US ROM used.
- **Map items for each area**: done for 15 of the 16 areas (the owner, with the rooms: "KatAM's map item, its sprite recolored to each
  NiDL and Squeak Squad area's theme, one per area, so the map viewer shows the area once its map is found"; then: "Add the map items
  for NiDL and Squeak Squad"). Each area's map is KatAM's map in the area's colors (Vegetable Valley lime, Ice Cream Island cream,
  Butter Building butter, Grape Garden violet, Yogurt Yard brown, Orange Ocean coral, Rainbow Resort aqua, Fountain of Dreams
  periwinkle; Prism Plains mint, Nature Notch olive, Cushy Cloud pink-white, Jam Jungle raspberry, Vocano Valley lava, Ice Island ice,
  Secret Sea blue, Gamble Galaxy indigo), owned in the big chests' field's free bits 10-25 (KatAM's maps 0-9, its completion
  unchanged), each in one chest of its area: a NiDL level's best chest (the planner's first: behind breakable blocks, a dead end, far
  in), a Squeak Squad world's deepest chest that held a consumable, a big one first (none of the chests holding the owner's Squeak
  Squad items). Opening one gives the area's map: its map rises out of the chest in the area's colors (KatAM's map, the chest giving
  one of KatAM's maps for a moment, which is taken back). The Collection Room shows them in the grid's third (NiDL) and fourth (Squeak
  Squad) rows after KatAM's columns, named "• AREA" over each language's own "Use this for navigation." (KatAM's letters, J and X
  drawn in their style; the Japanese ROM, which has no English letters, Squeak Squad's font). The chest shuffle leaves these chests
  alone; the map viewer names them ("Small Chest: Vegetable Valley Map") and reveals an area from its map in the save (its Lua script
  now reports bits 10-25). `KatAMExpansionMaps.cs`, `chest.c`, `collection.c`.
  - Tested in the GBA emulator: Vegetable Valley's chest (room 0x06F, Kirby put on it by a RAM write, Up): its bit set, KatAM's map
    bit given back, the lime map rising; Prism Plains' big chest (1-6-5, room 0x425): its bit, the mint map; the Collection Room with
    every map owned by a RAM write (USA; German, French, Japanese names), every scroll position from 1 to 8 with no colors clashing.
    A full randomization with the chest shuffle and random chest contents keeps all 15 map chests as they were. Logic checks
    unchanged (359 of 498 Squeak Squad rooms, 8 switches, nowhere trapped, all three versions, with and without thin-floor drops).
  - Not tested: the other 13 chests opened, the map viewer's reveal with a real save (its code built; the window needs Windows),
    Italian and Spanish names on screen, multiplayer.
  - The Fountain of Dreams' map (the owner: "The fountain of dreams hub can only exist within the amazing Mirror after the
    nightmare fight. It's a prelude before the final fight."): its level is all boss rooms, so its chest stands in the Fountain of
    Dreams hub, where Nightmare's end leads, on the floor's left (the Maximum Tomato on its right). Checked in the GBA emulator
    (Kirby put on it by a RAM write, then Up; USA, Europe, Japan): its bit (17) set, the periwinkle map rising; its Collection Room
    cell filled as the others' (by the same code: not looked at on screen).
- **The Secret Map**: its pieces done, the ability room it opens left (the owner: "Start building the secret map logic and then
  display the respective icons on the collection room"; with the rooms: "Only the Squeak Squad ability room can be unlocked via the
  hidden map"). Squeak Squad's 7 Secret Map pieces (treasures 93-99) stay in their own chests (2-6-6, 3-3-7, 4-4-5, 5-4-5, 6-6-5,
  7-6-9, 8-2-6: rooms 0x463, 0x48F, 0x4D9, 0x515, 0x56A, 0x5A8, 0x5CC), owned in the music field's free bits 16-22 (KatAM's Sound
  Player and sheets take bits 0-10 and its completion counts only those). Opening one gives the piece: KatAM's map rises in the
  parchment's colors (the chest gives one of KatAM's maps for a moment, which is taken back). The map is whole once all 7 are found
  (`squeak.h`'s `SecretMapWhole()`, for the ability room's door). The Collection Room shows them in the grid's second row after
  KatAM's columns, each Squeak Squad's own piece (`collection/map`) shrunk to 16 pixels with 3 parchment shades, named "• SECRET
  MAP PIECE" over each language's map text. The chest shuffle leaves these chests alone; the map viewer names them ("Big Chest:
  Secret Map Piece 1"). `KatAMExpansionMaps.cs`, `chest.c`, `collection.c`.
  - Tested in the GBA emulator (Kirby put on the chest by a RAM write, then Up): piece 1's chest (room 0x463) in USA and Japan,
    piece 7's (room 0x5CC) in Europe: the piece's bit set, KatAM's map bit given back, the parchment map rising with a black outline
    (Squeak Squad's chests here have red where KatAM's popup draws its outline: now set to KatAM's black, the chest unchanged); the
    Collection Room with 4 of the 7 owned by a RAM write (USA; English, German, French, Spanish, Japanese texts), the missing ones
    "?" as KatAM's. A full randomization with the chest shuffle and random chest contents keeps all 7 piece chests. Logic checks: all
    7 rooms are reached (with and without thin-floor drops), nothing else changed (359 of 498 Squeak Squad rooms, 8 switches, NiDL's
    280 of 280 and 7 switches, nowhere trapped, all three versions). Vegetable Valley's map chest opened again after the outline
    change: unchanged.
  - Not tested: the other 5 chests opened, getting into each chest's spot by playing (the logic checks only that its room is reached),
    the map viewer's naming on screen (its code built; the window needs Windows), Italian on screen.
  - Which chest holds which piece takes treasures 93-99 as pieces 1-7 in order (`collection/map`'s cells 0-6): not checked against
    the DS.
  - The Squeak Squad ability room's door (the owner: "The Squeak Squad ability room should go in the Squeak Squad hub at the same
    position the ability room is"): Central Circle's ability room door, at its spot (cell 3, 4), drawn as Central Circle's, shown once
    the Secret Map is whole (world slot 0x1C, KatAM's free one, read from the 7 pieces: `rooms.c`'s BigGetStateSlot), leading to the
    Squeak Squad ability room, after the switch rooms, its door back to the Squeak Squad hub's. The map viewer puts it on the hub's
    left. Checked in the GBA emulator (pieces owned by a RAM write; USA, Europe, Japan): no door without the 7 pieces, the door
    shown with KatAM's sparkle with them, in to the ability room and back out to the hub.
- **The Squeak Squad ability room**: done, Squeak Squad's own (the owner, after the first version, a copy of KatAM's ability
  room's layout in Squeak Squad's tiles: "No, just import Squeak Squad's ability room to KatAM along with its pedestals, it's
  literally that simple what I'm asking"). It's the one room of the DS's stage 9-1 (`map/a9s1s1`, the Arena's: a hall 92 by 16
  cells with two shelves, a bell tower at its right end; 9-10 has the same room), converted as the worlds' rooms are (its tiles and
  tile animations, its far view d_viw_0B, its collision: the shelves are thin floors, Down + B drops through them, Kirby jumps or
  floats back up through them). Its 25 pedestals stand where the DS puts them (its item list, kinds 0x17 on: the DS's ability the
  kind less 0x16), drawn as the DS draws them (`item/copy`, the pedestal item's animation table: the ability's icon on a gold
  stand, in the DS's own palettes), and a Kirby with no ability who touches one gets its ability, as KatAM's ability statues
  (`squeak/gimmicks.c`'s `SqueakCreatePedestal`). They give Fire, Ice, Spark, Beam, Tornado, Parasol, Cutter, Laser, Bomb, Wheel,
  UFO, Cupid, Sleep, Hi-Jump (Starman's, NiDL's port), Sword, Throw (Bun's, as Bun gives in this port) and Fighter (Boxin's); the
  other 8 show Squeak Squad's own abilities KatAM doesn't have (Ninja, Animal, Bubble, Metal, Ghost, Triple Star, and two the
  DS gives from no enemy: an orange lantern and a black top hat) and give nothing. Kirby lands on its doorway (the hall's right
  end), which leads back to the Squeak Squad hub's door to it (world slot 0x1C, once the Secret Map is whole). Its music is KatAM's
  ability room's: the DS's room sets none. `BuildSqueakAbilityRoom()`.
  - Checked in the GBA emulator (USA): in through the hub's door (the Secret Map's pieces owned by a RAM write) onto the doorway,
    out again onto the hub's door; the whole hall seen (both shelves, the floor row, the Triple Star pedestal on the roof under
    the bells); touched Throw (Bun's), Beam, Cupid, Fighter and Hi-Jump's pedestals and got each (Hi-Jump used to jump back up
    through the shelves), Bubble's and got nothing; dropped through the shelves with Down + B. The pedestals' colors checked
    against the DS's palettes (a render of `item/copy` with each piece's own palette). Europe and Japan: the same round trip in the
    emulator (onto the doorway, back onto the hub's door) and Beam's pedestal giving Beam. All three built and logic-checked (every
    room reached, every switch pressed, nowhere trapped, with and without thin-floor drops: unchanged).
  - Not tested: the DS's own look of the room in play (the DS shows the room only in its Arena mode: swapped into stage 1-1 in
    the DS emulator, its pedestals didn't come out), so each pedestal's height (its stand's foot on the shelf) is the data's, not
    seen on the DS; every pedestal one by one (6 taken); helpers taking one.
- **The NiDL hub's ability room door opens only with all the NiDL hub's switches**: done (the owner: "For the NiDL ability room, it
  can only be unlocked after all switches have been pressed on the NiDL hub"). The NiDL hub's copy of Central Circle's ability
  room door no longer waits for KatAM's switch (world slot 0x12, after KatAM's 15 big switches): it waits for world slot 0x1D
  (KatAM's, free), set while the 7 NiDL levels' big switches (slots 0x14-0x1A; the Fountain of Dreams has none) are all pressed
  (`rooms.c`'s `BigGetStateSlot`, `NIDL_ABILITY_SLOT`; `KatAMExpansionHub.cs`'s `NidlAbilitySlot`). It leads to the NiDL ability
  room (below). Central Circle's own door is unchanged, and the Unlock the Ability Room option now opens Central Circle's door
  only (the NiDL hub's follows this rule).
  - Checked in the GBA emulator (USA, the switches' slots set by RAM writes): in the NiDL hub with 6 of the 7 set, no door; the
    7th set, the door appeared with KatAM's sparkle, and Up took Kirby into the ability room. Europe and Japan: the same in the
    emulator. NiDL's logic check: its switches are slots 0x14-0x1A, the 7 the door waits for. Not tested: pressing the 7
    switches by playing, the Unlock option's ROM in the emulator.
- **The NiDL ability room**: done (the owner: "I think we can generate our own pedestals for NiDL exclusive abilities. I would
  like to create a custom room that holds all NiDL exclusive abilities in a pedestal just like that. For a pedestal, it's a matter
  of shrinking down the enemy and making sure it looks alright in game"). Room 0x002: KatAM's ability room (its look, collision,
  music) with 7 statues where KatAM's first 7 stand, one per NiDL-only ability: Needle, Freeze, Light, Ball, Hi-Jump, Mike and
  Backdrop, each figure the NiDL enemy or mini-boss that gives it (Needlous, Chilly, Cool Spook, Bubbles, Starman, Mr. Tick-Tock,
  Bugzzy), shrunk to 20 pixels (colors averaged), in one shared palette as KatAM's statues share theirs, on KatAM's own statue
  stand; touched by a Kirby with no ability, each gives its ability as KatAM's statues do (`abilities/star_rod_stand.c`'s statue
  mode on object 0x9A, `KatAMExpansionMotions.cs`'s `BuildNidlStatues()`, `KatAMExpansionHub.cs`'s `BuildNidlAbilityRoom()`).
  The NiDL hub's ability room door leads there and its door back to the NiDL hub (the rulebook). The Star Rod keeps its own stand in
  the NiDL hub. KatAM's ability room is unchanged.
  - Checked in the GBA emulator (USA; Kirby moved in by RAM writes, the NiDL switches' slots set by RAM writes for the hub's
    door): each of the 7 statues touched gave its ability (27-33); the door from the NiDL hub into the room and back out to the
    hub. Screenshots of the room. Not tested: Europe and Japan in the emulator (built and logic-checked), helpers touching a
    statue.
  - Redone (the owner: "The pedestals look really messy and unpolished... some powers can use an icon, for example Mike can be a
    music note"): each statue is now the enemy's own NiDL frame at its native size, cropped, with its own colors (one palette per
    pair of statues, `gNidlStandKeys`), not shrunk and averaged; Mike's is Mr. Tick-Tock's own music note; only Bugzzy is shrunk
    (by the color most of each block has, its outline kept dark). Checked by screenshots of room 0x002 in the GBA emulator (USA,
    NiDL only). Not touched again in the emulator after the redo (each statue's ability came from the same code before it).
  - The Map Viewer (the owner: "Add the Map Viewer Changes"): room 0x002 is marked in its area entry (+0x4A) and placed beside
    the NiDL hub in the hub's cluster (`RomMap.IsNidlAbilityRoom()`); it also names the chests a barrel or grass holds and the
    Ability Scroll chests ("Ability Scroll N"). Checked with the viewer's Core built on Linux (its placement and its object lists
    on a built ROM: all 23 scrolls and the 7 held chests); the window itself needs Windows (not run).
- **Ability Scrolls**: in the game as data (the owner: "Implement all Ability Scrolls in their respective chests to add this data
  to the game already"). All 23 of Squeak Squad's scrolls (treasures 28-50) are in their own chests where Squeak Squad has them:
  opening one marks the scroll owned in the save (`rooms.c`'s `SqueakTakeScroll()`, 4 bytes of the expansion's save area), and
  its popup shows the Secret Map pieces' parchment (KatAM's map item, given back). 15 are in plain chests; 3 were in chests a barrel
  or a grass tuft holds (above); 5 are in chests the DS brings out only in the chase after the stage's big treasure is carried
  (6-1-7, 6-5-6, 7-1-6, 7-3-4, 8-1-15); the chase is ported now (above) but not its chests, so those 5 chests are there from the start (the chase's other things
  stay out; 7-3-4's, between two pillars over water there, stands on the ledge right of them). What a scroll does (Squeak Squad's
  power-ups of each ability) isn't done: it's owned only.
  - Checked in the GBA emulator (USA; Kirby moved onto each chest by a RAM write, then Up): 1-3-5's (scroll 0), 2-1-3's barrel's
    (20) and 4-1-6's grass's (21), and all 5 chase chests (15, 7, 18, 10, 13): each set its own bit. Europe and Japan: 2-1-3's
    barrel's (20) and 7-3-4's (10), the same. Not tested: the 12 other plain ones one by one, a Collection Room icon (none yet).
- **Hiding in barrels** (the owner: "Bring the hiding in barrels graphic from Squeak Squad since it's the same spriting style to
  KatAM to port the barrel hiding"): done, with Squeak Squad's own barrel (its six animations, already the barrel gimmick's). As on
  the DS (the barrel's states, vtable `0x02229594`): Kirby on the floor in front of an empty barrel presses Down and gets in, the
  lid shutting; pressing a direction shakes it; A opens it and he hops out; none gets in for 30 frames after. Inside he's hidden and
  out of reach, as in KatAM's cannon (its own hold, `sub_08051C40`); a barrel with him inside isn't burst. Only empty barrels open,
  as on the DS: 8-1-4's two (the others hold something and burst when hit). `squeak/gimmicks.c`'s `SqueakBarrel`.
  - Checked in the GBA emulator (USA, 8-1-4, Kirby moved to the barrel by a RAM write, then played by buttons): in with Down, the
    shake with Right, out with A (a hop of about 37 pixels, back on the floor), in again, out and walking away. First tried with
    KatAM's cannon landing to let him out: it threw him to the ceiling; then with his own new-press bits: his crouch takes Down
    before the barrel sees it, so the barrel reads the held keys itself. Europe and Japan: in, shaken, out onto the floor and in
    again, the same in the emulator. Not tested: a helper, an enemy touching the barrel with Kirby inside. The DS's shut sound isn't played (not identified).
- **The chases**: done (the owner: "In Squeak Squad stages there will be an invisible trigger wall on the position of the chest,
  the height of the wall will be of the entire room ... whenever the player passes that threshold, the chase will activate"; "I
  would also like to add chase sequences to KatAM, these last random KatAM spawns should be random and optional with a toggle in
  the randomizer. Since KatAM's chests can not be stolen like Squeak Squad's, the main cast mouses should be faster, more aggresive
  and go for Kirby to incentivize this chase aspect"; and that they jump and don't fall into the void). `squeak/gimmicks.c`,
  `squeak.h`'s "The chases", `KatAMExpansionSqueakRooms.cs`' `SqueakChaseOf()`, `KatAMExpansionChases.cs`; the ROM Expansion README's
  **The chases**.
  - Squeak Squad's stages (with Include Rooms, always): each stage with chase spawns (the DS's objects with flag 4 that are Squeaks)
    has a chase: an invisible wall at its big chest's x, the room's whole height; a Kirby crossing it either way starts the chase,
    and the Squeaks the DS brings out in the stage's rooms come out as the camera nears their spots, after Kirby. It ends when the
    player's Kirby goes into a room of another stage or a KatAM room. 37 stages (442 rooms), 66 walls, 787 Squeaks (at most 6 a
    room, none by a door). 6-5 has chase Squeaks but no big chest (only three small ones; the DS starts its chase some other way,
    not found), so it has no wall and its chase never starts: its Squeaks don't come out.
  - KatAM's rooms (the window's "Add Chase Sequences to KatAM", beside Include Rooms; off by default): half of KatAM's rooms with
    a big chest and no fight (the tutorial's left out), picked by the seed, each its own chase: the wall at the chest, one to three
    Squeaks (Spinni, Doc, Storo, Squeakers of any color, picked by the seed) at the room's own enemy spots at least 96 pixels from
    the chest. 7 rooms (seed 1234: 0xBE, 0x136, 0x213, 0x262, 0x269, 0x2E7, 0x338). The spoiler log lists them ("Chase Sequences").
  - The chase's Squeaks: never culled off screen, take no chests (every one goes for Kirby), Squeak Squad's angry pace (Spinni and
    Doc fast, Storo at its fastest, Squeakers' hops 1.5 times), short waits before attacking (Spinni 40 frames, Squeakers 20, all
    Squeaker colors throw). At a pit they jump only if their own jump, traced through the room, lands; else they go down if Kirby is
    lower and the pit has a floor, else they wait at the edge. One that still falls into the void is put back on its last floor.
  - Checked in the GBA emulator (USA):
    - 1-3-5 to 1-3-6: Kirby walked right across the chest's wall by buttons (chase on), was moved to the door by a RAM write and
      took it with Up; in 1-3-6 the chase was still on and Spinni came out with its HP bar and hit Kirby. Run again after the
      steering changes: the same.
    - KatAM room 0x262: walking left past the chest started its chase (key 0x83); Spinni came out once Kirby was moved near its
      spot (RAM write) and attacked.
    - Pits, the chase turned on by a RAM write and Kirby put across a gap: Storo in 7-6-12 (0x5AB) walked to the edge of a three-
      cell gap, jumped it and hit Kirby on the far ledge. A Squeaker in 6-3-9 (0x549), over the void between floating ledges: before
      the fixes it hopped into the void, then hung in the air where it was put back; now it hops to its ledge's end, high-hops across
      and lands on the far ledge, then throws at Kirby. Spinni in 8-1-15 (0x5C1) jumped the void up onto a higher block and back
      for 400 frames without falling. Another Squeaker there and in 6-3-9 stayed on its ledge, throwing.
    - Europe and Japan (emulator): the 1-3-5 to 1-3-6 chain (the same: chase on, kept through the door, Spinni out and attacking)
      and Storo's gap in 7-6-12 (the same jump, 144 to 241).
    - All three built and logic-checked, unchanged: Squeak Squad 359 of 498 rooms, the 8 switches, nowhere trapped; NiDL 280 of
      280, the 7 switches, nowhere trapped; with and without thin-floor drops. The window's code compiles (against stubs: the
      Windows Forms SDK isn't here).
  - Not tested: Doc in a chase, the chase ending by leaving the stage, every chase room, a real playthrough of a whole chase, helpers
    and a second player (the wall reacts to every Kirby; the chase ends by the player's), the KatAM chase rooms in Europe and Japan,
    the window itself (it needs Windows), the spoiler log (compiled only).
  - Left: the 5 Ability Scroll chests the DS brings out only in the chase are still there from the start, and its other chase-only
    chests aren't placed (both above). The Squeak Lairs (below) don't depend on the chase: they're always there.
  - The DS's angry colors for the chase's Squeaks are not ported, by the owner's call: "No need to keep the Angry colors since these
    are already in Angry mode."
- **The finale chain**: done (the owner: "move the Squeak Squad final boss sequence in between of Nightmare and KatAM's final boss.
  The sequence should be like this: Entire NiDL final boss sequence that's already ported -> Entire final boss sequence from Squeak
  Squad recreated and ported 1:1 -> KatAM's final boss. The additional sequences only appear when the ROM expension ROMs are
  added to the program."). `KatAMExpansionSqueakLevels.cs`' `SqueakFinale()`, `SqueakShardExits()`; `squeak.c`'s `SqueakDestroy()`;
  the rulebook's rule 8.
  - With both ROMs: Nightmare's ending (Kirby going down from the moon, NiDL's) lands Kirby in the Fountain of Dreams hub, as
    with NiDL alone, and its Amazing Mirror leads to the start of Squeak Squad's final stage (8-4-1, 0x5E2); the stage plays as
    ported (Dark Daroach in 8-4-2, the rooms between by its warp stars and one-way mirrors, Dark Nebula in 8-4-6); Dark Nebula
    leaves no shard: beaten, every Kirby dances out (NiDL's shard exit) into the Dark Nebula hub (below), whose Amazing Mirror
    leads to KatAM's final boss.
  - With the Squeak Squad ROM alone: Central Circle's Dimension Mirror (unlocked by the 8 shards, as KatAM's) leads to 8-4-1, and
    Dark Nebula's way out is the Dark Nebula hub, its mirror to KatAM's final boss sequence (0x38D). With the NiDL ROM alone:
    NiDL's finale as before.
  - **The Dark Nebula hub**: done (the owner: "After the Dark Nebula fight you have to go to a hub room just like the NiDL hub we
    have, so create a copy of it and place it before the last fight sequence to give players and interlude"; then, asked how the
    hubs chain: "One hub per finale (If available) Nightmare -> Fountain of Dreams hub -> its mirror -> (If available) Squeak
    Squad's final stage -> Dark Nebula -> new Dark Nebula hub -> its mirror -> (always available) KatAM's final boss. Without NiDL,
    the copy is made from Squeak Squad tiles."). `KatAMExpansionSqueakHubs.cs`' `BuildSqueakFinaleHub()` (the room after the Squeak
    Squad ability room, 0x5F3 in the USA build), `SqueakFinale()`; the rulebook's finale chain rule. A copy of the Fountain of
    Dreams hub: no other door in or out, no big switch, its Amazing Mirror high up in the middle (to KatAM's final boss), the
    Master Sword's stand under it, a Maximum Tomato on the floor's right (once a visit), a Cutter and a Sword statue on its sides;
    no map's chest (the Fountain of Dreams' stays in its own hub). With the NiDL ROM it's the Fountain of Dreams hub's own look;
    without it, the same layout drawn in the tiles of Squeak Squad's final stage, with Dark Nebula's galaxy behind it and the
    Squeak Squad hub's music. The switch rooms' drawing was split so this room is drawn the same way (their rooms come out the
    same). The Map Viewer puts it in world 8 beside Dark Nebula's room, linked by his way out; it now reads the bosses' ways out
    without the NiDL ROM too (none were drawn in a Squeak Squad-only ROM before).
    - Checked: the ROM data (USA, Europe, Japan): Nightmare's way on to the Fountain of Dreams hub (4,9), its mirror to 0x5E2
      (4,10), Dark Nebula's to the new hub (4,9), its mirror to 0x38D (16,16); without NiDL, Central Circle's mirror to 0x5E2. In
      the GBA emulator: the hub's look in both builds (screenshots); Dark Nebula beaten (his HP set to 1 by a RAM write, then
      Kirby's slide) dancing out into the new hub (USA, Europe, Japan); each hub's Amazing Mirror flown into by buttons with the 8
      shards written in (a forced state): the Fountain of Dreams hub's to 8-4-1, the Dark Nebula hub's to KatAM's final boss, with
      and without NiDL (USA), and with NiDL in Europe and Japan. The logic checks follow the new chain (Squeak Squad's: the
      Fountain of Dreams hub's mirror a way in, the new hub one of its rooms: 394 of 499 reached, 0 trapped; NiDL's: 280 of 280,
      0 trapped). Not tested: the whole chain played through by hand (Nightmare's ending into the Fountain of Dreams hub was
      checked in the ROM data only this time: it's NiDL's own, unchanged), the music in the new hub by ear, the build without NiDL
      in Europe and Japan in the emulator.
  - World 8 no longer has a boss stage or a shard of its own (Dark Daroach and Dark Nebula are the finale's): with Squeak Squad,
    its 7 world bosses leave the shards (one each by default; Bosses Give Random Shards spreads the 8 over them).
  - Checked: the ROM data of all three versions (Nightmare's shard exit and the ending flight's room step to 0x5E2 (4,10); Dark
    Nebula's room 0x5E7's exit to the hub 0x3 (4,9); without NiDL, 0x5E7 to 0x38D (16,16)); the logic checks follow the finale
    as a passage. In the GBA emulator: the Squeak Squad rooms of 8-4 as before (earlier rounds).
  - Dark Nebula's own defeat, 1:1 with the DS's (0x0211DF2C-0x0211CDCC, `squeak/dark_nebula.c`): done. KatAM's mini-boss defeat
    is caught the frame it starts and his runs instead: the controls taken, the far view darkening, three whitenings with his forms
    flickering, his black star in the middle, the controls given back, his eye shaking, sparks thrown off him and bouncing on the
    floor (a new falling kind of Squeak Squad effect, `squeak.c`'s `SqueakFallInRoom()`), his star shaking then breaking up, his
    eye left alone, its twinkle, the white-out; his music fades out with it (his template's unk22 bit 2 keeps KatAM's defeat from
    putting the room's music back). Here the white then clears and the finale's dance-out goes on.
  - Fixed on the way: in his own room (8-4-6, 0x5E7) his whole fight ran about 100 pixels over the screen (his floor was taken
    from his spot, the dark star's in the air): now the floor under him, the DS's.
  - Checked in the GBA emulator (USA, Europe, Japan; forced: his fight played to his first idle, then KatAM's mini-boss defeat's
    writes made by hand, as `sub_0809D060` makes them on a killing blow): every state of the DS's defeat with its timings, the far
    view darkening and coming back, the controls taken and given back, the sparks, the break-up, the white-out, the dance-out
    after. His coming and fight now on screen in 0x5E7 (his spots logged against the DS's).
  - Not tested in the emulator: a real killing blow (a scripted Kirby's slide never lands: mini-bosses' unk5C makes them immune to
    it, and the engine's other checks weren't worked out), the whole chain played through (Nightmare's ending into 8-4-1; Dark
    Nebula beaten, the dance out to the hub), the music fade by ear.
  - His last touches (the owner: "Add the Dark Nebula last touches."): done. His background tint by form (the DS's 0x021225B4 and
    0x021222E4): the far view goes toward his form's color over 120 frames, at his fight's start and at each form he picks (fire
    red, spark green, ice blue), and keeps it through KatAM's palette effects (a copy ability's flash used to wipe it); back to its
    own colors at his defeat. His HP bar shows from the end of his black star's wait with its sound (0x020B308C), not from his
    coming. His music: his room's own (map music 37), never changed through his fight, as on the DS. The two other leftovers were
    traced and left out on purpose: the message at his white-out (0x020217D0) only sets an effect's animation while the screen is
    all white, and the stage's +0x9C bit 2 only holds a few of the DS Kirby's own states, which KatAM's Kirby doesn't have.
    - Checked in the GBA emulator (USA; the tint also in Europe and Japan): the far view's colors logged and screenshots through
      his forms (spark green, then toward fire red), through a copy ability's flash, and back to their own at his defeat (his HP
      set to 1 by a RAM write, then Kirby's slide: his whole DS defeat ran, the next room's colors untouched); the song the same
      from his room's start to his defeat; the bar showing after the wait (KatAM's HUD brings it in about 55 frames later, then
      fills it, as for every boss). Not tested: a whole fight played by hand, the music by ear.
- **The Squeak Lairs**: done (the owner: "I think Squeak Lair doors should be ported to the game and treated as miniboss rooms
  Squeak Squad rooms lead to and then exit to the next expected exit. Like instead of Entrance 1 -> Entrance 2, it would be great
  to add them as Entrance 1 -> Squeak Lair with Miniboss -> Entrance 2 as an interlude between zones in the Squeak Squad map. This
  should use the same system NiDL made us create of Beat Boss, reveal exit, only 1 exit per boss lock in room. That way we can
  reuse these boss rooms sensibly. The minibosses on these rooms should be placed sensibly based on context, room layout, and
  boss abilities. Any boss can appear in these lairs and these can also be randomized as well."). `KatAMExpansionSqueakLevels.cs`'
  `PlanSqueakLevels()` (SqueakLairPlan), `KatAMExpansionSqueakRooms.cs`' `SqueakObjects()`, `KatAMEnemiesBase.cs`'
  `AddSqueakLairMinibosses()`, `squeak.c`'s `SqueakBossFloor()`; the rulebook's rules 5 and 8.
  - 38 lairs: each Squeak that steals a stage's treasure on the DS (Spinni, Doc or Storo) has its own hideout (the DS's: one of
    the stage's rooms after its goal, where it fights with its Squeakers) as a mini-boss room between two of the stage's rooms:
    the DS door into the room the thief waits in (else the nearest to it) leads into the hideout, one-way, onto the DS's arrival;
    the Squeak fights as a mini-boss (KatAM's arena: the camera held, its HP bar), with up to 4 of the hideout's Squeakers; the
    one way out is shut until it's beaten, then shows (KatAM's one-way door) and leads where the door led. 6-5 has three (its
    three thieves); 7-4 has no Squeak thief, so none.
  - Their mini-boss is randomized with KatAM's mini-bosses (Randomize Mini-Bosses; with Include Bosses, any boss can be there): a
    boss there stands on the floor Kirby lands on, whatever ledge the Squeak's spot is on.
  - Checked in the GBA emulator (USA): Kirby walked onto 1-3-5's lair door (by buttons) and took it with Up into Spinni's hideout
    (1-3-9), landing on its doorway, Spinni attacking with the Squeakers on the ledges; held on the way out's cell (Lua) with
    Spinni alive, no mirror and Up did nothing; Spinni's HP set to 0 by a RAM write (a forced state: he isn't destroyed by it),
    the mirror showed; Up took Kirby to 1-3-6, where the door led. Logic checks (all three versions): Squeak Squad 393 of 498 rooms
    reached (359 before), the 8 switches, nowhere trapped, with and without thin-floor drops; NiDL unchanged.
  - Randomized (USA, seed 77, mini-bosses random, Include Bosses): the 38 lairs' mini-bosses became KatAM's (a KatAM mini-boss
    there takes facing 0, not the Squeak's behavior byte), NiDL's and Squeak Squad's mini-bosses and bosses; in the emulator,
    1-3's lair had Dark Nebula, his HP bar up, fighting over the floor. Europe and Japan (emulator): 1-3-5's stump and its door
    into Spinni's hideout, Spinni attacking, the same as USA.
  - Not tested: a real fight won in a lair (the exit after a real defeat), the other 37 lairs in the emulator, a KatAM mini-boss in
    a lair in the emulator.
  - Left: the other hideouts of each stage (each stage has the same four, Squeakers / Spinni / Storo / Doc; the DS uses the thief's)
    stay unused, nothing leading in: the owner may want them used too (more interludes per stage).
- **The lairs' nests**: done, with the DS's own art (the owner, sending two screenshots of the DS's: "The original entrace 'nests'
  things should be placed where the door where this Squeak Lair area leads to, to provide the player a warning, that, that will be
  a miniboss fight to go to the next area in Squeak Squad stages."; then: "Look for the nest entrance art since this is definitely
  on the files."). Found: every DS room's front block table holds its world's nest, 5 blocks by 3 (blocks 0x110-0x114,
  0x130-0x134, 0x150-0x154), drawn with the room's front tileset (`map/NN_f.bin`): stumps, grassy mounds, rock piles, castles,
  igloos, a submarine, factories, UFOs, each with its flag (the owner's two screenshots are 04_f's stump and a rock pile). No map
  places them (the DS must write them in when a lair door shows; not traced in its code). Now each lair's door has its room's own
  nest drawn into the room's foreground, its doorway on the door, its feet on the door's floor, behind Kirby
  (`KatAMExpansionSqueakNests.cs`' `SqueakNests()`); the drawn-from-screenshots sprite is gone. Every one of the 38 doors' rooms
  has a nest in its tileset (4-3-8's is the DS's black silhouette, as that room's terrain). Checked: all 38 drawn from the DS
  files at their doors (a picture), and in the GBA emulator (USA): 1-3-5's stump, 4-4-7's factory, 8-2-6's UFO and 4-2-3's mossy
  stump in the game, Kirby in front.
- **The lairs' exits**: done (the owner: "The black exit of the Squeak Lairs should be replaced with the 1 way door. Squeak Lairs
  are not 2 way in this implementation of them. Do make sure that these exits only appear after defeating the miniboss only. These
  can have any miniboss in game not only mouse minibosses."). The hideouts' black DS doorway (their third map layer) isn't drawn
  in a lair; the way out is KatAM's one-way door, hidden and shut until no mini-boss with HP is left in the room, in every build
  (before, without the NiDL ROM it was open from the start): `squeak/gimmicks.c`'s `LairExit()` (gimmick entry 0xF2), whatever
  mini-boss is there (types 0x38-0x52: KatAM's, NiDL's or Squeak Squad's; the randomizer's mini-bosses already take the lairs'
  with KatAM's, Include Bosses any boss). Checked in the GBA emulator, 1-3-9 (Spinni's hideout), USA with and without the NiDL
  ROM, Europe, Japan: no door on the exit's cell while Spinni fought; his HP set to 0 by a RAM write (a forced defeat: KatAM's own
  defeat runs from there), KatAM's one-way door showed there; Kirby put on it (Lua) took it with Up to 1-3-6, where the lair's
  door led. Logic checks unchanged (all three versions, and without the NiDL ROM). Not tested: a real fight won, another
  mini-boss in a lair.
  - Then (the owner: "The one way door on the Squeak Lairs should be where the black door was..."): the way out is always on
    the hideout's doorway, where the black doorway was, also where Kirby lands beside it (Spinni's and the Squeakers' hideouts;
    before, it went to another floor there, 1-3-9's on its right ledge). Checked in the GBA emulator, 1-3-9, USA and Japan: no
    door on the doorway while Spinni fought; after his forced defeat (a RAM write) KatAM's one-way door on it, and Up there took
    Kirby to 1-3-6. Logic checks (all three versions, and without the NiDL ROM): Squeak Squad 393 of 498 rooms, the 8 switches,
    nowhere trapped, with and without thin-floor drops; NiDL 280 of 280.
- **Randomize Squeak Lairs**: done (the owner: "I think we can store the Unused Hideouts and actually randomize these per run, as
  in these hideouts can be anywhere since these are area transitioners. So, it would be a good idea to randomly remove or add these
  connections to these rooms in a per seed basis as a gimmick to the Squeak Squad ROM Expansion. Of course, we have to have an
  unrandomized map as well for times where these won't be randomized, for this, the current layout is perfect."). A toggle in the
  ROM Expansion tab's Squeak Squad group, Randomize Squeak Lairs (with Include Rooms): the lairs are picked by the seed among all of
  each stage's hideouts (the ones the DS leaves unused too), each one time in three, so a stage may have none or several; each
  between two of its stage's rooms, at a DS door the seed picks among the ones that can be one and that show the DS's nest; the
  Squeakers' hideout gets Spinni, Doc or Storo (from the seed) to lead them. Off (and in an unrandomized ROM), the lairs are the
  current 38: the USA ROM made without it is byte for byte the one before this change. The spoiler log lists the seed's lairs
  ("Squeak Lairs"). `KatAMExpansionSqueakLevels.cs`' `PlanSqueakLevels()`, `KatAMExpansionSqueakNests.cs`' `SqueakNestKey()` and
  `SqueakNestDrawn()`, `KatAMSpoilerLog.cs`; the rulebook's rule 5.
  - Checked: seeds 77 and 1234 (USA) gave 37 and 44 lairs; every door's nest drawn from the DS files (all 37 of seed 77's in a
    picture, after leaving out the rooms whose tileset or blocks have no nest: 8-1-1's 20_f and 8-3-9's own blocks had drawn
    garbage); logic checks: Squeak Squad 392 and 399 of 498 rooms reached, the 8 switches, nowhere trapped, with and without
    thin-floor drops. In the GBA emulator (seed 1234): 7-1-7's door, its grassy mound nest beside Kirby, Up (Kirby put on the door
    by Lua) into 7-1-8, a Squeakers' hideout, where Spinni fought Kirby with the Squeakers, his HP bar up; no door while he
    fought; his HP set to 0 by a RAM write (a forced defeat), KatAM's one-way door showed on the right ledge.
  - Not tested: the window itself (it compiles; this machine can't show it), Europe and Japan with the option, the way out of a
    per-seed lair taken in the emulator.
- **Leftover areas**: 1-1-5 (the field past 1-1's goal, 100 by 15) was the one room of the DS's own not reached: it now has a
  hidden one-way door in, as any room (rule 9). The others not reached are the hideouts and gold-pile rooms no lair uses (above)
  and the sky rooms the DS's warp stars fly through (3-4-5, 8-3-6, 8-3-7: the port's warp stars fly through them, Kirby never
  stands in them).
  - The sky rooms (the owner: "Add the warp stars necessary to transport the player to these rooms."): the warp stars that take
    Kirby there are already in, the DS's own: 3-4-4's star flies him up through 3-4-5 (a cloud shaft) into 3-4-6, and 8-3-5's
    through 8-3-6 (a crystal shaft) and 8-3-7 (a crystal corridor) into 8-3-8, as on the DS. Checked in the GBA emulator (USA),
    Kirby put on each star: his room went 3-4-4, 3-4-5 (about 185 frames), 3-4-6, and 8-3-5, 8-3-6 (about 260), 8-3-7 (about
    270), 8-3-8, each shown on the way. None was added: the three rooms have nothing to stand on (no floor), so a star landing
    Kirby in one would need a floor built there (a design change: the owner's call). The Map Viewer and the logic checks count a
    ride by where it lands, so they still list these three as not reached.
- **Squeak Squad backgrounds' bottom rows**: fixed (the owner: "some Squeak Squad backgrounds have some stretching artifacts at
  the bottom, this should be fixed immediately so backgrounds look normal in-game"). The far views were shrunk to 3/4: the DS's
  192 rows became 144, short of KatAM's 160-row screen, and the last row was repeated under them in streaks. Now one that would be
  shorter than the screen is shrunk less, both ways, so its own rows fill it (`ConvertSqueakFarView()`). The background space this
  took was found by sharing identical room tiles and maps (`AllocateShared()`: about 500 KB, which also fixed the European build
  running out). Checked: every converted far view drawn out (no repeated rows left) and the Squeak Squad hub in the GBA emulator.
  Not looked at in game: each room whose background's bottom shows.
- **The Collection Room scrolls left and right**: done as one grid (the owner, with the rooms: "the collection room should be
  scrollable left and right to accommodate for all these new items"; then, of a second page: "I wanted an scrollable menu from
  left to right to showcase all treasures in one single screen. I didn't ask for a second page, I asked for a seamless
  scrollable menu", keeping the base game's items). The Collection Room (from the main menu) shows KatAM's 10 x 4 cells as
  KatAM does, then more columns to their right with the expansion's treasures; the screen shows 10 columns. The cursor going
  past the last column shown (or the first) slides the whole grid a column, cells, icons and the lattice together, the striped
  background staying (5 frames); past the grid's last column it goes round to the first, and Left on the first to the last;
  L and R move it a screen. The new columns hold Squeak Squad's 5 sprays (their row: KatAM's sprays' row), the NiDL levels' and
  Squeak Squad worlds' maps (the third and fourth rows: above), and the Secret Map's pieces (the second row: above). `collection.c`,
  `KatAMExpansionCollection.cs`.
  - Tested in the GBA emulator, sprays owned by a RAM write (a forced state, not a playthrough): USA (sliding a column with
    Right at the last column, the frames between, both ends going round, L and R, choosing a new color after a slide, leaving
    with B to the main menu, which looks as it should, coming back in at the start), Europe in its 5 languages and Japan (both
    ends, the composed names). The Sound Player only opens with the grid at its start (its cell is in KatAM's first column),
    where everything is KatAM's.
  - Not tested: holding Right for several slides in a row with real key repeat (each slide was pressed on its own), the Sound
    Player's screens after slides back to the start.
- **NiDL's music and sound effects** (the owner: "Implement the sounds and music and enemy sound effects for both NiDL and Squeak
  Squad"): done for NiDL. Its whole song table (579 songs: music, sound effects, their instruments and samples) is copied into
  KatAM's grown song table after KatAM's 620 (`KatAMExpansionSound.cs`; NiDL's song n is KatAM's 620 + n). The NiDL rooms play their
  own room's music, the switch rooms their level hub's; a ported mini-boss or boss in a room with NiDL music keeps it (NiDL's
  boss rooms set none and keep KatAM's fight music). The ported code plays NiDL's own sounds, found in NiDL's code (its sound call
  `0x080031B8` and the room-checked `0x0803E34C`, each call matched to its move): the bosses (King Dedede, Paint Roller, Kracko,
  Heavy Mole, Mr. Shine & Mr. Bright, Meta Knight, Nightmare, the Power Orb's dash and crescents); the mini-bosses (Bugzzy, Fire Lion,
  Grand Wheelie, Mr. Tick-Tock, Poppy Bros. Sr., the Meta-Knights: their landings, dashes, charges, throws, the alarm, the notes,
  the warp); the enemies (Chilly's freeze, Blade Knight's slash, Needlous' roll, Gip's throw, Kabu's and Slippy's jumps, Twizzy's
  take-offs, Bubbles' bounces; the others make none in NiDL); the abilities (Freeze, Mike's three songs, Needle, Hi-Jump, the Star
  Rod's star). Looping ones stop when their move does (Freeze's, also when Kirby is hit or in water; Mr. Tick-Tock's alarm, also
  when it's hit or gone).
  - Checked in the GBA emulator (USA, EU, Japanese): the music player on NiDL's songs in rooms 0x06F and 0x2EF (imported song
    headers at `0x0966xxxx`); King Dedede's fight (0x313) on NiDL's music with his effects; Mr. Tick-Tock's fight (0x2EF, USA:
    its run 0x1F6, notes 0x1FB and alarm 0x219 on the effects player, the alarm cut by the next note as in NiDL; EU and Japanese:
    the same three imported effects). Logic checks unchanged in all three.
  - Not tested in the emulator: the abilities' sounds (an ability set by a RAM write didn't take; they're built only), and the
    other mini-bosses' and enemies' sounds one by one (the same call as Mr. Tick-Tock's). Not found in NiDL's code and so still
    KatAM's: Light's and Ball's sounds (Ball's code wasn't identified), Whispy Woods' puffs and the Power Orb's other shots (NiDL
    plays none there), the bosses' hit and defeat sounds (KatAM's own for its bosses), Kirby's own common sounds. The NiDL hub (a
    Central Circle copy) keeps KatAM's music.
- **Squeak Squad's music and sound effects** (the owner: "Import Squeak Squad's sounds and adapt them to the GBA"; "Implement the
  sounds and music and enemy sound effects for both NiDL and Squeak Squad"): done. `KatAMExpansionSqueakSound.cs` reads the
  owner's Squeak Squad ROM's `sound/sound_data.sdat` and compiles it for KatAM's MP2K engine: the wave archives (all IMA-ADPCM)
  decoded to 8-bit samples, the instrument banks turned into voicegroups (key splits, drum sets, the DS's envelopes on KatAM's),
  the sequences into MP2K tracks (loops as jumps, calls inlined, the DS's tempo, volume on the DS's curve). They go after NiDL's
  in a second grown song table: the 43 music ids first (the DS's table at `0x02159770`), then all 760 sound ids (`0x02159DDC`), so
  Squeak Squad's sound n is `SQUEAK_SE(n)` in the C code. Each Squeak Squad room plays its own map's music (its header's byte
  0x0B; 0 keeps the one before, as on the DS); a world's boss fight keeps its arena's music, the DS's boss themes (21, 33, 34),
  where the mini-boss framework used to switch to KatAM's mini-boss music. The gimmicks play their DS sounds (shutters, barrels,
  grass, fire, icicles, rocks, platforms, clouds' break and thunder, ropes, boulders, quakes, the sound gimmick), and every ported
  enemy, mid-boss, Squeak and boss plays its own: found in the DS's code (every call of its sound functions `0x02013EE8`,
  `0x02013E6C` and `0x020B0604`, matched to the port's states by their DS addresses) and placed in the same state; the sounds the
  DS keeps playing (Bubble Head's big blow, Maiga's bite, King Dedede's inhale, Tedhaun's hum, Doc's saucer, Daroach's charge,
  Mecha-Kracko's field, Dark Nebula's and the dark star's arrival) are stopped where the DS stops them, when a hit starts the character over, and when it goes.
  - Fixed since (found as the dark star's rooms froze the game): the DS's sequences wait out each note unless they say otherwise
    (its note-wait mode, on by default, switched by the sequence's 0xC7); the conversion had only followed the music's (which turn
    it off), so the sound effects' notes had no waits, and a sound effect that loops (the dark star's) ran forever in one frame.
    Each note now waits as the DS's mode says, and a loop with no wait in it gets one. All 1765 converted songs checked by a scan
    (none loops without a wait); the dark star's rooms played in the emulator. This changes every Squeak Squad sound effect's
    timing to the DS's.
  - Fixed on the way, for NiDL's sounds too: the expansion code's `PlaySfx()` checks the song's player and priority in
    `gSongTable`, linked at KatAM's address; the table's move only repointed KatAM's own code, so those checks read past KatAM's
    620 songs and refused some sounds at random (Mrs. Moley's thrown rocks and digging never played). The move now repoints the
    expansion's code too.
  - Checked in the GBA emulator (USA): room 1-1-1 plays its DS music (the music player on an imported song); King Dedede's arena
    (0x42E) plays the DS's boss theme 21 through the fight with his sounds (his inhale 0x181 ending with it, the impact 0x183 and
    0x15A, the jumps 0x17E); Mrs. Moley's arena (0x46B) on the boss theme 33 with her looks, dives, digging, holes, throws and
    rocks (0x191, 0x195, 0x194, 0x197, 0x198, 0x199); 1-3-1's grass 0x59; 1-6-1's Maiga hops (0x132) and bite (0x133, gone once it
    let go); 4-3-2's sound gimmick (0x32); NiDL's Mr. Tick-Tock (0x2EF) still with his. Europe and Japan: King Dedede's arena on the same boss
    theme with his sounds; both built and logic-checked (Squeak Squad 359 of 498 rooms as before, all 8 big switches, nothing
    trapped; NiDL 280 of 280; with and without thin-floor drops).
  - Not tested one by one in the emulator: the other characters' sounds (built from the same calls), the loops' stops in every
    case. Where the DS's own timing wasn't followed: King Dedede's and his hammer walk's steps every 25 frames (the DS's on two
    frames of the walk's animation). Not ported: Starman's (none in its own code), the elemental hurt voices of the Squeaks and
    the bosses' hit and defeat sounds (KatAM's hurt and defeat stand in), mid-bosses' fight music in ordinary rooms (KatAM's
    mini-boss music: the DS's own wasn't found).
- **ROM size** (the owner: "if needed, expand the ROM size to fit all this new content but only if needed"): 32 MB is the most
  the GBA maps (0x08000000-0x09FFFFFF), and the expansion is already there, so it can't grow. With both expansions in, the
  expansion's 16 MB has about 530 KB left (USA build; identical room tiles and maps are now stored once). The next big additions
  will need room made first (sharing more, or compressing the Squeak Squad samples).

## 10. Screen shake as KatAM's
- The owner: the screen shakes far too much. Mr. Shine & Mr. Bright's and King Dedede's endless shakes are fixed; anything else
  that shakes too much: tell where.
- Squeak Squad's stone shutters shake it as the DS's do (the owner: "Stone shutters should shake the screen like KatAM does for
  consistency sake"): KatAM's timed shake (its third, 24 frames, nearest the DS's 22) as a stone pillar shuts or opens, not the
  spikes (`squeak/gimmicks.c`'s ShutterSound). Checked in the GBA emulator (USA, 1-4-3: Kirby walked onto its switch, the pillar
  crumbled up with the screen shaking for 24 frames).

## 11. Polish the Squeak Squad hubs
- The owner: "Add a pending tasks to the list that we need to thoroughly polish the Squeak Squad hubs." The Squeak Squad hub (the
  Central Circle copy, its grey door, its mirror and doors' places and looks, still provisional), the 8 world switch rooms (their
  provisional tiles, statues and pedestals): a thorough pass on looks and layout, the owner's picks first (the rulebook's frozen
  designs stay as they are unless the owner says). The Squeak Squad ability room is no longer part of it: it's Squeak Squad's own
  room now, as the owner asked (see its entry above).
- **The Squeak Squad hub's background and cloud colors** (the owner: "Change the background of the Squeak Squad hub along with its
  palette for the clouds, show me options in a screenshot"): done, the owner picked D (sending its mock-up: "The Squeak Squad hub
  should have this background as well"), now in: Prism Plains' castle and rainbow with Central Circle's own cloud colors. Checked
  in the GBA emulator (USA, a screenshot of the hub). The hub no longer shows Central
  Circle's background: a Squeak Squad far view, converted as its rooms' (shared with the rooms showing the same view), panning
  across the hub as the NiDL hub's does, and Central Circle's cloud and pillar colors (its palettes 4 and 5; the cycled colors
  are in other palettes) tinted toward it, in a palette of the hub's own. Five mock-ups, built and shown in the GBA emulator
  (USA, three spots in the hub each): A, Cushy Cloud's sky (d_viw_09), clouds sky blue; B, twilight over the clouds
  (d_viw_17), lavender; C, starry night (d_viw_26), violet and darker; D, Prism Plains' castle and rainbow (d_viw_00), Central
  Circle's own cloud colors (tinted, they went green); E, the galaxy (d_viw_1e), lilac. `KatAMExpansionSqueakLevels.cs`'s
  `SqueakHubLooks` (name, view, hue, saturation, how much, brightness), `SqueakHubLook` the one in.
  - Europe and Japan: look A seen in the hub in the emulator. Not tested: the hub's world doors opened (their own palettes,
    untouched) with each look.

## 12. Map Tracker: every Kirby on the map
- The owner: "Can you make it so the map tracks the position of all Kirby's and draws arrows based on what position they are in
  the map?", then: "I meant these arrows. I don't care about where the kirby moves in the room. I would only want to see a Kirby
  icon in the room if the Kirby is inside the room, if I zoom in our out or the Kirby companion is not visible on the screen, or
  I'm not visible on the screen, the arrows will point to where the kirby is in the map, same as my Kirby." Done: each Kirby (you
  and the three CPU Kirbys) is his head in his color in the square of the room he's in, the game's own sprite from the pause
  screen's area map (its 16x16 head, the four players' palettes: read off the area map in the GBA emulator, USA, then found in all
  three versions: tiles 0x628CCC USA, palettes 0x4BB10C, 0x4BB1AC, 0x4BB1CC, 0x4BB1EC), alone in the middle or, with others in
  the room, each in his own corner as the area map puts them. Where he stands inside the room isn't drawn. A Kirby whose room is
  out of the window (zoomed or panned away, you included) is an arrow in his color on the window's edge pointing at his room.
  Kirbys in rooms the map doesn't show yet get none. `View > Show All Kirbys` (on by default) turns them off. The emulator script
  (version 5) reports every Kirby's room a few times a second (`KIRBYS`; `gKirbys` at `0x02020EE0`, `gNumKirbys` at `0x0203AD44`).
  `KatAM Map Tracker`: `Core/MapEntities.cs` (the heads), `MapView.cs`, `TrackerForm.cs`, `Lua/katam_tracker.lua`, `TrackerServer.cs`.
- The owner: "the map lags real bad when you zoom out too much." Fixed in `MapView.cs`: zoomed out, the map (regions, lines, rooms,
  their icons) is drawn once into a picture at that zoom and only copied while the view pans, the current room pulses, the cursor
  moves or the Kirbys move; it's drawn again when the zoom settles at another one or what it shows changes (a room or mirror found,
  Reveal, the icons shown...), and scaled while a zoom eases. Before, every one of those redrew every room and line (an expanded
  map: 1,109 rooms, 1,920 mirrors), and each line piece made a new brush every time. Zoomed in, only what's in the part being
  redrawn is drawn. The picture is used up to zoom 0.84 on the expanded map (12 million pixels at most), 1.56 on KatAM's own.
  - Checked: the heads read from the USA, European and Japanese ROMs and an expanded one by the tracker's own code (a console
    harness), the same pixels in all four and as the area map shows them. The tracker builds (against Windows Forms' reference
    assemblies). Not tested: the window itself (it needs Windows), so neither the heads and arrows nor the smoother zoom have
    been seen on screen; how much smoother it is wasn't measured.

## 13. The ROM Expansion's progress while the ROM is made
- The owner: "I would need you to actually reflect the process of the ROM Expansion and where is at at all times, plus make it count
  in the progress bar. [...] Ensure the program does not get stuck while also making ROM progress obvious." Done: the expansion
  tells the window every step it is on (`KatAMExpansion.Progress`), shown as "ROM Expansion: ..." under the bar, which fills from
  5% to 40% as it goes: drawing the enemies and characters, NiDL's rooms counted one by one, Squeak Squad's rooms' tiles and far
  views counted one by one, the rooms written one by one, each switch room, the hubs. Each part of the bar is its share of the time
  it takes, so the bar moves evenly (a part for what isn't added takes none).
- It wasn't stuck, only slow: Squeak Squad's 42 far views (their colors fitted to KatAM's palettes) took a minute and the rooms'
  tiles 15 seconds, one at a time with nothing shown. Now: the far views' and the busiest rooms' tile merging work out each pair
  of tiles' difference once instead of every round (the same pairs merged, the same bytes), and the far views and the rooms' tiles
  are made on all the computer's cores at once, then written in the same order. The whole expansion: 91 seconds before, 26 now (USA,
  both ROMs, on a 4-core machine; a computer with more cores is faster still).
  - Checked: the ROMs are byte for byte the ones the old code made (USA with both ROMs; USA with Squeak Squad only, with its chases
    and randomized lairs), so nothing in the game changed. Every far view's and every room's conversion gave the same bytes too.
    The steps as the window gets them (the e2e harness printing each one with the time): the bar never goes back, reaches 100%,
    and no step is silent for more than 3 seconds, with both ROMs, NiDL only, and Squeak Squad only. All three versions built,
    and the logic checks are unchanged (Squeak Squad: 394 of 499 rooms reached, every switch pressed, none trapped; NiDL: 280 of
    280, none trapped; with and without dropping through thin floors). The window compiles. Not tested: the window itself (it
    needs Windows), so the bar and its text haven't been seen on screen.

## 14. Squeak Squad's levels: two-way mirrors, the camera, platforms, ropes, switches
- **Two-way mirrors and branching** (the owner, high priority: "Squeak Squad's rooms are plagued with 1 way mirrors. The map layouts
  should encourage exploration with Two-ways as we defined in the rules. This makes map exploration stupidly linear. There's not even
  branching paths as I requested."): the DS's doors only lead on (an arrival has no door back), so every DS door that can now has a
  way back: a mirror where Kirby arrives, leading back to the door he came through (`PlanSqueakLevels()`'s ways back, `DoorKind.Back`:
  139 of them), not from a goal, a fight, a lair's way or a hidden way, and only where it can't strand him (a flood from the DS door,
  the stone shutters as walls, reaches the room's entries: `SqueakShutCells()`, `Rejoins()`). The branches into other stages are
  two-way too (31 two-way with their 31 ways back, 6 one-way where no way back fits), and a stage's way in can be any of its rooms
  that holds one, not only its first. The rulebook's rule 1 and rule 3 say so ("Squeak Squad's doors lead both ways too").
- **Black doors become mirrors** (the owner: "All doors have to be mirrors unless the door is wide and warps to another place that is
  not a goal room."): every DS door gets a mirror (one-way where it has no way back, hidden until the block over it breaks), but the
  wide doorways leading on (the goals' 31 plain ones), a nest (a lair's) and a hidden way. One door in all the rooms has none: its
  room is at KatAM's 31 objects.
  - Checked: the logic checks (USA, Europe, Japan; with and without dropping through thin floors): Squeak Squad 394 of 499 rooms
    reached, the same as before, every big switch pressed, none trapped; NiDL 280 of 280. In the GBA emulator (USA, a forced
    start): the two-way round trip between 0x400 and 0x401 and the mirrors' looks. Not tested: a whole stage played through.
- **The camera's bottom and the pits** (the owner: "Some death pits in some rooms are too high up, the camera barely scrolls a bit
  down and if you touch it for a bit, you instantly die even though you're still in a livable area. Room 0x48F for example. In
  general, rooms are 2 tiles off in the camera locking"): measured in the DS emulator (Prism Plains 1's first stage, 1600 x 240),
  the DS's screen never shows a stage's last two rows (y 16-208 there, held with Kirby at 216). Squeak Squad's rooms now stop the
  camera 32 pixels over their bottom as the DS does (`rooms.c`'s `ROOM_DS_BOTTOM`; it was 8), so the filler rows don't show and
  Kirby falling into a pit leaves the screen before he's lost. Checked in the GBA emulator (USA, 0x48F: Kirby dropped into the
  right pit falls out of view, then loses the life; the view stops at y 208). The DS also hides its stages' top row and 16 pixels
  at the sides; the port still shows the top row and hides 8 at the sides: not changed (not asked; say if they should match).
- **Hanging platforms and ropes** (the owner: "Are movable platforms supposed to be swallowed? I just swallowed one.", "If I hit the
  platform, the platform falls but if I hit the string with a cutting ability, it stays there. Is that expected behavior? The string
  never goes away when hit.", "Entities go through the movable platforms, which doesn't happen in Squeak Squad."):
  - Platforms, thunder clouds and ropes can't be inhaled any more (unk5C 0x20).
  - A rope is cut only by a cutting attack, as the DS's (its flag 8: here a Kirby with Cutter, Sword or Master), not by an inhale or a
    plain hit near its plank, which is what dropped the plank before. Cut, the rope goes as on the DS (its stage clears the rope's
    cells from the cut out): each rope cell has a block record now, the cell without the rope, so it vanishes cell by cell from the
    cut and can't be climbed. Checked in the GBA emulator (USA, 3-3-7, room 0x48F): with a real Sword the left rope vanishes top to
    bottom in a few frames and its plank shakes, falls and breaks, the right one stays; plain Kirby inhaling at both for a second
    leaves them.
  - Enemies stand on platforms and clouds and ride them (KatAM's own objects carried as Kirby is; Squeak Squad's, which move
    themselves, land on them in their own move). Checked in the GBA emulator (USA, 0x48F: the room's Gold Waddle Dee dropped over the
    right plank lands on its top, y 128, and walks along it; before, it fell through to the floor).
- **The silver chest** (the owner: "The silver Treasure Chest has a hitbox, meaning if I use an ability the attack clashes with the
  chest, this doesn't happen in the vanilla KatAM."): not reproduced. The silver chest is Squeak Squad's big (platinum) chest, which
  is KatAM's own big chest with Squeak Squad's look; its object is the game's, with no hit box for attacks (unk5C kind 0, as
  vanilla's). In the GBA emulator (USA, room 0x409's big chest) a real Cutter flies through it and back, Sword swings and Fire pass
  over it, nothing stops. Which ability, and which room or chest (one a Squeak was carrying?), would show it?
- **Stone door switches** (the owner: "Stone door buttons do not have collisions for ability usage. If I throw a knife at them, they do
  not activate which this doesn't happen in KatAM."): Squeak Squad's star switches are pressed by attacks too now, thrown ones
  included (as the DS's: a body or an attack presses it), not only by Kirby touching them. Checked in the GBA emulator (USA, room
  0x419): a real Cutter thrown from 48 pixels presses the switch and the switch stays down; inhaling at it presses it too and
  doesn't swallow it.
- Not tested in any of these: a playthrough; Europe and Japan beyond the builds, the logic checks and the room booting in the
  emulator.
