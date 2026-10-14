#!/bin/bash
# Builds broom_hatter.c into the code blob KatAMExpansion.cs embeds (BroomHatterCode), linked at 0x08FC0000;
# Needs the KatAM decompilation (https://github.com/jiangzhengwenjz/katam) built for the USA version (katam.elf), with agbcc installed in it,
# and arm-none-eabi binutils. Usage: ./build.sh /path/to/katam;
set -e
D=${1:?path to the katam decompilation}

cpp -I "$D/tools/agbcc/include" -I "$D/tools/agbcc" -nostdinc -iquote "$D/include" -DREVISION=0 -DENGLISH -Wno-trigraphs -DMODERN=0 broom_hatter.c \
    | "$D/tools/agbcc/bin/agbcc" -mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm -o broom_hatter.s -
arm-none-eabi-as -mcpu=arm7tdmi -o broom_hatter.o broom_hatter.s
arm-none-eabi-ld -Ttext=0x08FC0000 --just-symbols="$D/katam.elf" -e CreateBroomHatter -o broom_hatter.elf broom_hatter.o
arm-none-eabi-objcopy -O binary broom_hatter.elf broom_hatter.bin

# Symbols KatAMExpansion.cs needs (offsets from 0x08FC0000);
arm-none-eabi-nm broom_hatter.elf | grep -E " (CreateBroomHatter|BroomHatterStart|gBroomHatterAnims)$"
rm broom_hatter.s broom_hatter.o broom_hatter.elf
echo "broom_hatter.bin: $(stat -c %s broom_hatter.bin) bytes"
