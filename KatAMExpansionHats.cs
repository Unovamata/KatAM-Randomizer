namespace KatAMRandomizer {
    /* KatAMExpansion's ability hats: the new abilities' hats are NiDL's, put on KatAM's own Kirby, who is NiDL's art (see "ROM Expansion/
     * abilities.h" and the Documentation's "Kirby and his abilities").
     * • KatAM draws Kirby as a body (gUnk_02021590[player][animation index], Normal's gUnk_0834C5D4 for these abilities) plus a hat (gUnk_08D60FDC[ability][animation index]),
     *   two sprites that animate in step. NiDL's Kirby frames have the same two layers.
     * • BodyMatches says which of NiDL's plain Kirby frames (0x00-0x75) each frame of KatAM's body animations is, and the offset to
     *   align them (matched offline by silhouette); HatFrames, which frame of the ability's block (its frames with the hat) has the
     *   same body. The hat is that frame's second layer.
     * • Each ability gets one anim, with a variant per animation index: the body's commands, with its frames swapped for the hat's; */
    internal static partial class KatAMExpansion {
        const int KirbyFrameTable = 0x74CFEC;   // NiDL USA (+ region.FrameShift);
        const int AnimationIndexes = 0xB9;      // Entries of a hat table (one per entry of the body table);
        const int HatTiles = 16;                // VRAM for a hat frame (KatAM's biggest hats use 16; NiDL's here need at most 15);

        /* KatAM body frames (anim, frame index; Normal's body table's, indexes 0x00-0x75) -> NiDL's plain Kirby frame and the offset from
         * it (x, y), matched by silhouette (mirrored frames unmirrored) and checked by eye. Anim 0x35 (Kirby facing the camera with
         * something on his head) is left out: it only matched NiDL's back views; */
        static readonly (int anim, int frame, int nidl, int dx, int dy)[] BodyMatches = {
            (0x0, 0, 0x00, 0, -3), (0x0, 1, 0x05, 0, -3), (0x0, 2, 0x01, 0, -3), (0x0, 3, 0x06, 0, -3), (0x0, 4, 0x01, 0, -3), (0x0, 5, 0x06, 0, -3),
            (0x0, 6, 0x02, 0, -3), (0x0, 7, 0x07, 0, -3), (0x0, 8, 0x02, 0, -3), (0x0, 9, 0x07, 0, -3), (0x1, 0, 0x10, 0, -3), (0x1, 1, 0x11, 0, -3),
            (0x1, 2, 0x12, 0, -3), (0x1, 3, 0x13, 0, -3), (0x1, 4, 0x14, 0, -3), (0x1, 5, 0x15, 0, -3), (0x1, 6, 0x0c, 0, -3), (0x1, 7, 0x0d, 0, -3),
            (0x1, 8, 0x0e, 0, -3), (0x1, 9, 0x0f, 0, -3), (0x1, 10, 0x1e, 0, -3), (0x2, 0, 0x16, 0, -3), (0x2, 1, 0x17, 0, -3), (0x2, 2, 0x18, 0, -3),
            (0x2, 3, 0x19, 0, -3), (0x2, 4, 0x1a, 0, -3), (0x2, 5, 0x1b, 0, -3), (0x2, 6, 0x1c, 0, -3), (0x2, 7, 0x1d, 0, -3), (0x3, 0, 0x61, 0, -3),
            (0x3, 1, 0x62, 0, -3), (0x4, 0, 0x1f, 0, -3), (0x4, 1, 0x20, 0, -3), (0x4, 2, 0x21, 0, -3), (0x4, 3, 0x22, 0, -3), (0x4, 4, 0x23, 0, -3),
            (0x4, 5, 0x24, 0, -3), (0x4, 6, 0x25, 0, -3), (0x4, 7, 0x29, 0, -3), (0x4, 8, 0x28, 0, -3), (0x4, 9, 0x26, 0, -3), (0x4, 10, 0x27, 0, -3),
            (0x4, 11, 0x2a, 0, -3), (0x4, 12, 0x2b, 0, -3), (0x4, 13, 0x2c, 0, -3), (0x4, 14, 0x2d, 0, -3), (0x4, 15, 0x2e, 0, -3),
            (0x4, 16, 0x2f, 0, -3), (0x4, 17, 0x31, 0, -3), (0x5, 0, 0x64, 0, -3), (0x5, 1, 0x65, 3, -3), (0x5, 2, 0x63, 0, -2),
            (0x6, 0, 0x39, 0, -3), (0x6, 1, 0x3a, 0, -3), (0x6, 2, 0x48, 0, -3), (0x6, 3, 0x28, -1, -3), (0x6, 4, 0x29, 0, -3),
            (0x6, 5, 0x29, -1, -3), (0x6, 6, 0x35, 0, -1), (0x6, 7, 0x36, 0, -1), (0x6, 8, 0x36, 0, -1), (0x6, 9, 0x36, 0, -1),
            (0x6, 10, 0x36, 0, -1), (0x6, 11, 0x44, 0, -3), (0x6, 12, 0x45, 0, -3), (0x6, 13, 0x46, 0, -3), (0x6, 14, 0x55, -1, -3),
            (0x6, 15, 0x48, 0, -3), (0x6, 16, 0x39, 0, -3), (0x6, 17, 0x1f, 0, -3), (0x6, 18, 0x55, -2, -4), (0x6, 19, 0x35, 0, -3),
            (0x6, 20, 0x36, 1, -1), (0x6, 21, 0x35, -1, -1), (0x6, 22, 0x1f, 0, -3), (0x6, 23, 0x56, -1, -1), (0x6, 24, 0x56, -1, 0),
            (0x6, 25, 0x2a, 0, -1), (0x6, 26, 0x34, 0, -2), (0x6, 27, 0x2a, 0, -1), (0x6, 28, 0x34, 0, -2), (0x7, 0, 0x33, 0, -3),
            (0x7, 1, 0x34, 0, -3), (0x7, 2, 0x35, 0, -3), (0x7, 3, 0x36, 0, -3), (0x7, 4, 0x37, 0, -3), (0x7, 5, 0x38, 0, -3), (0x8, 0, 0x66, 0, -3),
            (0x8, 1, 0x67, 0, -3), (0x8, 2, 0x68, 0, -3), (0x8, 3, 0x69, 0, -3), (0x9, 0, 0x54, 0, -3), (0x9, 1, 0x55, 0, -3), (0x9, 2, 0x56, 0, -3),
            (0x9, 3, 0x57, 0, -3), (0x9, 4, 0x58, 0, -3), (0x9, 5, 0x59, 0, -3), (0x9, 6, 0x5a, 0, -3), (0x9, 7, 0x5b, 0, -3), (0x9, 8, 0x5c, 0, -3),
            (0x9, 9, 0x5d, 0, -3), (0x9, 10, 0x5e, 0, -3), (0x9, 11, 0x5f, 0, -3), (0x9, 12, 0x60, 0, -3), (0xa, 0, 0x1e, 0, -2),
            (0xa, 1, 0x24, 1, -1), (0xa, 2, 0x2f, 0, -1), (0xa, 3, 0x23, 1, -2), (0xa, 4, 0x28, 1, -2), (0xa, 5, 0x31, 0, -1), (0xa, 6, 0x26, 1, -1),
            (0xa, 7, 0x5b, 1, 0), (0xa, 8, 0x28, 1, -3), (0xb, 0, 0x1e, 0, -3), (0xb, 1, 0x1e, -1, -4), (0xb, 2, 0x56, 0, 1), (0xb, 3, 0x1e, -1, -4),
            (0xb, 4, 0x26, 0, -1), (0xb, 5, 0x56, 0, 1), (0xb, 6, 0x26, 0, -1), (0xb, 7, 0x1e, 0, -3), (0xb, 8, 0x1e, -1, -4), (0xb, 9, 0x56, 0, 1),
            (0xb, 10, 0x1e, -1, -4), (0xb, 11, 0x1e, 0, -3), (0xb, 12, 0x26, 0, -1), (0xb, 13, 0x56, 0, 1), (0xb, 14, 0x26, 0, -1),
            (0xf, 0, 0x1a, 0, -4), (0xf, 1, 0x1d, 1, -5), (0xf, 2, 0x0a, 1, -5), (0xf, 3, 0x2f, 0, -3), (0xf, 4, 0x10, 1, -5), (0xf, 5, 0x24, -1, -4),
            (0xf, 6, 0x31, 0, -4), (0xf, 7, 0x39, 1, -6), (0xf, 8, 0x26, 1, -3), (0xf, 9, 0x0a, 1, -4), (0xf, 10, 0x11, 2, -4),
            (0xf, 11, 0x0a, 2, -4), (0xf, 12, 0x2f, 1, -3), (0xf, 13, 0x24, 1, -4), (0xf, 14, 0x24, 1, -4), (0xf, 15, 0x27, 2, -3),
            (0xf, 16, 0x55, -1, -3), (0xf, 17, 0x55, -1, -4), (0xf, 18, 0x23, 1, -5), (0xf, 19, 0x28, 0, -5), (0xf, 20, 0x48, 0, -6),
            (0xf, 21, 0x48, 0, -6), (0xf, 22, 0x00, 0, -6), (0xf, 23, 0x00, 0, -6), (0xf, 24, 0x68, 0, -5), (0xf, 25, 0x57, 0, -3),
            (0xf, 26, 0x69, 0, -5), (0xf, 27, 0x1f, 0, -3), (0x11, 0, 0x00, 0, -3), (0x11, 1, 0x10, 0, -3), (0x11, 2, 0x10, -1, -5),
            (0x11, 3, 0x11, 0, -4), (0x11, 4, 0x69, -1, -4), (0x11, 5, 0x12, 0, -4), (0x11, 6, 0x0f, 0, -3), (0x11, 7, 0x13, 0, -4),
            (0x11, 8, 0x14, 0, -3), (0x11, 9, 0x15, 0, -3), (0x11, 10, 0x68, -1, -4), (0x11, 11, 0x0c, 0, -4), (0x11, 12, 0x0c, 0, -4),
            (0x11, 13, 0x66, 0, -4), (0x11, 14, 0x14, 0, -3), (0x11, 15, 0x0e, 0, -4), (0x11, 16, 0x0f, 0, -3), (0x11, 17, 0x21, 1, -2),
            (0x11, 18, 0x25, 0, -2), (0x11, 19, 0x2a, 1, -1), (0x11, 20, 0x28, 0, -3), (0x16, 0, 0x66, 0, -3), (0x16, 1, 0x67, 0, -3),
            (0x16, 2, 0x68, 0, -3), (0x16, 3, 0x69, 0, -3), (0x16, 4, 0x36, 0, -3), (0x16, 5, 0x5b, 0, -2), (0x16, 6, 0x15, 0, -5),
            (0x18, 0, 0x44, 0, -3), (0x18, 1, 0x45, 0, -3), (0x18, 2, 0x46, 0, -3), (0x18, 3, 0x55, -1, -3), (0x18, 4, 0x48, 0, -3),
            (0x18, 5, 0x36, 0, -3), (0x18, 6, 0x5b, 0, -2), (0x18, 7, 0x15, 0, -5), (0x1c, 0, 0x39, 0, -4), (0x1c, 1, 0x25, -1, -1),
            (0x1c, 2, 0x56, -1, 0), (0x1c, 3, 0x56, -1, 0), (0x1c, 4, 0x23, 1, -2), (0x1c, 5, 0x23, 1, -2), (0x1c, 6, 0x0e, 0, -4),
            (0x1c, 7, 0x68, -1, -2), (0x1c, 8, 0x1d, 0, -4), (0x1c, 9, 0x5d, 0, -4), (0x1c, 10, 0x5c, 1, -2), (0x1c, 11, 0x1f, 0, -3),
            (0x1e, 0, 0x66, 0, -3), (0x1e, 1, 0x67, 0, -3), (0x1e, 2, 0x68, 0, -3), (0x1e, 3, 0x69, 0, -3), (0x1e, 4, 0x0a, 0, -3),
            (0x2c, 0, 0x1e, 0, -3), (0x2c, 1, 0x1e, -1, -4), (0x2c, 2, 0x56, 0, 1), (0x2c, 3, 0x1e, -1, -4), (0x2c, 4, 0x1e, 0, -3),
            (0x2c, 5, 0x26, 0, -1), (0x2c, 6, 0x56, 0, 1), (0x2c, 7, 0x26, 0, -1), (0x2c, 8, 0x1e, 0, -3), (0x2c, 9, 0x1e, -1, -4),
            (0x2c, 10, 0x56, 0, 1), (0x2c, 11, 0x1e, -1, -4), (0x2c, 12, 0x1e, 0, -3), (0x2c, 13, 0x26, 0, -1), (0x2c, 14, 0x56, 0, 1),
            (0x2c, 15, 0x26, 0, -1), (0x31, 0, 0x5d, -3, -2), (0x31, 1, 0x55, -2, -1), (0x31, 2, 0x5b, 0, 1), (0x31, 3, 0x29, 0, -2),
            (0x31, 4, 0x29, 0, -2), (0x36, 0, 0x25, -1, -2), (0x61, 0, 0x48, 0, -3), (0x61, 1, 0x48, -2, -4), (0x61, 2, 0x29, -3, -3),
            (0x61, 3, 0x29, -2, -3), (0x61, 4, 0x57, -3, -2), (0x61, 5, 0x0d, 1, -3), (0x72, 0, 0x14, 0, -6), (0x72, 1, 0x14, 0, -6),
            (0x72, 2, 0x14, 0, -6), (0x72, 3, 0x1c, -1, -6), (0x72, 5, 0x73, 3, -4), (0x72, 6, 0x73, 3, -5), (0x72, 7, 0x1c, 0, 1),
            (0x72, 8, 0x1b, 0, -4), (0x72, 9, 0x14, 0, -6), (0x72, 10, 0x0f, 0, -6), (0x72, 11, 0x1c, -1, -6), (0x72, 12, 0x0a, 1, -6),
            (0x72, 13, 0x73, 3, -4), (0x72, 14, 0x73, 3, -5), (0x72, 15, 0x1c, 0, 1), (0x72, 16, 0x14, 0, 1), (0x8d, 0, 0x20, 1, 0),
            (0x8d, 1, 0x48, 1, -4), (0x8d, 2, 0x0d, 1, -3), (0x8d, 3, 0x0d, 1, -3), (0x8d, 4, 0x5c, 0, -2), (0xca, 0, 0x00, -1, -3),
            (0xca, 1, 0x25, -1, 0), (0xca, 2, 0x25, 0, -1), (0xca, 3, 0x19, 1, -3), (0xca, 4, 0x5c, 1, -2), (0xca, 5, 0x15, 1, -6),
            (0xca, 6, 0x69, 0, -2), (0x17e, 5, 0x48, 0, -3), (0x17e, 6, 0x25, -4, -3), (0x17e, 7, 0x25, -4, -2), (0x17e, 8, 0x26, -4, -4),
            (0x17e, 9, 0x26, -4, -5), (0x17e, 10, 0x29, -1, -4), (0x17e, 11, 0x29, -1, -6), (0x17e, 12, 0x26, -3, -3), (0x17e, 13, 0x11, -2, -5),
            (0x17e, 14, 0x25, -1, -4), (0x17e, 15, 0x5a, 3, -2), (0x17e, 16, 0x6a, 4, -2), (0x17e, 17, 0x00, 3, -3), (0x17e, 18, 0x29, -1, -4),
            (0x17e, 19, 0x0c, 1, -4), (0x17e, 20, 0x20, 3, -2), (0x17e, 21, 0x48, 4, -4), (0x17e, 22, 0x22, 3, -1), (0x17e, 23, 0x1a, 4, -2),
            (0x17e, 24, 0x23, 4, -2), (0x17e, 25, 0x48, 4, -3),
        };

        // Per new ability: NiDL plain Kirby frame (0x00-0x75) -> the frame with the same body in the ability's block (-1: none); null: no hat;
        static readonly int[][] HatFrames = {
            new int[] {     // Needle;
                0x8f8, 0x8f9, 0x8fa, 0x8fb, 0x8fc, 0x8fd, 0x8fe, 0x8ff, 0x900, 0x901, 0x902, 0x902,
                0x903, 0x904, 0x905, 0x906, 0x955, 0x956, 0x909, 0x90a, 0x90b, 0x90c, 0x90d, 0x90e,
                0x90f, 0x910, 0x911, 0x912, 0x913, 0x914, 0x915, 0x916, 0x917, 0x918, 0x919, 0x91a,
                0x91b, 0x91c, 0x91d, 0x91e, 0x91f, 0x920, 0x921, 0x922, 0x923, 0x924, 0x925, 0x926,
                0x91b, 0x928, 0x91c, 0x92a, 0x92b, 0x92c, 0x92d, 0x92e, 0x92f, 0x930, 0x931, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, 0x922, 0x8fd, 0x8fd, -1,
                0x930, 0x930, 0x930, 0x931, 0x932, 0x933, 0x934, -1, -1, -1, -1, 0x90c,
                0x935, 0x936, 0x937, 0x938, 0x939, 0x93a, 0x93b, 0x93c, 0x93d, 0x93e, 0x93f, 0x940,
                0x941, 0x942, 0x943, 0x944, 0x945, 0x946, 0x947, 0x948, 0x949, 0x94a, 0x94b, 0x94c,
                0x94c, 0x94e, 0x94f, 0x950, 0x951, 0x952, 0x953, 0x954, 0x955, 0x956,
            },
            new int[] {     // Freeze;
                0xa10, 0xa11, 0xa12, 0xa13, 0xa14, 0xa15, 0xa16, 0xa17, 0xa18, 0xa19, 0xa1a, 0xa1a,
                0xa1b, 0xa1c, 0xa1d, 0xa22, 0xa1f, 0xa20, 0xa21, 0xa22, 0xa23, 0xa24, 0xa25, 0xa26,
                0xa27, 0xa28, 0xa29, 0xa2a, 0xa2b, 0xa2c, 0xa2d, 0xa2e, 0xa2f, 0xa30, 0xa31, 0xa32,
                0xa33, 0xa34, 0xa35, 0xa36, 0xa38, 0xa37, 0xa39, 0xa3a, 0xa3b, 0xa3c, 0xa3d, 0xa3e,
                0xa33, 0xa40, 0xa34, 0xa42, 0xa43, 0xa44, 0xa45, 0xa46, 0xa47, 0xa48, 0xa49, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, 0xa15, 0xa15, 0xa49, 0xa4b,
                0xa48, 0xa48, 0xa48, 0xa49, 0xa4a, 0xa4b, 0xa4c, -1, -1, 0xa4b, 0xa4b, 0xa4b,
                0xa4d, 0xa4e, 0xa4f, 0xa50, 0xa51, 0xa52, 0xa53, 0xa54, 0xa55, 0xa56, 0xa57, 0xa58,
                0xa59, 0xa5a, 0xa5b, 0xa5c, 0xa5d, 0xa5e, 0xa5f, 0xa60, 0xa61, 0xa62, 0xa63, 0xa64,
                0xa65, 0xa66, 0xa67, 0xa68, 0xa69, 0xa6a, 0xa6b, 0xa6c, 0xa1f, 0xa20,
            },
            null,           // Light: no hat;
            new int[] {     // Ball;
                0xc54, 0xc55, 0xc56, 0xc57, 0xc58, 0xc59, 0xc5a, 0xc5b, 0xc5c, 0xc5d, 0xc5e, 0xc5e,
                0xc5f, 0xc60, 0xc61, 0xc62, 0xc63, 0xc64, 0xc65, 0xc66, 0xc67, 0xc68, 0xc69, 0xc6a,
                0xc6b, 0xc6c, 0xc6d, 0xc6e, 0xc6f, 0xc70, 0xc71, 0xc72, 0xc73, 0xc74, 0xc75, 0xc76,
                0xc77, 0xc78, 0xc79, 0xc7a, 0xc7b, 0xc7c, 0xc7d, 0xc7e, 0xc7f, 0xc80, 0xc81, 0xc82,
                0xc77, 0xc84, 0xc78, 0xc86, 0xc87, 0xc88, 0xc89, 0xc8a, 0xc8b, 0xc8c, 0xc8d, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, 0xc8d, 0xc59, 0xc8d, -1,
                0xc8c, 0xc8c, 0xc8c, 0xc8d, 0xc8e, 0xc8f, 0xc90, -1, -1, -1, -1, 0xcb5,
                0xc91, 0xc92, 0xc93, 0xc94, 0xc95, 0xc96, 0xc97, 0xc98, 0xc99, 0xc9a, 0xc9b, 0xc9c,
                0xc9d, 0xc9e, 0xc9f, 0xca0, 0xca1, 0xca2, 0xca3, 0xca4, 0xca5, 0xca6, 0xca7, 0xca8,
                0xca9, 0xcaa, 0xcab, 0xcac, 0xcad, 0xcae, 0xcaf, 0xcb0, 0xc63, 0xc64,
            },
            new int[] {     // Hi-Jump;
                0xa8f, 0xa93, 0xa97, 0xa9b, 0xa9f, 0xaa3, 0xaa7, 0xaab, 0xaaf, 0xab3, 0xab7, 0xab7,
                0xabb, 0xabd, 0xabe, 0xabf, 0xac0, 0xac1, 0xac3, 0xac4, 0xac5, 0xac6, 0xac7, 0xac8,
                0xac9, 0xaca, 0xacb, 0xacc, 0xacd, 0xace, 0xacf, 0xad0, 0xad2, 0xad3, 0xad4, 0xad5,
                0xad6, 0xad7, 0xad8, 0xad9, 0xadb, 0xada, 0xadc, 0xade, 0xadf, 0xae0, 0xae1, 0xae2,
                0xad6, 0xae4, 0xad7, 0xae6, 0xae7, 0xae8, 0xae9, 0xaea, 0xaeb, 0xaec, 0xaed, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, 0xade, 0xaa5, 0xade, -1,
                0xb2b, 0xaec, 0xaec, 0xaed, 0xaee, 0xaef, 0xaf0, -1, -1, -1, -1, 0xaf7,
                0xaf1, 0xaf2, 0xaf3, 0xaf4, 0xaf5, 0xaf6, 0xaf7, 0xaf8, 0xaf9, 0xafa, 0xafb, 0xafc,
                0xafd, 0xafe, 0xb00, 0xb01, 0xb02, 0xb03, 0xb04, 0xb05, 0xb06, 0xb07, 0xb08, 0xb09,
                0xb0a, 0xb0b, 0xb0c, 0xb0d, 0xb0e, 0xb0f, 0xb10, 0xb11, 0xac0, 0xac1,
            },
            null,           // Mike: no hat;
            null,           // Backdrop: no hat (it has Throw's body, headband included, see BuildBackdropPalette);
        };

        // Per new ability, which pixels are its hat: null, the second layer's; else the first layer's of these colors (Ball's cap is drawn with Kirby);
        static readonly int[][] HatColors = { null, null, null, new[] { 12, 13, 14 }, null, null, null };

        record KirbyFrame(List<Piece> Pieces, byte[] Palette1, byte[] Tiles1, byte[] Palette2, byte[] Tiles2);

        /* ReadKirbyFrame(); A frame of NiDL's Kirby: { OAM records | 1 if two layers, palette, tiles, [palette 2, tiles 2] }. Palettes are a
         * byte count and the colors; tiles are rows of 32 tiles (a byte count and the data each) until 0xFFFF. The second layer's tiles
         * are tile 0x40 on. Pieces are the 8-byte records (x unflipped, as KatAM's Kirby faces); */
        static KirbyFrame ReadKirbyFrame(byte[] nightmare, NightmareRegion region, int frame) {
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + frame * 4) - 0x08000000;
            int first = ReadInt(nightmare, entry), address = (first & ~1) - 0x08000000;
            bool twoLayers = (first & 1) != 0;
            List<Piece> pieces = new List<Piece>();

            for (int i = 0; i < 64; i++, address += 8) {
                int shape = nightmare[address + 1] >> 6, size = nightmare[address + 3] >> 6;

                pieces.Add(new Piece((sbyte)nightmare[address + 2], (sbyte)nightmare[address], Shapes[shape, size, 0], Shapes[shape, size, 1], shape, size,
                    (nightmare[address + 3] >> 4) & 1, (nightmare[address + 3] >> 5) & 1, nightmare[address + 6] | (nightmare[address + 7] & 3) << 8));
                if ((nightmare[address + 1] & 0x10) != 0) break;
            }

            return new KirbyFrame(pieces, KirbyPalette(nightmare, ReadInt(nightmare, entry + 4)), KirbyTiles(nightmare, ReadInt(nightmare, entry + 8)),
                twoLayers ? KirbyPalette(nightmare, ReadInt(nightmare, entry + 12)) : null, twoLayers ? KirbyTiles(nightmare, ReadInt(nightmare, entry + 16)) : null);
        }

        static byte[] KirbyPalette(byte[] nightmare, int pointer) {
            if (pointer == 0) return null;
            int address = pointer - 0x08000000;
            byte[] palette = new byte[32];

            Array.Copy(nightmare, address + 2, palette, 0, Math.Min(32, ReadShort(nightmare, address)));
            return palette;
        }

        static byte[] KirbyTiles(byte[] nightmare, int pointer) {
            byte[] tiles = new byte[0x1000];
            if (pointer == 0) return tiles;

            for (int address = pointer - 0x08000000, row = 0; ReadShort(nightmare, address) != 0xFFFF && row < 4; row++) {
                int length = Math.Min(ReadShort(nightmare, address), 0x400);
                Array.Copy(nightmare, address + 2, tiles, row * 0x400, length);
                address += 2 + ReadShort(nightmare, address);
            }
            return tiles;
        }

        /* HatFrame(); KatAM frame (tiles in 1D order, OAM entries, attributes) of a NiDL frame's hat pixels (see HatColors), moved by (dx, dy):
         * drawn on a canvas and cut into 8x8 pieces (CutFrame); NiDL's Kirby is always pink, so its hats (Needle's) draw parts of his
         * body in his colors (kirbyColors: plain Kirby's 2-11), which KatAM's hat palette would keep pink on every Kirby: they're left to
         * the body; */
        static (byte[] tiles, ushort[] oams, short[] attributes) HatFrame(KirbyFrame frame, int dx, int dy, int[] colors, (int flags, int w, int h, int ox, int oy) body,
                                                                          HashSet<int> kirbyColors) {
            const int Size = 128, Center = 64;
            int[,] canvas = new int[Size, Size];

            foreach (Piece piece in frame.Pieces) {
                bool second = piece.Tile >= 0x40;
                byte[] tiles = second ? frame.Tiles2 : frame.Tiles1;
                if (tiles == null || second != (colors == null)) continue;

                for (int py = 0; py < piece.Height; py++) {
                    for (int px = 0; px < piece.Width; px++) {
                        int tile = piece.Tile - (second ? 0x40 : 0) + py / 8 * 32 + px / 8;
                        int value = tiles[tile * 32 + py % 8 * 4 + px % 8 / 2] >> (px % 2 * 4) & 0xF;
                        if (value == 0 || (colors != null && !colors.Contains(value))) continue;
                        // (the second layer's pixels in plain Kirby's colors are his body, which KatAM's body frame draws in the player's colors);
                        if (second && kirbyColors.Contains(ReadShort(frame.Palette2, value * 2))) continue;

                        int x = Center + piece.X + dx + (piece.HFlip != 0 ? piece.Width - 1 - px : px);
                        int y = Center + piece.Y + dy + (piece.VFlip != 0 ? piece.Height - 1 - py : py);
                        if (x < 0 || y < 0 || x >= Size || y >= Size) continue;
                        canvas[x, y] = value;
                    }
                }
            }
            /* A mirrored body frame (its attributes' oamStart | 0x4000 horizontally, | 0x8000 vertically) is drawn mirrored within its box,
             * which needn't be centered on the origin; BodyMatches has its unmirrored match, so the hat is mirrored the same way; */
            if ((body.flags & 0xC000) != 0) {
                int[,] mirrored = new int[Size, Size];
                for (int y = 0; y < Size; y++)
                    for (int x = 0; x < Size; x++) {
                        if (canvas[x, y] == 0) continue;
                        int mx = (body.flags & 0x4000) != 0 ? Center + body.w - 1 - 2 * body.ox - (x - Center) : x;
                        int my = (body.flags & 0x8000) != 0 ? Center + body.h - 1 - 2 * body.oy - (y - Center) : y;
                        if (mx >= 0 && my >= 0 && mx < Size && my < Size) mirrored[mx, my] = canvas[x, y];
                    }
                canvas = mirrored;
            }
            return CutFrame(canvas, HatTiles);
        }

        /* BuildHats(); Each new ability's hat anim (added to the anim lists, after the enemies'), and its hat table (an AnimInfo per animation
         * index: the anim and variant i) for gUnk_08D60FDC, with its palette's offset and which of its colors the hat uses (`hatPalettes`,
         * `paletteUsed`, for BuildMotions). Light's hat is empty (its motions draw on the hat sprite, see BuildMotions).
         * `bodyTable` is gUnk_0834C5D4 (Normal's, which they use; KatAM's own hat tables have up to 118 entries, these have one per
         * body table entry, the frames without a match hatless); */
        static int[] BuildHats(byte[] rom, byte[] nightmare, NightmareRegion region, int bodyTable, int anims, int attrTable, int tilesBase, int paletteBase,
                               ref int cursor, int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams,
                               int[] hatPalettes, bool[][] paletteUsed) {
            int[] hatTables = new int[NewAbilities];
            Dictionary<(int, int), (int nidl, int dx, int dy)> matches = BodyMatches.ToDictionary(m => (m.anim, m.frame), m => (m.nidl, m.dx, m.dy));
            HashSet<int> kirbyColors = PlainKirby(nightmare, region).Skip(2).Take(10).Select(c => (int)c).ToHashSet();

            for (int k = 0; k < NewAbilities; k++) {
                int animId = firstAnimId + animVariants.Count;

                // Frames, one per (ability frame, offset); frame 0 is empty, for body frames without a hat;
                Dictionary<(int, int, int, (int, int, int, int, int)), int> frameIndexes = new Dictionary<(int, int, int, (int, int, int, int, int)), int>();
                List<(byte[] tiles, ushort[] oams, short[] attributes)> frames = new List<(byte[], ushort[], short[])> { (new byte[0], new ushort[0], new short[] { 0, 0, 0, 0, 0, 0 }) };
                byte[] palette = null;

                int HatFrameIndex(int bodyAnim, int bodyFrame) {
                    if (HatFrames[k] == null || !matches.TryGetValue((bodyAnim, bodyFrame), out (int nidl, int dx, int dy) m) || m.nidl >= HatFrames[k].Length || HatFrames[k][m.nidl] < 0) return 0;
                    int attributes = ReadInt(rom, attrTable + bodyAnim * 4) - 0x08000000 + bodyFrame * 12;
                    var body = (ReadShort(rom, attributes) & 0xC000, (int)(short)ReadShort(rom, attributes + 4), (int)(short)ReadShort(rom, attributes + 6),
                                (int)(short)ReadShort(rom, attributes + 8), (int)(short)ReadShort(rom, attributes + 10));
                    (int, int, int, (int, int, int, int, int)) key = (HatFrames[k][m.nidl], m.dx, m.dy, body.Item1 == 0 ? default : body);
                    if (frameIndexes.TryGetValue(key, out int index)) return index;

                    KirbyFrame frame = ReadKirbyFrame(nightmare, region, HatFrames[k][m.nidl]);
                    palette ??= HatColors[k] == null ? frame.Palette2 : frame.Palette1;
                    frames.Add(HatFrame(frame, m.dx, m.dy, HatColors[k], body, kirbyColors));
                    return frameIndexes[key] = frames.Count - 1;
                }

                // Commands per animation index: the body's, with its frames swapped for the hat's (GetTiles + ShowFrame), JumpBacks kept;
                Dictionary<(int, int), List<(int command, int[] words)>> bodies = new Dictionary<(int, int), List<(int, int[])>>();
                List<int>[] variants = new List<int>[AnimationIndexes];
                for (int i = 0; i < AnimationIndexes; i++) {
                    int bodyAnim = ReadShort(rom, bodyTable + i * 4), bodyVariant = rom[bodyTable + i * 4 + 2];
                    variants[i] = HatCommands(rom, anims, bodyAnim, bodyVariant, HatFrameIndex);
                }

                // Data: tiles (aligned to the tiles base), palette, attributes, OAM, commands;
                int tilesAt = Allocate(ref cursor, Math.Max(32, frames.Sum(f => f.tiles.Length)), 32, tilesBase);
                int paletteAt = Allocate(ref cursor, 32, 32, paletteBase);
                (palette ?? new byte[32]).CopyTo(rom, paletteAt);
                hatPalettes[k] = paletteAt;
                paletteUsed[k] = new bool[16];
                foreach (var f in frames)
                    foreach (byte pair in f.tiles) { paletteUsed[k][pair & 0xF] = true; paletteUsed[k][pair >> 4] = true; }
                paletteUsed[k][0] = true;
                int tileIndex = (tilesAt - tilesBase) / 32, paletteIndex = (paletteAt - paletteBase) / 32;
                int attributesAt = Allocate(ref cursor, frames.Count * 12, 4), oamsAt = Allocate(ref cursor, Math.Max(2, frames.Sum(f => f.oams.Length) * 2), 4);
                int[] frameTile = new int[frames.Count];

                for (int i = 0, tile = 0, oam = 0; i < frames.Count; i++) {
                    (byte[] tiles, ushort[] oams, short[] attributes) = frames[i];
                    tiles.CopyTo(rom, tilesAt + tile * 32);
                    WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), attributes[1], attributes[2], attributes[3], attributes[4], attributes[5]);
                    for (int o = 0; o < oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, oams[o]);
                    frameTile[i] = tile;
                    tile += tiles.Length / 32;
                    oam += oams.Length;
                }

                int variantsAt = Allocate(ref cursor, AnimationIndexes * 4, 4);
                for (int i = 0; i < AnimationIndexes; i++) {
                    List<int> words = new List<int> { -2, paletteIndex, 16 };
                    for (int w = 0; w < variants[i].Count; w++) {
                        // Frames are written as (-1000 - frame): GetTiles of that frame then ShowFrame;
                        words.Add(variants[i][w]);
                    }
                    words = ResolveHatFrames(words, frames, frameTile, tileIndex);
                    int at = Allocate(ref cursor, words.Count * 4, 4);
                    for (int w = 0; w < words.Count; w++) WriteInt(rom, at + w * 4, words[w]);
                    WriteInt(rom, variantsAt + i * 4, at + 0x08000000);
                }

                animVariants.Add(variantsAt);
                animAttributes.Add(attributesAt);
                animOams.Add(oamsAt);

                hatTables[k] = Allocate(ref cursor, AnimationIndexes * 4, 4);
                for (int i = 0; i < AnimationIndexes; i++) {
                    WriteShort(rom, hatTables[k] + i * 4, animId);
                    rom[hatTables[k] + i * 4 + 2] = (byte)i;
                }
            }

            return hatTables;
        }

        /* HatCommands(); A body variant's commands for the hat: GetTiles and GetPalette dropped, ShowFrame(delay, frame) as (delay, -1000 - hat
         * frame) for ResolveHatFrames, JumpBack's offset fixed for the new positions, End kept; */
        static List<int> HatCommands(byte[] rom, int anims, int anim, int variant, Func<int, int, int> hatFrame) {
            int address = ReadInt(rom, ReadInt(rom, anims + anim * 4) - 0x08000000 + variant * 4) - 0x08000000;
            List<int> words = new List<int>();
            Dictionary<int, int> positions = new Dictionary<int, int>();   // Body word index -> words index (before the GetPalette);
            int[] lengths = { 0, 3, 3, 2, 1, 2, 3, 2, 3, 2, 4, 2, 2 };

            for (int position = 0, n = 0; n < 256; n++) {
                int command = ReadInt(rom, address + position * 4);
                positions[position] = words.Count;

                if (command >= 0) {
                    words.Add(command);
                    words.Add(-1000 - hatFrame(anim, ReadInt(rom, address + position * 4 + 4)));
                    position += 2;
                } else if (command == -3) {
                    int target = position - ReadInt(rom, address + position * 4 + 4);
                    words.Add(-3);
                    words.Add(positions.TryGetValue(target, out int to) ? words.Count - 1 - to : 0);
                    break;
                } else if (command == -4 || command < -12) {
                    words.Add(-4);
                    break;
                } else position += lengths[-command];
            }
            if (words.Count == 0 || (words[^2] != -3 && words[^1] != -4)) words.Add(-4);
            return words;
        }

        /* ResolveHatFrames(); Turns HatCommands' (delay, -1000 - frame) into GetTiles (when the frame has tiles) and ShowFrame, shifting
         * JumpBack offsets by the words added in between; */
        static List<int> ResolveHatFrames(List<int> words, List<(byte[] tiles, ushort[] oams, short[] attributes)> frames, int[] frameTile, int tileIndex) {
            List<int> output = new List<int>();
            Dictionary<int, int> positions = new Dictionary<int, int>();

            for (int i = 0; i < words.Count;) {
                positions[i] = output.Count;
                int command = words[i];

                if (command >= 0) {
                    int frame = -1000 - words[i + 1];
                    if (frames[frame].tiles.Length > 0) output.AddRange(new[] { -1, tileIndex + frameTile[frame], frames[frame].tiles.Length / 32 });
                    output.AddRange(new[] { command, frame });
                    i += 2;
                } else if (command == -3) {
                    int target = i - words[i + 1];
                    output.Add(-3);
                    output.Add(output.Count - 1 - positions.GetValueOrDefault(target, 0));
                    i += 2;
                } else if (command == -2 || command == -1) {
                    output.AddRange(words.GetRange(i, 3));
                    i += 3;
                } else {
                    output.Add(command);
                    i++;
                }
            }
            return output;
        }
    }
}
