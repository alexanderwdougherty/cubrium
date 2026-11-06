// main.cpp

# include <glad/glad.h>
# include <GLFW/glfw3.h>

# include "shader_s.h"
# include "camera.h"
# include "materials.h"

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

const unsigned int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f, lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f, lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

const int effectMode = 0;

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

    Shader lightingShader("shader.vs", "shader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader screenShader("screen.vs", "screen.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");

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
        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,      0.0f, 0.0f, -1.0f,      1.0f, 0.0f,

        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,      0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,       0.0f, 0.0f, -1.0f,      1.0f, 1.0f
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
        glm::vec3(1.5f, 0.0f, 0.5f),
        glm::vec3(0.0f, 0.0f, 1.7f),
        glm::vec3(-0.5f, 0.0f, -2.3f),
        glm::vec3(1.5f, 0.0f, -0.6f)
    };

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

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

    std::vector<std::string> skyboxFaces = {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg"
    };

    unsigned int diffuseMap = loadTexture("assets/pepe.jpg"),
    specularMap = loadTexture("assets/pepe_specular.png"),
    windowTexture = loadTexture("assets/window.png"),
    windowSpecular = loadTexture("assets/window_specular.png"),
    cubemapTexture = loadCubemap(skyboxFaces);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("skybox", 3);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 3);

    screenShader.use();
    screenShader.setInt("screenTex", 2);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
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

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        
        lightingShader.use();

        lightingShader.setInt("material.diffuse", 0);
        lightingShader.setInt("material.specular", 1);
        
        lightingShader.setBool("drawSingleColor", false);

        lightingShader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(lightingShader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        Material currentMaterial = Materials::Chrome;

        lightingShader.setVec3("material.ambient", currentMaterial.ambient.x, currentMaterial.ambient.y, currentMaterial.ambient.z);
        lightingShader.setFloat("material.shininess", currentMaterial.shininess * 128.0f);
        lightingShader.setFloat("material.reflectivity", currentMaterial.reflectivity);
        lightingShader.setFloat("material.refractiveIndex", currentMaterial.refractiveIndex);

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
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(25.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model"),
        viewLoc = glGetUniformLocation(lightingShader.ID, "view"),
        projLoc = glGetUniformLocation(lightingShader.ID, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        glBindVertexArray(cubeVAO);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        lightingShader.use();
        lightingShader.setBool("drawSingleColor", true);
        lightingShader.setVec3("singleColor", 0.0f, 0.5f, 0.0f);

        glm::mat4 outlineModel = glm::mat4(1.0f);
        outlineModel = glm::rotate(outlineModel, (float)glfwGetTime() * glm::radians(25.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        outlineModel = glm::scale(outlineModel, glm::vec3(1.04f, 1.04f, 1.04f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));

        glDrawArrays(GL_TRIANGLES, 0, 36);

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
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);

        std::map<float, glm::vec3> sortedWindows;
        for(unsigned int i = 0; i < sizeof(windowPositions) / sizeof(windowPositions[0]) ; i++)
        {
            float dist = glm::length(camera.Position - windowPositions[i]);
            sortedWindows[dist] = windowPositions[i];
        }

        glDisable(GL_CULL_FACE);

        lightingShader.use();
        lightingShader.setBool("drawSingleColor", false);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windowTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, windowSpecular);

        lightingShader.setFloat("material.reflectivity", 0.0f);
        lightingShader.setFloat("material.refractiveIndex", 0.0f);

        glBindVertexArray(transparentVAO);

        unsigned int windowModelLoc = glGetUniformLocation(lightingShader.ID, "model");

        for(std::map<float, glm::vec3>::reverse_iterator it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it -> second);

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
        glActiveTexture(GL_TEXTURE2);

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
    glDeleteFramebuffers(1, &rbo);

    glDeleteTextures(1, &textureColorBuffer);
    glDeleteTextures(1, &diffuseMap);
    glDeleteTextures(1, &specularMap);
    glDeleteTextures(1, &windowTexture);
    glDeleteTextures(1, &windowSpecular);
    glDeleteTextures(1, &cubemapTexture);

    glDeleteProgram(lightingShader.ID);
    glDeleteProgram(lightCubeShader.ID);
    glDeleteProgram(screenShader.ID);
    glDeleteProgram(skyboxShader.ID);
    
    glfwTerminate();
    return 0;
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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