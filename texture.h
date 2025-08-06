#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <GL/glew.h>
#include <iostream>
#include <cassert>

class Texture {
public:
    // Load and create textures from a jpeg file
    // -----------------------------------------
    static GLuint load(const char* filename, bool flipVertically = true) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(flipVertically);
        unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            return 0;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        assert(textureID != 0);

        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture filtering and wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "TEXTURE LOG: Loaded texture: " << filename << " (ID: " << textureID << ")" << std::endl;
        return textureID;
    }
};

#endif
