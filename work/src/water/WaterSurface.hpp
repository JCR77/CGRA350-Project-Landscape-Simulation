#pragma once

#include "../opengl.hpp"
#include "../cgra/cgra_mesh.hpp"

class WaterSurface
{
private:
    GLuint shader_ = 0;

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
};

// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_water_model
{
};