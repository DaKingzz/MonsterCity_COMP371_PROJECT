#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include <iostream>

class Geometry{
    private:
        unsigned int VBO, VAO;
    public:
        // Create a cubeVAO
        // ----------------
        int createCube(){
            float cubeVertices[] = {
            // Positions          // Colors    // UVs

            // Front face (+Z)
           -0.5f, -0.5f,  0.5f,   1, 0, 0,   0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,   1, 0, 0,   1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,   1, 0, 0,   1.0f, 1.0f,

            0.5f,  0.5f,  0.5f,   1, 0, 0,   1.0f, 1.0f,
           -0.5f,  0.5f,  0.5f,   1, 0, 0,   0.0f, 1.0f,
           -0.5f, -0.5f,  0.5f,   1, 0, 0,   0.0f, 0.0f,

            // Back face (−Z)
           -0.5f, -0.5f, -0.5f,   0, 1, 0,   1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,   0, 1, 0,   0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,   0, 1, 0,   0.0f, 1.0f,

            0.5f,  0.5f, -0.5f,   0, 1, 0,   0.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,   0, 1, 0,   1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,   0, 1, 0,   1.0f, 0.0f,

            // Left face (−X)
            -0.5f,  0.5f,  0.5f,   0, 0, 1,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0, 0, 1,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   0, 0, 1,   0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,   0, 0, 1,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   0, 0, 1,   1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   0, 0, 1,   1.0f, 1.0f,

            // Right face (+X)
            0.5f,  0.5f,  0.5f,   1, 1, 0,   0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,   1, 1, 0,   1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,   1, 1, 0,   1.0f, 1.0f,

            0.5f, -0.5f, -0.5f,   1, 1, 0,   1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,   1, 1, 0,   0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,   1, 1, 0,   0.0f, 0.0f,

            // Top face (+Y)
            -0.5f,  0.5f, -0.5f,   0, 1, 1,   0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,   0, 1, 1,   1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,   0, 1, 1,   1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,   0, 1, 1,   1.0f, 0.0f,
           -0.5f,  0.5f,  0.5f,   0, 1, 1,   0.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,   0, 1, 1,   0.0f, 1.0f,

            // Bottom face (−Y)
           -0.5f, -0.5f, -0.5f,   1, 0, 1,   1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,   1, 0, 1,   0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,   1, 0, 1,   0.0f, 1.0f,

            0.5f, -0.5f,  0.5f,   1, 0, 1,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   1, 0, 1,   1.0f, 1.0f,
           -0.5f, -0.5f,  0.5f,   1, 0, 1,   1.0f, 0.0f,
        };

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

            // Position Attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);   
            // Color Attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
            glEnableVertexAttribArray(1);
            // Texture Coords Attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2); 
            
            // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0); 

            // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
            // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
            glBindVertexArray(0); 
            std::cout << "GEOMETRY LOG: Cube created successfully" << std::endl;

            return VAO;
        }
        GLuint createLightCube(){
            float lightCubeVertices[] = {
            // positions          // normals           // texCoords
            // Front face
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

            // Back face
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,

            // Left face
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f, 0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f, 0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f, 0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f, 0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f, 0.0f,   0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f, 0.0f,   1.0f, 0.0f,

            // Right face
            0.5f,  0.5f,  0.5f,   1.0f,  0.0f, 0.0f,   1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,   1.0f,  0.0f, 0.0f,   0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,   1.0f,  0.0f, 0.0f,   1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,   1.0f,  0.0f, 0.0f,   0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,   1.0f,  0.0f, 0.0f,   1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,   1.0f,  0.0f, 0.0f,   0.0f, 0.0f,

            // Bottom face
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

            // Top face
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f
            };
            GLuint lightCubeVAO, lightCubeVBO;
            glGenVertexArrays(1, &lightCubeVAO);
            glGenBuffers(1, &lightCubeVBO);

            glBindVertexArray(lightCubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeVertices), lightCubeVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
           // position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // normal
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // texCoord
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            return lightCubeVAO;
        }
        void createSkybox(){
            const float skyboxVertices[] = {
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
            };
        }
};

#endif