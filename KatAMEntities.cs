using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAMEntities {
    public class Entity {
        public string Name { get; set; }
        public int Address { get; set; }
        public int Hp { get; set; }
        public int CopyAbility { get; set; }
        public int Palette { get; set; }
        public int X { get; set; }
        public int Y { get; set; }
        public int Id { get; set; }
        public int Behavior { get; set; }
        public int Speed { get; set; }

        public Entity() {
            Name = "";
            Address = 0;
            Hp = 0;
            CopyAbility = 0;
            Palette = 0;
            X = 0;
            Y = 0;
            Id = 0;
            Behavior = 0;
            Speed = 0;
        }

        public Entity(string Name, int Address, int Hp, int CopyAbility, int Palette,
                      int X, int Y, int Id, int Behavior, int Speed) {
            this.Name = Name;
            this.Address = Address;
            this.Hp = Hp;
            this.CopyAbility = CopyAbility;
            this.Palette = Palette;
            this.X = X;
            this.Y = Y;
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
        public int Properties { get; set; }
        public int Room { get; set; }

        public Item(string Name, int Properties, int Address, int HP, int X, int Y, int Room)
            : base(Name, Address, HP, 0, 0, X, Y, 0, 0, 0) {
            this.Properties = Properties;
            this.Room = Room;
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