namespace KatAMEntities {
    public class Entity {
        public string Name { get; set; }
        public long Address { get; set; }
        public int Hp { get; set; }
        public int CopyAbility { get; set; }
        public int Palette { get; set; }
        public string X { get; set; }
        public string Y { get; set; }
        public int Id { get; set; }
        public int Behavior { get; set; }
        public int Speed { get; set; }

        public Entity() {
            Name = "";
            Address = 0;
            Hp = 0;
            CopyAbility = 0;
            Palette = 0;
            X = "0";
            Y = "0";
            Id = 0;
            Behavior = 0;
            Speed = 0;
        }

        public Entity(string Name, long Address, int Hp, int CopyAbility, int Palette,
                      int X, int Y, int Id, int Behavior, int Speed) {
            this.Name = Name;
            this.Address = Address;
            this.Hp = Hp;
            this.CopyAbility = CopyAbility;
            this.Palette = Palette;
            this.X = X.ToString();
            this.Y = Y.ToString();
            this.Id = Id;
            this.Behavior = Behavior;
            this.Speed = Speed;
        }
    }

    public class Mirror : Entity {
        public int EightRom { get; set; }
        public int NineRom { get; set; }
        public string Exits { get; set; }
        public string Description { get; set; }

        public Mirror(string Name, int Address, int EightRom, int NineRom, int ID,
                      string Exits, string Description, int X, int Y)
            : base(Name, Address, 0, 0, 0, X, Y, ID, 0, 0) {
            this.EightRom = EightRom;
            this.NineRom = NineRom;
            this.Exits = Exits;
            this.Description = Description;
        }
    }

    public class Item : Entity {
        public long Properties { get; set; }
        public long Room { get; set; }

        public Item(string Name, long Properties, long Address, int HP, int X, int Y, long Room)
            : base(Name, Address, HP, 0, 0, X, Y, 0, 0, 0) {
            this.Properties = Properties;
            this.Room = Room;
        }

        public Item() {
            this.Name = "";
            this.Address = 0;
            this.Hp = 0;
            this.CopyAbility = 0;
            this.Palette = 0;
            this.X = "0";
            this.Y = "0";
            this.Id = 0;
            this.Behavior = 0;
            this.Speed = 0;
            this.Properties = 0;
            this.Room = 0;
        }
    }


    public class Enemy : Entity {
        public Enemy(string Name, int Address, int HP)
            : base(Name, Address, HP, 0, 0, 0, 0, 0, 0, 0) { }
    }


    public class Miniboss : Entity {
        public int GroundCoordinates { get; set; }
        public int AirCoordinates { get; set; }
        public int GroundCameraCoordinates { get; set; }
        public int AirCameraCoordinates { get; set; }
        public int Facing { get; set; }

        public Miniboss(string Name, int Address, int HP, int ID,
                        int GroundCoordinates, int AirCoordinates,
                        int GroundCameraCoordinates, int AirCameraCoordinates, int Facing)
            : base(Name, Address, HP, 0, 0, 0, 0, ID, 0, 0) {
            this.GroundCoordinates = GroundCoordinates;
            this.AirCoordinates = AirCoordinates;
            this.GroundCameraCoordinates = GroundCameraCoordinates;
            this.AirCameraCoordinates = AirCameraCoordinates;
            this.Facing = Facing;
        }
    }
}