#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <list>

#include "shader.h"
#include "geometry.h"
#include "camera.h"
#include "renderer.h"
#include "texture.h"
#include "projectile.h"

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs
#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>

using namespace std;
using namespace glm;

struct Tower {
    vec3 position;
    float height;
};

// Texture Index
// -------------
constexpr int ROAD_TEX_SLOT = 0;
constexpr int BUILDING_TEX_SLOT = 1;
constexpr int LAMP_TEX_SLOT = 2;
constexpr int LASER_TEX_SLOT = 3;

// Variables to call and define later
// ----------------------------------
GLFWwindow* window = nullptr;
float spinningCubeAngle = 0.0f;
Geometry geometry;
GLuint lightCubeVAO;
Shader* lightCubeShader;
vector<Tower> towerList;
list<Projectile> projectileList;

// Methods to call and define later
// --------------------------------
void processInput(GLFWwindow *window);
bool InitContext();
void renderScene(Shader& shader, const vector<Tower>& towers, GLuint vao, GLuint groundTex, GLuint buildingTex);
void renderLightCubes(Shader& shader, GLuint vao, const vec3& pos1, const vec3& pos2, GLuint tex);
void renderProjectiles(Shader& shader, GLuint tex);
void renderAvatar(Shader& shader);

// Screen Settings
// ---------------
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera Settings for View Transfom
// ---------------------------------
Camera camera;
bool cameraFirstPerson = true;
float dt;

// Frame Parameters + Mouse Parameters
// -----------------------------------
float lastFrameTime;
int lastMouseLeftState;
double lastMousePosX, lastMousePosY;

