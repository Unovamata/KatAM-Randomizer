using KatAMInternal;

namespace KatAM_Randomizer
{
    public interface IKatAMRandomizer
    {
        public bool FilterEntities(Entity entity);
    }

    internal class KatAMRandomizerComponent
    {
        protected static Processing System;
        protected static Settings Settings;
        protected static int Seed;
        protected static List<Entity> entities;

        public void InitializeComponents(Processing system)
        {
            System = system;
            Settings = system.Settings;
            Seed = system.Settings.Seed;
        }
    }
}
