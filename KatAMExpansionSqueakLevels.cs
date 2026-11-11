using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSqueakLevels; Squeak Squad's 8 worlds as KatAM levels, laid out by the rulebook ("Documentation/Map and Door Design
     * Rules.md"), as PlanDoors() lays NiDL's out, on Squeak Squad's own stages (PlanSqueakLevels()). The DS's own doors (its doorways,
     * drawn in the rooms' tiles, between a stage's rooms: AddSqueakRooms()) stay as they are; what this adds are KatAM mirrors:
     * • a world's regular stages are a tree from its first (its way in): every other stage is entered through a branch door in a room
     *   of another (deeper ones likelier, a big stage branching more; in a room's middle or hidden away, half and half), two-way, to a
     *   mirror back in the first room of the stage it leads to that takes one (rules 3 and 5); where no stage in the tree has a room
     *   taking a door (Prism Plains 1's are small, 2 doors each), a stage's goal door leads on into the next instead, one-way;
     * • only the deepest stage's goal leads to the boss stage (SqueakBossStages); the other goals lead on, one-way, into the world's
     *   next stage by number, but 1 or 2 a world, at the ends of paths, into one of KatAM's goal games (GoalRooms: Central Circle
     *   after) (the owner's; the DS's goal doors, drawn in the tiles, lead there: no mirror);
     * • the boss stage is entered only through its goal door; nothing else goes in, no door goes out of a boss room (rule 8: its shard
     *   is the way out); a room of it Kirby reaches only by a cutscene on the DS (the final stage's, between its bosses) gets a mirror
     *   from the room before it, where he lands there, one-way;
     * • loops, few (rule 5): one-way shortcuts from the deepest stages back to a shallower one (a third as many as the world has
     *   stages), and a hidden two-way door between each world and the next, into its first stage (Squeak Squad's alone: never into
     *   NiDL's, the owner's);
     * • no new door in a mini-boss's room, nor a landing there; a room keeps 2 exits at most, 3 if it's big (counting the DS's), never 3
     *   doors in one view, doors kept apart, from landings and from warp stars and cannons (rule 2: SqueakSpot()).
     * • each world's switch room (KatAMExpansionSqueakHubs.cs) found by a Mirra hidden in its second half, one-way, its floor mirrors
     *   both ways into the stage before the boss's and another branch's (rule 5). */
    internal static partial class KatAMExpansion {
        // The stage with each world's boss (5-7, a copy of 5-6's two rooms, is left out of the port: SqueakTestStages);
        static readonly Dictionary<int, int> SqueakBossStages = new Dictionary<int, int> { [1] = 7, [2] = 7, [3] = 7, [4] = 7, [5] = 6, [6] = 7, [7] = 7, [8] = 4 };
        const int SqueakLayoutSeed = 0x4B5353, SqueakGoalFlags = 0x23, SqueakWorlds = 8;

        /* BuildSqueakHub(); The Squeak Squad hub (room `hub`, after the worlds' rooms), a copy of Central Circle (the owner: "The way in is
         * a copy of the main game's central circle"): Central Circle's own tiles, palettes and tilemap (KatAM's entries, found by their
         * room: NiDL's ROM draws its golden mirror into copies of them), its clouds' palettes tinted and a Squeak Squad far view behind
         * (SqueakHubLook), its collision without its doors (its hidden
         * ones, their blocks and records, NiDL's golden mirror: air); Rainbow Route's mirror (MainHubEntry, 19, 14, drawn in its tiles)
         * leads to Prism Plains 1's start, the way in, as the NiDL hub's golden one; a mirror (object 0x6F) on its floor at
         * SqueakHubDoor, two-way with one at the same spot added to Central Circle (the grey door: the owner, "We will later decide on
         * position and visuals"). Its world doors (one per world: Central Circle's doors at MainHubDoors, the world's number's, their
         * records and hub mirror objects, 0x8C, as Central Circle's, shown by the world's big switch, SqueakSlot()) lead to the world's
         * switch room (from `firstSwitchRoom`), onto its hub door. Central Circle's door to the ability room is there too, at its spot, to the
         * Squeak Squad ability room (`abilityRoom`, BuildSqueakAbilityRoom()), shown once the Secret Map is whole (SecretMapSlot; the owner:
         * "The Squeak Squad ability room should go in the Squeak Squad hub at the same position the ability room is"). Central Circle
         * (`cc`: as it is now, read) gets its door to the hub once the randomizer is done (squeakFinish). Returns an error, or null; */
        static readonly (int x, int y) SqueakHubDoor = (11, 18);
        /* SqueakHubLook; The Squeak Squad hub's look (the owner: "Change the background of the Squeak Squad hub along with its palette for
         * the clouds, show me options in a screenshot"): the far view behind it (map/d_viw_NN), and the hue its clouds take, how
         * saturated, how much (0: Central Circle's, 1: all that hue) and their brightness (a factor); the owner's pick of
         * SqueakHubLooks' mock-ups: D, Prism Plains' castle with Central Circle's own cloud colors; */
        sealed record SqueakHubDesign(string Name, string View, double Hue, double Saturation, double Amount, double Brightness);
        static readonly SqueakHubDesign[] SqueakHubLooks = {
            new SqueakHubDesign("A: Cushy Cloud's sky", "map/d_viw_09.nsbtx", 205, 0.35, 0.6, 1.0),
            new SqueakHubDesign("B: twilight over the clouds", "map/d_viw_17.nsbtx", 285, 0.4, 0.65, 0.95),
            new SqueakHubDesign("C: starry night", "map/d_viw_26.nsbtx", 255, 0.5, 0.7, 0.8),
            new SqueakHubDesign("D: Prism Plains' castle", "map/d_viw_00.nsbtx", 45, 0.25, 0, 1.0),
            new SqueakHubDesign("E: the galaxy", "map/d_viw_1e.nsbtx", 265, 0.4, 0.6, 0.9),
        };
        // (the owner picked D: "The Squeak Squad hub should have this background as well", with the screenshot of D);
        static readonly SqueakHubDesign SqueakHubLook = SqueakHubLooks[3];
        // SqueakCloudTint(); A cloud color of the hub, colorized: its hue and saturation moved toward the look's by its amount, its brightness scaled;
        static int SqueakCloudTint(int color, SqueakHubDesign look) {
            (double h, double s, double v) = ToHsv(color);
            double delta = ((look.Hue - h + 540) % 360) - 180, hue = s < 0.05 ? look.Hue : (h + delta * look.Amount + 360) % 360;
            return FromHsv(hue, s + (look.Saturation - s) * look.Amount, Math.Min(1, v * look.Brightness));
        }
        const int CentralCircle = 0x321;
        static string BuildSqueakHub(byte[] rom, IReadOnlyDictionary<string, int> symbols, bool nidlRooms, int roomProps, int newProps, int newInfos,
                                     int roomInfos, int hub, int firstSwitchRoom, int abilityRoom, (int x, int y, byte facing) abilityLanding,
                                     (int room, int x, int y) wayIn, int navigationRoutes, int noTypes,
                                     Func<string, int, int> add, Func<List<byte>, int, int> doorsList,
                                     Func<string, (int index, int width, int height)?> viewBackground, out KatAMRoomEdit cc,
                                     out byte[] hubCollision, out int hubWidth, ref int cursor) {
            hubCollision = null;
            hubWidth = 0;
            int Props(int field) => ReadShort(rom, roomProps + CentralCircle * 0x28 + field * 2);
            int Table(string table) => nidlRooms ? roomTables[table].at : symbols[table];
            int Current(string table, int index) => ReadInt(rom, Table(table) + index * 4) - 0x08000000;
            cc = LoadKatAMRoom(rom, newProps, Table, CentralCircle);
            // (KatAM's own entry of a table for Central Circle: the one naming its room at `field`);
            int Vanilla(string table, int field, int fallback) {
                for (int i = 0; i < 0x400; i++) {
                    int entry = ReadInt(rom, symbols[table] + i * 4);
                    if (entry >> 24 != 0x08) break;
                    if (ReadShort(rom, entry - 0x08000000 + field) == CentralCircle) return i;
                }
                return fallback;
            }
            int width = hubWidth = ReadShort(rom, Current("gForegroundTilemaps", Props(12))) / 2;

            // Central Circle's collision and special records, as they are now;
            byte[] collision = cc.Collision;
            int height = collision.Length / width;
            List<byte[]> specials = cc.Specials.Select(r => (byte[])r.Clone()).ToList();
            byte[] Ending() => new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF };
            byte Facing(int x, int roomWidth) => (byte)((x - 1) * 2 < roomWidth ? 1 : 2);
            if (collision[SqueakHubDoor.y * width + SqueakHubDoor.x] != KatAMAir || collision[(SqueakHubDoor.y + 1) * width + SqueakHubDoor.x] != KatAMSolid)
                return "Central Circle has no floor for the Squeak Squad door.";

            // The hub: Central Circle's cells, its doors air, and its two doors;
            hubCollision = collision.Select(v => v >= 0xFC ? KatAMAir : v).ToArray();
            Door wayInDoor = new Door(MainHubEntry.x, MainHubEntry.y, wayIn.room, wayIn.x, wayIn.y, 1);
            Door backDoor = new Door(SqueakHubDoor.x, SqueakHubDoor.y, CentralCircle, SqueakHubDoor.x + 1, SqueakHubDoor.y + 1, Facing(SqueakHubDoor.x + 1, width));
            List<byte> hubSpecials = new List<byte>();
            List<byte[]> templates = new List<byte[]> { MirrorTemplate(SqueakHubDoor.x, SqueakHubDoor.y, 0) };
            List<byte> types = new List<byte> { MirrorType }, nineROM = new List<byte>();
            foreach (Door door in new[] { wayInDoor, backDoor }) {
                hubCollision[door.Y * width + door.X] = 0xFF;
                hubSpecials.AddRange(DoorRecord(door, 0xFF));
                nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
            }
            // (the world doors: Central Circle's door's 3x3 block records, which draw it, its record leading to the world's switch room,
            // and its hub mirror object, 0x8C, on the cell above, the world's slot breaking them, as Central Circle's);
            squeakHubDoors = new List<Door>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                (int x, int y) spot = MainHubDoors[world - 1];
                (int x, int y) target = SwitchHubDoor(SqueakLayoutLevel(world));
                Door door = new Door(spot.x, spot.y, firstSwitchRoom + world - 1, target.x + 1, target.y + 1, 1, Slot: SqueakSlot(world), Kind: DoorKind.Hub);
                List<byte[]> blocks = specials.Where(r => r[0] == 1 && Math.Abs(r[2] - spot.x) <= 1 && r[3] >= spot.y - 2 && r[3] <= spot.y).ToList();
                if (blocks.Count != 9) return $"Central Circle's door at ({spot.x}, {spot.y}) isn't as it should be.";
                foreach (byte[] block in blocks) hubSpecials.AddRange(block);
                hubSpecials.AddRange(DoorRecord(door, 0xFF));
                templates.Add(HubMirrorTemplate(spot.x, spot.y - 1, templates.Count, door.Slot));
                if (!types.Contains(HubMirrorType)) types.Add(HubMirrorType);
                nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
                squeakHubDoors.Add(door);
            }
            /* (the ability room's door: Central Circle's own blocks drawing it, its record, to the Squeak Squad ability room's door (its
             * doorway, `abilityLanding`: the landing cell + 1, the facing), and its hub mirror object of SecretMapSlot; its 9ROM entry last,
             * as Central Circle's and the NiDL hub's: the game's door visits look for a door in its list, endlessly); */
            byte[] abilityTemplate = cc.Templates.FirstOrDefault(t => t[12] == HubMirrorType && t[4] == AbilityRoomSlot);
            if (abilityTemplate == null) return "Central Circle has no door to the ability room.";
            int abilityX = ReadShort(abilityTemplate, 6) / 16, abilityY = ReadShort(abilityTemplate, 8) / 16 + 1;
            byte[] abilityRecord = specials.FirstOrDefault(r => r[0] == 3 && r.Length >= 14 && r[2] == abilityX && r[3] == abilityY);
            List<byte[]> abilityBlocks = specials.Where(r => r[0] == 1 && Math.Abs(r[2] - abilityX) <= 1 && r[3] >= abilityY - 2 && r[3] <= abilityY).ToList();
            if (abilityRecord == null || ReadShort(abilityRecord, 8) != KatAMAbilityRoom || abilityBlocks.Count != 9)
                return "Central Circle's door to the ability room isn't as it should be.";
            Door abilityDoor = new Door(abilityX, abilityY, abilityRoom, abilityLanding.x, abilityLanding.y, abilityLanding.facing, Slot: SecretMapSlot,
                                        Kind: DoorKind.Hub);
            foreach (byte[] block in abilityBlocks) hubSpecials.AddRange(block);
            hubSpecials.AddRange(DoorRecord(abilityDoor, 0xFF));
            byte[] abilityMirror = (byte[])abilityTemplate.Clone();
            abilityMirror[3] = (byte)templates.Count;
            abilityMirror[4] = SecretMapSlot;
            templates.Add(abilityMirror);
            nineROM.AddRange(NineROMEntry(abilityDoor.Room, abilityDoor.DestX, abilityDoor.DestY));
            squeakAbilityDoor = abilityDoor;
            hubSpecials.AddRange(Ending());
            int collisionAt = AllocateCompressed(rom, CompressRL(hubCollision), ref cursor), specialsAt = AllocateCompressed(rom, hubSpecials.ToArray(), ref cursor);
            int hubSolidity = Allocate(ref cursor, 8, 4);
            WriteInts(rom, hubSolidity, collisionAt + 0x08000000, specialsAt + 0x08000000);
            int objects = WriteObjectList(rom, templates, types, noTypes, ref cursor);
            int navigation = BuildNavigation(rom, width, height, false, hubCollision, navigationRoutes, new List<(int, int, int, int)>(), ref cursor);
            int doors = doorsList(nineROM, 0);

            /* (its look, SqueakHubLook: Squeak Squad's far view behind it, BackgroundRates() as the NiDL hub's (KatAM's camera: the range
             * the room's height - 168), and Central Circle's clouds, its palettes 4 and 5 (scenery: its cycled colors are in palettes 0, 2
             * and 3), tinted to it, SqueakCloudTint(), in a palette entry of its own: Central Circle's, its cycles kept); */
            int paletteEntry = ReadInt(rom, symbols["gForegroundPalettes"] + Vanilla("gForegroundPalettes", 10, Props(11)) * 4) - 0x08000000;
            int sourceColors = ReadInt(rom, paletteEntry) - 0x08000000, colorsAt = Allocate(ref cursor, 0xC0, 4), paletteAt = Allocate(ref cursor, 12, 4);
            for (int i = 0; i < 96; i++) {
                int color = ReadShort(rom, sourceColors + i * 2) & 0x7FFF;
                WriteShort(rom, colorsAt + i * 2, i >= 4 * 16 && i % 16 != 0 ? SqueakCloudTint(color, SqueakHubLook) : color);
            }
            Array.Copy(rom, paletteEntry, rom, paletteAt, 12);
            WriteInt(rom, paletteAt, colorsAt + 0x08000000);
            WriteShort(rom, paletteAt + 10, hub);
            if (viewBackground(SqueakHubLook.View) is not { } farView)
                return $"Squeak Squad has no far view {SqueakHubLook.View} for its hub.";
            (int rateX, int rateY) = BackgroundRates(farView.width, farView.height, width * 16, height * 16 + 40);

            // (its props: Central Circle's, KatAM's own graphics, its palettes and far view, the new collision, objects, routes and doors;
            // room 0x397's info, no routes);
            Array.Copy(rom, newProps + CentralCircle * 0x28, rom, newProps + hub * 0x28, 0x28);
            WriteShorts(rom, newProps + hub * 0x28 + 7 * 2, rateX, rateY);
            WriteShort(rom, newProps + hub * 0x28 + 10 * 2, Vanilla("gForegroundInfo", 14, Props(10)));
            WriteShort(rom, newProps + hub * 0x28 + 11 * 2, add("gForegroundPalettes", paletteAt));
            WriteShort(rom, newProps + hub * 0x28 + 12 * 2, Vanilla("gForegroundTilemaps", 0x1E, Props(12)));
            WriteShort(rom, newProps + hub * 0x28 + 15 * 2, farView.index);
            WriteShort(rom, newProps + hub * 0x28 + 13 * 2, add("gSolidityMaps", hubSolidity));
            WriteShort(rom, newProps + hub * 0x28 + 16 * 2, add("gLevelObjLists", objects));
            WriteShort(rom, newProps + hub * 0x28 + 17 * 2, add("gUnk_08D63C28", navigation));
            WriteShort(rom, newProps + hub * 0x28 + 18 * 2, add("gUnk_08D640A4", doors));
            int roomInfo = Allocate(ref cursor, 0x4C, 4);
            Array.Copy(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000, rom, roomInfo, 0x4C);
            WriteShort(rom, roomInfo + 0x44, hub);
            WriteInt(rom, newInfos + hub * 4, roomInfo + 0x08000000);

            return null;
        }
        // (the hub's world doors, for the tests' door lists; its door to the ability room);
        static List<Door> squeakHubDoors = new List<Door>();
        static Door squeakAbilityDoor;

        /* BuildSqueakAbilityRoom(); The Squeak Squad ability room (room `room`, after the switch rooms; the owner: "just import Squeak
         * Squad's ability room to KatAM along with its pedestals"): Squeak Squad's own, the one room of its stage 9-1 (map/a9s1s1, the
         * Arena's: a hall of shelves, its copy abilities each on a pedestal), converted as the worlds' rooms are (its tiles, far view,
         * collision, tile animations: AddSqueakRooms()); its door (the doorway at the hall's right end, SqueakAbilityRoomDoor()) leads back
         * to the Squeak Squad hub's door to it (Central Circle's spot: where KatAM's ability room's door leads Kirby there), the hub's door
         * landing Kirby on it; its pedestals (its .doe's items, kinds 0x17 on, Squeak Squad's ability the kind less 0x16) squeak/gimmicks.c's
         * (object 0x8D, its template's unk11 SqueakPedestalObject: unk12 the DS's ability, its look; unk14 KatAM's, SqueakPedestalAbility();
         * unk16 1 if its icon has pieces in the DS's other palettes), each on the floor under its spot, giving its ability to a Kirby with
         * none touching it, as KatAM's ability statues; its music KatAM's ability room's (the DS's room sets none); reached from the hub
         * once the Secret Map is whole (BuildSqueakHub(), SecretMapSlot). Returns an error, or null; */
        const int KatAMAbilityRoom = 0x3CA;
        // SecretMapSlot; The world slot the Squeak Squad hub's door to its ability room waits for: KatAM's free slot 0x1C, set while the
        // Secret Map is whole ("ROM Expansion/rooms.c"'s BigGetStateSlot, squeak.h's SECRET_MAP_SLOT);
        const byte SecretMapSlot = 0x1C;
        // (the room: Squeak Squad's area 9, stage 1; its pedestals' object, squeak.h's SQUEAK_PEDESTAL_OBJECT, its stand's foot 4 pixels
        // under the DS's spot, the frames' bottom);
        const int SqueakAbilityArea = 9, SqueakAbilityStage = 1, PedestalFoot = 4;
        const byte SqueakPedestalObject = 0x8F;

        // SqueakAbilityRoomFiles(); Squeak Squad's ability room's files (null: not in the ROM);
        static SqueakRoomFiles SqueakAbilityRoomFiles(byte[] squeak) => SqueakStageRooms(squeak, SqueakAbilityArea, SqueakAbilityStage).FirstOrDefault();
        // SqueakAbilityRoomDoor(); Its door: its doorway, the one that isn't where Kirby starts (null: none);
        static SqueakDoor SqueakAbilityRoomDoor(SqueakMap map) => map.Doors().FirstOrDefault(d => d.Kind != SqueakStartKind && d.X < map.Width && d.Y < map.Height);
        /* SqueakPedestalAbility(); KatAM's ability a pedestal gives: SqueakKatAMAbilities' (KatAM's Hi-Jump the NiDL one), and 17, Bun's,
         * KatAM's Throw (squeak.c's SQUEAK_BUN); Squeak Squad's own (Ninja, Animal, Bubble, Metal, Ghost, Triple Star, 12, 18) none; */
        static byte SqueakPedestalAbility(int ability) => ability == 17 ? (byte)10 : SqueakKatAMAbilities.GetValueOrDefault(ability);

        static string BuildSqueakAbilityRoom(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, Func<string, int> table, int newProps,
                                             int newInfos, int room, int hub, int navigationRoutes, int noTypes, int paletteCycles,
                                             Func<string, int, int> add, Func<List<byte>, int, int> doorsList,
                                             Func<string, (int index, int width, int height)?> viewBackground, ref int cursor) {
            SqueakRoomFiles f = SqueakAbilityRoomFiles(squeak);
            if (f == null) return "Squeak Squad's ability room wasn't found.";
            SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, f.Map)));
            SqueakTileset front = f.Front == null || SqueakFileAnyCase(squeak, f.Front) is not byte[] frontFile ? null : new SqueakTileset(frontFile);
            SqueakTileset main = f.Main == null || SqueakFileAnyCase(squeak, f.Main) is not byte[] mainFile ? null : new SqueakTileset(mainFile);
            int width = map.Width, height = map.Height;
            (byte[] collision, List<(int x, int y)> blocks, _, _, HashSet<(int x, int y)> chains, HashSet<(int x, int y)> burnt) = SqueakRoomCollision(map);
            if (blocks.Count > 0 || chains.Count > 0) return "Squeak Squad's ability room has blocks it shouldn't.";
            if (SqueakAbilityRoomDoor(map) is not SqueakDoor door) return "Squeak Squad's ability room has no door.";

            // Its door: back to the hub's door to it, where KatAM's ability room's door leads Kirby in Central Circle (the hub its copy);
            KatAMRoomEdit katam = LoadKatAMRoom(rom, newProps, table, KatAMAbilityRoom);
            byte[] back = katam.Specials.FirstOrDefault(r => r[0] == 3 && r.Length >= 14 && ReadShort(r, 8) == CentralCircle);
            if (back == null) return "KatAM's ability room has no door back to Central Circle.";
            Door way = new Door(door.X, door.Y, hub, back[10], back[11], back[12]);
            collision[door.Y * width + door.X] = 0xFF;
            List<byte> specials = new List<byte>(DoorRecord(way, 0xFF));
            specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
            int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref cursor), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref cursor);
            int solidity = Allocate(ref cursor, 8, 4);
            WriteInts(rom, solidity, collisionAt + 0x08000000, specialsAt + 0x08000000);

            // Its look: its tiles and tile animations, its far view (shared), panning as the worlds' rooms' (AddSqueakRooms());
            SqueakForeground foreground = ConvertSqueakForeground(map, front, main, ForegroundTiles, blocks, SqueakSigns(map), chains.Union(burnt).ToHashSet());
            (int info, int palette, int tilemapEntry) = WriteSqueakForeground(rom, foreground, room, paletteCycles, ref cursor);
            (int index, int width, int height)? view = f.View == null ? null : viewBackground(f.View);
            int background = view?.index ?? ReadShort(rom, newProps + 0x3C9 * 0x28 + 15 * 2);
            (int rateX, int rateY) = view is (int, int, int) v ? BackgroundRates(v.width, v.height, width * 16, height * 16 + 32, 0) : (0, 0);
            int backgroundDown = -((8 * rateY + 0xFF) >> 8);

            // Its pedestals: its items' abilities, each on the floor under its spot (the first cell down that isn't air), its stand's foot on it;
            byte[] objectsFile = f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects);
            if (objectsFile == null || objectsFile.Length < 8) return "Squeak Squad's ability room has no objects.";
            SqueakCharacter look = ReadSqueakCharacter(squeak, "copy", "item");
            List<byte[]> templates = new List<byte[]>();
            bool Floor(int cx, int cy) => collision[cy * width + cx] != KatAMAir;
            int items = ReadShort(objectsFile, 6);
            for (int i = 0, count = items + 2 <= objectsFile.Length ? ReadShort(objectsFile, items) : 0; i < count; i++) {
                int at = items + 2 + i * 16, ability = ReadShort(objectsFile, at + 2) - SqueakAbilityKinds;
                int x = ReadShort(objectsFile, at + 4), y = ReadShort(objectsFile, at + 6), cy = y / 16;
                if (ability < 1 || ability > SqueakPedestalAnims.Length || x / 16 >= width) continue;
                while (cy < height && !Floor(x / 16, cy)) cy++;
                if (cy >= height) return $"Squeak Squad's ability room's pedestal at ({x}, {y}) has no floor under it.";
                byte[] pedestal = ObjectTemplate(x, cy * 16 - PedestalFoot, templates.Count, HammerPegType);
                pedestal[0x11] = SqueakPedestalObject;
                pedestal[0x12] = (byte)ability;
                pedestal[0x14] = SqueakPedestalAbility(ability);
                // (its icon's pieces in the DS's other palettes, its icon's sprite: its cell, the pedestal animation's);
                int cell = look.Anims[SqueakPedestalAnims[ability - 1]].Steps[0].Cell;
                pedestal[0x16] = (byte)(look.Cells[cell].Pieces.Any(piece => piece.Attr2 >> 12 != 0) ? 1 : 0);
                templates.Add(pedestal);
            }
            if (templates.Count == 0) return "Squeak Squad's ability room has no pedestals.";
            if (templates.Count > 31) return "Squeak Squad's ability room has more pedestals than a KatAM room holds.";
            int objects = WriteObjectList(rom, templates, new List<byte> { HammerPegType }, noTypes, ref cursor);
            int navigation = BuildNavigation(rom, width, height, false, collision, navigationRoutes, new List<(int, int, int, int)>(), ref cursor);
            int doorsIndex = add("gUnk_08D640A4", doorsList(NineROMEntry(hub, way.DestX, way.DestY).ToList(), 0));
            if (doorsIndex < FirstDoorsList || doorsIndex >= FirstDoorsList + VisitedLists) return "The Squeak Squad ability room's doors list is out of rooms.c's visited doors.";

            // Its info KatAM's ability room's, its room; its camera from the room's top as the worlds' rooms'; its props;
            int roomInfo = Allocate(ref cursor, 0x4C, 4);
            Array.Copy(rom, ReadInt(rom, newInfos + KatAMAbilityRoom * 4) - 0x08000000, rom, roomInfo, 0x4C);
            WriteShort(rom, roomInfo + 0x44, room);
            WriteInt(rom, newInfos + room * 4, roomInfo + 0x08000000);
            rom[CodeAddress + version.CodeSymbols["gNidlRoomTops"] + room] = SqueakCameraTop;
            WriteShorts(rom, newProps + room * 0x28, ReadShort(rom, newProps + KatAMAbilityRoom * 0x28), 0, 0, 0x100, 0x100, 0, backgroundDown, rateX, rateY,
                        RoomFlags, add("gForegroundInfo", info), add("gForegroundPalettes", palette), add("gForegroundTilemaps", tilemapEntry),
                        add("gSolidityMaps", solidity), 0xFFFF, background, add("gLevelObjLists", objects), add("gUnk_08D63C28", navigation), doorsIndex, 0);
            return null;
        }

        /* The finale (the owner: "move the Squeak Squad final boss sequence in between of Nightmare and KatAM's final boss ... Entire NiDL
         * final boss sequence that's already ported -> Entire final boss sequence from Squeak Squad recreated and ported 1:1 -> KatAM's
         * final boss. The additional sequences only appear when the ROM expansion ROMs are added"): Squeak Squad's final stage
         * (SqueakFinalStage, 8-4: Dark Daroach, the dark star's corridor and shaft, Dark Nebula's coming and fight) is no world's boss
         * stage; it's entered through the Fountain of Dreams hub's Amazing Mirror, after NiDL's finale (Nightmare's ending leads into that
         * hub, as without Squeak Squad) or, without NiDL's finale, through Central Circle's Dimension Mirror (SqueakFinale()); Dark Nebula
         * leaves no shard (his template's SqueakBossShard SqueakFinaleShard: "ROM Expansion/squeak.c" dances every Kirby out instead), his
         * way out the Dark Nebula hub (BuildSqueakFinaleHub()), its Amazing Mirror to where the mirror into the stage led (KatAM's final
         * boss); */
        static readonly (int area, int stage) SqueakFinalStage = (8, 4);
        const int SqueakFinaleShard = 0xFF;
        static (int room, int x, int y)? squeakFinaleEntry;
        /* SqueakLairPlan; A Squeak Lair (PlanSqueakLevels()): its hideout room, the Squeak fought there (its character), the DS door
         * Kirby takes into it (its room, its place in the room's list, its cell), where he lands in it and where its way out leads
         * (the room, the cell + 1); its nest the DS's own on that door (SqueakNests()); */
        internal sealed record SqueakLairPlan(int Room, int Character, int FromRoom, int FromDoor, int FromX, int FromY, (int room, int x, int y) Landing,
                                              (int room, int x, int y) Exit);
        static readonly List<SqueakLairPlan> squeakLairs = new List<SqueakLairPlan>();
        public static IReadOnlyList<SqueakLairPlan> SqueakLairs => squeakLairs;
        /* Randomize Squeak Lairs (the owner: "we can store the Unused Hideouts and actually randomize these per run, as in these hideouts
         * can be anywhere since these are area transitioners. So, it would be a good idea to randomly remove or add these connections to
         * these rooms in a per seed basis ... Of course, we have to have an unrandomized map as well for times where these won't be
         * randomized, for this, the current layout is perfect"): the seed the lairs are picked from (AddEnemies()), null for the DS's
         * thieves' hideouts. Each of a stage's hideouts is a lair one time in LairShare, its door in picked among the stage's doors that
         * can be one; the Squeakers' hideout's fight is one of LairFighters (Spinni, Doc, Storo) leading them; the rest of the map is
         * the same either way (its own generator); */
        static int? squeakLairSeed;
        const int LairShare = 3;
        static readonly int[] LairFighters = { 29, 31, 33 };
        // (a chase record's flags: 8, the stage's thief; a DS door's: 4, a hidden way (the lair doors); the Squeakers' character);
        const int SqueakThief = 8, SqueakHiddenDoor = 4, SqueakerCharacter = 27;
        static int squeakFinaleBoss = -1;

        /* SqueakFinale(); The finale's ways in and out of Squeak Squad's final stage (see SqueakFinalStage; the rulebook's finale chain):
         * with NiDL's finale in the ROM, the Fountain of Dreams hub's Amazing Mirror (where Nightmare's way on leads, as without Squeak
         * Squad: its shard object of subtype 8, +0x1E the room, +0x1A, +0x1C the landing + 1; `fountainHub` its room) into the stage's
         * start, its end (`exit`) where that mirror led (KatAM's final boss); without it, Central Circle's Dimension Mirror the same way.
         * Dark Nebula's way out is the Dark Nebula hub (BuildSqueakFinaleHub()), whose Amazing Mirror leads to `exit`. `table`: a room
         * table's base (NiDL's copies with its rooms). Returns an error, or null; */
        static string SqueakFinale(byte[] rom, IReadOnlyDictionary<string, int> symbols, Func<string, int> table, out (int room, int x, int y) exit,
                                   out int fountainHub) {
            exit = default;
            fountainHub = -1;
            if (squeakFinaleEntry is not (int, int, int) entry) return null;
            var nightmare = bossExitsOf.FirstOrDefault(e => (e.Key.Level, e.Key.Stage, e.Key.Index) == NightmareBossRoom);
            int mirrorRoom = EntryRoom;
            if (nightmare.Key != null && endingFlight >= 0) mirrorRoom = fountainHub = nightmare.Value.room;
            int index = ReadShort(rom, symbols["gRoomProps"] + mirrorRoom * 0x28 + 16 * 2);
            int list = ReadInt(rom, table("gLevelObjLists") + index * 4) - 0x08000000;
            for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36)
                if (rom[at + 12] == ShardType && rom[at + 14] == 8) {
                    exit = (ReadShort(rom, at + 0x1E), ReadShort(rom, at + 0x1A), ReadShort(rom, at + 0x1C));
                    WriteShorts(rom, at + 0x1A, entry.x, entry.y, entry.room);
                    return null;
                }
            return fountainHub >= 0 ? "The Fountain of Dreams hub's Amazing Mirror wasn't found." : "Central Circle's Dimension Mirror wasn't found.";
        }

        /* SqueakShardExits(); Where each world's boss's shard takes Kirby (the rulebook's rule 8: a shard only ever sends Kirby to a central
         * hub, onto the level's door there): gNidlShardExits' entries after the NiDL bosses' ("ROM Expansion/nidl_boss.c", NidlShardExit()
         * after the shard's cutscene, HookShardExit()), the Squeak Squad hub, at the world's door's spot (Central Circle's door of the area
         * of its number, MainHubDoors, as the NiDL hub's levels': the world doors themselves come with the switch rooms), each Kirby on his
         * own cell of its ground (HubGroundBeside()); */
        const int ShardExitCount = 24;
        static string SqueakShardExits(byte[] rom, KatAMExpansionCode.Version version, IReadOnlyDictionary<string, int> symbols, int hub,
                                       Dictionary<int, int> bossRooms, byte[] hubCollision, int hubWidth, (int room, int x, int y) finaleExit) {
            if (HookShardExit(rom, version) is string error) return error;
            int exits = CodeAddress + version.CodeSymbols["gNidlShardExits"], n = 0;
            while (n < ShardExitCount && ReadShort(rom, exits + n * 8) != 0) n++;
            foreach ((int room, int world) in bossRooms.OrderBy(b => b.Key)) {
                if (n >= ShardExitCount) return "Too many boss rooms for the shards' exits.";
                // (the finale's: the Dark Nebula hub, BuildSqueakFinaleHub(); no ground either side);
                if (world == SqueakFinaleShard) {
                    WriteShorts(rom, exits + n * 8, room, finaleExit.room);
                    rom[exits + n * 8 + 4] = (byte)finaleExit.x;
                    rom[exits + n * 8 + 5] = (byte)finaleExit.y;
                    rom[exits + n * 8 + 6] = rom[exits + n * 8 + 7] = 0;
                    n++;
                    continue;
                }
                (int x, int y) spot = MainHubDoors[(world - 1) % MainHubDoors.Length];
                WriteShorts(rom, exits + n * 8, room, hub);
                rom[exits + n * 8 + 4] = (byte)(spot.x + 1);
                rom[exits + n * 8 + 5] = (byte)(spot.y + 1);
                (rom[exits + n * 8 + 6], rom[exits + n * 8 + 7]) = HubGroundBeside(rom, hubCollision, hubWidth, symbols["gCollisionAttributes"], spot.x, spot.y);
                n++;
            }
            // (the map tracker's count of them, "SX": AddRooms() writes it with the NiDL rooms);
            System.Text.Encoding.ASCII.GetBytes("SX").CopyTo(rom, MapShardExits);
            WriteShort(rom, MapShardExits + 2, n);
            WriteInt(rom, MapShardExits + 4, exits + 0x08000000);
            return null;
        }

        // SqueakPlanRoom; What the plan knows of a Squeak Squad room: its place, size, KatAM collision, the DS's doors, its fights, its warp
        // stars and cannons (cells);
        internal sealed class SqueakPlanRoom {
            public int Index, Room, Area, Stage, Number, Width, Height;
            public byte[] Collision;
            public List<SqueakDoor> Doors;
            public bool Midboss, Boss;
            // (its Squeaks the DS brings out in a chase or a hideout's fight (records with flag 4): the character, the spot, the flags);
            public List<(int character, int x, int y, int flags)> Squeaks = new List<(int, int, int, int)>();
            public List<(int x, int y)> Transports = new List<(int, int)>();
            // (its nest blocks' key and whether its front tileset draws a nest: SqueakNestKey(), SqueakNestDrawn());
            public string NestKey;
            public bool NestDrawn;
            // (the stage's rooms its warp stars and fuse cannons take Kirby to, numbered from 1);
            public List<int> Rides = new List<int>();
            // (where each of them stands, its cell, and lands Kirby: the room, the cell; for the way-out pass);
            public List<(int x, int y, int room, int toX, int toY)> RideTo = new List<(int, int, int, int, int)>();
            public byte At(int x, int y) => x < 0 || y < 0 || x >= Width || y >= Height ? KatAMSolid : Collision[y * Width + x];
        }

        // (a cell Kirby goes through: air, water, ladders, doors; star blocks too, once broken, if blocks);
        static bool SqueakOpen(byte v, bool blocks = false) => v is KatAMAir or KatAMWater or KatAMLadder or KatAMLadderTop or >= 0xFC || blocks && v == KatAMStarBlock;
        // (a cell Kirby stands on: solid, slopes, thin floors, a ladder's top, blocks);
        static bool SqueakFloor(byte v) => v is (>= 0x05 and <= 0x1B) or KatAMLadderTop or (>= KatAMStarBlock and <= KatAMMetalBlock);
        // SqueakGoal(); Whether a DS door is its stage's goal; SqueakArrival(), where its doors land Kirby (no room, not a goal);
        /* (a goal: flags 0x23, with bit 7 too (4-4-9's: as the doors' 0x90), whatever its room (2-8-7's names room 13 of a stage of 7, the
         * only one of the 48 that names one: the DS's goal all the same); a room's start: no room, not a goal); */
        static bool SqueakGoal(SqueakDoor d) => (d.Flags & 0x7F) == SqueakGoalFlags;
        static bool SqueakArrival(SqueakDoor d) => d.Room == SqueakNoRoom && !SqueakGoal(d);

        /* PlanSqueakLevels(); The mirrors to add to each room (by its index), from SqueakLayoutSeed; each world's way in (its first
         * stage's start: the room, cell + 1), the boss rooms (their room ids, by world) and the switch rooms' floor mirrors (by world: the
         * switch rooms are rooms `firstSwitchRoom` on, KatAMExpansionSqueakHubs.cs); */
        static Dictionary<int, List<Door>> PlanSqueakLevels(List<SqueakPlanRoom> rooms, int firstSwitchRoom, out (int room, int x, int y)[] worldStarts,
                                                           out Dictionary<int, int> bossRooms, out Dictionary<int, List<Door>> switchDoors) {
            Random random = new Random(SqueakLayoutSeed);
            Dictionary<int, List<Door>> doors = rooms.ToDictionary(r => r.Index, r => new List<Door>());
            var byStage = rooms.GroupBy(r => (r.Area, r.Stage)).ToDictionary(g => g.Key, g => g.OrderBy(r => r.Number).ToList());
            Dictionary<int, SqueakPlanRoom> byRoom = rooms.ToDictionary(r => r.Room);
            bool IsBossStage((int area, int stage) s) => SqueakBossStages.TryGetValue(s.area, out int boss) && boss == s.stage;
            HashSet<int> goalRooms = rooms.Where(r => r.Doors.Any(SqueakGoal)).Select(r => r.Room).ToHashSet();
            List<(int room, int x, int y)> landings = new List<(int, int, int)>();
            byte Facing(SqueakPlanRoom to, int x) => (byte)((x - 1) * 2 < to.Width ? 1 : 2);

            // Where Kirby stands at a cell: down to the floor;
            (int x, int y) Drop(SqueakPlanRoom r, int x, int y) {
                while (y + 1 < r.Height && SqueakOpen(r.At(x, y + 1))) y++;
                return (x, y);
            }
            // A stage's start: its first room's first arrival (the DS's kind 6, or another entrance), on the floor;
            (int room, int x, int y) StageStart((int area, int stage) s) {
                SqueakPlanRoom first = byStage[s][0];
                SqueakDoor arrival = first.Doors.FirstOrDefault(SqueakArrival);
                (int x, int y) at = arrival != null ? Drop(first, arrival.X, arrival.Y) : SqueakSpot(first, false, false) ?? (first.Width / 2, first.Height / 2);
                return (first.Room, at.x + 1, at.y + 1);
            }

            // The doors a room has (the DS's ways on, goals, and the new mirrors), and its cells taken (doors, arrivals, landings, rides);
            int Exits(SqueakPlanRoom r) => r.Doors.Count(d => d.Room != SqueakNoRoom || SqueakGoal(d)) + doors[r.Index].Count;
            int MaxExits(SqueakPlanRoom r) => r.Width * r.Height <= 2 * ViewWidth * (ViewHeight + 1) ? 2 : 3;
            IEnumerable<(int x, int y)> Drawn(SqueakPlanRoom r) => r.Doors.Where(d => d.Room != SqueakNoRoom || SqueakGoal(d)).Select(d => (d.X, d.Y))
                                                                           .Concat(doors[r.Index].Select(d => (d.X, d.Y)));
            IEnumerable<(int x, int y)> Taken(SqueakPlanRoom r) => Drawn(r).Concat(r.Doors.Where(SqueakArrival).Select(d => (d.X, d.Y)))
                                                                          .Concat(landings.Where(l => l.room == r.Room).Select(l => (l.x - 1, l.y - 1)));
            bool ViewFits(SqueakPlanRoom r, int x, int y) {
                List<(int x, int y)> near = Drawn(r).Where(d => Math.Abs(d.x - x) < ViewWidth && Math.Abs(d.y - y) < ViewHeight).ToList();
                for (int i = 0; i < near.Count; i++)
                    for (int j = i + 1; j < near.Count; j++) {
                        int[] xs = { x, near[i].x, near[j].x }, ys = { y, near[i].y, near[j].y };
                        if (xs.Max() - xs.Min() < ViewWidth && ys.Max() - ys.Min() < ViewHeight) return false;
                    }
                return true;
            }
            /* The rooms Kirby reaches in each stage from its start, through the DS's doors (they lead on, one-way: a room's arrival is no
             * door back) and the ones added so far; a room nothing leads into on the DS (Prism Plains 1's fifth) isn't one, until a door
             * throws him in (see the end); */
            HashSet<int> reachable = new HashSet<int>();
            /* Through(); A step Kirby takes from a cell to the next: into one he goes through (blocks broken, thin floors crossed, slopes:
             * the room's own walls only), but never down through a thin floor (rule 9: without Down and B); */
            static bool Through(SqueakPlanRoom r, int y, int nx, int ny) {
                byte v = r.At(nx, ny);
                bool through = SqueakOpen(v, true) || v is KatAMThin or KatAMFirmThin or KatAMSpikes or (>= 0x10 and <= 0x13) or 0x18 or 0x19
                                                       or (>= 0x01 and <= 0x0C) or (>= 0x34 and <= 0x43) or KatAMMetalBlock or (>= 0xF1 and <= 0xF3);
                return through && !(ny == y + 1 && v is KatAMThin or KatAMFirmThin or (>= 0x10 and <= 0x1B));
            }
            /* Leads(); Whether a DS door of a room is one Kirby gets to from where he comes in: its arrivals, the doors the stage's others
             * land him on, the landings of the doors added so far, joined by the cells he goes through (Through(); 7-3-1's hidden way
             * into 7-3-5, walled in by clouds, isn't: 7-3-5 gets a throw, below; 7-3-2's way on, under a thin floor, isn't either); */
            bool Leads(SqueakPlanRoom r, SqueakDoor door) {
                List<SqueakPlanRoom> stage = byStage[(r.Area, r.Stage)];
                List<(int x, int y)> starts = r.Doors.Where(d => SqueakArrival(d) || stage.Any(t => t.Doors.Any(o => o.Room + 1 == r.Number && o.Door == r.Doors.IndexOf(d))))
                                                     .Select(d => (d.X, d.Y))
                                                     .Concat(doors.Values.SelectMany(list => list).Where(d => d.Room == r.Room).Select(d => (d.DestX - 1, d.DestY - 1))).ToList();
                bool[] seen = new bool[r.Width * r.Height];
                Queue<(int x, int y)> open = new Queue<(int x, int y)>();
                foreach ((int x, int y) in starts)
                    if (x >= 0 && y >= 0 && x < r.Width && y < r.Height && !seen[y * r.Width + x]) { seen[y * r.Width + x] = true; open.Enqueue((x, y)); }
                while (open.Count > 0) {
                    (int x, int y) = open.Dequeue();
                    if (x == door.X && y == door.Y) return true;
                    foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                        if (nx >= 0 && ny >= 0 && nx < r.Width && ny < r.Height && !seen[ny * r.Width + nx]
                            && (Through(r, y, nx, ny) || nx == door.X && ny == door.Y)) {
                            seen[ny * r.Width + nx] = true;
                            open.Enqueue((nx, ny));
                        }
                }
                return starts.Count == 0;
            }
            void Reach(int room) {
                Queue<int> queue = new Queue<int>();
                if (reachable.Add(room)) queue.Enqueue(room);
                while (queue.Count > 0) {
                    SqueakPlanRoom r = byRoom[queue.Dequeue()];
                    // (a lair's DS doors aren't doors: its way out is its mirror, below);
                    bool lair = squeakLairs.Any(l => l.Room == r.Room);
                    IEnumerable<int> next = r.Doors.Where(d => d.Room != SqueakNoRoom && !lair && Leads(r, d)).Select(d => byStage[(r.Area, r.Stage)].FirstOrDefault(t => t.Number == d.Room + 1)?.Room ?? -1)
                                                   .Concat(r.Rides.Select(n => byStage[(r.Area, r.Stage)].FirstOrDefault(t => t.Number == n)?.Room ?? -1))
                                                   .Concat(doors[r.Index].Select(d => d.Room)).Where(byRoom.ContainsKey);
                    foreach (int n in next) if (reachable.Add(n)) queue.Enqueue(n);
                }
            }
            // Hosts(); Whether a room takes a new door: one Kirby reaches, under its exits, not a goal room, a boss stage's, nor a mini-boss's;
            bool Hosts(SqueakPlanRoom r) => reachable.Contains(r.Room) && Exits(r) < MaxExits(r) && !goalRooms.Contains(r.Room)
                                            && !IsBossStage((r.Area, r.Stage)) && !r.Midboss && !r.Boss;

            /* SqueakSpot(); A floor cell for a mirror (door) or a landing: a floor 3 cells wide under it, open beside and above it, not
             * water, where the camera shows it (rows MirrorTop to the height - 3), one Kirby reaches from where he comes in (through star
             * blocks too), away from the room's doors, arrivals, landings and rides (2 x MirrorSpacing across, 2 up or down), the view
             * keeping 2 doors at most; in the room's middle, or hidden (the farthest from where he comes in, behind blocks); */
            // (firm: its floor no block that breaks, for a Mirra, which falls if it does, KatAM's not opening its door there);
            // (within: only among these cells, for the way-out pass);
            (int x, int y)? SqueakSpot(SqueakPlanRoom r, bool hidden, bool door, bool firm = false, HashSet<(int, int)> within = null) {
                int width = r.Width, height = r.Height;
                int[] Distances(bool blocks) {
                    int[] distance = Enumerable.Repeat(-1, width * height).ToArray();
                    Queue<(int x, int y)> open = new Queue<(int, int)>();
                    foreach (SqueakDoor d in r.Doors)
                        if (d.X >= 0 && d.Y >= 0 && d.X < width && d.Y < height && distance[d.Y * width + d.X] < 0) { distance[d.Y * width + d.X] = 0; open.Enqueue((d.X, d.Y)); }
                    while (open.Count > 0) {
                        (int x, int y) = open.Dequeue();
                        foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                            if (nx >= 0 && ny >= 0 && nx < width && ny < height && distance[ny * width + nx] < 0 && SqueakOpen(r.At(nx, ny), blocks)) {
                                distance[ny * width + nx] = distance[y * width + x] + 1;
                                open.Enqueue((nx, ny));
                            }
                    }
                    return distance;
                }
                int[] plain = Distances(false), broken = Distances(true);
                List<(int x, int y)> taken = Taken(r).ToList();
                (int x, int y)? best = null;
                double bestScore = double.MinValue;
                for (int y = MirrorTop; y <= height - 3; y++) {
                    for (int x = 1; x <= width - 2; x++) {
                        int distance = broken[y * width + x];
                        if (within != null && !within.Contains((x, y))) continue;
                        if (distance < 0 || r.At(x, y) != KatAMAir || !SqueakOpen(r.At(x, y - 1)) || !SqueakOpen(r.At(x - 1, y)) || !SqueakOpen(r.At(x + 1, y))) continue;
                        if (Enumerable.Range(x - 1, 3).Any(fx => !SqueakFloor(r.At(fx, y + 1)))) continue;
                        if (firm && Enumerable.Range(x - 1, 3).Any(fx => r.At(fx, y + 1) is >= KatAMStarBlock and <= KatAMMetalBlock)) continue;
                        if (taken.Any(c => Math.Abs(c.x - x) < MirrorSpacing * 2 && Math.Abs(c.y - y) <= 2)) continue;
                        if (r.Transports.Any(t => Math.Abs(t.x - x) < MirrorSpacing * 2 && Math.Abs(t.y - y) <= 2)) continue;
                        if (door && !ViewFits(r, x, y)) continue;
                        double score = hidden ? distance + (plain[y * width + x] < 0 ? 60 : 0) : -Math.Abs(x - width / 2.0) + random.NextDouble() * 6;
                        if (score > bestScore) { bestScore = score; best = (x, y); }
                    }
                }
                return best;
            }
            // HostSpot(); A spot for a new door in a room of a stage that takes one (not its first, if it has others), bigger rooms likelier;
            (SqueakPlanRoom room, int x, int y)? HostSpot((int area, int stage) s, bool hidden) {
                List<SqueakPlanRoom> list = byStage[s];
                var order = list.Skip(list.Count > 1 ? 1 : 0).OrderByDescending(r => random.NextDouble() * (1 + r.Width * r.Height / 330.0)).Concat(list.Take(list.Count > 1 ? 1 : 0));
                foreach (SqueakPlanRoom r in order)
                    if (Hosts(r) && SqueakSpot(r, hidden, true) is (int x, int y)) return (r, x, y);
                return null;
            }
            /* EntrySpot(); Where a door into a stage stands, two-way (a branch's way back): in its first room, in its middle, if it takes
             * one (the DS's doors only lead on: entered past its first room, the rooms before would be cut off); else none, and the
             * branch throws Kirby onto its start, one-way; */
            (SqueakPlanRoom room, int x, int y)? EntrySpot((int area, int stage) s) {
                SqueakPlanRoom first = byStage[s][0];
                bool hosts = Exits(first) < MaxExits(first) && !goalRooms.Contains(first.Room) && !first.Midboss && !first.Boss;
                return hosts && SqueakSpot(first, false, true) is (int x, int y) ? (first, x, y) : null;
            }
            // LandingIn(); Where a one-way door throws Kirby into a stage: a floor in one of its rooms away from its doors; its start, if none;
            (int room, int x, int y) LandingIn((int area, int stage) s) {
                // (nor in a hideout the Squeak Lairs keep: SqueakLair());
                foreach (SqueakPlanRoom r in byStage[s].Where(r => !goalRooms.Contains(r.Room) && !r.Midboss && !r.Boss && !SqueakLair(r)).OrderBy(_ => random.Next()))
                    if (SqueakSpot(r, false, false) is (int x, int y)) {
                        landings.Add((r.Room, x + 1, y + 1));
                        return (r.Room, x + 1, y + 1);
                    }
                return StageStart(s);
            }
            Door Add(SqueakPlanRoom at, int x, int y, (int room, int x, int y) to, DoorKind kind, bool oneWay, bool hidden = false) {
                Door door = new Door(x, y, to.room, to.x, to.y, byRoom.TryGetValue(to.room, out SqueakPlanRoom target) ? Facing(target, to.x) : (byte)1,
                                     OneWay: oneWay, Hidden: hidden, Kind: kind);
                doors[at.Index].Add(door);
                if (reachable.Contains(at.Room) && byRoom.ContainsKey(to.room)) Reach(to.room);
                return door;
            }
            // A two-way pair: a door at one spot to the other, and one back; Kirby lands on the door back, as KatAM's;
            void Pair((SqueakPlanRoom room, int x, int y) a, (SqueakPlanRoom room, int x, int y) b, DoorKind kind, bool hidden) {
                Add(a.room, a.x, a.y, (b.room.Room, b.x + 1, b.y + 1), kind, false, hidden);
                Add(b.room, b.x, b.y, (a.room.Room, a.x + 1, a.y + 1), kind == DoorKind.Branch ? DoorKind.BranchBack : kind, false);
            }

            worldStarts = new (int, int, int)[SqueakWorlds + 1];
            bossRooms = new Dictionary<int, int>();
            List<(int area, int stage)> stages = byStage.Keys.OrderBy(s => s.Item1).ThenBy(s => s.Item2).ToList();
            Dictionary<(int, int), int> depthOf = new Dictionary<(int, int), int>(), childrenOf = new Dictionary<(int, int), int>();
            Dictionary<(int, int), (int, int)> parentOf = new Dictionary<(int, int), (int, int)>();
            Dictionary<(int, int), (int, int)> goalBranch = new Dictionary<(int, int), (int, int)>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> regular = stages.Where(s => s.area == world && !IsBossStage(s)).ToList();
                if (regular.Count == 0) continue;
                worldStarts[world] = StageStart(regular[0]);
                Reach(worldStarts[world].room);

                // The tree: each stage after the first hangs off one already in it (deeper ones likelier, up to 3 branches a big stage);
                Dictionary<(int, int), int> depth = new Dictionary<(int, int), int> { [regular[0]] = 0 }, children = new Dictionary<(int, int), int> { [regular[0]] = 0 };
                int Branches((int area, int stage) p) => Math.Clamp(1 + byStage[p].Count / 4, 1, 3);
                bool hiddenNext = random.Next(2) == 0;
                // (a stage hangs off the first in the tree with a room that takes a door, those under their branches first, deeper ones
                // likelier; Prism Plains 1's rooms are small, 2 doors each: its branches hang off the stages after it; the stages left
                // out wait for more to be in, round after round);
                List<(int area, int stage)> waiting = regular.Skip(1).OrderBy(_ => random.Next()).ToList();
                for (bool progress = true; progress && waiting.Count > 0;) {
                    progress = false;
                    foreach (var s in waiting.ToList()) {
                        var froms = depth.Keys.OrderBy(p => children[p] < Branches(p) ? 0 : 1).ThenByDescending(p => random.NextDouble() * (1 + depth[p])).ToList();
                        foreach (var from in froms) {
                            // (the branch door in a room of `from`, two-way to a mirror in `s`'s first room that takes one; else one-way, onto
                            // a floor there);
                            if (HostSpot(from, hiddenNext) is not (SqueakPlanRoom, int, int) at) continue;
                            if (EntrySpot(s) is (SqueakPlanRoom, int, int) entry) Pair(at, entry, DoorKind.Branch, hiddenNext);
                            else Add(at.room, at.x, at.y, StageStart(s), DoorKind.Branch, true, hiddenNext);
                            hiddenNext = !hiddenNext;
                            children[from]++;
                            depth[s] = depth[from] + 1;
                            parentOf[s] = from;
                            children[s] = 0;
                            waiting.Remove(s);
                            progress = true;
                            break;
                        }
                        // (none takes a door: a stage's goal door, unused, leads on into it instead, one-way, as the DS's goals lead on);
                        if (!waiting.Contains(s)) continue;
                        foreach (var from in froms) {
                            if (goalBranch.ContainsKey(from) || !byStage[from].Any(r => r.Doors.Any(SqueakGoal))) continue;
                            goalBranch[from] = s;
                            Reach(StageStart(s).room);
                            children[from]++;
                            depth[s] = depth[from] + 1;
                            parentOf[s] = from;
                            children[s] = 0;
                            waiting.Remove(s);
                            progress = true;
                            break;
                        }
                    }
                }
                foreach (var pair in depth) depthOf[pair.Key] = pair.Value;
                foreach (var pair in children) childrenOf[pair.Key] = pair.Value;
            }

            /* The goals: the deepest stage's to the boss stage; the others in turn into a goal game, the next world's stage, a goal game
             * (the DS's goal doors, its own doorways: Plain); */
            Dictionary<(int, int), (int room, int x, int y)> goals = new Dictionary<(int, int), (int, int, int)>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> reached = stages.Where(s => s.area == world && depthOf.ContainsKey(s)).ToList();
                if (reached.Count == 0) continue;
                var gate = reached.OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage).First();
                // (Squeak Squad's final stage isn't world 8's boss stage: it's the finale's, SqueakFinalStage);
                var boss = stages.FirstOrDefault(s => s.area == world && IsBossStage(s) && s != SqueakFinalStage);
                /* (the others: on into the world's next stage by number, as the DS's goals lead on (the owner: "Goal doors reach to the
                 * next stage correct"), but 1 or 2 a world, ends of paths, into one of KatAM's goal games (the owner: "keep in mind the
                 * Goal minigame doors from KatAM as well for path ending sections, 1 or 2 per area"): those with no stage after them
                 * first (never the boss stage: only the deepest stage's goal leads there), then the stages nothing hangs off (a path's
                 * end), the deepest first; 2 where the world has 4 such goals or more); */
                List<(int area, int stage)> others = reached.Where(s => !goalBranch.ContainsKey(s) && !(s == gate && boss != default)).ToList();
                (int area, int stage)? NextStage((int area, int stage) s) =>
                    reached.Where(t => t.stage > s.stage && !IsBossStage(t)).OrderBy(t => t.stage).Cast<(int, int)?>().FirstOrDefault();
                int games = others.Count >= 4 ? 2 : Math.Min(1, others.Count);
                List<(int area, int stage)> toGames = others.Where(s => NextStage(s) == null)
                    .Concat(others.Where(s => NextStage(s) != null && childrenOf.GetValueOrDefault(s) == 0).OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage))
                    .Concat(others.OrderByDescending(s => depthOf[s])).Distinct().ToList();
                toGames = toGames.Take(Math.Max(games, others.Count(s => NextStage(s) == null))).ToList();
                int ends = 0;
                foreach (var s in reached) {
                    if (goalBranch.TryGetValue(s, out var child)) { goals[s] = StageStart(child); continue; }
                    if (s == gate && boss != default) { goals[s] = StageStart(boss); continue; }
                    goals[s] = toGames.Contains(s) ? (GoalRooms[(world + ends++) % GoalRooms.Length], GoalLanding.x, GoalLanding.y) : StageStart(NextStage(s).Value);
                }
            }
            foreach (SqueakPlanRoom r in rooms)
                foreach (SqueakDoor goal in r.Doors.Where(SqueakGoal))
                    if (goals.TryGetValue((r.Area, r.Stage), out var to) && goal.X < r.Width && goal.Y < r.Height)
                        doors[r.Index].Add(new Door(goal.X, goal.Y, to.room, to.x, to.y, byRoom.TryGetValue(to.room, out SqueakPlanRoom target) ? Facing(target, to.x) : (byte)1,
                                                    OneWay: true, Kind: DoorKind.Goal, Plain: true));

            /* The boss stages: the DS's doors on (into the boss room: one-way, there's none back); a room the DS reaches only by a
             * cutscene (no door or warp star of the stage leads to it: the final stage's, between its fights, room 5 to 7, 6 to 9...)
             * gets a mirror, one-way, from the first room before it with no way on yet (its fight's end), where Kirby lands there; the
             * last room with a boss is the world's boss room. A warp star is a way on (8-4's rides, 3 to 4 to 5: the star lands Kirby
             * where the DS does; its arrival door in 8-4-4 is under the room's floor, in its last rows); */
            foreach (var s in stages.Where(IsBossStage)) {
                List<SqueakPlanRoom> list = byStage[s];
                HashSet<int> led = list.SelectMany(r => r.Doors.Where(d => d.Room != SqueakNoRoom).Select(d => d.Room + 1).Concat(r.Rides)).ToHashSet();
                List<SqueakPlanRoom> ends = new List<SqueakPlanRoom>();
                foreach (SqueakPlanRoom next in list) {
                    if (next != list[0] && !led.Contains(next.Number) && ends.Count > 0) {
                        SqueakPlanRoom before = ends[0];
                        ends.RemoveAt(0);
                        (int x, int y) arrival = before.Doors.FirstOrDefault(SqueakArrival) is SqueakDoor d ? Drop(before, d.X, d.Y) : (before.Width / 2, before.Height / 2);
                        (int x, int y) landing = next.Doors.FirstOrDefault(SqueakArrival) is SqueakDoor n ? Drop(next, n.X, n.Y) : (next.Width / 2, next.Height / 2);
                        Add(before, arrival.x, arrival.y, (next.Room, landing.x + 1, landing.y + 1), DoorKind.Boss, true);
                    }
                    if (!next.Doors.Any(d => d.Room != SqueakNoRoom) && next.Rides.Count == 0) ends.Add(next);
                }
                if (list.LastOrDefault(r => r.Boss) is SqueakPlanRoom bossRoom) bossRooms[bossRoom.Room] = s.area;
            }
            /* (the finale's part, SqueakFinalStage: its way in where the finale leads in (squeakFinaleEntry: its start), its boss's way out
             * where the finale goes on (SqueakFinaleShard: no shard, SqueakShardExits()); */
            squeakFinaleEntry = null;
            squeakFinaleBoss = -1;
            if (byStage.ContainsKey(SqueakFinalStage) && byStage[SqueakFinalStage].LastOrDefault(r => r.Boss) is SqueakPlanRoom finalBoss) {
                squeakFinaleEntry = StageStart(SqueakFinalStage);
                squeakFinaleBoss = finalBoss.Room;
                bossRooms[finalBoss.Room] = SqueakFinaleShard;
            }

            /* Loops, few and each for a reason: one-way shortcuts from the deepest stages back to one at least two branches shallower
             * (else any shallower), a third as many as a world has stages; a hidden two-way door between each world and the next; */
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> reached = stages.Where(s => s.area == world && depthOf.ContainsKey(s)).ToList();
                foreach (var a in reached.OrderByDescending(s => depthOf[s]).Take(Math.Max(1, reached.Count / 3))) {
                    var home = reached.Where(s => depthOf[s] <= depthOf[a] - 2).ToList();
                    if (home.Count == 0) home = reached.Where(s => depthOf[s] < depthOf[a]).ToList();
                    if (home.Count == 0) continue;
                    if (HostSpot(a, true) is (SqueakPlanRoom, int, int) at) Add(at.room, at.x, at.y, LandingIn(home[random.Next(home.Count)]), DoorKind.Loop, true, true);
                }
                if (world == SqueakWorlds) continue;
                List<(int area, int stage)> next = stages.Where(s => s.area == world + 1 && depthOf.ContainsKey(s)).ToList();
                if (reached.Count == 0 || next.Count == 0) continue;
                // (into the next world's first stage, its way in: its tree is reached from there; from any of ours that takes a door);
                var theirs = next[0];
                foreach (var ours in reached.OrderBy(_ => random.Next()))
                    if (HostSpot(ours, true) is (SqueakPlanRoom, int, int) here) {
                        if (EntrySpot(theirs) is (SqueakPlanRoom, int, int) there) Pair(here, there, DoorKind.Link, true);
                        else Add(here.room, here.x, here.y, StageStart(theirs), DoorKind.Link, true, true);
                        break;
                    }
            }
            /* The switch rooms (rule 5): each world's reached from a Mirra hidden in its second half (its stages from the middle on by number,
             * the deepest first; standing on a floor that can't break), one-way, onto its floor off its mirrors (SwitchArrival); its right
             * floor mirror both ways with the start of the stage before the boss's (the deepest: its goal leads there), its left one with
             * the start of the deepest stage off the way from the world's first stage to that one, not the Mirra's own (else the way's
             * first); in each a mirror back in its first room, if it takes one (EntrySpot()), else the switch room's leads on, one-way; */
            switchDoors = new Dictionary<int, List<Door>>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> reached = stages.Where(s => s.area == world && depthOf.ContainsKey(s)).ToList();
                if (reached.Count == 0) continue;
                int switchRoom = firstSwitchRoom + world - 1, level = SqueakLayoutLevel(world);
                (int x, int y) arrival = SwitchArrival(level);
                var gate = reached.OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage).First();
                (int area, int stage)? mirraStage = null;
                foreach (var s in reached.Where(s => s.stage > reached.Max(t => t.stage) / 2).OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage)) {
                    foreach (SqueakPlanRoom r in byStage[s].Where(Hosts).OrderBy(_ => random.Next()))
                        if (SqueakSpot(r, true, true, true) is (int x, int y)) {
                            Add(r, x, y, (switchRoom, arrival.x + 1, arrival.y + 1), DoorKind.SwitchEntry, true, true);
                            mirraStage = s;
                            break;
                        }
                    if (mirraStage != null) break;
                }
                HashSet<(int, int)> way = new HashSet<(int, int)>();
                for (var s = gate; ; s = parentOf[s]) { way.Add(s); if (!parentOf.ContainsKey(s)) break; }
                var off = reached.Where(s => !way.Contains(s) && s != mirraStage).OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage).ToList();
                var left = off.Count > 0 ? off[0] : reached.Where(s => way.Contains(s) && s != gate).OrderBy(s => depthOf[s]).Cast<(int, int)?>().FirstOrDefault() ?? gate;
                List<Door> list = switchDoors[world] = new List<Door>();
                (int x, int y)[] sides = SwitchSides(level);
                foreach (((int x, int y) side, (int area, int stage) stage) in new[] { (sides[1], gate), (sides[0], left) }) {
                    byte facing = (byte)(side.x * 2 < SwitchWidth ? 1 : 2);
                    if (EntrySpot(stage) is (SqueakPlanRoom, int, int) entry) {
                        list.Add(new Door(side.x, side.y, entry.room.Room, entry.x + 1, entry.y + 1, Facing(entry.room, entry.x + 1), Kind: DoorKind.SwitchFloor));
                        Add(entry.room, entry.x, entry.y, (switchRoom, side.x + 1, side.y + 1), DoorKind.SwitchFloor, false);
                        doors[entry.room.Index][^1] = doors[entry.room.Index][^1] with { Facing = facing };
                    } else {
                        (int room, int x, int y) start = StageStart(stage);
                        list.Add(new Door(side.x, side.y, start.room, start.x, start.y, Facing(byRoom[start.room], start.x), OneWay: true, Kind: DoorKind.SwitchFloor));
                    }
                }
            }

            /* Every room reached (rule 9): one nothing leads into gets a one-way door into it, where the DS lands Kirby there (its
             * arrival), from a room of its stage that takes one (else of its world), hidden; but for the rooms a stage reaches only
             * through its Squeaks on the DS (after its goal: the gold-pile treasure rooms and the hideouts where Spinni, Doc or Storo
             * fight for the chest they took), the Squeak Lairs' (below), so SqueakLair(), SqueakHideout(); */
            bool SqueakLair(SqueakPlanRoom r) {
                List<SqueakPlanRoom> list = byStage[(r.Area, r.Stage)];
                int goal = list.FirstOrDefault(t => t.Doors.Any(SqueakGoal))?.Number ?? int.MaxValue;
                if (r.Number <= goal) return false;
                // (but not one the DS's doors lead to from the stage's rooms up to its goal: 7-3-5, behind 7-3-1's hidden way);
                HashSet<int> led = list.Where(t => t.Number <= goal).Select(t => t.Number).ToHashSet();
                for (bool more = true; more;) {
                    more = false;
                    foreach (SqueakPlanRoom t in list.Where(t => led.Contains(t.Number)).ToList())
                        foreach (SqueakDoor d in t.Doors.Where(d => d.Room != SqueakNoRoom && !SqueakGoal(d)))
                            more |= led.Add(d.Room + 1);
                }
                return !led.Contains(r.Number);
            }
            /* SqueakHideout(); Whether a room after a stage's goal is one of the DS's hideouts (one of the four fights, its Squeaks in it)
             * or gold-pile rooms (a screen, or two tall, with nothing in it), every stage's copies of the same few: the ones no lair
             * uses are left out; a stage's other rooms after its goal (1-1-5, the field past its goal) are reached as any (below; the
             * sky rooms the DS's warp stars fly through, 3-4-5, 8-3-6 and 8-3-7, are its rides', never stood in); */
            static bool SqueakHideout(SqueakPlanRoom r) => r.Squeaks.Count > 0 || r.Width == 18 && r.Height is 15 or 32;
            /* The Squeak Lairs (the owner: "Entrance 1 -> Squeak Lair with Miniboss -> Entrance 2 as an interlude between zones in the
             * Squeak Squad map. This should use the same system NiDL made us create of Beat Boss, reveal exit, only 1 exit per boss lock
             * in room"): for each Squeak that steals a stage's treasure on the DS (its thief: Spinni, Doc or Storo, a chase record with
             * SqueakThief too), the hideout it fights in among the stage's rooms after the goal (SqueakLair(), with its Squeakers) is a
             * mini-boss room (rule 8): a DS door of the stage leads into it, one-way, onto the DS's arrival there (the door into the room
             * the thief waits in, else the nearest to it: one Kirby gets to in a room he reaches, not hidden, into another of the
             * stage's own rooms (not its lair rooms), not a fight's), a nest standing at that door (the DS's own, its world's look:
             * SqueakNests()), and its one way out is a mirror, shut until the Squeak is beaten and away from where Kirby lands, to
             * where that door led; */
            squeakLairs.Clear();
            Dictionary<(int room, int door), (int room, int x, int y)> lairRedirect = new Dictionary<(int, int), (int, int, int)>();
            // (Randomize Squeak Lairs: squeakLairSeed; its own generator, the rest of the map as without it);
            Random lairRandom = squeakLairSeed is int lairSeed ? new Random(lairSeed ^ 0x5A1E) : null;
            // (a picked lair's door only in a room that draws the DS's nest: the rooms' usual nest blocks, a tileset with one);
            string nestKey = rooms.Where(r => r.NestKey != null).GroupBy(r => r.NestKey).OrderByDescending(g => g.Count()).Select(g => g.Key).FirstOrDefault();
            foreach (var s in stages.Where(s => !IsBossStage(s))) {
                List<SqueakPlanRoom> list = byStage[s];
                var thieves = list.Where(r => !SqueakLair(r))
                                  .SelectMany(r => r.Squeaks.Where(q => (q.flags & SqueakThief) != 0 && q.character != SqueakerCharacter).Select(q => (q.character, room: r.Number)))
                                  .GroupBy(t => t.character).Select(g => (character: g.Key, room: g.Max(t => t.room))).OrderByDescending(t => t.room).ToList();
                /* (the lairs: each thief's hideout, its door the nearest to the room it waits in; picked from the seed, each of the
                 * stage's hideouts with its Squeaks one time in LairShare, its fight its own Squeak (the Squeakers' one of LairFighters),
                 * its door any that can be one); */
                List<(int thief, int thiefRoom, SqueakPlanRoom hideout)> picks = lairRandom == null
                    ? thieves.Select(t => (t.character, t.room, list.FirstOrDefault(r => SqueakLair(r) && !reachable.Contains(r.Room) && r.Squeaks.Any(q => q.character == t.character)))).ToList()
                    : list.Where(r => SqueakLair(r) && !reachable.Contains(r.Room) && r.Squeaks.Count > 0).Where(_ => lairRandom.Next(LairShare) == 0)
                          .Select(r => (r.Squeaks.Select(q => q.character).FirstOrDefault(c => c != SqueakerCharacter, LairFighters[lairRandom.Next(LairFighters.Length)]), -1, r))
                          .ToList();
                foreach ((int thief, int thiefRoom, SqueakPlanRoom hideout) in picks) {
                    if (hideout == null || hideout.Doors.FirstOrDefault(SqueakArrival) is not SqueakDoor arrival) continue;
                    var entrances = new List<(SqueakPlanRoom r, int index, SqueakDoor d, SqueakPlanRoom to, SqueakDoor at)>();
                    foreach (SqueakPlanRoom r in list.Where(r => reachable.Contains(r.Room) && !r.Midboss && !r.Boss && !SqueakLair(r)))
                        for (int k = 0; k < r.Doors.Count; k++) {
                            SqueakDoor d = r.Doors[k];
                            if (d.Room == SqueakNoRoom || SqueakGoal(d) || (d.Flags & SqueakHiddenDoor) != 0 || d.Kind == SqueakStartKind || d.X >= r.Width || d.Y >= r.Height
                                || lairRedirect.ContainsKey((r.Room, k)) || lairRandom != null && (r.NestKey != nestKey || !r.NestDrawn)) continue;
                            SqueakPlanRoom to = list.FirstOrDefault(t => t.Number == d.Room + 1);
                            if (to == null || to == r || SqueakLair(to) || to.Midboss || to.Boss) continue;
                            SqueakDoor at = d.Door < to.Doors.Count ? to.Doors[d.Door] : to.Doors.FirstOrDefault(o => o.Number == d.Door);
                            if (at != null && Leads(r, d)) entrances.Add((r, k, d, to, at));
                        }
                    var entrance = lairRandom != null ? (entrances.Count > 0 ? entrances[lairRandom.Next(entrances.Count)] : default)
                                 : entrances.OrderBy(e => Math.Abs(e.to.Number - thiefRoom)).ThenBy(e => e.to.Number > thiefRoom ? 1 : 0).ThenByDescending(e => e.r.Number).FirstOrDefault();
                    if (entrance.r == null) continue;
                    (int x, int y) landing = Drop(hideout, arrival.X, arrival.Y);
                    landings.Add((hideout.Room, landing.x + 1, landing.y + 1));
                    /* (the way out on the hideout's own DS doorway, where its black doorway was (the owner: "The one way door on the Squeak
                     * Lairs should be where the black door was"), even where Kirby lands beside it (Spinni's and the Squeakers' hideouts:
                     * it's shut till the fight's won); else, a hideout without one, a floor away from where he lands, SqueakSpot()); */
                    SqueakDoor doorway = hideout.Doors.FirstOrDefault(d => d.Room != SqueakNoRoom && !SqueakGoal(d) && d.X < hideout.Width && d.Y < hideout.Height);
                    (int x, int y)? exitSpot = doorway != null ? (doorway.X, doorway.Y) : SqueakSpot(hideout, false, true) ?? SqueakSpot(hideout, false, false);
                    if (exitSpot is not (int ex, int ey)) {
                        landings.RemoveAt(landings.Count - 1);
                        continue;
                    }
                    (int x, int y) after = Drop(entrance.to, entrance.at.X, entrance.at.Y);
                    (int room, int x, int y) exit = (entrance.to.Room, after.x + 1, after.y + 1);
                    doors[hideout.Index].Add(new Door(ex, ey, exit.room, exit.x, exit.y, Facing(entrance.to, exit.x), OneWay: true, Kind: DoorKind.Lair, Shut: true));
                    hideout.Midboss = true;
                    lairRedirect[(entrance.r.Room, entrance.index)] = (hideout.Room, landing.x + 1, landing.y + 1);
                    squeakLairs.Add(new SqueakLairPlan(hideout.Room, thief, entrance.r.Room, entrance.index, entrance.d.X, entrance.d.Y,
                                                       (hideout.Room, landing.x + 1, landing.y + 1), exit));
                    Reach(hideout.Room);
                }
            }
            for (bool progress = true; progress;) {
                progress = false;
                foreach (SqueakPlanRoom r in rooms.Where(r => !reachable.Contains(r.Room) && !IsBossStage((r.Area, r.Stage)) && !(SqueakLair(r) && SqueakHideout(r)))) {
                    // (where the DS lands him: its arrival, else the door a DS door from the stage lands him on: 7-2-8's, under 7-2-7's thin
                    // floor);
                    SqueakDoor arrival = r.Doors.FirstOrDefault(SqueakArrival)
                        ?? byStage[(r.Area, r.Stage)].SelectMany(t => t.Doors).Where(o => o.Room != SqueakNoRoom && o.Room + 1 == r.Number)
                                                     .Select(o => o.Door < r.Doors.Count ? r.Doors[o.Door] : r.Doors.FirstOrDefault(d => d.Number == o.Door)).FirstOrDefault(d => d != null);
                    if (arrival == null) continue;
                    (int x, int y) landing = Drop(r, arrival.X, arrival.Y);
                    // (from the world's other stages first: a room of its own stage may lead to it, back into the throw's own room, 7-3-5
                    // into 7-3-2's top, whose free spot then held its way on);
                    var hosts = rooms.Where(t => t.Area == r.Area && t.Stage != r.Stage).Concat(byStage[(r.Area, r.Stage)]).Where(Hosts);
                    foreach (SqueakPlanRoom host in hosts)
                        if (SqueakSpot(host, true, true) is (int x, int y)) {
                            Add(host, x, y, (r.Room, landing.x + 1, landing.y + 1), DoorKind.Loop, true, true);
                            progress = true;
                            break;
                        }
                }
            }

            /* Every place Kirby gets to gets him back out (rule 9): a place (a room's cells he gets through from where he arrives, never
             * down through a thin floor: Through()) none of whose ways on (the DS's doors, the doors added, its warp stars and fuse
             * cannons) leads out of the worlds' rooms (into a goal game, a switch room, or a boss's shard), nor to a place that does,
             * gets a one-way door out, as KatAM's one-way mirrors throw him on: onto the start of the first stage of its world he gets
             * out from (7-3-2's top, whose way on is under a thin floor, the DS's own; not in a goal room, a fight's or a boss stage's); */
            List<(int room, int x, int y)> entries = worldStarts.Where(w => byRoom.ContainsKey(w.room))
                                                                .Concat(switchDoors.Values.SelectMany(list => list).Select(d => (d.Room, d.DestX, d.DestY))).ToList();
            HashSet<int> shardRooms = bossRooms.Keys.ToHashSet();
            for (int pass = 0; pass < rooms.Count; pass++) {
                List<(SqueakPlanRoom r, HashSet<(int, int)> cells, List<(int room, int x, int y)> leads)> places = new List<(SqueakPlanRoom, HashSet<(int, int)>, List<(int, int, int)>)>();
                Dictionary<(int, int, int), int> placeOf = new Dictionary<(int, int, int), int>();
                // PlaceAt(); The place an arrival leads into (-1: out of the worlds' rooms), flooding it the first time;
                int PlaceAt(int room, int x, int y) {
                    if (!byRoom.TryGetValue(room, out SqueakPlanRoom r)) return -1;
                    x = Math.Clamp(x, 0, r.Width - 1); y = Math.Clamp(y, 0, r.Height - 1);
                    if (placeOf.TryGetValue((room, x, y), out int known)) return known;
                    HashSet<(int, int)> seen = new HashSet<(int, int)> { (x, y) };
                    Queue<(int x, int y)> open = new Queue<(int, int)>(seen);
                    while (open.Count > 0) {
                        (int cx, int cy) = open.Dequeue();
                        foreach ((int nx, int ny) in new[] { (cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1) })
                            if (nx >= 0 && ny >= 0 && nx < r.Width && ny < r.Height && Through(r, cy, nx, ny) && seen.Add((nx, ny))) open.Enqueue((nx, ny));
                    }
                    bool Near(int dx, int dy) => Enumerable.Range(-1, 3).Any(ox => Enumerable.Range(-1, 3).Any(oy => seen.Contains((dx + ox, dy + oy))));
                    List<(int, int, int)> leads = new List<(int, int, int)>();
                    // (a DS door into a lair leads there; a lair's own DS doors aren't doors);
                    bool lair = squeakLairs.Any(l => l.Room == r.Room);
                    for (int k = 0; k < r.Doors.Count && !lair; k++) {
                        SqueakDoor d = r.Doors[k];
                        if (d.Room == SqueakNoRoom || SqueakGoal(d) || !Near(d.X, d.Y)) continue;
                        if (lairRedirect.TryGetValue((r.Room, k), out var into)) leads.Add((into.room, into.x - 1, into.y - 1));
                        else if (byStage[(r.Area, r.Stage)].FirstOrDefault(t => t.Number == d.Room + 1) is SqueakPlanRoom t
                                 && (d.Door < t.Doors.Count ? t.Doors[d.Door] : t.Doors.FirstOrDefault(o => o.Number == d.Door)) is SqueakDoor landing)
                            leads.Add((t.Room, landing.X, landing.Y));
                    }
                    foreach (Door d in doors[r.Index].Where(d => Near(d.X, d.Y))) leads.Add((d.Room, d.DestX - 1, d.DestY - 1));
                    foreach (var ride in r.RideTo.Where(t => Near(t.x, t.y))) leads.Add((ride.room, ride.toX, ride.toY));
                    if (shardRooms.Contains(r.Room)) leads.Add((-1, 0, 0));
                    places.Add((r, seen, leads));
                    return placeOf[(room, x, y)] = places.Count - 1;
                }
                Queue<int> unvisited = new Queue<int>();
                HashSet<int> visited = new HashSet<int>();
                foreach ((int room, int x, int y) in entries)
                    if (PlaceAt(room, x - 1, y - 1) is int first && first >= 0 && visited.Add(first)) unvisited.Enqueue(first);
                while (unvisited.Count > 0)
                    foreach ((int room, int x, int y) in places[unvisited.Dequeue()].leads)
                        if (PlaceAt(room, x, y) is int next && next >= 0 && visited.Add(next)) unvisited.Enqueue(next);
                // (the places he gets out from: leading out, or to one that does);
                HashSet<int> outs = new HashSet<int>();
                for (bool more = true; more;) {
                    more = false;
                    foreach (int i in visited)
                        if (!outs.Contains(i) && places[i].leads.Any(l => PlaceAt(l.room, l.x, l.y) is int to && (to < 0 || outs.Contains(to)))) { outs.Add(i); more = true; }
                }
                List<int> trapped = visited.Where(i => !outs.Contains(i)).OrderBy(i => (places[i].r.Area, places[i].r.Stage, places[i].r.Number)).ToList();
                if (trapped.Count == 0) break;
                (SqueakPlanRoom room, int x, int y)? from = null;
                foreach (int i in trapped) {
                    SqueakPlanRoom r = places[i].r;
                    if (IsBossStage((r.Area, r.Stage)) || r.Midboss || r.Boss || goalRooms.Contains(r.Room)) continue;
                    if ((SqueakSpot(r, false, true, within: places[i].cells) ?? SqueakSpot(r, false, false, within: places[i].cells)) is (int x, int y)) {
                        from = (r, x, y);
                        break;
                    }
                }
                if (from is not (SqueakPlanRoom, int, int) spot) throw new InvalidOperationException($"No way out of Squeak Squad room 0x{places[trapped[0]].r.Room:X}.");
                (int room, int x, int y)? way = null;
                foreach (var stage in stages.Where(t => t.area == spot.room.Area && !IsBossStage(t) && depthOf.ContainsKey(t)).OrderBy(t => t.stage)) {
                    (int room, int x, int y) start = StageStart(stage);
                    if (PlaceAt(start.room, start.x - 1, start.y - 1) is int at && at >= 0 && outs.Contains(at)) { way = start; break; }
                }
                if (way is not (int, int, int) to) throw new InvalidOperationException($"No way out of Squeak Squad room 0x{spot.room.Room:X}: no stage of its world leads out.");
                Add(spot.room, spot.x, spot.y, to, DoorKind.Link, true);
            }
            return doors;
        }
    }
}