// Main Function
// -------------
int main(){
    
    // Initialize GLFW and OpenGL version
    // ----------------------------------
    if (!InitContext()) return -1;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Configure Global openGL State
    // -----------------------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Load and Create Textures
    // ------------------------
    GLuint roadTextureID = Texture::load("Textures/road.jpg");
    GLuint buildingTextureID = Texture::load("Textures/building.jpg");
    GLuint lampTextureID = Texture::load("Textures/lamp.png");
    GLuint laserTextureID = Texture::load("Textures/laser.png");

    // Build and Compile and Link Shaders
    // ----------------------------------
    Shader lightingShaderProgram("Shaders/Phong.vert", "Shaders/Phong.frag");
    lightCubeShader = &lightingShaderProgram;

    // Manage Building Postions Generation
    // -----------------------------------
    srand(static_cast<unsigned>(time(0))); // RNG
    int numTowers = 100;
    float minDist = 5.0f;
    float maxRange = 40.0f;
    for (int i = 0; i < numTowers; ++i) {
        float x = static_cast<float>((rand() % static_cast<int>(2 * maxRange)) - static_cast<int>(maxRange));
        float z = static_cast<float>((rand() % static_cast<int>(2 * maxRange)) - static_cast<int>(maxRange));
        if (glm::length(glm::vec2(x, z)) < minDist) continue;
        float height = 5.0f + static_cast<float>(rand() % 20);
        towerList.push_back({ glm::vec3(x, 0.0f, z), height });
    }

    // Set up Models
    // -------------

    // Set initial transformation matrices to shaders
    mat4 projectionMatrix = glm::perspective(radians(70.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.03f, 800.0f);
    Renderer::setProjectionMatrix(lightingShaderProgram.getID(), projectionMatrix);
    mat4 identity = mat4(1.0f);
    Renderer::setWorldMatrix(lightingShaderProgram.getID(), identity);

    // Set up Vertex Data (buffers)
    // ----------------------------
    lightCubeVAO = geometry.createLightCube();

    // Frame time calculations for mouse (Comes with Frame Parameters at the top of this file)
    // ---------------------------------------------------------------------------------------
    lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Light cube parameters
    // ---------------------
    vec3 center = vec3(0.0f, 5.0f, 0.0f);
    float radius = 10.0f;
    auto getLightPos = [&](float timeOffset, float time) -> vec3 {
        return vec3(cos(time + timeOffset) * radius, 0.0f, sin(time + timeOffset) * radius) + center;
    };

    // Render Loop
    // -----------
    while(!glfwWindowShouldClose(window)){
        // Frame time calculation
        // ----------------------
        dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;

        // Process Input
        // -------------
        processInput(window);
        
        // Renderer Clear
        // --------------
        Renderer::clear(); // Clears color and depth buffers

        // Activate Shader to draw with colors or textures
        // -----------------------------------------------
        float time = glfwGetTime();
        vec3 center = vec3(0.0f, 5.0f, 0.0f);
        float radius = 10.0f;
        vec3 lightPos1 = getLightPos(0.0f, time);
        vec3 lightPos2 = getLightPos(3.14f, time);

        lightingShaderProgram.use();
        Renderer::setViewMatrix(lightingShaderProgram.getID(), camera.getViewMatrix());

        // Set lighting uniforms
        lightingShaderProgram.setVec3("lightPos1", lightPos1);
        lightingShaderProgram.setVec3("lightPos2", lightPos2);
        lightingShaderProgram.setVec3("viewPos", camera.getPosition());
        
        // Render the scene
        // ----------------
        renderScene(lightingShaderProgram, towerList, lightCubeVAO, roadTextureID, buildingTextureID);
        // Render the light cubes
        // ----------------------
        renderLightCubes(*lightCubeShader, lightCubeVAO, lightPos1, lightPos2, lampTextureID);
        // Render the projectiles
        // ----------------------
        renderProjectiles(lightingShaderProgram, laserTextureID);
        // Render the avatar
        // -----------------
        renderAvatar(lightingShaderProgram);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        //  Update camera horizontal and vertical angle
        // ---------------------------------------------
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        camera.updateOrientation(mousePosX, mousePosY, dt);


    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// ----------------------------------------------------------------------------------------------------------------------------------------------
//                                                       DEFINE METHODS AND UTILITIES
// ----------------------------------------------------------------------------------------------------------------------------------------------

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    // Exit the appplication
    // ---------------------
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 
    // Toggle between camera modes
    // ---------------------------
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        cameraFirstPerson = true;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        cameraFirstPerson = false;
    // Use camera lookat and side vectors to update positions with ASDW + SHIFT
    // ------------------------------------------------------------------------
    camera.processInput(window);
}

// Draw the scene, ground, buildings and so on
// -------------------------------------------
void renderScene(Shader& shader, const vector<Tower>& towers, GLuint vao, GLuint groundTex, GLuint buildingTex) {
    mat4 identity = mat4(1.0f);

    mat4 groundMatrix = glm::scale(glm::translate(identity, vec3(0.0f, -1.0f, 0.0f)), vec3(100.0f, 0.1f, 100.0f));
    shader.use();
    Renderer::bindTexture(shader.getID(), groundTex, "textureSampler", ROAD_TEX_SLOT);
    Renderer::setWorldMatrix(shader.getID(), groundMatrix);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    Renderer::bindTexture(shader.getID(), buildingTex, "textureSampler", BUILDING_TEX_SLOT);
    for (const auto& tower : towers) {
        mat4 model = glm::scale(glm::translate(identity, tower.position), vec3(2.0f, tower.height, 2.0f));
        Renderer::setWorldMatrix(shader.getID(), model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

// Draw orbiting light cubes in the scene
// --------------------------------------
void renderLightCubes(Shader& shader, GLuint vao, const vec3& pos1, const vec3& pos2, GLuint tex) {
    mat4 identity = mat4(1.0f);
    shader.use();
    Renderer::bindTexture(shader.getID(), tex, "textureSampler", LAMP_TEX_SLOT);

    auto drawCube = [&](const vec3& pos) {
        mat4 model = glm::scale(glm::translate(identity, pos), vec3(0.5f));
        shader.setMat4("worldMatrix", model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };
    drawCube(pos1);
    drawCube(pos2);
}

// Draw Projectiles as they are shot
// ---------------------------------
void renderProjectiles(Shader& shader, GLuint tex){
    shader.use();
    Renderer::bindTexture(shader.getID(), tex, "textureSampler", LASER_TEX_SLOT);
    // Update and draw projectiles
    for (list<Projectile>::iterator it = projectileList.begin(); it != projectileList.end(); ++it)
    {
        it->Update(dt);
        it->Draw();
    }

    // Shoot projectiles on mouse left click
    if (lastMouseLeftState == GLFW_RELEASE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        const float projectileSpeed = 25.0f;
        vec3 direction = normalize(camera.getlookAt());
        vec3 velocity = direction * projectileSpeed;
        vec3 spawnPosition = camera.getPosition() + direction * 2.0f;
        
        projectileList.push_back(Projectile(spawnPosition,velocity ,  shader.getID()));
    }
    lastMouseLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
}

// Draw avatar in 1st or 3rd person
// --------------------------------
void renderAvatar(Shader& shader){
        spinningCubeAngle += 180.0f * dt;
        // Draw avatar in view space for first person camera
        // and in world space for third person camera
        if (cameraFirstPerson){
            mat4 spinningCubeViewMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.5f)) *
                                          rotate(mat4(1.0f), radians(spinningCubeAngle), vec3(0.0f, 1.0f, 0.0f)) *
                                          scale(mat4(1.0f), vec3(0.05f));
            
            Renderer::setWorldMatrix(shader.getID(), mat4(1.0f));
            Renderer::setViewMatrix(shader.getID(), spinningCubeViewMatrix);
        }
        else{
            vec3 avatarOffset = normalize(camera.getlookAt()) * 2.0f;
            vec3 avatarPosition = camera.getPosition() + avatarOffset;

            mat4 spinningCubeWorldMatrix = translate(mat4(1.0f), avatarPosition) *
                                           rotate(mat4(1.0f), radians(spinningCubeAngle), vec3(0.0f, 1.0f, 0.0f)) *
                                           scale(mat4(1.0f), vec3(0.3f));
            
            Renderer::setWorldMatrix(shader.getID(), spinningCubeWorldMatrix);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Set the view matrix for first and third person cameras
        // - In first person, camera lookat is set like below
        // - In third person, camera position is on a sphere looking towards center
        mat4 viewMatrix(1.0f);
        
        if (cameraFirstPerson){
            viewMatrix = lookAt(camera.getPosition(), camera.getPosition() + camera.getlookAt(), camera.getUp());
        }
        else{
            // Position of the camera is on the sphere looking at the point of interest (cameraPosition)
            float radius = 5.0f;
            vec3 position = camera.getPosition() - vec3(radius * cosf(camera.getPhi())*cosf(camera.getTheta()),
                                                  radius * sinf(camera.getPhi()),
                                                  -radius * cosf(camera.getPhi())*sinf(camera.getTheta()));
            viewMatrix = lookAt(position, camera.getPosition(), camera.getUp());
        }
        Renderer::setViewMatrix(shader.getID(), viewMatrix);
}


// Initialize the libraries and window
// -----------------------------------
bool InitContext() {
    glfwInit();
    
    #if defined(PLATFORM_OSX)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #else
        // On windows, we set OpenGL version to 2.1, to support more hardware
        // ------------------------------------------------------------------
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    #endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    // ------------------------------------------------------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Comp 371 - Project", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Tell GLFW to capture mouse movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwMakeContextCurrent(window);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Initialize GLEW
    // ---------------
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    cout << "INITIALIZING WINDOW: SUCCESS" << endl;
    return true;
}