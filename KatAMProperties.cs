using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAMRandomizer {
    public class Properties {
        public byte[] Definition { get; set; }
        public int Address { get; set; }
        public string Name { get; set; }
        public byte[] DamageSprites { get; set; }
        public byte HP { get; set; }
        public byte CopyAbility { get; set; }
        public byte Palette { get; set; }

        public Properties() {
            Definition = new byte[24];
            Name = "";
            DamageSprites = new byte[2];
            HP = 0;
            CopyAbility = 0;
            Palette = 0;
        }
    }
}
