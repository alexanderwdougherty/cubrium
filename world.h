// world.h

# ifndef WORLD_H
# define WORLD_H

# include <glad/glad.h>
# include <glm/glm.hpp>

# include <vector>
# include <array>
# include <unordered_map>
# include <memory>
# include <cstdint> // 1-Byte Storage for Optimization (I saw it in a video ~a year ago)
# include <cmath>
# include <algorithm>
# include <numeric>
# include <random>

const int CHUNK_SIZE = 16;

enum BlockType : uint8_t {
    AIR = 0, PEPE, EMERALD, OBSIDIAN, ICE, COPPER, GOLD, SILVER, DIAMOND, GLASS, CRATE
};

struct ChunkBatch {
    unsigned int vao = 0, vbo = 0;
    GLsizei count = 0;
};

class Chunk
{
    public:
        glm::ivec3 chunkPos;
        std::array<uint8_t, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> blocks{};
        std::array<ChunkBatch, CRATE + 1 > batches{};

        Chunk(glm::ivec3 pos)
        {
            chunkPos = pos;
        }

        ~Chunk()
        {
            cleanup();
        }

        Chunk(const Chunk&) = delete;
        Chunk& operator = (const Chunk&) = delete;

        bool isInsideChunk(int x, int y, int z) const
        {
            if (x < 0 || x >= CHUNK_SIZE) return false;
            if (y < 0 || y >= CHUNK_SIZE) return false;
            if (z < 0 || z >= CHUNK_SIZE) return false;
            return true;
        }

        uint8_t get (int x, int y, int z) const
        {
            if (!isInsideChunk(x, y, z))
                return AIR;
            
                return blocks[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE];
        }

        void set(int x, int y, int z, uint8_t type)
        {
            if (!isInsideChunk(x, y, z))
                return;
            
                blocks[x + y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE] = type;
        }

        bool isExposed(int x, int y, int z) const
        {
            return get(x + 1, y, z) == AIR || get(x - 1, y, z) == AIR || get(x, y + 1, z) == AIR || get(x, y - 1, z) == AIR || get(x, y, z + 1) == AIR || get(x, y, z - 1) == AIR;
        }

        void rebuildMesh(unsigned int cubeVBO, GLsizei stride)
        {
            cleanup();

            float baseX = (float)(chunkPos.x * CHUNK_SIZE), baseY = (float)(chunkPos.y * CHUNK_SIZE), baseZ = (float)(chunkPos.z * CHUNK_SIZE);

            std::vector<glm::vec3> offsets[CRATE + 1];

            for (int x = 0; x < CHUNK_SIZE; x++)
                for (int y = 0; y < CHUNK_SIZE; y++)
                    for (int z = 0; z < CHUNK_SIZE; z++)
                    {
                        uint8_t type = get(x, y, z);

                        if (type != AIR && isExposed(x, y, z))
                        {
                            glm::vec3 worldPos;
                            worldPos.x = baseX + (float)x;
                            worldPos.y = baseY + (float)y;
                            worldPos.z = baseZ + (float)z;
                            offsets[type].push_back(worldPos);
                        }
                    }
            
            for (int i = PEPE; i <= CRATE; i++)
            {
                if (offsets[i].size() == 0)
                    continue;
                
                batches[i].count = (GLsizei)offsets[i].size();

                glGenBuffers(1, &batches[i].vbo);
                glBindBuffer(GL_ARRAY_BUFFER, batches[i].vbo);
                glBufferData(GL_ARRAY_BUFFER, offsets[i].size() * sizeof(glm::vec3), offsets[i].data(), GL_STATIC_DRAW);

                glGenVertexArrays(1, &batches[i].vao);
                glBindVertexArray(batches[i].vao);

                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
                glEnableVertexAttribArray(4);

                glBindBuffer(GL_ARRAY_BUFFER, batches[i].vbo);
                glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(5);
                glVertexAttribDivisor(5, 1);
            }
        }

        void cleanup()
        {
            for (int i = 0; i <= CRATE; i++)
            {
                if (batches[i].vao != 0)
                {
                    glDeleteVertexArrays(1, &batches[i].vao);
                    batches[i].vao = 0;
                }
                if (batches[i].vbo != 0)
                {
                    glDeleteBuffers(1, &batches[i].vbo);
                    batches[i].vbo = 0;
                }
                batches[i].count = 0;
            }
        }

        private:
        // e
        
    };
    
struct IVec3Hash { // Really Cool Hash Function that uses large prime numbers to avoid Hash Collisions!
    size_t operator()(const glm::ivec3& v) const noexcept
    {
        size_t hashX = std::hash<int>()(v.x) * 73856093;
        size_t hashY = std::hash<int>()(v.y) * 19349663;
        size_t hashZ = std::hash<int>()(v.z) * 83492791;
        return hashX ^ hashY ^ hashZ;
    }
};

class World
{
    public:
        std::unordered_map<glm::ivec3, Chunk*, IVec3Hash> chunks;
        int renderDistance;

        World(int renderDist = 8)
        {
            renderDistance = renderDist;
        }

        ~World()
        {
            for (auto& pair : chunks)
                delete pair.second;

            chunks.clear();
        }

        void generateTerrain(Chunk* chunk)
        {
            if (chunk -> chunkPos.y != 0)
                return;

            for (int x = 0; x < CHUNK_SIZE; x++)
                for (int z = 0; z < CHUNK_SIZE; z++)
                    chunk -> set(x, 0, z, OBSIDIAN);
        }

        void update(glm::vec3 camPos, unsigned int cubeVBO, GLsizei stride)
        {
            int camChunkX = (int)std::floor(camPos.x / (float)CHUNK_SIZE),
            camChunkY = (int)std::floor(camPos.y / (float)CHUNK_SIZE),
            camChunkZ = (int)std::floor(camPos.z / (float)CHUNK_SIZE);
            glm::ivec3 camChunk(camChunkX, camChunkY, camChunkZ);

            for (int x = -renderDistance; x <= renderDistance; x++)
                for (int y = -renderDistance; y <= renderDistance; y++)
                    for (int z = -renderDistance; z <= renderDistance; z++)
                    {
                        glm::ivec3 targetPos(camChunkX + x, camChunkY + y, camChunkZ + z);

                        if (chunks.find(targetPos) == chunks.end())
                        {
                            Chunk* newChunk = new Chunk(targetPos);
                            generateTerrain(newChunk);
                            newChunk -> rebuildMesh(cubeVBO, stride);
                            chunks[targetPos] = newChunk;
                        }
                    }

            auto i = chunks.begin();
            while (i != chunks.end())
            {
                glm::ivec3 chunkCoord = i -> first;
                int distX = std::abs(chunkCoord.x - camChunkX),
                distY = std::abs(chunkCoord.y - camChunkY),
                distZ = std::abs(chunkCoord.z - camChunkZ);

                if (distX > renderDistance + 1 || distY > renderDistance + 1 || distZ > renderDistance + 1)
                {
                    delete i -> second;
                    i = chunks.erase(i);
                }
                else
                {
                    i++;
                }
            }
        }
};

#endif