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

    // Textures
    GLuint refraction_texture_, reflection_texture_, normal_map_, dudv_map_;

    cgra::gl_mesh mesh_;
    glm::vec3 colour_{0, 0, 1}; // temp

    float height_;

public:
    WaterSurface() = default;

    /**
     * Contructs a square plane mesh at a certain height in the scene.
     */
    WaterSurface(float size, float height);

    void draw(const glm::mat4 &view, const glm::mat4 proj);

    float getHeight() const { return height_; }
    void setHeight(float height);

    void setTextures(int refraction, int reflection);
};