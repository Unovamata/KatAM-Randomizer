using System;
using System.Collections.Generic;

namespace KatAMMapTracker {
    // EntityIcon; The in-game sprites the map can draw next to the rooms;
    public enum EntityIcon {
        Cherry, PepBrew, Meat, MaximTomato, Battery, OneUp, InvincibleCandy, MirrorShard, ChestSmall, ChestBig, WarpStar, BigSwitch, FuseCannon,
        MrFrosty, Bonkers, PhanPhan, Batafire, BoxBoxer, Boxy, MasterHandMini, Bombar,
        Kracko, KingGolem, MasterHand, Gobbler, Wiz, Moley, MegaTitan, DarkMetaKnight, DarkMind, DarkMindOrb,

        // Treasure, as the Collection Room shows it;
        Vitality, SoundPlayer,
        Spray1, Spray2, Spray3, Spray4, Spray5, Spray6, Spray7, Spray8, Spray9, Spray10, Spray11, Spray12, Spray13, Spray14,
        Music1, Music2, Music3, Music4, Music5, Music6, Music7, Music8, Music9, Music10,
        MapTutorial, MapRainbowRoute, MapMoonlightMansion, MapCabbageCavern, MapMustardMountain, MapCarrotCastle,
        MapOliveOcean, MapPeppermintPalace, MapRadishRuins, MapCandyConstellation,
    }

    // EntityKind; What an important object is, in drawing order (the first ones are the most important);
    public enum EntityKind { Boss, MiniBoss, MirrorShard, BigSwitch, Treasure, Item, FuseCannon, WarpStar }

    /* MapEntity; An important object of a room: what it is, the sprite drawn for it and its name.
     * ContentIcon: for chests, the sprite of what they hold (null if unknown), shown when chest contents are spoiled; */
    public sealed record MapEntity(EntityKind Kind, EntityIcon Icon, string Name, EntityIcon? ContentIcon = null);

    /* MapEntities; Which room objects are worth showing on the map (bosses, mini-bosses, mirror shards, big switches, treasure,
     * items, fuse cannons and warp stars), and their in-game sprites, read from the ROM like the rest of the map graphics.
     *
     * The sprites were located by planting each object next to Kirby in an emulator and reading back the hardware sprites:
     * every sprite is a few OAM pieces, each one a run of consecutive 4bpp 8x8 tiles in the ROM (in 1D mapping order) with a
     * 16 color palette. Pieces are listed like the game lists them in OAM, so the first one is drawn on top.
     * The addresses are the Japanese ones; The other versions keep the sprite graphics and object palettes in blocks moved
     * as a whole, translated below;
     *
     * • Room object IDs: 0x38-0x3F mini-bosses, 0x45-0x52 bosses, 0x5E-0x64 items, 0x65 Mirror Shard (behavior 0-7; 8 is the
     *   Dimensional Mirror), 0x80 & 0x81 small & big chests (behavior: what they hold), 0x84 Warp Star, 0x86 Big Switch,
     *   0x8A Fuse Cannon.
     * • Chest contents: 0x00-0x05 the items 0x5E-0x63, 0x06 Vitality, 0x0A-0x13 area maps, 0x14-0x21 sprays, 0x28 the
     *   Sound Player, 0x29-0x32 music, 0x63 the big treasures.
     * • Treasure sprites (sprays, music, maps, Vitality, Sound Player) come from the Collection Room, which shows them as
     *   16x16 sprites (Japanese tiles at 0x7BAEB8 and on, palettes at 0x4A2818 and on); */
    public static class MapEntities {
        // IconPart; A sprite piece: position in the sprite, size, flips, palette and first tile (Japanese addresses);
        readonly record struct IconPart(int X, int Y, int Width, int Height, bool FlipH, bool FlipV, int Palette, int Tiles);

        static IconPart P(int x, int y, int width, int height, int flipH, int flipV, int palette, int tiles) =>
            new IconPart(x, y, width, height, flipH != 0, flipV != 0, palette, tiles);

