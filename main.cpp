// main.cpp

# include <glad/glad.h>
# include <GLFW/glfw3.h>

# include "shader_s.h"
# include "camera.h"
# include "materials.h"
# include "world.h"

# define STB_IMAGE_IMPLEMENTATION
# include <stb_image.h>

# include <glm/glm.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtc/type_ptr.hpp>

# include <iostream>
# include <cmath>

# include <map>
# include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
std::vector<float> buildVertexData(const float* v, size_t vertexCount);

const unsigned int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;
int windowWidth = SCREEN_WIDTH, windowHeight = SCREEN_HEIGHT;
bool framebufferResized = false;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f, lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f, lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int effectMode = 0;

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__ // Apple Support
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif 

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Test", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, & windowWidth, &windowHeight);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    Shader lightingShader("shaders/shader.vs", "shaders/shader.fs");
    Shader lightCubeShader("shaders/light_cube.vs", "shaders/light_cube.fs");
    Shader screenShader("shaders/screen.vs", "shaders/screen.fs");
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    // X, Y, Z, NX, NY, NZ, S, T
    float vertices[] = {
        // Back
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 0.0f, -1.0f,      1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 0.0f, -1.0f,      1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 1.0f,

        // Front
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f,

        // Left
        -0.5f,  0.5f,  0.5f,    -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,

        // Right
        0.5f,  0.5f,  0.5f,     1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,       0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,     1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f,       0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,     1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     1.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        // Bottom
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,      0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     0.0f, -1.0f, 0.0f,      1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,     0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,      0.0f, 1.0f,

        // Top
        -0.5f, 0.5f, -0.5f,     0.0f, 1.0f, 0.0f,       0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,       0.0f, 1.0f
    };

    // X, Y, Z, NX, NY, NZ, S, T
    float transparentVertices[] = {
        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f,       1.0f, 0.0f,

        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 1.0f,       1.0f, 1.0f
    };

    // X, Y, S, T
    float quadVertices[] = {
        -1.0f, 1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
        1.0f, -1.0f,    1.0f, 0.0f,

        -1.0f, 1.0f,    0.0f, 1.0f,
        1.0f, -1.0f,    1.0f, 0.0f,
        1.0f, 1.0f,     1.0f, 1.0f
    };

    // X, Y, Z
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)
    };
    
    // X, Y, Z
    glm::vec3 windowPositions[] = {
        glm::vec3(1.5f, 0.0f, 1.5f),
        glm::vec3(0.0f, 0.0f, 1.7f),
        glm::vec3(-0.5f, 0.0f, -2.3f),
        glm::vec3(1.5f, 0.0f, -1.6f)
    };

    const GLsizei stride = 14 * sizeof(float); // pos: 0-2, normal: 3-5, uv = 6-7, tangent: 8-10, bitangent: 11-13

    std::vector<float> cubeData = buildVertexData(vertices, 36);
    std::vector<float> windowData = buildVertexData(transparentVertices, 6);

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cubeData.size() * sizeof(float), cubeData.data(), GL_STATIC_DRAW);

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

    glDisableVertexAttribArray(5);
    glVertexAttrib3f(5, 0.0f, 0.0f, 0.0f);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, windowData.size() * sizeof(float), windowData.data(), GL_STATIC_DRAW);

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

    glDisableVertexAttribArray(5);
    glVertexAttrib3f(5, 0.0f, 0.0f, 0.0f);

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    World world(32, 4, 32);
    world.generateTerrain(1, ICE);

    glm::vec3 worldOrigin(-world.sizeX / 2.0f, -2.0f, -world.sizeZ / 2.0f);

    struct VoxelBatch { unsigned int vao, vbo; GLsizei count; };

    auto buildVoxelBatch = [&](uint8_t type)
    {
        std::vector<glm::vec3> offsets = world.buildInstanceOffsets(worldOrigin, type);

        VoxelBatch batch;
        batch.count = (GLsizei)offsets.size();

        glGenBuffers(1, &batch.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
        glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec3), offsets.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &batch.vao);
        glBindVertexArray(batch.vao);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

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

        glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);

        return batch;
    };

    std::vector<VoxelBatch> voxelBatches(ICE + 1);
    for (unsigned int type = PEPE; type <= ICE; type++)
        voxelBatches[type] = buildVoxelBatch((uint8_t)type);

    std::vector<std::string> skyboxFaces = {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg"
    };

    unsigned int windowTexture = loadTexture("assets/window/diffuse.png"),
    windowSpecular = loadTexture("assets/window/specular.png"),
    windowNormal = loadTexture("assets/window/normal.png"),
    windowDisplacement = loadTexture("assets/window/displacement.png"),
    windowAmbient = loadTexture("assets/window/ambient.png");

    struct CubeSetup { unsigned int diffuse, specular, normal, displacement, ambient; Material material; };
    std::vector<CubeSetup> cubeTextures = {
        { loadTexture("assets/pepe/diffuse.png"), loadTexture("assets/pepe/specular.png"), loadTexture("assets/pepe/normal.png"),
        loadTexture("assets/pepe/displacement.png"), loadTexture("assets/pepe/ambient.png"), Materials::PlasticWhite },

        { loadTexture("assets/emerald/diffuse.png"), loadTexture("assets/emerald/specular.png"), loadTexture("assets/emerald/normal.png"),
        loadTexture("assets/emerald/displacement.png"), loadTexture("assets/emerald/ambient.png"), Materials::Emerald },

        { loadTexture("assets/obsidian/diffuse.png"), loadTexture("assets/obsidian/specular.png"), loadTexture("assets/obsidian/normal.png"),
        loadTexture("assets/obsidian/displacement.png"), loadTexture("assets/obsidian/ambient.png"), Materials::Obsidian },

        { loadTexture("assets/ice/diffuse.png"), loadTexture("assets/ice/specular.png"), loadTexture("assets/ice/normal.png"),
        loadTexture("assets/ice/displacement.png"), loadTexture("assets/ice/ambient.png"), Materials::Ice }
    };

    const int blockTexture[] = { -1, 0, 1, 2, 3 };

    unsigned int cubemapTexture = loadCubemap(skyboxFaces);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specularMap", 1);
    lightingShader.setInt("material.normalMap", 2);
    lightingShader.setInt("material.displacementMap", 3);
    lightingShader.setInt("material.ambientMap", 4);
    lightingShader.setInt("skybox", 5);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 5);

    screenShader.use();
    screenShader.setInt("screenTex", 6);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer Validation failed!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (framebufferResized)
        {
            glViewport(0, 0, windowWidth, windowHeight);

            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);

            framebufferResized = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        
        lightingShader.use();
        
        lightingShader.setBool("drawSingleColor", false);

        lightingShader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(lightingShader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++) {
            std::string baseName = "pointLights[" + std::to_string(i) +"].";
            lightingShader.setVec3(baseName + "position", pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);

            lightingShader.setVec3(baseName + "ambient", 0.05f, 0.05f, 0.05f);
            lightingShader.setVec3(baseName + "diffuse", 0.8f, 0.8f, 0.8f);
            lightingShader.setVec3(baseName + "specular", 1.0f, 1.0f, 1.0f);

            lightingShader.setFloat(baseName + "constant", 1.0f);
            lightingShader.setFloat(baseName + "linear", 0.09f);
            lightingShader.setFloat(baseName + "quadratic", 0.032f);
        }

        lightingShader.setVec3("spotLight.position", camera.Position.x, camera.Position.y, camera.Position.z);
        lightingShader.setVec3("spotLight.direction", camera.Front.x, camera.Front.y, camera.Front.z);

        lightingShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);

        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);

        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        glm::mat4 model = glm::mat4(1.0f);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model"),
        viewLoc = glGetUniformLocation(lightingShader.ID, "view"),
        projLoc = glGetUniformLocation(lightingShader.ID, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        const float spacing = 1.5f;
        auto cubeModel = [&](unsigned int i, float scale) {
            float x = (i - (cubeTextures.size() - 1) / 2.0f) * spacing;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, 0.0f));
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(25.0f), glm::vec3(0.5f, 1.0f, 0.0f));
            return glm::scale(model, glm::vec3(scale));
        };

        lightingShader.setFloat("heightScale", 0.025f);

        /*

        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < cubeTextures.size(); i++)
        {
            const Material &material = cubeTextures[i].material;

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i].diffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i].specular);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i].normal);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i].displacement);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, cubeTextures[i].ambient);

            lightingShader.setVec3("material.ambient", material.ambient.x, material.ambient.y, material.ambient.z);
            lightingShader.setFloat("material.shininess", material.shininess * 128.0f);
            lightingShader.setFloat("material.reflectivity", material.reflectivity);
            lightingShader.setFloat("material.refractiveIndex", material.refractiveIndex);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel(i, 1.0f)));
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        */

        glStencilMask(0x00);
        lightingShader.setFloat("heightScale", 0.025f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        for (unsigned int type = PEPE; type <= ICE; type++)
        {
            const VoxelBatch &batch = voxelBatches[type];
            if (batch.count == 0 || blockTexture[type] < 0)
                continue;
            
                const CubeSetup &blockSet = cubeTextures[blockTexture[type]];
                const Material &material = blockSet.material;

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, blockSet.diffuse);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, blockSet.specular);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, blockSet.normal);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, blockSet.displacement);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, blockSet.ambient);

                lightingShader.setVec3("material.ambient", material.ambient.x, material.ambient.y, material.ambient.z);
                lightingShader.setFloat("material.shininess", material.shininess * 128.0f);
                lightingShader.setFloat("material.reflectivity", material.reflectivity);
                lightingShader.setFloat("material.refractiveIndex", material.refractiveIndex);

                glBindVertexArray(batch.vao);
                // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArraysInstanced(GL_TRIANGLES, 0, 36, batch.count);
        }

        glBindVertexArray(cubeVAO);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        lightingShader.use();
        lightingShader.setBool("drawSingleColor", true);
        lightingShader.setVec3("singleColor", 0.0f, 0.5f, 0.0f);

        for (unsigned int i = 0; i < cubeTextures.size(); i++)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel(i, 1.05f)));
            // glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);

        lightCubeShader.use();

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        modelLoc = glGetUniformLocation(lightCubeShader.ID, "model");
        viewLoc = glGetUniformLocation(lightCubeShader.ID, "view");
        projLoc = glGetUniformLocation(lightCubeShader.ID, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(lightCubeVAO);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);

        std::multimap<float, glm::vec3> sortedWindows;
        for(unsigned int i = 0; i < sizeof(windowPositions) / sizeof(windowPositions[0]) ; i++)
        {
            float dist = glm::length(camera.Position - windowPositions[i]);
            sortedWindows.insert({dist, windowPositions[i]});
        }

        glDisable(GL_CULL_FACE);

        lightingShader.use();
        lightingShader.setBool("drawSingleColor", false);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windowTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, windowSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, windowNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, windowDisplacement);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, windowAmbient);

        lightingShader.setFloat("heightScale", 0.0f);

        lightingShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setFloat("material.reflectivity", 0.0f);
        lightingShader.setFloat("material.refractiveIndex", 0.0f);

        glBindVertexArray(transparentVAO);

        unsigned int windowModelLoc = glGetUniformLocation(lightingShader.ID, "model");

        for(auto i = sortedWindows.rbegin(); i != sortedWindows.rend(); ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, i -> second);

            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniformMatrix4fv(windowModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glEnable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenShader.setInt("effectMode", effectMode);

        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE6);

        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &quadVAO);
    
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteBuffers(1, &quadVBO);
    
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &rbo);

    glDeleteTextures(1, &textureColorBuffer);
    for (CubeSetup &c : cubeTextures)
    {
        glDeleteTextures(1, &c.diffuse);
        glDeleteTextures(1, &c.specular);
        glDeleteTextures(1, &c.normal);
        glDeleteTextures(1, &c.displacement);
        glDeleteTextures(1, &c.ambient);
    }
    glDeleteTextures(1, &windowTexture);
    glDeleteTextures(1, &windowSpecular);
    glDeleteTextures(1, &windowNormal);
    glDeleteTextures(1, &windowDisplacement);
    glDeleteTextures(1, &windowAmbient);
    glDeleteTextures(1, &cubemapTexture);

    glDeleteProgram(lightingShader.ID);
    glDeleteProgram(lightCubeShader.ID);
    glDeleteProgram(screenShader.ID);
    glDeleteProgram(skyboxShader.ID);

    for (VoxelBatch &batch : voxelBatches)
    {
        glDeleteVertexArrays(1, &batch.vao);
        glDeleteBuffers(1, &batch.vbo);
    }
    
    glfwTerminate();
    return 0;
}

