#pragma once

#include "../opengl.hpp"
#include "../cgra/cgra_mesh.hpp"

class WaterSurface
{
private:
    enum TextureUnit : int
    {
        Refraction,
        Reflection,
        NormalMap,
        DudvMap,
    };

    GLuint shader_ = 0;

    glm::vec2 movement_direction_{-1, -1};

    // Textures
    GLuint refraction_texture_, reflection_texture_, normal_map_, dudv_map_;

    cgra::gl_mesh mesh_;
    glm::vec3 colour_{0, 0, 1}; // temp

    void updateMovementOffset(float delta_time);

protected:
    friend class WaterRenderer;

    float height = 0;
    float movement_speed = 0.03;
    glm::vec2 movement_offset{0};
    float distortion_strength = 0.01;
    float ripple_size = 10;

public:
    WaterSurface() = default;

    /**
     * Contructs a square plane mesh at a certain height in the scene.
     */
    WaterSurface(float size, float height);

    void draw(const glm::mat4 &view, const glm::mat4 proj, float delta_time);

    void setTextures(int refraction, int reflection);
};