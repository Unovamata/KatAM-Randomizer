# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad: lag
- The owner: "Some squeak lairs lag a TON, we need to fix that." (Measured again, USA, every mini-boss room the expansion makes, 600
  frames each from Kirby's start at (40, 159) with Sword, B every 12 frames: 0x51E, 0x537 and 0x58A dropped 16 frames, the rest 0-3.
  Fixed since: Moving()'s 40 tests each frame for every Kirby without Animal or Ninja (the three now 4, the rest 0-2), Dark
  Nebula's and Daroach's sounds. Which lair still lags for the owner, and with what, wanted.)
- The owner: "Squeak lairs are very prone to lag and garbage tiles." (Garbage tiles not reproduced: six lairs' fights, every
  sprite's VRAM its own, the heap at most 338 of 512. Room 0x49C's garbled sprites were the gimmicks' VRAM, fixed.)

## 2. Squeak Squad: music and sounds
- The owner: "The music sounds kind of wrong? Off beat and off tone and mistimed. Squeak Squad actually uses a lot of the music from
  NiDL and KatAM. This needs special attention."
- The owner: "Some sound effects are not ported properly." / "Invest extra time making sure music and sounds are ported as expected.
  Same applies with abilities."

## 3. Squeak Squad: bosses
- The owner: "Is the beam supposed to be this thin?" (Daroach's beam.)
- The owner: "Daroach's star rod's stars lag a bit whenever there's 4 Kirbys in the room."
- The owner: "Squeak Squad stars from bosses have a hitbox that the player can hit, these are supposed to be inhalable only."
- The owner: "Mecha Kracko when I died on phase 2, it showed garbled tiles on phase 1." (Not reproduced: USA with the NiDL ROM, a
  death by a fall through the broken floor and one by a hit in its second form, the room and its first form came back whole. Which
  build, and how Kirby died, wanted.)
- The owner: "Some of the boss rooms have the camera locked in/zoomed in closer than they should, cutting away some details of the map."
  (Every Squeak boss's fight is held to KatAM's 240x160, the DS's arena being 256x192; widening the area lets the camera scroll, but
  the bosses place their moves from the camera, Dark Nebula's from its middle: which rooms cut what, wanted.)
- The owner: "Fire Lion if its jump is interrupted, it will softlock itself until killed or the blocks above him are destroyed. He
  should jump as normal."

## 4. Squeak Squad: rooms and objects
- The owner: "The squeaker with the chest in 0x4EF runs its animation and then just disappears for whatever reason."
- The owner: "Room 0x50E bugged sound effects and double pedestals. The gold waddle dee when it dies, it spawns the chest not in the
  waddle dee position but in the default position which is not how the puzzle works, the waddle dee, when it dies, it spawns the
  chest where the waddle dee died."
- The owner: "Room 0x4D5, I went right through the metal blocks."
- The owner: "Room 0x4A0 do down and B on the star blocks, that we're you'll find the huge screen vibration bug. Go frame and frame
  and fix it so you can properly see the issue in action. This vibration issue has been plaguing both injections."
- The owner: "Down + B on stone doors if opened, send kirby jumping back."
- The owner: "Bomb blocks attacked from any angle can blow up even if you don't hit the bomb square."
- The owner: "Some Squeak Lairs are not locking the camera. Check all of them." (Checked: with the NiDL ROM every lair (38, and 46 with
  Randomize Squeak Lairs) has its arena, the camera held to the DS hideout's screen, 272x224, scrolling the 32 pixels it's wider than
  the GBA's; without the NiDL ROM none has one, the arena being NiDL's object type. Which build, and which lair, wanted.)
- The owner: "Room 0x494 enemies going through the ground. Nothing is ever allowed to go through the ground."
- The owner: "Kirby doesn't grab items when riding a warp star, this should be changed so Kirby is able to do so."
- The owner: "Squeaks should be able to take treasure chests and work similar to Golden Waddle Dees where if killed they would spawn
  the stolen chest at the location they are."
- The owner: "When squeaks are matching a coordinate whether it is X or Y of Kirby's, they continue roaming all over the place, they
  should be intelligent enough to recognize that 'I'm on Kirby's coordinate, I should wait' this happens in room 0x496, the squeak
  presses a button but doesn't move forward as it would be the next logical step."

## 5. Squeak Squad: Kirby, abilities and pedestals
- The owner: "When I die, lives are not being taken away from me." (Not reproduced: past the tutorial, with Skip the Tutorial or
  without, a death takes a life in KatAM's rooms, the Squeak Squad stages, lairs and boss rooms, by a hit or a fall, the HUD too; KatAM
  takes none in its tutorial (gAIKirbyState under 200). How the game was started, and where, wanted.)
- The owner: "On idle, Kirby is off the ground with Squeak Squad abilities. Most sprites have inverted facing."
- The owner: "Hammer pedestal in Squeak's gives light for whatever reason, even though Hammer is on the base game."
- The owner: "When using the Hi-Jump pedestal, you get a very off animation."
- The owner: "Broken color palettes with Ninja and inverted facing as well as being off the ground."
- The owner: "Ghost pedestal doesn't give the ghost ability, is it even implemented?"