        static readonly Dictionary<EntityIcon, IconPart[]> Icons = new Dictionary<EntityIcon, IconPart[]> {
            [EntityIcon.Cherry] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603598),
            },
            [EntityIcon.PepBrew] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603498),
            },
            [EntityIcon.Meat] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603618),
            },
            [EntityIcon.MaximTomato] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603518),
            },
            [EntityIcon.Battery] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603698),
            },
            [EntityIcon.OneUp] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603398),
            },
            [EntityIcon.InvincibleCandy] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603418),
            },
            [EntityIcon.MirrorShard] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x4A0D78, 0x6087F8),
                P(0, 16, 16, 8, 0, 0, 0x4A0D78, 0x608878),
                P(16, 0, 8, 16, 0, 0, 0x4A0D78, 0x6088B8),
                P(16, 16, 8, 8, 0, 0, 0x4A0D78, 0x5F0045),
            },
            [EntityIcon.ChestSmall] = new[] {
                P(0, 0, 16, 16, 0, 0, 0x4A0BB8, 0x603E98),
            },
            [EntityIcon.ChestBig] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A0BB8, 0x603A98),
            },
            [EntityIcon.WarpStar] = new[] {
                P(0, 8, 32, 16, 1, 0, 0x49F758, 0x5F56B8),
                P(0, 0, 32, 8, 1, 0, 0x49F758, 0x5F57B8),
            },
            [EntityIcon.BigSwitch] = new[] {
                P(16, 0, 32, 32, 1, 0, 0x4A0F78, 0x610ED8),
                P(0, 0, 16, 32, 1, 0, 0x4A0F78, 0x6110D8),
            },
            [EntityIcon.FuseCannon] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A0F18, 0x60DA78),
            },
            [EntityIcon.MrFrosty] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A1938, 0x6CC198),
                P(0, 32, 32, 8, 0, 0, 0x4A1938, 0x6CBFD8),
                P(32, 0, 16, 32, 0, 0, 0x4A1938, 0x6CC058),
                P(32, 32, 16, 8, 0, 0, 0x4A1938, 0x6CC158),
            },
            [EntityIcon.Bonkers] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1978, 0x6D1ED8),
                P(0, 32, 32, 16, 0, 0, 0x4A1978, 0x6D22D8),
                P(32, 32, 32, 16, 0, 0, 0x4A1978, 0x6D23D8),
            },
            [EntityIcon.PhanPhan] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A19D8, 0x6DE558),
                P(0, 32, 32, 16, 0, 0, 0x4A19D8, 0x6DE958),
                P(0, 48, 32, 8, 0, 0, 0x4A19D8, 0x6DEA58),
                P(32, 32, 32, 16, 0, 0, 0x4A19D8, 0x6DEAD8),
                P(32, 48, 32, 8, 0, 0, 0x4A19D8, 0x6DEBD8),
            },
            [EntityIcon.Batafire] = new[] {
                P(43, 56, 16, 16, 0, 0, 0x4A1018, 0x6369F8),
                P(43, 72, 16, 8, 0, 0, 0x4A1018, 0x636A78),
                P(0, 0, 64, 64, 0, 0, 0x4A1018, 0x613518),
                P(0, 64, 32, 16, 0, 0, 0x4A1018, 0x613D18),
                P(0, 80, 32, 8, 0, 0, 0x4A1018, 0x5F98FD),
                P(32, 64, 32, 16, 0, 0, 0x4A1018, 0x613E98),
                P(32, 80, 32, 8, 0, 0, 0x4A1018, 0x613F98),
                P(64, 0, 16, 32, 0, 0, 0x4A1018, 0x614018),
                P(64, 32, 16, 32, 0, 0, 0x4A1018, 0x614118),
                P(64, 64, 16, 16, 0, 0, 0x4A1018, 0x5F98FD),
                P(64, 80, 16, 8, 0, 0, 0x4A1018, 0x5F009D),
            },
            [EntityIcon.BoxBoxer] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A1538, 0x658D78),
                P(0, 32, 32, 16, 0, 0, 0x4A1538, 0x658F78),
                P(32, 0, 16, 32, 0, 0, 0x4A1538, 0x659078),
                P(32, 32, 16, 16, 0, 0, 0x4A1538, 0x659178),
            },
            [EntityIcon.Boxy] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1DB8, 0x70A9D8),
                P(0, 32, 32, 8, 0, 0, 0x4A1DB8, 0x70ADD8),
                P(32, 32, 32, 8, 0, 0, 0x4A1DB8, 0x70AE58),
                P(64, 0, 8, 32, 0, 0, 0x4A1DB8, 0x70AED8),
                P(64, 32, 8, 8, 0, 0, 0x4A1DB8, 0x5F0045),
            },
            [EntityIcon.MasterHandMini] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1D38, 0x6FE858),
                P(0, 32, 32, 16, 0, 0, 0x4A1D38, 0x6141DA),
                P(32, 32, 32, 16, 0, 0, 0x4A1D38, 0x6FED58),
                P(64, 0, 8, 32, 0, 0, 0x4A1D38, 0x6FEE58),
                P(64, 32, 8, 16, 0, 0, 0x4A1D38, 0x6FCA5C),
            },
            [EntityIcon.Bombar] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1878, 0x6A3078),
                P(0, 32, 32, 16, 0, 0, 0x4A1878, 0x6A2878),
                P(32, 32, 32, 16, 0, 0, 0x4A1878, 0x6A2978),
            },
            [EntityIcon.Kracko] = new[] {
                P(36, 20, 16, 16, 0, 0, 0x4A1DF8, 0x719A38),
                P(36, 36, 16, 8, 0, 0, 0x4A1DF8, 0x719AB8),
                P(52, 20, 8, 16, 0, 0, 0x4A1DF8, 0x719AF8),
                P(52, 36, 8, 8, 0, 0, 0x4A1DF8, 0x719B38),
                P(0, 0, 64, 32, 0, 0, 0x4A1E18, 0x719D98),
                P(64, 0, 32, 32, 0, 0, 0x4A1E18, 0x71A198),
                P(0, 32, 64, 32, 0, 1, 0x4A1E38, 0x719D98),
                P(64, 32, 32, 32, 0, 1, 0x4A1E38, 0x71A198),
            },
            [EntityIcon.KingGolem] = new[] {
                P(0, 0, 32, 64, 0, 0, 0x4A15D8, 0x673118),
                P(0, 64, 32, 32, 0, 0, 0x4A15D8, 0x673518),
                P(0, 96, 32, 8, 0, 0, 0x4A15D8, 0x66C618),
                P(32, 32, 16, 32, 0, 0, 0x4A15D8, 0x673898),
                P(32, 64, 16, 32, 0, 0, 0x4A15D8, 0x673998),
                P(32, 96, 16, 8, 0, 0, 0x4A15D8, 0x66C918),
                P(48, 32, 8, 32, 0, 0, 0x4A15D8, 0x5F98FD),
                P(48, 64, 8, 32, 0, 0, 0x4A15D8, 0x6718E4),
                P(48, 96, 8, 8, 0, 0, 0x4A15D8, 0x66CA98),
            },
            [EntityIcon.MasterHand] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1D38, 0x6FC2B8),
                P(0, 32, 32, 16, 0, 0, 0x4A1D38, 0x6FC6B8),
                P(0, 48, 32, 8, 0, 0, 0x4A1D38, 0x5F98FD),
                P(32, 32, 32, 16, 0, 0, 0x4A1D38, 0x6FC838),
                P(32, 48, 32, 8, 0, 0, 0x4A1D38, 0x6FC938),
                P(64, 0, 8, 32, 0, 0, 0x4A1D38, 0x6FC9B8),
                P(64, 32, 8, 16, 0, 0, 0x4A1D38, 0x6FCA38),
                P(64, 48, 8, 8, 0, 0, 0x4A1D38, 0x5F0045),
            },
            [EntityIcon.Gobbler] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A1718, 0x687578),
                P(0, 32, 32, 16, 0, 0, 0x4A1718, 0x687778),
                P(0, 48, 32, 8, 0, 0, 0x4A1718, 0x6871F8),
                P(32, 0, 16, 32, 0, 0, 0x4A1718, 0x6878F8),
                P(32, 32, 16, 16, 0, 0, 0x4A1718, 0x6879F8),
                P(32, 48, 16, 8, 0, 0, 0x4A1718, 0x6102DC),
                P(48, 0, 8, 32, 0, 0, 0x4A1718, 0x686D98),
                P(48, 32, 8, 16, 0, 0, 0x4A1718, 0x60DF30),
                P(48, 48, 8, 8, 0, 0, 0x4A1718, 0x5F0045),
                P(48, 8, 32, 32, 0, 0, 0x4A1718, 0x691D98),
            },
            [EntityIcon.Wiz] = new[] {
                P(54, 26, 8, 8, 0, 0, 0x49F758, 0x5F6518),
                P(0, 0, 64, 32, 1, 0, 0x4A18B8, 0x6C3078),
                P(32, 32, 32, 16, 1, 0, 0x4A18B8, 0x6C3478),
                P(32, 48, 32, 8, 1, 0, 0x4A18B8, 0x5F98FD),
                P(0, 32, 32, 16, 1, 0, 0x4A18B8, 0x6C35F8),
                P(0, 48, 32, 8, 1, 0, 0x4A18B8, 0x6C2FF8),
            },
            [EntityIcon.Moley] = new[] {
                P(0, 0, 64, 32, 0, 0, 0x4A1F38, 0x72EC38),
                P(0, 32, 32, 16, 0, 0, 0x4A1F38, 0x72F038),
                P(32, 32, 32, 16, 0, 0, 0x4A1F38, 0x72F138),
            },
            [EntityIcon.MegaTitan] = new[] {
                P(0, 0, 32, 32, 0, 0, 0x4A1EB8, 0x71CE18),
                P(0, 32, 32, 16, 0, 0, 0x4A1EB8, 0x71D018),
                P(32, 0, 16, 32, 0, 0, 0x4A1EB8, 0x71D118),
                P(32, 32, 16, 16, 0, 0, 0x4A1EB8, 0x71D218),
            },
            [EntityIcon.DarkMetaKnight] = new[] {
                P(0, 0, 16, 32, 0, 0, 0x4A2A18, 0x7CB9F8),
                P(16, 0, 8, 32, 0, 0, 0x4A2A18, 0x7CBAF8),
                P(8, 8, 32, 32, 0, 0, 0x4A29D8, 0x7BCF98),
                P(40, 8, 8, 32, 0, 0, 0x4A29D8, 0x7BD198),
            },
            [EntityIcon.DarkMind] = new[] {
                P(0, 0, 64, 64, 0, 0, 0x4A2A98, 0x7DCA58),
                P(0, 64, 32, 16, 0, 0, 0x4A2A98, 0x7DD258),
                P(0, 80, 32, 8, 0, 0, 0x4A2A98, 0x7DD358),
                P(32, 64, 32, 16, 0, 0, 0x4A2A98, 0x7DD3D8),
                P(32, 80, 32, 8, 0, 0, 0x4A2A98, 0x7DD4D8),
                P(64, 0, 32, 64, 0, 0, 0x4A2A98, 0x7DD558),
                P(64, 64, 32, 16, 0, 0, 0x4A2A98, 0x7DD958),
                P(64, 80, 32, 8, 0, 0, 0x4A2A98, 0x7DDA58),
            },
            // Dark Mind's open eye (anim 0x399, variant 0x2D), over the slit of its second form's orb (see RenderDarkMindOrb);
            [EntityIcon.DarkMindOrb] = new[] {
                P(48, 64, 32, 32, 0, 0, 0x4A2AD8, 0x81DB78),
                P(48, 96, 32, 16, 0, 0, 0x4A2AD8, 0x81DD78),
            },

            [EntityIcon.Vitality] = new[] { P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603998) },
            [EntityIcon.SoundPlayer] = new[] { P(0, 0, 16, 16, 0, 0, 0x49F758, 0x603A18) },
            [EntityIcon.Spray1] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2818, 0x7BAF38) },
            [EntityIcon.Spray2] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2818, 0x7BAFB8) },
            [EntityIcon.Spray3] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2838, 0x7BB038) },
            [EntityIcon.Spray4] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2858, 0x7BB0B8) },
            [EntityIcon.Spray5] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2878, 0x7BAF38) },
            [EntityIcon.Spray6] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2858, 0x7BAF38) },
            [EntityIcon.Spray7] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2838, 0x7BAF38) },
            [EntityIcon.Spray8] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2898, 0x7BAF38) },
            [EntityIcon.Spray9] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2898, 0x7BB138) },
            [EntityIcon.Spray10] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2858, 0x7BB1B8) },
            [EntityIcon.Spray11] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2878, 0x7BB238) },
            [EntityIcon.Spray12] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2878, 0x7BB2B8) },
            [EntityIcon.Spray13] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A2818, 0x7BB338) },
            [EntityIcon.Spray14] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28B8, 0x7BAF38) },
            [EntityIcon.Music1] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB5B8) },
            [EntityIcon.Music2] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB438) },
            [EntityIcon.Music3] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB538) },
            [EntityIcon.Music4] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB4B8) },
            [EntityIcon.Music5] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BB7B8) },
            [EntityIcon.Music6] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BB738) },
            [EntityIcon.Music7] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB3B8) },
            [EntityIcon.Music8] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BB838) },
            [EntityIcon.Music9] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BB6B8) },
            [EntityIcon.Music10] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BB638) },
            [EntityIcon.MapTutorial] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB8B8) },
            [EntityIcon.MapRainbowRoute] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB938) },
            [EntityIcon.MapMoonlightMansion] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BBA38) },
            [EntityIcon.MapCabbageCavern] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BBD38) },
            [EntityIcon.MapMustardMountain] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BB9B8) },
            [EntityIcon.MapCarrotCastle] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BBCB8) },
            [EntityIcon.MapOliveOcean] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BBB38) },
            [EntityIcon.MapPeppermintPalace] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28D8, 0x7BBAB8) },
            [EntityIcon.MapRadishRuins] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BBC38) },
            [EntityIcon.MapCandyConstellation] = new[] { P(0, 0, 16, 16, 0, 0, 0x4A28F8, 0x7BBBB8) },
        };

        // Translations of the Japanese addresses: (start, end, shift) per block, by game code;
        static readonly Dictionary<string, (int start, int end, int shift)[]> TileBlocks = new Dictionary<string, (int, int, int)[]> {
            ["B8KJ"] = new[] { (0, int.MaxValue, 0) },
            ["B8KE"] = new[] { (0x5F0000, 0x603000, 0x1C0D4), (0x603000, 0x7B0000, 0x1C0D4), (0x7B0000, 0x7BCA00, 0x2BEF4), (0x7BCA00, 0x839C00, 0x2CC74), (0x839C00, 0x843000, 0x2FE74) },
            ["B8KP"] = new[] { (0x5F0000, 0x603000, 0x26098), (0x603000, 0x7B0000, 0x25E18), (0x7B0000, 0x7BCA00, 0x36358), (0x7BCA00, 0x839C00, 0x370D8), (0x839C00, 0x843000, 0x3A2D8) },
        };

        static readonly Dictionary<string, (int start, int end, int shift)[]> PaletteBlocks = new Dictionary<string, (int, int, int)[]> {
            ["B8KJ"] = new[] { (0, int.MaxValue, 0) },
            ["B8KE"] = new[] { (0x49F000, 0x4A2920, 0x1BED4), (0x4A2920, 0x4A3000, 0x1BEF4) },
            ["B8KP"] = new[] { (0x49F000, 0x4A2920, 0x25EB8), (0x4A2920, 0x4A3000, 0x25ED8) },
        };

        static int Translate(Dictionary<string, (int start, int end, int shift)[]> blocks, string gameCode, int japanese) {
            foreach ((int start, int end, int shift) in blocks[gameCode]) {
                if (japanese >= start && japanese < end) return japanese + shift;
            }

            return -1;
        }

        /* King Golem's tower is part of his arena's tiles, and only his face is a sprite. His icon stitches both: the arena's
         * own tiles (no background) around the tower, with the face sprite where the game draws it; */
        const int KingGolemRoom = 0x2C6, KingGolemCropX = 150, KingGolemCropY = 112, KingGolemCropWidth = 106, KingGolemCropHeight = 190,
                  KingGolemFaceX = 176, KingGolemFaceY = 176;

        /* Render(); Draws an icon from the ROM as 0xAARRGGBB pixels (transparent where the game draws color 0), or null if
         * the ROM doesn't hold it where expected; */
        public static Sprite Render(RomMap map, EntityIcon icon) {
            if (icon == EntityIcon.KingGolem) return RenderKingGolem(map);
            if (icon == EntityIcon.DarkMindOrb) return RenderDarkMindOrb(map);

            return RenderSprite(map, icon);
        }

        static Sprite RenderKingGolem(RomMap map) {
            Sprite face = RenderSprite(map, EntityIcon.KingGolem);

            if (face == null || !map.Rooms.ContainsKey(KingGolemRoom)) return face;

            int[] room = map.RenderRoom(KingGolemRoom, 8, out int roomWidth, out int roomHeight, false);
            int[] pixels = new int[KingGolemCropWidth * KingGolemCropHeight];

            for (int y = 0; y < KingGolemCropHeight; y++) {
                for (int x = 0; x < KingGolemCropWidth; x++) {
                    int roomX = KingGolemCropX + x, roomY = KingGolemCropY + y;
                    int faceX = roomX - KingGolemFaceX, faceY = roomY - KingGolemFaceY;
                    int facePixel = faceX >= 0 && faceY >= 0 && faceX < face.Width && faceY < face.Height ? face.Pixels[faceY * face.Width + faceX] : 0;

                    if (facePixel != 0) pixels[y * KingGolemCropWidth + x] = facePixel;
                    else if (roomX < roomWidth && roomY < roomHeight) pixels[y * KingGolemCropWidth + x] = room[roomY * roomWidth + roomX];
                }
            }

            return new Sprite(KingGolemCropWidth, KingGolemCropHeight, pixels);
        }

        /* Dark Mind's second form, as it appears after the first one, before the chase: a 128x128 orb the game draws as an 8bpp
         * background in its room (anim 0x39B: 256 tiles from 8bpp tile 24, left half then right half, 8x16 tiles each), with its
         * open eye sprite over the slit. The orb uses the room's palette (0x396) for colors 0-18, and colors 19-30 (the aura) from
         * the room's palette animation, whose first 12 colors are the full set (the next frames darken one at a time); */
        const int DarkMindRoom = 0x396, DarkMindOrbTiles = 0x83A2F8, DarkMindOrbSize = 128;
        static readonly Dictionary<string, int> DarkMindAura = new Dictionary<string, int> {
            ["B8KJ"] = 0x39A930, ["B8KE"] = 0x39A930 + 0x1B574, ["B8KP"] = 0x39A930 + 0x2553C,
        };

        static Sprite RenderDarkMindOrb(RomMap map) {
            Sprite eye = RenderSprite(map, EntityIcon.DarkMindOrb);
            int[] colors = map.Version == null ? null : map.RoomPalette(DarkMindRoom);
            int tiles = Translate(TileBlocks, map.GameCode, DarkMindOrbTiles);

            if (colors == null || tiles < 0 || tiles + 256 * 64 > map.Rom.Length || !DarkMindAura.TryGetValue(map.GameCode, out int aura)) return eye;

            for (int i = 0; i < 12; i++) colors[19 + i] = RomMap.ToArgb(map.Rom[aura + i * 2] | (map.Rom[aura + i * 2 + 1] << 8));

            int[] pixels = new int[DarkMindOrbSize * DarkMindOrbSize];

            for (int tile = 0; tile < 256; tile++) {
                int left = tile / 128 * 64 + tile % 128 % 8 * 8, top = tile % 128 / 8 * 8;

                for (int y = 0; y < 8; y++) {
                    for (int x = 0; x < 8; x++) {
                        int color = map.Rom[tiles + tile * 64 + y * 8 + x];
                        if (color != 0) pixels[(top + y) * DarkMindOrbSize + left + x] = colors[color];
                    }
                }
            }

            if (eye != null) {
                for (int y = 0; y < eye.Height && y < DarkMindOrbSize; y++)
                    for (int x = 0; x < eye.Width && x < DarkMindOrbSize; x++)
                        if (eye.Pixels[y * eye.Width + x] != 0) pixels[y * DarkMindOrbSize + x] = eye.Pixels[y * eye.Width + x];
            }

            return new Sprite(DarkMindOrbSize, DarkMindOrbSize, pixels);
        }

        static Sprite RenderSprite(RomMap map, EntityIcon icon) {
            if (map.Version == null || !Icons.TryGetValue(icon, out IconPart[] parts)) return null;

            byte[] rom = map.Rom;
            int width = 0, height = 0;

            foreach (IconPart part in parts) {
                width = Math.Max(width, part.X + part.Width);
                height = Math.Max(height, part.Y + part.Height);
            }

            int[] pixels = new int[width * height];

            // The first piece is on top, so pieces are drawn last to first;
            for (int p = parts.Length - 1; p >= 0; p--) {
                IconPart part = parts[p];
                int palette = Translate(PaletteBlocks, map.GameCode, part.Palette), tiles = Translate(TileBlocks, map.GameCode, part.Tiles);
                int columns = part.Width / 8, tileCount = columns * (part.Height / 8);

                if (palette < 0 || tiles < 0 || palette + 32 > rom.Length || tiles + tileCount * 32 > rom.Length) return null;

                for (int t = 0; t < tileCount; t++) {
                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            int data = rom[tiles + t * 32 + y * 4 + x / 2], color = (x & 1) != 0 ? data >> 4 : data & 15;

                            if (color == 0) continue;

                            if (color == 0) continue;

                            int pieceX = t % columns * 8 + x, pieceY = t / columns * 8 + y;

                            if (part.FlipH) pieceX = part.Width - 1 - pieceX;
                            if (part.FlipV) pieceY = part.Height - 1 - pieceY;

                            pixels[(part.Y + pieceY) * width + part.X + pieceX] = RomMap.ToArgb(rom[palette + color * 2] | (rom[palette + color * 2 + 1] << 8));
                        }
                    }
                }
            }

            return new Sprite(width, height, pixels);
        }

        // Item names, by item ID (0x5E-0x64);
        static readonly (EntityIcon icon, string name)[] Items = {
            (EntityIcon.Cherry, "Cherry"), (EntityIcon.PepBrew, "Pep Brew"), (EntityIcon.Meat, "Meat"), (EntityIcon.MaximTomato, "Maxim Tomato"),
            (EntityIcon.Battery, "Battery"), (EntityIcon.OneUp, "1UP"), (EntityIcon.InvincibleCandy, "Invincible Candy"),
        };

        static readonly Dictionary<int, (EntityIcon icon, string name)> MiniBosses = new Dictionary<int, (EntityIcon, string)> {
            [0x38] = (EntityIcon.MrFrosty, "Mr. Frosty"), [0x39] = (EntityIcon.Bonkers, "Bonkers"), [0x3A] = (EntityIcon.PhanPhan, "Phan Phan"),
            [0x3B] = (EntityIcon.Batafire, "Batafire"), [0x3C] = (EntityIcon.BoxBoxer, "Box Boxer"), [0x3D] = (EntityIcon.Boxy, "Boxy"),
            [0x3E] = (EntityIcon.MasterHandMini, "Master Hand"), [0x3F] = (EntityIcon.Bombar, "Bombar"),
        };

        static readonly Dictionary<int, (EntityIcon icon, string name)> Bosses = new Dictionary<int, (EntityIcon, string)> {
            [0x45] = (EntityIcon.Kracko, "Kracko"), [0x46] = (EntityIcon.KingGolem, "King Golem"), [0x47] = (EntityIcon.MasterHand, "Master Hand & Crazy Hand"),
            [0x48] = (EntityIcon.Gobbler, "Gobbler"), [0x49] = (EntityIcon.Wiz, "Wiz"), [0x4A] = (EntityIcon.Moley, "Moley"), [0x4B] = (EntityIcon.MegaTitan, "Mega Titan"),
            [0x4E] = (EntityIcon.DarkMetaKnight, "Dark Meta Knight"), [0x4F] = (EntityIcon.DarkMind, "Dark Mind"), [0x50] = (EntityIcon.DarkMindOrb, "Dark Mind (2nd form)"),
            [0x51] = (EntityIcon.DarkMindOrb, "Dark Mind (2nd form)"), [0x52] = (EntityIcon.DarkMetaKnight, "Dark Meta Knight"),
        };

        // Classify(); The entity a room object stands for, or null if it isn't worth showing (enemies, doors, scenery...);
        public static MapEntity Classify(RoomObject obj) {
            int id = obj.Id, behavior = obj.Behavior;

            if (Bosses.TryGetValue(id, out var boss)) return new MapEntity(EntityKind.Boss, boss.icon, boss.name);
            if (MiniBosses.TryGetValue(id, out var miniBoss)) return new MapEntity(EntityKind.MiniBoss, miniBoss.icon, miniBoss.name);
            if (id >= 0x5E && id <= 0x64) return new MapEntity(EntityKind.Item, Items[id - 0x5E].icon, Items[id - 0x5E].name);

            switch (id) {
                case 0x65: return behavior < 8 ? new MapEntity(EntityKind.MirrorShard, EntityIcon.MirrorShard, "Mirror Shard") : null;
                case 0x80: case 0x81: return Chest(id == 0x81, behavior);
                case 0x84: return new MapEntity(EntityKind.WarpStar, EntityIcon.WarpStar, "Warp Star");
                case 0x86: return new MapEntity(EntityKind.BigSwitch, EntityIcon.BigSwitch, "Big Switch");
                case 0x8A: return new MapEntity(EntityKind.FuseCannon, EntityIcon.FuseCannon, "Fuse Cannon");
            }

            return null;
        }

        static readonly string[] AreaNames = {
            "Tutorial", "Rainbow Route", "Moonlight Mansion", "Cabbage Cavern", "Mustard Mountain", "Carrot Castle",
            "Olive Ocean", "Peppermint Palace", "Radish Ruins", "Candy Constellation",
        };

        /* Chest(); Chests show the chest sprite; Their ContentIcon is what they hold (an item, or the Collection Room sprite of
         * the treasure), for when chest contents are spoiled; */
        static MapEntity Chest(bool isBig, int contents) {
            EntityIcon chest = isBig ? EntityIcon.ChestBig : EntityIcon.ChestSmall;
            string size = isBig ? "Big" : "Small";

            if (contents <= 0x05) return new MapEntity(EntityKind.Treasure, chest, $"{size} Chest: {Items[contents].name}", Items[contents].icon);

            (string treasure, EntityIcon? icon) = contents switch {
                0x06 => ("Vitality", EntityIcon.Vitality),
                >= 0x0A and <= 0x13 => ($"{AreaNames[contents - 0x0A]} Map", EntityIcon.MapTutorial + (contents - 0x0A)),
                >= 0x14 and <= 0x21 => ($"Spray Paint {contents - 0x13}", EntityIcon.Spray1 + (contents - 0x14)),
                0x28 => ("Sound Player", EntityIcon.SoundPlayer),
                >= 0x29 and <= 0x32 => ($"Music Sheet {contents - 0x28}", EntityIcon.Music1 + (contents - 0x29)),
                0x63 => ("Special Treasure", (EntityIcon?)null),
                _ => ($"Treasure 0x{contents:X2}", (EntityIcon?)null),
            };

            return new MapEntity(EntityKind.Treasure, chest, $"{size} Chest: {treasure}", icon);
        }

        // EntitiesOf(); The important objects of a room, the most important first;
        public static List<MapEntity> EntitiesOf(RoomInfo room) {
            List<MapEntity> entities = new List<MapEntity>();

            foreach (RoomObject obj in room.Objects) {
                MapEntity entity = Classify(obj);
                if (entity != null) entities.Add(entity);
            }

            entities.Sort((a, b) => a.Kind.CompareTo(b.Kind));

            return entities;
        }
    }
}
