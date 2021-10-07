
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "WaterRenderer.hpp"
#include "SkyBox.hpp"
#include "../cgra/cgra_geometry.hpp"
#include "../cgra/cgra_gui.hpp"
#include "../cgra/cgra_image.hpp"
#include "../cgra/cgra_shader.hpp"
#include "../cgra/cgra_wavefront.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

WaterRenderer::WaterRenderer(TerrainRenderer *terrain_renderer, GLFWwindow *window)
    : window_(window), terrain_renderer_(terrain_renderer)
{
    // create fbos
    initFbos();

    water_ = WaterSurface(100, 0);
    water_.setTextures(refraction_texture_, reflection_texture_);

    sky_ = SkyBox(200.f, {"sky_right.png", "sky_left.png", "sky_top.png", "sky_bottom.png", "sky_front.png", "sky_back.png"});
    // sky_ = SkyBox(500, {"checkerboard.jpg", "checkerboard.jpg", "checkerboard.jpg", "checkerboard.jpg", "checkerboard.jpg", "checkerboard.jpg"});
}

vec4 WaterRenderer::getClipPlane(Type type)
{
    // todo bias?
    if (type == Type::Refraction)
    {
        // clips everything above the water
        return vec4(0, -1, 0, water_.getHeight());
    }
    else if (type == Type::Reflection)
    {
        // clips everything below the water
        return vec4(0, 1, 0, -water_.getHeight());
    }
    return vec4(0);
}

/**
 * Creates the fbos for the reflection and refraction textures.
 */
void WaterRenderer::initFbos()
{
    glGenFramebuffers(1, &refraction_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo_);
    refraction_texture_ = generateColourTexture(Type::Refraction);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &reflection_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo_);
    reflection_texture_ = generateColourTexture(Type::Reflection);

    // reflection needs depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // get current frame buffer size
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    glViewport(0, 0, width, height);
}

int WaterRenderer::generateColourTexture(Type type)
{
    // get current frame buffer size
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    /**
     * We can afford to have a lower resolution for the relection texture,
     * as it will be distorted later.
     */
    if (type == Type::Reflection)
    {
        width /= 2;
        height /= 2;

        // reflection needs depth buffer
        GLuint depth_buffer;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    }

    GLuint colour_texture;
    glGenTextures(1, &colour_texture);
    glBindTexture(GL_TEXTURE_2D, colour_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // TODO?
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colour_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "Error creating colour texture attachment" << endl;
        exit(1);
    }
    return colour_texture;
}

void WaterRenderer::render(const glm::mat4 &view, const glm::mat4 &proj)
{
    sky_.draw(view, proj);

    glEnable(GL_CLIP_PLANE0);
    // position camera for reflection (make the terrain appear upside down)
    mat4 scale = glm::scale(mat4(1), vec3(1, -1, 1));
    mat4 translate = glm::translate(mat4(1), vec3(0, 2 * water_.getHeight(), 0));
    mat4 reflection_view = view * translate * scale;

    // get current frame buffer size
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // render reflection to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo_);
    glClearColor(1, 1, 1, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width / 2, height / 2); //TODO
    sky_.draw(reflection_view, proj);
    terrain_renderer_->render(reflection_view, proj, getClipPlane(Type::Reflection));

    glCullFace(GL_FRONT);
    // render refraction to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo_);
    glClearColor(1, 1, 1, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height); //TODO
    terrain_renderer_->render(view, proj, getClipPlane(Type::Refraction));

    glDisable(GL_CULL_FACE);
    glDisable(GL_CLIP_PLANE0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); //TODO
    water_.draw(view, proj);
}

void WaterRenderer::renderGUI()
{
    // example of how to use input boxes
    float height;
    if (ImGui::SliderFloat("Height", &height, -10, 20))
    {
        water_.setHeight(height);
    }
}