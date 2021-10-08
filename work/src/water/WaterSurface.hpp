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

    float movement_speed_ = 0.03;
    glm::vec2 movement_direction_{-1, -1};
    glm::vec2 movement_offset_{0};

    GLuint shader_ = 0;

    // Textures
    GLuint refraction_texture_, reflection_texture_, normal_map_, dudv_map_;

    cgra::gl_mesh mesh_;
    glm::vec3 colour_{0, 0, 1}; // temp

    float height_;
    float distortion_strength_ = 0.01;

    void updateMovementOffset(float delta_time);

public:
    WaterSurface() = default;

    /**
     * Contructs a square plane mesh at a certain height in the scene.
     */
    WaterSurface(float size, float height);

    void draw(const glm::mat4 &view, const glm::mat4 proj, float delta_time);

    float getHeight() const { return height_; }
    float getMovementSpeed() const { return movement_speed_; }
    void setHeight(float height);
    void setDistortionStrength(float strength);
    void setMovementSpeed(float speed);

    void setTextures(int refraction, int reflection);
};