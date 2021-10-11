#pragma once

#include <string>
#include <vector>

#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"

/**
 * https://learnopengl.com/Advanced-OpenGL/Cubemaps
 */
class SkyBox
{
private:
    GLuint shader_;
    GLuint texture_;
    cgra::gl_mesh mesh_;

    glm::mat4 transform_;

    void createMesh();

public:
    /**
     * Uses default sky images
     */
    SkyBox(float size);
    ~SkyBox();

    /**
     * right, left, top, bottom, front, back
     */
    SkyBox(float size, std::vector<std::string> file_names);

    void draw(const glm::mat4 &view, const glm::mat4 &proj);
};