namespace KatAMRandomizer {
    /* Rooms: every room of Kirby: Nightmare in Dream Land, converted from the player's NiDL ROM into KatAM's free room slots (the
     * rooms whose gRoomProps entry is all 0xFFFF), with their foreground, background, collision, enemies, mini-bosses, bosses and
     * items; their doors become KatAM's mirrors (PlanDoors()), their warp stars and cannons KatAM's own (Transport).
     * NiDL rooms (USA addresses; the European and Japanese ROMs hold the same rooms, their table elsewhere: region.RoomTable):
     * • RoomTable: per level (0-7) 8 stage lists (RoomTable + level * 0x20 + stage * 4), then the levels' hub lists (+0x100 + level *
     *   4); each list holds room records, null where a room is missing, until a record whose index (+2) isn't its place.
     * • Room record: +8 cells (LZ77, width * height u32: metatile, attribute; the attribute's high byte is the collision), +0xC
     *   breakable block numbers (LZ77, u16 per cell), +0x10 object data, +0x14 u16 width, height (16x16 metatiles), +0x18 the
     *   foreground (palette: u16 byte count and colors, loaded at palette 2; LZ77 tiles at 0x06004000; LZ77 metatiles, 4 tilemap
     *   entries each), +0x28 the background (palette: u16 byte count and colors, at palette 8; LZ77 tiles; map: u16 byte count, u16
     *   width, height, then 0xFFFF and the LZ77 map in 64x32 screenblock order, or the map itself, row by row), +0x44 exits (12 bytes:
     *   room, 0, u16 tile x, y, landing x, y, 0), +0x48 objects (8 bytes: category + 1, index, variant, flags, u16 x, y) up to the
     *   record itself, +0x50 u16 x, y: where Kirby starts.
     * • Collision: high nibble = kind (0, 5, 7 solid; 1 one-way (0x10 a door); 2 breakable; 3 ladders (0x30, 0x31), walls, and 0x34:
     *   platforms Kirby jumps through, clouds and bridges, or blocks a bomb sets off where they touch breakable blocks, RoomCells());
     *   thin floors NiDL lets Kirby drop through (Down) are KatAM's that do too (0x0E, slopes 0x10-0x13, 0x18, 0x19; its 0x0F,
     *   0x14-0x17, 0x1A, 0x1B don't: a way on through one would be shut); 4 air currents; 6 spikes; 8, C water; 9 water with one-way floors; E water with spikes), low nibble = shape in NiDL's height maps
     *   (0x087328F0: empty, full, 45° floors, gentle floors, ceilings). Collision maps it to KatAM's values (gCollisionAttributes).
     * KatAM (per version, KatAMExpansionCode's RomSymbols): gRoomProps (0x28 bytes per room: song, ..., indexes into the tables below,
     * doorsIdx), gForegroundInfo (tiles at 0x06008000), gForegroundPalettes (6 palettes at 0), gForegroundTilemaps (RoomTiledBG: width,
     * height in tiles, LZ77 tilemap), gRoomTiledBGs (the background: tiles at 0x06000000, at most 900 of them as KatAM's HUD tiles
     * follow, palettes 6-13, its map row by row), gSolidityMaps (RL collision per 16x16 block, special blocks list), gLevelObjLists
     * (object templates), gUnk_08D63C28 (CPU helpers' zones), gUnk_08D6CD0C (per room: area, routes; written in place) and
     * gUnk_08D640A4 (doors per doorsIdx). The tables are copied with the new entries and every reference to them repointed. KatAM's
     * rooms have at most 1950 blocks (its buffers for the collision, its zones and tilemap, per loaded room); "ROM Expansion/rooms.c"
     * gives them room for BigRoomBlocks (see BigRooms()), which every NiDL room fits in, whole. Every new room uses doorsIdx 287, the
     * one gVisitedDoors entry (0x120) no vanilla room uses; */
    internal static partial class KatAMExpansion {
        const int MaxRoomBlocks = BigRoomBlocks, BackgroundTiles = 900, SharedDoors = 287, FirstDoorsList = 0x120, RoomFlags = 0x79;

        /* A cannon's fuse (NiDL 0x38-0x3D, inside walls: up-down, left-right, up-right, up-left, right-down, left-down) -> KatAM's
         * (0x22-0x27: up-down, right-down, left-down, left-right, up-right, up-left; solid, as walls), which its fuse object
         * (0x78, cannon_fuse.c) burns along to the cannon; elsewhere they're walls (0x0D, RoomCollision); */
        static readonly byte[] FuseCollision = { 0x22, 0x25, 0x26, 0x27, 0x23, 0x24 };

        // NiDL collision -> KatAM collision (see the notes above);
        static readonly byte[] RoomCollision = {
            0x00, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00,
            0x00, 0x0E, 0x18, 0x19, 0x10, 0x11, 0x12, 0x13, 0x00, 0x0E, 0x18, 0x19, 0x10, 0x11, 0x12, 0x13,
            0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
            0x21, 0x20, 0x21, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x0B, 0x0C, 0x09, 0x0A, 0x09, 0x0A,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x00,
            0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D,
            0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D, 0x0D,
            0x48, 0x0D, 0x38, 0x3A, 0x40, 0x41, 0x42, 0x43, 0x34, 0x36, 0x3C, 0x3D, 0x3E, 0x3F, 0x48, 0x48,
            0x48, 0x0E, 0x18, 0x19, 0x10, 0x11, 0x12, 0x13, 0x48, 0x0E, 0x18, 0x19, 0x10, 0x11, 0x12, 0x13,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x38, 0x3A, 0x38, 0x3A, 0x34, 0x36, 0x34, 0x36,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
        };

        /* NiDL enemies (category 0, index = NiDL type - 8) -> KatAM object types: the ones in both games, and the ported ones (type 0x31,
         * the behavior's high nibble, NIDL_* in nidl.h; NiDL's variant in the low nibble). NiDL's 0x09 (a Waddle Dee with a headband) is
         * a Waddle Dee, 0x19 and 0x2A are both Shotzos; */
        static readonly Dictionary<int, byte> NativeEnemies = new Dictionary<int, byte> {
            [0] = 0x00, [1] = 0x00, [2] = 0x1E, [4] = 0x16, [5] = 0x15, [7] = 0x18, [9] = 0x12, [10] = 0x06, [12] = 0x01, [14] = 0x28,
            [17] = 0x35, [18] = 0x19, [19] = 0x1C, [20] = 0x04, [21] = 0x05, [24] = 0x14, [25] = 0x03, [27] = 0x02, [28] = 0x1A,
            [34] = 0x35, [35] = 0x13, [37] = 0x1B,
        };
        static readonly Dictionary<int, int> PortedEnemies = new Dictionary<int, int> {
            [3] = 0, [8] = 1, [16] = 2, [22] = 3, [26] = 4, [13] = 5, [15] = 6, [33] = 7, [23] = 8, [36] = 9, [29] = 10, [6] = 11,
            [11] = 12, [30] = 13, [31] = 13, [32] = 13, [39] = 14,
        };
        /* NiDL mini-bosses (category 6, Miniboss.NightmareIndex) -> KatAM type and behavior (0: Bonkers, 3: Mr. Frosty, 7: Phan Phan;
         * the rest ported, NiDL's variant in the low nibble when the port has it); Meta Knight's event (category 7, its variant the
         * knights' line-up) is the Meta-Knights. Each fight's screen is where its camera lock (category 5, index 0) is (see ArenaOf()); */
        static readonly Dictionary<int, (byte type, byte behavior)> RoomMinibosses = new Dictionary<int, (byte, byte)> {
            [0] = (0x39, 0), [1] = (0x40, 0x00), [2] = (0x40, 0x20), [3] = (0x38, 0), [4] = (0x40, 0x10), [5] = (0x41, 0x30),
            [6] = (0x41, 0x40), [7] = (0x3A, 0),
        };
        static readonly (byte type, byte behavior) RoomMetaKnights = (0x42, 0x50);
        const int MinibossCategory = 6, MetaKnightsCategory = 7, CameraLockCategory = 5, BossCategory = 2;
        const byte ArenaBehavior = 0xF0;    // NIDL_SPECIAL << 4 | 0, the arena (nidl.h);
        /* NiDL bosses (category 2: 0 King Dedede, 1 Paint Roller, 2 Meta Knight, 3 Heavy Mole, 4 Mr. Shine & Mr. Bright, 5 Whispy Woods,
         * 6 Kracko, 7 and 8 Nightmare) -> the ported ones (type 0x43, "ROM Expansion/nidl_boss.c"): the boss (NIDL_* bosses), the spot
         * NiDL's spawn puts it on (-1: the template's), and the middle of its fight's screen from that spot. Its behavior's low nibble is
         * the mirror shard it leaves: its level's (0-7). The others aren't ported yet: their rooms stay empty, their mirror on; */
        static readonly Dictionary<int, (byte boss, int x, int y, int screenDx, int screenDy)> RoomBosses = new Dictionary<int, (byte, int, int, int, int)> {
            [0] = (6, 200, 280, -80, 0),    // King Dedede: the ring's right (NiDL's (232, 296) is off its screen: he walks in);
            [1] = (4, -1, -1, 0, 0),        // Paint Roller: the template's (24, 40), his easels (bosses/paint_roller.c) from it;
            [2] = (3, -1, -1, 0, 0),        // Meta Knight: the template's (232, 56), NiDL's ledge (he drops to the floor);
            [3] = (5, 200, 300, -64, -40),    // Heavy Mole: on the right of its screen (NiDL's (136, 264) its middle: it scrolls with the mole there);
            [4] = (2, -1, -1, 0, 0),        // Mr. Shine & Mr. Bright: the template's (208, 104), on the floor of their one-screen room;
            [5] = (0, 206, 256, 0, 64),     // Whispy Woods: its face (0x080B2A00), the screen down to the floor under it;
            [7] = (7, 160, 830, 0, 200),    // Nightmare's Power Orb: over the floor at the bottom of the fall (NiDL's (232, 24) is its top, in space);
            [8] = (8, 150, 280, 0, 200),    // Nightmare: over the floor at the room's bottom (NiDL's (215, 72), in the sky);
            [6] = (1, -1, -1, 0, 0),        // Kracko: the template's (its tower's bottom, where Kirby comes in), that screen;
        };
        // NiDL items (category 3): 1-Up, Maxim Tomato, Invincibility Candy, Energy Drink -> KatAM's 1UP, Maximum Tomato, Lollipop, Pep Brew;
        static readonly Dictionary<int, byte> RoomItems = new Dictionary<int, byte> { [1] = 0x63, [2] = 0x61, [3] = 0x64, [4] = 0x5F };
        // KatAM song per NiDL level (Vegetable Valley ... Rainbow Resort, then the Fountain of Dreams);
        static readonly int[] LevelSongs = { 5, 8, 9, 3, 6, 8, 10, 3 };

        /* Areas: each NiDL level is an area of its own, as KatAM's (gUnk_08D6CD0C +0x46: 0-8 KatAM's, 9 the tutorial, 10 the Dimension
         * Mirror), AreaBase + level (the NiDL hub with the Fountain of Dreams), and its rooms (hub first) take the free room numbers of
         * one hundred (LevelHundreds; KatAM numbers its rooms by hundreds too, mostly by area): the NiDL hub and the Fountain of Dreams
         * 1-99, Vegetable Valley 100-199 ... Rainbow Resort 700-799. The HUD shows the area's plate (its name, AreaPlates) in its corner,
         * and the CPU helpers' routes are per area (gUnk_08D6DCAC); both only had areas 0-10: see ExtendAreas(); */
        const int AreaBase = 11, MapRoomsBlock = MapTableAddress + 0xE0, MapShardExits = MapTableAddress + 0xD8, MapBosses = MapTableAddress + 0xCC;
        static readonly int[] LevelHundreds = { 1, 2, 3, 4, 5, 6, 7, 0 };
        static readonly (string first, string second)[] LevelNames = {
            ("VEGETABLE", "VALLEY"), ("ICE CREAM", "ISLAND"), ("BUTTER", "BUILDING"), ("GRAPE", "GARDEN"), ("YOGURT", "YARD"),
            ("ORANGE", "OCEAN"), ("RAINBOW", "RESORT"), ("FOUNTAIN OF", "DREAMS"),
        };

        /* Big switches: one per level, in its switch room (see SwitchRooms; NiDL's own aren't ported), as KatAM's areas: KatAM's
         * object 0x86, setting the level's world state slot (template +4, saved: gUnk_02028C10, 0x20 of them; KatAM's big switches use
         * slots 1-15, Central Circle's last mirror 0x12, Dark Meta Knight 0x13), which lights the level's mirror in the NiDL hub and the
         * one its switch room shows above the switch (object 0x8C), and its door on the world map (KatAMExpansionWorldMap.cs);
         * LevelLinks: where that mirror leads, as its path on the world map (worldmap.c's gNidlWorldMapDoors): a KatAM mini hub, another
         * level's switch room (Nidl(level)) or the NiDL hub (Hub, onto the level's door there); the first two get a mirror back, onto
         * which Kirby arrives (KatAMLinkDoors: off their big switch). Chains of at most 2 links, a few to KatAM rooms:
         * Vegetable Valley to Rainbow Route East (0x316), Ice Cream Island to the Moonlight Mansion door (0xBE), Butter Building to Rainbow
         * Resort to the hub, Grape Garden to Peppermint Palace East (0x122), Yogurt Yard to Cabbage Cavern West (0x250), Orange Ocean and
         * the Fountain of Dreams to Cabbage Cavern East (0xBF) (the KatAM rooms of KatAM's world map doors: sWorldMapDoorRoomIds);
         * Door designs (object 0x97's subtype, its frame: CreateDoorToHub; KatAM shows the design of where a door leads, KatAMDesigns):
         * each level has one KatAM's Central Circle doesn't (NidlDesigns), for its door in the NiDL hub and the doors leading to it; */
        static int LevelSlot(int level) => 0x14 + level;
        const int Hub = -1;
        static int Nidl(int level) => -2 - level;
        static readonly int[] LevelLinks = { 0x316, 0xBE, Nidl(6), 0x122, 0x250, 0xBF, Hub, 0xBF };
        /* KatAMLinkDoors: where, in the KatAM mini hub a level's mirror leads to, its way back stands, and where Kirby lands (as KatAM's
         * paths: both rooms get a mirror once the big switch is pressed): a door (object 0x97, the level's slot, in the level's design;
         * FinishRooms()) as KatAM's hub doors, on the top row (0x316 and 0xBE left; 0x122 in the middle, Kirby landing on the floor, off
         * its big switch) or on the lower floor (0x250, 0xBF, beside their own); Butter Building's leads to Rainbow Resort's switch room,
         * onto its SwitchLinkDoor. The top rows stay symmetric, as KatAM's own (0x122: doors at cells 3 and 12, statues between):
         * 0x316 and 0xBE take 0x122's layout, Central Circle's door moving from the middle to cell 12 (MovedDoors: where it leads stays,
         * and the doors landing on it land on it there) and their statues between (DecorationMoves); 0x122's door stands in the
         * middle of the room (DoorDrawX: its cell 8, drawn at x 128, between its statues). 0x250's lower floor takes 0xBF's: its own
         * door (to Mustard Mountain's hub) moves from under its big switch to cell 11, the level's at cell 5; */
        static readonly ((int x, int y) door, (int x, int y) landing)[] KatAMLinkDoors = {
            ((3, 3), (3, 3)), ((3, 3), (3, 3)), ((0, 0), (0, 0)), ((8, 3), (4, 8)), ((5, 8), (5, 8)), ((5, 8), (5, 8)), ((0, 0), (0, 0)), ((11, 8), (11, 8)),
        };
        static readonly Dictionary<int, (byte type, int fromX, int toX)[]> DecorationMoves = new Dictionary<int, (byte, int, int)[]> {
            [0x316] = new[] { ((byte)0x92, 64, 96), ((byte)0x92, 200, 160) }, [0xBE] = new[] { ((byte)0x92, 64, 96), ((byte)0x92, 200, 160) },
        };
        static readonly Dictionary<int, (int x, int y, int toX, int toY)> MovedDoors = new Dictionary<int, (int, int, int, int)> {
            [0x316] = (8, 3, 12, 3), [0xBE] = (8, 3, 12, 3), [0x250] = (8, 8, 11, 8),
        };
        // (the rooms whose doors land on a moved door, so FinishRooms() moves their landings too: Central Circle's, Mustard Mountain's hub);
        static readonly int[] LandingRooms = { 0x321, 0x2B2 };
        static readonly Dictionary<int, int> DoorDrawX = new Dictionary<int, int> { [0x122] = 128 };
        // LandsOn(); Whether a landing (x, y: Kirby at x * 16 - 8, y * 16 - 7) is on a moved door (within a cell: KatAM's hub doors land
        // Kirby on their cell, 9, 3 on 8, 3, or beside it, 8, 8 on 8, 8); it moves with it;
        static bool LandsOn(int x, int y, (int x, int y, int toX, int toY) door) => Math.Abs(x - 1 - door.x) <= 1 && Math.Abs(y - 1 - door.y) <= 1;
        // (and none the KatAM rooms its doors stand in have: Yogurt Yard's, in 0x250 beside its door to Mustard Mountain's hub, 5, isn't);
        static readonly int[] NidlDesigns = { 11, 3, 5, 7, 13, 2, 6, 1 };
        static readonly Dictionary<int, int> KatAMDesigns = new Dictionary<int, int> {
            [0x316] = 14, [0x2B2] = 5, [0x24F] = 7, [0xC1] = 13, [0x123] = 6, [0x37A] = 11, [0x186] = 2, [0x122] = 15, [0xC0] = 8, [0xC2] = 10,
            [0x250] = 4, [0xBF] = 12, [0x24E] = 16, [0x1EA] = 0, [0xBE] = 9,
        };
        const int HubDesign = 17;
        static int LinkDesign(int link) => link == Hub ? HubDesign : link < 0 ? NidlDesigns[-2 - link] : KatAMDesigns.GetValueOrDefault(link, HubDesign);
        const byte BigSwitchType = 0x86, HubMirrorType = 0x8C;

        /* Switch rooms: a level's entry, as KatAM's switch rooms (0x24F, 0x316, 0xC1...: one screen walled on top, a pedestal in the
         * middle with the big switch on it, a mirror on each side's floor) in the level's tiles (see BuildSwitchRooms()): the way in a
         * Mirra hidden in the level (PlanDoors(), SecretSpot(): as KatAM's, found by exploring, not on the way), one-way; its 2 floor
         * mirrors (SwitchSides) its 2 ways into the level, both ways, as KatAM's hubs branch: the right one with the start of the
         * level's stage before its boss, the left one with another branch's; its 2 hub doors, as KatAM's rooms with 2 (LayoutB()), show once its big switch is pressed: SwitchLinkDoor to
         * where LevelLinks says, SwitchHubDoor back to the NiDL hub (as KatAM's mini hubs' to Central Circle; Kirby arrives on it from
         * the hub; Rainbow Resort's link is the hub's, so there, and its SwitchLinkDoor leads back to Butter Building's); Kirby starts
         * at SwitchArrival; */
        const int SwitchStage = 8, SwitchWidth = 16, SwitchHeight = 12, SwitchFloor = 9;
        // LayoutB(); KatAM's two layouts of a room with 2 hub doors: 0x122's (A: top left and right) or 0x250's (B: top and bottom middle);
        // (the Fountain of Dreams' A, though odd: no platform in the middle, the Fountain behind it shown whole);
        static bool LayoutB(int level) => level % 2 == 1 && level != 7;
        // (the Fountain of Dreams' inside its pillars, which run floor to ceiling: SwitchPillared);
        static (int x, int y)[] SwitchSides(int level) => LayoutB(level) ? new[] { (1, 6), (14, 6) } : SwitchPillared(level) ? new[] { (3, 8), (12, 8) } : new[] { (1, 8), (14, 8) };
        static bool SwitchPillared(int level) => level == 7 && Environment.GetEnvironmentVariable("SWITCH_ROOM_PILLARED") != null;
        static (int x, int y) SwitchLinkDoor(int level) => LayoutB(level) ? (8, 8) : (3, 3);
        static (int x, int y) SwitchHubDoor(int level) => LayoutB(level) ? (8, 3) : (12, 3);
        static (int x, int y) SwitchAt(int level) => LayoutB(level) ? (128, 96) : (128, 128);
        static (int x, int y) SwitchArrival(int level) => LayoutB(level) ? (3, 6) : (3, 8);
        /* SwitchStatues: the ability statues on each switch room's top row, as KatAM's hub rooms' (objects 0x92: Beam, Burning, Laser,
         * Fire, Stone, Fighter, Bomb, Tornado; 0x93: Wheel, Ice, Spark, Missile, Smash, Throw, Hammer; 0x94: Parasol; 0x95: Sword,
         * Cutter, Cupid: subtype1), with the level's abilities: Vegetable Valley Beam (Waddle Doo) and Sword (Sword Knight), Ice Cream
         * Island Parasol and Fire (the beach's), Butter Building Cutter (Sir Kibble) and Hammer, Grape Garden Tornado (Twister) and
         * Spark (Sparky), Yogurt Yard Stone and Throw, Orange Ocean Laser (Laser Ball) and Wheel, Rainbow Resort Ice (Chilly) and
         * Cupid, the Fountain of Dreams Smash and Burning; between the hub doors (layout A, as 0x122's) or beside the top one (B, 0x250's); */
        static readonly (byte type, byte subtype)[][] SwitchStatues = {
            new[] { ((byte)0x92, (byte)0), ((byte)0x95, (byte)0) }, new[] { ((byte)0x94, (byte)0), ((byte)0x92, (byte)3) },
            new[] { ((byte)0x95, (byte)1), ((byte)0x93, (byte)6) }, new[] { ((byte)0x92, (byte)7), ((byte)0x93, (byte)2) },
            new[] { ((byte)0x92, (byte)4), ((byte)0x93, (byte)5) }, new[] { ((byte)0x92, (byte)2), ((byte)0x93, (byte)0) },
            new[] { ((byte)0x93, (byte)1), ((byte)0x95, (byte)2) }, new[] { ((byte)0x93, (byte)4), ((byte)0x92, (byte)1) },
        };
        static int[] SwitchStatueXs(int level) => LayoutB(level) ? new[] { 64, 208 } : new[] { 96, 160 };

        /* The NiDL hub (see KatAMExpansionHub.cs): Central Circle again, at night; as there, its golden mirror (MainHubEntry, Central
         * Circle's to Rainbow Route) is the way in: to Vegetable Valley's first room (onto a mirror back, where Kirby starts it), from
         * which every room is reached (each level's way into the next; a boss's shard leads back here); a pink one on the platform's left end
         * (MainHubReturn) leads back to Central Circle's, onto which Kirby arrives; each level has one of Central
         * Circle's own doors (MainHubDoors: its door cells), recolored for the level's theme, hidden until the level's big switch is
         * pressed, as KatAM's: Vegetable Valley the trees', Ice Cream Island the waves', Butter Building the stone pillars', Grape
         * Garden the purple one, Yogurt Yard the rocks', Orange Ocean the blue blocks', Rainbow Resort the ice one, the Fountain of
         * Dreams the starry one; */
        static readonly (int x, int y)[] MainHubDoors = { (6, 12), (25, 17), (15, 4), (28, 11), (15, 18), (5, 17), (2, 11), (27, 4) };
        static readonly (int x, int y) MainHubEntry = (19, 14), MainHubReturn = (11, 14);

        /* NightmareRoom; A NiDL room: level, stage, index, record (file offset); its part when split
         * (X, Y, Width, Height in metatiles), and the KatAM room it becomes; */
        public sealed record NightmareRoom(int Level, int Stage, int Index, int Record, int X, int Y, int Width, int Height) {
            public int Room { get; set; }
            public bool IsSwitchRoom => Stage == SwitchStage;
            public int Area => AreaBase + Level;
            // The room as KatAM holds it (see Padded());
            public int FullWidth => Padded(Width, Height).width;
            public int FullHeight => Padded(Width, Height).height;
        }

        /* Padded(); A room's size in KatAM: KatAM's camera never shows a room's bottom 40 pixels nor 8 on its other sides (its rooms
         * hide that much past their edges), so NiDL's rooms get their last row repeated HiddenRows times below (one: the camera stops
         * 24 pixels above the NiDL room's bottom, so the repeated row never shows), and their last column and row repeated up to a
         * screen (256 x 208: smaller rooms push the camera off them; 11 rows, a screen in NiDL: the camera stops 8 pixels above); */
        static (int width, int height) Padded(int width, int height) => (Math.Max(width, MinWidth), Math.Max(height + HiddenRows, MinHeight));
        const int HiddenRows = 1, MinWidth = 16, MinHeight = 13;

        /* OneWayEntrances; Doors the owner wants one-way (level, stage, room index as NiDL numbers them): from the first room into the
         * second, with no door back (PlanDoors()). Ice Cream Island's second stage, its second room into its tall third (0xEC, 0xED); */
        static readonly ((int level, int stage, int index) first, (int level, int stage, int index) second)[] OneWayEntrances = {
            ((1, 1, 1), (1, 1, 2)),
        };

        /* Shown(); Whether the camera shows a whole mirror on a room's row: it's 2 cells tall, on its cell and the one above, and KatAM's
         * camera shows from 8 pixels below a room's top to 40 above its bottom (rows MirrorTop to FullHeight - 4); */
        const int MirrorTop = 2, MirrorSpacing = 4, ReturnReach = 10, LayoutSeed = 0x4E49444C, KatAMLinks = 2, ViewWidth = 15, ViewHeight = 10;
        // KatAM's goal games (goal_game_bonus.c: their mirrors, object 0x6F, show as goal mirrors for doors to rooms 0x3D4-0x3DD,
        // mirror_door.c), and where KatAM's goal mirrors land Kirby in them (56, 1176: cells + 1); done, the game takes him to Central
        // Circle;
        static readonly int[] GoalRooms = { 0x3D4, 0x3D5, 0x3D6 };
        static readonly (int x, int y) GoalLanding = (4, 74);
        static bool Shown(NightmareRoom r, int y) => y >= MirrorTop && y <= r.FullHeight - 4;

        public static IReadOnlyList<NightmareRoom> PortedRooms { get; private set; } = new List<NightmareRoom>();
        public static IReadOnlyDictionary<NightmareRoom, List<Door>> PortedDoors { get; private set; } = new Dictionary<NightmareRoom, List<Door>>();
        // The NiDL hub's room and doors (see BuildMainHub());
        public static (int Room, IReadOnlyList<Door> Doors) PortedMainHub { get; private set; } = (0, new List<Door>());
        /* For the randomizer, which reads the expanded ROM's items and chests as KatAM's: the ported rooms' items and chests (room, template
         * address; none in a boss room, a switch room or a mini-boss's fight area), and where the doors lists' table went (-1: not moved,
         * no rooms ported); chestLists: each such room's doors list (its table entry) and objects, for FinishRooms(); */
        static List<(int room, int address)> portedItems = new List<(int, int)>();
        public static IReadOnlyList<(int room, int address)> PortedItems => portedItems;
        public static int DoorsTable { get; private set; } = -1;
        static List<(int room, int list, int objects)> chestLists = new List<(int, int, int)>();

        // ForgetRooms(); Before a ROM is made: nothing ported yet (what the last ROM's expansion left);
        public static void ForgetRooms() {
            portedItems = new List<(int, int)>();
            chestLists = new List<(int, int, int)>();
            chestsOf = new Dictionary<NightmareRoom, List<NidlChest>>();
            DoorsTable = -1;
        }

        /* NightmareRooms(); NiDL's level rooms, split where KatAM can't hold them whole. Left out: the levels' hub lists (hub, museum,
         * arena, boss tower, boss room: no gameplay of their own), level 7's stage 1 (the boss rush's copies of the boss rooms) and
         * stage 2 (unused test rooms), rooms with no exits nor objects outside the boss stages (empty floors no door leads to), and a
         * boss stage's rooms after its last boss (NiDL's empty walks out after the fight: a boss's shard takes Kirby to the NiDL hub,
         * so nothing leads there; Shine & Bright's two, Heavy Mole's, Nightmare's); */
        static List<NightmareRoom> NightmareRooms(byte[] nightmare, int table) {
            List<(int level, int stage, int index, int record)> records = new List<(int, int, int, int)>();

            for (int level = 0; level < 8; level++) {
                for (int stage = 0; stage < (level == 7 ? 1 : 8); stage++) {
                    int list = ReadInt(nightmare, table + level * 0x20 + stage * 4);
                    if (list == 0) continue;
                    list -= 0x08000000;

                    for (int index = 0; index < 80; index++) {
                        int record = ReadInt(nightmare, list + index * 4);
                        if (record == 0) continue;
                        if ((record >> 24) != 8 || ReadShort(nightmare, (record & 0x1FFFFFF) + 2) != index) break;
                        records.Add((level, stage, index, record & 0x1FFFFFF));
                    }
                }
            }

            // (a room with a boss NiDL's spawn is ported for: RoomBosses);
            bool Boss(int record) {
                int list = ReadInt(nightmare, record + 0x48);
                for (int at = list == 0 ? record : list - 0x08000000; list != 0 && at + 8 <= record; at += 8)
                    if (nightmare[at] - 1 == BossCategory && RoomBosses.ContainsKey(nightmare[at + 1])) return true;
                return false;
            }
            List<NightmareRoom> rooms = new List<NightmareRoom>();
            foreach ((int level, int stage, int index, int record) in records) {
                int width = ReadShort(nightmare, record + 0x14), height = ReadShort(nightmare, record + 0x16);
                if (width <= 2 && height <= 2) continue;
                bool bossStage = level == 7 || stage == records.Where(r => r.level == level).Max(r => r.stage);
                if (bossStage && !Boss(record) && records.Any(r => r.level == level && r.stage == stage && r.index < index && Boss(r.record))) continue;
                if (!bossStage && ReadInt(nightmare, record + 0x44) == 0 && ReadInt(nightmare, record + 0x48) == 0) continue;

                // Split along the longer side into the fewest parts that fit (padded, see Padded());
                bool across = width >= height;
                int length = across ? width : height, parts = 1;
                int Blocks(int part) {
                    (int w, int h) = across ? Padded((length + part - 1) / part, height) : Padded(width, (length + part - 1) / part);
                    return w * h;
                }
                while (Blocks(parts) > MaxRoomBlocks) parts++;
                for (int part = 0; part < parts; part++) {
                    int start = length * part / parts, end = length * (part + 1) / parts;
                    rooms.Add(across ? new NightmareRoom(level, stage, index, record, start, 0, end - start, height)
                                     : new NightmareRoom(level, stage, index, record, 0, start, width, end - start));
                }
            }
            return rooms;
        }

        /* BuildSwitchRooms(); The switch rooms, as NiDL rooms appended to a copy of the NiDL ROM (records whose cells are made here, the
         * rest the first room's with the level's own tileset (its first stage's, or its most solid where that's mostly sky: foreground
         * tiles, palette, metatiles, background), so they're converted as the others: per level, cells copied from its stages (below),
         * laid out as KatAM's rooms with 2 hub doors, symmetric (rows 0-11, the floor's top on row 9 and ground under it, so the padding
         * (to KatAM's 13 rows) repeats ground, not the floor's top; L, H: SwitchLinkDoor, SwitchHubDoor; P: the pedestal, the big switch
         * on it, and ledges; M: the floor mirrors):
         *   A (0x122's)           B (0x250's)
         *   ################      ################      0
         *   ##.L........H.##      ##.....H......##      1-3
         *   ##............##      #..............#      4
         *   ##............##      #..............#      5
         *   ................      #M..PPPPPPPP..M#      6
         *   ................      ####........####      7
         *   .M...PPPPPP...M.      ####....L...####      8
         *   ################      ################      9-11 */
        // ModelRanks; Per level, the room its switch room is drawn after (see BuildSwitchRooms()), by rank: 0 the one in its colours
        // with the most of its shapes;
        static readonly int[] ModelRanks = { 0, 0, 0, 0, 4, 0, 0, 0 };
        /* SwitchDesigns; Switch rooms picked by eye, kept as they are whatever BuildSwitchRooms() would draw now: per level, the left
         * half's metatiles, row by row (~: flipped left to right, ^: upside down, @: both), the right half its mirror image; the same
         * metatiles in every region (NiDL's tilesets are); Rainbow Resort's in its Game Boy tiles (SwitchLook), after its last stage's fifth room, its floor (flower
         * grass) after the mix of all its rooms;
         * Grape Garden's after its second stage's fifth room, no ledges (NoLedges); the Fountain of
         * Dreams' in FountainLook's castle, the castle's circle pillars as its walls'
         * inner faces; Butter Building's after its sixth stage's eighth room, its floor after its first stage's
         * third; Ice Cream Island's after its third stage's third room (its grass ends curling under as its
         * stages', the ledges and pedestal one floating platform with rounded ends); Vegetable Valley's built from its raised hedges (its stages have no
         * ceilings: their rounded tops upside down, their sides and the joins at their feet turned round), its first stage's
         * pedestal, its second stage's dirt on the side floors, dressed as its stages (grass along the floors, flower beds,
         * white tusks framing the big switch); Yogurt Yard's after its sixth stage's second room, inside its rock the fills (and moss) joining best
         * (least colour across their edges), Orange Ocean's after its fifth stage's first; */
        static readonly Dictionary<int, string[]> SwitchDesigns = new Dictionary<int, string[]> {
            [6] = new[] {
                "~163 0F5 ~0ED ~0ED ~0ED ~0ED ~0ED ~0ED",
                "~163 0EE 01E 01E 01E 01E 01E 01E",
                "~163 0EE 01E 01E 01E 01E 01E 01E",
                "~163 0EE 01E 01E 01E 01E 01E 01E",
                "~163 0EE 01E 01E 01E 01E 01E 01E",
                "~0ED 0F1 01E 01E 01E 01E 01E 01E",
                "01E 01E 01E 01E 01E 01E 01E 01E",
                "01E 01E 01E 01E 01E 01E 01E 01E",
                "01E 01E 01E 01E 01E ~002 ~004 ~004",
                "~004 ~004 ~004 ~004 ~004 ~02E ~005 ~005",
                "~005 ~005 ~005 ~005 ~005 ~02F ~005 ~005",
                "~02E ~02E ~02E ~02E ~02E ~035 ~02E ~02E",
            },
            [7] = new[] {
                "001 090 00B 00A 00B 00A 00B 00A",
                "001 014 000 000 000 000 000 000",
                "001 013 000 000 000 000 000 000",
                "001 016 000 000 000 000 000 000",
                "001 012 000 000 000 000 000 000",
                "00B 00F 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 006 003 002",
                "003 002 003 002 003 08F 001 001",
                "001 001 001 001 001 001 001 001",
                "001 001 001 001 001 001 001 001",
            },
            [0] = new[] {
                "087 @0A6 ^091 ^091 ^091 ^091 ^091 ^091",
                "087 09F 000 000 000 000 000 000",
                "087 09F 000 000 000 000 000 000",
                "087 09F 000 000 000 000 000 000",
                "087 09F 000 000 000 000 000 000",
                "^091 ^09E 000 000 000 02A 027 000",
                "000 000 000 000 000 029 026 000",
                "000 000 000 000 000 028 025 01C",
                "01B 01A 020 021 022 ~09E ~002 ~001",
                "001 036 037 038 03B ~0B1 ~087 ~087",
                "087 041 04A 04A 051 ~087 ~087 ~087",
                "087 08B 005 005 005 ~08A ~087 ~06B",
            },
            [1] = new[] {
                "009 @017 0FA 044 0FA 044 0FA 044",
                "007 010 000 000 000 000 000 000",
                "009 011 000 000 000 000 000 000",
                "003 005 000 000 000 000 000 000",
                "010 000 000 000 000 000 000 000",
                "010 000 000 000 000 000 000 000",
                "011 000 000 000 0B3 0B2 0B2 0B2",
                "002 004 002 156 000 000 000 000",
                "0ED 0EC 0ED 0F0 000 000 000 000",
                "009 008 009 157 004 002 002 002",
                "007 006 007 0ED 0EC 0ED 0EC 0ED",
                "009 008 009 008 008 009 008 009",
            },
            [2] = new[] {
                "~01C 09D 09C 09C 09C 09C 09C 09C",
                "~01C ~0B2 000 000 000 000 000 000",
                "~01C ~0AA 000 000 000 000 000 000",
                "~01C ~0AA 000 000 000 000 000 000",
                "~01C ~0AA 000 000 000 000 000 000",
                "09C 09B 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 ~0CA 09F 09E",
                "09F 09E 09F 09E 09F 09D ~01B ~01C",
                "~01B ~01C ~01B ~01C ~01B ~01C ~0CE ~0CF",
                "~0CE ~0CF ~0CE ~0CF ~0CE ~0CF ~0CE ~0CF",
            },
            [3] = new[] {
                "001 090 00B 00A 00B 00A 00B 00A",
                "001 013 000 000 000 000 000 000",
                "001 016 000 000 000 000 000 000",
                "090 00F 000 000 000 000 000 000",
                "013 000 000 000 000 000 000 000",
                "016 000 000 000 000 000 000 000",
                "012 000 000 000 000 000 078 01E",
                "003 002 003 004 000 000 000 000",
                "001 001 001 008 000 000 000 000",
                "001 001 001 003 002 003 002 003",
                "001 001 001 001 001 001 001 001",
                "001 001 001 001 001 001 001 001",
            },
            [4] = new[] {
                "16E 002 01E 059 020 021 022 059",
                "17A 0BB 000 000 000 000 000 000",
                "063 08C 000 000 000 000 000 000",
                "090 08D 000 000 000 000 000 000",
                "1D6 08E 000 000 000 000 000 000",
                "01E 0AF 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 000 000 000",
                "000 000 000 000 000 1F9 1DF 1DF",
                "093 094 2E3 1DF 1E0 1C6 063 31E",
                "240 12C 31E 063 31E 17A 318 1DC",
                "17A 318 001 318 22C 254 327 324",
            },
            [5] = new[] {
                "008 1C6 101 100 044 0FA 044 0FA",
                "006 010 000 000 000 000 000 000",
                "008 00D 000 000 000 000 000 000",
                "1C5 0FD 000 000 000 000 000 000",
                "00D 000 000 000 000 000 000 000",
                "00D 000 000 000 000 000 000 000",
                "0EB 000 000 000 0B3 0B4 18B 04D",
                "157 003 004 156 000 000 000 000",
                "006 0EC 0ED 0EB 000 000 000 000",
                "008 008 009 157 003 004 138 004",
                "007 006 007 006 0EC 0ED 145 0ED",
                "009 008 009 008 008 009 008 009",
            },
        };
        /* SwitchDecorations; What stands about a switch room by eye, as its stages dress theirs, each side its own (the terrain is
         * symmetric, its dressing needn't be): blocks "x y rows" (the block's top left cell, its metatiles row by row, / between
         * rows, 000 nothing, ~ ^ @ flips as SwitchDesigns'), put in air cells after the room is mirrored (! first: over the terrain
         * too, its collision kept: flowers in grass, a side's own texture); */
        static readonly Dictionary<int, string[]> SwitchDecorations = new Dictionary<int, string[]> {
            // (Ice Cream Island's: red flowers in the grass on the left, blue on the right, sand hills in the pit, a palm leaning in
            // from the right);
            // (the Fountain of Dreams': King Dedede's arena's light, lines under the ceiling and streaks, a cyan one left, a pink one
            // right, its pieces, ForeignDecorations);
            [7] = new[] {
                "2 1 *111 *110 *10F",
                "11 1 *111 *110 *10F",
                "3 2 *14C/*14B/*0EC/*14B/*14C",
                "12 3 *14F/*146/*146/*0F7/*0F8/*300",
            },
            // (Rainbow Resort's: the Game Boy forest of its last stage's second room behind it (its columns 6-21: canopy and trunks),
            // a pale trunk each side, the left one the right one's mirror image, thin dark trunks at x6 and x10,
            // two rows of grass, tall and leafy, down to the floor);
            [6] = new[] {
                "0 1 038 038 ~05A ~05D 038 05C 038 061 038 05E 05A 038 05D 05A 060 038/05A 038 ~05E ~05E 038 038 05D 060 038 05B 038 061 05E 05E 05E 05F/03C 03E ~03C ~053 03C 042 05E 05E 05E 05A 05B 05C 053 03C 042 038/04E 03C 03F 041 04E 04E 042 03C 042 053 05E 03C 03F 041 04E 042/03F 052 ~048 ~047 ~03F 052 04E 03F 041 04E 03E 052 047 048 052 04E/~041 01E ~048 ~047 01E 018 ~04F 01E 017 041 04F 01E 047 048 01E 041/~054 ~054 ~048 ~047 054 054 ~055 054 054 054 055 054 047 048 054 054/~059 ~059 ~048 ~047 059 059 04C 059 059 059 04C 059 047 048 059 059",
            },
            // (Grape Garden's: its first stage's castle keep behind the pedestal, from its spire to its walls, its clouds on the side
            // floors, ForeignDecorations; the clouds first, their colours kept, the keep's least used colours sharing the nearest row);
            [3] = new[] {
                "1 6 *007 *008 *009",
                "12 6 *003 *004 *005",
                "3 1 000 000 000 000 *076 *077 000 000 000 000/000 000 000 *075 *08E *08F *078 000 000 000/000 000 *074 *08A *08B *08C *08D *079 000 000/000 *073 *084 *085 *086 *087 *088 *089 *07A 000/*072 *07C *07D *07E *07F *080 *081 *082 *083 *07B/*042 *043 *044 *045 *046 *047 *048 *049 *04A *071/*037 *03C *03F *03F *051 *051 *03F *03F *04C *035/*033 *03D *04B *03B *052 *052 *03B *03B *04D *038",
            },
            // (Butter Building's: its first stage's skyline as it is there, its columns 14-25, 2 cells right of the right wall's face
            // (its tall tower going on behind the wall), the walkways across its towers left out (their walls behind instead), the
            // small tower's piece left of the pale pillar left out);
            [2] = new[] {
                "4 1 000 000 000 000 000 000 000 000 01A 019 015 016/000 000 000 000 031 000 000 000 018 017 013 014/000 000 000 000 030 000 000 000 012 011 091 08C/000 000 000 000 02E 000 01A 019 012 092 08D 04A/000 000 000 000 036 000 018 017 012 011 091 08C/000 000 000 000 037 019 012 092 012 039 03A 094/000 000 000 000 036 093 08F 011 012 036 08D 04A/097 096 097 095 036 03D 04D 090 08F 037 03A 08C",
            },
            // (Yogurt Yard's: its brown boulders, each side its own: one clinging to the left wall, a cluster in the right ceiling
            // corner, heaps of different sizes on the floor);
            [4] = new[] {
                "2 3 1B0/1AF/1A4",
                "2 7 000 1B1 1B0/1AD 1AE 1AE",
                "6 7 1B1 1B0",
                "12 1 1CE 1AE/1CF 1D0/000 1D1/000 1D2",
                "11 8 1B1 1B0",
            },
            [1] = new[] {
                "!0 7 138",
                "!2 7 137",
                "!13 7 13B 13D",
                "!5 9 139 138",
                "!9 9 13B",
                "!11 9 13D",
                "5 8 050 054",
                "9 8 058 059 05A",
                "10 2 ~09F ~09E ~09D ~09C/~0A3 ~0A2 ~0A1 ~0A0/000 000 ~094 000/000 000 ~095 000/000 000 ~096 ~097",
            },
        };
        // SwitchExtraGround; Cells of ground a level's switch room has besides its layout's (solid, drawn by its design);
        static readonly Dictionary<int, (int x, int y)[]> SwitchExtraGround = new Dictionary<int, (int, int)[]>();
        // NoLedges; The levels whose switch rooms (layout B) have no ledges beside the pedestal;
        static readonly HashSet<int> NoLedges = new HashSet<int> { 3 };
        // UnmirroredFill; The levels whose switch rooms' ground inside isn't mirrored (see BuildSwitchRooms());
        static readonly HashSet<int> UnmirroredFill = new HashSet<int> { 1 };
        /* ForeignDecorations; Levels whose switch rooms take decorations (SwitchDecorations' * metatiles) from another level's last
         * room: the Fountain of Dreams' from King Dedede's arena (Rainbow Resort's last), its light streaks, Grape Garden's from its
         * castle room; their colour rows go in
         * rows the room's own metatiles don't use (8 on are the arena background's); */
        static readonly Dictionary<int, (int level, int stage, int index)> ForeignDecorations = new Dictionary<int, (int, int, int)> {
            [7] = (6, -1, -1),
            // (Grape Garden's: its first stage's castle room);
            [3] = (3, 1, 0),
        };
        // SwitchLook; The room (stage, index) whose tileset a level's switch room is drawn in, picked by eye: Rainbow Resort's last
        // stage's Game Boy look;
        static readonly Dictionary<int, (int stage, int index)> SwitchLook = new Dictionary<int, (int, int)> { [6] = (5, 0) };
        // FountainLook; The room (level, stage, index) whose tiles and colours the Fountain of Dreams' switch room takes, picked by
        // eye: Grape Garden's castle in its stage 6's dusk colours;
        static readonly (int level, int stage, int index) FountainLook = (3, 5, 5);
        // FountainInBack; Whether the Fountain of Dreams' switch room shows the Fountain behind it (King Dedede's arena's background), or
        // the Fountain of Dreams' nebula; picked by eye;
        const bool FountainInBack = true;
        static List<NightmareRoom> BuildSwitchRooms(ref byte[] nightmare, List<NightmareRoom> rooms) {
            List<NightmareRoom> switchRooms = new List<NightmareRoom>();
            int start = (nightmare.Length + 3) & ~3;
            List<byte> added = new List<byte>();
            int Put(byte[] data) {
                while (added.Count % 4 != 0) added.Add(0);
                int at = start + added.Count;
                added.AddRange(data);
                return at;
            }

            for (int level = 0; level < 8; level++) {
                // (its scenery, as KatAM's hubs are built of their area's own walls, not its skies; its first room with that tileset);
                byte[] nidl0 = nightmare;
                // (a tileset: its tiles and metatiles, +0x1C and +0x20, whatever its palette; how solid: the share of its rooms' cells
                // that are ground, so a castle's walls beat a sky's few clouds);
                string Tileset(NightmareRoom r) => $"{ReadInt(nidl0, r.Record + 0x1C)},{ReadInt(nidl0, r.Record + 0x20)}";
                (int ground, int cells) Ground(int record) {
                    byte[] c = DecompressLZ77(nidl0, ReadInt(nidl0, record + 8) - 0x08000000);
                    int count = 0;
                    for (int i = 3; i < c.Length; i += 4) if (RoomCollision[c[i]] == 0x0D) count++;
                    return (count, c.Length / 4);
                }
                double Solidness(IEnumerable<NightmareRoom> group) {
                    var counts = group.Select(r => r.Record).Distinct().Select(Ground).ToList();
                    return counts.Sum(c => c.ground) / (double)Math.Max(1, counts.Sum(c => c.cells));
                }
                // (the first stage's tileset, the level's own look: Vegetable Valley's grass, 28% ground; one mostly sky, under 27%:
                // Grape Garden's clouds 25%, Yogurt Yard's and Rainbow Resort's skies: instead the one in 2 rooms or more most solid and most
                // used, its share of ground times the root of its rooms: Grape Garden's castle, Yogurt Yard's purple caves);
                // (the Fountain of Dreams': FountainLook's tileset and colours, learnt from every room with that tileset, under the
                // Fountain of Dreams' own nebula or the Fountain itself, below);
                NightmareRoom pastel = rooms.First(r => (r.Level, r.Stage, r.Index) == FountainLook);
                if (Environment.GetEnvironmentVariable("SWITCH_ROOM_WHERE") is string where && level == 0)
                    foreach (var r in rooms.Where(r => r.Record == Convert.ToInt32(where, 16))) Console.WriteLine($"WHERE {r.Level} {r.Stage} {r.Index}");
                if (Environment.GetEnvironmentVariable("SWITCH_ROOM_FOUNTAIN") is string look)
                    pastel = rooms.First(r => r.Record == Convert.ToInt32(look, 16));
                NightmareRoom fountain = rooms.Where(r => r.Level == 7).OrderBy(r => r.Stage).ThenBy(r => r.Index).First();
                var levelRooms = level == 7 ? rooms.Where(r => Tileset(r) == Tileset(pastel)).ToList()
                                            : rooms.Where(r => r.Level == level).ToList();
                // (or SwitchLook's room's, picked by eye: Rainbow Resort's, its last stage's Game Boy look);
                bool looked = SwitchLook.TryGetValue(level, out var lookRoom) && Environment.GetEnvironmentVariable($"SWITCH_ROOM_NO_LOOK_{level}") == null;
                NightmareRoom first = level == 7 ? pastel : looked ? rooms.First(r => r.Level == level && r.Stage == lookRoom.stage && r.Index == lookRoom.index)
                                                                 : levelRooms.OrderBy(r => r.Stage).ThenBy(r => r.Index).First();
                var tilesets = levelRooms.GroupBy(Tileset).ToList();
                var chosen = tilesets.First(g => g.Key == Tileset(first));
                int Rooms(IEnumerable<NightmareRoom> group) => group.Select(r => r.Record).Distinct().Count();
                if (Solidness(chosen) < 0.27 && level != 7 && !looked)
                    chosen = tilesets.Where(g => Rooms(g) >= 2).DefaultIfEmpty(chosen).OrderByDescending(g => Solidness(g) * Math.Sqrt(Rooms(g))).First();
                NightmareRoom source = level == 7 ? pastel : chosen.OrderBy(r => r.Stage).ThenBy(r => r.Index).First();
                byte[] sourceCells = DecompressLZ77(nightmare, ReadInt(nightmare, source.Record + 8) - 0x08000000);
                int sourceWidth = ReadShort(nightmare, source.Record + 0x14), sourceHeight = ReadShort(nightmare, source.Record + 0x16);
                byte Kind(int x, int y) => RoomCollision[sourceCells[(y * sourceWidth + x) * 4 + 3]];
                // The most common metatile of the cells (with a cell above) that match;
                int Common(Func<byte, byte, bool> match, int fallback) {
                    Dictionary<int, int> counts = new Dictionary<int, int>();
                    for (int y = 1; y < sourceHeight; y++) {
                        for (int x = 0; x < sourceWidth; x++) {
                            if (!match(Kind(x, y), Kind(x, y - 1))) continue;
                            int metatile = ReadShort(sourceCells, (y * sourceWidth + x) * 4);
                            counts[metatile] = counts.GetValueOrDefault(metatile) + 1;
                        }
                    }
                    return counts.Count == 0 ? fallback : counts.MaxBy(c => c.Value).Key;
                }
                int air = Common((kind, above) => kind == 0x00, 0);
                int top = Common((kind, above) => kind == 0x0D && above == 0x00, -1);
                int ground = Common((kind, above) => kind == 0x0D && above == 0x0D, top);
                if (top < 0) top = ground < 0 ? air : ground;
                if (ground < 0) ground = top;
                // The pedestal's blocks: the most common solid metatile with air above and below (floating blocks), or the floor's top;
                Dictionary<int, int> blocks = new Dictionary<int, int>();
                for (int y = 1; y + 1 < sourceHeight; y++)
                    for (int x = 0; x < sourceWidth; x++)
                        if (Kind(x, y) == 0x0D && Kind(x, y - 1) == 0x00 && Kind(x, y + 1) == 0x00) {
                            int metatile = ReadShort(sourceCells, (y * sourceWidth + x) * 4);
                            blocks[metatile] = blocks.GetValueOrDefault(metatile) + 1;
                        }
                int block = blocks.Count == 0 ? top : blocks.MaxBy(b => b.Value).Key;

                /* How the level builds its terrain, copied: the switch room's cells are taken from the level's own rooms with that tileset
                 * (their metatiles the same), each from where the level has the same shape of ground around (5x5 cells: solid, one-way,
                 * air; past a room's edge solid) and the same metatiles as the switch room's cells taken before (beside, above): so
                 * neighbours come from one place in the level, and whole stretches of its walls, floors, corners and fills (Vegetable
                 * Valley's hedges and dirt, Butter Building's towers and brick frames, Grape Garden's beaded edges, Rainbow Resort's ice
                 * caps) carry over as the level draws them, not one metatile over and over; then what stands on its floors (flowers,
                 * crystals: a cell big, alone), on the main floor, off the doors, mirrors and pedestal; */
                const int Air = 0, Earth = 1, OneWay = 2, Other = 3;
                // (plain ground only: NiDL's solid, 0x01, and Rainbow Resort's ice, 0x70; not slopes, water, doors, ladders, blocks);
                int Class(byte raw) => raw == 0x00 ? Air : raw == 0x01 || raw == 0x70 ? Earth : raw == 0x11 ? OneWay : Other;
                byte[] nidl = nightmare;
                var learntRecords = levelRooms.Where(r => Tileset(r) == Tileset(source)).Select(r => r.Record).Distinct().ToList();
                var learnt = learntRecords
                    .Select(record => (cells: DecompressLZ77(nidl, ReadInt(nidl, record + 8) - 0x08000000), w: ReadShort(nidl, record + 0x14),
                                       h: ReadShort(nidl, record + 0x16), same: ReadInt(nidl, record + 0x18) == ReadInt(nidl, source.Record + 0x18))).ToList();
                int SourceClass(int s, int x, int y) => x < 0 || y < 0 || x >= learnt[s].w || y >= learnt[s].h ? Earth : Class(learnt[s].cells[(y * learnt[s].w + x) * 4 + 3]);
                int SourceMetatile(int s, int x, int y) => x < 0 || y < 0 || x >= learnt[s].w || y >= learnt[s].h ? -1 : ReadShort(learnt[s].cells, (y * learnt[s].w + x) * 4);

                // (the metatile of the tileset drawing a metatile's pixels flipped, its mirror image, for the room's other half, below);
                byte[] tiles = DecompressLZ77(nidl, ReadInt(nidl, source.Record + 0x1C) - 0x08000000);
                byte[] metatiles = DecompressLZ77(nidl, ReadInt(nidl, source.Record + 0x20) - 0x08000000);
                string Pixels(int m, bool flip) {
                    char[] pixels = new char[256];
                    for (int py = 0; py < 16; py++)
                        for (int px = 0; px < 16; px++) {
                            int x = flip ? 15 - px : px, e = ReadShort(metatiles, m * 8 + (py / 8 * 2 + x / 8) * 2), tile = e & 0x3FF;
                            int tx = (e & 0x400) != 0 ? 7 - x % 8 : x % 8, ty = (e & 0x800) != 0 ? 7 - py % 8 : py % 8;
                            int color = tile * 32 + 32 <= tiles.Length ? tiles[tile * 32 + ty * 4 + tx / 2] >> (tx % 2 * 4) & 0xF : 0;
                            pixels[py * 16 + px] = (char)(color == 0 ? 0 : (e >> 12) * 16 + color);
                        }
                    return new string(pixels);
                }
                Dictionary<string, int> drawing = new Dictionary<string, int>();
                for (int m = metatiles.Length / 8 - 1; m >= 0; m--) drawing[Pixels(m, false)] = m;
                // (one drawing nothing, its tiles not in the tileset's (animated), only itself);
                int Mirror(int m) {
                    if (m == air || m < 0 || m >= metatiles.Length / 8) return m == air ? air : -1;
                    string flipped = Pixels(m, true);
                    if (flipped.All(c => c == 0)) return m;
                    return drawing.TryGetValue(flipped, out int image) ? image : -1;
                }
                /* (how the level uses each metatile: how often it has no ground above, below, left, right, and anything but ground,
                 * air and one-way floors 2 cells around, under water, and inside the ground: a grass top goes where there's air above
                 * it as the level draws it, not under more ground where it once drew one under a step; rock drawn under water stays out
                 * of dry ground; deep inside the ground (2 rows under a floor's top) the level's usual fill, so fills are even, not a
                 * patchwork, the row under a floor's top what the level draws there (Vegetable Valley's dirt under its grass); and what most of its rooms
                 * draw, its own terrain, over what one room draws once: Ice Cream Island's sandy cobble, not its first room's strip of
                 * blue under its water); */
                Dictionary<int, double[]> use = new Dictionary<int, double[]>();
                Dictionary<int, HashSet<int>> roomsDrawing = new Dictionary<int, HashSet<int>>();
                for (int s = 0; s < learnt.Count; s++)
                    for (int sy = 0; sy < learnt[s].h; sy++)
                        for (int sx = 0; sx < learnt[s].w; sx++) {
                            if (SourceClass(s, sx, sy) != Earth) continue;
                            int m = SourceMetatile(s, sx, sy);
                            if (!use.TryGetValue(m, out double[] u)) use[m] = u = new double[8];
                            u[0]++;
                            if (!roomsDrawing.TryGetValue(m, out var drawers)) roomsDrawing[m] = drawers = new HashSet<int>();
                            drawers.Add(s);
                            if (SourceClass(s, sx, sy - 1) != Earth) u[1]++;
                            if (SourceClass(s, sx, sy + 1) != Earth) u[2]++;
                            if (SourceClass(s, sx - 1, sy) != Earth) u[3]++;
                            if (SourceClass(s, sx + 1, sy) != Earth) u[4]++;
                            bool near = false;
                            for (int dy = -2; dy <= 2 && !near; dy++)
                                for (int dx = -2; dx <= 2 && !near; dx++) near = SourceClass(s, sx + dx, sy + dy) == Other;
                            if (near) u[5]++;
                            // (under water: water above it in its column, through ground only);
                            int above = sy - 1;
                            while (above >= 0 && SourceClass(s, sx, above) == Earth) above--;
                            if (above >= 0 && learnt[s].cells[(above * learnt[s].w + sx) * 4 + 3] >> 4 == 8) u[6]++;
                            // (inside the ground, the 8 around ground: how often it's the level's fill);
                            if (Enumerable.Range(-1, 3).All(dy => Enumerable.Range(-1, 3).All(dx => SourceClass(s, sx + dx, sy + dy) == Earth))) u[7]++;
                        }
                double mostFill = use.Values.Max(u => u[7]), mostRooms = roomsDrawing.Values.Max(r => r.Count);

                byte[] cells = new byte[SwitchWidth * SwitchHeight * 4];
                void Set(int x, int y, int metatile, byte collision) {
                    WriteShort(cells, (y * SwitchWidth + x) * 4, metatile);
                    cells[(y * SwitchWidth + x) * 4 + 3] = collision;
                }
                // NiDL collision: 0 nothing, 0x01 solid, 0x11 a one-way floor (KatAM's 0x0E);
                bool b = LayoutB(level);
                // (and the level's own extra ground, SwitchExtraGround: Yogurt Yard's stalactite);
                var extra = Environment.GetEnvironmentVariable($"SWITCH_ROOM_EXTRA_{level}") is string triedExtra
                    ? triedExtra.Split(';', StringSplitOptions.RemoveEmptyEntries).Select(c => (int.Parse(c.Split(',')[0]), int.Parse(c.Split(',')[1]))).ToHashSet()
                    : SwitchExtraGround.GetValueOrDefault(level, new (int, int)[0]).ToHashSet();
                // (pillared, the Fountain of Dreams': its walls 3 cells, the outer 2 and a pillar, floor to ceiling, its ceiling a step lower
                // above its top doors, as Grape Garden's castle room 0x3742CC);
                bool pillared = SwitchPillared(level);
                bool Pillared(int x, int y) => pillared && y >= 1 && y <= 8 && (x <= 2 || x >= SwitchWidth - 3 || y == 1 && (x == 3 || x == SwitchWidth - 4));
                bool Solid(int x, int y) => y == 0 || y >= SwitchFloor || Pedestal(x, y) || extra.Contains((x, y)) || Pillared(x, y) || (b
                    ? (y <= 3 && (x <= 1 || x >= SwitchWidth - 2)) || (y >= 4 && (x == 0 || x == SwitchWidth - 1)) || (y >= 7 && (x <= 3 || x >= SwitchWidth - 4))
                    : y <= 5 && (x <= 1 || x >= SwitchWidth - 2));
                bool Pedestal(int x, int y) => b ? y == 6 && x >= 6 && x <= 9 : y == 8 && x >= 5 && x <= 10;
                // (none in NoLedges' rooms: Grape Garden's, its castle shown whole behind);
                bool Ledge(int x, int y) => b && y == 6 && (x == 4 || x == 5 || x == 10 || x == 11) && !NoLedges.Contains(level) && Environment.GetEnvironmentVariable($"SWITCH_ROOM_NO_LEDGES_{level}") == null;
                // (past the room's edges solid, as the level's);
                int TargetClass(int x, int y) => x < 0 || y < 0 || x >= SwitchWidth || y >= SwitchHeight ? Earth : Ledge(x, y) ? OneWay : Solid(x, y) ? Earth : Air;
                for (int y = 0; y < SwitchHeight; y++)
                    for (int x = 0; x < SwitchWidth; x++)
                        Set(x, y, air, (byte)(TargetClass(x, y) == Earth ? 0x01 : TargetClass(x, y) == OneWay ? 0x11 : 0x00));
                int Current(int x, int y) => ReadShort(cells, (y * SwitchWidth + x) * 4);
                bool[] taken = new bool[SwitchWidth * SwitchHeight];

                // (how unlike a cell of the level is to a switch room's: its 5x5 shape, the nearest 8 counting most; the metatiles of the
                // switch room's cells taken, which it should draw beside as the level does; the level's rooms in other colours last);
                // (which metatiles the level draws side by side and one above another: a pair it never draws doesn't join, a wall's
                // face against more rock, a floor's grass running under a wall);
                Dictionary<(int, int), int> beside = new Dictionary<(int, int), int>(), under = new Dictionary<(int, int), int>();
                foreach (var room in learnt)
                    for (int y = 0; y < room.h; y++)
                        for (int x = 0; x < room.w; x++) {
                            int m = ReadShort(room.cells, (y * room.w + x) * 4);
                            if (x + 1 < room.w) beside[(m, ReadShort(room.cells, (y * room.w + x + 1) * 4))] = beside.GetValueOrDefault((m, ReadShort(room.cells, (y * room.w + x + 1) * 4))) + 1;
                            if (y + 1 < room.h) under[(m, ReadShort(room.cells, ((y + 1) * room.w + x) * 4))] = under.GetValueOrDefault((m, ReadShort(room.cells, ((y + 1) * room.w + x) * 4))) + 1;
                        }
                // Seam(); How many of the 16 pixels along the edge a metatile shares with its neighbour (dx, dy) one is drawn and the other not;
                int Seam(int m, int n, int dx, int dy) {
                    if (m < 0 || n < 0 || m >= metatiles.Length / 8 || n >= metatiles.Length / 8) return 0;
                    string a = Pixels(m, false), b = Pixels(n, false);
                    int seam = 0;
                    for (int i = 0; i < 16; i++) {
                        int mine = dx == -1 ? i * 16 : dx == 1 ? i * 16 + 15 : dy == -1 ? i : 240 + i;
                        int theirs = dx == -1 ? i * 16 + 15 : dx == 1 ? i * 16 : dy == -1 ? 240 + i : i;
                        if ((a[mine] == 0) != (b[theirs] == 0)) seam++;
                    }
                    return seam;
                }
                double Cost(int s, int sx, int sy, int x, int y) {
                    double cost = learnt[s].same ? 0 : 6;
                    if (TargetClass(x, y) == Earth && use.TryGetValue(SourceMetatile(s, sx, sy), out double[] u)) {
                        (int dx, int dy)[] sides = { (0, -1), (0, 1), (-1, 0), (1, 0) };
                        for (int i = 0; i < 4; i++) cost += 4 * Math.Abs(u[i + 1] / u[0] - (TargetClass(x + sides[i].dx, y + sides[i].dy) != Earth ? 1 : 0));
                        cost += 10 * (u[5] + u[6]) / u[0] + 5 * (1 - roomsDrawing[SourceMetatile(s, sx, sy)].Count / mostRooms);
                        // (deep inside: the row under a floor's top is where the level draws its dirt, flowers and roots, as it does);
                        bool inside = Enumerable.Range(-2, 4).All(dy => Enumerable.Range(-1, 3).All(dx => TargetClass(x + dx, y + dy) == Earth));
                        if (inside && mostFill > 0) cost += 8 * (1 - Math.Sqrt(u[7] / mostFill));
                    }
                    for (int dy = -2; dy <= 2; dy++)
                        for (int dx = -2; dx <= 2; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            bool near = Math.Abs(dx) <= 1 && Math.Abs(dy) <= 1;
                            if (SourceClass(s, sx + dx, sy + dy) != TargetClass(x + dx, y + dy)) cost += near ? 4 : 1;
                            int nx = x + dx, ny = y + dy;
                            if (!near || nx < 0 || ny < 0 || nx >= SwitchWidth || ny >= SwitchHeight) continue;
                            bool known = taken[ny * SwitchWidth + nx] || TargetClass(nx, ny) == Air;
                            if (!known) continue;
                            if (taken[ny * SwitchWidth + nx]) cost += SourceMetatile(s, sx + dx, sy + dy) == Current(nx, ny) ? -6 : 3;
                            int m = SourceMetatile(s, sx, sy), n = Current(nx, ny);
                            if ((dx == 0 || dy == 0) && Environment.GetEnvironmentVariable("SWITCH_ROOM_LOOSE") == null) {
                                if ((dx == -1 ? beside.GetValueOrDefault((n, m)) : dx == 1 ? beside.GetValueOrDefault((m, n)) : dy == -1 ? under.GetValueOrDefault((n, m)) : under.GetValueOrDefault((m, n))) == 0)
                                    cost += 8;
                                // (no seam: between two ground cells, their facing edges drawn at the same pixels);
                                if (taken[ny * SwitchWidth + nx] && TargetClass(nx, ny) != Air) cost += 0.5 * Seam(m, n, dx, dy);
                            }
                        }
                    return cost;
                }
                // (the level's cells of the class: those shaped as the switch room's cell (its 8 around) where there are);
                /* (one room of the level first, the model: each room has its own look, Vegetable Valley's grass and dirt here, its
                 * hedges there, so the switch room is drawn after the one in its colours with the most of its shapes, other rooms
                 * only for shapes it hasn't); */
                int model = -1;
                bool ShapedAt(int s, int sx, int sy, int x, int y) {
                    for (int dy = -1; dy <= 1; dy++)
                        for (int dx = -1; dx <= 1; dx++)
                            if (SourceClass(s, sx + dx, sy + dy) != TargetClass(x + dx, y + dy)) return false;
                    for (int dy = -2; dy <= 2; dy++)
                        for (int dx = -2; dx <= 2; dx++)
                            if (SourceClass(s, sx + dx, sy + dy) == Other) return false;
                    return true;
                }
                int Covered(int s) {
                    int covered = 0;
                    for (int y = 0; y < SwitchHeight; y++)
                        for (int x = 0; x < SwitchWidth / 2; x++) {
                            if (TargetClass(x, y) == Air) continue;
                            bool found = false;
                            for (int sy = 0; sy < learnt[s].h && !found; sy++)
                                for (int sx = 0; sx < learnt[s].w && !found; sx++)
                                    found = SourceClass(s, sx, sy) == TargetClass(x, y) && ShapedAt(s, sx, sy, x, y);
                            if (found) covered++;
                        }
                    return covered;
                }
                // (ModelRanks: which, by that order, as picked by eye; -1 none, every room alike);
                var models = Enumerable.Range(0, learnt.Count).OrderByDescending(s => (learnt[s].same ? 1000 : 0) + Covered(s)).ToList();
                int rank = int.TryParse(Environment.GetEnvironmentVariable("SWITCH_ROOM_MODEL_RANK"), out int asked) ? asked : ModelRanks[level];
                model = rank >= 0 && rank < models.Count ? models[rank] : -1;
                if (Environment.GetEnvironmentVariable("SWITCH_ROOM_MODEL") is string named) model = learntRecords.IndexOf(Convert.ToInt32(named, 16));
                List<(int s, int x, int y)> Candidates(int x, int y, Func<int, int, int, bool> allowed) {
                    var all = new List<(int s, int x, int y)>();
                    for (int s = 0; s < learnt.Count; s++)
                        for (int sy = 0; sy < learnt[s].h; sy++)
                            for (int sx = 0; sx < learnt[s].w; sx++)
                                if (allowed(s, sx, sy)) all.Add((s, sx, sy));
                    bool Shaped((int s, int x, int y) c) {
                        for (int dy = -1; dy <= 1; dy++)
                            for (int dx = -1; dx <= 1; dx++)
                                if (SourceClass(c.s, c.x + dx, c.y + dy) != TargetClass(x + dx, y + dy)) return false;
                        return true;
                    }
                    // (and away from anything but ground, air and one-way floors, 2 cells around, where there are: no rock from under
                    // water, by a door, a slope or blocks; from rooms in the switch room's colours first: a metatile another palette
                    // draws orange may draw green in its);
                    bool Clean((int s, int x, int y) c) {
                        for (int dy = -2; dy <= 2; dy++)
                            for (int dx = -2; dx <= 2; dx++)
                                if (SourceClass(c.s, c.x + dx, c.y + dy) == Other) return false;
                        return true;
                    }
                    var pool = new[] { all.Where(c => c.s == model && Shaped(c) && Clean(c)), all.Where(c => Shaped(c) && Clean(c) && learnt[c.s].same), all.Where(c => Shaped(c) && Clean(c)),
                                       all.Where(c => Shaped(c) && learnt[c.s].same), all.Where(Shaped) }
                        .Select(p => p.ToList()).FirstOrDefault(p => p.Count > 0) ?? all;
                    return pool;
                }
                (int s, int x, int y) Best(int x, int y, Func<int, int, int, bool> allowed) {
                    var pool = Candidates(x, y, allowed);
                    return pool.Count == 0 ? (-1, 0, 0) : pool.MinBy(c => Cost(c.s, c.x, c.y, x, y));
                }
                // Earth and one-way floors, row by row, then again with every neighbour taken;
                for (int pass = 0; pass < 2; pass++)
                    for (int y = 0; y < SwitchHeight; y++)
                        for (int x = 0; x < SwitchWidth; x++) {
                            int target = TargetClass(x, y);
                            if (target == Air) continue;
                            var best = Best(x, y, (s, sx, sy) => SourceClass(s, sx, sy) == target);
                            int metatile = best.s >= 0 ? SourceMetatile(best.s, best.x, best.y) : target == OneWay || Pedestal(x, y) ? block : y > 0 && !Solid(x, y - 1) ? top : ground;
                            WriteShort(cells, (y * SwitchWidth + x) * 4, metatile);
                            taken[y * SwitchWidth + x] = true;
                        }
                /* What stands on the floors: on each air cell on the main floor (off the mirrors, the doors and the pedestal), what the
                 * level draws on the same floor metatile in the same shape, where that's a thing standing alone a cell big (air above
                 * and beside it: a flower, a crystal, not part of a tree, a hill or a fence), or nothing; */
                var mirrors = SwitchSides(level);
                bool Clear(int x, int y) => mirrors.All(m => Math.Abs(m.x - x) > 1) && !(b && y >= 7 && Math.Abs(x - SwitchLinkDoor(level).x) <= 1)
                                         && !Pedestal(x, y + 1) && !Ledge(x, y + 1) && !Pedestal(x, y + 2) && !Ledge(x, y + 2);
                for (int y = 1; y < SwitchHeight; y++)
                    for (int x = 0; x < SwitchWidth; x++) {
                        if (TargetClass(x, y) != Air || TargetClass(x, y + 1) != Earth || !Clear(x, y)) continue;
                        int floor = Current(x, y + 1);
                        var best = Best(x, y, (s, sx, sy) => SourceClass(s, sx, sy) == Air && SourceMetatile(s, sx, sy + 1) == floor
                            && SourceMetatile(s, sx, sy - 1) == air && SourceMetatile(s, sx - 1, sy) == air && SourceMetatile(s, sx + 1, sy) == air);
                        if (best.s < 0) continue;
                        Set(x, y, SourceMetatile(best.s, best.x, best.y), 0x00);
                        taken[y * SwitchWidth + x] = true;
                    }

                /* Symmetric, as KatAM's hub rooms: one half as above (the one the level would draw: its metatiles side by side and one
                 * above another as often as the level draws them), the other its mirror image, each metatile there flipped: the
                 * tileset's own drawing its pixels flipped (walls' left and right pieces, corners; itself where it's symmetric), else
                 * made, its tiles flipped (the level's art is mostly drawn one way: Orange Ocean's rock, Yogurt Yard's frosting); */
                double Drawn(int from, int to) {
                    double drawn = 0;
                    for (int y = 0; y < SwitchHeight; y++)
                        for (int x = from; x <= to; x++) {
                            if (x < to) drawn += Math.Log(1 + beside.GetValueOrDefault((Current(x, y), Current(x + 1, y))));
                            if (y + 1 < SwitchHeight) drawn += Math.Log(1 + under.GetValueOrDefault((Current(x, y), Current(x, y + 1))));
                        }
                    return drawn;
                }
                bool designed = SwitchDesigns.TryGetValue(level, out string[] design) && Environment.GetEnvironmentVariable("SWITCH_ROOM_NO_DESIGNS") == null;
                if (Environment.GetEnvironmentVariable($"SWITCH_ROOM_DESIGN_{level}") is string tried) { design = tried.Split('/'); designed = true; }
                bool leftKept = designed || Drawn(0, SwitchWidth / 2 - 1) >= Drawn(SwitchWidth / 2, SwitchWidth - 1);
                // (a metatile flipped: its tiles swapped left and right, each drawn flipped (0x400), added to the room's own copy of the
                // tileset's metatiles where the tileset hasn't one drawing it so);
                List<byte> table = new List<byte>(metatiles);
                Dictionary<(int m, bool h, bool v), int> made = new Dictionary<(int, bool, bool), int>();
                Dictionary<int, (int m, bool h, bool v)> madeFrom = new Dictionary<int, (int, bool, bool)>();
                // Flip(); A metatile flipped left to right (h) and/or upside down (v): the tileset's own drawing it so (left to right),
                // else one made: its tiles swapped and each drawn flipped (0x400, 0x800);
                int Flip(int m, bool h, bool v) {
                    if (madeFrom.TryGetValue(m, out var from)) (m, h, v) = (from.m, from.h ^ h, from.v ^ v);
                    if (!h && !v) return m;
                    if (h && !v && Mirror(m) >= 0) return Mirror(m);
                    if (made.TryGetValue((m, h, v), out int id)) return id;
                    int[] e = Enumerable.Range(0, 4).Select(q => ReadShort(metatiles, m * 8 + q * 2) ^ (h ? 0x400 : 0) ^ (v ? 0x800 : 0)).ToArray();
                    foreach (int q in Enumerable.Range(0, 4)) {
                        int entry = e[q ^ (h ? 1 : 0) ^ (v ? 2 : 0)];
                        table.Add((byte)entry); table.Add((byte)(entry >> 8));
                    }
                    id = table.Count / 8 - 1;
                    made[(m, h, v)] = id; madeFrom[id] = (m, h, v);
                    return id;
                }
                // (or the level's design, SwitchDesigns, as its left half: ~ a metatile flipped left to right, ^ upside down, @ both);
                if (designed)
                    for (int y = 0; y < SwitchHeight; y++)
                        for (int x = 0; x < SwitchWidth / 2; x++) {
                            string id = design[y].Split(' ')[x];
                            bool h = id[0] is '~' or '@', v = id[0] is '^' or '@';
                            WriteShort(cells, (y * SwitchWidth + x) * 4, Flip(Convert.ToInt32(id.TrimStart('~', '^', '@'), 16), h, v));
                        }
                for (int y = 0; y < SwitchHeight; y++)
                    for (int x = leftKept ? SwitchWidth / 2 : 0; x < (leftKept ? SwitchWidth : SwitchWidth / 2); x++)
                        WriteShort(cells, (y * SwitchWidth + x) * 4, Flip(Current(SwitchWidth - 1 - x, y), true, false));
                // (UnmirroredFill: inside the ground, the other half's metatiles as they are, not flipped: its texture runs on across
                // the room, not mirrored, only the edges are);
                if (UnmirroredFill.Contains(level) || Environment.GetEnvironmentVariable($"SWITCH_ROOM_UNMIRRORED_{level}") != null)
                    for (int y = 0; y < SwitchHeight; y++)
                        for (int x = leftKept ? SwitchWidth / 2 : 0; x < (leftKept ? SwitchWidth : SwitchWidth / 2); x++) {
                            int from = leftKept ? x - SwitchWidth / 2 : x + SwitchWidth / 2;
                            bool Inside(int cx) => Enumerable.Range(-1, 3).All(dy => Enumerable.Range(-1, 3).All(dx => TargetClass(cx + dx, y + dy) == Earth));
                            if (Inside(x) && Inside(from)) WriteShort(cells, (y * SwitchWidth + x) * 4, Current(from, y));
                        }
                // (its decorations, SwitchDecorations, each side its own, in air only);
                string[] decorations = SwitchDecorations.GetValueOrDefault(level, new string[0]);
                if (Environment.GetEnvironmentVariable($"SWITCH_ROOM_DECOR_{level}") is string triedDecor) decorations = triedDecor.Split(';', StringSplitOptions.RemoveEmptyEntries);
                /* (* before a metatile: one of ForeignDecorations' room's (King Dedede's arena's, for the Fountain of Dreams'), its tiles
                 * added after the room's own and its colour rows put in rows the room's metatiles don't use: so a room can be dressed in
                 * another tileset's pieces); */
                List<byte> roomTiles = null;
                byte[] roomPalette = null;
                Dictionary<int, int> foreignTile = new Dictionary<int, int>(), foreignMetatile = new Dictionary<int, int>(), foreignRow = new Dictionary<int, int>();
                Queue<int> freeRows = null;
                NightmareRoom foreignRoom = null;
                if (Environment.GetEnvironmentVariable($"SWITCH_ROOM_FOREIGN_{level}") is string triedForeign)
                    ForeignDecorations[level] = (int.Parse(triedForeign.Split(',')[0]), int.Parse(triedForeign.Split(',')[1]), int.Parse(triedForeign.Split(',')[2]));
                if (ForeignDecorations.TryGetValue(level, out var foreignOf)) {
                    // (its stage, or -1 its level's last; its index, or -1 the stage's last);
                    int stage = foreignOf.stage >= 0 ? foreignOf.stage : rooms.Where(r => r.Level == foreignOf.level && !r.IsSwitchRoom).Max(r => r.Stage);
                    var inStage = rooms.Where(r => r.Level == foreignOf.level && r.Stage == stage);
                    foreignRoom = foreignOf.index >= 0 ? inStage.First(r => r.Index == foreignOf.index) : inStage.MaxBy(r => r.Index);
                }
                int Foreign(int m) {
                    if (foreignMetatile.TryGetValue(m, out int id)) return id;
                    byte[] theirTiles = DecompressLZ77(nidl, ReadInt(nidl, foreignRoom.Record + 0x1C) - 0x08000000);
                    byte[] theirMetatiles = DecompressLZ77(nidl, ReadInt(nidl, foreignRoom.Record + 0x20) - 0x08000000);
                    if (roomTiles == null) {
                        roomTiles = new List<byte>(DecompressLZ77(nidl, ReadInt(nidl, source.Record + 0x1C) - 0x08000000));
                        int ours = ReadInt(nidl, source.Record + 0x18) - 0x08000000;
                        roomPalette = nidl.Skip(ours).Take(2 + 0xC0).ToArray();
                        // (the rows the room's own metatiles don't use, for theirs);
                        var used = Enumerable.Range(0, SwitchWidth * SwitchHeight).SelectMany(c => Enumerable.Range(0, 4).Select(q => table[ReadShort(cells, c * 4) * 8 + q * 2 + 1] >> 4)).ToHashSet();
                        freeRows = new Queue<int>(Enumerable.Range(2, 6).Where(r => !used.Contains(r)));
                    }
                    foreach (int q in Enumerable.Range(0, 4)) {
                        int entry = ReadShort(theirMetatiles, m * 8 + q * 2), tile = entry & 0x3FF;
                        if (!foreignTile.TryGetValue(tile, out int mine)) {
                            foreignTile[tile] = mine = roomTiles.Count / 32;
                            roomTiles.AddRange(tile * 32 + 32 <= theirTiles.Length ? theirTiles.Skip(tile * 32).Take(32) : new byte[32]);
                        }
                        if (!foreignRow.TryGetValue(entry >> 12, out int row)) {
                            // (rows 2-7 the foreground's, 8 on the background's; none left, the row of the room's colours nearest theirs);
                            int from = entry >> 12, theirs = ReadInt(nidl, foreignRoom.Record + (from < 8 ? 0x18 : 0x28)) - 0x08000000;
                            int source = theirs + 2 + (from - (from < 8 ? 2 : 8)) * 32;
                            if (freeRows.Count > 0) {
                                foreignRow[from] = row = freeRows.Dequeue();
                                Array.Copy(nidl, source, roomPalette, 2 + (row - 2) * 32, 32);
                            } else {
                                int Distance(int r) => Enumerable.Range(1, 15).Sum(c => {
                                    int a = ReadShort(nidl, source + c * 2), b = ReadShort(roomPalette, 2 + (r - 2) * 32 + c * 2);
                                    return Math.Abs((a & 31) - (b & 31)) + Math.Abs((a >> 5 & 31) - (b >> 5 & 31)) + Math.Abs((a >> 10 & 31) - (b >> 10 & 31));
                                });
                                foreignRow[from] = row = Enumerable.Range(2, 6).MinBy(Distance);
                            }
                        }
                        entry = (entry & 0x0C00) | mine | row << 12;
                        table.Add((byte)entry); table.Add((byte)(entry >> 8));
                    }
                    return foreignMetatile[m] = table.Count / 8 - 1;
                }
                foreach (string decoration in decorations) {
                    // (! first: over the terrain too, as flowers in grass, its collision kept);
                    bool over = decoration.Trim().StartsWith("!");
                    string[] parts = decoration.Trim().TrimStart('!').Split(' ', 3);
                    int left = int.Parse(parts[0]), up = int.Parse(parts[1]);
                    string[] lines = parts[2].Split('/');
                    for (int dy = 0; dy < lines.Length; dy++) {
                        string[] ids = lines[dy].Split(' ', StringSplitOptions.RemoveEmptyEntries);
                        for (int dx = 0; dx < ids.Length; dx++) {
                            int x = left + dx, y = up + dy;
                            if (ids[dx] == "000" || x < 0 || y < 0 || x >= SwitchWidth || y >= SwitchHeight || !over && TargetClass(x, y) != Air) continue;
                            if (ids[dx][0] == '*') { WriteShort(cells, (y * SwitchWidth + x) * 4, Foreign(Convert.ToInt32(ids[dx][1..], 16))); continue; }
                            bool h = ids[dx][0] is '~' or '@', v = ids[dx][0] is '^' or '@';
                            WriteShort(cells, (y * SwitchWidth + x) * 4, Flip(Convert.ToInt32(ids[dx].TrimStart('~', '^', '@'), 16), h, v));
                        }
                    }
                }
                if (Environment.GetEnvironmentVariable("SWITCH_ROOM_DUMP") is string tableDump)
                    File.WriteAllBytes(Path.Combine(tableDump, $"switch{level}.metatiles"), table.ToArray());
                if (Environment.GetEnvironmentVariable("SWITCH_ROOM_DUMP") is string dump)
                    File.WriteAllText(Path.Combine(dump, $"switch{level}.txt"), $"{source.Record:X} {(model >= 0 ? learntRecords[model].ToString("X") : "-")}\n" + string.Join("\n", Enumerable.Range(0, SwitchHeight).Select(y => string.Join(" ", Enumerable.Range(0, SwitchWidth).Select(x => Current(x, y).ToString("X3"))))));

                byte[] record = nightmare.Skip(source.Record).Take(0x54).ToArray();
                // (the Fountain of Dreams': the Fountain of Dreams' own background, its starry nebula, as a room a screen high shows only
                // 20 rows of it, from the top: its rows 47-66, the nebula with the stars over it, uncompressed);
                if (level == 7) {
                    // (or, FountainInBack, King Dedede's arena's, Rainbow Resort's last stage, on the Fountain itself: its rows 15-34, the
                    // grass and the Fountain, a column (8 pixels) right: its stem at x 128, the room's middle, under the doors);
                    bool inBack = Environment.GetEnvironmentVariable("SWITCH_ROOM_FOUNTAIN_BACK") is string back ? back == "1" : FountainInBack;
                    int dededeStage = rooms.Where(r => r.Level == 6).Max(r => r.Stage);
                    NightmareRoom framing = inBack ? rooms.Where(r => r.Level == 6 && r.Stage == dededeStage).OrderBy(r => r.Index).First() : fountain;
                    Array.Copy(nightmare, framing.Record + 0x28, record, 0x28, 0x10);
                    int map = ReadInt(nightmare, framing.Record + 0x30) - 0x08000000, mapWidth = ReadShort(nightmare, map + 2);
                    bool packed = ReadShort(nightmare, map + 6) == 0xFFFF;
                    byte[] whole = packed ? DecompressLZ77(nightmare, map + 8) : null;
                    int FirstRow = inBack ? 15 : 47; const int Rows = 20;
                    byte[] framed = new byte[6 + mapWidth * Rows * 2];
                    WriteShorts(framed, 0, mapWidth * Rows * 2, mapWidth, Rows);
                    // (as it is across: Shift columns right where a feature should be mid-screen, the camera showing columns 0.5-15.5);
                    int Shift = inBack ? 1 : 0;
                    for (int y = 0; y < Rows; y++)
                        for (int column = 0; column < mapWidth; column++) {
                            int row = FirstRow + y, x = (column - Shift + mapWidth) % mapWidth;
                            WriteShort(framed, 6 + (y * mapWidth + column) * 2, packed ? ReadShort(whole, (((row >> 5) * ((mapWidth + 31) >> 5) + (x >> 5)) * 1024 + (row & 31) * 32 + (x & 31)) * 2)
                                                                                : ReadShort(nightmare, map + 6 + (row * mapWidth + x) * 2));
                        }
                    WriteInt(record, 0x30, Put(framed) + 0x08000000);
                }
                if (table.Count > metatiles.Length) WriteInt(record, 0x20, Put(CompressLZ77(table.ToArray())) + 0x08000000);
                if (roomTiles != null) {
                    WriteInt(record, 0x1C, Put(CompressLZ77(roomTiles.ToArray())) + 0x08000000);
                    WriteInt(record, 0x18, Put(roomPalette) + 0x08000000);
                }
                WriteShort(record, 2, 0);
                WriteInts(record, 8, Put(CompressLZ77(cells)) + 0x08000000, Put(CompressLZ77(new byte[SwitchWidth * SwitchHeight * 2])) + 0x08000000, 0);
                WriteShorts(record, 0x14, SwitchWidth, SwitchHeight);
                WriteInts(record, 0x44, 0, 0);
                WriteShorts(record, 0x50, SwitchArrival(level).x * 16 + 8, SwitchArrival(level).y * 16 + 8);
                int at = Put(record);
                switchRooms.Add(new NightmareRoom(level, SwitchStage, 0, at, 0, 0, SwitchWidth, SwitchHeight));

            }

            byte[] extended = new byte[start + added.Count];
            nightmare.CopyTo(extended, 0);
            added.CopyTo(extended, start);
            nightmare = extended;
            return switchRooms;
        }

        /* AddRooms(); Every NiDL room into KatAM's free room slots (see the notes above); the room records' foregrounds, palettes and
         * backgrounds are shared as in NiDL; */
        /* BigRooms(); Rooms of up to BigRoomBlocks blocks ("ROM Expansion/rooms.c": each loaded room's collision, zones and tilemap in
         * buffers that big, two in KatAM's own, two at the end of the EWRAM heap): the game's functions using those buffers jump to
         * rooms.c's copies, and the heap (EwramInitHeap's size, 0x20080) shrinks to NidlHeapSize; */
        const int BigRoomBlocks = 3584, VanillaHeapSize = 0x20080, NidlHeapSize = 0xE478;   // rooms.c's BIG_BLOCKS, NIDL_HEAP_SIZE;
        // (and the ported rooms' doors lists, from FirstDoorsList, their visited doors in rooms.c's NIDL_VISITED: the functions reading them);
        static readonly string[] BigRoomFunctions = {
            "FillLevelInfo", "GetCollisionTile", "sub_08002434", "sub_08002470", "sub_080024F0", "sub_0800255C", "sub_080025AC",
            "sub_08002624", "sub_080026A8", "sub_08001408", "sub_080015A8",
            "sub_08002A5C", "sub_08002A80", "sub_08002AD0", "sub_08002B30", "sub_08002BA8", "sub_08002C18",
            "sub_08001678", "sub_080027E8", "sub_080027A8",
        };
        static string BigRooms(byte[] rom, KatAMExpansionCode.Version version) {
            int codeBase = CodeAddress + 0x08000000, heap = version.RomSymbols["EwramInitHeap"], size = -1;
            for (int at = (heap + 3) & ~3; at < heap + 0x20; at += 4)
                if (ReadInt(rom, at) == VanillaHeapSize) { size = at; break; }
            if (size < 0) return $"The {version.Name} ROM's EWRAM heap isn't what it should be.";
            WriteInt(rom, size, NidlHeapSize);
            foreach (string function in BigRoomFunctions) {
                string replacement = "Big" + (function.StartsWith("sub_") ? "Sub" + function.Substring(3) : function);
                Redirect(rom, version.RomSymbols[function], codeBase + version.CodeSymbols[replacement]);
            }
            // (and the completion counts KatAM's chests only, not the NiDL rooms' (PlanChests()): rooms.c's NidlNumChestsCollected);
            Redirect(rom, version.RomSymbols["NumChestsCollected"], codeBase + version.CodeSymbols["NidlNumChestsCollected"]);
            /* (and the ported rooms' visited doors are saved with the game's world props, in SRAM after them: rooms.c's NidlWriteSave;
             * the game's functions that write and read them detoured there, their trampolines in gNidlSave, and the world props' clear,
             * sbClearFunctions[1], pointed to NidlClearSave, the original after them); */
            int saveTrampolines = CodeAddress + version.CodeSymbols["gNidlSaveTrampolines"], save = CodeAddress + version.CodeSymbols["gNidlSave"];
            string[] saveFunctions = { "WriteSaveSectionByOffset", "VerifySaveByOffset" }, saveHooks = { "NidlWriteSave", "NidlVerifySave" };
            for (int i = 0; i < saveFunctions.Length; i++) {
                int trampoline = Detour(rom, version.RomSymbols[saveFunctions[i]], codeBase + version.CodeSymbols[saveHooks[i]], ref saveTrampolines);
                if (trampoline < 0) return $"The {version.Name} ROM's save code doesn't match.";
                WriteInt(rom, save + i * 4, trampoline + 0x08000000 + 1);
            }
            int clears = version.RomSymbols["sbClearFunctions"];
            WriteInt(rom, save + 8, ReadInt(rom, clears + 4));
            WriteInt(rom, clears + 4, codeBase + version.CodeSymbols["NidlClearSave"] + 1);
            return null;
        }

        /* Redirect(); Makes the Thumb function at `function` jump to `target` for good, all registers kept (it can take 4 arguments):
         * bx pc (into ARM, at the next word), ldr ip, [pc]; bx ip; the target; */
        static void Redirect(byte[] rom, int function, int target) {
            int arm = (function + 4) & ~3;
            WriteShorts(rom, function, 0x4778, 0x46C0);
            WriteInts(rom, arm, unchecked((int)0xE59FC000), unchecked((int)0xE12FFF1C), target + 1);
        }

        static string AddRooms(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, ref int cursor) {
            IReadOnlyDictionary<string, int> symbols = version.RomSymbols;
            string bigRooms = BigRooms(rom, version);
            if (bigRooms != null) return bigRooms;
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"];
            List<NightmareRoom> rooms;

            try {
                rooms = NightmareRooms(nightmare, region.RoomTable);
                rooms.AddRange(BuildSwitchRooms(ref nightmare, rooms));
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            // The vanilla rooms: their tables' sizes, the free slots, and what new rooms borrow (all from rooms every version has);
            const int RoomCount = 0x3D7;
            List<int> free = new List<int>();
            int[] tableSizes = new int[20];
            for (int room = 1; room < RoomCount; room++) {
                int props = roomProps + room * 0x28;
                if (Enumerable.Range(0, 19).All(i => ReadShort(rom, props + i * 2) == 0xFFFF)) { free.Add(room); continue; }
                for (int field = 10; field < 19; field++) {
                    int index = ReadShort(rom, props + field * 2);
                    if (index != 0xFFFF) tableSizes[field] = Math.Max(tableSizes[field], index + 1);
                }
            }

            int Props(int room, int field) => ReadShort(rom, roomProps + room * 0x28 + field * 2);
            int Entry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;
            int paletteCycles = ReadInt(rom, Entry("gForegroundPalettes", Props(0x323, 11)) + 4),        // Unused (no cycles), but read;
                navigationRoutes = Entry("gUnk_08D63C28", Props(0x3C9, 17));                                  // Its 4 generic route lists;

            List<int> infos = new List<int>(), palettes = new List<int>(), tilemaps = new List<int>(), backgrounds = new List<int>(),
                      solidities = new List<int>(), objects = new List<int>(), navigations = new List<int>();
            Dictionary<(int, int), int> infoOf = new Dictionary<(int, int), int>();
            Dictionary<int, int> paletteOf = new Dictionary<int, int>(), builtBackground = new Dictionary<int, int>(), frameCopies = new Dictionary<int, int>();
            Dictionary<(int, string), int> backgroundOf = new Dictionary<(int, string), int>();
            int noSpecials = Allocate(ref cursor, 8, 4);
            WriteInts(rom, noSpecials, 0, unchecked((int)0xFFFF0000));
            int noTypes = Allocate(ref cursor, 4, 4);
            WriteInt(rom, noTypes, 0xFF);

            // Room numbers: the NiDL hub's first, then each level's (its switch room first) from its hundred, then from any left;
            if (free.Count < rooms.Count + 1) return "KatAM has too few free rooms for Nightmare in Dream Land's.";
            List<int> left = new List<int>(free);
            int mainHub = left.Where(n => n / 100 == LevelHundreds[7]).DefaultIfEmpty(left[0]).First();
            left.Remove(mainHub);
            for (int level = 0; level < 8; level++) {
                foreach (NightmareRoom r in rooms.Where(r => r.Level == level).OrderBy(r => r.IsSwitchRoom ? 0 : 1)) {
                    int room = left.Where(n => n / 100 == LevelHundreds[level]).DefaultIfEmpty(left[0]).First();
                    r.Room = room;
                    left.Remove(room);
                }
            }
            Dictionary<NightmareRoom, List<Door>> doors;
            List<Door> mainHubDoors;
            Dictionary<int, List<Door>> katamDoors;
            (int room, int x, int y) firstStart;
            byte firstFacing;
            try {
                doors = PlanDoors(rom, symbols, nightmare, rooms, mainHub, out mainHubDoors, out katamDoors, out firstStart, out firstFacing);
                chestsOf = PlanChests(nightmare, rooms, doors, mainHubDoors, katamDoors);
                takenItems = 0;
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            try {
                for (int i = 0; i < rooms.Count; i++) {
                    NightmareRoom r = rooms[i];
                    int record = r.Record, roomWidth = ReadShort(nightmare, record + 0x14);
                    int Address(int offset) => ReadInt(nightmare, record + offset) - 0x08000000;
                    byte[] cells = RoomCells(nightmare, record);
                    // The NiDL cell for a cell of the padded room (see Padded()): the edge's own where past it;
                    int Cell(int x, int y) => (r.Y + Math.Min(y, r.Height - 1)) * roomWidth + r.X + Math.Min(x, r.Width - 1);

                    // Foreground tiles (NiDL's own LZ77) and their animations (see TileAnimations()), and palette (NiDL's palettes 2-7,
                    // KatAM's 0-5), shared;
                    int tilesAddress = Address(0x1C), paletteAddress = Address(0x18), animationSet = ReadShort(nightmare, record + 0x40);
                    if (!infoOf.TryGetValue((tilesAddress, animationSet), out int info)) {
                        int tiles = CopyLZ77(rom, nightmare, tilesAddress, ref cursor);
                        (int animations, int count) = TileAnimations(rom, nightmare, region.RoomTable, animationSet, frameCopies, ref cursor);
                        int at = Allocate(ref cursor, 16, 4);
                        WriteInts(rom, at, tiles, 0, animations);
                        WriteShorts(rom, at + 12, count, r.Room);
                        infoOf[(tilesAddress, animationSet)] = info = tableSizes[10] + infos.Count;
                        infos.Add(at + 0x08000000);
                    }
                    if (!paletteOf.TryGetValue(paletteAddress, out int palette)) {
                        int colors = Allocate(ref cursor, 0xC0, 4);
                        Array.Copy(nightmare, paletteAddress + 2, rom, colors, 0xC0);
                        int at = Allocate(ref cursor, 12, 4);
                        WriteInts(rom, at, colors + 0x08000000, paletteCycles);
                        WriteShorts(rom, at + 8, 0, r.Room);
                        paletteOf[paletteAddress] = palette = tableSizes[11] + palettes.Count;
                        palettes.Add(at + 0x08000000);
                    }

                    // Foreground tilemap: each metatile's 4 entries, palettes down by 2;
                    byte[] metatiles = DecompressLZ77(nightmare, Address(0x20));
                    int width = r.FullWidth * 2, height = r.FullHeight * 2;
                    byte[] tilemap = new byte[width * height * 2];
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            int cell = Cell(x / 2, y / 2);
                            int entry = ReadShort(metatiles, ReadShort(cells, cell * 4) * 8 + ((y & 1) * 2 + (x & 1)) * 2);
                            WriteShort(tilemap, (y * width + x) * 2, ShiftPalette(entry));
                        }
                    }
                    byte[] compressedTilemap = CompressLZ77(tilemap);
                    int tilemapAt = Allocate(ref cursor, compressedTilemap.Length, 4);
                    compressedTilemap.CopyTo(rom, tilemapAt);
                    int tilemapEntry = Allocate(ref cursor, 0x20, 4);
                    WriteShorts(rom, tilemapEntry, width, height);
                    WriteInts(rom, tilemapEntry + 0x18, tilemapAt + 0x08000000, r.Room << 16);
                    tilemaps.Add(tilemapEntry + 0x08000000);

                    // Background, shared (a room with its own colours for it, as Orange Ocean's sea at sunset, shares its tiles and map);
                    int backgroundMap = Address(0x30);
                    string colours = BackgroundColours(nightmare, Address(0x28));
                    if (!backgroundOf.TryGetValue((backgroundMap, colours), out int background)) {
                        int at = builtBackground.TryGetValue(backgroundMap, out int built) ? RecolourBackground(rom, built, nightmare, Address(0x28), r.Room, ref cursor)
                                                                                         : BuildBackground(rom, nightmare, Address(0x2C), Address(0x28), backgroundMap, r.Room, ref cursor);
                        builtBackground.TryAdd(backgroundMap, at);
                        backgroundOf[(backgroundMap, colours)] = background = tableSizes[15] + backgrounds.Count;
                        backgrounds.Add(at + 0x08000000);
                    }

                    // Collision, and a special block record per breakable block (what's left after: NiDL's metatile for the block's number);
                    // (the padding's breakable blocks are solid: KatAM would look for their special block records);
                    byte[] collision = new byte[r.FullWidth * r.FullHeight];
                    for (int y = 0; y < r.FullHeight; y++) {
                        for (int x = 0; x < r.FullWidth; x++) {
                            int kind = cells[Cell(x, y) * 4 + 3];
                            bool padding = x >= r.Width || y >= r.Height;
                            collision[y * r.FullWidth + x] = padding && kind >> 4 == 2 ? (byte)0x0D : !padding && kind is >= 0x38 and <= 0x3D ? FuseCollision[kind - 0x38] : RoomCollision[kind];
                        }
                    }
                    // Thin floors Kirby needn't drop through (ThinFloors());
                    var arrivals = doors.Values.SelectMany(list => list).Concat(mainHubDoors).Concat(katamDoors.Values.SelectMany(list => list))
                                        .Where(d => d.Room == r.Room).Select(d => (d.DestX - 1, d.DestY - 1))
                                        .Concat(transportsOf.Values.SelectMany(list => list).Where(t => t.Room == r.Room).Select(t => (t.DestX - 1, t.DestY - 1))).ToList();
                    ThinFloors(r, collision, arrivals, doors[r].Select(d => (d.X, d.Y)).Concat(transportsOf[r].Select(t => (t.X / 16, (t.Y - 1) / 16))).ToList());
                    // Mirras (see MirraSpots()): not in a boss's room nor a switch room;
                    bool bossRoom = !r.IsSwitchRoom && (r.Level == 7 || r.Stage == rooms.Where(o => o.Level == r.Level && !o.IsSwitchRoom).Max(o => o.Stage));
                    var mirras = r.IsSwitchRoom || bossRoom ? new List<(int x, int y, bool door)>() : MirraSpots(r, doors[r], collision);
                    int solidity = BuildSolidity(rom, nightmare, r, cells, collision, metatiles, doors[r], mirras, noSpecials, ref cursor);
                    solidities.Add(solidity + 0x08000000);

                    // (where doors land Kirby in it: no enemy waits there, BuildObjects());
                    var landings = doors.Values.SelectMany(list => list).Concat(mainHubDoors).Concat(katamDoors.Values.SelectMany(list => list))
                                        .Where(d => d.Room == r.Room && !d.Edge).Select(d => (d.DestX - 1, d.DestY - 1)).ToList();
                    int objectList = BuildObjects(rom, nightmare, r, doors[r], mirras, landings, chestsOf[r], noTypes, ref cursor);
                    objects.Add(objectList + 0x08000000);
                    // (the randomizer's: its items and chests, but in a boss room, a switch room or a mini-boss's fight area; FinishRooms()
                    // lists its chests in its doors list, once the randomizer has moved them);
                    if (!r.IsSwitchRoom && !bossRoom) {
                        var fights = Arenas(nightmare, r);
                        for (int at = ReadInt(rom, objectList) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) {
                            int x = (short)ReadShort(rom, at + 6), y = (short)ReadShort(rom, at + 8);
                            if (rom[at + 12] is (>= 0x5E and <= 0x64) or SmallChestType
                                && !fights.Any(a => x >= a.left - 16 && x < a.left + a.width + 16 && y >= a.top - 16 && y < a.top + a.height + 16))
                                portedItems.Add((r.Room, at));
                        }
                        chestLists.Add((r.Room, i, objectList));
                    }
                    navigations.Add(BuildNavigation(rom, r, collision, navigationRoutes, Arenas(nightmare, r), ref cursor) + 0x08000000);

                    /* Area and CPU helpers' routes (gUnk_08D6CD0C, written into the vanilla table's free entry), as room 0x397's (the
                     * Dimension Mirror's): no routes (+0, +0x48 their count: 0), flags 0x81 (+0x47: roaming helpers don't go through a
                     * door into it, and one in it, brought by the phone, heads back where it came from: a helper looking for a way to
                     * Kirby through rooms the routes don't know got 0xFFFF and loaded that room, jumping into garbage); */
                    // (all of 0x397's, its own room and area after: free space is 0xFF, and the game reads every byte to +0x49);
                    int roomInfo = Allocate(ref cursor, 0x4C, 4);
                    Array.Copy(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000, rom, roomInfo, 0x4C);
                    WriteShort(rom, roomInfo + 0x44, r.Room);
                    rom[roomInfo + 0x46] = (byte)r.Area;
                    // For the Map Tracker (the game never reads the padding, +0x4A-0x4B): the NiDL stage + 1 (9: the switch room) and room number;
                    rom[roomInfo + 0x4A] = (byte)(r.Stage + 1); rom[roomInfo + 0x4B] = (byte)r.Index;
                    WriteInt(rom, roomInfos + r.Room * 4, roomInfo + 0x08000000);

                    // The camera's top (CameraTop()); the background spans the room once, both ways (BackgroundRates());
                    int top = CameraTop(nightmare, r);
                    // (rooms.c's code: 0 KatAM's 8 pixels, 1 the room's top, 2 a row down; it was top / 8 + 1, two rows down, 0x147);
                    rom[CodeAddress + version.CodeSymbols["gNidlRoomTops"] + r.Room] = (byte)(top == 8 ? 0 : top / 16 + 1);
                    (int rateX, int rateY) = BackgroundRates(ReadShort(nightmare, backgroundMap + 2) * 8, ReadShort(nightmare, backgroundMap + 4) * 8,
                                                             r.FullWidth * 16, r.FullHeight * 16, top);
                    WriteShorts(rom, roomProps + r.Room * 0x28, LevelSongs[r.Level], 0, 0, 0x100, 0x100, 0, 0, rateX, rateY, RoomFlags,
                                info, palette, tableSizes[12] + i, tableSizes[13] + i, 0xFFFF, background, tableSizes[16] + i,
                                tableSizes[17] + i, FirstDoorsList + i, 0);
                }
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            WarpFlights(rom, symbols, ref cursor);
            if (WarpRideDetours(rom, version) is string rideError) return rideError;

            BuildMainHub(rom, nightmare, symbols, version.SpriteTables, mainHub, mainHubDoors, rooms.First(r => r.Level == 7 && r.Stage == 0 && r.Index == 0).Record, tableSizes,
                         infos, palettes, tilemaps, solidities, backgrounds, objects, noTypes, ref cursor, out Door abilityDoor);
            WriteShort(rom, CodeAddress + version.CodeSymbols["gNidlHubRoom"], mainHub);
            /* The boss rooms' shards (bossExitsOf): gNidlShardExits ("ROM Expansion/nidl_boss.c"), and sub_0804C614's `kirby->roomId = 0x321`
             * and spawn spot (after the shard cutscene) a call to NidlShardExit(): `adds r0, r4, #0; ldr r1, =NidlShardExit; bl
             * _call_via_r1; b` past the spot's stores, the 0x321 literal holding the address; */
            {
                int exits = CodeAddress + version.CodeSymbols["gNidlShardExits"], site = version.RomSymbols["sub_0804C614"] + 0x174;
                int literal = version.RomSymbols["sub_0804C614"] + 0x1B4;
                if (ReadShort(rom, site) != 0x2083 || ReadInt(rom, literal) != 0x321)
                    return $"The {region.KatAM} ROM's shard code doesn't match.";
                int n = 0;
                foreach (var (part, exit) in bossExitsOf.OrderBy(e => e.Key.Room)) {
                    if (n >= 24) break;
                    WriteShorts(rom, exits + n * 8, part.Room, exit.room);
                    rom[exits + n * 8 + 4] = (byte)exit.x;
                    rom[exits + n * 8 + 5] = (byte)exit.y;
                    n++;
                }
                int call = version.RomSymbols["_call_via_r1"] - (site + 0x08000000 + 4 + 4);
                WriteShorts(rom, site, 0x1C20, 0x4900 | (literal - ((site + 2 + 4) & ~3)) / 4, 0xF000 | (call >> 12 & 0x7FF), 0xF800 | (call >> 1 & 0x7FF),
                            0xE000 | ((0x198 - (0x174 + 8) - 4) / 2 & 0x7FF));
                WriteInt(rom, literal, CodeAddress + 0x08000000 + version.CodeSymbols["NidlShardExit"] + 1);
                // (for the map tracker, in the map table after its mini-bosses: "SX", u16 the exits, the table's address);
                System.Text.Encoding.ASCII.GetBytes("SX").CopyTo(rom, MapShardExits);
                WriteShort(rom, MapShardExits + 2, n);
                WriteInt(rom, MapShardExits + 4, exits + 0x08000000);
            }

            /* The KatAM rooms getting doors (Central Circle: its door to Nightmare in Dream Land; the KatAM rooms NiDL levels lead to: their
             * doors back) get collision and object list entries of their own (still their vanilla ones: the randomizer writes into them),
             * which FinishRooms() replaces with copies that have the doors, once the randomizer is done; */
            List<(int room, int solidity, int objects, int tilemap, List<Door> doors)> edited = new List<(int, int, int, int, List<Door>)> {
                (EntryRoom, 0, -1, 0, new List<Door> { new Door(EntryX, EntryY, firstStart.room, firstStart.x, firstStart.y, firstFacing) })
            };
            foreach (var (room, list) in katamDoors) edited.Add((room, 0, 0, 0, list));
            foreach (int room in LandingRooms.Where(r => r != EntryRoom && !katamDoors.ContainsKey(r))) edited.Add((room, 0, 0, 0, new List<Door>()));
            for (int i = 0; i < edited.Count; i++) {
                int room = edited[i].room, solidity = tableSizes[13] + solidities.Count, objectList = -1;
                solidities.Add(Entry("gSolidityMaps", Props(room, 13)) + 0x08000000);
                WriteShort(rom, roomProps + room * 0x28 + 13 * 2, solidity);
                if (edited[i].objects == 0) {
                    objectList = tableSizes[16] + objects.Count;
                    objects.Add(Entry("gLevelObjLists", Props(room, 16)) + 0x08000000);
                    WriteShort(rom, roomProps + room * 0x28 + 16 * 2, objectList);
                }
                edited[i] = (room, solidity, objectList, Entry("gForegroundTilemaps", Props(room, 12)), edited[i].doors);
            }
            int entryTilemap = edited[0].tilemap;

            // Its tilemap: the golden mirror to Nightmare in Dream Land on its platform's left end (door EntryX, EntryY: MirrorTwin());
            {
                int columns = ReadShort(rom, entryTilemap);
                byte[] map = DecompressLZ77(rom, ReadInt(rom, entryTilemap + 0x18) - 0x08000000);
                int info = Entry("gForegroundInfo", Props(EntryRoom, 10)), tilesPointer = ReadInt(rom, info);
                byte[] tiles = DecompressLZ77(rom, tilesPointer - 0x08000000);
                int paletteEntry = Entry("gForegroundPalettes", Props(EntryRoom, 11)), sourceColors = ReadInt(rom, paletteEntry) - 0x08000000;
                int[] colors = Enumerable.Range(0, 96).Select(i => ReadShort(rom, sourceColors + i * 2) & 0x7FFF).ToArray();
                List<int> specialEntries = new List<int>();
                int entrySpecials = ReadInt(rom, solidities[edited[0].solidity - tableSizes[13]] - 0x08000000 + 4) - 0x08000000;
                for (int at = entrySpecials; rom[at + 4] != 0; at += rom[at + 4])
                    if (rom[at] == 1) for (int q = 0; q < 4; q++) specialEntries.Add(ReadShort(rom, at + 8 + q * 2));
                MirrorTwin(map, columns, tiles, colors, specialEntries, CycledColors(rom, paletteEntry), 0);
                int infoAt = Allocate(ref cursor, 16, 4), tilesAt = AllocateCompressed(rom, CompressLZ77(tiles), ref cursor);
                Array.Copy(rom, info, rom, infoAt, 16);
                for (int i = 0; i < 3; i++) if (ReadInt(rom, infoAt + i * 4) == tilesPointer) WriteInt(rom, infoAt + i * 4, tilesAt + 0x08000000);
                WriteShort(rom, roomProps + EntryRoom * 0x28 + 10 * 2, tableSizes[10] + infos.Count);
                infos.Add(infoAt + 0x08000000);
                int colorsAt = Allocate(ref cursor, 0xC0, 4), newPalette = Allocate(ref cursor, 12, 4);
                for (int i = 0; i < 96; i++) WriteShort(rom, colorsAt + i * 2, colors[i]);
                Array.Copy(rom, paletteEntry, rom, newPalette, 12);
                WriteInt(rom, newPalette, colorsAt + 0x08000000);
                WriteShort(rom, roomProps + EntryRoom * 0x28 + 11 * 2, tableSizes[11] + palettes.Count);
                palettes.Add(newPalette + 0x08000000);

                byte[] compressedMap = CompressLZ77(map);
                int mapAt = Allocate(ref cursor, compressedMap.Length, 4), tilemapEntry = Allocate(ref cursor, 0x20, 4);
                compressedMap.CopyTo(rom, mapAt);
                Array.Copy(rom, entryTilemap, rom, tilemapEntry, 0x20);
                WriteInt(rom, tilemapEntry + 0x18, mapAt + 0x08000000);
                WriteShort(rom, roomProps + EntryRoom * 0x28 + 12 * 2, tableSizes[12] + tilemaps.Count);
                tilemaps.Add(tilemapEntry + 0x08000000);
            }
            ExtendAreas(rom, symbols, ref cursor);

            /* 9ROM: each ported room's doors list (doorsIdx FirstDoorsList + its number in rooms; the NiDL hub's SharedDoors, KatAM's last
             * free one): an entry (kind 2, size 8, room, landing + 1) per place its doors lead to. KatAM looks a door up there when Kirby
             * goes through it, until it finds it (it must be there), and every mirror looks its own up every frame (sub_08002BA8, from the
             * list's start): one list for all the rooms (hundreds of entries) made rooms with mirrors lag. Mirrors' entries first, the
             * edges' last. Their door counts are 0, so the map counts none of them; which were used is kept in rooms.c (NIDL_VISITED). Its
             * chests (PlanChests()) come first, as KatAM's (kind 1, size 8, the chest's spot): opening one marks it (sub_08002B30, which
             * looks until it finds it), and the list's count (+4) is its chests'; */
            int listCursor = cursor;
            int DoorsList(IEnumerable<Door> list, List<NidlChest> chests) {
                List<byte> nineROM = new List<byte>();
                foreach (NidlChest chest in chests) nineROM.AddRange(ChestEntry(chest.X, chest.Y));
                foreach (Door door in list.Where(d => !d.Edge).Concat(list.Where(d => d.Edge)).DistinctBy(d => (d.Room, d.DestX, d.DestY)))
                    nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
                nineROM.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
                int nineROMAt = Allocate(ref listCursor, nineROM.Count, 4), entry = Allocate(ref listCursor, 8, 4);
                nineROM.ToArray().CopyTo(rom, nineROMAt);
                WriteInts(rom, entry, nineROMAt + 0x08000000, chests.Count);
                return entry + 0x08000000;
            }
            // (the hub's, with its door to the ability room last: the game's door visits look for a door in its list, endlessly);
            List<int> doorLists = new List<int> { DoorsList(abilityDoor == null ? mainHubDoors : mainHubDoors.Append(abilityDoor), new List<NidlChest>()) };
            doorLists.AddRange(rooms.Select(r => DoorsList(doors[r], chestsOf[r].Where(c => !c.Battery).ToList())));
            cursor = listCursor;

            // The tables, copied with the new entries;
            (string table, int size, List<int> entries)[] tables = {
                ("gForegroundInfo", tableSizes[10], infos), ("gForegroundPalettes", tableSizes[11], palettes),
                ("gForegroundTilemaps", tableSizes[12], tilemaps), ("gSolidityMaps", tableSizes[13], solidities),
                ("gRoomTiledBGs", tableSizes[15], backgrounds), ("gLevelObjLists", tableSizes[16], objects),
                ("gUnk_08D63C28", tableSizes[17], navigations), ("gUnk_08D640A4", SharedDoors, doorLists),
            };
            Dictionary<string, int> moved = new Dictionary<string, int>();
            foreach ((string table, int size, List<int> entries) in tables) {
                int at = moved[table] = Allocate(ref cursor, (size + entries.Count) * 4, 4);
                Array.Copy(rom, symbols[table], rom, at, size * 4);
                WriteInts(rom, at + size * 4, entries.ToArray());
                Repoint(rom, symbols[table] + 0x08000000, at + 0x08000000);
                // and the expansion's code's (rooms.c's copy of FillLevelInfo reads them);
                for (int i = CodeAddress; i < CodeAddress + version.Code.Length; i += 4)
                    if (ReadInt(rom, i) == symbols[table] + 0x08000000) WriteInt(rom, i, at + 0x08000000);
            }

            PortedRooms = rooms;
            PortedDoors = doors;
            DoorsTable = moved["gUnk_08D640A4"];
            chestLists = chestLists.Select(c => (c.room, moved["gUnk_08D640A4"] + (FirstDoorsList + c.list) * 4, c.objects)).ToList();
            PortedMainHub = (mainHub, mainHubDoors);

            /* The map tracker's rooms block, in the map table (see WriteMapTable(): its mini-bosses end before it): "NR", the NiDL hub's
             * room, then where the room tables are (gSolidityMaps, gForegroundTilemaps, gForegroundPalettes, gForegroundInfo,
             * gRoomTiledBGs, gLevelObjLists: the copies; gUnk_08D6CD0C: the areas); */
            System.Text.Encoding.ASCII.GetBytes("NR").CopyTo(rom, MapRoomsBlock);
            WriteShort(rom, MapRoomsBlock + 2, mainHub);
            WriteInts(rom, MapRoomsBlock + 4, new[] { "gSolidityMaps", "gForegroundTilemaps", "gForegroundPalettes", "gForegroundInfo", "gRoomTiledBGs", "gLevelObjLists" }
                .Select(t => moved[t] + 0x08000000).Append(roomInfos + 0x08000000).ToArray());
            editedRooms = edited.Select(e => new EditedRoom(e.room, moved["gSolidityMaps"] + e.solidity * 4, e.objects < 0 ? 0 : moved["gLevelObjLists"] + e.objects * 4,
                                                            moved["gUnk_08D640A4"] + Props(e.room, 18) * 4, e.tilemap, e.doors)).ToList();
            HashSet<int> ported = rooms.Select(r => r.Room).Append(mainHub).ToHashSet();
            katamLandings = doors.Values.SelectMany(list => list).Concat(mainHubDoors).Where(d => !d.Edge && !ported.Contains(d.Room) && Props(d.Room, 16) != 0xFFFF)
                                 .GroupBy(d => moved["gLevelObjLists"] + Props(d.Room, 16) * 4).ToDictionary(g => g.Key, g => g.Select(d => (d.DestX - 1, d.DestY - 1)).ToList());
            finishCursor = cursor;
            return null;
        }

        /* WarpFlights(); The ported warp stars' flights (their subtype): KatAM's flights (gUnk_08D60A84, 6, each a script of Kirby's
         * speed frame by frame, sub_080534D0: 12-byte steps, x and y speed (8.8 pixels a frame, up positive), frames, Kirby's animation
         * (90: on the star), an effect (12: the star's sparkles; 0xFF none), flags (0x40 a room change, 0x80 the end: Kirby hops off))
         * fly him up, take him to the star's room (the step flagged 0x40; he arrives where gUnk_0893CB44 says for that room), then fly
         * him in along a path drawn for that room alone, which took him out of the NiDL rooms. The table moves (the code's pointer to
         * it with it) to hold these:
         * • WarpFlight, a star with no ride of NiDL's: flight 0 up to its room change, then Kirby lands where the star says (the
         *   rooms' gUnk_0893CB44 entries) and hops off, as at a flight's end;
         * • after it, each star's NiDL ride (Transport.Ride, NidlRides), its moves as NiDL's star makes them: a frame to where NiDL's
         *   star jumps as Kirby gets on (from the KatAM star's spot, where he is), then the star's moves, sparkling where it flies (2
         *   pixels a frame and more); before each room change, the star's moves in NiDL's fade to white (RideLeg.Fade), KatAM's
         *   own fade to white started on the first (effect 0, as KatAM's flights start it before their room change, which loads the
         *   next room under the white); into each room after the first, a room change marked for "ROM Expansion/warpstar.c" (+8
         *   NIDL_RIDE_ROOM, +0xA the room, +0 and +2 the cell Kirby comes in at: KatAM's flights change room once, to the star's
         *   room), then a frame to where NiDL's star comes in, and its moves there; at the last room's end, Kirby hops off: the orb
         *   where the star goes, then his tumble as NiDL's (RideLeg.Hop, RideLeg.Bounce), as KatAM's hop-off draws it (tumbling,
         *   bouncing), to where he stands; the end (+8 NIDL_RIDE_HOP, not 0x80: warpstar.c ends the flight there) leaves him as he
         *   is, facing where he went; where NiDL draws its star smaller (RideLeg.Scale: flying off into the distance), the steps carry
         *   the size (+0xA: NiDL's matrix pa, 256 / the size in 8.8; 0 his own), warpstar.c drawing Kirby and his star at it. As
         *   KatAM draws Kirby after his step's run (sub_080534D0, where it moves on to the next step on its last frame), a step
         *   carries the size of the frame before it, so each frame is drawn at NiDL's size for it. The rooms'
         *   gUnk_0893CB44 entries (where Kirby comes back if he's lost after a flight) are the star's landing (DestX, DestY), and in
         *   the room a ride lands in, where Kirby hops off;
         * warpstar.c also makes the ported stars leave at once (KatAM's revs 300 frames with Kirby on it for the helpers); */
        const int WarpFlight = 6, FlightStep = 12, FlightRise = 201, RideRoom = 0xFE, RideHop = 0xFD, Sparkles = 12, FlightRider = 90;
        // (Kirby's animations tumbling, KatAM's: 74, then 89 down to 75, round again; bouncing off the floor; the effect making the orb,
        // 0x292, where he hops off);
        const int Tumbling = 74, Bouncing = 20, HopOrb = 10;
        // (a flight step's effect starting KatAM's fade to white, sub_0803CA20: its flights' before their room change);
        const int FlightFade = 0;
        static void WarpFlights(byte[] rom, IReadOnlyDictionary<string, int> symbols, ref int cursor) {
            int table = symbols["gUnk_08D60A84"], arrivals = symbols["gUnk_0893CB44"], first = ReadInt(rom, table) - 0x08000000;
            if (rom[first + (FlightRise - 1) * FlightStep + 9] != 0x40) return;     // (its room change: +9, the flags);
            List<Transport> rides = transportsOf.Values.SelectMany(list => list).Where(t => !t.Cannon && t.Ride != null).OrderBy(t => t.Flight).ToList();
            int flight = Allocate(ref cursor, (FlightRise + 2) * FlightStep, 4), moved = Allocate(ref cursor, (WarpFlight + 1 + rides.Count) * 4, 4);
            Array.Copy(rom, first, rom, flight, FlightRise * FlightStep);
            WriteShorts(rom, flight + FlightRise * FlightStep, 0, 0, 1, FlightRider);  // A frame still (it moves on to the end);
            WriteShorts(rom, flight + FlightRise * FlightStep + 8, 0xFF, 0);
            WriteShorts(rom, flight + (FlightRise + 1) * FlightStep, 0, 0, 0, FlightRider);   // The end: Kirby hops off;
            WriteShorts(rom, flight + (FlightRise + 1) * FlightStep + 8, 0x80FF, 0);
            Array.Copy(rom, table, rom, moved, WarpFlight * 4);
            WriteInt(rom, moved + WarpFlight * 4, flight + 0x08000000);
            Repoint(rom, table + 0x08000000, moved + 0x08000000);
            foreach (Transport t in transportsOf.Values.SelectMany(list => list).Where(t => !t.Cannon)) {
                rom[arrivals + t.Room * 2] = (byte)t.DestX;
                rom[arrivals + t.Room * 2 + 1] = (byte)t.DestY;
            }
            foreach (Transport t in rides) {
                List<byte[]> steps = new List<byte[]>();
                void Step(int xSpeed, int ySpeed, int frames, int effect = 0xFF, int flags = 0, int room = 0, int animation = FlightRider, int scale = 0) {
                    byte[] step = new byte[FlightStep];
                    WriteShorts(step, 0, xSpeed, ySpeed, frames, animation);
                    step[8] = (byte)effect;
                    step[9] = (byte)flags;
                    WriteShort(step, 10, effect == RideRoom ? room : scale);
                    steps.Add(step);
                }
                // (from where Kirby is, 8.8 pixels, to a spot: a frame; then the moves, and before a room change those in the fade, the
                // fade started on its first, in steps of a move, an effect and a size: move k carries the size of the frame before it, the
                // leg's start's for the first, RideLeg.Scale[k]);
                void Move(int fromX, int fromY, int x, int y) => Step(x * 256 - fromX, fromY - y * 256, 1);
                void Moves(RideLeg leg, bool fade) {
                    List<(int dx, int dy)> path = leg.Moves.Concat(fade ? leg.Fade : Array.Empty<(int dx, int dy, int frames)>()).SelectMany(m => Enumerable.Repeat((m.dx, m.dy), m.frames)).ToList();
                    int fadeAt = fade ? leg.Moves.Sum(m => m.frames) : -1;
                    int Effect(int k) => k == fadeAt ? FlightFade : Math.Abs(path[k].dx) + Math.Abs(path[k].dy) >= 2 ? Sparkles : 0xFF;
                    for (int k = 0, n; k < path.Count; k += n) {
                        for (n = 1; k + n < path.Count && path[k + n] == path[k] && Effect(k + n) == Effect(k) && leg.SizeAt(k + n) == leg.SizeAt(k); n++) { }
                        Step(path[k].dx * 256, -path[k].dy * 256, n, Effect(k), scale: leg.SizeAt(k));
                    }
                }
                Move(t.X * 256, t.Y * 256, t.Ride[0].X, t.Ride[0].Y);
                Moves(t.Ride[0], t.Ride.Length > 1);
                foreach (RideLeg leg in t.Ride.Skip(1)) {
                    // (KatAM's room change leaves Kirby at the cell's middle less 8 pixels, its top less 7: sub_080562D0);
                    int cellX = (leg.X + 16) / 16, cellY = (leg.Y + 15) / 16;
                    Step(cellX, cellY, 1, RideRoom, 0x40, leg.Room);
                    Move(cellX * 0x1000 - 0x800, cellY * 0x1000 - 0x701, leg.X, leg.Y);
                    Moves(leg, leg != t.Ride[^1]);
                }
                // (his tumble, a frame at a time, facing where he goes, then his bounce, to where he stands);
                var hop = t.Ride[^1].Hop.SelectMany(m => Enumerable.Repeat((m.dx, m.dy), m.frames)).ToList();
                int facing = hop.Sum(m => m.dx) < 0 ? 1 : 0;
                for (int k = 0; k < hop.Count; k++)
                    Step(hop[k].dx * 256, -hop[k].dy * 256, 1, k == 0 ? HopOrb : 0xFF, facing, animation: Tumbling + ((16 - k) & 15));
                foreach ((int dx, int dy, int frames) in t.Ride[^1].Bounce) Step(dx * 256, -dy * 256, frames, 0xFF, facing, animation: Bouncing);
                Step(0, 0, 0, RideHop, facing, animation: Bouncing);
                int script = Allocate(ref cursor, steps.Count * FlightStep, 4);
                for (int i = 0; i < steps.Count; i++) Array.Copy(steps[i], 0, rom, script + i * FlightStep, FlightStep);
                WriteInt(rom, moved + t.Flight * 4, script + 0x08000000);
                RideLeg last = t.Ride[^1];
                (int x, int y) landing = (last.End.x + last.Hop.Concat(last.Bounce).Sum(m => m.dx * m.frames), last.End.y + last.Hop.Concat(last.Bounce).Sum(m => m.dy * m.frames));
                rom[arrivals + last.Room * 2] = (byte)(landing.x / 16 + 1);
                rom[arrivals + last.Room * 2 + 1] = (byte)(landing.y / 16 + 1);
            }
        }

        /* WarpRideDetours(); "ROM Expansion/warpstar.c"'s detours (see WarpFlights()): KatAM's warp star's rev (sub_0800C270), a
         * flight's room change (sub_08055D9C) and its step (sub_080534D0), Kirby's drawing (sub_0803F46C), their trampolines in
         * gNidlRideTrampolines, their addresses in gNidlRide; and the task of the star Kirby rides (sub_0808EEF4: its one pointer,
         * sub_0808EDB8's, which starts it) becomes NidlRideStar, which runs it (its first instructions keep gCurTask in r3, which a
         * detour's jump back would need). Returns an error, or null; */
        static string WarpRideDetours(byte[] rom, KatAMExpansionCode.Version version) {
            int cursor = CodeAddress + version.CodeSymbols["gNidlRideTrampolines"], ride = CodeAddress + version.CodeSymbols["gNidlRide"];
            if (ReadInt(rom, ride) != 0) return null;  // (applied already);
            int starTask = version.RomSymbols["sub_0808EEF4"] + 0x08000000 + 1;
            List<int> starPointers = Enumerable.Range(0, 0x400000 / 4).Select(i => i * 4).Where(at => ReadInt(rom, at) == starTask).ToList();
            if (starPointers.Count != 1) return $"The {version.Name} ROM's warp star code doesn't match.";
            int rev = Detour(rom, version.RomSymbols["sub_0800C270"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlRideRev"], ref cursor);
            int change = rev < 0 ? -1 : Detour(rom, version.RomSymbols["sub_08055D9C"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlRideRoomChange"], ref cursor);
            int step = change < 0 ? -1 : Detour(rom, version.RomSymbols["sub_080534D0"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlRideStep"], ref cursor);
            int draw = step < 0 ? -1 : Detour(rom, version.RomSymbols["sub_0803F46C"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlRideDraw"], ref cursor);
            if (rev < 0 || change < 0 || step < 0 || draw < 0) return $"The {version.Name} ROM's warp star code doesn't match.";
            WriteInt(rom, ride, rev + 0x08000000 + 1);
            WriteInt(rom, ride + 4, change + 0x08000000 + 1);
            WriteInt(rom, ride + 8, step + 0x08000000 + 1);
            WriteInt(rom, ride + 12, draw + 0x08000000 + 1);
            WriteInt(rom, starPointers[0], CodeAddress + 0x08000000 + version.CodeSymbols["NidlRideStar"] + 1);
            return null;
        }

        /* ExtendAreas(); Areas AreaBase... for the NiDL levels (see Areas):
         * • gUnk_08D6DCAC (the CPU helpers' routes, per area: 0-10) is copied with the Dimension Mirror's (no routes, as KatAM's rooms
         *   of area 10) for each new area, and repointed;
         * • AreaPlates (gUnk_08D60EE4: per language, a block of 0x6900 bytes ending with the area plates, 0x200 each from +0x5300, 4bpp,
         *   8x2 tiles: the area's number and name on two lines; the HUD copies area * 0x200 from there, so area 11's would be the next
         *   block's start): each block is copied with a plate per new area, composed of the letters of the game's English plates
         *   (ComposePlate()), or the Dimension Mirror's blank plate where they can't be read; */
        const int PlateBlockSize = 0x6900, PlatesAt = 0x5300, PlateSize = 0x200, VanillaAreas = 11;
        static void ExtendAreas(byte[] rom, IReadOnlyDictionary<string, int> symbols, ref int cursor) {
            int routes = symbols["gUnk_08D6DCAC"], newRoutes = Allocate(ref cursor, (AreaBase + 8) * 4, 4);
            Array.Copy(rom, routes, rom, newRoutes, VanillaAreas * 4);
            for (int area = VanillaAreas; area < AreaBase + 8; area++) WriteInt(rom, newRoutes + area * 4, ReadInt(rom, routes + 10 * 4));
            Repoint(rom, routes + 0x08000000, newRoutes + 0x08000000);

            int table = symbols["AreaPlates"];
            List<int> blocks = Enumerable.Range(0, 6).Select(i => ReadInt(rom, table + i * 4)).Where(b => (b >> 24) == 8).Distinct().ToList();
            Dictionary<char, bool[,]> letters = blocks.Select(b => PlateLetters(rom, b - 0x08000000)).FirstOrDefault(l => l != null);
            Dictionary<int, int> copies = new Dictionary<int, int>();
            foreach (int block in blocks) {
                int from = block - 0x08000000, at = Allocate(ref cursor, PlateBlockSize + 8 * PlateSize, 4);
                Array.Copy(rom, from, rom, at, PlateBlockSize);
                for (int level = 0; level < 8; level++) {
                    byte[] plate = letters == null ? null : ComposePlate(letters, LevelNames[level].first, LevelNames[level].second);
                    if (plate == null) Array.Copy(rom, from + PlatesAt + 10 * PlateSize, rom, at + PlatesAt + (AreaBase + level) * PlateSize, PlateSize);
                    else plate.CopyTo(rom, at + PlatesAt + (AreaBase + level) * PlateSize);
                }
                copies[block] = at + 0x08000000;
            }
            for (int i = 0; i < 6; i++) {
                int block = ReadInt(rom, table + i * 4);
                if (copies.TryGetValue(block, out int copy)) WriteInt(rom, table + i * 4, copy);
            }
        }

        /* A plate's pixels (64 x 16): letters (colors 8, the first of a word, and 9) outlined (13), the number ("(1)": its digit in color
         * 8, the brackets 12) before the first line, and on the second, stripes (12) on rows 2, 4, 5 around the name; the English plates'
         * words, to read their letters from: */
        static readonly (string first, string second)[] PlateWords = {
            ("RAINBOW", "ROUTE"), ("MOONLIGHT", "MANSION"), ("CABBAGE", "CAVERN"), ("MUSTARD", "MOUNTAIN"), ("CARROT", "CASTLE"),
            ("OLIVE", "OCEAN"), ("PEPPERMINT", "PALACE"), ("RADISH", "RUINS"), ("CANDY", "CONSTELLATION"),
        };
        static int[,] PlatePixels(byte[] rom, int plate) {
            int[,] pixels = new int[16, 64];
            for (int y = 0; y < 16; y++)
                for (int x = 0; x < 64; x++) {
                    int b = rom[plate + ((y / 8) * 8 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2];
                    pixels[y, x] = (x & 1) != 0 ? b >> 4 : b & 15;
                }
            return pixels;
        }

        /* PlateLetters(); The letters of a block's English plates (their letter pixels: 6 rows), or null if they aren't English ones;
         * a run of letters touching each other (CANDY's D and Y) is split by the widths the other plates give its first letters; */
        static Dictionary<char, bool[,]> PlateLetters(byte[] rom, int block) {
            Dictionary<char, bool[,]> letters = new Dictionary<char, bool[,]>();
            List<(int[,] pixels, int top, string word, List<(int from, int to)> runs)> lines = new List<(int[,], int, string, List<(int, int)>)>();
            for (int area = 0; area < PlateWords.Length; area++) {
                int[,] pixels = PlatePixels(rom, block + PlatesAt + area * PlateSize);
                foreach ((int top, string word) in new[] { (1, PlateWords[area].first), (9, PlateWords[area].second) }) {
                    // The columns with letter pixels, in runs: one run per letter (the first line's first: the number's digit);
                    List<(int from, int to)> runs = new List<(int, int)>();
                    for (int x = 0; x < 64; x++) {
                        if (!Enumerable.Range(top, 6).Any(y => pixels[y, x] is 8 or 9)) continue;
                        if (runs.Count > 0 && runs[^1].to == x - 1) runs[^1] = (runs[^1].from, x);
                        else runs.Add((x, x));
                    }
                    if (top == 1 && runs.Count > 0) runs.RemoveAt(0);
                    if (runs.Count > word.Length) return null;
                    lines.Add((pixels, top, word, runs));
                }
            }
            void Take(int[,] pixels, int top, char letter, int from, int to) {
                bool[,] glyph = new bool[6, to - from + 1];
                for (int y = 0; y < 6; y++)
                    for (int x = from; x <= to; x++) glyph[y, x - from] = pixels[top + y, x] is 8 or 9;
                letters.TryAdd(letter, glyph);
            }
            foreach (var line in lines.Where(l => l.runs.Count == l.word.Length))
                for (int i = 0; i < line.word.Length; i++) Take(line.pixels, line.top, line.word[i], line.runs[i].from, line.runs[i].to);
            foreach (var line in lines.Where(l => l.runs.Count < l.word.Length)) {
                List<(int from, int to)> runs = new List<(int, int)>(line.runs);
                for (int i = 0, j = 0; i < line.word.Length && j < runs.Count; i++, j++) {
                    int width = runs[j].to - runs[j].from + 1;
                    if (runs.Count - j < line.word.Length - i && letters.TryGetValue(line.word[i], out bool[,] known) && known.GetLength(1) < width) {
                        Take(line.pixels, line.top, line.word[i], runs[j].from, runs[j].from + known.GetLength(1) - 1);
                        runs.Insert(j + 1, (runs[j].from + known.GetLength(1), runs[j].to));
                    } else Take(line.pixels, line.top, line.word[i], runs[j].from, runs[j].to);
                }
            }
            // F: E without its bottom bar;
            if (letters.TryGetValue('E', out bool[,] e) && !letters.ContainsKey('F')) {
                bool[,] f = (bool[,])e.Clone();
                for (int x = 1; x < f.GetLength(1); x++) f[5, x] = false;
                letters['F'] = f;
            }
            return letters;
        }

        /* ComposePlate(); A plate: the first line left-aligned from x 3, the second right-aligned to x 58 between stripes, as KatAM's
         * (without a number); null if a letter is missing or a line doesn't fit; */
        static byte[] ComposePlate(Dictionary<char, bool[,]> letters, string first, string second) {
            int[,] pixels = new int[16, 64];
            bool Line(string text, int top, bool right) {
                if (text.Any(c => c != ' ' && !letters.ContainsKey(c))) return false;
                int width = 0;
                for (int i = 0; i < text.Length; i++)
                    width += text[i] == ' ' ? 3 : letters[text[i]].GetLength(1) + (i + 1 < text.Length && text[i + 1] != ' ' ? 1 : 0);
                int x = right ? 59 - width : 3;
                if (x < 1 || x + width > 61) return false;
                for (int i = 0; i < text.Length; i++) {
                    if (text[i] == ' ') { x += 3; continue; }
                    bool[,] glyph = letters[text[i]];
                    int color = i == 0 || text[i - 1] == ' ' ? 8 : 9;
                    for (int y = 0; y < 6; y++)
                        for (int c = 0; c < glyph.GetLength(1); c++)
                            if (glyph[y, c]) pixels[top + y, x + c] = color;
                    x += glyph.GetLength(1) + 1;
                }
                // The outline: every empty pixel of the line next to a letter's;
                for (int y = top - 1; y <= top + 6; y++)
                    for (int c = 0; c < 64; c++) {
                        if (pixels[y, c] != 0) continue;
                        bool near = false;
                        for (int dy = -1; dy <= 1; dy++)
                            for (int dx = -1; dx <= 1; dx++) {
                                int yy = y + dy, xx = c + dx;
                                if (yy >= top && yy < top + 6 && xx >= 0 && xx < 64 && pixels[yy, xx] is 8 or 9) near = true;
                            }
                        if (near) pixels[y, c] = 13;
                    }
                return true;
            }
            if (!Line(first, 1, false) || !Line(second, 9, true)) return null;
            foreach (int y in new[] { 10, 12, 13 })
                for (int x = 0; x < 63; x++) if (pixels[y, x] == 0) pixels[y, x] = 12;

            byte[] plate = new byte[PlateSize];
            for (int y = 0; y < 16; y++)
                for (int x = 0; x < 64; x++)
                    plate[((y / 8) * 8 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)((x & 1) != 0 ? pixels[y, x] << 4 : pixels[y, x]);
            return plate;
        }

        /* Door; A door of a ported room (KatAM's 8ROM: a special block record, kind 3, at its cell, collision 0xFF, or 0xFD in water):
         * where it leads (the KatAM room, and the cell Kirby lands in plus one, as the records hold it), and whether a mirror (object
         * 0x6F) stands on it, and which way Kirby faces there (1 right, 2 left: towards the room's middle); an edge (collision 0xFE, 0xFC in
         * water) takes Kirby through as he touches it, without a mirror; a secret one (behind a breakable block in NiDL, see
         * BuildSolidity()) stays a block until it's broken, and its mirror shows then; a shut one (a mini-boss's, see ArenaCells()) is
         * built air until the fight is won; */
        public sealed record Door(int X, int Y, int Room, int DestX, int DestY, byte Facing, bool Edge = false, int Slot = 0, int Design = HubDesign,
                                  bool Secret = false, bool OneWay = false, bool Hidden = false, DoorKind Kind = DoorKind.Nidl, bool Plain = false,
                                  bool Shut = false);
        // DoorKind; Why a door is there (see PlanDoors()): NiDL's own, a stage's goal, the boss stages' chain, a branch into a stage and
        // its way back, a loop, a link between levels or with a KatAM area, a switch room's (its Mirra's way in,
        // its floor mirror), the hubs', the way into NiDL, an edge (a fall or climb out of a room), a split room's parts;
        public enum DoorKind { Nidl, Goal, Boss, Branch, BranchBack, Loop, Link, KatAMLink, SwitchEntry, SwitchFloor, Hub, Entry, Edge, Split }

        /* Transport; A warp star or cannon of a ported room (NiDL's gimmicks 4 and 6, their exits kind 30), as KatAM's own (objects 0x84,
         * 0x8A: BuildObjects()): where it stands (pixels, TransportSpot(): a cannon on a cell's top, a warp star in a cell's middle, so its cell is (Y - 1) / 16), and where it takes
         * Kirby (the room, the landing cell + 1); a warp star's flight (its subtype, see WarpFlights()) and NiDL's ride, where it has one
         * (NidlRides); */
        public sealed record Transport(int X, int Y, bool Cannon, int Room, int DestX, int DestY, int Flight = WarpFlight, RideLeg[] Ride = null);
        static Dictionary<NightmareRoom, List<Transport>> transportsOf = new Dictionary<NightmareRoom, List<Transport>>();
        /* RideLeg; A room a warp star's ride goes through (NidlRides): the room (a ported part), where the star comes into it (its
         * middle, in the part's pixels), its moves frame by frame (pixels a frame, down positive, for so many frames) up to its room
         * change, and where they end; its moves in the fade before the room change (NidlRides, after "/"); the size NiDL draws the
         * star and Kirby at, frame by frame from the leg's start through the fade (its affine matrix's pa: 256 / the size, in 8.8),
         * where it draws them smaller (null: their own size all along); */
        public sealed record RideLeg(int Room, int X, int Y, (int dx, int dy, int frames)[] Moves, (int dx, int dy, int frames)[] Hop = null,
                                     (int dx, int dy, int frames)[] Bounce = null, int[] Scale = null, (int dx, int dy, int frames)[] Fade = null) {
            public (int x, int y) End => (X + Moves.Sum(m => m.dx * m.frames), Y + Moves.Sum(m => m.dy * m.frames));
            // SizeAt(); The size at frame k from the start (Scale's pa), or 0 for the star's own;
            public int SizeAt(int k) => Scale != null && k < Scale.Length && Scale[k] > 256 ? Scale[k] : 0;
        }
        /* NidlRides; NiDL's warp star rides as its stars fly them, recorded from NiDL frame by frame (each star runs a script of its own:
         * the warp star object, type 0x4A, 0x08071030, its flights picked by the record's variant, 0x0873FBC4; see "Documentation/Nightmare
         * in Dream Land Notes.md"): by the star's room (level, stage, room), each room the ride goes through: its index in the stage,
         * where the star is as it comes in (its middle: NiDL draws Kirby on the star as one 32x32 sprite there, as KatAM draws its rider),
         * then its moves ("dx,dy" pixels a frame, down positive; "*n": for n frames) up to the room change (where NiDL's fade to white
         * starts), and after "/" its moves in the fade (18 frames, flown with KatAM's own fade to white, as its flights fade before their
         * room change), up to where NiDL's star goes and the next room loads. The first room's starts
         * where NiDL's star jumps to as Kirby gets on (16 pixels up), then bobs down under him before it's off; the last room's ends where
         * Kirby hops off. Then "hop": his moves as he tumbles off it (NiDL's: up 4 pixels a frame, slowing, then down) to where he first
         * touches the floor, and "bounce": his bounce from there, to where he stands (NiDL's own Kirby stands up to 4 pixels higher than
         * KatAM's on the same floor: he drops onto it). "scale": in the rides where NiDL draws the star and Kirby smaller as they fly off
         * into the distance (its affine matrix's pa, 256 / the size in 8.8, all their pieces scaled around the star's middle), a leg's
         * room, then the pa frame by frame from where the star comes in, through its fade ("pa*n": for n frames; 256 after them). NiDL's
         * landing room shows the last pa for one more frame, under its fade's white: not taken.
         * Grape Garden 3's (3, 2, 2) isn't here: Kirby didn't get on it (it's off until something in its room is done) */
        static readonly Dictionary<(int level, int stage, int room), string[]> NidlRides = new Dictionary<(int, int, int), string[]> {
            // Vegetable Valley 2's;
            [(0, 1, 0)] = new[] {
                "0 888,64 0,0*2 0,1*8 0,2*8 0,1*8 -1,-8 1,-8 0,-8 2,-8 1,-8 4,-8 3,-4 6,-4 5,-4 6,-4 5,-4 6,-4 5,-1 6,-1 5,-1 6,-1 5,-1 6,-1 5,-1 6,0 5,-1 6,0 5,-1 6,0 5,1 6,1 5,1 6,1 5,1 6,1 5,2 6,2 5,2 6,2 5,2 6,2 5,4 6,4 5,4 6,4 5,4 6,4 4,-8*3 2,-8*2 0,-8*2 -1,-8*2 -2,-8*3 / 0,0*18",
                "1 164,496 0,0 -4,-8 -4,-16 -4,-12 -4,-10*4 -4,-8*3 -4,-6*3 -4,-2*3 -4,0*4 -4,2*4 0,2*2 1,-12*4 2,-12*3 4,-10*4 4,-8*4 4,-6*4 4,-2*4 4,0*4 4,2*8 0,-10*2 -6,-10*3 -6,-8*3 -6,-6*3 -6,0*3 -6,2*3 -6,4*3 0,-10*2 4,-6 4,-4*8 4,-2*5 4,-1*11 4,0 4,-1 4,0 4,-1 4,0 4,-1 4,0*2 4,1 4,0 4,1 4,0 4,1 4,0 4,1 / 4,0 4,1 4,0 4,1 4,0 4,1 4,0 4,1 4,0 4,1 4,0 4,1 4,0 4,1 4,0 4,1 4,0 0,0",
                "2 12,14 1,2*58",
                "hop 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,2 1,2 0,2 1,2 0,2 1,2 0,2 1,2 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,1",
                "bounce 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,0*2 0,-2" },
            // Ice Cream Island 2's;
            [(1, 1, 2)] = new[] {
                "0 128,391 0,0 6,-3*6 2,-2*6 1,-1*6 -1,-1 -1,0 -1,-1 -1,0 -1,-1 -1,0 -2,-1 -2,0 -2,-1 -2,0 -2,-1 -2,0 -4,0*6 -6,1 -6,0 -6,1 -6,0 -6,1 -6,0 -4,1*6 -2,2*6 -1,3*6 0,2 1,2 0,2 1,2 0,2 1,2 1,1*6 2,1 2,0 2,1 2,0 2,1 2,0 4,1 4,0 4,1 4,0 4,1 4,0 2,1 2,0 2,1 2,0 2,1 2,0 1,1 1,0 1,1 1,0 1,1 1,0 1,1 1,0 1,1 1,0 1,1 1,0 1,1 1,0*7 0,0 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1*2 0,-3*5 1,-3 -1,-3 0,-3*3 0,-5*4 -1,-5 0,-5 -1,-5 0,-5 -1,-5 0,-5 -1,-5 0,-5 -1,-5*9 0,-6 -1,-5 0,-6 -1,-5 0,-6 -1,-5 0,-6 0,-8*10 0,-10*4 0,-8*2 0,-6 0,-4*3 0,-6*16 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,0 1,0*3 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0*2 0,1 0,0 0,1 0,0*2 0,1 0,0*2 0,1 0,0 0,1 0,0*2 0,1 0,0*2 0,1 0,0 0,1 0,0*2 0,1 0,0*2 0,1 0,0 0,1 0,0*2 0,1 0,0*2 0,1 0,0 0,1 0,0*2 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*3 0,1 0,0*4 0,1 0,0*7 0,1 0,0*7 0,1 0,0*7 0,1 0,0*7 0,1 0,0*9 0,1 0,0*15 0,1 0,0*15 0,1 0,0*4 / 0,0*11 0,1 0,0*6",
                "3 12,14 1,2*58",
                "scale 0 256*105 260*6 264*7 268*7 273*6 277*7 282*7 287*7 292*6 297*7 303*7 309*7 315*6 321*7 327*7 334*7 341*6 348*7 356*7 364*7 372*6 381*6 1024*21 1092*21 1170*22 1260*21 1365*21 1489*22 1638*21 1820*21 2048*22 2340*21 2730*4",
                "hop 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,2 1,2 0,2 1,2 0,2 1,2 0,2 1,2 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,1",
                "bounce 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,0*2 0,-2" },
            // Butter Building 4's;
            [(2, 3, 0)] = new[] {
                "0 128,103 0,0*2 0,1 0,0 0,1 0,0 0,1*7 0,0 0,1 0,0 0,1 0,0 0,1 -1,-1 0,0 -1,-1 0,0 -1,-1*4 -3,-4*4 -3,-2*4 -3,-1*4 -3,1*4 -3,2*4 -2,2*8 -1,1*8 -1,0 0,1 -1,0 0,1 -1,0 0,1 -1,0 0,1 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 2,-2*2 -2,2*2 8,-6*12 6,-6*4 4,-6*4 3,-6*4 / 0,0*18",
                "1 64,1720 8,-2 8,-14 8,-8*10 6,-8*6 4,-8*6 2,-8*4 1,-8*4 0,-8 1,-8 0,-8 1,-8 -1,-8 -1,-7*3 -2,-7*4 -4,-7 -4,-6 -4,-7 -4,-6 -8,-3*18 -4,-5*4 -2,-5*2 2,-7 2,-6 4,-7*4 8,-8*12 6,-8*6 4,-8*6 2,-8*4 1,-8*4 0,-8 1,-8 0,-8 1,-8 -1,-8 -1,-7*3 -2,-7*4 -4,-7 -4,-6 -4,-7 -4,-6 -8,-3*18 -4,-5*4 -2,-5*2 2,-7 2,-6 4,-7*4 8,-8*12 6,-8*6 4,-8*6 2,-8*4 1,-8*4 0,-8 1,-8 0,-8 1,-8 -1,-8 -1,-7*3 -2,-7*4 -4,-7 -4,-6 -4,-7 -4,-6 -8,-3*18 -4,-5*4 -2,-5*2 2,-7 2,-6 4,-7*4 8,-8*4 8,-9*4 8,-12*4 8,-14*8 0,-6*2 0,0*2 / 0,0*18",
                "2 78,197 6,-3*13 6,-4*4 6,-6*6 -6,6 6,-6*2 -6,6 6,-6*2 -6,6 0,-6*2 0,6 0,-6*2 0,6 0,-6*2 0,6 0,-2 0,2 0,-2 0,2 0,-2 0,2 0,0*8 -1,0 0,0*7 -1,1 0,0 -1,1 0,0 -1,1*4 -2,2*4 -4,4*6 -8,4*6",
                "hop -1,-9 0,-4 -1,-4 0,-4 -1,-3 0,-3 -1,-3 0,-3 -1,-2 0,-2 -1,-2 0,-2 -1,-1 0,-1 -1,-2 0,-1 -1,-1 0,-1 -1,-1 0,-1 -1,-1 0,0 -1,-1 0,-1 -1,0 0,-1 -1,0 0,-1 -1,1 0,0 -1,1 0,0 -1,1 0,1 -1,0 0,1 -1,1 0,1 -1,1 0,1 -1,1 0,2 -1,1 0,1 -1,2 0,2 -1,2 0,2 -1,3 0,3 -1,3 0,3 -1,4 0,4 -1,4 0,0",
                "bounce -1,-1 0,-1 -1,-1 0,-1 -1,-1 0,-1 -1,-1 0,-1 -1,0 0,-1 -1,0 0,-1 -1,0 0,-1 -1,0 0,-1 -1,1 0,0 -1,1 0,0 -1,1 0,0 -1,1 0,0 -1,1 0,1 -1,1 0,1 -1,1 0,1 -1,1 0,1 -1,0 0,0 0,-2" },
            // Yogurt Yard 5's;
            [(4, 4, 1)] = new[] {
                "0 216,119 0,0 0,6*2 0,4*2 0,3 0,2 0,1 0,2 0,1 0,0*4 0,-1 0,-2 0,-1 0,-2 0,-3*4 0,-4 0,-5 0,-4 0,-5*2 0,-4 0,-3*3 0,-1 0,-2 0,0*3 0,1*2 0,2 0,3*2 0,4*2 0,5*2 0,6 0,0 -2,0 -3,2 -2,1 -3,-4 -2,-3 -2,1 -3,2 -2,-4*2 -2,2 -3,2 -2,-4*2 -3,2 -2,1 -2,-4 -2,-3 -3,1 -2,2 -3,-4 -2,-4 -2,2 -3,1 -2,-3 -2,-4 -2,2 -3,1 -2,-4*2 -2,2 -3,2 -2,-4*2 -3,2 -2,1 -3,-4 -2,-3 -2,1 -3,2 -2,-4*2 -2,2 -3,2 -2,-4*2 -2,2 -3,1 -2,-4 -2,-3 -4,0*2 -3,0*3 -2,0*5 -1,0*2 -1,1 0,0 0,1 0,2 0,1*2 1,2*3 2,2 2,3 2,2 2,3*2 3,3 3,4 3,3 4,3 4,2 4,3 4,2*2 5,1 5,2 5,1 7,1 6,-1 7,0 7,-1*2 8,-2 7,-2 8,-2 8,-3 9,-2 9,-4 9,-3 9,-4 10,-4*2 9,-5 11,-5 10,-5 11,-6*3 8,-5*2 8,-4 8,-3 7,-2 8,-2 5,0 4,0 5,1 3,1 3,3 2,3 2,4 1,5 1,6 2,5*2 4,4 5,4 6,4 6,3 8,3 9,2*2 11,1 12,1 12,0 14,0 15,0 15,-1 17,-1 18,-2*2 20,-3 21,-3 19,-2 16,-1 15,-1 13,-1 10,-1 9,-1 7,0 4,0 3,0 1,1 -2,1 -3,1 -5,1 -8,1 -9,2 -8,2 -6,2 -5,1 -3,2 -2,2 0,2 1,1 3,2 4,2 6,2 7,1 8,2 11,2*2 13,1 15,2 16,2 18,2 19,1 21,2 21,1 20,0 21,-2 20,-2 20,-3 20,-4 19,-6*2 18,-7 18,-8 18,-10 16,-10 17,-11 16,-12 17,-14 16,-14 17,-15 17,-16 17,-18*2 17,-19 17,-20 17,-22*2 17,-23 18,-24 17,-26 18,-26 17,-27 18,-28 4,0*10 / 4,0*17 0,0",
                "2 12,14 0,0 1,2*58",
                "hop 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,2 1,2 0,2 1,2 0,2 1,2 0,2 1,2 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,1",
                "bounce 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,0*2 0,-2" },
            // Orange Ocean 3's;
            [(5, 2, 3)] = new[] {
                "0 408,31 0,0*2 -8,2*8 -4,2*8 -2,1*8 -1,0 -1,1 -1,0 -1,1 -1,0 -1,1 -1,0 -1,1 1,-1 2,-1*7 3,-2 5,-2*7 7,-4 8,-4*3 10,-2*4 10,-1*4 8,0 8,1 8,0 8,2*4 7,4*4 6,6*4 5,8*2 3,8*2 2,6*4 1,4*4 0,2*2 -1,2*2 -3,1*5 -3,0 -4,1 -4,0 -2,0*4 -1,0*2 -3,0*2 -2,0 -2,-1 1,-16*4 2,-8*13 / 0,0*18",
                "5 88,310 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-1 0,-1 1,-1 0,-2 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,0 0,-1 1,-1 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,0 1,0 0,-1 1,0 0,0 1,0 0,-1 1,0 0,0 1,0 0,0 1,0 0,0 1,0 0,0 1,0 0,0 1,0 0,0 1,0 0,0 1,1 0,0 1,0 0,0 1,1 0,0 1,0 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 / 0,0 0,1 0,0 0,1 0,0 0,1 0,0 1,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0 0,1 0,0*2",
                "4 11,30 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*3 0,2 1,2*2",
                "scale 5 256*2 260*2 264*3 268*3 273*3 277*4 282*3 287*3 292*3 297*3 303*4 309*3 315*3 321*3 327*3 334*4 341*3 348*3 356*3 364*3 372*4 381*3 390*3 399*3 409*3 420*4 431*3 442*3 455*3 468*3 481*4 496*3 512*3 528*3 546*3 564*4 585*3 606*3 630*3 655*3 682*4 712*3 744*3 780*3 819*3 862*4 910*3 963*3 1024*3 1092*3 1170*4 1260*3 1365*3 1489*3 1638*3 1820*4 2048*3 2340*3 2730*3 3276*3 4096*4 5461",
                "hop 0,-4*3 1,-4 0,-4*3 1,-4 0,-2*3 1,-2 0,-2*3 1,-2 0,-1*3 1,-1 0,-1*3 1,-1 0,-1 0,0 0,-1 1,0 0,-1 0,0 0,-1 1,0 0,0 0,1 0,0 1,1 0,0 0,1 0,0 1,1 0,1*3 1,1 0,1*3 1,1 0,2*3 1,2 0,2*3 1,2 0,4*3 1,4 0,4*3 1,4 0,4 0,1",
                "bounce 0,-1 1,-1 0,-1*3 1,-1 0,-1*2 0,0 1,-1 0,0 0,-1 0,0 1,-1 0,0 0,-1 0,1 1,0 0,1 0,0 0,1 1,0 0,1 0,0 0,1 1,1 0,1*3 1,1 0,1*2 0,0*2 0,-2" },
            // Rainbow Resort 2's;
            [(6, 1, 10)] = new[] {
                "0 136,471 0,0*2 0,1*8 1,0 1,1 1,0 1,1 1,0 1,1 1,0 1,1 1,0*7 1,1 1,-1 1,0 1,-1 1,0 1,-1 1,0 1,-1 1,0 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 -1,-2*8 -2,-1*9 -2,0 -2,-1 -2,0 -2,-1 -2,0 -2,-1 -2,0 -2,1*8 -2,2*8 -2,4*2 -2,5 -2,4 -2,5*4 -1,6 -1,7*2 -1,8 -1,5 0,5 -1,5 0,8 -1,5 0,6 0,5 0,6 0,4 0,7*3 0,6 0,7 0,6 0,7 0,6*8 -1,6 0,6*2 0,4*2 0,5 0,4*3 0,2*6 0,1*4 0,0 0,1 0,0 0,1 0,0*4 0,-1 0,0 0,-1 0,2 0,1*2 0,0 0,-1*2 0,0 0,-1 0,-2 0,-3*5 1,-3 0,-5*6 0,-7 1,-7 0,-7*2 1,-7 0,-7 1,-7 0,-7 1,-7 0,-7 1,-7 0,-7 1,-7 0,-7 1,-7 0,-7 1,-7 0,-7 1,-7*20 2,-7 1,-7*3 2,-7 1,-7*3 2,-7 1,-7*3 2,-7*17 3,-7*14 3,-5*6 3,-3*6 3,-2*7 3,-1 3,-2 3,-1 3,-2 3,-1*3 / 0,0*18",
                "24 17,102 6,-2*7 6,-1*9 6,0 6,-1 6,0*2 6,1 6,0 6,1*7 6,2*8 / 6,2*17 0,0",
                "11 12,14 1,2*62",
                "hop 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-4 1,-4 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-2 1,-2 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,-1 1,0 0,0 1,1 0,0 1,1 0,0 1,1 0,0 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,2 1,2 0,2 1,2 0,2 1,2 0,2 1,2 0,4 1,4 0,4 1,4 0,4 1,4 0,4 1,1",
                "bounce 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,-1 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,0 1,-1 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,0 0,1 1,1 0,1 1,1 0,1 1,1 0,1 1,1 0,0*2 0,-2" },
        };
        // Ride(); A warp star's ride (NidlRides), its rooms the parts holding them, or null;
        static RideLeg[] Ride((int level, int stage, int room) star, Func<(int, int, int), int, int, (NightmareRoom part, int x, int y)> partAt) {
            if (!NidlRides.TryGetValue(star, out string[] legs)) return null;
            static (int, int, int)[] Moves(IEnumerable<string> words) => words.Select(word => {
                string[] pair = word.Split('*');
                int[] d = pair[0].Split(',').Select(int.Parse).ToArray();
                return (d[0], d[1], pair.Length > 1 ? int.Parse(pair[1]) : 1);
            }).ToArray();
            // (a leg's sizes, by its room: "pa*n" for n frames);
            Dictionary<string, int[]> scales = legs.Where(leg => leg.StartsWith("scale ")).Select(leg => leg.Split(' ', StringSplitOptions.RemoveEmptyEntries))
                .ToDictionary(words => words[1], words => words.Skip(2).SelectMany(word => {
                    string[] pair = word.Split('*');
                    return Enumerable.Repeat(int.Parse(pair[0]), pair.Length > 1 ? int.Parse(pair[1]) : 1);
                }).ToArray());
            RideLeg[] ride = legs.Where(leg => !leg.StartsWith("hop ") && !leg.StartsWith("bounce ") && !leg.StartsWith("scale ")).Select(leg => {
                string[] halves = leg.Split(" / "), words = halves[0].Split(' ', StringSplitOptions.RemoveEmptyEntries);
                int[] at = words[1].Split(',').Select(int.Parse).ToArray();
                var part = partAt((star.level, star.stage, int.Parse(words[0])), at[0] / 16, at[1] / 16).part;
                return new RideLeg(part.Room, at[0] - part.X * 16, at[1] - part.Y * 16, Moves(words.Skip(2)), Scale: scales.GetValueOrDefault(words[0]),
                                   Fade: halves.Length > 1 ? Moves(halves[1].Split(' ', StringSplitOptions.RemoveEmptyEntries)) : Array.Empty<(int, int, int)>());
            }).ToArray();
            (int, int, int)[] Tail(string kind) => Moves(legs.First(leg => leg.StartsWith(kind + " ")).Split(' ', StringSplitOptions.RemoveEmptyEntries).Skip(1));
            ride[^1] = ride[^1] with { Hop = Tail("hop"), Bounce = Tail("bounce") };
            return ride;
        }
        /* bossExitsOf; Where a boss room's shard takes Kirby (the room, the landing cell + 1), by its room (each part): a boss room has no
         * door out, as KatAM's: taking the shard its boss leaves is the way on (gNidlShardExits, "ROM Expansion/nidl_boss.c": where the
         * shard cutscene leaves Kirby, KatAM's Central Circle otherwise); */
        static Dictionary<NightmareRoom, (int room, int x, int y)> bossExitsOf = new Dictionary<NightmareRoom, (int, int, int)>();
        public static IReadOnlyDictionary<NightmareRoom, (int room, int x, int y)> PortedBossExits => bossExitsOf;
        public static IReadOnlyDictionary<NightmareRoom, List<Transport>> PortedTransports => transportsOf;

        // The KatAM room with the mirror to Nightmare in Dream Land: Central Circle, on its platform's left end (FinishRooms());
        const int EntryRoom = 0x321, EntryX = 11, EntryY = 14;
        /* EditedRoom; A KatAM room getting doors (FinishRooms()): its table entries of its own (collision, objects: 0 none; its doors
         * list's), its tilemap (file offset), the doors; */
        sealed record EditedRoom(int Room, int SolidityEntry, int ObjectsEntry, int DoorsEntry, int Tilemap, List<Door> Doors);
        static List<EditedRoom> editedRooms = new List<EditedRoom>();
        // (where the ported rooms' doors land Kirby in KatAM's rooms, by the rooms' object list entries: FinishRooms() keeps KatAM's
        // enemies off them, as BuildObjects() the ported rooms', 3 cells across, 2 up or down);
        static Dictionary<int, List<(int x, int y)>> katamLandings = new Dictionary<int, List<(int, int)>>();
        static int finishCursor;

        /* TransportSpot(); Where a warp star or cannon stands (NiDL's room pixels): a warp star where NiDL's does (in a cell's middle: it
         * floats, bobbing, as NiDL's); a cannon on its fuse, as KatAM's own (rooms 0x7E, 0x143, 0x1F6, 0x33B: x the right edge of the
         * fuse's cell its rope enters, y that cell's top), its fuse the nearest of NiDL's fuse cells (0x38-0x3D) under NiDL's spot (in a
         * cell's middle, 8 pixels up), else on the floor's line under it (up to 3 cells down); */
        static (int x, int y) TransportSpot(byte[] nightmare, NightmareRoom r, int x, int y, bool cannon) {
            if (!cannon) return (x, y);
            byte[] cells = RoomCells(nightmare, r.Record);
            int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
            int Kind(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height ? cells[(cy * width + cx) * 4 + 3] : 0;
            for (int dy = 0; dy <= 3; dy++)
                foreach (int dx in new[] { 0, -1, 1 })
                    if (Kind(x / 16 + dx, y / 16 + dy) is >= 0x38 and <= 0x3D) return ((x / 16 + dx + 1) * 16, (y / 16 + dy) * 16);
            bool Floor(int cy) => Kind(x / 16, cy) is int kind && (RoomCollision[kind] is >= 0x01 and <= 0x1B || kind >> 4 == 2);
            for (int cy = y / 16; cy + 1 < height && cy <= y / 16 + 3; cy++)
                if (Floor(cy + 1)) return (x, (cy + 1) * 16);
            return (x, y);
        }

        /* PlanDoors(); Every ported room's doors, as KatAM mirrors:
         * • NiDL's doors (exits to a room of the same stage) where they were, to where they led;
         * • warp stars and cannons (their exits, kind 30) stay warp stars and cannons, KatAM's own objects where they stood (Transport), to
         *   where Kirby starts in the room they lead to (their landing is where their flight starts: under the tower they fly Kirby up
         *   into, in 3-2);
         * • a stage's goal door (kind 34) leads where the level's structure says (see below: the boss stage, or one-way, ending its branch);
         *   the levels' boss stages (their last) and the Fountain of
         *   Dreams have no doors of their own, so each of their rooms gets a mirror where Kirby lands, to the next room, then to the next
         *   level's first room (the Fountain of Dreams: the NiDL hub, onto its golden mirror); they're one-way, as KatAM's boss rooms:
         *   no mirror back into one, nor out of one;
         * • a room left past its edge (kind 26: its x the room, 0xFFFF none; Kirby falls from the sky out of its bottom, climbs a tower
         *   out of its top) gets edges on its top and bottom rows' open cells;
         * • the hubs: Central Circle's golden mirror (EntryRoom) leads to the NiDL hub (mainHub, its doors in mainHubDoors), whose
         *   mirrors lead to the levels' switch rooms (hidden until their big switch is pressed, Slot) and back, its golden one to
         *   Vegetable Valley's first room, the way in;
         *   a switch room is reached from a Mirra hidden in its level (SecretSpot(); the Fountain of Dreams': in Rainbow Resort), its 2
         *   floor mirrors lead into 2 of the level's branches and back, and the mirror its big switch reveals to where LevelLinks says;
         * • a split room's parts lead into each other through their shared edge.
         * The doors to the hubs' museum and arena (kind 21) aren't ported; */
        static Dictionary<NightmareRoom, List<Door>> PlanDoors(byte[] rom, IReadOnlyDictionary<string, int> symbols, byte[] nightmare, List<NightmareRoom> rooms,
                                                               int mainHub, out List<Door> mainHubDoors,
                                                               out Dictionary<int, List<Door>> katamDoors, out (int room, int x, int y) firstStart, out byte firstFacing) {
            Dictionary<NightmareRoom, List<Door>> doors = rooms.ToDictionary(r => r, r => new List<Door>());
            transportsOf = rooms.ToDictionary(r => r, r => new List<Transport>());
            bossExitsOf = new Dictionary<NightmareRoom, (int, int, int)>();
            // HasBoss(); Whether a room (any of its parts) holds a ported boss (NiDL's category 2, RoomBosses);
            bool HasBoss(NightmareRoom r) {
                int list = ReadInt(nightmare, r.Record + 0x48);
                for (int at = list == 0 ? r.Record : list - 0x08000000; list != 0 && at + 8 <= r.Record; at += 8)
                    if (nightmare[at] - 1 == BossCategory && RoomBosses.ContainsKey(nightmare[at + 1])) return true;
                return false;
            }
            clearedBlocks = new Dictionary<int, HashSet<int>>();
            var byRoom = rooms.GroupBy(r => (r.Level, r.Stage, r.Index)).ToDictionary(g => g.Key, g => g.ToList());
            List<(int level, int stage)> stages = rooms.Where(r => !r.IsSwitchRoom).Select(r => (r.Level, r.Stage)).Distinct().ToList();
            bool IsBossStage((int level, int stage) s) => s.level == 7 || s.stage == stages.Where(t => t.level == s.level).Max(t => t.stage);

            // The part holding a NiDL room's cell, and that cell in it;
            (NightmareRoom part, int x, int y) PartAt((int, int, int) key, int x, int y) {
                foreach (NightmareRoom part in byRoom[key]) {
                    if (x >= part.X && x < part.X + part.Width && y >= part.Y && y < part.Y + part.Height) return (part, x - part.X, y - part.Y);
                }
                NightmareRoom last = byRoom[key].Last();
                return (last, Math.Clamp(x - last.X, 0, last.Width - 1), Math.Clamp(y - last.Y, 0, last.Height - 1));
            }
            byte[] Cells(NightmareRoom r) => RoomCells(nightmare, r.Record);
            // Whether Kirby can go through a NiDL cell (nothing, water, ladders; breakable blocks too, once broken, if blocks: not metal
            // ones, 0x2F, which take an ability);
            bool OpenCell(byte[] cells, int width, int x, int y, bool blocks = false) {
                int kind = cells[(y * width + x) * 4 + 3];
                return RoomCollision[kind] is 0x00 or 0x48 or 0x20 or 0x21 || blocks && kind >> 4 == 2 && kind != 0x2F;
            }
            /* Where Kirby starts (+0x50, pixels), down to the floor. A start in a closed pocket, without exits (NiDL's rides: Butter
             * Building's revolving tower, 3-2, whose warp star flies Kirby up around it, a chamber of its walls here), moves to the
             * nearest cell that leads out of the room, at the same height; */
            (NightmareRoom part, int x, int y) Start((int, int, int) key) {
                NightmareRoom any = byRoom[key][0];
                return Land(key, ReadShort(nightmare, any.Record + 0x50) / 16, ReadShort(nightmare, any.Record + 0x52) / 16, true);
            }
            // A cell Kirby arrives in (pixels / 16), down to the floor if drop, out of a closed pocket (see Start());
            (NightmareRoom part, int x, int y) Land((int, int, int) key, int x, int y, bool drop) {
                NightmareRoom any = byRoom[key][0];
                int width = ReadShort(nightmare, any.Record + 0x14), height = ReadShort(nightmare, any.Record + 0x16);
                x = Math.Clamp(x, 0, width - 1); y = Math.Clamp(y, 0, height - 1);
                byte[] cells = Cells(any);
                HashSet<(int, int)> exitCells = new HashSet<(int, int)>();
                int exits = ReadInt(nightmare, any.Record + 0x44), objects = ReadInt(nightmare, any.Record + 0x48);
                for (int at = exits - 0x08000000; exits != 0 && at + 12 <= (objects != 0 ? objects - 0x08000000 : any.Record); at += 12)
                    if (nightmare[at + 1] is 0 or 34) exitCells.Add((ReadShort(nightmare, at + 2), ReadShort(nightmare, at + 4)));
                bool Pocket(int px, int py) => Shut(cells, width, height, px, py, exitCells);
                bool FloorAt(int cx, int cy) => RoomCollision[cells[(cy * width + cx) * 4 + 3]] is >= 0x01 and <= 0x1B || (cells[(cy * width + cx) * 4 + 3] >> 4) == 2;
                // (a cell Kirby stands in: open, a floor under it, above the room's last rows, which KatAM's camera hides and he falls out of);
                bool Standable(int cx, int cy) => OpenCell(cells, width, cx, cy) && cy + 1 < height - HiddenRows && FloorAt(cx, cy + 1);
                // The nearest cell he stands in (not shut in, when `open`), else the cell itself;
                (int x, int y) Nearest(int fx, int fy, bool open) {
                    (int x, int y) best = (fx, fy);
                    int distance = int.MaxValue;
                    for (int cy = 0; cy < height; cy++)
                        for (int cx = 0; cx < width; cx++) {
                            int d = Math.Abs(cx - fx) + Math.Abs(cy - fy);
                            if (d < distance && Standable(cx, cy) && (!open || !Pocket(cx, cy))) { best = (cx, cy); distance = d; }
                        }
                    return best;
                }
                if (Pocket(x, y)) {
                    (int x, int y) best = Nearest(x, y, true);
                    return PartAt(key, best.x, best.y);
                }
                while (drop && y + 1 < height && !FloorAt(x, y + 1)) y++;
                // (a drop down a pit, out of the room: Kirby would die there and come back to it, again and again (Butter Building's
                // tower top, 0x152, its warp star's landing); the nearest cell he stands in instead);
                if (drop && !Standable(x, y)) (x, y) = Nearest(x, y, false);
                return PartAt(key, x, y);
            }
            // Whether Kirby at a cell is shut in: the cells it reaches are few (64 at most: rooms have 176 or more) and hold no exit;
            bool Shut(byte[] cells, int width, int height, int x, int y, HashSet<(int, int)> exitCells) {
                HashSet<(int, int)> seen = new HashSet<(int, int)> { (x, y) };
                Queue<(int x, int y)> queue = new Queue<(int, int)>(seen);
                while (queue.Count > 0) {
                    (int cx, int cy) = queue.Dequeue();
                    if (seen.Count > 64 || exitCells.Contains((cx, cy))) return false;
                    foreach ((int nx, int ny) in new[] { (cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1) })
                        if (nx >= 0 && ny >= 0 && nx < width && ny < height && RoomCollision[cells[(ny * width + nx) * 4 + 3]] is not (0x0D or (>= 0x01 and <= 0x04)) && seen.Add((nx, ny)))
                            queue.Enqueue((nx, ny));
                }
                return true;
            }
            (int room, int x, int y) Target((NightmareRoom part, int x, int y) at) => (at.part.Room, at.x + 1, at.y + 1);
            (int, int, int) StageStart((int level, int stage) s) => Target(Start((s.level, s.stage, byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage).Min(k => k.Item3))));
            (int room, int x, int y) back = (EntryRoom, EntryX + 1, EntryY + 1);
            NightmareRoom SwitchRoomOf(int level) => rooms.First(r => r.IsSwitchRoom && r.Level == level);
            (int, int, int) MainHubAt((int x, int y) cell) => (mainHub, cell.x + 1, cell.y + 1);
            (int, int, int) After((int level, int stage) s) {
                if (IsBossStage(s)) return s.level == 7 ? MainHubAt(MainHubEntry) : StageStart(stages.Where(t => t.level == s.level + 1).Min());
                List<(int level, int stage)> level = stages.Where(t => t.level == s.level).ToList();
                return StageStart(level[level.IndexOf(s) + 1]);
            }
            Dictionary<int, int> widths = rooms.ToDictionary(r => r.Room, r => r.Width);
            byte Facing((int room, int x, int y) to) => (byte)(!widths.TryGetValue(to.room, out int w) || (to.x - 1) * 2 < w ? 1 : 2);
            Dictionary<NightmareRoom, byte[]> partCells = new Dictionary<NightmareRoom, byte[]>();
            Door Add((NightmareRoom part, int x, int y) at, (int room, int x, int y) to, DoorKind kind, bool edge = false, int slot = 0, int design = HubDesign, bool oneWay = false, bool hidden = false) {
                // (KatAM keeps Kirby 8 pixels and his half width inside a room: a mirror on its first or last column moves in a cell; one
                // the camera wouldn't show whole, Shown(), moves in through open cells: NiDL's doors on a room's top row);
                if (!edge) {
                    at.x = Math.Clamp(at.x, 1, at.part.FullWidth - 2);
                    if (!partCells.TryGetValue(at.part, out byte[] cells)) partCells[at.part] = cells = Cells(at.part);
                    int width = ReadShort(nightmare, at.part.Record + 0x14);
                    bool Open(int y) => y >= 0 && y < at.part.Height && OpenCell(cells, width, at.part.X + at.x, at.part.Y + y);
                    while (!Shown(at.part, at.y) && at.y < MirrorTop && Open(at.y + 1)) at.y++;
                    while (!Shown(at.part, at.y) && at.y > MirrorTop && Open(at.y - 1)) at.y--;
                }
                // (one door of a door 2 cells wide, NiDL's, or next to one to the same room: one mirror);
                if (doors[at.part].Any(d => d.X == at.x && d.Y == at.y || d.Room == to.room && !edge && !d.Edge && Math.Abs(d.X - at.x) <= 1 && Math.Abs(d.Y - at.y) <= 1)) return null;
                Door door = new Door(at.x, at.y, to.room, to.x, to.y, Facing(to), edge, slot, design, OneWay: oneWay, Hidden: hidden, Kind: kind);
                doors[at.part].Add(door);
                return door;
            }

            /* The layout's rules, as KatAM's (NiDL's rooms lagged with many mirrors in view, and read as a mess): a room has 3 doors at most,
             * 2 if it's small (2 screens or less: MaxExits()); the camera never shows 3 at once (ViewFits(): none within a screen, ViewWidth
             * x ViewHeight cells, of 2 others); no door is made in a mini-boss's fight area (Arenas(): the ones there show once it's beaten,
             * "ROM Expansion/enemies/arena.c"), nor a two-way one in its room (a door into one throws Kirby in, one-way); a goal room keeps
             * its goal door alone (goalRooms: no branch, no way back); */
            int MaxExits(NightmareRoom r) => r.Width * r.Height <= 2 * ViewWidth * (ViewHeight + 1) ? 2 : 3;
            int Exits(NightmareRoom r) => doors[r].Count(d => !d.Edge);
            bool ViewFits(NightmareRoom r, int x, int y) => InView(doors[r], x, y);
            bool InView(List<Door> others, int x, int y) {
                List<Door> near = others.Where(d => !d.Edge && !d.Plain && Math.Abs(d.X - x) < ViewWidth && Math.Abs(d.Y - y) < ViewHeight).ToList();
                for (int i = 0; i < near.Count; i++)
                    for (int j = i + 1; j < near.Count; j++) {
                        int[] xs = { x, near[i].X, near[j].X }, ys = { y, near[i].Y, near[j].Y };
                        if (xs.Max() - xs.Min() < ViewWidth && ys.Max() - ys.Min() < ViewHeight) return false;
                    }
                return true;
            }
            Dictionary<NightmareRoom, List<(int left, int top, int width, int height)>> arenas = rooms.ToDictionary(r => r, r => Arenas(nightmare, r));
            // (a cell around the area too: Kirby lands on a door, and its mirror is 2 cells tall);
            bool InArena(NightmareRoom r, int x, int y) => arenas[r].Any(a => x * 16 + 24 > a.left && x * 16 - 8 < a.left + a.width && y * 16 + 24 > a.top && y * 16 - 24 < a.top + a.height);
            bool MinibossRoom(NightmareRoom r) => arenas[r].Count > 0;
            HashSet<NightmareRoom> goalRooms = new HashSet<NightmareRoom>();
            // Hosts(); Whether a room takes a new door: under its exits, not a goal room, a boss stage's, a switch room, a split room's
            // part, nor (two-way) a mini-boss's;
            bool Hosts(NightmareRoom r, bool twoWay) => Exits(r) < MaxExits(r) && !goalRooms.Contains(r) && !r.IsSwitchRoom && !IsBossStage((r.Level, r.Stage))
                                                     && !(twoWay && MinibossRoom(r)) && byRoom[(r.Level, r.Stage, r.Index)].Count == 1;

            /* The levels' structure, as KatAM's areas are laid out (see the notes above PlanDoors()), from LayoutSeed:
             * • a level's regular stages are a tree from its first (its way in): every other stage is entered through a branch door in a
             *   room of another (deeper ones likelier, at most 2 branches a stage; in a room's middle or hidden away, half and half), its
             *   start getting a mirror back;
             * • only the deepest stage's goal leads to the boss stage; the others' goals end their branch, one-way as KatAM's goal mirrors,
             *   in turn: into a goal game (GoalRooms: back to Central Circle after), a stage of a level next to it, a goal game, a room of
             *   the KatAM area paired with the level (KatAMArea());
             * • loops, few, each for a reason (KatAM connects sparingly: about a loop per 7 rooms): one-way shortcuts home from the
             *   deepest stages (a third as many as the level has stages), a two-way door between sibling branches (two in big levels);
             *   a hidden two-way door between each level and the next, one one-way throw into another level; and KatAMLinks between
             *   each level and dead ends of its KatAM area (rooms with few doors, far from them), which get mirrors: one one-way from
             *   NiDL, one one-way into it, one two-way hidden behind a Mirra (MirraSpots()); */
            Random random = new Random(LayoutSeed);
            int RoomProps(int room, int field) => ReadShort(rom, symbols["gRoomProps"] + room * 0x28 + field * 2);
            int TableEntry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;
            HashSet<int> katamUsed = new HashSet<int>(LevelLinks.Where(l => l >= 0)) { EntryRoom, mainHub };
            katamUsed.UnionWith(rooms.Select(r => r.Room));

            // KatAMSpot(); A floor cell in a room of a KatAM area, the farthest from its doors (fewer doors better, if deadEnd), the camera
            // showing a mirror there, none of its objects owning the room (big switch, scroll lock, bosses, hub doors, shards, goal);
            (int room, int x, int y)? KatAMSpot(int area, bool deadEnd) {
                (int room, int x, int y)? best = null;
                double bestScore = double.MinValue;
                Dictionary<int, (double score, int x, int y)> clearOf = new Dictionary<int, (double, int, int)>();
                for (int room = 0; room < 0x3C0; room++) {
                    if (katamUsed.Contains(room) || RoomProps(room, 0) != area) continue;
                    int tilemapIndex = RoomProps(room, 12), solidityIndex = RoomProps(room, 13), objectsIndex = RoomProps(room, 16);
                    if (tilemapIndex == 0xFFFF || solidityIndex == 0xFFFF || objectsIndex == 0xFFFF) continue;
                    int templates = 0;
                    bool owned = false;
                    // (its enemies: no spot, where Kirby lands, within 3 cells across of one, 2 up or down, as the NiDL rooms' (BuildObjects()):
                    // room 0x142's Waddle Doo stood by the landing from 0xF4);
                    List<(int x, int y)> enemies = new List<(int, int)>();
                    for (int at = ReadInt(rom, TableEntry("gLevelObjLists", objectsIndex)) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) {
                        templates++;
                        if (rom[at + 12] is 0x86 or 0x77 or 0x8C or 0x97 or 0x65 or 0x85 or (>= 0x38 and <= 0x5F)) owned = true;
                        if (rom[at + 12] < 0x38) enemies.Add(((short)ReadShort(rom, at + 6) / 16, (short)ReadShort(rom, at + 8) / 16));
                    }
                    if (owned || templates > 28) continue;
                    int tilemap = TableEntry("gForegroundTilemaps", tilemapIndex), width = ReadShort(rom, tilemap) / 2, height = ReadShort(rom, tilemap + 2) / 2;
                    byte[] collision;
                    try { collision = DecompressRL(rom, ReadInt(rom, TableEntry("gSolidityMaps", solidityIndex)) - 0x08000000); }
                    catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException) { continue; }
                    if (width < 16 || collision.Length < width * height) continue;
                    byte At(int x, int y) => collision[y * width + x];
                    bool Passable(byte v) => v != 0x0D && v is not (>= 0xF0 and < 0xFC) && v is not (>= 0x1C and <= 0x1F);
                    List<(int x, int y)> doorCells = new List<(int, int)>();
                    for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) if (At(x, y) >= 0xFC) doorCells.Add((x, y));
                    int doorCount = doorCells.Count(c => At(c.x, c.y) is 0xFF or 0xFD);
                    if (doorCells.Count == 0) continue;
                    int[] distance = Enumerable.Repeat(-1, width * height).ToArray();
                    Queue<(int x, int y)> open = new Queue<(int, int)>();
                    foreach ((int x, int y) in doorCells) { distance[y * width + x] = 0; open.Enqueue((x, y)); }
                    while (open.Count > 0) {
                        (int x, int y) = open.Dequeue();
                        foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                            if (nx >= 0 && ny >= 0 && nx < width && ny < height && distance[ny * width + nx] < 0 && Passable(At(nx, ny))) {
                                distance[ny * width + nx] = distance[y * width + x] + 1;
                                open.Enqueue((nx, ny));
                            }
                    }
                    for (int y = MirrorTop; y <= height - 4; y++) {
                        for (int x = 1; x <= width - 2; x++) {
                            if (At(x, y) != 0 || At(x, y - 1) != 0 || At(x - 1, y) != 0 || At(x + 1, y) != 0 || Enumerable.Range(x - 1, 3).Any(fx => At(fx, y + 1) is < 0x05 or > 0x1B) || distance[y * width + x] < 0) continue;
                            if (doorCells.Any(c => Math.Abs(c.x - x) < MirrorSpacing * 2 && Math.Abs(c.y - y) <= 2)) continue;
                            double score = distance[y * width + x] - (deadEnd ? 12 * doorCount : 0) + random.NextDouble() * 3;
                            if (score > bestScore) { bestScore = score; best = (room, x, y); }
                            if (!enemies.Any(e => Math.Abs(e.x - x) <= 3 && Math.Abs(e.y - y) <= 2) && (!clearOf.TryGetValue(room, out var clear) || score > clear.score))
                                clearOf[room] = (score, x, y);
                        }
                    }
                }
                // (the best spot's room, as before, and its best spot clear of its enemies: another room changed the levels' doors);
                if (best is (int used, _, _)) {
                    katamUsed.Add(used);
                    if (clearOf.TryGetValue(used, out var clear)) best = (used, clear.x, clear.y);
                }
                return best;
            }
            static int KatAMArea(int level) => 5 + level;

            // RoomSpot(); A floor cell for a mirror in a NiDL room (a floor 3 cells wide under it, open cells beside it: Kirby landing on a
            // ledge 2 wide drops off it, next to a wall he's pushed off it), where the camera shows it, with room above, not water, no door, warp star or cannon within
            // 2 x MirrorSpacing cells, one Kirby reaches; in the room's middle, or hidden (the farthest from where he comes in, behind blocks);
            // (a landing, door false: the same, for a one-way door's: away from doors, no door made, a mini-boss's fight area as good);
            // (within: only those cells);
            (NightmareRoom part, int x, int y)? RoomSpot(NightmareRoom r, bool hidden, int spacing = MirrorSpacing * 2, bool door = true, ISet<(int, int)> within = null) {
                var key = (r.Level, r.Stage, r.Index);
                int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                if (!partCells.TryGetValue(r, out byte[] cells)) partCells[r] = cells = Cells(r);
                int Kind(int x, int y) => cells[(y * width + x) * 4 + 3];
                int[] Distances(bool withBlocks) {
                    int[] distance = Enumerable.Repeat(-1, width * height).ToArray();
                    Queue<(int x, int y)> open = new Queue<(int, int)>();
                    // (from where Kirby comes in: where he starts it, and where its doors and the other rooms' land him: not its exits
                    // alone, as its edges, which a tower's falls only leave by);
                    var arrivals = doors.Values.SelectMany(list => list).Where(d => d.Room == r.Room && !d.Edge).Select(d => (x: d.DestX - 1, y: d.DestY - 1))
                                        .Concat(doors[r].Where(d => !d.Edge && !d.OneWay).Select(d => (x: d.X, y: d.Y))).Append((x: Start(key).x, y: Start(key).y));
                    foreach ((int x, int y) in arrivals)
                        if (x >= 0 && y >= 0 && x < width && y < height && distance[y * width + x] < 0) { distance[y * width + x] = 0; open.Enqueue((x, y)); }
                    while (open.Count > 0) {
                        (int x, int y) = open.Dequeue();
                        foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                            if (nx >= 0 && ny >= 0 && nx < width && ny < height && distance[ny * width + nx] < 0 && OpenCell(cells, width, nx, ny, withBlocks)) {
                                distance[ny * width + nx] = distance[y * width + x] + 1;
                                open.Enqueue((nx, ny));
                            }
                    }
                    return distance;
                }
                int[] plain = Distances(false), broken = Distances(true);
                // (nor where other rooms' doors land Kirby: their mirrors back go there);
                var landings = doors.Values.SelectMany(list => list).Where(d => d.Room == r.Room && !d.Edge).Select(d => (x: d.DestX - 1, y: d.DestY - 1)).ToList();
                (NightmareRoom part, int x, int y)? best = null;
                double bestScore = double.MinValue;
                for (int y = MirrorTop; y + 1 < height; y++) {
                    for (int x = 1; x < width - 1 && x < r.FullWidth - 1; x++) {
                        int distance = broken[y * width + x];
                        if (within != null && !within.Contains((x, y))) continue;
                        if (distance < 0 || !Shown(r, y) || Kind(x, y) != 0 || !OpenCell(cells, width, x, y - 1) || !OpenCell(cells, width, x - 1, y) || !OpenCell(cells, width, x + 1, y)) continue;
                        if (Enumerable.Range(x - 1, 3).Any(fx => RoomCollision[Kind(fx, y + 1)] is not (0x0D or (>= 0x05 and <= 0x0E) or (>= 0x10 and <= 0x1B)))) continue;
                        if (doors[r].Any(d => Math.Abs(d.X - x) < spacing && Math.Abs(d.Y - y) <= 2) || landings.Any(c => Math.Abs(c.x - x) < spacing && Math.Abs(c.y - y) <= 2)) continue;
                        // (nor by a warp star or cannon: the ways on they are stay clear);
                        if (transportsOf[r].Any(t => Math.Abs(t.X / 16 - x) < spacing && Math.Abs((t.Y - 1) / 16 - y) <= 2)) continue;
                        if (door && (InArena(r, x, y) || !ViewFits(r, x, y))) continue;
                        double score = hidden ? distance + (plain[y * width + x] < 0 ? 60 : 0) : -Math.Abs(x - width / 2.0) + random.NextDouble() * 6;
                        if (score > bestScore) { bestScore = score; best = (r, x, y); }
                    }
                }
                return best;
            }
            // HostSpot(); A spot for a new door in a room of a stage that takes one (Hosts(): not its first, if it has others), the rooms in
            // a random order, bigger ones likelier (they have room for more ways on);
            (NightmareRoom part, int x, int y)? HostSpot((int level, int stage) s, bool hidden, bool twoWay, int spacing = MirrorSpacing * 2) {
                var keys = byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage && byRoom[k].Count == 1).OrderBy(k => k.Item3).ToList();
                var order = keys.Skip(keys.Count > 1 ? 1 : 0).OrderByDescending(k => random.NextDouble() * (1 + byRoom[k][0].Width * byRoom[k][0].Height / 330.0))
                                .Concat(keys.Take(keys.Count > 1 ? 1 : 0)).ToList();
                foreach (var key in order) {
                    NightmareRoom r = byRoom[key][0];
                    if (Hosts(r, twoWay) && RoomSpot(r, hidden, spacing) is (NightmareRoom, int, int) spot) return spot;
                }
                return null;
            }
            // EntrySpot(); Where a door into a stage stands, two-way (a branch's way back, the switch room's): in its first room that takes
            // one (in its middle), else none;
            (NightmareRoom part, int x, int y)? EntrySpot((int level, int stage) s, int spacing = MirrorSpacing * 2) {
                foreach (var key in byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage && byRoom[k].Count == 1).OrderBy(k => k.Item3)) {
                    NightmareRoom r = byRoom[key][0];
                    if (Hosts(r, true) && RoomSpot(r, false, spacing) is (NightmareRoom, int, int) spot) return spot;
                }
                return null;
            }
            // LandingIn(); Where a one-way door throws Kirby into a stage: a floor in one of its rooms (a random one, or its first), away
            // from its doors (none lands him in front of one, as KatAM's); its start, if none;
            (int room, int x, int y) LandingIn((int level, int stage) s, bool first = false) {
                var keys = byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage && byRoom[k].Count == 1 && !goalRooms.Contains(byRoom[k][0])).OrderBy(k => k.Item3).ToList();
                foreach (var key in first ? keys : keys.OrderBy(_ => random.Next()).ToList())
                    if (!IsBossStage(s) && RoomSpot(byRoom[key][0], false, door: false) is (NightmareRoom, int, int) spot) return Target(spot);
                return StageStart(s);
            }

            List<((int level, int stage) from, (int level, int stage) to)> branches = new List<((int, int), (int, int))>();
            Dictionary<(int level, int stage), (int level, int stage)> parentOf = new Dictionary<(int, int), (int, int)>();
            List<((int level, int stage) a, (int level, int stage) b, bool hidden, bool oneWay)> links = new List<((int, int), (int, int), bool, bool)>();
            Dictionary<(int level, int stage), (int room, int x, int y)> goals = new Dictionary<(int, int), (int, int, int)>();
            List<List<(int level, int stage)>> regulars = new List<List<(int, int)>>();
            Dictionary<int, (int level, int stage)> gates = new Dictionary<int, (int, int)>();
            for (int level = 0; level < 7; level++) {
                List<(int level, int stage)> regular = stages.Where(t => t.level == level && !IsBossStage(t)).OrderBy(t => t.stage).ToList();
                regulars.Add(regular);
                if (regular.Count == 0) continue;
                (int level, int stage) boss = stages.First(t => t.level == level && IsBossStage(t));
                Dictionary<(int, int), int> depth = new Dictionary<(int, int), int> { [regular[0]] = 0 }, children = new Dictionary<(int, int), int> { [regular[0]] = 0 };
                Dictionary<(int, int), (int, int)> parent = new Dictionary<(int, int), (int, int)>();
                // (a big stage, of many rooms, branches more: up to 3 ways on);
                int Branches((int level, int stage) p) => Math.Clamp(1 + byRoom.Keys.Count(k => k.Item1 == p.level && k.Item2 == p.stage) / 4, 1, 3);
                foreach (var s in regular.Skip(1).OrderBy(_ => random.Next()).ToList()) {
                    var from = depth.Keys.Where(p => children[p] < Branches(p)).DefaultIfEmpty(depth.Keys.First()).OrderByDescending(p => random.NextDouble() * (1 + depth[p])).First();
                    branches.Add((from, s));
                    parent[s] = parentOf[s] = from;
                    children[from]++;
                    depth[s] = depth[from] + 1;
                    children[s] = 0;
                }
                var gate = depth.Keys.OrderByDescending(p => depth[p]).ThenByDescending(p => p.Item2).First();
                gates[level] = gate;
                int ends = 0;
                foreach (var s in regular) {
                    if (s == gate) { goals[s] = StageStart(boss); continue; }
                    (int room, int x, int y)? to = null;
                    switch (ends++ % 4) {
                        case 0: case 2:
                            to = (GoalRooms[(level + ends) % GoalRooms.Length], GoalLanding.x, GoalLanding.y);
                            break;
                        case 1:
                            int other = level == 0 ? 1 : level == 6 ? 5 : level + (random.Next(2) == 0 ? -1 : 1);
                            List<(int level, int stage)> them = stages.Where(t => t.level == other && !IsBossStage(t)).ToList();
                            to = StageStart(them[random.Next(them.Count)]);
                            break;
                        case 3:
                            if (KatAMSpot(KatAMArea(level), false) is (int room, int x, int y)) to = (room, x + 1, y + 1);
                            break;
                    }
                    goals[s] = to ?? StageStart(regular[0]);
                }
                /* Loops, few and each for a reason, as KatAM's (about one per 7 rooms of an area): shortcuts, one-way, from the deepest
                 * stages back to one at least two branches shallower (else any shallower but its own parent: a way home earned by the
                 * long way out, as KatAM's mirrors back from a branch's end), at least 2; */
                List<(int level, int stage)> deepest = regular.OrderByDescending(t => depth[t]).ThenBy(t => t.stage).ToList();
                for (int i = 0; i < Math.Max(2, regular.Count / 3) && i < deepest.Count; i++) {
                    var a = deepest[i];
                    var home = regular.Where(t => depth[t] <= depth[a] - 2).ToList();
                    if (home.Count == 0) home = regular.Where(t => depth[t] < depth[a] && parent.GetValueOrDefault(a) != t).ToList();
                    if (home.Count == 0) continue;
                    links.Add((a, home[random.Next(home.Count)], random.Next(2) == 0, true));
                }
            }
            for (int level = 0; level < 7; level++) {
                if (regulars[level].Count == 0) continue;
                (int, int) Any(int l) => regulars[l][random.Next(regulars[l].Count)];
                // (between levels: a hidden door to the next, and a throw into another, one-way as KatAM's few mirrors between areas);
                if (level + 1 < 7 && regulars[level + 1].Count > 0) links.Add((Any(level), Any(level + 1), true, true));
                int other = (level + 2 + random.Next(5)) % 7;
                if (regulars[other].Count > 0) links.Add((Any(level), Any(other), random.Next(2) == 0, true));
            }

            List<((NightmareRoom, int, int) at, (int, int, int) to)> edges = new List<((NightmareRoom, int, int), (int, int, int))>();
            int rideCount = 0;
            foreach (var key in byRoom.Keys) {
                NightmareRoom r = byRoom[key][0];
                if (r.IsSwitchRoom) continue;
                (int level, int stage) s = (key.Item1, key.Item2);
                int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                int exits = ReadInt(nightmare, r.Record + 0x44), objects = ReadInt(nightmare, r.Record + 0x48);
                int exitsEnd = objects != 0 ? objects - 0x08000000 : r.Record;
                List<(int x, int y, bool cannon)> transports = new List<(int, int, bool)>();
                for (int at = objects == 0 ? r.Record : objects - 0x08000000; objects != 0 && at + 8 <= r.Record; at += 8) {
                    // Warp stars (gimmick 4) and cannons (6);
                    if (nightmare[at] == 5 && (nightmare[at + 1] == 4 || nightmare[at + 1] == 6)) {
                        var (sx, sy) = TransportSpot(nightmare, r, ReadShort(nightmare, at + 4), ReadShort(nightmare, at + 6), nightmare[at + 1] == 6);
                        transports.Add((sx, sy, nightmare[at + 1] == 6));
                    }
                }

                // NiDL's exits; a door with two exits (one only after a big switch) keeps its plain one, listed later;
                List<(int x, int y, (int, int, int) to, DoorKind kind)> found = new List<(int, int, (int, int, int), DoorKind)>();
                for (int at = exits - 0x08000000; exits != 0 && at + 12 <= exitsEnd; at += 12) {
                    int destination = nightmare[at], kind = nightmare[at + 1], x = ReadShort(nightmare, at + 2), y = ReadShort(nightmare, at + 4);
                    int landX = ReadShort(nightmare, at + 6) / 16, landY = ReadShort(nightmare, at + 8) / 16;
                    if (kind == 0 && x < width && y < height && byRoom.ContainsKey((s.level, s.stage, destination)))
                        found.Insert(0, (x, y, Target(PartAt((s.level, s.stage, destination), landX, landY)), DoorKind.Nidl));
                    else if (kind == 34 && x < width && y < height) {
                        found.Add((x, y, goals.TryGetValue(s, out var goal) ? goal : After(s), DoorKind.Goal));
                        goalRooms.Add(PartAt(key, x, y).part);
                    } else if (kind == 30 && transports.Count > 0 && byRoom.ContainsKey((s.level, s.stage, x))) {
                        // (a warp star or cannon: KatAM's own object, where it stands, to where its exit lands Kirby: a cannon's, the top
                        // of the room it shoots him up into);
                        var (tx, ty, cannon) = transports[0];
                        transports.RemoveAt(0);
                        var (part, _, _) = PartAt(key, tx / 16, ty / 16);
                        // (a warp star's lands Kirby in the air past a fall: he lands where the room starts, on its floor; one with a ride of
                        // NiDL's, NidlRides, flies it: straight into the room it lands in, he lands where he hops off it there);
                        var to = cannon ? Target(PartAt((s.level, s.stage, x), landX, landY)) : Target(Start((s.level, s.stage, x)));
                        RideLeg[] ride = cannon ? null : Ride(key, PartAt);
                        if (ride != null && ride.Length == 2 && ride[1].Room == to.room)
                            to = (to.room, (ride[1].End.x + ride[1].Hop.Concat(ride[1].Bounce).Sum(m => m.dx * m.frames)) / 16 + 1,
                                  (ride[1].End.y + ride[1].Hop.Concat(ride[1].Bounce).Sum(m => m.dy * m.frames)) / 16 + 1);
                        transportsOf[part].Add(new Transport(tx - part.X * 16, ty - part.Y * 16, cannon, to.room, to.x, to.y,
                                                             ride == null ? WarpFlight : WarpFlight + 1 + rideCount++, ride));
                    }
                    else if (kind == 26 && byRoom.ContainsKey((s.level, s.stage, x))) {
                        byte[] cells = Cells(r);
                        // (landing on the room's first column, NiDL's warp star skies carrying Kirby on, Rainbow Resort's 0x2F9 into 0x2EC:
                        // where that room starts, not its corner);
                        var to = landX == 0 ? Target(Start((s.level, s.stage, x))) : Target(Land((s.level, s.stage, x), landX, landY, false));
                        for (int cx = 0; cx < width; cx++) {
                            if (OpenCell(cells, width, cx, 0)) edges.Add((PartAt(key, cx, 0), to));
                            if (OpenCell(cells, width, cx, height - 1)) edges.Add((PartAt(key, cx, height - 1), to));
                        }
                    }
                }
                // (a goal door is one-way, as KatAM's goal mirrors);
                foreach ((int x, int y, (int, int, int) to, DoorKind kind) in found) Add(PartAt(key, x, y), to, kind, oneWay: kind != DoorKind.Nidl);

                if (IsBossStage(s)) {
                    int next = byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage && k.Item3 > key.Item3).Select(k => k.Item3).DefaultIfEmpty(-1).Min();
                    (int room, int x, int y) on = next >= 0 ? Target(Start((s.level, s.stage, next))) : After(s);
                    /* (a boss's room: no door out, its shard takes Kirby back to the NiDL hub, onto his level's door there, as KatAM's take
                     * him to Central Circle (bossExitsOf); but for a fight in two rooms, the Power Orb's, which goes on to Nightmare's); */
                    if (HasBoss(byRoom[key][0])) {
                        bool goesOn = next >= 0 && HasBoss(byRoom[(s.level, s.stage, next)][0]);
                        foreach (NightmareRoom part in byRoom[key]) bossExitsOf[part] = goesOn ? on : MainHubAt(MainHubDoors[s.level]);
                    } else Add(Start(key), on, DoorKind.Boss, oneWay: true);
                }
            }

            foreach (var (at, to) in edges) Add(at, to, DoorKind.Edge, true);

            // A split room's parts: the cells on their shared edge (not solid) take Kirby across;
            foreach (var parts in byRoom.Values.Where(p => p.Count > 1)) {
                byte[] cells = Cells(parts[0]);
                int width = ReadShort(nightmare, parts[0].Record + 0x14);
                // (through breakable blocks too: Heavy Mole's arena is all blocks);
                bool Open(int x, int y) => OpenCell(cells, width, x, y, true);
                for (int i = 0; i + 1 < parts.Count; i++) {
                    NightmareRoom a = parts[i], b = parts[i + 1];
                    if (b.X > a.X) {
                        for (int y = 0; y < a.Height; y++) {
                            if (Open(b.X - 1, y)) Add((a, a.Width - 1, y), (b.Room, 2, y + 1), DoorKind.Split, true);
                            if (Open(b.X, y)) Add((b, 0, y), (a.Room, a.Width - 1, y + 1), DoorKind.Split, true);
                        }
                    } else {
                        for (int x = 0; x < a.Width; x++) {
                            if (Open(x, b.Y - 1)) Add((a, x, a.Height - 1), (b.Room, x + 1, 2), DoorKind.Split, true);
                            if (Open(x, b.Y)) Add((b, x, 0), (a.Room, x + 1, a.Height - 1), DoorKind.Split, true);
                        }
                    }
                }
            }

            /* Two-way, as KatAM's: NiDL's own doors are pairs, and one whose landing has no mirror back nearby (NiDL's few one-way doors)
             * gets one there, or beside it on the same floor (the first free cell up to 10 away that Kirby walks to, none within 2 cells,
             * where the camera shows it: then the mirror lands there; with no floor, the landing itself), where the room takes it (its
             * exits, the camera: see MaxExits(), not a goal room); not into or out of the boss stages (one-way); a mirror with one back
             * lands Kirby on it, as KatAM's (NiDL's doors land him beside theirs); */
            Dictionary<int, NightmareRoom> partOf = rooms.ToDictionary(r => r.Room, r => r);
            bool IsBossRoom(NightmareRoom r) => !r.IsSwitchRoom && IsBossStage((r.Level, r.Stage));
            Dictionary<NightmareRoom, byte[]> cellsOf = new Dictionary<NightmareRoom, byte[]>();
            (int x, int y)? FreeFloor(NightmareRoom r, int x, int y) {
                if (!cellsOf.TryGetValue(r, out byte[] cells)) cellsOf[r] = cells = Cells(r);
                int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                int Kind(int cx, int cy) => cells[((r.Y + cy) * width + r.X + cx) * 4 + 3];
                bool Open(int cx, int cy) => cx >= 0 && cx < r.Width && cy >= 0 && cy < r.Height && OpenCell(cells, width, r.X + cx, r.Y + cy);
                bool Floor(int cx, int cy) => r.Y + cy < height && RoomCollision[Kind(cx, cy)] is >= 0x01 and <= 0x1B;
                if (!Open(x, y)) return null;
                int landingY = y;
                while (Open(x, y + 1) && !Floor(x, y + 1)) y++;
                foreach (int dx in Enumerable.Range(0, 2 * ReturnReach + 1).Select(i => (i + 1) / 2 * (i % 2 == 0 ? -1 : 1))) {
                    int cx = x + dx;
                    if (cx >= 1 && cx <= r.FullWidth - 2 && Shown(r, y) && Enumerable.Range(Math.Min(x, cx), Math.Abs(dx) + 1).All(ix => Open(ix, y)) && Open(cx - 1, y) && Open(cx + 1, y) && y + 1 < r.Height && Floor(cx, y + 1) && (cx == 0 || Floor(cx - 1, y + 1)) && (cx + 1 >= r.Width || Floor(cx + 1, y + 1)) && !doors[r].Any(d => d.Edge ? d.Y == y && d.X == cx : Math.Abs(d.Y - y) <= 2 && Math.Abs(d.X - cx) < MirrorSpacing))
                        return (cx, y);
                }
                // (no floor nearby: the landing itself, in the air, as KatAM's hub doors are, where the camera shows it; none where Kirby
                // falls out of the room, as a cannon's shot into a sky he falls through: he'd never stay on it);
                if (y + 1 >= r.Height) return null;
                x = Math.Clamp(x, 1, r.FullWidth - 2);
                while (!Shown(r, landingY) && landingY < MirrorTop && Open(x, landingY + 1)) landingY++;
                while (!Shown(r, landingY) && landingY > MirrorTop && Open(x, landingY - 1)) landingY--;
                return doors[r].Any(d => !d.Edge && Math.Abs(d.Y - landingY) <= 2 && Math.Abs(d.X - x) < MirrorSpacing) ? null : (x, landingY);
            }
            // FallsOut(); Whether Kirby, there, falls out of the room (nothing but air and water below it): landing on a mirror floating
            // over a fall, he'd drop out at once (NiDL's doors in its skies: Kirby flies into them, and lands beside them);
            bool FallsOut(NightmareRoom r, int x, int y) {
                if (!cellsOf.TryGetValue(r, out byte[] cells)) cellsOf[r] = cells = Cells(r);
                int width = ReadShort(nightmare, r.Record + 0x14);
                while (y + 1 < r.Height && OpenCell(cells, width, r.X + x, r.Y + y + 1)) y++;
                return y + 1 >= r.Height;
            }
            foreach (NightmareRoom from in rooms) {
                foreach (Door door in doors[from].ToList()) {
                    if (door.Kind != DoorKind.Nidl || door.OneWay || !partOf.TryGetValue(door.Room, out NightmareRoom to)) continue;
                    int x = door.DestX - 1, y = door.DestY - 1;
                    // (NiDL lands Kirby beside the door back; KatAM, on it);
                    // (beside it: up to 2 cells across, 3 up or down, where Kirby lands above a door and drops by it);
                    Door partner = doors[to].Where(d => !d.Edge && d.Room == from.Room && Math.Abs(d.X - x) <= 2 && Math.Abs(d.Y - y) <= 3)
                                            .OrderBy(d => Math.Abs(d.X - x) + Math.Abs(d.Y - y)).FirstOrDefault();
                    if (partner != null) {
                        if ((partner.X, partner.Y) != (x, y) && !FallsOut(to, partner.X, partner.Y))
                            doors[from][doors[from].IndexOf(door)] = door with { DestX = partner.X + 1, DestY = partner.Y + 1, Facing = Facing((to.Room, partner.X + 1, partner.Y + 1)) };
                        continue;
                    }
                    // (boss stages are one-way, as KatAM's boss rooms: no way back into one, nor out of one back to it; a room with a door
                    // back already, elsewhere, gets no second one);
                    if (IsBossRoom(from) || IsBossRoom(to)) continue;
                    if (doors[to].Any(d => !d.Edge && d.Room == from.Room) || goalRooms.Contains(to) || Exits(to) >= MaxExits(to)) continue;
                    if (FreeFloor(to, x, y) is not (int, int) cell || !ViewFits(to, cell.x, cell.y)) continue;
                    if (cell != (x, y)) doors[from][doors[from].IndexOf(door)] = door with { DestX = cell.x + 1, DestY = cell.y + 1, Facing = Facing((to.Room, cell.x + 1, cell.y + 1)) };
                    doors[to].Add(new Door(cell.x, cell.y, from.Room, door.X + 1, door.Y + 1, Facing((from.Room, door.X + 1, door.Y + 1)), Kind: DoorKind.Nidl));
                }
            }
            /* A goal room keeps its goal door alone, as KatAM's (no branch, no way back): its doors back towards its stage's start (fewer
             * doors from it: the way Kirby came) go, and the doors into it there throw him in, one-way; NiDL's goal rooms that lead on
             * (into side rooms, up Butter Building's tower, Orange Ocean's room of doors) keep those doors: the rooms past them are
             * reached only through them; */
            foreach (NightmareRoom goalRoom in goalRooms) {
                Dictionary<int, int> fromStart = new Dictionary<int, int>();
                Queue<NightmareRoom> queue = new Queue<NightmareRoom>();
                foreach (NightmareRoom first in byRoom[byRoom.Keys.Where(k => k.Item1 == goalRoom.Level && k.Item2 == goalRoom.Stage).Min()]) { fromStart[first.Room] = 0; queue.Enqueue(first); }
                while (queue.Count > 0) {
                    NightmareRoom at = queue.Dequeue();
                    foreach (Door d in doors[at])
                        if (partOf.TryGetValue(d.Room, out NightmareRoom next) && (next.Level, next.Stage) == (goalRoom.Level, goalRoom.Stage) && fromStart.TryAdd(next.Room, fromStart[at.Room] + 1))
                            queue.Enqueue(next);
                }
                foreach (Door way in doors[goalRoom].Where(d => !d.Edge && d.Kind != DoorKind.Goal).ToList()) {
                    if (!fromStart.TryGetValue(way.Room, out int distance) || distance >= fromStart.GetValueOrDefault(goalRoom.Room, int.MaxValue)) continue;
                    var into = rooms.SelectMany(r => doors[r].Select(d => (r, d)))
                                    .Where(p => p.r.Room == way.Room && p.d.Room == goalRoom.Room && !p.d.Edge && p.d.DestX == way.X + 1 && p.d.DestY == way.Y + 1).ToList();
                    if (into.Count == 0) continue;
                    doors[goalRoom].Remove(way);
                    foreach (var (r, d) in into) doors[r][doors[r].IndexOf(d)] = d with { OneWay = true };
                }
            }

            /* Stages unlinked from themselves: pairs of NiDL's own two-way doors (a door and the one back, in rooms of a regular stage) of
             * two stages swap their other ends (a-b, c-d: a-d, c-b), so a stage's rooms lead on into another's, still two-way: per level,
             * as many swaps as it has stages, one of them with another level; */
            List<(NightmareRoom a, Door da, NightmareRoom b, Door db)> Pairs(int level) {
                var found = new List<(NightmareRoom a, Door da, NightmareRoom b, Door db)>();
                foreach (NightmareRoom a in rooms.Where(r => r.Level == level && !r.IsSwitchRoom && !IsBossStage((r.Level, r.Stage)) && byRoom[(r.Level, r.Stage, r.Index)].Count == 1)) {
                    foreach (Door da in doors[a].Where(d => !d.Edge && !d.OneWay && d.Slot == 0)) {
                        NightmareRoom b = rooms.FirstOrDefault(r => r.Room == da.Room && r.Stage == a.Stage && r.Level == a.Level && byRoom[(r.Level, r.Stage, r.Index)].Count == 1);
                        if (b == null || b == a || b.Room < a.Room) continue;
                        Door db = doors[b].FirstOrDefault(d => !d.Edge && d.Room == a.Room && Math.Abs(d.X - (da.DestX - 1)) <= 2 && Math.Abs(d.Y - (da.DestY - 1)) <= 2);
                        if (db == null || Math.Abs(da.X - (db.DestX - 1)) > 2 || Math.Abs(da.Y - (db.DestY - 1)) > 2) continue;
                        found.Add((a, da, b, db));
                    }
                }
                return found.Where(p => Walked(p.a, p.da) && Walked(p.b, p.db)).ToList();
            }
            // Walked(); Whether Kirby walks to a door from the rest of its room (where he starts it, the other doors' landings): not one
            // in a pocket he's only in after coming in through it, which leads back only where it came from;
            bool Walked(NightmareRoom r, Door door) {
                int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                if (!partCells.TryGetValue(r, out byte[] cells)) partCells[r] = cells = Cells(r);
                bool Mine(int x, int y) => Math.Abs(x - door.X) <= 1 && Math.Abs(y - door.Y) <= 1;
                var seen = new bool[width * height];
                var open = new Queue<(int x, int y)>();
                var key = (r.Level, r.Stage, r.Index);
                var arrivals = doors.Values.SelectMany(list => list).Where(d => d.Room == r.Room && !d.Edge).Select(d => (x: d.DestX - 1, y: d.DestY - 1))
                                    .Append((x: Start(key).x, y: Start(key).y)).Where(c => !Mine(c.x, c.y));
                foreach ((int x, int y) in arrivals)
                    if (x >= 0 && y >= 0 && x < width && y < height && !seen[y * width + x]) { seen[y * width + x] = true; open.Enqueue((x, y)); }
                while (open.Count > 0) {
                    (int x, int y) = open.Dequeue();
                    if (Mine(x, y)) return true;
                    foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                        if (nx >= 0 && ny >= 0 && nx < width && ny < height && !seen[ny * width + nx] && OpenCell(cells, width, nx, ny, true)) {
                            seen[ny * width + nx] = true;
                            open.Enqueue((nx, ny));
                        }
                }
                return false;
            }
            void Swap((NightmareRoom a, Door da, NightmareRoom b, Door db) one, (NightmareRoom a, Door da, NightmareRoom b, Door db) two) {
                void Point(NightmareRoom from, Door door, NightmareRoom to, Door back) =>
                    doors[from][doors[from].IndexOf(door)] = door with { Room = to.Room, DestX = back.X + 1, DestY = back.Y + 1, Facing = Facing((to.Room, back.X + 1, back.Y + 1)) };
                Point(one.a, one.da, two.b, two.db);
                Point(two.b, two.db, one.a, one.da);
                Point(two.a, two.da, one.b, one.db);
                Point(one.b, one.db, two.a, two.da);
            }
            var swapped = new HashSet<(int room, int x, int y)>();
            bool Free((NightmareRoom a, Door da, NightmareRoom b, Door db) p) => !swapped.Contains((p.a.Room, p.da.X, p.da.Y)) && !swapped.Contains((p.b.Room, p.db.X, p.db.Y));
            for (int level = 0; level < 7; level++) {
                int count = stages.Count(t => t.level == level && !IsBossStage(t));
                for (int i = 0; i < count; i++) {
                    var mine = Pairs(level).Where(Free).ToList();
                    var theirs = (i == 0 && level < 6 ? Pairs(level + 1).Where(Free).ToList() : mine);
                    if (mine.Count == 0 || theirs.Count == 0) continue;
                    var one = mine[random.Next(mine.Count)];
                    var choices = theirs.Where(p => (p.a.Level, p.a.Stage) != (one.a.Level, one.a.Stage)).ToList();
                    if (choices.Count == 0) continue;
                    var two = choices[random.Next(choices.Count)];
                    Swap(one, two);
                    swapped.UnionWith(new[] { (one.a.Room, one.da.X, one.da.Y), (one.b.Room, one.db.X, one.db.Y), (two.a.Room, two.da.X, two.da.Y), (two.b.Room, two.db.X, two.db.Y) });
                }
            }

            /* Pockets made branches: a room whose doors all lead back into one room (NiDL's side rooms with two doors to where they hang
             * off: Orange Ocean 2-6, 0x279) keeps one, and its last throws Kirby one-way into another stage of the level, so the pocket is a
             * way on; */
            foreach (NightmareRoom pocket in rooms.Where(r => !r.IsSwitchRoom && !IsBossStage((r.Level, r.Stage)) && !goalRooms.Contains(r) && byRoom[(r.Level, r.Stage, r.Index)].Count == 1
                                                         && transportsOf[r].Count == 0).ToList()) {
                List<Door> ways = doors[pocket].Where(d => !d.Edge).ToList();
                if (ways.Count < 2 || ways.Select(d => d.Room).Distinct().Count() > 1 || doors[pocket].Any(d => d.Edge)) continue;
                var others = regulars[pocket.Level].Where(t => t.stage != pocket.Stage).ToList();
                if (others.Count == 0) continue;
                Door last = ways.Last();
                (int room, int x, int y) to = LandingIn(others[random.Next(others.Count)]);
                doors[pocket][doors[pocket].IndexOf(last)] = last with { Room = to.room, DestX = to.x, DestY = to.y, Facing = Facing(to), OneWay = true, Kind = DoorKind.Link };
            }

            /* SecretSpot(); Where a level's way to its switch room is, as KatAM's big switches are: hidden, and earned. In the second half
             * of its stages (not the boss's), in the room farthest from its stage's start (by doors), on the floor cell farthest from
             * where Kirby comes into it (by open cells, through breakable blocks), best behind blocks he must break to get there; a cell
             * the camera shows a mirror on (Shown()), with room above, no door within 2 cells, not water; not in a room taken; */
            // (in a room that takes a door, Hosts(), the camera showing 2 at most there; in any stage if none in the second half);
            (NightmareRoom part, int x, int y) SecretSpot(int level, HashSet<NightmareRoom> taken, bool anyStage = false) {
                List<(int level, int stage)> regular = stages.Where(t => t.level == level && !IsBossStage(t)).ToList();
                (NightmareRoom part, int x, int y) best = default;
                int bestScore = int.MinValue;
                foreach ((int level, int stage) s in regular.Skip(anyStage ? 0 : regular.Count / 2)) {
                    // The stage's rooms by doors from its first;
                    var keys = byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage).ToList();
                    Dictionary<int, int> depth = new Dictionary<int, int> { [keys.Min(k => k.Item3)] = 0 };
                    Queue<int> queue = new Queue<int>(depth.Keys);
                    while (queue.Count > 0) {
                        int index = queue.Dequeue();
                        foreach (NightmareRoom part in byRoom[(s.level, s.stage, index)])
                            foreach (Door d in doors[part])
                                foreach (var k in keys.Where(k => byRoom[k].Any(p => p.Room == d.Room) && !depth.ContainsKey(k.Item3))) {
                                    depth[k.Item3] = depth[index] + 1;
                                    queue.Enqueue(k.Item3);
                                }
                    }
                    foreach (var key in keys) {
                        NightmareRoom r = byRoom[key][0];
                        if (taken.Contains(r) || !Hosts(r, false)) continue;
                        int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                        byte[] cells = Cells(r);
                        int Kind(int x, int y) => cells[(y * width + x) * 4 + 3];
                        bool In(int x, int y) => x >= 0 && y >= 0 && x < width && y < height;
                        // Distances from where Kirby comes in (its doors, where it starts), through open cells (and blocks: withBlocks);
                        int[] Distances(bool withBlocks) {
                            int[] distance = Enumerable.Repeat(-1, width * height).ToArray();
                            Queue<(int x, int y)> open = new Queue<(int, int)>();
                            var starts = doors[r].Select(d => (d.X, d.Y)).Append((Start(key).x, Start(key).y));
                            foreach ((int x, int y) in starts) if (In(x, y) && distance[y * width + x] < 0) { distance[y * width + x] = 0; open.Enqueue((x, y)); }
                            while (open.Count > 0) {
                                (int x, int y) = open.Dequeue();
                                foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                                    if (In(nx, ny) && distance[ny * width + nx] < 0 && OpenCell(cells, width, nx, ny, withBlocks)) {
                                        distance[ny * width + nx] = distance[y * width + x] + 1;
                                        open.Enqueue((nx, ny));
                                    }
                            }
                            return distance;
                        }
                        int[] plain = Distances(false), broken = Distances(true);
                        for (int y = MirrorTop; y + 1 < height; y++) {
                            for (int x = 1; x < width - 1 && x < r.FullWidth - 1; x++) {
                                int distance = broken[y * width + x];
                                if (distance < 0 || !Shown(r, y) || Kind(x, y) != 0 || !OpenCell(cells, width, x, y - 1) || !OpenCell(cells, width, x - 1, y) || !OpenCell(cells, width, x + 1, y)) continue;
                                if (Enumerable.Range(x - 1, 3).Any(fx => RoomCollision[Kind(fx, y + 1)] is not (0x0D or (>= 0x05 and <= 0x0E) or (>= 0x10 and <= 0x1B)))) continue;
                                if (doors[r].Any(d => Math.Abs(d.X - x) < MirrorSpacing * 2 && Math.Abs(d.Y - y) <= 2) || InArena(r, x, y) || !ViewFits(r, x, y)) continue;
                                int score = distance + 20 * depth.GetValueOrDefault(key.Item3) + (plain[y * width + x] < 0 ? 60 : 0);
                                if (score > bestScore) { bestScore = score; best = (r, x, y); }
                            }
                        }
                    }
                }
                return best.part != null || anyStage ? best : SecretSpot(level, taken, true);
            }

            // The switch rooms; the NiDL hub's mirrors (Central Circle's golden mirror leads to the NiDL hub's, FinishRooms());
            mainHubDoors = new List<Door>();
            katamDoors = new Dictionary<int, List<Door>>();
            HashSet<NightmareRoom> secretRooms = new HashSet<NightmareRoom>();
            HashSet<(int level, int stage)> sideStages = new HashSet<(int, int)>();
            int StageDepth((int level, int stage) s) => parentOf.TryGetValue(s, out var p) ? 1 + StageDepth(p) : 0;
            for (int level = 0; level < 8; level++) {
                NightmareRoom switchRoom = SwitchRoomOf(level);
                // The way in: a Mirra's, hidden in the level (SecretSpot(); the Fountain of Dreams, all boss stage: in Rainbow Resort's,
                // another room), one-way, as KatAM's Mirras, onto its floor where Kirby starts it (SwitchArrival; off its floor mirrors,
                // two-way: the crowded landings' pass below moves it on, as any one-way door's);
                (int x, int y)[] sides = SwitchSides(level);
                (NightmareRoom part, int x, int y) secret = SecretSpot(level < 7 ? level : 6, secretRooms);
                secretRooms.Add(secret.part);
                Add(secret, Target((switchRoom, SwitchArrival(level).x, SwitchArrival(level).y)), DoorKind.SwitchEntry, oneWay: true, hidden: true);
                // its 2 floor mirrors (as KatAM's hubs, 2 ways into their area: the level branches from it), both ways: its right one
                // with the start of the stage leading to its boss (the Fountain of Dreams', Rainbow Resort's), a shortcut earned by
                // finding the room;
                if (gates.TryGetValue(level < 7 ? level : 6, out var gate) && EntrySpot(gate) is (NightmareRoom, int, int) gateSpot) {
                    Add((switchRoom, sides[1].x, sides[1].y), Target(gateSpot), DoorKind.SwitchFloor);
                    Add(gateSpot, Target((switchRoom, sides[1].x, sides[1].y)), DoorKind.SwitchFloor);
                    // its left one with the start of a stage on another branch (off the way from the level's first stage to that one,
                    // the deepest, not the Mirra's, nor the other switch room's into the same level; else the way's, nearest it);
                    var way = new HashSet<(int, int)>();
                    for ((int, int) s = gate; ; s = parentOf[s]) { way.Add(s); if (!parentOf.ContainsKey(s)) break; }
                    (int level, int stage) secretStage = (secret.part.Level, secret.part.Stage);
                    var sides0 = regulars[gate.level].Where(s => s != gate && !sideStages.Contains(s))
                        .OrderBy(s => way.Contains(s) ? 1 : 0).ThenBy(s => s == secretStage ? 1 : 0).ThenByDescending(StageDepth).ThenByDescending(s => s.stage);
                    foreach (var side in sides0) {
                        if (EntrySpot(side) is not (NightmareRoom, int, int) sideSpot) continue;
                        sideStages.Add(side);
                        Add((switchRoom, sides[0].x, sides[0].y), Target(sideSpot), DoorKind.SwitchFloor);
                        Add(sideSpot, Target((switchRoom, sides[0].x, sides[0].y)), DoorKind.SwitchFloor);
                        break;
                    }
                }

                // Its hub doors (2, see LayoutB()): the mirror its big switch reveals (SwitchLinkDoor; Rainbow Resort's, to the hub, is its
                // SwitchHubDoor), and its way back on where it leads (KatAMLinkDoors; Butter Building's on Rainbow Resort's SwitchLinkDoor);
                // the mirror back to the NiDL hub (SwitchHubDoor), Kirby arriving on it from the hub;
                int linked = LevelLinks[level];
                (int x, int y) linkDoor = linked == Hub ? SwitchHubDoor(level) : SwitchLinkDoor(level), hubDoor = SwitchHubDoor(level);
                (int room, int x, int y) linkTarget = Target((switchRoom, linkDoor.x, linkDoor.y));
                (int x, int y) door = KatAMLinkDoors[level].door, landing = KatAMLinkDoors[level].landing;
                (int x, int y) linkedDoor = linked < Hub ? SwitchLinkDoor(-2 - linked) : default;
                (int, int, int) link = linked == Hub ? MainHubAt(MainHubDoors[level]) : linked < 0 ? Target((SwitchRoomOf(-2 - linked), linkedDoor.x, linkedDoor.y))
                                                                                    : (linked, landing.x + 1, landing.y + 1);
                Add((switchRoom, linkDoor.x, linkDoor.y), link, DoorKind.Hub, slot: LevelSlot(level), design: LinkDesign(linked));
                if (linked < Hub) Add((SwitchRoomOf(-2 - linked), linkedDoor.x, linkedDoor.y), linkTarget, DoorKind.Hub, slot: LevelSlot(level), design: NidlDesigns[level]);
                else if (linked != Hub) {
                    if (!katamDoors.ContainsKey(linked)) katamDoors[linked] = new List<Door>();
                    katamDoors[linked].Add(new Door(door.x, door.y, linkTarget.room, linkTarget.x, linkTarget.y, Facing(linkTarget), Slot: LevelSlot(level), Design: NidlDesigns[level], Kind: DoorKind.Hub));
                }
                (int room, int x, int y) arrival = Target((switchRoom, hubDoor.x, hubDoor.y));
                if (linked != Hub) Add((switchRoom, hubDoor.x, hubDoor.y), MainHubAt(MainHubDoors[level]), DoorKind.Hub, slot: LevelSlot(level), design: HubDesign);
                mainHubDoors.Add(new Door(MainHubDoors[level].x, MainHubDoors[level].y, arrival.room, arrival.x, arrival.y, Facing(arrival), Slot: LevelSlot(level), Kind: DoorKind.Hub));
            }
            mainHubDoors.Add(new Door(MainHubReturn.x, MainHubReturn.y, back.room, back.x, back.y, 1, Kind: DoorKind.Hub));
            firstStart = MainHubAt(MainHubReturn);
            firstFacing = 1;

            // The way in, the hub's golden mirror: to where Kirby starts Vegetable Valley's first room, onto a mirror back;
            int firstStage = stages.Where(t => t.level == 0).Min(t => t.stage);
            var firstRoom = Start((0, firstStage, byRoom.Keys.Where(k => k.Item1 == 0 && k.Item2 == firstStage).Min(k => k.Item3)));
            (int room, int x, int y) entryArrival = Target(firstRoom);
            mainHubDoors.Add(new Door(MainHubEntry.x, MainHubEntry.y, entryArrival.room, entryArrival.x, entryArrival.y, Facing(entryArrival), Kind: DoorKind.Entry));
            Add(firstRoom, MainHubAt(MainHubEntry), DoorKind.Entry);

            // The levels' structure (see above): branch doors into stages (two-way, as KatAM's: a branch's way back lands Kirby on its
            // mirror; one-way into the stage where the rooms take no more doors), loops and links between levels, one-way;
            // (from its parent, else from a stage further up, or any other the branch doesn't lead to: the rooms took no more doors);
            bool Under((int, int) stage, (int, int) of) { for (var at = stage; ; at = parentOf[at]) { if (at == of) return true; if (!parentOf.ContainsKey(at)) return false; } }
            foreach (var (parent, to) in branches) {
                bool hiddenBranch = random.Next(2) == 0;
                List<(int level, int stage)> froms = new List<(int, int)>();
                for (var at = parent; ; at = parentOf[at]) { froms.Add(at); if (!parentOf.ContainsKey(at)) break; }
                froms.AddRange(regulars[to.level].Where(t => !froms.Contains(t) && !Under(t, to)).OrderBy(_ => random.Next()));
                bool placed = false;
                // (mirrors kept 2 x MirrorSpacing cells apart, else MirrorSpacing, in small rooms);
                foreach (int spacing in new[] { MirrorSpacing * 2, MirrorSpacing }) {
                    foreach (var from in froms) {
                        if (placed || (HostSpot(from, hiddenBranch, true, spacing) ?? HostSpot(from, !hiddenBranch, true, spacing)) is not (NightmareRoom, int, int) at
                            || EntrySpot(to, spacing) is not (NightmareRoom, int, int) entry) continue;
                        Add(at, Target(entry), DoorKind.Branch);
                        Add(entry, Target(at), DoorKind.BranchBack);
                        placed = true;
                    }
                    foreach (var from in froms) {
                        if (placed || (HostSpot(from, hiddenBranch, false, spacing) ?? HostSpot(from, !hiddenBranch, false, spacing)) is not (NightmareRoom, int, int) host) continue;
                        Add(host, LandingIn(to, true), DoorKind.Branch, oneWay: true);
                        placed = true;
                    }
                }
                // (else another stage's goal throws Kirby into it, as KatAM's goal mirrors throw him on: one not leading to the boss, of a
                // stage it doesn't lead to);
                foreach (var from in froms) {
                    NightmareRoom goalRoom = goalRooms.FirstOrDefault(g => (g.Level, g.Stage) == from && doors[g].Any(d => d.Kind == DoorKind.Goal && !partOf.TryGetValue(d.Room, out NightmareRoom t)));
                    if (placed || goalRoom == null) continue;
                    Door goal = doors[goalRoom].First(d => d.Kind == DoorKind.Goal);
                    (int room, int x, int y) into = LandingIn(to, true);
                    doors[goalRoom][doors[goalRoom].IndexOf(goal)] = goal with { Room = into.room, DestX = into.x, DestY = into.y, Facing = Facing(into) };
                    placed = true;
                }
                if (!placed) throw new InvalidOperationException($"No room for a branch door in level {parent.level}, into stage {to.stage}.");
            }
            // (a link between levels from any stage of its level that takes a door; small rooms' mirrors MirrorSpacing apart, if none);
            foreach (var (a, b, hidden, _) in links) {
                var froms = new[] { a }.Concat(a.level == b.level ? Array.Empty<(int, int)>() : regulars[a.level].Where(t => t != a).OrderBy(_ => random.Next())).ToList();
                var host = froms.Select(f => HostSpot(f, hidden, false)).FirstOrDefault(h => h != null) ?? froms.Select(f => HostSpot(f, hidden, false, MirrorSpacing)).FirstOrDefault(h => h != null);
                if (host is not (NightmareRoom, int, int) one) continue;
                Add(one, LandingIn(b), a.level == b.level ? DoorKind.Loop : DoorKind.Link, oneWay: true);
            }
            // KatAMLinks (per level: one throwing Kirby one-way from NiDL into KatAM's area, one from KatAM's into NiDL, as KatAM's own
            // one-way mirrors throw him across areas);
            for (int link = 0; link < KatAMLinks * 7; link++) {
                int level = link % 7;
                if (regulars[level].Count == 0) continue;
                (int level, int stage) stage = regulars[level][random.Next(regulars[level].Count)];
                if (link / 7 == 0) {
                    var host = regulars[level].OrderBy(t => t == stage ? 0 : 1 + random.Next(100)).Select(t => HostSpot(t, false, false) ?? HostSpot(t, false, false, MirrorSpacing)).FirstOrDefault(h => h != null);
                    if (host is not (NightmareRoom, int, int) spot || KatAMSpot(KatAMArea(level), true) is not (int room, int x, int y) katam) continue;
                    Add(spot, (katam.room, katam.x + 1, katam.y + 1), DoorKind.KatAMLink, oneWay: true);
                } else {
                    if (KatAMSpot(KatAMArea(level), true) is not (int room, int x, int y) katam) continue;
                    (int room, int x, int y) to = LandingIn(stage);
                    if (!katamDoors.ContainsKey(katam.room)) katamDoors[katam.room] = new List<Door>();
                    katamDoors[katam.room].Add(new Door(katam.x, katam.y, to.room, to.x, to.y, Facing(to), OneWay: true, Kind: DoorKind.KatAMLink));
                }
            }


            /* Every room reached from the NiDL hub (Reached(): by its doors, warp stars and cannons, through its open cells): a group of
             * rooms none leads into (the swaps above can leave one, a stage's far end leading only into another's) gets a one-way door
             * into its first room, a throw from a room reached, of the same level if one takes it, as KatAM's one-way mirrors; */
            Dictionary<int, NightmareRoom> numbered = rooms.ToDictionary(r => r.Room, r => r);
            List<Door> hubDoors = mainHubDoors;
            // Reached(); The rooms Kirby gets to from the hub: from each cell he arrives in, through open cells (and breakable blocks),
            // to the doors, warp stars and cannons there (a door at a cell he reaches, or beside it); not through KatAM's rooms;
            HashSet<int> Reached() {
                Dictionary<int, HashSet<(int, int)>> arrived = new Dictionary<int, HashSet<(int, int)>>();
                Queue<(int room, int x, int y)> open = new Queue<(int, int, int)>();
                void Arrive(int room, int x, int y) {
                    if (!numbered.ContainsKey(room)) return;
                    if (!arrived.TryGetValue(room, out var cells)) arrived[room] = cells = new HashSet<(int, int)>();
                    if (cells.Add((x, y))) open.Enqueue((room, x, y));
                }
                foreach (Door d in hubDoors) Arrive(d.Room, d.DestX - 1, d.DestY - 1);
                Dictionary<int, HashSet<(int, int)>> flooded = new Dictionary<int, HashSet<(int, int)>>();
                while (open.Count > 0) {
                    var (room, sx, sy) = open.Dequeue();
                    NightmareRoom r = numbered[room];
                    if (!flooded.TryGetValue(room, out var seen)) flooded[room] = seen = new HashSet<(int, int)>();
                    if (!partCells.TryGetValue(r, out byte[] cells)) partCells[r] = cells = Cells(r);
                    int width = ReadShort(nightmare, r.Record + 0x14);
                    bool Open(int x, int y) => x >= 0 && y >= 0 && x < r.Width && y < r.Height && OpenCell(cells, width, r.X + x, r.Y + y, true);
                    Queue<(int x, int y)> cellsOpen = new Queue<(int, int)>();
                    sx = Math.Clamp(sx, 0, r.Width - 1); sy = Math.Clamp(sy, 0, r.Height - 1);
                    if (seen.Add((sx, sy))) cellsOpen.Enqueue((sx, sy));
                    while (cellsOpen.Count > 0) {
                        var (x, y) = cellsOpen.Dequeue();
                        foreach (var (nx, ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                            if (Open(nx, ny) && seen.Add((nx, ny))) cellsOpen.Enqueue((nx, ny));
                    }
                    bool Near(int x, int y) => Enumerable.Range(-1, 3).Any(dx => Enumerable.Range(-1, 3).Any(dy => seen.Contains((x + dx, y + dy))));
                    foreach (Door d in doors[r]) if (d.Edge ? seen.Contains((d.X, d.Y)) : Near(d.X, d.Y)) Arrive(d.Room, d.DestX - 1, d.DestY - 1);
                    foreach (Transport t in transportsOf[r]) if (Near(t.X / 16, (t.Y - 1) / 16)) Arrive(t.Room, t.DestX - 1, t.DestY - 1);
                    if (bossExitsOf.TryGetValue(r, out var shard)) Arrive(shard.room, shard.x - 1, shard.y - 1);
                }
                return arrived.Keys.ToHashSet();
            }
            for (int pass = 0; pass < rooms.Count; pass++) {
                HashSet<int> reached = Reached();
                NightmareRoom lost = rooms.Where(r => !r.IsSwitchRoom && !reached.Contains(r.Room)).OrderBy(r => (r.Level, r.Stage, r.Index)).FirstOrDefault();
                if (lost == null) break;
                var host = rooms.Where(r => reached.Contains(r.Room) && Hosts(r, false)).OrderBy(r => r.Level == lost.Level ? 0 : 1).ThenBy(_ => random.Next())
                                .Select(r => RoomSpot(r, false) ?? RoomSpot(r, false, MirrorSpacing)).FirstOrDefault(h => h != null);
                if (host is not (NightmareRoom, int, int) from) throw new InvalidOperationException($"No room for a door into room 0x{lost.Room:X}.");
                (int room, int x, int y) into = RoomSpot(lost, false, door: false) is (NightmareRoom, int, int) spot ? Target(spot) : Target(Start((lost.Level, lost.Stage, lost.Index)));
                Add(from, into, DoorKind.Link, oneWay: true);
            }

            /* Every place Kirby gets to gets him back out (to the NiDL hub or KatAM's rooms, which lead back to it): a place (a room's
             * cells he gets through from where he arrives) none of whose doors, warp stars and cannons lead out, nor to a
             * place that does (the swaps and spots above can leave one: a cannon's landing past the stage's end), gets a one-way door
             * out, as KatAM's one-way mirrors throw him on: into the first stage of its level he gets back from (else onto the hub's
             * pink mirror); */
            for (int pass = 0; pass < rooms.Count; pass++) {
                List<(NightmareRoom r, HashSet<(int, int)> cells, List<(int room, int x, int y)> leads)> places = new List<(NightmareRoom, HashSet<(int, int)>, List<(int, int, int)>)>();
                Dictionary<(int, int, int), int> placeOf = new Dictionary<(int, int, int), int>();
                // PlaceAt(); The place an arrival leads into (-1: out of NiDL's rooms), flooding it the first time (one per arrival: a thin
                // floor lets Kirby up, not down);
                int PlaceAt(int room, int x, int y) {
                    if (!numbered.TryGetValue(room, out NightmareRoom r)) return -1;
                    x = Math.Clamp(x, 0, r.Width - 1); y = Math.Clamp(y, 0, r.Height - 1);
                    if (placeOf.TryGetValue((room, x, y), out int known)) return known;
                    if (!partCells.TryGetValue(r, out byte[] cells)) partCells[r] = cells = Cells(r);
                    int width = ReadShort(nightmare, r.Record + 0x14);
                    // (through all but walls: Kirby flies, breaks blocks; never down through a thin floor, which needs Down and B);
                    byte Kind(int cx, int cy) => cells[((r.Y + cy) * width + r.X + cx) * 4 + 3];
                    bool Open(int cx, int cy) => cx >= 0 && cy >= 0 && cx < r.Width && cy < r.Height && (RoomCollision[Kind(cx, cy)] != 0x0D || Kind(cx, cy) >> 4 == 2 && Kind(cx, cy) != 0x2F);
                    bool Thin(int cx, int cy) => RoomCollision[Kind(cx, cy)] is 0x0E or (>= 0x10 and <= 0x13) or 0x18 or 0x19;
                    HashSet<(int, int)> seen = new HashSet<(int, int)> { (x, y) };
                    Queue<(int x, int y)> open = new Queue<(int, int)>(seen);
                    while (open.Count > 0) {
                        var (cx, cy) = open.Dequeue();
                        foreach (var (nx, ny) in new[] { (cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1) })
                            if (Open(nx, ny) && !(ny == cy + 1 && Thin(nx, ny)) && seen.Add((nx, ny))) open.Enqueue((nx, ny));
                    }
                    bool Near(int dx, int dy) => Enumerable.Range(-1, 3).Any(ox => Enumerable.Range(-1, 3).Any(oy => seen.Contains((dx + ox, dy + oy))));
                    var leads = doors[r].Where(d => d.Edge ? seen.Contains((d.X, d.Y)) : Near(d.X, d.Y)).Select(d => (d.Room, d.DestX - 1, d.DestY - 1))
                                        .Concat(transportsOf[r].Where(t => Near(t.X / 16, (t.Y - 1) / 16)).Select(t => (t.Room, t.DestX - 1, t.DestY - 1)))
                                        .Concat(bossExitsOf.TryGetValue(r, out var shard) ? new[] { (shard.room, shard.x - 1, shard.y - 1) } : new (int, int, int)[0]).ToList();
                    places.Add((r, seen, leads));
                    return placeOf[(room, x, y)] = places.Count - 1;
                }
                Queue<int> unvisited = new Queue<int>();
                HashSet<int> visited = new HashSet<int>();
                foreach (Door d in hubDoors) if (PlaceAt(d.Room, d.DestX - 1, d.DestY - 1) is int first && first >= 0 && visited.Add(first)) unvisited.Enqueue(first);
                while (unvisited.Count > 0)
                    foreach (var (room, x, y) in places[unvisited.Dequeue()].leads)
                        if (PlaceAt(room, x, y) is int next && next >= 0 && visited.Add(next)) unvisited.Enqueue(next);
                // (the places he gets out from: leading out, or to one that does);
                HashSet<int> outs = new HashSet<int>();
                for (bool more = true; more;) {
                    more = false;
                    foreach (int i in visited)
                        if (!outs.Contains(i) && places[i].leads.Any(l => PlaceAt(l.room, l.x, l.y) is int to && (to < 0 || outs.Contains(to)))) { outs.Add(i); more = true; }
                }
                var trapped = visited.Where(i => !outs.Contains(i)).OrderBy(i => (places[i].r.Level, places[i].r.Stage, places[i].r.Index)).ToList();
                if (trapped.Count == 0) break;
                (NightmareRoom part, int x, int y)? from = null;
                foreach (int i in trapped) {
                    NightmareRoom r = places[i].r;
                    if (r.IsSwitchRoom || IsBossStage((r.Level, r.Stage)) || byRoom[(r.Level, r.Stage, r.Index)].Count != 1) continue;
                    from = RoomSpot(r, false, within: places[i].cells) ?? RoomSpot(r, false, MirrorSpacing, within: places[i].cells)
                        ?? RoomSpot(r, false, MirrorSpacing, false, places[i].cells);
                    if (from != null) break;
                }
                if (from is not (NightmareRoom, int, int) spot) throw new InvalidOperationException($"No way out of room 0x{places[trapped[0]].r.Room:X}.");
                (int room, int x, int y) to = MainHubAt(MainHubReturn);
                foreach (var stage in stages.Where(t => t.level == spot.part.Level && !IsBossStage(t)).OrderBy(t => t.stage)) {
                    var start = StageStart(stage);
                    if (PlaceAt(start.Item1, start.Item2 - 1, start.Item3 - 1) is int at && outs.Contains(at)) { to = start; break; }
                }
                Add(spot, to, DoorKind.Link, oneWay: true);
            }

            /* The owner's one-way entrances (OneWayEntrances): the door from the first room into the second throws Kirby in, one-way; the
             * second's doors back to the first go, after all the rest is planned (so no other door changes: dropping it earlier
             * moved the doors planned after it, and cut Grape Garden's first stage off); the logic checks find nowhere trapped; */
            foreach (((int, int, int) first, (int, int, int) second) in OneWayEntrances) {
                foreach (NightmareRoom to in rooms.Where(r => (r.Level, r.Stage, r.Index) == second))
                    foreach (NightmareRoom from in rooms.Where(r => (r.Level, r.Stage, r.Index) == first)) {
                        doors[to].RemoveAll(d => !d.Edge && d.Room == from.Room);
                        for (int i = 0; i < doors[from].Count; i++)
                            if (!doors[from][i].Edge && doors[from][i].Room == to.Room) doors[from][i] = doors[from][i] with { OneWay = true };
                    }
            }
            /* Which doors are one-way, for good: one is two-way when the door it lands Kirby on leads back onto it (or, NiDL's over a fall,
             * where Kirby lands beside them, the door beside the landing back beside it: up to 2 cells across, 3 up or down); KatAM draws
             * the others as one-way mirrors (BuildObjects()); the doors into KatAM's rooms keep what they were made as. A mini-boss's room
             * is one-way in and one-way out (0x29B's two doors, NiDL's, were two-way pairs): its doors and the doors into it are one-way,
             * their landings off the doors (Clear(), below); */
            Dictionary<int, List<Door>> listOf = doors.ToDictionary(p => p.Key.Room, p => p.Value);
            listOf[mainHub] = mainHubDoors;
            foreach (var (room, list) in katamDoors) listOf[room] = list;
            bool Near(int x, int y, int toX, int toY) => Math.Abs(x - toX) <= 2 && Math.Abs(y - toY) <= 3;
            bool Back(int room, Door d) => listOf[d.Room].Any(e => !e.Edge && e.Room == room && Near(e.X, e.Y, d.DestX - 1, d.DestY - 1) && Near(e.DestX - 1, e.DestY - 1, d.X, d.Y));
            bool Miniboss(int room) => partOf.TryGetValue(room, out NightmareRoom part) && MinibossRoom(part);
            foreach (var (room, list) in listOf)
                for (int i = 0; i < list.Count; i++)
                    if (!list[i].Edge && list[i].Slot == 0 && listOf.ContainsKey(list[i].Room) && (partOf.ContainsKey(list[i].Room) || list[i].Room == mainHub))
                        list[i] = list[i] with { OneWay = !Back(room, list[i]) || Miniboss(room) || Miniboss(list[i].Room) };

            // Blocks a door lands Kirby in are left broken (clearedBlocks);
            foreach (Door d in listOf.Values.SelectMany(list => list).Where(d => !d.Edge)) {
                if (!partOf.TryGetValue(d.Room, out NightmareRoom to)) continue;
                if (!cellsOf.TryGetValue(to, out byte[] cells)) cellsOf[to] = cells = Cells(to);
                int width = ReadShort(nightmare, to.Record + 0x14), height = ReadShort(nightmare, to.Record + 0x16);
                int first = (to.Y + d.DestY - 1) * width + to.X + d.DestX - 1;
                if (d.DestX < 1 || d.DestY < 1 || d.DestX > to.Width || d.DestY > to.Height || cells[first * 4 + 3] >> 4 != 2) continue;
                if (!clearedBlocks.TryGetValue(to.Record, out HashSet<int> group)) clearedBlocks[to.Record] = group = new HashSet<int>();
                Queue<int> open = new Queue<int>();
                if (group.Add(first)) open.Enqueue(first);
                while (open.Count > 0) {
                    int i = open.Dequeue();
                    foreach (int n in new[] { i % width > 0 ? i - 1 : -1, i % width + 1 < width ? i + 1 : -1, i - width, i + width })
                        if (n >= 0 && n < width * height && cells[n * 4 + 3] >> 4 == 2 && group.Add(n)) open.Enqueue(n);
                }
            }

            /* NiDL's door halls (Butter Building's tower floors, Rainbow Resort's rooms of doors) keep their doors past the rules
             * (MaxExits(), ViewFits()), without mirrors: NiDL's own doorways, drawn in its tiles, as NiDL has them; */
            foreach (NightmareRoom r in rooms.Where(r => !r.IsSwitchRoom)) {
                List<Door> mirrored = new List<Door>();
                foreach (Door d in doors[r].Where(d => !d.Edge).OrderBy(d => d.Kind == DoorKind.Nidl ? 1 : 0).ToList()) {
                    if (d.Kind != DoorKind.Nidl || mirrored.Count < MaxExits(r) && InView(mirrored, d.X, d.Y)) mirrored.Add(d);
                    else doors[r][doors[r].IndexOf(d)] = d with { Plain = true };
                }
            }

            /* No one-way door throws Kirby onto a door, nor in front of a two-way one (4 cells across, 2 up or down), as KatAM's: its landing
             * moves to a floor nearby away from them (up to ReturnReach cells, the camera showing it), else anywhere in the room
             * (RoomSpot()); not the boss stages' (Kirby lands by the next room's mirror there, as KatAM's boss rooms); */
            // (a mini-boss room's doors, and doors into one, count as two-way ones: one-way in and out, Kirby never lands right by the way
            // back, which made 0xF5's pair with 0xF1 work both ways);
            bool Crowded(NightmareRoom r, int x, int y, bool near = false) => doors[r].Any(e => !e.Edge && (Math.Abs(e.X - x) <= 1 && Math.Abs(e.Y - y) <= 1
                || !near && (!e.OneWay || MinibossRoom(r) || Miniboss(e.Room)) && Math.Abs(e.X - x) <= 4 && Math.Abs(e.Y - y) <= 2));
            // (near: a second try, off the doors only, twice as far, before anywhere in the room: a mini-boss room's way out landed
            // Kirby out of reach of the rest, 0x29A's into 0x224);
            // Joined(); Whether Kirby goes from one cell of a room to another through its open cells;
            bool Joined(NightmareRoom r, (int x, int y) from, (int x, int y) to) {
                if (!cellsOf.TryGetValue(r, out byte[] cells)) cellsOf[r] = cells = Cells(r);
                int width = ReadShort(nightmare, r.Record + 0x14);
                bool Open((int x, int y) c) => c.x >= 0 && c.x < r.Width && c.y >= 0 && c.y < r.Height && OpenCell(cells, width, r.X + c.x, r.Y + c.y);
                HashSet<(int, int)> seen = new HashSet<(int, int)> { from };
                Queue<(int x, int y)> open = new Queue<(int, int)>(seen);
                while (open.Count > 0) {
                    (int x, int y) c = open.Dequeue();
                    if (c == to) return true;
                    foreach ((int x, int y) n in new[] { (c.x + 1, c.y), (c.x - 1, c.y), (c.x, c.y + 1), (c.x, c.y - 1) })
                        if (Open(n) && seen.Add(n)) open.Enqueue(n);
                }
                return false;
            }
            // (spaced: the second try, twice as far, still keeping clear of the doors as the first does: a mini-boss room's doors');
            // (blocks: through the blocks Kirby breaks, too, to a floor past them: 0xF1's door into 0xF5 sits behind a bomb block's
            // chain, all walled in, and the way back from 0xF5 landed by it, a two-way door);
            (int x, int y)? Clear(NightmareRoom r, int x, int y, bool near = false, bool spaced = false, bool blocks = false) {
                int reach = near ? 2 * ReturnReach : ReturnReach;
                if (!cellsOf.TryGetValue(r, out byte[] cells)) cellsOf[r] = cells = Cells(r);
                int width = ReadShort(nightmare, r.Record + 0x14);
                bool Open(int cx, int cy, bool broken = false) => cx >= 1 && cx < Math.Min(r.Width, r.FullWidth - 1) && cy >= 0 && cy < r.Height && OpenCell(cells, width, r.X + cx, r.Y + cy, broken);
                bool Floor(int cx, int cy) => cy < r.Height && RoomCollision[cells[((r.Y + cy) * width + r.X + cx) * 4 + 3]] is >= 0x01 and <= 0x1B;
                // (a cell Kirby goes to from the landing, through open cells: not one behind a wall);
                HashSet<(int, int)> reached = new HashSet<(int, int)> { (x, y) };
                Queue<(int x, int y)> open = new Queue<(int, int)>(reached);
                while (open.Count > 0) {
                    (int ox, int oy) = open.Dequeue();
                    foreach ((int nx, int ny) in new[] { (ox + 1, oy), (ox - 1, oy), (ox, oy + 1), (ox, oy - 1) })
                        if (Math.Abs(nx - x) <= reach && Math.Abs(ny - y) <= reach && Open(nx, ny, blocks) && reached.Add((nx, ny))) open.Enqueue((nx, ny));
                }
                (int x, int y)? best = null;
                int bestDistance = int.MaxValue;
                for (int cy = Math.Max(MirrorTop, y - reach); cy <= y + reach && cy + 1 < r.Height; cy++)
                    for (int cx = x - reach; cx <= x + reach; cx++) {
                        int distance = Math.Abs(cx - x) + 2 * Math.Abs(cy - y);
                        if (distance < bestDistance && reached.Contains((cx, cy)) && Open(cx, cy) && Shown(r, cy) && Open(cx, cy - 1) && Floor(cx, cy + 1) && !Crowded(r, cx, cy, near && !spaced)) {
                            best = (cx, cy);
                            bestDistance = distance;
                        }
                    }
                return best;
            }
            foreach (var (room, list) in listOf) {
                for (int i = 0; i < list.Count; i++) {
                    Door d = list[i];
                    if (!d.OneWay || d.Edge || d.Kind == DoorKind.Boss || !partOf.TryGetValue(d.Room, out NightmareRoom to) || IsBossRoom(to) || !Crowded(to, d.DestX - 1, d.DestY - 1)) continue;
                    // (a mini-boss room's doors and the doors into one first try twice as far spaced as the first: off the doors only, it
                    // landed Kirby by the way back, 0xF5's into 0xF1; only where there's no such floor, as 0x224's pocket 0x29A's way out
                    // lands in, with the only door to 0x238, the second try as the others');
                    bool fight = Miniboss(room) || Miniboss(d.Room);
                    (int x, int y)? cell = Clear(to, d.DestX - 1, d.DestY - 1) ?? (fight ? Clear(to, d.DestX - 1, d.DestY - 1, near: true, spaced: true)
                                                                                         ?? Clear(to, d.DestX - 1, d.DestY - 1, near: true, spaced: true, blocks: true) : null);
                    // (then, for those, a floor anywhere in the room, if Kirby walks there from where the door landed him: 0xF1's);
                    if (cell == null && fight && RoomSpot(to, false, door: false) is (NightmareRoom, int, int) far && Joined(to, (d.DestX - 1, d.DestY - 1), (far.x, far.y)))
                        cell = (far.x, far.y);
                    cell ??= Clear(to, d.DestX - 1, d.DestY - 1, near: true);
                    if (cell == null && RoomSpot(to, false, door: false) is (NightmareRoom, int, int) spot) cell = (spot.x, spot.y);
                    if (cell is (int x, int y)) list[i] = d with { DestX = x + 1, DestY = y + 1, Facing = Facing((to.Room, x + 1, y + 1)) };
                }
            }
            return doors;
        }

        /* FinishRooms(); After the randomizer: the KatAM rooms getting doors (editedRooms: Central Circle, EntryRoom, its door to Nightmare
         * in Dream Land, the golden mirror its tilemap got in AddRooms(); the KatAM rooms NiDL levels lead to, their hidden doors back)
         * get them in copies of their collision, special blocks (8ROM), objects and doors list (9ROM) as the randomizer left them; the
         * 9ROM lists keep their door counts, so the map's doors don't change; */
        public static void FinishRooms(byte[] rom) {
            int cursor = finishCursor;

            /* The ported rooms' doors lists: their chests as the randomizer left them (moved, removed, emptied into items, a KatAM chest
             * shuffled in), first, then the list's doors (a shard chest isn't listed: chest.c doesn't mark it); */
            foreach ((int room, int list, int objects) in chestLists) {
                List<byte> chests = new List<byte>();
                for (int at = ReadInt(rom, objects) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36)
                    if (rom[at + 12] is SmallChestType or 0x81 && !(rom[at + 14] >= KatAMItems.ShardChest && rom[at + 14] < KatAMItems.ShardChest + 8))
                        chests.AddRange(ChestEntry((short)ReadShort(rom, at + 6), (short)ReadShort(rom, at + 8)));
                int header = ReadInt(rom, list) - 0x08000000;
                List<byte> listed = new List<byte>(), rest = new List<byte>();
                for (int at = ReadInt(rom, header) - 0x08000000; rom[at + 1] != 0; at += rom[at + 1])
                    (rom[at] == 1 ? listed : rest).AddRange(rom.Skip(at).Take(rom[at + 1]));
                if (listed.SequenceEqual(chests)) continue;
                byte[] nineROM = chests.Concat(rest).Concat(new byte[] { 0, 0, 0xFF, 0xFF }).ToArray();
                int nineROMAt = Allocate(ref cursor, nineROM.Length, 4);
                nineROM.CopyTo(rom, nineROMAt);
                WriteInt(rom, header, nineROMAt + 0x08000000);
                rom[header + 4] = (byte)(chests.Count / 8);
            }
            chestLists = new List<(int, int, int)>();

            // KatAM's enemies (types under 0x38) where the ported rooms' doors land Kirby in its rooms, left out (room 0x142's by the
            // landing from 0xF4, the only spot there its door reaches);
            foreach ((int entry, List<(int x, int y)> spots) in katamLandings) {
                int list = ReadInt(rom, entry) - 0x08000000;
                List<byte[]> templates = new List<byte[]>();
                for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) templates.Add(rom.Skip(at).Take(36).ToArray());
                int removed = templates.RemoveAll(t => t[12] < 0x38 && spots.Any(c => Math.Abs((short)ReadShort(t, 6) / 16 - c.x) <= 3 && Math.Abs((short)ReadShort(t, 8) / 16 - c.y) <= 2));
                if (removed == 0) continue;
                List<byte> types = new List<byte>();
                for (int i = 0, at = ReadInt(rom, list + 4) - 0x08000000; i < rom[list + 12]; i++) types.Add(rom[at + i]);
                WriteInt(rom, entry, WriteObjectList(rom, templates, types, ReadInt(rom, list + 8) - 0x08000000, ref cursor) + 0x08000000);
            }
            foreach (EditedRoom edited in editedRooms) {
                int width = ReadShort(rom, edited.Tilemap) / 2, columns = width * 2;
                byte[] tilemap = DecompressLZ77(rom, ReadInt(rom, edited.Tilemap + 0x18) - 0x08000000);

                // Collision and special blocks: a door's record; a hidden door (see BuildSolidity()), a special block its door object
                // breaks (the cell's own tiles, and the door's collision);
                int solidity = ReadInt(rom, edited.SolidityEntry) - 0x08000000;
                byte[] collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);
                List<byte> specials = new List<byte>();
                for (int at = ReadInt(rom, solidity + 4) - 0x08000000; rom[at + 4] != 0; at += rom[at + 4]) {
                    byte[] record = rom.Skip(at).Take(rom[at + 4]).ToArray();
                    // (a moved door, MovedDoors: its records at its new cell; a door landing on it, there);
                    if (MovedDoors.TryGetValue(edited.Room, out var moved) && record[2] == moved.x && record[3] == moved.y) { record[2] = (byte)moved.toX; record[3] = (byte)moved.toY; }
                    if (record[0] == 3 && record.Length >= 12 && MovedDoors.TryGetValue(ReadShort(record, 8), out var landing) && LandsOn(record[10], record[11], landing)) {
                        record[10] += (byte)(landing.toX - landing.x); record[11] += (byte)(landing.toY - landing.y);
                    }
                    specials.AddRange(record);
                }
                if (MovedDoors.TryGetValue(edited.Room, out var move)) {
                    (collision[move.y * width + move.x], collision[move.toY * width + move.toX]) = (collision[move.toY * width + move.toX], collision[move.y * width + move.x]);
                }
                foreach (Door door in edited.Doors) {
                    specials.AddRange(DoorRecord(door, 0xFF));
                    if (door.Slot == 0) { collision[door.Y * width + door.X] = 0xFF; continue; }
                    specials.AddRange(new byte[] { 1, collision[door.Y * width + door.X], (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                    for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ReadShort(tilemap, ((door.Y * 2 + q / 2) * columns + door.X * 2 + q % 2) * 2)));
                    specials.AddRange(new byte[] { 0xFF, 0xFF });
                }
                specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                byte[] compressed = CompressRL(collision);
                int collisionAt = Allocate(ref cursor, compressed.Length, 4), specialsAt = Allocate(ref cursor, specials.Count, 4), newSolidity = Allocate(ref cursor, 8, 4);
                compressed.CopyTo(rom, collisionAt);
                specials.ToArray().CopyTo(rom, specialsAt);
                WriteInts(rom, newSolidity, collisionAt + 0x08000000, specialsAt + 0x08000000);
                WriteInt(rom, edited.SolidityEntry, newSolidity + 0x08000000);

                // Objects: the hidden doors' door objects (0x97) after the room's;
                if (edited.ObjectsEntry != 0) {
                    int list = ReadInt(rom, edited.ObjectsEntry) - 0x08000000;
                    List<byte[]> templates = new List<byte[]>();
                    for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) templates.Add(rom.Skip(at).Take(36).ToArray());
                    if (DecorationMoves.TryGetValue(edited.Room, out var moves))
                        foreach (var decoration in moves)
                            foreach (byte[] template in templates.Where(t => t[12] == decoration.type && ReadShort(t, 6) == decoration.fromX)) WriteShort(template, 6, decoration.toX);
                    if (MovedDoors.TryGetValue(edited.Room, out var door97))
                        foreach (byte[] template in templates.Where(t => t[12] == DoorToHubType && ReadShort(t, 6) == door97.x * 16 + 8 && ReadShort(t, 8) == (door97.y + 1) * 16))
                            WriteShorts(template, 6, door97.toX * 16 + 8, (door97.toY + 1) * 16);
                    List<byte> types = new List<byte>();
                    for (int i = 0, at = ReadInt(rom, list + 4) - 0x08000000; i < rom[list + 12]; i++) types.Add(rom[at + i]);
                    foreach (Door door in edited.Doors.Where(d => d.Slot != 0)) {
                        byte[] template = DoorToHubTemplate(door.X, door.Y, templates.Count, door.Slot, door.Design);
                        if (DoorDrawX.TryGetValue(edited.Room, out int drawX)) WriteShort(template, 6, drawX);
                        templates.Add(template);
                    }
                    if (!types.Contains(DoorToHubType)) types.Add(DoorToHubType);
                    // (the mirrors to NiDL's levels, always there: KatAM's mirror object, as the ported rooms');
                    foreach (Door door in edited.Doors.Where(d => d.Slot == 0 && !d.Edge && edited.Room != EntryRoom)) {
                        byte[] mirror = MirrorTemplate(door.X, door.Y, templates.Count);
                        if (door.OneWay) mirror[0x23] |= 1;    // (a one-way mirror, as in BuildObjects());
                        templates.Add(mirror);
                    }
                    if (edited.Doors.Any(d => d.Slot == 0 && edited.Room != EntryRoom) && !types.Contains(MirrorType)) types.Add(MirrorType);
                    int newList = WriteObjectList(rom, templates, types, ReadInt(rom, list + 8) - 0x08000000, ref cursor);
                    WriteInt(rom, edited.ObjectsEntry, newList + 0x08000000);
                }

                // 9ROM: its doors list with the new doors, before the list's end (its door count stays: the map doesn't count them);
                int doorsInfo = ReadInt(rom, edited.DoorsEntry) - 0x08000000;
                List<byte> nineROM = new List<byte>();
                for (int at = ReadInt(rom, doorsInfo) - 0x08000000; rom[at + 1] != 0; at += rom[at + 1]) {
                    byte[] entry = rom.Skip(at).Take(rom[at + 1]).ToArray();
                    if (entry[0] == 2 && entry.Length >= 8 && MovedDoors.TryGetValue(ReadShort(entry, 4), out var landing) && LandsOn(entry[6], entry[7], landing)) {
                        entry[6] += (byte)(landing.toX - landing.x); entry[7] += (byte)(landing.toY - landing.y);
                    }
                    nineROM.AddRange(entry);
                }
                foreach (Door door in edited.Doors) nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
                nineROM.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
                int nineROMAt = Allocate(ref cursor, nineROM.Count, 4), newDoorsInfo = Allocate(ref cursor, 8, 4);
                nineROM.ToArray().CopyTo(rom, nineROMAt);
                Array.Copy(rom, doorsInfo, rom, newDoorsInfo, 8);
                WriteInt(rom, newDoorsInfo, nineROMAt + 0x08000000);
                WriteInt(rom, edited.DoorsEntry, newDoorsInfo + 0x08000000);
            }
            editedRooms = new List<EditedRoom>();
        }

        const byte MirrorType = 0x6F, MirraType = 0x34, WarpStarType = 0x84, CannonType = 0x8A, FuseType = 0x78;

        // MirrorTemplate(); A mirror (object 0x6F) standing on a door cell, as KatAM's (its x, y: the cell's center);
        static byte[] MirrorTemplate(int x, int y, int number) => ObjectTemplate(x * 16 + 8, y * 16 + 8, number, MirrorType);

        /* DoorToHubTemplate(); A door object (0x97, CreateDoorToHub) under a door cell (its x, y: the cell's bottom center), as KatAM's
         * switch rooms': once the world slot is set, it breaks the cell's special block (making the door) and shows its mirror, whose
         * frame is the design (anim 0x2C6's variant); */
        const byte DoorToHubType = 0x97;
        static byte[] DoorToHubTemplate(int x, int y, int number, int slot, int design) {
            byte[] template = ObjectTemplate(x * 16 + 8, (y + 1) * 16, number, DoorToHubType);
            template[4] = (byte)slot;
            template[14] = template[15] = (byte)design;
            return template;
        }

        // HubMirrorTemplate(); A hub mirror object (0x8C) at a cell's center, revealing the door below it once the world slot is set;
        static byte[] HubMirrorTemplate(int x, int y, int number, int slot) {
            byte[] template = ObjectTemplate(x * 16 + 8, y * 16 + 8, number, HubMirrorType);
            template[4] = (byte)slot; template[5] = 1;
            return template;
        }

        /* BigSwitchTemplate(); A big switch (object 0x86, as KatAM's: +0x22 6) setting the world slot; its +0x14 picks what pressing
         * it runs: the level's world map door (see KatAMExpansionWorldMap.cs); */
        static byte[] BigSwitchTemplate(int x, int y, int number, int slot, int level) {
            byte[] template = ObjectTemplate(x, y, number, BigSwitchType);
            template[4] = (byte)slot; template[5] = 1;
            WriteShort(template, 0x14, NidlBigSwitchId(level));
            template[0x22] = 6;
            return template;
        }

        static byte[] ObjectTemplate(int x, int y, int number, byte type) {
            byte[] template = new byte[36];
            template[0] = 0x01; template[1] = 0x24; template[3] = (byte)number;
            WriteShorts(template, 6, x, y);
            template[10] = template[11] = template[13] = 0xFF;
            template[12] = type;
            return template;
        }

        // NineROMEntry(); A 9ROM entry: kind 2 (a door), size 8, the room and landing cell + 1 it leads to;
        // ChestEntry(); A 9ROM chest entry: kind 1, size 8, the chest's spot (its template's x, y);
        static byte[] ChestEntry(int x, int y) => new byte[] { 1, 8, 0xFF, 0xFF, (byte)x, (byte)(x >> 8), (byte)y, (byte)(y >> 8) };
        static byte[] NineROMEntry(int room, int x, int y) => new byte[] { 2, 8, 0xFF, 0xFF, (byte)room, (byte)(room >> 8), (byte)x, (byte)y };

        // DoorRecord(); An 8ROM door record: kind 3, collision, cell, size 14, room, landing cell + 1, facing;
        static byte[] DoorRecord(Door door, byte collision) {
            byte[] record = { 3, collision, (byte)door.X, (byte)door.Y, 0x0E, 0, 0xFF, 0xFF, 0, 0, (byte)door.DestX, (byte)door.DestY, door.Facing, 0xFF };
            WriteShort(record, 8, door.Room);
            return record;
        }

        /* Breakable blocks (NiDL collision 0x2X) -> KatAM's: star blocks 0xF0 (0x24, and Rainbow Resort's few 0x20), bomb blocks 0xF1
         * (0x2E), the blocks a bomb block sets off 0xF2 (0x2D, touching it and each other: KatAM breaks them as a chain from the bomb
         * block), metal blocks 0xF5 (0x2F, NiDL's grey ones: KatAM's silver metal blocks, as room 0x134's, which only some abilities break;
         * 0xF4 breaks with any ability's attack: code_0800ECAC_2.c's helpers try type 0 and 1 blocks only), but where a room has more
         * than MetalBlocks of them (Heavy Mole's dirt, which any attack digs through) star blocks (RoomCells()); */
        /* RoomCells(); A NiDL room's cells (u32 each: metatile, attribute, collision), its bomb chains as NiDL's: a chain's blocks are
         * NiDL's own (its bomb blocks only where NiDL has them: Kirby arriving on a chain's far side leaves by the two-way door he came
         * through, and the logic checks find every room reached and nowhere trapped). A chain without a bomb block is star blocks. NiDL's
         * 0x34 cells touching a breakable block are blocks a bomb sets off too (Ice Cream Island's sand path under its bomb block in stage
         * 2, Rainbow Resort's), not one-way clouds (Grape Garden's boss tower: those touch none). NiDL's 0x35 cells touching a chain (and
         * the 0x34 cells among them) are its invisible links: a bomb's chain goes on through them (Rainbow Resort 3-4's, 0x2FD: one bomb
         * block sets off its ceiling and its pits' floors through columns of them; 3-7's, 0x304: the bomb at the bottom opens the goal
         * door's at the top); they're KatAM's own, 0xF3 (not solid, its chain attribute 0x40: code_0800E0E4.c breaks a chain's
         * neighbours that have it), linkCells; */
        static Dictionary<int, HashSet<int>> linkCells = new Dictionary<int, HashSet<int>>();
        static byte[] RoomCells(byte[] nightmare, int record) {
            int width = ReadShort(nightmare, record + 0x14), height = ReadShort(nightmare, record + 0x16);
            byte[] cells = DecompressLZ77(nightmare, ReadInt(nightmare, record + 8) - 0x08000000);
            if (cells.Length < width * height * 4) return cells;
            int Kind(int i) => cells[i * 4 + 3];
            HashSet<int> links = new HashSet<int>();
            bool Chain(int i) => Kind(i) is 0x2D or 0x2E || links.Contains(i);
            IEnumerable<int> Around(int i) {
                int x = i % width, y = i / width;
                if (x > 0) yield return i - 1;
                if (x + 1 < width) yield return i + 1;
                if (y > 0) yield return i - width;
                if (y + 1 < height) yield return i + width;
            }
            List<int> Flood(int from, Func<int, bool> through, int[] marks, int mark) {
                List<int> found = new List<int> { from };
                marks[from] = mark;
                for (int k = 0; k < found.Count; k++)
                    foreach (int n in Around(found[k]))
                        if (marks[n] < 0 && through(n)) { marks[n] = mark; found.Add(n); }
                return found;
            }

            // 0x34 groups touching (even corner to corner) a breakable block: blocks a bomb sets off (0x2D);
            int[] clouds = Enumerable.Repeat(-1, width * height).ToArray();
            for (int i = 0, groups = 0; i < width * height; i++) {
                if (clouds[i] >= 0 || Kind(i) != 0x34) continue;
                List<int> group = Flood(i, n => Kind(n) == 0x34, clouds, groups++);
                bool Breakable(int n) => n >= 0 && n < width * height && Kind(n) >> 4 == 2;
                bool touches = group.Any(g => new[] { -1, 0, 1 }.Any(dy => new[] { -1, 0, 1 }.Any(dx =>
                    g % width + dx >= 0 && g % width + dx < width && Breakable(g + dy * width + dx))));
                if (touches) foreach (int g in group) cells[g * 4 + 3] = 0x2D;
            }

            // 0x35 groups (with the 0x34 cells in them: Rainbow Resort 3-7's link column, 0x304) touching a chain: its links (linkCells);
            int[] linkGroups = Enumerable.Repeat(-1, width * height).ToArray();
            for (int i = 0, groups = 0; i < width * height; i++) {
                if (linkGroups[i] >= 0 || Kind(i) != 0x35) continue;
                List<int> group = Flood(i, n => Kind(n) is 0x35 or 0x34, linkGroups, groups++);
                if (group.Any(g => Around(g).Any(n => Kind(n) is 0x2D or 0x2E))) links.UnionWith(group);
            }
            linkCells[record] = links;

            // (Heavy Mole's dirt: star blocks);
            if (Enumerable.Range(0, width * height).Count(i => Kind(i) == 0x2F) > MetalBlocks)
                for (int i = 0; i < width * height; i++) if (Kind(i) == 0x2F) cells[i * 4 + 3] = 0x24;

            // (blocks a door lands Kirby in, PlanDoors()'s clearedBlocks: what breaking them leaves, air; before the chains: one whose bomb
            // block starts broken so is star blocks, 0x27C's);
            if (clearedBlocks.TryGetValue(record, out HashSet<int> cleared))
                foreach (int i in cleared) {
                    WriteShort(cells, i * 4, LeftMetatile(nightmare, record, i));
                    cells[i * 4 + 3] = 0;
                }
            int[] chain = Enumerable.Repeat(-1, width * height).ToArray();
            for (int i = 0, chains = 0; i < width * height; i++) {
                if (chain[i] >= 0 || !Chain(i)) continue;
                List<int> blocks = Flood(i, Chain, chain, chains++);
                if (blocks.Any(b => Kind(b) == 0x2E)) continue;
                // No bomb block sets them off (Rainbow Resort 3-7's): blocks any attack breaks, as star blocks (links: nothing);
                foreach (int block in blocks) {
                    if (links.Remove(block)) continue;
                    cells[block * 4 + 3] = 0x24;
                }
            }
            return cells;
        }

        /* clearedBlocks; Per NiDL room (record), its breakable blocks a door lands Kirby inside (cells): KatAM lands him on a door, and one
         * hidden in blocks (NiDL's secret doors, their door back landing on them: Orange Ocean 5-3's chamber of bomb blocks, 0x294) put
         * him in the ground, falling through it forever; those blocks (the group, 4 sides) are left broken (PlanDoors()); */
        static Dictionary<int, HashSet<int>> clearedBlocks = new Dictionary<int, HashSet<int>>();

        // LeftMetatile(); The metatile a breakable block leaves (its number's in the room's object data: u32 each, up to 0xFFFFFFFF);
        static int LeftMetatile(byte[] nightmare, int record, int cell) {
            int data = ReadInt(nightmare, record + 0x10) - 0x08000000;
            int number = ReadShort(DecompressLZ77(nightmare, ReadInt(nightmare, record + 0xC) - 0x08000000), cell * 2);
            for (int n = 0; data >= 0 && n <= number; n++) {
                int value = ReadInt(nightmare, data + n * 4);
                if (value == -1) return 0;
                if (n == number) return value & 0xFFFF;
            }
            return 0;
        }

        const int MetalBlocks = 64;

        const byte LinkCollision = 0xF3;   // KatAM's invisible chain link (gCollisionAttributes 0x41: not solid, a chain goes through);
        static byte BreakableBlock(int collision) => collision switch { 0x2D => 0xF2, 0x2E => 0xF1, 0x2F => 0xF5, _ => 0xF0 };

        /* BuildSolidity(); The room's (part's) SolidityMap: its RL collision and its special blocks: per breakable block, its KatAM
         * value, where it is, and what it leaves when broken (the 4 tilemap entries of the metatile NiDL's object data lists for the
         * block's number, nothing or water); then its doors (see Door); returns its address; */
        static int BuildSolidity(byte[] rom, byte[] nightmare, NightmareRoom r, byte[] cells, byte[] collision, byte[] metatiles,
                                 List<Door> doors, List<(int x, int y, bool door)> mirras, int noSpecials, ref int cursor) {
            int roomWidth = ReadShort(nightmare, r.Record + 0x14), data = ReadInt(nightmare, r.Record + 0x10) - 0x08000000;
            byte[] numbers = DecompressLZ77(nightmare, ReadInt(nightmare, r.Record + 0xC) - 0x08000000);
            List<byte> specials = new List<byte>();
            Dictionary<(int x, int y), int> leaves = new Dictionary<(int, int), int>();
            bool Water(int x, int y) => x >= 0 && y >= 0 && x < r.FullWidth && y < r.FullHeight && collision[y * r.FullWidth + x] is >= 0x34 and <= 0x48;

            for (int y = 0; y < r.Height && y < 256; y++) {
                for (int x = 0; x < r.Width && x < 256; x++) {
                    int cell = (r.Y + y) * roomWidth + r.X + x, kind = cells[cell * 4 + 3];
                    if (kind >> 4 != 2) continue;

                    int number = ReadShort(numbers, cell * 2), metatile = 0, leftKind = 0;
                    // The numbers' cells: u32 each from the object data (metatile, attribute, collision), up to 0xFFFFFFFF;
                    for (int n = 0; data >= 0 && n <= number; n++) {
                        int value = ReadInt(nightmare, data + n * 4);
                        if (value == -1) { metatile = 0; break; }
                        if (n == number) { metatile = value & 0xFFFF; leftKind = value >> 24 & 0xFF; }
                    }
                    // (what it leaves is that cell, its collision too: air (water by water), or NiDL's wall, floor, ladder, spikes, water,
                    // or the block again, solid, which no other bomb breaks (room 0x248's frames of blocks: their look stayed, their
                    // collision went);
                    byte left = leftKind != 0 ? RoomCollision[leftKind] : (byte)(Water(x - 1, y) || Water(x + 1, y) || Water(x, y - 1) || Water(x, y + 1) ? 0x48 : 0);
                    byte block = BreakableBlock(kind);
                    collision[y * r.FullWidth + x] = block;
                    specials.AddRange(new byte[] { 1, block, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                    for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                    leaves[(x, y)] = specials.Count;
                    specials.AddRange(new byte[] { left, 0xFF });
                }
            }

            /* NiDL's chain links (RoomCells()'s linkCells): KatAM's 0xF3, with a special block record a chain applies (nothing, or water:
             * the cell looks as it did, its own metatile); */
            if (linkCells.TryGetValue(r.Record, out HashSet<int> links))
                for (int y = 0; y < r.Height && y < 256; y++) {
                    for (int x = 0; x < r.Width && x < 256; x++) {
                        int cell = (r.Y + y) * roomWidth + r.X + x;
                        if (!links.Contains(cell)) continue;
                        byte left = (byte)(Water(x, y) ? 0x48 : 0);
                        collision[y * r.FullWidth + x] = LinkCollision;
                        specials.AddRange(new byte[] { 1, LinkCollision, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                        for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, ReadShort(cells, cell * 4) * 8 + q * 2))));
                        specials.AddRange(new byte[] { left, 0xFF });
                    }
                }

            /* A fuse's cells: two special block records each, as KatAM's (cannon_fuse.c applies the first as it burns, the second when
             * it's lit again): the wall without the rope (the metatile of the nearest NiDL wall, kind 0x01), then the cell's own; */
            for (int y = 0; y < r.Height && y < 256; y++) {
                for (int x = 0; x < r.Width && x < 256; x++) {
                    byte value = collision[y * r.FullWidth + x];
                    if (value is < 0x22 or > 0x27) continue;
                    int own = ReadShort(cells, ((r.Y + y) * roomWidth + r.X + x) * 4), wall = own;
                    for (int d = 1, best = int.MaxValue; d <= 6 && best == int.MaxValue; d++)
                        for (int wy = y - d; wy <= y + d; wy++)
                            for (int wx = x - d; wx <= x + d; wx++)
                                if (wx >= 0 && wy >= 0 && wx < r.Width && wy < r.Height && cells[((r.Y + wy) * roomWidth + r.X + wx) * 4 + 3] == 0x01 && Math.Abs(wx - x) + Math.Abs(wy - y) < best) {
                                    best = Math.Abs(wx - x) + Math.Abs(wy - y);
                                    wall = ReadShort(cells, ((r.Y + wy) * roomWidth + r.X + wx) * 4);
                                }
                    foreach (int metatile in new[] { wall, own }) {
                        specials.AddRange(new byte[] { 1, value, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                        for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                        specials.AddRange(new byte[] { value, 0xFF });
                    }
                }
            }

            /* Doors: their cells and 8ROM records. A door behind a breakable block (NiDL's secret ones: the block leaves the door's
             * metatile) stays the block, which leaves the door's collision, its record first (a later record of a cell is what the cell
             * is, as for the hidden doors below): its mirror (subtype 1, KatAM's) shows once it's a door; */
            // (a door of a mini-boss's fight, ArenaCells(), is built shut, air or water, its record alone: the arena opens it once the
            // mini-boss is beaten, "ROM Expansion/enemies/arena.c"; it showed in a long room before the arena came on screen);
            HashSet<(int x, int y)> fightCells = ArenaCells(Arenas(nightmare, r), collision, r.FullWidth, r.Width, r.Height);
            List<byte> secrets = new List<byte>();
            for (int i = 0; i < doors.Count; i++) {
                Door door = doors[i];
                bool water = Water(door.X, door.Y) && collision[door.Y * r.FullWidth + door.X] != 0;
                byte value = door.Edge ? (byte)(water ? 0xFC : 0xFE) : (byte)(water ? 0xFD : 0xFF);
                if (door.Slot == 0 && !door.Edge && leaves.TryGetValue((door.X, door.Y), out int leave)) {
                    specials[leave] = value;
                    doors[i] = door with { Secret = true };
                    secrets.AddRange(DoorRecord(door, value));
                    continue;
                } else if (door.Slot == 0 && mirras.Contains((door.X, door.Y, true))) {
                    // (a door a Mirra hides: its cell stays as it is until the Mirra breaks, its record making it the door, and its
                    // mirror shows then, as the hidden doors below);
                    int own = ReadShort(cells, ((r.Y + door.Y) * roomWidth + r.X + door.X) * 4);
                    specials.AddRange(DoorRecord(door, value));
                    specials.AddRange(new byte[] { 1, collision[door.Y * r.FullWidth + door.X], (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                    for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, own * 8 + q * 2))));
                    specials.AddRange(new byte[] { value, 0xFF });
                    doors[i] = door with { Secret = true };
                    continue;
                } else if (door.Slot == 0 && !door.Edge && fightCells.Contains((door.X, door.Y))) {
                    collision[door.Y * r.FullWidth + door.X] = (byte)(water ? 0x48 : 0);
                    doors[i] = door with { Shut = true };
                } else if (door.Slot == 0) {
                    collision[door.Y * r.FullWidth + door.X] = value;
                }
                specials.AddRange(DoorRecord(door, value));
                if (door.Slot == 0) continue;

                /* A hidden door (a switch room's, lit by its big switch): its door object (0x97, as KatAM's switch rooms') breaks the
                 * special block of its cell once the switch's slot is set: the cell's own tiles, and the door's collision; */
                int metatile = ReadShort(cells, ((r.Y + door.Y) * roomWidth + r.X + door.X) * 4);
                specials.AddRange(new byte[] { 1, collision[door.Y * r.FullWidth + door.X], (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                specials.AddRange(new byte[] { value, 0xFF });
            }

            specials.InsertRange(0, secrets);
            byte[] compressed = CompressRL(collision);
            int collisionAt = Allocate(ref cursor, compressed.Length, 4);
            compressed.CopyTo(rom, collisionAt);
            int specialsAt = noSpecials;
            if (specials.Count > 0) {
                specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                specialsAt = Allocate(ref cursor, specials.Count, 4);
                specials.ToArray().CopyTo(rom, specialsAt);
            }
            int at = Allocate(ref cursor, 8, 4);
            WriteInts(rom, at, collisionAt + 0x08000000, specialsAt + 0x08000000);
            return at;
        }

        /* MirrorTwin(); Central Circle's Rainbow Route mirror (tiles 37-40, rows 24-29 of its tilemap: door 19, 14) mirrored onto its
         * platform's left end (door 11, 14) of a Central Circle tilemap (map, columns wide), flipped and pink (EntryMirrorColor()): new
         * tiles (in the ones neither the map nor otherEntries use), each in the palette (firstPalette to 5) that shows it best, the pinks
         * in the colors no tile uses nor the palette cycles change (cycled); */
        static void MirrorTwin(byte[] map, int columns, byte[] tiles, int[] colors, IEnumerable<int> otherEntries, IEnumerable<int> cycled, int firstPalette) {
            List<int> entries = Enumerable.Range(0, map.Length / 2).Select(i => ReadShort(map, i * 2)).Concat(otherEntries).ToList();
            HashSet<int> used = entries.Select(e => e & 0x3FF).ToHashSet();
            HashSet<int> usedColors = entries.SelectMany(e => Enumerable.Range(0, 64).Select(i => tiles[(e & 0x3FF) * 32 + i / 2] >> (i % 2 * 4) & 0xF)
                                                             .Where(index => index != 0).Select(index => (e >> 12) * 16 + index)).ToHashSet();
            usedColors.UnionWith(cycled);
            // The pinks, in each palette's free colors;
            List<int> pinks = new List<int>();
            for (int y = 24; y <= 29; y++)
                for (int x = 37; x <= 40; x++) {
                    int entry = ReadShort(map, (y * columns + x) * 2);
                    for (int i = 0; entry != 0 && i < 64; i++) {
                        int index = tiles[(entry & 0x3FF) * 32 + i / 2] >> (i % 2 * 4) & 0xF, color = colors[(entry >> 12) * 16 + index];
                        if (index != 0 && EntryMirrorColor(color) != color) pinks.Add(EntryMirrorColor(color));
                    }
                }
            for (int palette = firstPalette; palette < 6 && pinks.Count > 0; palette++) {
                int[] slots = Enumerable.Range(palette * 16 + 1, 15).Where(c => !usedColors.Contains(c)).ToArray();
                if (slots.Length == 0) continue;
                int[] shades = Quantize(pinks, slots.Length);
                for (int i = 0; i < slots.Length && i < shades.Length; i++) colors[slots[i]] = shades[i];
            }
            Queue<int> freeTiles = new Queue<int>(Enumerable.Range(1, tiles.Length / 32 - 1).Where(t => !used.Contains(t)));
            Dictionary<int, int> recolored = new Dictionary<int, int>();
            for (int y = 24; y <= 29; y++) {
                for (int x = 37; x <= 40; x++) {
                    int entry = ReadShort(map, (y * columns + x) * 2);
                    if (entry == 0) continue;
                    if (!recolored.TryGetValue(entry & 0xF3FF, out int made)) {
                        int tile = entry & 0x3FF, palette = entry >> 12;
                        int[] wanted = Enumerable.Range(0, 64).Select(i => tiles[tile * 32 + i / 2] >> (i % 2 * 4) & 0xF)
                                                 .Select(index => index == 0 ? -1 : EntryMirrorColor(colors[palette * 16 + index])).ToArray();
                        int Nearest(int p, int color) => 1 + NearestColor(colors.Skip(p * 16 + 1).Take(15).ToArray(), color);
                        double Error(int p) => wanted.Where(c => c >= 0).Sum(c => Distance(Rgb(colors[p * 16 + Nearest(p, c)]), Rgb(c)));
                        int best = Enumerable.Range(firstPalette, 6 - firstPalette).MinBy(Error);
                        made = entry & 0xF3FF;
                        bool changed = Enumerable.Range(0, 64).Any(i => wanted[i] >= 0 && wanted[i] != colors[palette * 16 + (tiles[tile * 32 + i / 2] >> (i % 2 * 4) & 0xF)]);
                        if (changed && freeTiles.Count > 0) {
                            int newTile = freeTiles.Dequeue();
                            for (int i = 0; i < 64; i += 2)
                                tiles[newTile * 32 + i / 2] = (byte)((wanted[i] < 0 ? 0 : Nearest(best, wanted[i])) | (wanted[i + 1] < 0 ? 0 : Nearest(best, wanted[i + 1])) << 4);
                            made = best << 12 | newTile;
                        }
                        recolored[entry & 0xF3FF] = made;
                    }
                    WriteShort(map, (y * columns + columns - 1 - x) * 2, (made | (entry & 0xC00)) ^ 0x400);
                }
            }
        }

        // CycledColors(); The colors a ForegroundPalette's cycles change (ForegroundPalette_4pp: unk4[1] the first color, unk8 how many);
        static IEnumerable<int> CycledColors(byte[] rom, int paletteEntry) {
            int cycles = ReadInt(rom, paletteEntry + 4) - 0x08000000;
            for (int i = 0; cycles >= 0 && i < ReadShort(rom, paletteEntry + 8); i++) {
                int cycle = ReadInt(rom, cycles + i * 4) - 0x08000000;
                for (int c = 0; c < ReadShort(rom, cycle + 8); c++) yield return ReadShort(rom, cycle + 6) + c;
            }
        }

        // EntryMirrorColor(); A color of Central Circle's golden mirror, for its NiDL one: gold, orange and brown to pink;
        static int EntryMirrorColor(int color) {
            (double h, double s, double v) = ToHsv(color);
            if (s <= 0.25 || h >= 72 && h <= 324) return color;
            return FromHsv((288 + (h < 72 ? h : h - 360) * 0.6 + 360) % 360, s, v);
        }

        /* TileAnimations(); A room's animated foreground tiles (water, waterfalls, lights...) as KatAM's (ForegroundInfo +4, +8, +0xC):
         * NiDL's room record +0x40 picks a set (0: none) of RoomTable + 0x1C8 (pointers per set, the same in every region: null
         * terminated lists of scripts, at most 10). A script: 8 bytes per step, u16 the command, u16 its parameter, u32 its argument:
         * 0 shows a frame (for the parameter's frames), 3 loops, 4 stops; the others (1: palette animations, 2, 6) aren't ported, a
         * script without frames is left out; a frame: u16 the first foreground tile it's copied to, u16 its bytes, then the raw tiles.
         * KatAM's (ForegroundInfo_8p, 0x14 bytes): the VRAM offset from 0x06008000 (KatAM's foreground tiles: NiDL's, same numbers),
         * bytes per copy, stride, its steps (u32 source, +4 its base: 0 here, u16 frames; 8 bytes each), how many, copies per step (1);
         * KatAM runs 16 at most. The frames are copied once (frameCopies); returns the list and its count; */
        static (int animations, int count) TileAnimations(byte[] rom, byte[] nightmare, int roomTable, int set, Dictionary<int, int> frameCopies, ref int cursor) {
            if (set == 0) return (0, 0);
            int list = ReadInt(nightmare, roomTable + 0x1C8 + set * 4) - 0x08000000;
            if (list < 0 || list >= nightmare.Length) return (0, 0);
            List<List<(int frame, int duration)>> scripts = new List<List<(int, int)>>();
            for (int i = 0; i < 16 && (ReadInt(nightmare, list + i * 4) >> 24) == 8; i++) {
                List<(int frame, int duration)> script = new List<(int, int)>();
                for (int at = ReadInt(nightmare, list + i * 4) - 0x08000000, n = 0; n < 64; at += 8, n++) {
                    int command = ReadShort(nightmare, at), frame = ReadInt(nightmare, at + 4) - 0x08000000;
                    if (command is 3 or 4) break;
                    if (command == 0 && frame >= 0 && frame < nightmare.Length) script.Add((frame, Math.Max(1, ReadShort(nightmare, at + 2))));
                }
                if (script.Count > 0) scripts.Add(script);
            }
            if (scripts.Count == 0) return (0, 0);

            int animations = Allocate(ref cursor, scripts.Count * 0x14, 4);
            for (int k = 0; k < scripts.Count; k++) {
                List<(int frame, int duration)> steps = scripts[k];
                int tile = ReadShort(nightmare, steps[0].frame), bytes = ReadShort(nightmare, steps[0].frame + 2);
                int stepsAt = Allocate(ref cursor, steps.Count * 8, 4);
                for (int i = 0; i < steps.Count; i++) {
                    if (!frameCopies.TryGetValue(steps[i].frame, out int copy)) {
                        copy = frameCopies[steps[i].frame] = Allocate(ref cursor, bytes, 4);
                        Array.Copy(nightmare, steps[i].frame + 4, rom, copy, bytes);
                    }
                    WriteInt(rom, stepsAt + i * 8, copy + 0x08000000);
                    WriteShorts(rom, stepsAt + i * 8 + 4, steps[i].duration, 0);
                }
                int animation = animations + k * 0x14;
                WriteInts(rom, animation, tile * 32, bytes, bytes, stepsAt + 0x08000000);
                WriteShorts(rom, animation + 0x10, steps.Count, 1);
            }
            return (animations + 0x08000000, scripts.Count);
        }

        // ShiftPalette(); A NiDL tilemap entry with its palette moved down by 2 (NiDL's level palettes start at 2, KatAM's at 0);
        static int ShiftPalette(int entry) => (entry & 0xFFF) | (Math.Max((entry >> 12) - 2, 0) << 12);

        // CopyLZ77(); NiDL's LZ77 data as it is (it's VRAM safe: NiDL decompresses it to VRAM too);
        static int CopyLZ77(byte[] rom, byte[] nightmare, int address, ref int cursor) {
            int length = CompressedLengthLZ77(nightmare, address);
            int at = Allocate(ref cursor, length, 4);
            Array.Copy(nightmare, address, rom, at, length);
            return at + 0x08000000;
        }

        static int CompressedLengthLZ77(byte[] source, int address) {
            int size = ReadInt(source, address) >> 8, position = address + 4;

            for (int written = 0; written < size;) {
                byte flags = source[position++];
                for (int bit = 0; bit < 8 && written < size; bit++) {
                    if ((flags & (0x80 >> bit)) == 0) { position++; written++; continue; }
                    written += (source[position] >> 4) + 3;
                    position += 2;
                }
            }
            return position - address;
        }

        /* BackgroundRates(); A room's background scroll rates (props unk0E, unk10, 8.8), so it pans across the whole background once,
         * evenly, as the camera crosses the room (NiDL's own rates are for its camera; past the background's edge KatAM repeats it):
         * KatAM (sub_080019F8) draws it at the camera's x times the rate from its left, and bottom-aligned at the camera's distance from
         * the room's bottom times the rate; its camera shows x from 8 to the width - 8, y from 8 to the height - 40 (it moves the room's
         * width - 256, height - 208, or from its top, CameraTop()); a room the camera hardly moves in (less than StillRange: one that fits it, a screen and a bit)
         * keeps its background still (a background no bigger than the screen too), and none moves faster than the room (0x100); */
        const int StillRange = 48;
        static (int x, int y) BackgroundRates(int backgroundWidth, int backgroundHeight, int roomWidth, int roomHeight, int top = 8) {
            int Rate(int background, int screen, int range) => range < StillRange || background <= screen ? 0 : Math.Min((background - screen) * 0x100 / range, 0x100);
            return (Rate(backgroundWidth, 240, roomWidth - 256), Rate(backgroundHeight, 160, roomHeight - 200 - top));
        }

        /* CameraTop(); Where the camera's view starts at a ported room's top (pixels; "ROM Expansion/rooms.c", gNidlRoomTops): KatAM's
         * camera hides 8 pixels, cutting half of the top row NiDL shows (Vegetable Valley 3-4's trees, 0x9E); a room whose top row
         * repeats the one under it (4 cells or more, besides the row's sky: NiDL's filler, which it never shows) starts a row down,
         * the others at the top; a split room's lower parts as KatAM's; never lower than the camera can go (a screen's room: 8); */
        static int CameraTop(byte[] nightmare, NightmareRoom r) {
            if (r.Y > 0 || r.Height < 2) return 8;
            byte[] cells = DecompressLZ77(nightmare, ReadInt(nightmare, r.Record + 8) - 0x08000000);
            int roomWidth = ReadShort(nightmare, r.Record + 0x14);
            int Metatile(int x, int y) => ReadShort(cells, ((r.Y + y) * roomWidth + r.X + x) * 4);
            int sky = Enumerable.Range(0, r.Width).GroupBy(x => Metatile(x, 1)).OrderByDescending(g => g.Count()).First().Key;
            bool repeated = Enumerable.Range(0, r.Width).Count(x => Metatile(x, 0) == Metatile(x, 1) && Metatile(x, 1) != sky) >= 4;
            return repeated ? Math.Min(16, r.FullHeight * 16 - 200) : 0;
        }

        /* BuildBackground(); A KatAM RoomTiledBG for a NiDL background: only the tiles its map uses (renumbered, flipped copies shared,
         * at most BackgroundTiles), palettes 8-15 as KatAM's 6-13, the map row by row; returns its address; */
        static int BuildBackground(byte[] rom, byte[] nightmare, int tilesAddress, int paletteAddress, int map, int room, ref int cursor) {
            byte[] tiles = DecompressLZ77(nightmare, tilesAddress);
            int width = ReadShort(nightmare, map + 2), height = ReadShort(nightmare, map + 4);
            bool compressed = ReadShort(nightmare, map + 6) == 0xFFFF;
            byte[] source = compressed ? DecompressLZ77(nightmare, map + 8) : null;

            // The map's entries, and its tiles' pixels (64 each, unflipped);
            int[] mapEntries = new int[width * height];
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    // (in GBA screen blocks: 32x32 each, left to right, then top to bottom);
                    mapEntries[y * width + x] = compressed ? ReadShort(source, (((y >> 5) * ((width + 31) >> 5) + (x >> 5)) * 1024 + (y & 31) * 32 + (x & 31)) * 2)
                                                           : ReadShort(nightmare, map + 6 + (y * width + x) * 2);
            byte[] Pixels(int tile, int flip) {
                byte[] pixels = new byte[64];
                for (int i = 0; i < 64; i++) {
                    int x = (flip & 1) != 0 ? 7 - i % 8 : i % 8, y = (flip & 2) != 0 ? 7 - i / 8 : i / 8;
                    pixels[i] = tile * 32 + 32 <= tiles.Length ? (byte)(tiles[tile * 32 + y * 4 + x / 2] >> (x % 2 * 4) & 0xF) : (byte)0;
                }
                return pixels;
            }

            /* Tiles the same but flipped share one (KatAM's tilemap flips them); past KatAM's limit (BackgroundTiles: one of NiDL's
             * backgrounds), the least used share the most alike of those kept; */
            Dictionary<string, (int tile, int flip)> kept = new Dictionary<string, (int, int)>();
            Dictionary<int, (int tile, int flip)> same = new Dictionary<int, (int, int)>();
            Dictionary<int, int> uses = mapEntries.GroupBy(e => e & 0x3FF).ToDictionary(g => g.Key, g => g.Count());
            foreach (int tile in uses.Keys.OrderByDescending(t => uses[t])) {
                (int, int)? match = null;
                for (int flip = 0; flip < 4 && match == null; flip++)
                    if (kept.TryGetValue(Convert.ToBase64String(Pixels(tile, flip)), out var k)) match = (k.tile, flip);
                if (match == null) kept[Convert.ToBase64String(Pixels(tile, 0))] = (tile, 0);
                same[tile] = match ?? (tile, 0);
            }
            List<int> distinct = same.Where(t => t.Value.tile == t.Key).Select(t => t.Key).ToList();     // Most used first;
            List<int> chosen = distinct.Take(BackgroundTiles).ToList();
            foreach (int tile in distinct.Skip(BackgroundTiles)) {
                byte[] pixels = Pixels(tile, 0);
                (int tile, int flip) best = (chosen[0], 0);
                int bestDistance = int.MaxValue;
                foreach (int candidate in chosen)
                    for (int flip = 0; flip < 4; flip++) {
                        byte[] other = Pixels(candidate, flip);
                        int distance = 0;
                        for (int i = 0; i < 64 && distance < bestDistance; i++) distance += Math.Abs(pixels[i] - other[i]);
                        if (distance < bestDistance) { bestDistance = distance; best = (candidate, flip); }
                    }
                foreach (int t in same.Keys.Where(t => same[t].tile == tile).ToList()) same[t] = (best.tile, best.flip ^ same[t].flip);
            }
            Dictionary<int, int> number = chosen.Select((tile, n) => (tile, n)).ToDictionary(t => t.tile, t => t.n);
            List<byte> usedTiles = new List<byte>();
            foreach (int tile in chosen) usedTiles.AddRange(tile * 32 + 32 <= tiles.Length ? tiles.Skip(tile * 32).Take(32) : new byte[32]);
            byte[] entries = new byte[width * height * 2];
            for (int i = 0; i < mapEntries.Length; i++) {
                (int tile, int flip) = same[mapEntries[i] & 0x3FF];
                WriteShort(entries, i * 2, ShiftPalette((mapEntries[i] & 0xF000) | (((mapEntries[i] >> 10 & 3) ^ flip) << 10) | number[tile]));
            }

            byte[] compressedTiles = CompressLZ77(usedTiles.ToArray());
            int tilesAt = Allocate(ref cursor, compressedTiles.Length, 4);
            compressedTiles.CopyTo(rom, tilesAt);
            int colors = BackgroundPalette(rom, nightmare, paletteAddress, ref cursor);
            int mapAt = Allocate(ref cursor, entries.Length, 4);
            entries.CopyTo(rom, mapAt);

            int at = Allocate(ref cursor, 0x20, 4);
            WriteShorts(rom, at, width, height);
            WriteInts(rom, at + 8, tilesAt + 0x08000000, usedTiles.Count / 32, colors + 0x08000000);
            WriteShorts(rom, at + 0x14, 0x60, 0x80);
            WriteInts(rom, at + 0x18, mapAt + 0x08000000, room << 16);
            return at;
        }

        /* BackgroundPalette(); A NiDL background's colours as KatAM's palettes 6-13 (NiDL's 8-15): a shorter palette ends at palette 15,
         * as NiDL loads it (Grape Garden 2's sky, 0x1B9-0x1BE: 7 palettes, its map using 9-15); returns its address; */
        static int BackgroundPalette(byte[] rom, byte[] nightmare, int paletteAddress, ref int cursor) {
            int colors = Allocate(ref cursor, 0x100, 4), length = Math.Min(0x100, (int)ReadShort(nightmare, paletteAddress));
            Array.Copy(nightmare, paletteAddress + 2, rom, colors + 0x100 - length, length);
            return colors;
        }

        // BackgroundColours(); A NiDL background palette's colours, to tell rooms sharing a background apart by them;
        static string BackgroundColours(byte[] nightmare, int paletteAddress) =>
            Convert.ToBase64String(nightmare, paletteAddress + 2, Math.Min(0x100, (int)ReadShort(nightmare, paletteAddress)));

        /* RecolourBackground(); A copy of a built background's RoomTiledBG (BuildBackground()) with other colours, its tiles and map
         * shared, its room's; returns its address; */
        static int RecolourBackground(byte[] rom, int built, byte[] nightmare, int paletteAddress, int room, ref int cursor) {
            int at = Allocate(ref cursor, 0x20, 4);
            Array.Copy(rom, built, rom, at, 0x20);
            WriteInt(rom, at + 0x10, BackgroundPalette(rom, nightmare, paletteAddress, ref cursor) + 0x08000000);
            WriteInt(rom, at + 0x1C, room << 16);
            return at;
        }

        /* MirraSpots(); Where a room's Mirras stand (object 0x34, KatAM's mirror enemy: it breaks at a touch, and KatAM then applies the
         * special block record of its cell, gUnk_08D...: a cell a door is hidden in, its mirror showing then, as KatAM's rooms 0x75,
         * 0x8D, 0xAF... hide theirs; it isn't remembered, so it's back each visit): as few as KatAM's (9), one on each switch room's
         * hidden way in (DoorKind.SwitchEntry, one-way, one a room: see PlanDoors()), on an open cell over a floor; */
        static List<(int x, int y, bool door)> MirraSpots(NightmareRoom r, List<Door> doors, byte[] collision) {
            byte At(int x, int y) => x >= 0 && y >= 0 && x < r.Width && y < r.Height ? collision[y * r.FullWidth + x] : (byte)0x0D;
            return doors.Where(d => d.Kind == DoorKind.SwitchEntry && d.Hidden && !d.Edge && d.Slot == 0 && At(d.X, d.Y) == 0 && At(d.X, d.Y + 1) is >= 0x01 and <= 0x1B)
                        .Select(d => (d.X, d.Y, true)).Take(1).ToList();
        }

        /* NidlChest; A consumable chest in a ported room (PlanChests()): where it stands (room pixels, the middle of a cell with a floor
         * under it, as KatAM's), what it holds (KatAM's chest items: 0 1UP, 1 Invincible Candy, 2 Maximum Tomato, 3 Meat, 4 Pep Brew,
         * 5 Cherry), its treasure index (from FirstNidlChest), and the NiDL item it replaces (its record in the NiDL ROM; -1 a new one); or
         * a battery (KatAM's 0x62, the phone's) standing there, new; */
        public sealed record NidlChest(int X, int Y, byte Item, byte Index, int Replaces, bool Battery = false);
        static Dictionary<NightmareRoom, List<NidlChest>> chestsOf = new Dictionary<NightmareRoom, List<NidlChest>>();
        public static IReadOnlyDictionary<NightmareRoom, List<NidlChest>> PortedChests => chestsOf;
        /* The treasure indices of the NiDL chests: KatAM's 84 chests use 0-83 (gTreasures.chestFields has 128 bits; 0x80 on are the shard
         * chests', KatAMItems.ShardChestIndex); the game's completion counts only KatAM's (rooms.c's NidlNumChestsCollected,
         * NIDL_CHESTS_FIRST); */
        public const int FirstNidlChest = 84, LastNidlChest = 127;
        const byte SmallChestType = 0x80, BatteryType = 0x62;
        /* The NiDL items' gone bits (rooms.c's NIDL_TAKEN_KIND: template unk2, unk3 the item's number, 256 of them): KatAM keeps its items'
         * in gUnk_02023368, whose 256 numbers its own items use, so taken once they stay gone until the game clears them (a game's start,
         * a game over...); NiDL's come back otherwise each time Kirby enters the room; */
        const byte TakenKind = 3;
        static int takenItems;
        public static int TakenItems => takenItems;
        // A NiDL item as a chest's item (KatAM's 1UP, Invincible Candy, Maximum Tomato, Meat, Pep Brew, Cherry);
        static readonly Dictionary<byte, byte> ChestItems = new Dictionary<byte, byte> { [0x63] = 0, [0x64] = 1, [0x61] = 2, [0x60] = 3, [0x5F] = 4, [0x5E] = 5 };
        // KatAM's 41 consumable chests' items (18 1UPs, 7 Pep Brews, 6 Meats, 5 Maximum Tomatoes, 4 Cherries, an Invincible Candy), dealt
        // in turn to the new chests;
        static readonly byte[] KatAMChestItems = Enumerable.Repeat((byte)0, 18).Concat(Enumerable.Repeat((byte)4, 7)).Concat(Enumerable.Repeat((byte)3, 6))
                                                           .Concat(Enumerable.Repeat((byte)2, 5)).Concat(Enumerable.Repeat((byte)5, 4)).Append((byte)1).ToArray();

        /* PlanChests(); The NiDL chests, made by the ROM Expansion whatever the randomizer does after: a few per level, as KatAM has (a
         * chest per 8 of its rooms, 2 to 5), one a room at most, where KatAM puts its chests: a reward for exploring. Each level's spots
         * are ranked: a loose item behind breakable blocks (Kirby reaches it only by breaking some), a dead end (a room with one way in
         * and out), far from where Kirby comes in; a loose item there becomes a chest holding it, and a dead end with none gets a new chest,
         * as far into the room as Kirby goes. Never in a boss room, a switch room or the NiDL hub, nor in a mini-boss's fight area; a chest
         * stands on a floor (solid ground, ice or a one-way floor) Kirby reaches. The Fountain of Dreams is all boss rooms: none there. A
         * dead end with nothing in it (no enemy, item or boss) always gets a chest, past the level's few (0x298). Batteries too, as many
         * as the chests a level gets, at the best spots left (the farthest floor behind breakable blocks, else the farthest in a dead
         * end), one a room, none in a chest's; */
        static Dictionary<NightmareRoom, List<NidlChest>> PlanChests(byte[] nightmare, List<NightmareRoom> rooms, Dictionary<NightmareRoom, List<Door>> doors,
                                                                    List<Door> mainHubDoors, Dictionary<int, List<Door>> katamDoors) {
            const int FarCells = 12, DoorClear = 3, MostRecords = 20, EmptyRoom = 100;
            var plan = rooms.ToDictionary(r => r, r => new List<NidlChest>());
            var arrivals = doors.Values.SelectMany(list => list).Concat(mainHubDoors).Concat(katamDoors.Values.SelectMany(list => list)).Where(d => !d.Edge).ToList();
            int index = FirstNidlChest, dealt = 0;
            for (int level = 0; level < 8; level++) {
                var levelRooms = rooms.Where(r => r.Level == level && !r.IsSwitchRoom).ToList();
                if (levelRooms.Count == 0) continue;
                int bossStage = levelRooms.Max(o => o.Stage);
                var candidates = new List<(NightmareRoom room, int score, int distance, NidlChest chest)>();
                var batterySpots = new List<(NightmareRoom room, int score, int distance, NidlChest battery)>();
                foreach (NightmareRoom r in levelRooms.Where(o => o.Level != 7 && o.Stage != bossStage)) {
                    byte[] cells = RoomCells(nightmare, r.Record);
                    int roomWidth = ReadShort(nightmare, r.Record + 0x14);
                    int Kind(int cx, int cy) => cx >= 0 && cy >= 0 && cx < r.Width && cy < r.Height ? cells[((r.Y + cy) * roomWidth + r.X + cx) * 4 + 3] : 0x01;
                    bool Breakable(int cx, int cy) => Kind(cx, cy) >> 4 == 2;
                    bool Solid(int cx, int cy) => RoomCollision[Kind(cx, cy)] == 0x0D || Breakable(cx, cy);
                    bool Air(int cx, int cy) => cx >= 0 && cy >= 0 && cx < r.Width && cy < r.Height && RoomCollision[Kind(cx, cy)] == 0 && !Breakable(cx, cy);
                    bool Floor(int cx, int cy) => cy < r.Height && Kind(cx, cy) is 0x01 or 0x70 or 0x11;

                    // Where Kirby comes in (doors' landings, warp stars' and cannons'), and how far each place is from there: without
                    // breaking blocks, and breaking them (he flies: any cell not solid);
                    var entries = arrivals.Where(d => d.Room == r.Room).Select(d => (x: d.DestX - 1, y: d.DestY - 1))
                                          .Concat(transportsOf.Values.SelectMany(list => list).Where(t => t.Room == r.Room).Select(t => (x: t.DestX - 1, y: t.DestY - 1)))
                                          .Where(c => c.x >= 0 && c.y >= 0 && c.x < r.Width && c.y < r.Height).Distinct().ToList();
                    if (entries.Count == 0) continue;
                    Dictionary<(int, int), int> Reach(bool breaking) {
                        var distance = new Dictionary<(int, int), int>();
                        var queue = new Queue<(int x, int y)>();
                        foreach (var e in entries) { distance[e] = 0; queue.Enqueue(e); }
                        while (queue.Count > 0) {
                            var (x, y) = queue.Dequeue();
                            foreach (var (nx, ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) }) {
                                if (nx < 0 || ny < 0 || nx >= r.Width || ny >= r.Height || distance.ContainsKey((nx, ny))) continue;
                                if (Solid(nx, ny) && !(breaking && Breakable(nx, ny))) continue;
                                distance[(nx, ny)] = distance[(x, y)] + 1;
                                queue.Enqueue((nx, ny));
                            }
                        }
                        return distance;
                    }
                    var open = Reach(false);
                    var all = Reach(true);
                    var fights = Arenas(nightmare, r);
                    bool InFight(int cx, int cy) => fights.Any(a => cx * 16 + 8 >= a.left - 16 && cx * 16 + 8 < a.left + a.width + 16 && cy * 16 + 8 >= a.top - 16 && cy * 16 + 8 < a.top + a.height + 16);
                    var doorCells = doors[r].Where(d => !d.Edge).Select(d => (x: d.X, y: d.Y)).Concat(transportsOf[r].Select(t => (x: t.X / 16, y: (t.Y - 1) / 16))).Concat(entries).ToList();
                    bool NearDoor(int cx, int cy) => doorCells.Any(c => Math.Abs(c.x - cx) <= DoorClear && Math.Abs(c.y - cy) <= 2);
                    bool deadEnd = doors[r].Count + transportsOf[r].Count <= 1;
                    // (a chest stands on a floor, down to it from a spot, up to 6 cells, where the camera shows it, as a mirror: Shown(); and
                    // whole across: KatAM's camera stops 8 pixels short of a room's sides, so never in its first or last column);
                    int cameraWidth = Padded(r.Width, r.Height).width;
                    (int x, int y)? OnFloor(int cx, int cy) {
                        if (cx < 1 || cx > cameraWidth - 2) return null;
                        for (int k = 0; k <= 6 && Air(cx, cy + k); k++)
                            if (Floor(cx, cy + k + 1)) return Shown(r, cy + k) ? (cx, cy + k) : null;
                        return null;
                    }

                    // Its loose items: one at a reward spot becomes a chest holding it;
                    int list = ReadInt(nightmare, r.Record + 0x48), records = 0, things = 0;
                    for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8) {
                        int category = nightmare[at] - 1;
                        int x = ReadShort(nightmare, at + 4) - r.X * 16, y = ReadShort(nightmare, at + 6) - r.Y * 16;
                        if (x < 0 || y < 0 || x >= r.Width * 16 || y >= r.Height * 16) continue;
                        records++;
                        if (category is 0 or 2 or 3 or MinibossCategory or MetaKnightsCategory) things++;
                        if (category != 3 || !RoomItems.TryGetValue(nightmare[at + 1], out byte type)) continue;
                        var spot = OnFloor(x / 16, y / 16);
                        if (spot == null || !all.TryGetValue(spot.Value, out int far) || InFight(spot.Value.x, spot.Value.y)) continue;
                        bool secret = !open.ContainsKey(spot.Value);
                        int score = (secret ? 3 : 0) + (deadEnd ? 2 : 0) + (far >= FarCells ? 1 : 0);
                        if (score >= 2) candidates.Add((r, score, far, new NidlChest(spot.Value.x * 16 + 8, spot.Value.y * 16 + 8, ChestItems[type], 0, at)));
                    }

                    // (a battery's spots: the farthest floor behind breakable blocks, the farthest in a dead end; off the doors);
                    (int x, int y, int far)? Farthest(Func<(int, int), bool> where) =>
                        all.Where(c => where(c.Key) && OnFloor(c.Key.Item1, c.Key.Item2) == c.Key && !NearDoor(c.Key.Item1, c.Key.Item2) && !InFight(c.Key.Item1, c.Key.Item2))
                           .OrderByDescending(c => c.Value).ThenBy(c => c.Key.Item2).ThenBy(c => c.Key.Item1).Select(c => ((int x, int y, int far)?)(c.Key.Item1, c.Key.Item2, c.Value)).FirstOrDefault();
                    if (records <= MostRecords && Farthest(c => !open.ContainsKey(c)) is (int bx, int by, int bfar))
                        batterySpots.Add((r, 3 + (bfar >= FarCells ? 1 : 0), bfar, new NidlChest(bx * 16 + 8, by * 16 + 8, 0, 0, -1, Battery: true)));
                    else if (deadEnd && records <= MostRecords && Farthest(c => true) is (int dx, int dy, int dfar))
                        batterySpots.Add((r, 2 + (dfar >= FarCells ? 1 : 0), dfar, new NidlChest(dx * 16 + 8, dy * 16 + 8, 0, 0, -1, Battery: true)));

                    // A dead end with no item there: a new chest, as far into the room as Kirby goes (off the doors);
                    if (deadEnd && records <= MostRecords && !candidates.Any(c => c.room == r)) {
                        var best = all.Where(c => OnFloor(c.Key.Item1, c.Key.Item2) == c.Key && !NearDoor(c.Key.Item1, c.Key.Item2) && !InFight(c.Key.Item1, c.Key.Item2))
                                      .OrderByDescending(c => c.Value).ThenBy(c => c.Key.Item2).ThenBy(c => c.Key.Item1).Select(c => ((int x, int y, int far)?)(c.Key.Item1, c.Key.Item2, c.Value)).FirstOrDefault();
                        if (best != null) {
                            bool secret = !open.ContainsKey((best.Value.x, best.Value.y));
                            // (a dead end with nothing at all in it always gets one, past the level's few: 0x298, Orange Ocean 5-6);
                            int score = things == 0 ? EmptyRoom : 2 + (secret ? 1 : 0) + (best.Value.far >= FarCells ? 1 : 0);
                            candidates.Add((r, score, best.Value.far, new NidlChest(best.Value.x * 16 + 8, best.Value.y * 16 + 8, 0xFF, 0, -1)));
                        }
                    }
                }

                // The best spots, one a room;
                int wanted = Math.Clamp(levelRooms.Count / 8, 2, 5);
                foreach (var c in candidates.OrderByDescending(c => c.score).ThenByDescending(c => c.distance).ThenBy(c => c.room.Room)) {
                    if (index > LastNidlChest) break;
                    if (plan[c.room].Count > 0 || wanted == 0 && c.score < EmptyRoom) continue;
                    byte item = c.chest.Item != 0xFF ? c.chest.Item : KatAMChestItems[dealt++ * 17 % KatAMChestItems.Length];
                    plan[c.room].Add(c.chest with { Item = item, Index = (byte)index++ });
                    if (c.score < EmptyRoom) wanted--;
                }
                // Batteries: as many again, at the best spots left, one a room (none in a chest's);
                int batteries = Math.Clamp(levelRooms.Count / 8, 2, 5);
                foreach (var b in batterySpots.OrderByDescending(b => b.score).ThenByDescending(b => b.distance).ThenBy(b => b.room.Room)) {
                    if (batteries == 0) break;
                    if (plan[b.room].Count > 0) continue;
                    plan[b.room].Add(b.battery);
                    batteries--;
                }
            }
            return plan;
        }

        // ChestTemplate(); A small chest (0x80) as KatAM's: subtype1 its item, unk11 its treasure index;
        static byte[] ChestTemplate(NidlChest chest, int number) {
            byte[] template = ObjectTemplate(chest.X, chest.Y, number, SmallChestType);
            template[14] = chest.Item; template[15] = 0; template[0x11] = chest.Index;
            return template;
        }

        /* BuildObjects(); The room's (part's) enemies, mini-bosses and items as KatAM object templates (LevelInfo_1E4: templates, the
         * types in the room, an empty list, how many types), and its doors' mirrors; returns its address. No enemy stands within
         * LandingClear cells across (2 up or down) of where a door lands Kirby (landings): KatAM lands him on the door, NiDL beside it,
         * and one there hit him as he came in; */
        static int BuildObjects(byte[] rom, byte[] nightmare, NightmareRoom r, List<Door> doors, List<(int x, int y, bool door)> mirras,
                                List<(int x, int y)> landings, List<NidlChest> chests, int noTypes, ref int cursor) {
            const int LandingClear = 3, BomberType = 0x28;
            // KatAM's enemies that walk the floor (Waddle Dee, Noddy, Pengy, Sir Kibble, Waddle Doo, Sparky, Twister, Hot Head, Sword Knight);
            byte[] walkers = { 0x00, 0x1E, 0x16, 0x18, 0x12, 0x19, 0x1C, 0x14, 0x1A };
            List<byte[]> templates = new List<byte[]>();
            List<byte> types = new List<byte>();
            int list = ReadInt(nightmare, r.Record + 0x48);
            (int x, int y)? cameraLock = null;
            for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8)
                if (nightmare[at] - 1 == CameraLockCategory && nightmare[at + 1] == 0)
                    cameraLock = (ReadShort(nightmare, at + 4) - r.X * 16, ReadShort(nightmare, at + 6) - r.Y * 16);

            byte[] cells = RoomCells(nightmare, r.Record);
            int roomWidth = ReadShort(nightmare, r.Record + 0x14);
            int Kind(int cx, int cy) => cells[((r.Y + cy) * roomWidth + r.X + cx) * 4 + 3];
            bool Solid(int cx, int cy) => RoomCollision[Kind(cx, cy)] == 0x0D || Kind(cx, cy) >> 4 == 2;
            for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8) {
                int category = nightmare[at] - 1, index = nightmare[at + 1], variant = nightmare[at + 2];
                int x = ReadShort(nightmare, at + 4) - r.X * 16, y = ReadShort(nightmare, at + 6) - r.Y * 16;
                if (x < 0 || y < 0 || x >= r.Width * 16 || y >= r.Height * 16) continue;
                // (an enemy or item inside a wall, or on the top row hanging from nothing, NiDL's on its ceilings (Orange Ocean 2-5's
                // Sparkys, out of reach): down to the first open cell, else left out; one on the top row resting on a wall or blocks
                // stays, trapped as NiDL has it until the blocks go (room 0xFA's above its bomb chains);
                bool resting = y < 16 && !Solid(x / 16, y / 16) && y / 16 + 1 < r.Height && Solid(x / 16, y / 16 + 1);
                if (category is 0 or 3 && !resting && (y < 16 || Solid(x / 16, y / 16))) {
                    int cy = Math.Max(y / 16, 1);
                    while (cy < r.Height && cy <= y / 16 + 4 && Solid(x / 16, cy)) cy++;
                    if (cy >= r.Height || cy > y / 16 + 4) continue;
                    y = cy * 16 + 8;
                }

                byte type, behavior = 0;
                bool miniboss = false;
                if (category == 0 && landings.Any(c => Math.Abs(x / 16 - c.x) <= LandingClear && Math.Abs(y / 16 - c.y) <= 2)) continue;
                // (a Shotzo on blocks that break stays: NiDL's falls when they go, KatAM's stays where it is, but they're the room's cannons,
                // as room 0x2FD's under its ceiling);
                if (category == 0 && NativeEnemies.TryGetValue(index, out type)) {
                    /* (one that walks starts on the floor under its spot (up to 8 cells down, one-way floors too): NiDL drops its
                     * enemies there, KatAM's start standing, and a fall onto a one-way floor left room 0x153's Waddle Doo, placed
                     * under the ceiling, stuck in the floor); */
                    if (walkers.Contains(type)) {
                        int cy = y / 16;
                        while (cy + 1 < r.Height && cy <= y / 16 + 8 && RoomCollision[Kind(x / 16, cy + 1)] is not (>= 0x01 and <= 0x1B) && !Solid(x / 16, cy + 1)) cy++;
                        if (cy + 1 < r.Height && cy <= y / 16 + 8 && cy > y / 16) y = cy * 16 + 8;
                    }
                    /* (a Bomber's spot is the top of the floor under it, as KatAM's own: InitObject looks for a floor once, at the
                     * template's spot, before the object has its box, and bomber.c starts falling when there's none, then blows up on
                     * landing: from NiDL's spots, 8 pixels up, every Bomber went off before Kirby saw it. That look misses one-way
                     * floors (NiDL's 0x1X), so one on them stands on the floor under them; over a pit it stays); */
                    if (type == BomberType) {
                        bool Floor(int cx, int cy) => RoomCollision[Kind(cx, cy)] is >= 0x01 and <= 0x1B && Kind(cx, cy) >> 4 != 1 || Solid(cx, cy);
                        int cy = y / 16;
                        while (cy + 1 < r.Height && cy <= y / 16 + 6 && !Floor(x / 16, cy + 1)) cy++;
                        if (cy + 1 < r.Height && cy <= y / 16 + 6) y = (cy + 1) * 16;
                    }
                }
                else if (category == 0 && PortedEnemies.TryGetValue(index, out int ported)) {
                    type = NightmareID;
                    // Poppy Bros. Jr. on an apple or with a bomb: their variants in poppy_bros_jr.c;
                    if (index == 31) variant = variant == 0 ? 3 : variant == 1 ? 7 : 5;
                    else if (index == 32) variant = variant == 0 ? 4 : variant == 1 ? 8 : 6;
                    behavior = (byte)(ported << 4 | (variant & 0xF));
                } else if (category == MinibossCategory && RoomMinibosses.TryGetValue(index, out (byte type, byte behavior) boss)) {
                    (type, behavior) = boss;
                    if (MinibossDefinitions.FirstOrDefault(m => m.NightmareIndex == index)?.Behaviors.Contains((byte)(variant & 0xF)) == true)
                        behavior |= (byte)(variant & 0xF);
                    miniboss = true;
                } else if (category == BossCategory && RoomBosses.TryGetValue(index, out (byte boss, int x, int y, int screenDx, int screenDy) spot)) {
                    type = BossTypeOf(spot.boss);
                    behavior = (byte)(spot.boss << 4 | r.Level & 7);
                    if (spot.x >= 0) (x, y) = (spot.x - r.X * 16, spot.y - r.Y * 16);
                    cameraLock = (x + spot.screenDx, y + spot.screenDy);
                    miniboss = true;
                } else if (category == MetaKnightsCategory) {
                    (type, behavior) = RoomMetaKnights;
                    behavior |= (byte)(variant & 0xF);
                    miniboss = true;
                } else if (category == 3 && RoomItems.TryGetValue(index, out type)) {
                    // (one PlanChests() made a chest: the chest, holding it);
                    NidlChest chest = chests.FirstOrDefault(c => c.Replaces == at);
                    if (chest != null) {
                        templates.Add(ChestTemplate(chest, templates.Count));
                        if (!types.Contains(SmallChestType)) types.Add(SmallChestType);
                        continue;
                    }
                }
                else if (category == 4 && index == 2 && transportsOf[r].Any(t => t.Cannon)) {
                    // A cannon's fuse, where Kirby lights it (NiDL's gimmick 2, over its first cell): KatAM's fuse object, as room 0x1F6's;
                    byte[] fuse = ObjectTemplate(x, y, templates.Count, FuseType);
                    fuse[4] = fuse[5] = 1;
                    templates.Add(fuse);
                    if (!types.Contains(FuseType)) types.Add(FuseType);
                    continue;
                } else continue;
                // (and none where it ends up: a walker dropped onto the floor, a Bomber onto its, landed in front of the door, rooms
                // 0x277, 0x278, 0x27A, 0x2CB);
                if (category == 0 && landings.Any(c => Math.Abs(x / 16 - c.x) <= LandingClear && Math.Abs(y / 16 - c.y) <= 2)) continue;

                byte[] template = new byte[36];
                template[0] = 0x01; template[1] = 0x24; template[3] = (byte)templates.Count;
                // (an item is taken once per visit, as KatAM's: its own gone bit, rooms.c's NIDL_TAKEN_KIND and its number);
                if (category == 3) { template[2] = TakenKind; template[3] = (byte)takenItems++; }
                WriteShorts(template, 6, x, y);
                template[10] = template[11] = template[13] = 0xFF;
                template[12] = type; template[14] = behavior;
                // A mini-boss fights on its screen: the camera stays there once Kirby is in it, as in NiDL;
                if (miniboss) {
                    (int left, int top, int width, int height) = ArenaOf(r, cameraLock ?? (x, y), x, y, CameraTop(nightmare, r), IsBossType(type) ? HiddenRows * 16 : 0);
                    WriteShorts(template, 0x1A, left - x, top - y, width, height);
                }
                // and the camera stays there, its edges walls, until it's beaten: an arena object ("ROM Expansion/enemies/arena.c"), first,
                // so the camera is held when the mini-boss starts (and keeps itself on that screen);
                if (miniboss) {
                    byte[] arena = (byte[])template.Clone();
                    arena[12] = NightmareID; arena[14] = ArenaBehavior;
                    templates.Add(arena);
                    if (!types.Contains(NightmareID)) types.Add(NightmareID);
                    template[3] = (byte)templates.Count;
                }
                templates.Add(template);
                if (!types.Contains(type)) types.Add(type);
            }

            // The new chests (PlanChests());
            foreach (NidlChest chest in chests.Where(c => c.Replaces < 0)) {
                // (and the batteries: taken once per visit, as the items);
                byte[] template = chest.Battery ? ObjectTemplate(chest.X, chest.Y, templates.Count, BatteryType) : ChestTemplate(chest, templates.Count);
                if (chest.Battery) { template[2] = TakenKind; template[3] = (byte)takenItems++; }
                templates.Add(template);
                if (!types.Contains(template[12])) types.Add(template[12]);
            }

            /* Warp stars and cannons (see Transport), as KatAM's: a warp star (0x84, warp_star.c: +0x1E the room, +0x1A, +0x1C the landing
             * cell + 1; subtype its flight, WarpFlight: straight up, landing there); a cannon (0x8A, kirby_cannon.s, as room 0x1F6's: +0x12 the room, +0x18 the
             * landing (x << 8 | y), the fuse's state slot); */
            foreach (Transport t in transportsOf[r]) {
                byte[] template = ObjectTemplate(t.X, t.Y, templates.Count, t.Cannon ? CannonType : WarpStarType);
                if (t.Cannon) {
                    template[4] = template[5] = 1;
                    WriteShort(template, 0x12, t.Room);
                    template[0x14] = 2;
                    WriteShort(template, 0x18, t.DestX << 8 | t.DestY);
                    template[0x22] = 2;
                } else {
                    // (its flight: its NiDL ride's, or WarpFlight, see WarpFlights());
                    template[14] = (byte)t.Flight;
                    WriteShorts(template, 0x1A, t.DestX, t.DestY, t.Room);
                }
                templates.Add(template);
                if (!types.Contains(template[12])) types.Add(template[12]);
            }

            // A switch room's big switch (NiDL's own aren't ported: a level has one, see LevelSlot()) and ability statues (SwitchStatues);
            if (r.IsSwitchRoom) {
                templates.Add(BigSwitchTemplate(SwitchAt(r.Level).x, SwitchAt(r.Level).y, templates.Count, LevelSlot(r.Level), r.Level));
                types.Add(BigSwitchType);
                for (int i = 0; i < 2; i++) {
                    (byte type, byte subtype) = SwitchStatues[r.Level][i];
                    byte[] statue = ObjectTemplate(SwitchStatueXs(r.Level)[i], 48, templates.Count, type);
                    statue[14] = subtype; statue[15] = 0;
                    templates.Add(statue);
                    if (!types.Contains(type)) types.Add(type);
                }
            }

            // A mirror on each door (see Door); a hidden one is a door object (0x97, its own mirror), showing once the slot is set; one in
            // a mini-boss's fight (ArenaCells()) shows only while its cell is a door: those are built shut (BuildSolidity()) and the
            // arena opens them once the mini-boss is beaten ("ROM Expansion/enemies/arena.c", OpenDoors());
            foreach (Door door in doors.Where(d => !d.Edge && !d.Plain)) {
                byte type = door.Slot != 0 ? DoorToHubType : MirrorType;
                byte[] mirror = door.Slot != 0 ? DoorToHubTemplate(door.X, door.Y, templates.Count, door.Slot, door.Design) : MirrorTemplate(door.X, door.Y, templates.Count);
                // (layout B's doors in the middle, on cell 8, drawn at x 128, the room's middle, as 0x122's: DoorDrawX);
                if (r.IsSwitchRoom && LayoutB(r.Level) && door.Slot != 0 && door.X == 8) WriteShort(mirror, 6, 128);
                if (door.Secret || door.Shut) mirror[14] = 1;    // subtype1 1: hidden until its cell is a door (CreateMirrorDoor);
                if (door.OneWay && door.Slot == 0) mirror[0x23] |= 1;    // +0x22 0x100: drawn as a one-way mirror (mirror_door.c), as KatAM's;
                templates.Add(mirror);
                if (!types.Contains(type)) types.Add(type);
            }

            // Mirras (see MirraSpots()), standing on their doors' cells as KatAM's (x the cell's middle, y its top);
            foreach ((int x, int y, bool door) in mirras) {
                templates.Add(ObjectTemplate(x * 16 + 8, y * 16, templates.Count, MirraType));
                if (!types.Contains(MirraType)) types.Add(MirraType);
            }

            // KatAM keeps which of a room's objects are gone in 32 bits per room (gUnk_02023358), by number (31: none);
            if (templates.Count > 31) throw new ArgumentException($"Room 0x{r.Room:X} has {templates.Count} objects (at most 31).");
            return WriteObjectList(rom, templates, types, noTypes, ref cursor);
        }

        /* ThinFloors(); NiDL's thin floors are KatAM's Kirby drops through (0x0E, slopes 0x10-0x13, 0x18, 0x19: Down and B, which slides,
         * or with Bomb throws one down: Kirby fell through Rainbow Resort 4-7's, 0x304, bombing on it), where a way on needs it (Butter
         * Building's 0x9B); in a room whose doors, warp stars and cannons Kirby reaches from each place he arrives without dropping
         * through any (he flies: any cell not solid, never down into a thin floor), they're KatAM's he doesn't (0x0F, 0x14-0x17, 0x1A,
         * 0x1B); */
        static void ThinFloors(NightmareRoom r, byte[] collision, List<(int x, int y)> arrivals, List<(int x, int y)> targets) {
            static byte Kept(byte value) => value switch { 0x0E => 0x0F, >= 0x10 and <= 0x13 => (byte)(value + 4), 0x18 => 0x1A, 0x19 => 0x1B, _ => value };
            int width = r.FullWidth, height = r.FullHeight;
            if (!collision.Any(v => Kept(v) != v)) return;
            HashSet<(int, int)> Reach((int x, int y) from, bool drop) {
                HashSet<(int, int)> seen = new HashSet<(int, int)>();
                Queue<(int x, int y)> open = new Queue<(int, int)>();
                if (from.x >= 0 && from.y >= 0 && from.x < width && from.y < height && seen.Add(from)) open.Enqueue(from);
                while (open.Count > 0) {
                    (int x, int y) = open.Dequeue();
                    foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) }) {
                        if (nx < 0 || ny < 0 || nx >= width || ny >= height || seen.Contains((nx, ny))) continue;
                        byte v = collision[ny * width + nx];
                        if (v is 0x0D or (>= 0x01 and <= 0x04) || v >= 0xF0 && v < 0xFC) continue;
                        if (!drop && ny == y + 1 && Kept(v) != v) continue;
                        seen.Add((nx, ny));
                        open.Enqueue((nx, ny));
                    }
                }
                return seen;
            }
            bool Near(HashSet<(int, int)> seen, (int x, int y) c) => Enumerable.Range(-1, 3).Any(dx => Enumerable.Range(-1, 3).Any(dy => seen.Contains((c.x + dx, c.y + dy))));
            foreach (var from in arrivals.Distinct()) {
                HashSet<(int, int)> withDrop = Reach(from, true), without = Reach(from, false);
                if (targets.Any(c => Near(withDrop, c) && !Near(without, c))) return;
            }
            for (int i = 0; i < collision.Length; i++) collision[i] = Kept(collision[i]);
        }

        /* ArenaCells(); The cells of a room's (part's) mini-boss fights whose doors are built shut until the fight is won: from the fight
         * area (Arenas()), the cells Kirby goes to through anything but walls (KatAM's collision 0x0D) within ArenaReach cells of it
         * across, ArenaReachY up or down (the room it's fought in: 0x244's door 2 cells past Fire Lion's screen, by the wall; not a
         * door behind a wall, in another part of the room); "ROM Expansion/enemies/arena.c" opens the doors in the same box
         * (ARENA_REACH, ARENA_REACH_Y); */
        const int ArenaReach = 4, ArenaReachY = 2;
        static HashSet<(int x, int y)> ArenaCells(List<(int left, int top, int width, int height)> fights, byte[] collision, int stride, int width, int height) {
            HashSet<(int x, int y)> cells = new HashSet<(int, int)>();
            foreach ((int left, int top, int areaWidth, int areaHeight) in fights) {
                int x0 = left >> 4, y0 = top >> 4, x1 = (left + areaWidth - 1) >> 4, y1 = (top + areaHeight - 1) >> 4;
                bool Open((int x, int y) c) => c.x >= Math.Max(0, x0 - ArenaReach) && c.x <= Math.Min(width - 1, x1 + ArenaReach)
                                              && c.y >= Math.Max(0, y0 - ArenaReachY) && c.y <= Math.Min(height - 1, y1 + ArenaReachY)
                                              && collision[c.y * stride + c.x] != 0x0D;
                Queue<(int x, int y)> open = new Queue<(int, int)>();
                for (int y = y0; y <= y1; y++)
                    for (int x = x0; x <= x1; x++)
                        if (Open((x, y)) && cells.Add((x, y))) open.Enqueue((x, y));
                while (open.Count > 0) {
                    (int x, int y) = open.Dequeue();
                    foreach ((int, int) n in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                        if (Open(n) && cells.Add(n)) open.Enqueue(n);
                }
            }
            return cells;
        }

        /* Arenas(); A room's (part's) mini-bosses' fight areas (ArenaOf(), pixels), as BuildObjects() gives them; */
        static List<(int left, int top, int width, int height)> Arenas(byte[] nightmare, NightmareRoom r) {
            List<(int, int, int, int)> arenas = new List<(int, int, int, int)>();
            int list = ReadInt(nightmare, r.Record + 0x48);
            (int x, int y)? cameraLock = null;
            for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8)
                if (nightmare[at] - 1 == CameraLockCategory && nightmare[at + 1] == 0)
                    cameraLock = (ReadShort(nightmare, at + 4) - r.X * 16, ReadShort(nightmare, at + 6) - r.Y * 16);
            for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8) {
                int category = nightmare[at] - 1, index = nightmare[at + 1];
                int x = ReadShort(nightmare, at + 4) - r.X * 16, y = ReadShort(nightmare, at + 6) - r.Y * 16;
                if (x < 0 || y < 0 || x >= r.Width * 16 || y >= r.Height * 16) continue;
                if (category == MinibossCategory && RoomMinibosses.ContainsKey(index) || category == MetaKnightsCategory)
                    arenas.Add(ArenaOf(r, cameraLock ?? (x, y), x, y, CameraTop(nightmare, r)));
                else if (category == BossCategory && RoomBosses.TryGetValue(index, out (byte boss, int x, int y, int screenDx, int screenDy) spot)) {
                    if (spot.x >= 0) (x, y) = (spot.x - r.X * 16, spot.y - r.Y * 16);
                    arenas.Add(ArenaOf(r, (x + spot.screenDx, y + spot.screenDy), x, y, CameraTop(nightmare, r), HiddenRows * 16));
                }
            }
            return arenas;
        }

        /* ArenaOf(); A mini-boss's fight area: the screen around NiDL's camera lock (its x the screen's middle, its y near the top), moved
         * to hold the mini-boss too, inside what KatAM's camera shows of the room (8 pixels in from its sides, cameraTop (CameraTop())
         * from its top, 40 from its bottom; see Padded()); a boss's may go `below` more (the room's hidden rows: down to where KatAM's
         * camera stops, so its floor shows: Whispy Woods'); */
        static (int left, int top, int width, int height) ArenaOf(NightmareRoom r, (int x, int y) cameraLock, int x, int y, int cameraTop, int below = 0) {
            const int ScreenWidth = 240, ScreenHeight = 160, Edge = 8, Margin = 24;
            int Bottom = 40 - below;
            int width = Math.Min(ScreenWidth, r.FullWidth * 16 - 2 * Edge), height = Math.Min(ScreenHeight, r.FullHeight * 16 - Edge - Bottom);
            int left = cameraLock.x - width / 2, top = cameraLock.y - Margin;
            left = Math.Clamp(left, x + Margin - width, x - Margin);
            top = Math.Clamp(top, y + Margin - height, y - Margin);
            left = Math.Clamp(left, Edge, r.FullWidth * 16 - Edge - width);
            top = Math.Clamp(top, Math.Min(cameraTop, r.FullHeight * 16 - Bottom - height), r.FullHeight * 16 - Bottom - height);
            return (left, top, width, height);
        }

        // WriteObjectList(); A room's LevelInfo_1E4: its templates (then an end), the types in it, an empty list, how many types;
        static int WriteObjectList(byte[] rom, List<byte[]> templates, List<byte> types, int noTypes, ref int cursor) {
            int templatesAt = Allocate(ref cursor, templates.Count * 36 + 36, 4);
            for (int i = 0; i < templates.Count; i++) templates[i].CopyTo(rom, templatesAt + i * 36);
            Array.Fill(rom, (byte)0xFF, templatesAt + templates.Count * 36, 36);
            rom[templatesAt + templates.Count * 36] = rom[templatesAt + templates.Count * 36 + 1] = 0;
            int typesAt = Allocate(ref cursor, types.Count + 1, 4);
            types.Append((byte)0xFF).ToArray().CopyTo(rom, typesAt);

            int objectList = Allocate(ref cursor, 16, 4);
            WriteInts(rom, objectList, templatesAt + 0x08000000, typesAt + 0x08000000, noTypes + 0x08000000);
            rom[objectList + 12] = (byte)types.Count;
            rom[objectList + 13] = rom[objectList + 14] = rom[objectList + 15] = 0;
            return objectList;
        }

        // AllocateCompressed(); Data copied into the expansion, 4-aligned; its offset;
        static int AllocateCompressed(byte[] rom, byte[] data, ref int cursor) {
            int at = Allocate(ref cursor, data.Length, 4);
            data.CopyTo(rom, at);
            return at;
        }

        // DecompressRL(); KatAM's RL data (collision): u32 size << 8, then runs (flag bit 7: (flag & 0x7F) + 3 copies of a byte; else (flag + 1) bytes);
        static byte[] DecompressRL(byte[] rom, int address) {
            int size = ReadInt(rom, address) >> 8, at = address + 4, written = 0;
            byte[] output = new byte[size];
            while (written < size) {
                byte flag = rom[at++];
                if ((flag & 0x80) != 0) { int n = (flag & 0x7F) + 3; for (int k = 0; k < n && written < size; k++) output[written++] = rom[at]; at++; }
                else { int n = (flag & 0x7F) + 1; for (int k = 0; k < n && written < size; k++) output[written++] = rom[at++]; }
            }
            return output;
        }

        /* BuildNavigation(); The CPU helpers' zones (LevelInfo_1E8): the route lists are room 0x3C9's (generic); +0x10 the zone of each
         * cell (RL), +0x14 the zones (0x34 bytes each: +0 the zone to go through towards each zone, 0xFF none; +0x20, +0x22 its point,
         * where a called helper appears when the player is in it; +0x24 0x20; +0x28 its number, +0x29 how many neighbours, then
         * them), +0x18 how many (32 at most: the table's size). The room is cut in blocks (8x6 cells, larger until there are 32 at
         * most), a zone per block Kirby can stand in (its point: the open cell over a floor nearest its middle), the others' cells
         * given to the nearest zone; blocks side by side are neighbours. A fight's area (Arenas(), where the camera is held) holds a
         * zone's point: a called helper looks for one inside the camera's bounds, forever if there's none (sub_08010034); returns
         * its address; */
        static int BuildNavigation(byte[] rom, NightmareRoom r, byte[] collision, int routes, List<(int left, int top, int width, int height)> arenas, ref int cursor) {
            int width = r.FullWidth, height = r.FullHeight;
            bool Open(int x, int y) => x >= 0 && y >= 0 && x < width && y < height
                                       && collision[y * width + x] is 0x00 or 0x48 or 0x20 or 0x21 or 0xFC or 0xFD or 0xFE or 0xFF;
            bool Standable(int x, int y) => Open(x, y) && y + 1 < height && !Open(x, y + 1);

            int blockWidth = 8, blockHeight = 6;
            List<(int bx, int by, int x, int y)> zones;
            while (true) {
                zones = new List<(int, int, int, int)>();
                for (int by = 0; by * blockHeight < height; by++)
                    for (int bx = 0; bx * blockWidth < width; bx++) {
                        int cx = bx * blockWidth + blockWidth / 2, cy = by * blockHeight + blockHeight / 2, best = int.MaxValue, px = -1, py = -1;
                        for (int y = by * blockHeight; y < Math.Min(height, (by + 1) * blockHeight); y++)
                            for (int x = bx * blockWidth; x < Math.Min(width, (bx + 1) * blockWidth); x++) {
                                int distance = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                                if (Standable(x, y) && distance < best) { best = distance; px = x; py = y; }
                            }
                        if (px >= 0) zones.Add((bx, by, px, py));
                    }
                if (zones.Count <= 32) break;
                if (blockWidth <= blockHeight * 2) blockWidth += 4; else blockHeight += 3;
            }
            if (zones.Count == 0) zones.Add((0, 0, width / 2, height / 2));
            // (each fight's area: the cell it can stand on nearest its middle, else an open one, becomes its block's zone's point);
            foreach (var arena in arenas) {
                bool Inside(int x, int y) => x * 16 + 8 > arena.left && x * 16 + 8 < arena.left + arena.width
                                             && y * 16 + 8 > arena.top && y * 16 + 8 < arena.top + arena.height;
                if (zones.Any(z => Inside(z.x, z.y))) continue;
                int mx = (arena.left + arena.width / 2) / 16, my = (arena.top + arena.height / 2) / 16;
                var cells = Enumerable.Range(0, height).SelectMany(y => Enumerable.Range(0, width).Select(x => (x, y))).Where(c => Inside(c.x, c.y)).ToList();
                var spot = cells.OrderBy(c => Standable(c.x, c.y) ? 0 : Open(c.x, c.y) ? 1 : 2)
                                .ThenBy(c => (c.x - mx) * (c.x - mx) + (c.y - my) * (c.y - my)).Select(c => ((int, int)?)c).FirstOrDefault();
                if (spot == null) continue;
                var (sx, sy) = spot.Value;
                int index = zones.FindIndex(z => z.bx == sx / blockWidth && z.by == sy / blockHeight);
                if (index >= 0) zones[index] = (zones[index].bx, zones[index].by, sx, sy);
                else if (zones.Count < 32) zones.Add((sx / blockWidth, sy / blockHeight, sx, sy));
            }

            // Each cell's zone: its block's, else the nearest zone's point;
            byte[] map = new byte[width * height];
            Dictionary<(int, int), int> zoneOf = zones.Select((z, i) => (z, i)).ToDictionary(t => (t.z.bx, t.z.by), t => t.i);
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    map[y * width + x] = (byte)(zoneOf.TryGetValue((x / blockWidth, y / blockHeight), out int z) ? z
                        : Enumerable.Range(0, zones.Count).MinBy(i => (zones[i].x - x) * (zones[i].x - x) + (zones[i].y - y) * (zones[i].y - y)));
            byte[] zonesMap = CompressRL(map);
            int zonesAt = Allocate(ref cursor, zonesMap.Length, 4);
            zonesMap.CopyTo(rom, zonesAt);

            // Neighbours (blocks side by side), and the way to each zone (breadth first);
            List<int>[] neighbours = zones.Select(z => new[] { (1, 0), (-1, 0), (0, 1), (0, -1) }
                .Where(d => zoneOf.ContainsKey((z.bx + d.Item1, z.by + d.Item2))).Select(d => zoneOf[(z.bx + d.Item1, z.by + d.Item2)]).ToList()).ToArray();
            int records = Allocate(ref cursor, zones.Count * 0x34, 4);
            for (int i = 0; i < zones.Count; i++) {
                int record = records + i * 0x34;
                Array.Fill(rom, (byte)0xFF, record, 0x34);
                // (from each target back towards this zone: the first step is the neighbour we came from);
                byte[] via = Enumerable.Repeat((byte)0xFF, 0x20).ToArray();
                via[i] = (byte)i;
                Queue<int> queue = new Queue<int>();
                foreach (int n in neighbours[i]) { via[n] = (byte)n; queue.Enqueue(n); }
                while (queue.Count > 0) {
                    int at = queue.Dequeue();
                    foreach (int n in neighbours[at]) if (via[n] == 0xFF) { via[n] = via[at]; queue.Enqueue(n); }
                }
                via.CopyTo(rom, record);
                WriteShorts(rom, record + 0x20, zones[i].x * 16 + 8, zones[i].y * 16 + 8, 0x20);
                rom[record + 0x26] = rom[record + 0x27] = 0;
                rom[record + 0x28] = (byte)i;
                rom[record + 0x29] = (byte)Math.Min(neighbours[i].Count, 8);
                for (int n = 0; n < neighbours[i].Count && n < 8; n++) rom[record + 0x2A + n] = (byte)neighbours[i][n];
                WriteShort(rom, record + 0x32, 0);
            }

            int nav = Allocate(ref cursor, 0x1C, 4);
            Array.Copy(rom, routes, rom, nav, 16);
            WriteInts(rom, nav + 16, zonesAt + 0x08000000, records + 0x08000000, zones.Count);
            return nav;
        }

        // CompressLZ77(); GBA BIOS LZ77 (type 0x10), VRAM safe (never copies from 1 byte back);
        static byte[] CompressLZ77(byte[] data) {
            List<byte> output = new List<byte> { 0x10, (byte)data.Length, (byte)(data.Length >> 8), (byte)(data.Length >> 16) };
            Dictionary<int, List<int>> heads = new Dictionary<int, List<int>>();
            int Key(int i) => i + 2 < data.Length ? data[i] | data[i + 1] << 8 | data[i + 2] << 16 : -1;
            void Add(int i) {
                int key = Key(i);
                if (key < 0) return;
                if (!heads.TryGetValue(key, out List<int> list)) heads[key] = list = new List<int>();
                list.Add(i);
            }

            for (int i = 0; i < data.Length;) {
                int flagsAt = output.Count;
                output.Add(0);
                for (int bit = 0; bit < 8 && i < data.Length; bit++) {
                    int bestLength = 0, bestDistance = 0;
                    if (heads.TryGetValue(Key(i), out List<int> candidates)) {
                        for (int c = candidates.Count - 1, tried = 0; c >= 0 && tried < 64; c--, tried++) {
                            int j = candidates[c], distance = i - j;
                            if (distance > 0x1000) break;
                            if (distance < 2) continue;
                            int length = 0;
                            while (length < 18 && i + length < data.Length && data[j + length] == data[i + length]) length++;
                            if (length > bestLength) { bestLength = length; bestDistance = distance; }
                            if (length == 18) break;
                        }
                    }
                    if (bestLength >= 3) {
                        output[flagsAt] |= (byte)(0x80 >> bit);
                        int code = (bestLength - 3) << 12 | (bestDistance - 1);
                        output.Add((byte)(code >> 8));
                        output.Add((byte)code);
                        for (int k = 0; k < bestLength; k++) Add(i + k);
                        i += bestLength;
                    } else {
                        output.Add(data[i]);
                        Add(i);
                        i++;
                    }
                }
            }
            while (output.Count % 4 != 0) output.Add(0);
            return output.ToArray();
        }

        // CompressRL(); GBA BIOS run-length (type 0x30);
        static byte[] CompressRL(byte[] data) {
            List<byte> output = new List<byte> { 0x30, (byte)data.Length, (byte)(data.Length >> 8), (byte)(data.Length >> 16) };
            List<byte> literal = new List<byte>();
            void Flush() {
                for (int i = 0; i < literal.Count; i += 128) {
                    int count = Math.Min(128, literal.Count - i);
                    output.Add((byte)(count - 1));
                    output.AddRange(literal.Skip(i).Take(count));
                }
                literal.Clear();
            }

            for (int i = 0; i < data.Length;) {
                int run = 1;
                while (i + run < data.Length && data[i + run] == data[i] && run < 130) run++;
                if (run >= 3) {
                    Flush();
                    output.Add((byte)(0x80 | (run - 3)));
                    output.Add(data[i]);
                    i += run;
                } else {
                    literal.Add(data[i++]);
                }
            }
            Flush();
            while (output.Count % 4 != 0) output.Add(0);
            return output.ToArray();
        }
    }
}
