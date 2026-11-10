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
      broken (KatAM's break only under Hammer and cannon shots): the owner's call if they should change too.
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
      a regrown tuft, an enemy or ability star let out, Europe and Japan in the emulator. Left out: hiding in an empty barrel
      (KatAM has no such move).
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
      cloud's look that zap on their two timers (rest 30 frames, zap 30), hurting a Kirby touching or riding them. Checked in the
      GBA emulator (USA and Japan, 3-5-1): Kirby stands on a cloud between zaps, a zap hurts him and knocks him off. The zap's look
      and length are approximated (the DS's phases follow its animations); a moving cloud not checked in the emulator.
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
      at him, hits him, bursts on the wall), Europe 2-4-1. The landing and bursting quakes are left out (screen shake, item 10).
      Not tested: 4-3-2's, 8-2-4's, Japan's boulders. Logic checks: 359 rooms both ways, 8 switches, no traps, all three versions.
    - The candles (0xA: the notes' candles bullet; 8, in 4-3-7, 4-3-8, 8-1-6): Fire or Burning lights one for its frames (400-540),
      then it goes out. Checked in the GBA emulator (USA and Japan, 4-3-7, Kirby with Fire). The dark room and Light: the candles and Light
      request above.
    - The camera areas (0x14) and the ambience gimmicks 0x18 (snow, wind streaks), 0x19 (lava sparks), 0x1A (a sound): decoded,
      none changes the way through (the notes); not ported: the camera's view stays KatAM's, the ambience is left for later.
    - The quake triggers (0x13: the notes' quake bullet): ported as a trigger area that messages its targets once (a Kirby within
      64 pixels of its box, or a switch's message, sets it off; the screen quake is left out). On the DS 4-3-2's is set off by a
      rolling boulder; here a Kirby nearing it opens its way as before. Not tested in the emulator.
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
    - Gimmicks left (the notes have what's decoded): the camera areas (0x14) and the ambience 0x18, 0x19, 0x1A (above: not
      ported), gimmick 0x12 (a color fade, no effect on the way), and:
      - The candles and the Light ability (the owner: "Add to the pending list candle and light ability interaction"): the
        Light ability (NiDL's, from Cool Spook) lighting the dark rooms' candles, as Fire does on the DS.
      - Hiding in an empty barrel (KatAM has no such move).
    - Vitality halves: done (the owner: "Two halves make one Vitality"). Each Vitality chest holds half: KatAM's Vitality,
      taken back on a pair's first half, so max HP grows on every second one (`chest.c`, `gSqueakVitalityChests`). Tested
      in the emulator in all three versions on 1-6-7's chest (Kirby walks onto it and presses Up): as the first half, max HP
      stays 6; with another half marked found by a RAM write, Vitality 1 and max HP 7. Not tested by opening two real chests
      in a row. Three of the eight are placed (1-6-7, 2-6-7, 3-6-2); the other five are chase-spawn chests (on hold).
    - Left: the chests' other Squeak Squad treasures (Ability Scrolls, the Secret Map's pieces: with their items, below; the
      Spray Paints are done, below), the chase spawns after the stage's treasure (the Squeakers among them, and 22 chests), the arenas
      without the NiDL ROM (NiDL's type holds the camera), the gimmicks, the areas.
  - Ability bubbles whose ability no regular enemy gives (Hammer, Bonkers' in KatAM; Squeak Squad's ability numbers 12, 18 and
    26) become food (the owner: "In the case of bubbles without no regular enemy, you can swap them out for food.").
- **Squeak Squad's spray paints**: done (the owner: "The Spray Paints should be the spray colors from Squeak Squad as new items in
  the collection screen", then: "remove any duplicate spray color, it would be pointless to have multiple if these are already in
  KatAM"). Squeak Squad has 18 sprays; 13 are KatAM's (YELLOW to CHALK by name, 7 of them in slightly other shades), so only its
  5 new ones are added: SHADOW, IVORY, CITRUS, WHITE, LAVENDER. They are Kirby colors 16-20 (the UFO's too), owned in the spray
  field's free bits 14-18 (the completion stays KatAM's), each in its Squeak Squad chest (treasures 83-87; the 13 others' chests
  hold items), shown on the Collection Room's second page as KatAM's spray can in their colors, with "SPRAY PAINT : NAME" in
  KatAM's letters after each language's own "SPRAY PAINT :" (the Japanese ROM has no English letters: Squeak Squad's name graphic
  there). A chest shows the closest KatAM can rising (it gives the spray, not KatAM's). The chest shuffle leaves these chests
  alone; the map viewer names them. `sprays.c`, `KatAMExpansionSprays.cs`.
  - Tested in the emulator, sprays owned by a RAM write (a forced state, not a playthrough): the page and its 5 cans and texts
    (USA, Europe in its 5 languages, Japan), choosing a color (the previews), the color worn in the game (USA, the owner's save:
    Lavender Kirby in Central Circle, also with the Master Sword), and a real chest opened (room 0x498, Lavender: its bit set,
    KatAM's Grape bit, whose can it shows, given back). A full randomization with the chest shuffle and with random chests keeps
    the 5 spray chests.
  - Not tested: the UFO ability in a new color, the screen that reads the colors' third table (the continue palettes,
    `gUnk_0834BB20`: its data checked only), multiplayer with a new color, the other 4 chests opened. Which chest holds which
    spray takes treasures 70-87 as Squeak Squad's sprays 1-18 in order: not checked against the DS's chests.
  - The names are English in every European language and Squeak Squad's in the Japanese ROM: Squeak Squad's European and
    Japanese names aren't in the US ROM used.
- **Map items for each area** (the owner, with the rooms): KatAM's map item, its sprite recolored to each NiDL and Squeak Squad
  area's theme, one per area, so the map viewer shows the area once its map is found.
- **The Collection Room scrolls left and right**: done for the spray paints (the owner, with the rooms: "the collection room
  should be scrollable left and right to accommodate for all these new items"). The Collection Room (from the main menu) has a
  second page: L or R turns the page, and so does Right on the last column or Left on the first (the cursor stays on its row,
  the pages loop). KatAM's page is as it was; the second holds Squeak Squad's 5 new sprays along its top row, its other cells
  empty for the rest. Tested in the emulator in all three versions (L/R, both edges, an empty cell's A, choosing, leaving with B).
  - Left: the new items for its other cells (the map items, Squeak Squad's ability scrolls, its secret map; the vitality halves
    count in KatAM's Vitality, its heart's cells).
- **ROM size** (the owner: "if needed, expand the ROM size to fit all this new content but only if needed"): 32 MB is the most
  the GBA maps (0x08000000-0x09FFFFFF), and the expansion is already there, so it can't grow; about 10 MB of it is free.

## 10. Screen shake as KatAM's
- The owner: the screen shakes far too much. Mr. Shine & Mr. Bright's and King Dedede's endless shakes are fixed; anything else
  that shakes too much: tell where.
