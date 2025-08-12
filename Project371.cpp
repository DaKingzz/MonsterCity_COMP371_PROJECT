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
#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"  //For loading .obj files using a polygon list format

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs
#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtx/norm.hpp>

using namespace std;
using namespace glm;

// Transform helper functions for Turret
// -------------------------------------
static mat4 T(const glm::vec3& p){ return glm::translate(glm::mat4(1.0f), p); }
static mat4 RX(float deg){ return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(1,0,0)); }
static mat4 RY(float deg){ return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(0,1,0)); }
static mat4 S(const glm::vec3& s){ return glm::scale(glm::mat4(1.0f), s); }


struct Tower {
    vec3 position;
    float height;
};

// Texture Index
// -------------
constexpr int GRASS_TEX_SLOT = 0;
constexpr int BUILDING_TEX_SLOT = 1;
constexpr int METAL_TEX_SLOT = 2;
constexpr int LAMP_TEX_SLOT = 3;
constexpr int LASER_TEX_SLOT = 4;
constexpr int MONSTER_TEX_SLOT = 5;
constexpr int JACK_O_LANTERN_TEX_SLOT = 6;
constexpr int MESH_TEX_SLOT = 7;
constexpr int GLOWSTONE_TEX_SLOT = 8;
int CURRENT_CUBE_TEX_SLOT;

//Texture ID declaration
//-----------------------
GLuint grassTextureID;
GLuint buildingTextureID ;
GLuint lampTextureID;
GLuint laserTextureID ;
GLuint monsterTextureID ;
GLuint jack_o_lanternTextureID ;
GLuint meshTextureID ;
GLuint glowstoneTextureID ;

// Flying cube texture & color control
//-------------------------------------
int flyingCubeTextureID;
glm::vec3 flyingCubeColor(1.0f, 1.0f, 1.0f); // default white
bool kKeyPressed = false; // to avoid multiple toggles per press

// Variables to call and define later
// ----------------------------------
GLFWwindow* window = nullptr;
float spinningCubeAngle = 0.0f;
Geometry geometry;
GLuint lightCubeVAO;
Shader* lightCubeShader;
vector<Tower> towerList;
list<Projectile> projectileList;

// Turret state varaibles
// ----------------------
float gTurretBaseYawDeg = 0.0f;        // updated every frame
float gTurretBarrelZDeg = 0.0f;        // controlled by Q/E, clamped to [-45, +45]
GLuint gMetalTexID = 0;                // set after loading textures
vec3 gTurretBasePos = glm::vec3(0.0f, 0.0f, 5.0f); // Right in front of the monster

// Methods to call and define later
// --------------------------------
void processInput(GLFWwindow *window);
bool InitContext();
void renderScene(Shader& shader, const vector<Tower>& towers, GLuint vao, GLuint groundTex, GLuint buildingTex);
void renderLightCubes(Shader& shader, GLuint vao, const vec3& pos1, const vec3& pos2, GLuint tex);
void renderProjectiles(Shader& shader, GLuint tex);
void renderAvatar(Shader& shader);
void renderMonster(Shader& shader, GLuint stoneVAO, int stoneVertices, GLuint tex, vec3 lightPos1, vec3 lightPos2);
void renderSceneFromLight(Shader& shadowShader, const std::vector<Tower>& towers, GLuint cubeVAO);
void renderMonsterFromLight(Shader& shadowShader, GLuint monsterVAO, int monsterVertexCount);
void renderTurret(Shader& shader, GLuint cubeVAO, const glm::mat4& parentWorld, float baseYawDeg, float barrelZDeg, GLuint metalTexID);
void renderTurretShadow(Shader& shadowShader, GLuint cubeVAO, const glm::mat4& parentWorld, float baseYawDeg, float barrelZDeg);
static void computeTurretBarrelTipAndDir(const mat4& parentWorld, float baseYawDeg, float barrelZDeg, vec3& outTip, vec3& outDir);
GLuint setupModelVBO(string path, int& vertexCount);
GLuint setupModelEBO(string path, int& vertexCount);

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

