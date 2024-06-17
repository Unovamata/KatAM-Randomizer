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
            Name = "";
            DamageSprites = new byte[2];
            HP = 0;
            CopyAbility = 0;
            Palette = 0;
        }

        public PropertiesSerializable SerializeEntity() {
            return new PropertiesSerializable(this);
        }
    }

    public class PropertiesSerializable : Properties {
        public string Definition { get; set; }
        public string DamageSprites { get; set; }

        public PropertiesSerializable() {}

        public PropertiesSerializable(Properties properties) {
            Definition = ByteArrayToHexString(properties.Definition);
            DamageSprites = ByteArrayToHexString(properties.DamageSprites);

            Name = properties.Name;
            HP = properties.HP;
            CopyAbility = properties.CopyAbility;
            Palette = properties.Palette;
        }

        public static string ByteArrayToHexString(byte[] byteArray) {
            return Utils.ByteArrayToHexString(byteArray);
        }

        public Properties DeserializeProperties() {
            Properties properties = new Properties();

            properties.Definition = StringToByteArray(this.Definition);
            properties.DamageSprites = StringToByteArray(this.DamageSprites);

            Name = properties.Name;
            HP = properties.HP;
            CopyAbility = properties.CopyAbility;
            Palette = properties.Palette;

            return properties;
        }

        public static byte[] StringToByteArray(string hexString) {
            return Utils.StringToByteArray(hexString);
        }
    }
}
