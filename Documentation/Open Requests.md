# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad: lag
- The owner: "Some squeak lairs lag a TON, we need to fix that." (Measured again, USA, every mini-boss room the expansion makes, 600
  frames each from Kirby's start at (40, 159) with Sword, B every 12 frames: 0x51E, 0x537 and 0x58A dropped 16 frames, the rest 0-3.
  Fixed since: Moving()'s 40 tests each frame for every Kirby without Animal or Ninja (the three now 4, the rest 0-2), Dark
  Nebula's and Daroach's sounds. Which lair still lags for the owner, and with what, wanted.) Then: "Room 0x48B has lag, try to keep
  the sprite count managable for the GBA in the Squeak Lair rooms." (0x48B is a lair only with Randomize Squeak Lairs. Every lair
  now has one Squeaker beside its Squeak, not two, and one food of its treasure pile; the same test, USA: 0x51E, 0x537 and 0x58A
  dropped 5 frames each, in one burst, 8 before. 0x48B itself not measured.)
- The owner: "Squeak lairs are very prone to lag and garbage tiles." (Garbage tiles not reproduced: six lairs' fights, every
  sprite's VRAM its own, the heap at most 338 of 512. Room 0x49C's garbled sprites were the gimmicks' VRAM, fixed.)

## 2. Squeak Squad: music and sounds
- The owner: "The music sounds kind of wrong? Off beat and off tone and mistimed. Squeak Squad actually uses a lot of the music from
  NiDL and KatAM. This needs special attention."
- The owner: "Some sound effects are not ported properly." / "Invest extra time making sure music and sounds are ported as expected.
  Same applies with abilities."

- The owner: "Room 0x4E4 hard game crash. Most likely had to do with the moving platforms." and "Room 0x4E5, crash sounds and a ton
  of lag but the game stays alive with crash sounds." (Not reproduced: USA with the NiDL ROM, Kirby in both rooms with their platforms
  running, no crash and no stray sounds. Which build, and what Kirby did there, wanted. 0x4E4's "I cannot go up" is fixed: its floor of blocks across the shaft is a thin floor now.)

## 3. Squeak Squad: bosses
- The owner: "Is the beam supposed to be this thin?" (Daroach's beam: drawn as dancho01.mca's beam composite, about 10 pixels high,
  its reach's box 9 high as the DS's; not yet compared with the DS's own frames.)
- The owner: "Mecha Kracko when I died on phase 2, it showed garbled tiles on phase 1." (Not reproduced: USA with the NiDL ROM, a
  death by a fall through the broken floor and one by a hit in its second form, the room and its first form came back whole. Which
  build, and how Kirby died, wanted.)
- The owner: "Some of the boss rooms have the camera locked in/zoomed in closer than they should, cutting away some details of the map."
  (Every Squeak boss's fight is held to KatAM's 240x160, the DS's arena being 256x192; widening the area lets the camera scroll, but
  the bosses place their moves from the camera, Dark Nebula's from its middle: which rooms cut what, wanted.)

## 4. Squeak Squad: rooms and objects
- The owner: "The squeaker with the chest in 0x4EF runs its animation and then just disappears for whatever reason." (Changed: a
  Squeaker with a chest keeps it, running off from Kirby, and drops it where it's hit; before, its way out was the spot it came from,
  so placed on its chest it went off with it at once. Not seen in the room: the Squeaker and its chest didn't come out on a plain
  visit, only in its chase, which wasn't played.)
- The owner: "Room 0x50E bugged sound effects and double pedestals." (The Gold Waddle Dee's chest is done: it comes where the enemy
  was beaten. The sounds and the pedestals not looked at yet.)
- The owner: "Down + B on stone doors if opened, send kirby jumping back."
- The owner: "Bomb blocks attacked from any angle can blow up even if you don't hit the bomb square." (On purpose so far: a chain
  whose bomb block can't be reached from outside, as room 0x4A0's in the middle of its 5 by 3 wall, has its outward blocks made bomb
  blocks too, as on the DS an attack from outside sets such a chain off. Only the bomb block itself would leave 0x4A0's chain
  unbreakable. The owner's call wanted.)
- The owner: "Some Squeak Lairs are not locking the camera. Check all of them." (Checked: with the NiDL ROM every lair (38, and 46 with
  Randomize Squeak Lairs) has its arena, the camera held to the DS hideout's screen, 272x224, scrolling the 32 pixels it's wider than
  the GBA's; without the NiDL ROM none has one, the arena being NiDL's object type. Which build, and which lair, wanted.)
- The owner: "When squeaks are matching a coordinate whether it is X or Y of Kirby's, they continue roaming all over the place, they
  should be intelligent enough to recognize that 'I'm on Kirby's coordinate, I should wait' this happens in room 0x496, the squeak
  presses a button but doesn't move forward as it would be the next logical step."

## 5. Squeak Squad: Kirby, abilities and pedestals
- The owner: "Scroll abilities don't seem to be upgrading, I got the Ice Scroll and the Ice ability is the same as ever." (Checked in
  the GBA emulator, room 0x4E2: its chest opened with Up gives the Ice Scroll, and Ice Kirby's breath then bends up with Up held
  (down with Down), as the DS's: that is all the DS's Ice Scroll changes, so without Up or Down it looks the same. Was Up or Down held?
  If another look was expected, which?)
- The owner: "When I die, lives are not being taken away from me." (Not reproduced: past the tutorial, with Skip the Tutorial or
  without, a death takes a life in KatAM's rooms, the Squeak Squad stages, lairs and boss rooms, by a hit or a fall, the HUD too; KatAM
  takes none in its tutorial (gAIKirbyState under 200). How the game was started, and where, wanted.)
- The owner: "When using the Hi-Jump pedestal, you get a very off animation." (After the swallow, KatAM's getting-an-ability poses
  (body animations 52-54) show NiDL's Hi-Jump hat and cape over KatAM's Normal body for about 25 frames; not changed yet.)
- The owner: "Broken color palettes with Ninja and inverted facing as well as being off the ground." (Facing and height done; the
  palettes looked right standing, walking, jumping and floating: in which move were they broken?)
- The owner: "Ghost pedestal doesn't give the ghost ability, is it even implemented?" (Not implemented: Squeak Squad's Ghost, and
  Triple Star, aren't in the ROM Expansion, so their pedestals give nothing.)
