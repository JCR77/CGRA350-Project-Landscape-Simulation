
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "WaterSurface.hpp"
#include "SkyBox.hpp"
#include "../terrainRenderer.hpp"

class WaterRenderer
{
private:
    enum class Type
    {
        Reflection,
        Refraction
    };

    bool show_sky_ = true;

    WaterSurface water_;
    float water_height_ = 5.0;
    SkyBox sky_;

    GLuint refraction_fbo_;
    GLuint reflection_fbo_;
    GLuint refraction_texture_;
    GLuint reflection_texture_;

    // temp, use a pointer instead
    TerrainRenderer *terrain_renderer_;

    GLFWwindow *window_;
    glm::ivec2 window_size_;

    void initFbos();
    int generateColourTexture(Type type);
    glm::vec4 getClipPlane(Type type);

    void renderRefraction(const glm::mat4 &view, const glm::mat4 &proj);
    void renderReflection(const glm::mat4 &view, const glm::mat4 &proj);

public:
    WaterRenderer() = default;

    // setup
    WaterRenderer(TerrainRenderer *terrain_renderer, GLFWwindow *window);

    // disable copy constructors (for safety)
    WaterRenderer(const WaterRenderer &) = delete;

    // rendering callbacks (every frame)
    void render(const glm::mat4 &view, const glm::mat4 &proj);
    void renderGUI();
};