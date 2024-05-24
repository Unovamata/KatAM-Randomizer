using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAMRandomizerClasses{
    public class Object {
        int address, hp, copyAbility, palette, x, y, id, behavior, speed;

        public Object(int Address, int HP, int CopyAbility, int Palette,
                      int X, int Y, int ID, int Behavior, int Speed) {
            address = Address;
            hp = HP;
            copyAbility = CopyAbility;
            palette = Palette;
            x = X; 
            y = Y;
            id = ID;
            behavior = Behavior;
            speed = Speed;
        }

        public int GetAddress() { return address; }
        public void SetAddress(int value) { address = value; }

        public int GetHp() { return hp; }
        public void SetHp(int value) { hp = value; }

        public int GetCopyAbility() { return copyAbility; }
        public void SetCopyAbility(int value) { copyAbility = value; }

        public int GetPalette() { return palette; }
        public void SetPalette(int value) { palette = value; }

        public int GetX() { return x; }
        public void SetX(int value) { x = value; }

        public int GetY() { return y; }
        public void SetY(int value) { y = value; }

        public int GetId() { return id; }
        public void SetId(int value) { id = value; }

        public int GetBehavior() { return behavior; }
        public void SetBehavior(int value) { behavior = value; }

        public int GetSpeed() { return speed; }
        public void SetSpeed(int value) { speed = value; }
    }

    public class Mirror : Object{
        string name;
        int eightRom, nineRom;
        string exits, description;

        public Mirror(string Name, int Address, int EightRom, int NineRom, int ID, 
                      string Exits, string Description, int X, int Y)
        : base(Address, 0, 0, 0, X, Y, ID, 0, 0) {
            name = Name;
            eightRom = EightRom;
            nineRom = NineRom;
            exits = Exits;
            description = Description;
        }
    }

    public class Item {

    }

    public class Chest {

    }
}
