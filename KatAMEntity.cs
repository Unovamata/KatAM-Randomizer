using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class Entity {
    public string Name { get; set; }
    public byte[] Definition { get; set; }
    public int Address { get; set; }

    // Object Number in a room;
    public byte[] Number { get; set; }

    // Object to which this entity is linked with; For doors or buttons;
    public byte[] Link { get; set; } = new byte[2];
    public byte[] X { get; set; } = new byte[2];
    public byte[] Y { get; set; } = new byte[2];
    public byte ID { get; set; }
    public byte Behavior { get; set; }
    public byte Speed { get; set; }
    public byte[] Properties { get; set; } = new byte[18];
    public byte Room { get; set; }

    public Entity() {}

    public EntitySerializable SerializeEntity() {
        return new EntitySerializable(this);
    }

    public void AreAllPropertiesZeroes() {
        if (this.Properties.All(x => x == 0)) {
            this.Properties = new byte[] { 0 };
        }
    }
}

public class EntitySerializable : Entity {
    public string Definition { get; set; }
    public string Number { get; set; }
    public string Link { get; set; }
    public string X { get; set; }
    public string Y { get; set; }
    public string Properties { get; set; }

    public EntitySerializable() {}

    public EntitySerializable(Entity entity) {
        Definition = ByteArrayToHexString(entity.Definition);
        Number = ByteArrayToHexString(entity.Number);
        Link = ByteArrayToHexString(entity.Link);
        X = ByteArrayToHexString(entity.X);
        Y = ByteArrayToHexString(entity.Y);
        Properties = ByteArrayToHexString(entity.Properties);

        Name = entity.Name;
        Address = entity.Address;
        ID = entity.ID;
        Behavior = entity.Behavior;
        Speed = entity.Speed;
        Room = entity.Room;
    }

    public static string ByteArrayToHexString(byte[] byteArray) {
        string result = "";

        foreach (byte bit in byteArray) {
            result += bit.ToString("X2");
        }

        return result;
    }

    public Entity DeserializeEntity() {
        Entity entity = new Entity();

        /*entity.Definition = StringToByteArray(this.Definition);*/
        entity.Number = StringToByteArray(this.Number);
        entity.Link = StringToByteArray(this.Link);
        entity.X = StringToByteArray(this.X);
        entity.Y = StringToByteArray(this.Y);
        entity.Properties = StringToByteArray(this.Properties);

        entity.Name = Name;
        entity.Address = Address;
        entity.ID = ID;
        entity.Behavior = Behavior;
        entity.Speed = Speed;

        return entity;
    }

    public static byte[] StringToByteArray(string hexString) {
        // Initialize the byte array
        byte[] byteArray = new byte[hexString.Length / 2];

        // Convert each pair of characters to a byte
        for (int i = 0; i < hexString.Length; i += 2) {
            string byteString = hexString.Substring(i, 2);
            byteArray[i / 2] = Convert.ToByte(byteString, 16);
        }

        return byteArray;
    }
}