// Monster state (position, scale, radius)
// ---------------------------------------
glm::vec3 gMonsterPos = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr float gMonsterScale = 0.5f;     // matches your render scale
constexpr float gMonsterRadiusLocal = 2.0f; // tweak to fit your Stone.obj bounds
inline float getMonsterRadiusWorld() {
    return gMonsterRadiusLocal * gMonsterScale;
}

// Squared distance between XZ points
// ----------------------------------
float dist2_xz(const glm::vec3& a, const vec3& b) {
    vec2 da(a.x, a.z), db(b.x, b.z);
    vec2 d = da - db;
    return glm::dot(d, d);
}

// Segment–sphere intersection (finite beam)
// -----------------------------------------
bool segmentHitsSphere(const glm::vec3& A, const glm::vec3& B,
                              const glm::vec3& C, float R)
{
    vec3 AB = B - A;
    float ab2 = dot(AB, AB);
    if (ab2 == 0.0f) return glm::length2(C - A) <= R*R;
    float t = dot(C - A, AB) / ab2;
    t = clamp(t, 0.0f, 1.0f);
    vec3 closest = A + t * AB;
    return length2(C - closest) <= R*R;
}

// Random spawn away from center and towers
// ----------------------------------------
vec3 randomMonsterSpawnNearCamera(const vec3& camPos, float minDist = 8.0f, float maxDist = 22.0f) {
    for (int tries = 0; tries < 64; ++tries) {
        float ang = ((float)rand() / RAND_MAX) * 6.2831853f;           // [0, 2π)
        float rad = minDist + ((float)rand() / RAND_MAX) * (maxDist - minDist);
        vec3 p = camPos + vec3(std::cos(ang)*rad, 0.0f, std::sin(ang)*rad);

        // keep away from towers a bit
        bool ok = true;
        for (const auto& t : towerList) {
            if (dist2_xz(p, t.position) < (2.5f * 2.5f)) { ok = false; break; }
        }
        if (ok) return p;
    }
    return camPos + vec3(maxDist, 0.0f, 0.0f); // fallback
}

