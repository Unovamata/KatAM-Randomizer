using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class Entity {
    public string Name { get; set; }
    public string Description { get; set; }
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
    public byte[] Properties { get; set; } = new byte[19];
    public int Room { get; set; }
    public byte AbilityID { get; set; }
    public bool IsUnderwater { get; set; }
    public bool IsFlying { get; set; }
    public bool IsInhalable { get; set; }

    public Entity() {}

    public Entity(Entity entity) {
        Name = entity.Name;
        Description = entity.Description;
        Definition = entity.Definition;
        Address = entity.Address;
        Number = entity.Number;
        Link = entity.Link;
        X = entity.X;
        Y = entity.Y;
        ID = entity.ID;
        Behavior = entity.Behavior;
        Speed = entity.Speed;
        Properties = entity.Properties;
        Room = entity.Room;
        AbilityID = entity.AbilityID;
        IsUnderwater = entity.IsUnderwater;
        IsFlying = entity.IsFlying;
        IsInhalable = entity.IsInhalable;
    }

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
        Description = entity.Description;
        Address = entity.Address;
        ID = entity.ID;
        Behavior = entity.Behavior;
        Speed = entity.Speed;
        Room = entity.Room; 
        AbilityID = entity.AbilityID;
        IsUnderwater = entity.IsUnderwater;
        IsFlying = entity.IsFlying;
        IsInhalable = entity.IsInhalable;
    }

    public static string ByteArrayToHexString(byte[] byteArray) {
        return Utils.ByteArrayToHexString(byteArray);
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
        entity.Description = Description;
        entity.Address = Address;
        entity.ID = ID;
        entity.Behavior = Behavior;
        entity.Speed = Speed;
        entity.Room = Room;
        entity.IsUnderwater = IsUnderwater;
        entity.IsFlying = IsFlying;
        entity.AbilityID = AbilityID;
        entity.IsUnderwater = IsUnderwater;
        entity.IsFlying = IsFlying;
        entity.IsInhalable = IsInhalable;

        return entity;
    }

    public static byte[] StringToByteArray(string hexString) {
        return Utils.StringToByteArray(hexString);
    }
}

public class Data {
    public string Name { get; set; }
    public byte AbilityID { get; set; }
    public bool IsInhalable { get; set; }
    public bool IsUnderwater { get; set; }
    public bool IsFlying { get; set; }

    public Data(string name, byte abilityID, bool isInhalable, bool isUnderwater, bool isFlying) {
        Name = name;
        AbilityID = abilityID;
        IsInhalable = isInhalable;
        IsUnderwater = isUnderwater;
        IsFlying = isFlying;
    }
}