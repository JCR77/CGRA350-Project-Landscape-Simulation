
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
#include "../cgra/cgra_shader.hpp"
#include "../cgra/cgra_wavefront.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

bool WaterRenderer::scene_updated = true;

WaterRenderer::WaterRenderer(weak_ptr<TerrainRenderer> terrain_renderer, weak_ptr<SkyBox> sky, weak_ptr<FogRenderer> fog)
    : terrain_renderer(terrain_renderer), fog_renderer(fog)
{
    glfwGetFramebufferSize(glfwGetCurrentContext(), &window_size.x, &window_size.y);

    // create fbos
    initFbos();

    water = make_unique<WaterSurface>(100, 2.566);
    water->setTextures(refraction_texture, reflection_texture, depth_texture);

    this->sky = sky;
}

vec4 WaterRenderer::getClipPlane(Type type)
{
    // raise the clipping plane depending on the amount of distortion,
    // otherwise we get strange artifacts
    float refraction_bias = 100 * water->distortion_strength;
    float reflection_bias = 30 * water->distortion_strength;
    if (type == Type::Refraction)
    {
        // clips everything above the water
        return vec4(0, -1, 0, water->height + refraction_bias);
    }
    else if (type == Type::Reflection)
    {
        // clips everything below the water
        return vec4(0, 1, 0, -water->height + reflection_bias);
    }
    return vec4(0);
}

/**
 * Creates the fbos for the reflection and refraction textures.
 */
void WaterRenderer::initFbos()
{
    glGenFramebuffers(1, &refraction_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo);
    refraction_texture = generateColourTexture(Type::Refraction);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &reflection_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo);
    reflection_texture = generateColourTexture(Type::Reflection);

    // reflection needs depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_size.x, window_size.y);
}

/**
 * Deletes the current fbos and textures
 */
void WaterRenderer::destroy()
{
    glDeleteFramebuffers(1, &refraction_fbo);
    glDeleteFramebuffers(1, &reflection_fbo);
    glDeleteTextures(1, &refraction_texture);
    glDeleteTextures(1, &reflection_texture);
    glDeleteTextures(1, &depth_texture);
}

int WaterRenderer::generateColourTexture(Type type)
{
    int width = window_size.x;
    int height = window_size.y;

    if (type == Type::Reflection)
    {
        // We can afford to have a lower resolution for the relection texture,
        // as it will be distorted later.
        width /= 2;
        height /= 2;

        // reflection needs depth buffer
        GLuint depth_buffer;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    }
    else if (type == Type::Refraction)
    {
        // Depth texture attachment to get the depth/distance of the terrain surface from the camera
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height,
                     0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }

    GLuint colour_texture;
    glGenTextures(1, &colour_texture);
    glBindTexture(GL_TEXTURE_2D, colour_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
    // get current frame buffer size
    glfwGetFramebufferSize(glfwGetCurrentContext(), &window_size.x, &window_size.y);

    // optimization:
    // only re-render reflection and refraction when absolutely necessary
    if (scene_updated)
    {
        glEnable(GL_CLIP_PLANE0);

        renderReflection(view, proj);
        renderRefraction(view, proj);

        glDisable(GL_CLIP_PLANE0);

        glViewport(0, 0, window_size.x, window_size.y);
    }

    float fog = show_fog ? fog_renderer.lock()->far : 0.f;
    water->draw(view, proj, timer.getDelta(), fog);
    scene_updated = false;
}

/**
 * Renders the refraction to an fbo
 */
void WaterRenderer::renderRefraction(const glm::mat4 &view, const glm::mat4 &proj)
{
    //Get Display FBO ID
    GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_size.x, window_size.y);
    sky.lock()->draw(view, proj);

    if (show_terrain)
        terrain_renderer.lock()->render(view, proj, getClipPlane(Type::Refraction));

    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
    glDisable(GL_CULL_FACE);
}

/**
 * Renders the reflection to an fbo
 */
void WaterRenderer::renderReflection(const glm::mat4 &view, const glm::mat4 &proj)
{
    //Get Display FBO ID
    GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // update view matrix to make scene appear upside down
    mat4 scale = glm::scale(mat4(1), vec3(1, -1, 1));
    mat4 translate = glm::translate(mat4(1), vec3(0, 2 * water->height, 0));
    mat4 reflection_view = view * translate * scale;

    // render reflection to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_size.x / 2, window_size.y / 2);

    // sky also needs to be reflected in the water
    sky.lock()->draw(reflection_view, proj);

    if (show_terrain)
        terrain_renderer.lock()->render(reflection_view, proj, getClipPlane(Type::Reflection));

    glViewport(0, 0, window_size.x, window_size.y);
    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
    glDisable(GL_CULL_FACE);
}

void WaterRenderer::renderGUI()
{
    if (ImGui::SliderFloat("Height", &water->height, -10, 20, "%.3f"))
    {
        setSceneUpdated();
    }
    ImGui::SliderFloat("Distortion Strength", &water->distortion_strength, 0.0, 0.02, "");
    ImGui::SliderFloat("Movement Speed", &water->distortion_speed, 0.0, 0.1, "");
    ImGui::SliderFloat("Ripple Size", &water->ripple_size, 1, 20, "");
}

WaterRenderer::~WaterRenderer()
{
    destroy();
}

/**
 * Resizes the fbos
 */
void WaterRenderer::resize(int width, int height)
{
    window_size = ivec2(width, height);
    destroy();
    initFbos(); // create new fbos with updated window size
    water->setTextures(refraction_texture, reflection_texture, depth_texture);
}

void WaterRenderer::setShowTerrain(bool show_terrain)
{
    this->show_terrain = show_terrain;
    setSceneUpdated();
}

void WaterRenderer::setShowFog(bool show_fog)
{
    this->show_fog = show_fog;
    setSceneUpdated();
}