// Respawn Monster
// ---------------
void respawnMonster() {
    gMonsterPos = randomMonsterSpawnNearCamera(camera.getPosition());
}

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
    GLuint grassTextureID = Texture::load("Textures/grass.jpg");
    GLuint buildingTextureID = Texture::load("Textures/building.jpg");
    GLuint metalTextureID = Texture::load("Textures/metal.jpg");
    GLuint lampTextureID = Texture::load("Textures/lamp.png");
    GLuint laserTextureID = Texture::load("Textures/laser.png");
    GLuint monsterTextureID = Texture::load("Textures/sand.jpg");
    GLuint jack_o_lanternTextureID = Texture::load("Textures/Jack_O_Lantern.png");
    GLuint meshTextureID = Texture::load("Textures/mesh.png");
    GLuint glowstoneTextureID = Texture::load("Textures/Glowstone.jpg");

    //Sets default textures
    //---------------------------------
    CURRENT_CUBE_TEX_SLOT = LAMP_TEX_SLOT;    
    flyingCubeTextureID = lampTextureID;
    gMetalTexID = metalTextureID; 
    
    // Create Framebuffer for shawfow mapping
    // --------------------------------------
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // Create the depth texture
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Build and Compile and Link Shaders
    // ----------------------------------
    Shader lightingShaderProgram("Shaders/Phong.vert", "Shaders/Phong.frag");
    Shader monsterShaderProgram("Shaders/Monster.vert","Shaders/Monster.frag");
    Shader shadowShaderProgram("Shaders/ShadowDepth.vert", "Shaders/ShadowDepth.frag");
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

    respawnMonster();

    // Set up Models
    // -------------
    string monsterPath = "Models/Stone.obj";
    int stoneVertices;

    GLuint stoneVAO = setupModelVBO(monsterPath, stoneVertices);

    // Set initial transformation matrices to shaders
    // ----------------------------------------------
    mat4 projectionMatrix = glm::perspective(radians(70.0f), SCR_WIDTH * 1.0f / SCR_HEIGHT, 0.03f, 800.0f);
    Renderer::setProjectionMatrix(lightingShaderProgram.getID(), projectionMatrix);
    Renderer::setProjectionMatrix(monsterShaderProgram.getID(), projectionMatrix);
    mat4 identity = mat4(1.0f);
    Renderer::setWorldMatrix(lightingShaderProgram.getID(), identity);
    Renderer::setWorldMatrix(monsterShaderProgram.getID(), identity);

    // Set up Vertex Data (buffers)
    // ----------------------------
    lightCubeVAO = geometry.createLightCube();

    // Frame time calculations for mouse (Comes with Frame Parameters at the top of this file)
    // ---------------------------------------------------------------------------------------
    lastFrameTime = glfwGetTime();
    lastMouseLeftState = GLFW_RELEASE;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Light cube parameters
    // ---------------------
    vec3 center = vec3(0.0f, 5.0f, 0.0f);
    float radius = 10.0f;
    auto getLightPos = [&](float timeOffset, float time) -> vec3 {
        return vec3(cos(time + timeOffset) * radius, 0.0f, sin(time + timeOffset) * radius) + center;
    };

    // Initialize some turret variables
    vec3 f = normalize(camera.getlookAt());
    gTurretBaseYawDeg = degrees(atan2(f.x, f.z));
    // Pitch in degrees: asin(y / length)
    gTurretBarrelZDeg = degrees(asin(clamp(f.y, -1.0f, 1.0f)));
    // Clamp rotation limits right away
    gTurretBarrelZDeg = clamp(gTurretBarrelZDeg, -75.0f, 75.0f);

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

        // Light Cube Variables
        // --------------------
        float time = glfwGetTime();
        vec3 center = vec3(0.0f, 5.0f, 0.0f);
        float radius = 10.0f;
        vec3 lightPos1 = getLightPos(0.0f, time);
        vec3 lightPos2 = getLightPos(3.14f, time);

        // Shadow Pass
        // -----------
        glm::vec3 lightPos = getLightPos(0.0f, time);
        glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 1.0f, 100.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // Render to depth map
        // -------------------
        shadowShaderProgram.use();
        shadowShaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Draw cubes (ground/buildings) with front-face culling to reduce acne
        glEnable(GL_CULL_FACE);
        GLint prevCull; glGetIntegerv(GL_CULL_FACE_MODE, &prevCull);
        glCullFace(GL_FRONT);

        renderSceneFromLight(shadowShaderProgram, towerList, lightCubeVAO);

        // Turret into the shadow map
        // --------------------------
        glm::mat4 turretParentWorld = T(gTurretBasePos);
        // Aim at the camera
        f = normalize(camera.getlookAt());
        gTurretBaseYawDeg = degrees(std::atan2(f.x, f.z));
        renderTurretShadow(shadowShaderProgram, lightCubeVAO, turretParentWorld, gTurretBaseYawDeg, gTurretBarrelZDeg);

        // Draw the monster into the depth map too.
        // Disable culling for safety
        glDisable(GL_CULL_FACE);
        renderMonsterFromLight(shadowShaderProgram, stoneVAO, stoneVertices);

        // Restore state
        glEnable(GL_CULL_FACE);
        glCullFace(prevCull);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Light Pass + Renderer Clear
        // ---------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        Renderer::clear(); // Clears color and depth buffers

        // Activate Shader to draw with colors or textures
        // -----------------------------------------------
        lightingShaderProgram.use();
        Renderer::setViewMatrix(lightingShaderProgram.getID(), camera.getViewMatrix());

        // Set lighting uniforms
        lightingShaderProgram.setVec3("lightPos1", lightPos1);
        lightingShaderProgram.setVec3("lightPos2", lightPos2);
        lightingShaderProgram.setVec3("viewPos", camera.getPosition());
        lightingShaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glActiveTexture(GL_TEXTURE14); // set to free unit
        glBindTexture(GL_TEXTURE_2D, depthMap);
        lightingShaderProgram.setInt("shadowMap", 14);

        
        // Render the scene
        // ----------------
        renderScene(lightingShaderProgram, towerList, lightCubeVAO, grassTextureID, buildingTextureID);
        // Render the turret
        turretParentWorld = T(gTurretBasePos);
        f = normalize(camera.getlookAt());
        gTurretBaseYawDeg = degrees(std::atan2(f.x, f.z));
        renderTurret(lightingShaderProgram, lightCubeVAO, turretParentWorld, gTurretBaseYawDeg, gTurretBarrelZDeg, gMetalTexID);
        // Compute turret tip & dir
        vec3 turretTip, turretDir;
        computeTurretBarrelTipAndDir(T(gTurretBasePos), gTurretBaseYawDeg, gTurretBarrelZDeg, turretTip, turretDir);

        // Handle left click edge: spawn two projectiles (camera + turret)
        // ---------------------------------------------------------------
        if (lastMouseLeftState == GLFW_RELEASE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            const float projectileSpeed = 25.0f;
            // From the flying cube (your existing behavior)
            /* We dont want this anymore
            {
                vec3 direction = glm::normalize(camera.getlookAt());
                vec3 velocity = direction * projectileSpeed;
                vec3 spawnPosition = camera.getPosition() + direction * 2.0f;
                projectileList.push_back(Projectile(spawnPosition, velocity, lightingShaderProgram.getID()));
            }
            */
            // From the turret barrel tip
            {
                vec3 velocity = turretDir * projectileSpeed;
                vec3 spawnPosition = turretTip + turretDir * 0.2f; // nudge forward to avoid self-collision
                projectileList.push_back(Projectile(spawnPosition, velocity, lightingShaderProgram.getID()));
            }
        }
        lastMouseLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        // Render the light cubes
        // ----------------------
        renderLightCubes(*lightCubeShader, lightCubeVAO, lightPos1, lightPos2, flyingCubeTextureID);
        // Render the projectiles
        // ----------------------
        renderProjectiles(lightingShaderProgram, laserTextureID);
        // Render the avatar
        // -----------------
        renderAvatar(lightingShaderProgram);
        // Render the monster using a model
        // --------------------------------
        Renderer::setViewMatrix(monsterShaderProgram.getID(), camera.getViewMatrix());
        monsterShaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        monsterShaderProgram.setInt("shadowMap", 14);
        renderMonster(monsterShaderProgram, stoneVAO, stoneVertices, monsterTextureID, lightPos1, lightPos2);

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

