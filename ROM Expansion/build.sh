#!/bin/bash
# Builds the NiDL enemies (nidl.c, nidl_data.c, enemies/*.c) into the code KatAMExpansion.cs embeds, linked at 0x09000000,
# once per version of KatAM, and writes ../KatAMExpansionCode.cs with each version's code, the symbols KatAMExpansion.cs needs
# and what it patches;
# Needs the KatAM decompilation (https://github.com/jiangzhengwenjz/katam) built for the USA version (katam.elf), with agbcc
# installed in it, and arm-none-eabi binutils. Usage: ./build.sh /path/to/katam;
# The USA build links against katam.elf; the others against versions/<version>.txt, the same symbols' addresses in that version
# (find_symbols.py finds them in a ROM of it: rerun it when the code starts using a KatAM symbol they don't have);
set -e
D=${1:?path to the katam decompilation}
BASE=0x09000000
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

objects=()
for c in nidl.c nidl_miniboss.c nidl_boss.c finale.c ending.c nidl_data.c worldmap.c rooms.c title.c calm.c chest.c skip.c unlock.c mirra.c warpstar.c abilities.c abilities/*.c enemies/*.c minibosses/*.c bosses/*.c; do
    o="$OUT/$(basename "${c%.c}").o"
    cpp -I "$D/tools/agbcc/include" -I "$D/tools/agbcc" -nostdinc -iquote "$D/include" -iquote . -DREVISION=0 -DENGLISH \
        -Wno-trigraphs -DMODERN=0 "$c" | "$D/tools/agbcc/bin/agbcc" -mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm -o "$OUT/a.s" -
    arm-none-eabi-as -mcpu=arm7tdmi -o "$o" "$OUT/a.s"
    objects+=("$o")
done

# The KatAM symbols the code uses;
comm -23 <(arm-none-eabi-nm -u "${objects[@]}" | awk '{print $2}' | sort -u) \
         <(arm-none-eabi-nm --defined-only "${objects[@]}" | awk '{print $3}' | sort -u) > "$OUT/external.txt"

link() { # link <name> <symbols file> [more objects];
    arm-none-eabi-ld -Ttext=$BASE --just-symbols="$2" -e CreateNidlEnemy -o "$OUT/$1.elf" "${objects[@]}" "${@:3}"
    arm-none-eabi-objcopy -O binary "$OUT/$1.elf" "$OUT/$1.bin"
}

# The decompilation's static functions the code uses (--just-symbols only gives the global ones), as an object;
arm-none-eabi-nm "$D/katam.elf" | awk 'NR == FNR { used[$1] = 1; next } $2 == "t" && used[$3] { print ".global " $3; print ".thumb_set " $3 ", 0x" $1 }' \
    "$OUT/external.txt" - > "$OUT/usa_locals.s"
arm-none-eabi-as -mcpu=arm7tdmi -o "$OUT/usa_locals.o" "$OUT/usa_locals.s"
link usa "$D/katam.elf" "$OUT/usa_locals.o"
for v in europe japan; do
    # Symbols of that version, as an object: functions are Thumb (.thumb_set), the rest plain addresses;
    python3 - "$OUT/external.txt" "versions/$v.txt" "$D/katam.elf" > "$OUT/$v.s" <<'PY'
import subprocess, sys
table = dict(line.split()[:2] for line in open(sys.argv[2]) if line.strip() and not line.startswith('#'))
kinds = {p[2]: p[1] for p in (l.split() for l in subprocess.run(['arm-none-eabi-nm', sys.argv[3]], capture_output=True, text=True).stdout.splitlines()) if len(p) == 3}
missing = [n.strip() for n in open(sys.argv[1]) if n.strip() not in table]
if missing: sys.exit(f'{sys.argv[2]} lacks {" ".join(missing)}: run find_symbols.py');
for n in (n.strip() for n in open(sys.argv[1])):
    print(f'.global {n}')
    print(f'.thumb_set {n}, {int(table[n], 16) & ~1:#x}' if kinds.get(n) in ('t', 'T') else f'.set {n}, {table[n]}')
PY
    arm-none-eabi-as -mcpu=arm7tdmi -o "$OUT/$v.o" "$OUT/$v.s"
    link $v "$OUT/$v.o"
done

# KatAMExpansionCode.cs: per version, the code, the offsets (from 0x09000000) of the symbols KatAMExpansion.cs uses and what it
# patches (versions/<version>.txt; USA's are the decompilation's);
python3 - "$OUT" > ../KatAMExpansionCode.cs <<'PY'
import subprocess, sys
out = sys.argv[1]
versions = [('usa', 'USA', 'B8KE', 0), ('europe', 'Europe', 'B8KP', 0), ('japan', 'Japan', 'B8KJ', 1)]
names = ['CreateNidlEnemy', 'NidlStart', 'NidlCopyName', 'gNidlData']
patched = ['SpriteTables', 'AnimCount', 'SpawnTable', 'gUnk_08351648', 'ObjectParameters2', 'gUnk_08D60EE4', 'HitFlashBranch', 'NameSite1', 'NameSite2']
# The abilities' (see abilities.h): code symbols, and the KatAM tables and functions it relocates or hooks;
codeNames = ['gNidlWorldMapDoors', 'NidlWorldMapUnlockSave', 'NidlWorldMapGetCoorFromRoom', 'NidlWorldMapDrawKirbys', 'NidlWorldMapUnlock0', 'NidlWorldMapUnlock1', 'NidlWorldMapUnlock2', 'NidlWorldMapUnlock3', 'NidlWorldMapUnlock4', 'NidlWorldMapUnlock5', 'NidlWorldMapUnlock6', 'NidlWorldMapUnlock7', 'NidlGetKirbyRoomFlagIndex', 'gNidlRoomFlagIndexOriginal', 'gNidlHubRoom', 'gNidlAbilityTables', 'gNidlAbilityData', 'NidlAbilityPortrait', 'NidlAbilityHudTimer', 'NidlDisplaySprite', 'NidlSwallow', 'NidlKirbyHit', 'gNidlKirbyHitOriginal', 'CreateNidlMiniboss', 'NidlMinibossStart',
             # The bosses (nidl_boss.c);
             'CreateNidlBoss', 'NidlBossStart', 'NidlBossDefeat', 'gNidlShardExits', 'NidlShardExit',
             # The finale (finale.c);
             'gNidlFinale', 'gNidlCloudSpeeds', 'gNidlNightmareFight', 'gNidlMetaKnightFight', 'gNidlShineBright', 'gNidlKingDedede', 'gNidlPowerOrbColors',
             # The NiDL title screen (title.c);
             'gNidlTitle', 'CreateNidlTitle',
             # Reduce flashing (calm.c);
             'gNidlCalm', 'gNidlCalmTrampolines', 'NidlCalmUpdateScreenDma', 'NidlCalmUpdateScreenCpuSet',
             # Mirror shards in chests (chest.c);
             'gNidlChest', 'gNidlChestTrampolines', 'NidlChestOpen', 'gNidlShard', 'NidlCreateShard', 'NidlShardDone',
             # Skip the tutorial (skip.c);
             'gNidlSkip', 'NidlSkipClear', 'NidlSkipStart', 'gNidlSkipTrampolines',
             # Unlock the ability room (unlock.c);
             'gNidlUnlock', 'NidlHubMirrorInit',
             # Mirras fall, and open their door where they are (mirra.c);
             'gNidlMirra', 'NidlCreateMirra', 'NidlMirraOpen',
             # The ported warp stars' rides (warpstar.c);
             'gNidlRide', 'gNidlRideTrampolines', 'NidlRideRev', 'NidlRideRoomChange', 'NidlRideStep', 'NidlRideDraw', 'NidlRideStar',
             # Bigger rooms (rooms.c): the replacements;
             'BigFillLevelInfo', 'BigGetCollisionTile', 'BigSub_08002434', 'BigSub_08002470', 'BigSub_080024F0', 'BigSub_0800255C', 'BigSub_080025AC', 'BigSub_08002624', 'BigSub_080026A8', 'BigSub_08001408', 'BigSub_080015A8',
             'gNidlRoomTops', 'NidlNumChestsCollected', 'gNidlSave', 'gNidlSaveTrampolines', 'NidlWriteSave', 'NidlVerifySave', 'NidlClearSave', 'BigSub_08001678', 'BigSub_080027E8', 'BigSub_080027A8', 'BigSub_08002A5C', 'BigSub_08002A80', 'BigSub_08002AD0', 'BigSub_08002B30', 'BigSub_08002BA8', 'BigSub_08002C18']
romNames = ['gUnk_0834C2AC', 'gUnk_0834C318', 'gUnk_083509D0', 'gUnk_08350A3C', 'gUnk_08D60FDC', 'gUnk_082DDE84', 'gUnk_082DDEF0',
            'gUnk_082DDF5C', 'gUnk_0834C4B0', 'gUnk_08350AAC', 'gUnk_08350B30', 'sub_080356AC', 'sub_08088640', 'gUnk_08D60B64', 'gUnk_08D60BE4',
            'sub_080547C4', 'sub_0804A728', 'SwallowSpecialSite', 'sub_08123FD4', 'HasShard', 'sub_0801C558', 'gUnk_08351648', 'gUnk_0834C5D4', 'gUnk_08D610B4',
            # The rooms' tables (see KatAMExpansionRooms.cs);
            'gRoomProps', 'gForegroundInfo', 'gForegroundPalettes', 'gForegroundTilemaps', 'gSolidityMaps', 'gRoomTiledBGs', 'gLevelObjLists',
            'gUnk_08D63C28', 'gUnk_08D6CD0C', 'gUnk_08D640A4', 'gUnk_08D6DCAC', 'AreaPlates', 'WorldMapUnlockSave', 'WorldMapGetCoorFromRoom', 'WorldMapDrawKirbys', 'gUnk_0834BD94', 'gWorldMapDotCoors', 'sWorldMapLineCoors', 'gUnk_08357B8C', 'sub_08039600', 'GetKirbyRoomFlagIndex',
            # KatAM's intro, which the NiDL title screen replaces;
            'sub_08145B64',
            # Bigger rooms (rooms.c): what they replace, and the EWRAM heap's size;
            'FillLevelInfo', 'GetCollisionTile', 'sub_08002434', 'sub_08002470', 'sub_080024F0', 'sub_0800255C', 'sub_080025AC', 'sub_08002624', 'sub_080026A8', 'sub_08001408', 'sub_080015A8', 'EwramInitHeap',
            'sub_08002A5C', 'sub_08002A80', 'sub_08002AD0', 'sub_08002B30', 'sub_08002BA8', 'sub_08002C18',
            # The NiDL rooms' chests (rooms.c): the completion's chest count;
            'NumChestsCollected',
            # The NiDL rooms' items, taken once per visit (rooms.c): the objects' gone bits;
            'sub_08001678', 'sub_080027E8', 'sub_080027A8',
            # Warp stars in the NiDL rooms (KatAMExpansionRooms.cs): their flights, where a flight lands Kirby in each room; the
            # warp star's rev, a flight's room change and its step, Kirby's drawing and the star he rides (warpstar.c);
            'gUnk_08D60A84', 'gUnk_0893CB44', 'sub_0800C270', 'sub_08055D9C', 'sub_080534D0', 'sub_0803F46C', 'sub_0808EEF4',
            # Reduce flashing (calm.c): the VBlank's copies of the registers;
            'UpdateScreenDma', 'UpdateScreenCpuSet',
            # Mirror shards in chests (chest.c): a chest's opening;
            'sub_0800AFC8',
            # Skip the tutorial (skip.c): the save buffers' clear functions;
            'sbClearFunctions',
            # Boss rooms' shards (nidl_boss.c): where Kirby goes after the shard cutscene;
            'sub_0804C614', '_call_via_r1',
            # The bosses: the handlers run as one explodes (by type - 0x43), the HP bar's fill per type;
            'gUnk_08350E58',
            # Unlock the Master Sword (KatAMExpansionCalm.cs): the stand's post-game checks, as it runs and as it's drawn;
            'sub_080AA618', 'sub_080AAA64',
            # Unlock the Ability Room (unlock.c): the hub mirror's creation, holding its init's address;
            'CreateSpecialHubMirror', 'SpecialHubMirrorInit',
            # The Star Rod's glow (abilities.c): every sprite's drawing;
            'DisplaySprite',
            # Mirras (mirra.c): their door's opening; where they land (KatAMExpansionRooms.cs' MirraLandings()): what holds objects;
            'sub_080B11C0', 'gCollisionAttributes',
            # The ported rooms' visited doors in the save (rooms.c): what writes and reads the game's (and sbClearFunctions above);
            'WriteSaveSectionByOffset', 'VerifySaveByOffset']
print('namespace KatAMRandomizer {')
print('    // Generated by "ROM Expansion/build.sh" from the C sources there: the NiDL enemies\' code, linked at 0x09000000, for each version')
print('    // of KatAM, with the offsets of the symbols KatAMExpansion.cs uses and the addresses it patches (file offsets; 0: none);')
print('    internal static class KatAMExpansionCode {')
print('        internal sealed record Version(string Name, string GameCode, int Revision, byte[] Code, int CreateNidlEnemy, int NidlStart,')
print('                                       int NidlCopyName, int NidlData, int SpriteTables, int AnimCount, int SpawnTable, int ObjectParameters,')
print('                                       int ObjectParameters2, int Names, int HitFlashBranch, int NameSite1, int NameSite2,')
print('                                       IReadOnlyDictionary<string, int> CodeSymbols, IReadOnlyDictionary<string, int> RomSymbols);')
# The code first: static fields are initialized in order;
for v, name, code, revision in versions:
    data = open(f'{out}/{v}.bin', 'rb').read()
    print()
    print(f'        static readonly byte[] {v.capitalize()} = {{')
    for i in range(0, len(data), 24):
        print('            ' + ' '.join(f'0x{b:02X},' for b in data[i:i + 24]))
    print('        };')
print()
print('        public static readonly Version[] Versions = {')
for v, name, code, revision in versions:
    syms = {}
    for line in subprocess.run(['arm-none-eabi-nm', f'{out}/{v}.elf'], capture_output=True, text=True, check=True).stdout.splitlines():
        p = line.split()
        if len(p) == 3: syms[p[2]] = int(p[0], 16) - 0x09000000
    table = dict(l.split()[:2] for l in open(f'versions/{v}.txt') if l.strip() and not l.startswith('#'))
    def at(n): x = int(table[n], 0); return x - 0x08000000 if x >= 0x08000000 else x
    data = open(f'{out}/{v}.bin', 'rb').read()
    values = [syms[n] for n in names] + [at(n) for n in patched]
    def dictionary(d): return 'new Dictionary<string, int> { ' + ', '.join(f'["{k}"] = 0x{x:X}' for k, x in d.items()) + ' }'
    print(f'            new Version("{name}", "{code}", {revision}, {v.capitalize()}, ' + ', '.join(f'0x{x:X}' for x in values) + ',')
    print('                ' + dictionary({n: syms[n] for n in codeNames}) + ',')
    print('                ' + dictionary({n: at(n) for n in romNames}) + '),')
print('        };')
print('    }')
print('}')
PY
for v in usa europe japan; do echo "$v: $(stat -c %s "$OUT/$v.bin") bytes of code"; done
