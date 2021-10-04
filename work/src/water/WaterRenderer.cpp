
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "WaterRenderer.hpp"
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
    water_ = WaterSurface(100, 0);

    // todo bias?
    refraction_plane_ = vec4(0, -1, 0, water_.getHeight());
    reflection_plane_ = vec4(0, 1, 0, -water_.getHeight());
    // create fbos
    initFbos();
    glEnable(GL_CLIP_PLANE0);

    // allow alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * Creates the fbos for the reflection and refraction textures.
 */
void WaterRenderer::initFbos()
{
    glGenFramebuffers(1, &refraction_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo_);
    generateColourTexture(Type::Refraction);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &reflection_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo_);
    generateColourTexture(Type::Reflection);
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
    }

    GLuint colour_texture;
    glGenTextures(1, &colour_texture);
    glBindTexture(GL_TEXTURE_2D, colour_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // TODO?
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colour_texture, 0);
    glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "Error creating colour texture attachment" << endl;
        exit(1);
    }
    return colour_texture;
}

void WaterRenderer::render(const glm::mat4 &view, const glm::mat4 &proj)
{
    // position camera for reflection (make the terrain appear upside down)
    mat4 reflection_view = scale(view, vec3(1, -1, 1));

    // get current frame buffer size
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    // render reflection to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo_);
    glViewport(0, 0, width / 2, height / 2); //TODO
    terrain_renderer_->render(reflection_view, proj, reflection_plane_);

    // render refraction to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo_);
    glViewport(0, 0, width, height); //TODO
    terrain_renderer_->render(view, proj, refraction_plane_);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    water_.draw(view, proj);
}

void WaterRenderer::renderGUI()
{
    // example of how to use input boxes
    static float exampleInput;
    if (ImGui::InputFloat("example input", &exampleInput))
    {
        cout << "example input changed to " << exampleInput << endl;
    }
}