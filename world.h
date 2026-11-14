// world.h

# ifndef WORLD_H
# define WORLD_H

# include <vector>
# include <cstdint> // 1-Byte Storage for Optimization (I saw it in a video ~a year ago)
# include <glm/glm.hpp>

enum BlockType : uint8_t {
    AIR = 0,
    PEPE,
    EMERALD,
    OBSIDIAN,
    ICE,
    COPPER,
    GOLD,
    SILVER,
    DIAMOND,
    GLASS,
    CRATE
};


class World
{
    public:
        const int sizeX, sizeY, sizeZ;

        World(int x, int y, int z) : sizeX(x), sizeY(y), sizeZ(z), blocks(x * y * z, AIR) {}

        bool inBounds(int x, int y, int z) const
        {
            if (x < 0 || x >= sizeX)
                return false;
            if (y < 0 || y >= sizeY)
                return false;
            if (z < 0 || z >= sizeZ)
                return false;
            return true;
        }

        uint8_t get(int x, int y, int z) const
        {
            if (!inBounds(x, y, z))
                return AIR;
        
            int pos = index(x, y, z);
            return blocks[pos];
        }

        void set(int x, int y, int z, uint8_t type)
        {
            if (!inBounds(x, y, z)) // Ignores Invalid Coords
                return;
            
                int pos = index(x, y, z);
                blocks[pos] = type;
        }

        void generateTerrain(int thickness, uint8_t type) // Slab of Pepes
        {
            for (int y = 0; y < thickness && y < sizeY; y++)
                for (int z = 0; z < sizeZ; z++)
                    for (int x = 0; x < sizeX; x++)
                        set(x, y, z, type);
        }

        bool isExposed(int x, int y, int z) const
        {
            return get(x + 1, y, z) == AIR || get(x - 1, y, z) == AIR || get(x, y + 1, z) == AIR || get(x, y - 1, z) == AIR || get(x, y, z + 1) == AIR || get(x, y, z - 1) == AIR;
        }

        std::vector<glm::vec3> buildInstanceOffsets(glm::vec3 origin, uint8_t type) const
        {
            std::vector<glm::vec3> offsets;

            for (int y = 0; y < sizeY; y++)
                for (int z = 0; z < sizeZ; z++)
                    for (int x = 0; x < sizeX; x++)
                        if (get(x, y, z) == type && isExposed(x, y, z))
                        {
                            glm::vec3 gridPos((float)x, (float)y, (float)z);
                            offsets.push_back(origin + gridPos);
                        }

            return offsets;
        }
    
    private:
        std::vector<uint8_t> blocks;
        int index(int x, int y, int z) const
        {
            int rowStart = z * sizeX, layerStart = y * sizeX * sizeZ;
            return x + rowStart + layerStart;
        }
};

#endif