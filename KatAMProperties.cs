using KatAMInternal;
using System.Windows.Forms.Design.Behavior;


namespace KatAMRandomizer {
    public class Properties {
        public byte[] Definition { get; set; }
        public byte ID { get; set; }
        public int Address { get; set; }
        public string Name { get; set; }
        public byte[] DamageSprites { get; set; }
        public byte HP { get; set; }
        public byte CopyAbility { get; set; }
        public byte Palette { get; set; }

        public Properties() {
            Definition = new byte[24];
            ID = 0;
            Address = 0;
            Name = "";
            DamageSprites = new byte[2];
            HP = 0;
            CopyAbility = 0;
            Palette = 0;
        }
    }
}