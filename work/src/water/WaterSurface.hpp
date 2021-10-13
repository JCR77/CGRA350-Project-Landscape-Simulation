#pragma once

#include "../opengl.hpp"
#include "../cgra/cgra_mesh.hpp"

struct DistortionOffset
{
    glm::vec2 direction;
    glm::vec2 current_offset{0};

    /**
     * @param direction the xz direction of offset
     */
    DistortionOffset(glm::vec2 offset_direction) : direction(glm::normalize(offset_direction)) {}

    /**
     * Updates the current distortion offset,
     * where its components are always in the range
     * [0, 1]
     */
    void update(float speed, float delta_time)
    {
        current_offset += speed * delta_time * direction;
        current_offset = glm::mod(current_offset, glm::vec2(1));
    }
};

class WaterSurface
{
private:
    enum TextureUnit : int
    {
        Refraction,
        Reflection,
        NormalMap,
        DudvMap,
        Depth
    };

    GLuint shader = 0;

    DistortionOffset primary_offset = DistortionOffset({-1, -1});
    DistortionOffset secondary_offset = DistortionOffset({0, -1});

    // Textures
    GLuint refraction_texture, reflection_texture, normal_map, dudv_map, depth_texture;

    cgra::gl_mesh mesh;
    glm::vec3 colour{0, 0, 1}; // temp

    void updateOffsets(float delta_time);
    void unbindTextures();
    void bindTextures();

protected:
    friend class WaterRenderer;

    float height = 0;
    float distortion_speed = 0.03;
    float distortion_strength = 0.01;
    float ripple_size = 6;

public:
    ~WaterSurface();

    /**
     * Contructs a square plane mesh at a certain height in the scene.
     */
    WaterSurface(float size, float height);

    void draw(const glm::mat4 &view, const glm::mat4 proj, float delta_time);

    void setTextures(int refraction, int reflection, int depth);
};