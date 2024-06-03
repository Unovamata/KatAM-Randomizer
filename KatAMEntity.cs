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
    public byte[] Link { get; set; }
    public byte[] X { get; set; }
    public byte[] Y { get; set; }
    public byte ID { get; set; }
    public byte Behavior { get; set; }
    public byte Speed { get; set; }
    public byte[] Properties { get; set; }
    public byte Room { get; set; }

    public Entity() {
        Link = new byte[2];
        X = new byte[2];
        Y = new byte[2];
        Properties = new byte[18];
    }

    public EntitySerializable SerializeEntity() {
        return new EntitySerializable(this);
    }

    public void AreAllPropertiesZeroes() {
        if (Properties.All(x => x == 0)) {
            Properties = new byte[] { 0 };
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

    public static string ByteArrayToString(byte[] byteArray) {
        // Convert byte array to a comma-separated string
        string result = "[" + string.Join(", ", byteArray) + "]";

        return result;
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

        entity.Definition = StringToByteArray(Definition);
        entity.Number = StringToByteArray(Number);
        entity.Link = StringToByteArray(Link);
        entity.X = StringToByteArray(X);
        entity.Y = StringToByteArray(Y);
        entity.Properties = StringToByteArray(Properties);

        entity.Name = Name;
        entity.Address = Address;
        entity.ID = ID;
        entity.Behavior = Behavior;
        entity.Speed = Speed;

        return entity;
    }

    public static byte[] StringToByteArray(string input) {
        // Remove the square brackets
        input = input.Trim(new char[] { '[', ']' });

        // Split the string by commas
        string[] stringArray = input.Split(',');

        // Convert each string element to a byte
        byte[] byteArray = stringArray.Select(byte.Parse).ToArray();

        return byteArray;
    }
}
