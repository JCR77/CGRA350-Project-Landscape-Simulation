
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "WaterSurface.hpp"
#include "../terrainRenderer.hpp"

class WaterRenderer
{
private:
    enum class Type
    {
        Refraction,
        Reflection
    };

    WaterSurface water_;

    GLuint refraction_fbo_;
    GLuint reflection_fbo_;
    GLuint refraction_texture;
    GLuint reflection_texture_;

    /* Clipping planes */

    // clips everything above the water surface
    glm::vec4 refraction_plane_;
    // clips everything below the water surface
    glm::vec4 reflection_plane_;

    // temp, use a pointer instead
    TerrainRenderer *terrain_renderer_;

    GLFWwindow *window_;

    void initFbos();
    int generateColourTexture(Type type);

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