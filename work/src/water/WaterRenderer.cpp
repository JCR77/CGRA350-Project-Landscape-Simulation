
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

WaterRenderer::WaterRenderer(weak_ptr<TerrainRenderer> terrain_renderer, weak_ptr<SkyBox> sky) : terrain_renderer_(terrain_renderer)
{
    glfwGetFramebufferSize(glfwGetCurrentContext(), &window_size_.x, &window_size_.y);

    // create fbos
    initFbos();

    water_ = make_unique<WaterSurface>(100, 5);
    water_->setTextures(refraction_texture_, reflection_texture_);

    sky_ = sky;
}

vec4 WaterRenderer::getClipPlane(Type type)
{
    // raise the clipping plane depending on the amount of distortion,
    // otherwise we get strange artifacts
    float refraction_bias = 100 * water_->distortion_strength;
    float reflection_bias = 30 * water_->distortion_strength;
    if (type == Type::Refraction)
    {
        // clips everything above the water
        return vec4(0, -1, 0, water_->height + refraction_bias);
    }
    else if (type == Type::Reflection)
    {
        // clips everything below the water
        return vec4(0, 1, 0, -water_->height + reflection_bias);
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
    glViewport(0, 0, window_size_.x, window_size_.y);
}

/**
 * Deletes the current fbos and textures
 */
void WaterRenderer::destroy()
{
    glDeleteFramebuffers(1, &refraction_fbo_);
    glDeleteFramebuffers(1, &reflection_fbo_);
    glDeleteTextures(1, &refraction_texture_);
    glDeleteTextures(1, &reflection_texture_);
}

int WaterRenderer::generateColourTexture(Type type)
{
    int width = window_size_.x;
    int height = window_size_.y;

    // We can afford to have a lower resolution for the relection texture,
    // as it will be distorted later.
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
    glfwGetFramebufferSize(glfwGetCurrentContext(), &window_size_.x, &window_size_.y);

    // optimization:
    // only re-render reflection and refraction when absolutely necessary
    if (scene_updated)
    {
        glEnable(GL_CLIP_PLANE0);

        renderReflection(view, proj);
        renderRefraction(view, proj);

        glDisable(GL_CLIP_PLANE0);

        glViewport(0, 0, window_size_.x, window_size_.y);
    }

    water_->draw(view, proj, timer_.getDelta());
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
    glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, window_size_.x, window_size_.y); // TODO

    if (show_terrain_)
        terrain_renderer_.lock()->render(view, proj, getClipPlane(Type::Refraction));
    
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
    mat4 translate = glm::translate(mat4(1), vec3(0, 2 * water_->height, 0));
    mat4 reflection_view = view * translate * scale;

    // render reflection to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, window_size_.x / 2, window_size_.y / 2); // TODO

    // sky also needs to be reflected in the water
    sky_.lock()->draw(reflection_view, proj);

    if (show_terrain_)
        terrain_renderer_.lock()->render(reflection_view, proj, getClipPlane(Type::Reflection));

    glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
    glDisable(GL_CULL_FACE);
}

void WaterRenderer::renderGUI()
{
    if (ImGui::SliderFloat("Height", &water_->height, -10, 20, "%.3f"))
    {
        setSceneUpdated();
    }
    ImGui::SliderFloat("Distortion Strength", &water_->distortion_strength, 0.0, 0.02, "");
    ImGui::SliderFloat("Movement Speed", &water_->distortion_speed, 0.0, 0.1, "");
    ImGui::SliderFloat("Ripple Size", &water_->ripple_size, 1, 20, "");
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
    window_size_ = ivec2(width, height);
    destroy();
    initFbos(); // create new fbos with updated window size
    water_->setTextures(refraction_texture_, reflection_texture_);
}

void WaterRenderer::setShowTerrain(bool show_terrain)
{
    show_terrain_ = show_terrain;
    setSceneUpdated();
}