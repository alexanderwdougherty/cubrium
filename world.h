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
# include <climits>
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

class PerlinNoise
{
    public:
        PerlinNoise(uint32_t seed = 0)
        {
            p.resize(256);
            std::iota(p.begin(), p.end(), 0);
            std::mt19937 engine(seed); // THE (4-Byte) MERSENNE TWISTER
            std::shuffle(p.begin(), p.end(), engine);
            p.insert(p.end(), p.begin(), p.end());
        }

        double noise(double x, double y) const
        {
            int cellX = static_cast<int>(std::floor(x)) & 255, cellY = static_cast<int>(std::floor(y)) & 255;
            double localX = x - std::floor(x), localY = y - std::floor(y);

            int a = p[cellX] + cellY, b = p[cellX + 1] + cellY;

            return lerp(fade(localY),
                lerp(fade(localX), grad(p[a], localX, localY), grad(p[b], localX - 1.0, localY)),
                lerp(fade(localX), grad(p[a + 1], localX, localY - 1.0), grad(p[b + 1], localX - 1.0, localY - 1.0))
            );
        }

        double octaveNoise(double x, double y, int octaves, double persistence = 0.5) const
        {
            double total = 0.0, frequency = 1.0, amplitude = 1.0, max = 0.0;

            for (int i = 0; i < octaves; i++)
            {
                total += noise(x * frequency, y * frequency) * amplitude;
                max += amplitude;
                amplitude *= persistence;
                frequency *= 2.0;
            }

            return total / max;
        }
    
    private:
        std::vector<int> p;
        static double fade(double t)
        { 
            return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
        }
        
        static double lerp(double weight, double start, double end)
        {
            return start + weight * (end - start);
        }

        static double grad(int hash, double x, double y)
        {
            int h = hash & 7; // 8 Semi-Random Gradient Vectors
            double gradientX, gradientY;

            if (h < 4)
                gradientX = 1.0, gradientY = 2.0;
            else
                gradientX = 2.0, gradientY = 1.0;

            if ((h & 1) != 0)
                gradientX = -gradientX;
            if ((h & 2) != 0)
                gradientY = -gradientY;

            return (gradientX * x) + (gradientY * y);
        }
};

class World
{
    public:
        std::unordered_map<glm::ivec3, Chunk*, IVec3Hash> chunks;
        int renderDistance, voxelGridSize;
        uint32_t seed;
        PerlinNoise perlin;

        unsigned int voxelTex3D = 0;
        glm::ivec3 voxelGridOrigin = glm::ivec3(0), lastVoxelCamChunk = glm::ivec3(INT_MAX);

        World(int renderDist = 8, uint32_t worldSeed = 123454321)
            : renderDistance(renderDist), voxelGridSize((renderDist * 2 + 1) * CHUNK_SIZE), seed(worldSeed), perlin(worldSeed) {}

        ~World()
        {
            for (auto& pair : chunks)
                delete pair.second;

            chunks.clear();

            if (voxelTex3D != 0)
                glDeleteTextures(1, &voxelTex3D);
        }

        void initVoxelTexture()
        {
            glGenTextures(1, &voxelTex3D);
            glBindTexture(GL_TEXTURE_3D, voxelTex3D);

            glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, voxelGridSize, voxelGridSize, voxelGridSize, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }

        void updateVoxelTexture(glm::ivec3 camChunk)
        {
            if (voxelTex3D == 0)
                initVoxelTexture();
            if (camChunk == lastVoxelCamChunk)
                return;
            
            lastVoxelCamChunk = camChunk;

            voxelGridOrigin = (camChunk - glm::ivec3(renderDistance)) * CHUNK_SIZE;
            std::vector<uint8_t> volume(voxelGridSize * voxelGridSize * voxelGridSize, AIR);

            for (int cx = -renderDistance; cx <= renderDistance; cx++ )
                for (int cy = -renderDistance; cy <= renderDistance; cy++)
                    for (int cz = -renderDistance; cz <= renderDistance; cz++)
                    {
                        auto i = chunks.find(camChunk + glm::ivec3(cx, cy, cz));
                        if (i == chunks.end())
                            continue;
                        
                        int baseX = (cx + renderDistance) * CHUNK_SIZE, baseY = (cy + renderDistance) * CHUNK_SIZE, baseZ = (cz + renderDistance) * CHUNK_SIZE;

                        for (int x = 0; x < CHUNK_SIZE; x++)
                            for (int y = 0; y < CHUNK_SIZE; y++)
                                for (int z = 0; z < CHUNK_SIZE; z++)
                                {
                                    uint8_t block = i -> second -> get(x, y, z);
                                    if (block == AIR)
                                        continue;
                                    volume[(baseX + x) + (baseY + y) * voxelGridSize + (baseZ + z) * voxelGridSize * voxelGridSize] = block;
                                }
                    }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glBindTexture(GL_TEXTURE_3D, voxelTex3D);
            glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, voxelGridSize, voxelGridSize, voxelGridSize, GL_RED_INTEGER, GL_UNSIGNED_BYTE, volume.data());
        }

        void generateTerrain(Chunk* chunk)
        {
            const float frequency = 0.02f;
            const int averageHeight = 8, max = 16;

            if (chunk -> chunkPos.y * CHUNK_SIZE > averageHeight + max)
                return;

            for (int x = 0; x < CHUNK_SIZE; x++)
                for (int z = 0; z < CHUNK_SIZE; z++)
                {
                    int worldX = chunk -> chunkPos.x * CHUNK_SIZE + x, worldZ = chunk -> chunkPos.z * CHUNK_SIZE + z;
                    double noiseValue = perlin.octaveNoise(worldX * frequency, worldZ * frequency, 4, 0.5);
                    int surfaceY = averageHeight + (int)std::round(noiseValue * max);

                    for (int y = 0; y < CHUNK_SIZE; y++)
                    {
                        int worldY = chunk -> chunkPos.y * CHUNK_SIZE + y;
                        if (worldY > surfaceY)
                            continue;
                        
                        if (worldY == surfaceY)
                            chunk -> set(x, y, z, OBSIDIAN);
                        else if (worldY > surfaceY - 4)
                            chunk -> set(x, y, z, COPPER);
                        else
                            chunk -> set(x, y, z, SILVER);
                    }
                }
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

            updateVoxelTexture(camChunk);
        }
};

#endif