// Draw the scene, ground, buildings and so on
// -------------------------------------------
void renderScene(Shader& shader, const vector<Tower>& towers, GLuint vao, GLuint groundTex, GLuint buildingTex) {
    mat4 identity = mat4(1.0f);

    shader.setVec3("overrideColor", glm::vec3(1.0f));
    mat4 groundMatrix = glm::scale(glm::translate(identity, vec3(0.0f, -1.0f, 0.0f)), vec3(100.0f, 0.1f, 100.0f));
    shader.use();
    Renderer::bindTexture(shader.getID(), groundTex, "textureSampler", GRASS_TEX_SLOT);
    Renderer::setWorldMatrix(shader.getID(), groundMatrix);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.setVec3("overrideColor", glm::vec3(1.0f));

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
    shader.setVec3("overrideColor",flyingCubeColor);
    Renderer::bindTexture(shader.getID(), tex, "textureSampler",CURRENT_CUBE_TEX_SLOT);

    auto drawCube = [&](const vec3& pos) {
        mat4 model = scale(translate(identity, pos), vec3(0.5f));
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
    for (auto it = projectileList.begin(); it != projectileList.end(); /* no ++ here */) {
        it->Update(dt);
        it->Draw();

        const glm::vec3& prev = it->prevPosition();
        const glm::vec3& curr = it->position();
        const float R = getMonsterRadiusWorld();

        // hit test against monster (segment vs sphere)
        if (segmentHitsSphere(prev, curr, gMonsterPos, R)) {
            respawnMonster();                        // move monster
            it = projectileList.erase(it);           // erase returns next iterator
            continue;
        }

        // lifetime cull
        if (glm::length2(curr) > 800.0f * 800.0f) {
            it = projectileList.erase(it);
            continue;
        }
        ++it; // only increment when we kept the element
    }
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

// Render monster using an OBJ model
// ---------------------------------
void renderMonster(Shader& shader, GLuint stoneVAO, int stoneVertices, GLuint tex, vec3 lightPos1, vec3 lightPos2){
    shader.use();

    shader.setVec3("lightPos1", lightPos1);
    shader.setVec3("lightPos2", lightPos2);
    shader.setVec3("viewPos", camera.getPosition());

    // Move the model within the world
    // -------------------------------
    mat4 monsterModelMatrix = translate(mat4(1.0f), gMonsterPos) *
                            scale(mat4(1.0f), vec3(gMonsterScale)); // scale down
    Renderer::setWorldMatrix(shader.getID(), monsterModelMatrix);

    Renderer::bindTexture(shader.getID(), tex, "textureSampler", MONSTER_TEX_SLOT);

    //Draw the stored vertex objects
    glBindVertexArray(stoneVAO);
    //TODO3 Draw model as elements, instead of as arrays
    glDrawArrays(GL_TRIANGLES, 0, stoneVertices);
    glBindVertexArray(0);
}

// Render scene from light for shadow mapping before rendering lighting
// --------------------------------------------------------------------
void renderSceneFromLight(Shader& shadowShader, const std::vector<Tower>& towers, GLuint cubeVAO)
{
    glm::mat4 identity = glm::mat4(1.0f);

    // Ground
    glm::mat4 groundMatrix = glm::scale(glm::translate(identity, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(60.0f, 0.1f, 60.0f));
    shadowShader.setMat4("worldMatrix", groundMatrix);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Buildings
    for (const auto& tower : towers) {
        glm::mat4 towerMatrix = glm::translate(identity, tower.position);
        towerMatrix = glm::scale(towerMatrix, glm::vec3(2.0f, tower.height, 2.0f));
        shadowShader.setMat4("worldMatrix", towerMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
}

// Render the monster into the shadow map (depth pass)
// ---------------------------------------------------
void renderMonsterFromLight(Shader& shadowShader, GLuint monsterVAO, int monsterVertexCount){
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)); // must match lighting pass!
    shadowShader.setMat4("worldMatrix", model);

    glBindVertexArray(monsterVAO);
    glDrawArrays(GL_TRIANGLES, 0, monsterVertexCount);
    glBindVertexArray(0);
}

// Hierarchical turret, Base -> Barrel
// -----------------------------------
void renderTurret(Shader& shader, GLuint cubeVAO, const glm::mat4& parentWorld, float baseYawDeg, float barrelZDeg, GLuint metalTexID){
    shader.use();
    Renderer::bindTexture(shader.getID(), metalTexID, "textureSampler", METAL_TEX_SLOT);

    // Base:
    glm::mat4 baseWorld = parentWorld *
        RY(baseYawDeg) *
        S(glm::vec3(2.0f, 0.3f, 2.0f));

    Renderer::setWorldMatrix(shader.getID(), baseWorld);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Mount: sits on top of base
    glm::mat4 mountWorld = baseWorld *
        T(glm::vec3(0.0f, 0.45f, 0.0f)) *
        S(glm::vec3(1.2f, 0.2f, 1.2f));

    Renderer::setWorldMatrix(shader.getID(), mountWorld);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Barrel:
    glm::mat4 barrelWorld = baseWorld *
        T(vec3(0.0f, 0.65f, 0.0f)) *                                   // pivot height
        rotate(mat4(1.0f), radians(barrelZDeg), vec3(1,0,0)) *         // Decide rotating direction
        T(vec3(0.0f, 1.0f, 0.0f)) *                                    // move to center after scaling
        S(vec3(0.25f, 2.0f, 0.25f));                                   // long Y box

    Renderer::setWorldMatrix(shader.getID(), barrelWorld);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Render turret shadow before lighting
// ------------------------------------
void renderTurretShadow(Shader& shadowShader, GLuint cubeVAO, const glm::mat4& parentWorld, float baseYawDeg, float barrelZDeg){
    auto draw = [&](const glm::mat4& w){
        shadowShader.setMat4("worldMatrix", w);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    };

    glm::mat4 baseWorld = parentWorld *
        RY(baseYawDeg) *
        S(glm::vec3(2.0f, 0.3f, 2.0f));
    draw(baseWorld);

    glm::mat4 mountWorld = baseWorld *
        T(glm::vec3(0.0f, 0.45f, 0.0f)) *
        S(glm::vec3(1.2f, 0.2f, 1.2f));
    draw(mountWorld);

    glm::mat4 barrelWorld = baseWorld *
        T(glm::vec3(0.0f, 0.65f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(barrelZDeg), glm::vec3(1,0,0)) *
        T(glm::vec3(0.0f, 1.0f, 0.0f)) *
        S(glm::vec3(0.25f, 2.0f, 0.25f));
    draw(barrelWorld);
}

// Set up model reading all vertices from a model file
// ---------------------------------------------------
GLuint setupModelVBO(string path, int& vertexCount) {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> UVs;
	
	//read the vertex data from the model's OBJ file
	loadOBJ(path.c_str(), vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs, as we are using multiple VAOs)
	vertexCount = vertices.size();
	return VAO;
}

// Sets up a model using an Element Buffer Object to refer to vertex data
// ---------------------------------------------------------------------
GLuint setupModelEBO(string path, int& vertexCount)
{
	vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

// Compute Direction to shoot at for the turret
// --------------------------------------------
static void computeTurretBarrelTipAndDir(const mat4& parentWorld, float baseYawDeg, float barrelZDeg, vec3& outTip, vec3& outDir){
    mat4 baseWorld = parentWorld * RY(baseYawDeg) * S(vec3(2.0f, 0.3f, 2.0f));
    mat4 barrelWorld = baseWorld *
        T(vec3(0.0f, 0.65f, 0.0f)) *
        rotate(mat4(1.0f), radians(barrelZDeg), vec3(1,0,0))*
        T(vec3(0.0f, 1.0f, 0.0f)) *
        S(vec3(0.25f, 2.0f, 0.25f));

    // Tip at local (0, +1, 0) after unit-cube → scaled to 2.0 along Y
    outTip = vec3(barrelWorld * glm::vec4(0, 1.0f, 0, 1));
    // Direction is local +Y transformed
    outDir = normalize(mat3(barrelWorld) * vec3(0, 1, 0));
}

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

    // Turret barrel Z rotation with Q / E  (±75°)
    const float barrelSpeed = 180.0f; // deg/sec
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        gTurretBarrelZDeg += barrelSpeed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        gTurretBarrelZDeg -= barrelSpeed * dt;
    }
    gTurretBarrelZDeg = glm::clamp(gTurretBarrelZDeg, -75.0f, 75.0f);

    //Random color chang of cubes
    //---------------------------------------
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !kKeyPressed) {
        kKeyPressed = true;
        flyingCubeColor = glm::vec3(
            static_cast<float>(rand()) / RAND_MAX,  
            static_cast<float>(rand()) / RAND_MAX,  
            static_cast<float>(rand()) / RAND_MAX);
        switch (rand()%4){
            case 0:
                CURRENT_CUBE_TEX_SLOT = MESH_TEX_SLOT;
                flyingCubeTextureID =MESH_TEX_SLOT +1;
                break;
            case 1:
                CURRENT_CUBE_TEX_SLOT = JACK_O_LANTERN_TEX_SLOT;
                flyingCubeTextureID =JACK_O_LANTERN_TEX_SLOT +1;
                break;
            case 2:
                CURRENT_CUBE_TEX_SLOT = LAMP_TEX_SLOT;
                flyingCubeTextureID =LAMP_TEX_SLOT +1;
                break;
            case 3:
                CURRENT_CUBE_TEX_SLOT = GLOWSTONE_TEX_SLOT;
                flyingCubeTextureID =GLOWSTONE_TEX_SLOT+1;
                break;
            default:
                break;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        kKeyPressed = false;
    }

    // Use camera lookat and side vectors to update positions with ASDW + SHIFT
    // ------------------------------------------------------------------------
    camera.processInput(window);
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
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Comp 371 - Project 371 Assignment - Monster City", NULL, NULL);
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