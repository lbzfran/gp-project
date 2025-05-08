#pragma once

#include "ShaderProgram.h"
#include <glad/glad.h>

const float screenVertices[] = {
  // positions   // texCoords
    1.0f, -1.0f,  1.0f, 0.0f,
   -1.0f, -1.0f,  0.0f, 0.0f,
   -1.0f,  1.0f,  0.0f, 1.0f,

    1.0f,  1.0f,  1.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
   -1.0f,  1.0f,  0.0f, 1.0f
};

class Framebuffer {
    public:
        uint32_t fboId; // framebuffer id
        uint32_t textureId; // texture buffer
        uint32_t rboId; // render buffer

        ShaderProgram program;

        Framebuffer(uint32_t& width, uint32_t& height, ShaderProgram p, bool enableCull = false, bool enableStencil = true) : program(p), winWidth(width), winHeight(height), cullEnabled(enableCull), stencilEnabled(enableStencil) {
            Resize();
        };

        ~Framebuffer() {
            glDeleteVertexArrays(1, &screenVAO);
            glDeleteBuffers(1, &screenVBO);
            glDeleteRenderbuffers(1, &rboId);
            glDeleteTextures(1, &textureId);
            glDeleteFramebuffers(1, &fboId);
        }

        void Resize() {
            if (rboId)
                glDeleteRenderbuffers(1, &rboId);

            if (textureId)
                glDeleteTextures(1, &textureId);

            if (fboId)
                glDeleteFramebuffers(1, &fboId);

            // sets up VAO and VBO that wil fit the whole screen.
            glGenVertexArrays(1, &screenVAO);
            glGenBuffers(1, &screenVBO);
            glBindVertexArray(screenVAO);
            glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);


            // set up the framebuffer
            glGenFramebuffers(1, &fboId);
            glBindFramebuffer(GL_FRAMEBUFFER, fboId);

            // set up texture buffer
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
            glBindTexture(GL_TEXTURE_2D, 0);


            // set up render buffer object
            glGenRenderbuffers(1, &rboId);
            glBindRenderbuffer(GL_RENDERBUFFER, rboId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winWidth, winHeight);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);


            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cout << "ERROR: Framebuffer incomplete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindVertexArray(0);
        }

        void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f, bool includeDepth = true) {
            glClearColor(r, g, b, a);
            auto options = GL_COLOR_BUFFER_BIT;

            if (includeDepth)
                options |= GL_DEPTH_BUFFER_BIT;

            if (stencilEnabled)
                options |= GL_STENCIL_BUFFER_BIT;

            glClear(options);
        }

        /*
         *
         * only for reference/debugging
         *
         */
        void RenderOnScreen() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_DEPTH_TEST);

            if (stencilEnabled)
                glEnable(GL_STENCIL_TEST);

            if (cullEnabled)
                glEnable(GL_CULL_FACE);

            Clear();
        }

        void RenderOnTexture() {
            // binds the framebuffer for drawing
            glBindFramebuffer(GL_FRAMEBUFFER, fboId);
            glViewport(0, 0, winWidth, winHeight);
            // glViewport(0, 0, width, height);

            glEnable(GL_DEPTH_TEST);
            if (stencilEnabled)
                glEnable(GL_STENCIL_TEST);

            if (cullEnabled)
                glEnable(GL_CULL_FACE);

            Clear(0.45f, 0.45f, 0.45f, 1.0f);
        }

        void TextureToScreen() {
            // binds view buffer for drawing
            // glViewport(0, 0, width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, winWidth, winHeight);

            Clear(1.0f, 1.0f, 1.0f, 1.0f);

            // draws texture to view
            program.activate();
            glBindVertexArray(screenVAO);

            glDisable(GL_DEPTH_TEST);
            if (stencilEnabled)
                glDisable(GL_STENCIL_TEST);

            if (cullEnabled)
                glDisable(GL_CULL_FACE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
        }
    private:
        uint32_t screenVAO;
        uint32_t screenVBO;

        uint32_t& winWidth;
        uint32_t& winHeight;

        bool cullEnabled;
        bool stencilEnabled;
};