std::vector<float> buildVertexData(const float* inputVertices, size_t vertexCount)
{
    const int inputFPV = 8, outputFPV = 14; // FPV = Floats Per Vertex

    std::vector<float> outputVertices;
    outputVertices.reserve(vertexCount * outputFPV);

    for (size_t i = 0; i + 2 < vertexCount; i += 3)
    {
        auto readPos = [&](size_t k) // k = Corners
        {
            size_t base = (i + k) * inputFPV;
            return glm::vec3(inputVertices[base], inputVertices[base + 1], inputVertices[base + 2]);
        };

        auto readTexCoord = [&](size_t k)
        {
            size_t base = (i + k) * inputFPV;
            return glm::vec2(inputVertices[base + 6], inputVertices[base + 7]);
        };

        glm::vec3 edgeK2 = readPos(1) - readPos(0);
        glm::vec3 edgeK3 = readPos(2) - readPos(0);

        glm::vec2 uvK2 = readTexCoord(1) - readTexCoord(0);
        glm::vec2 uvK3 = readTexCoord(2) - readTexCoord(0);

        float inverseFactor = 1.0f / (uvK2.x * uvK3.y - uvK3.x * uvK2.y);

        glm::vec3 tangent = glm::normalize(inverseFactor * (uvK3.y * edgeK2 - uvK2.y * edgeK3));
        glm::vec3 bitangent = glm::normalize(inverseFactor * (-uvK3.x * edgeK2 + uvK2.x * edgeK3));

        for (size_t k = 0; k < 3; k++)
        {
            size_t base = (i + k) * inputFPV;

            for (int j = 0; j < inputFPV; j++)
                outputVertices.push_back(inputVertices[base + j]);

            outputVertices.push_back(tangent.x);
            outputVertices.push_back(tangent.y);
            outputVertices.push_back(tangent.z);

            outputVertices.push_back(bitangent.x);
            outputVertices.push_back(bitangent.y);
            outputVertices.push_back(bitangent.z);
        }
    }

    return outputVertices;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum imageFormat = GL_RGB;
        if (nrChannels == 1) imageFormat = GL_RED;
        if (nrChannels == 3) imageFormat = GL_RGB;
        if (nrChannels == 4) imageFormat = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, & textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum imageFormat = GL_RGB;
            if (nrChannels == 1) imageFormat = GL_RED;
            if (nrChannels == 3) imageFormat = GL_RGB;
            if (nrChannels == 4) imageFormat = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0)
        return;
    
    windowWidth = width;
    windowHeight = height;
    framebufferResized = true;
}

bool iKeyPressed = false;

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        if (!iKeyPressed)
        {
            effectMode = (effectMode + 1) % 5;
            iKeyPressed = true;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
        iKeyPressed = false;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn), ypos = static_cast<float>(yposIn);

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX, yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}