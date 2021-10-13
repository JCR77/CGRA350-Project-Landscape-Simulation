#pragma once

#include <string>
#include <vector>

#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"

#include "fogRenderer.hpp"

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

    std::weak_ptr<FogRenderer> fog_;
    bool show_fog_ = false;
    void createMesh();

    /**
     * right, left, top, bottom, front, back
     */
    SkyBox(float size, std::vector<std::string> file_names);

public:
    /**
     * Uses default sky images
     */
    SkyBox(float size, std::weak_ptr<FogRenderer> fog);
    ~SkyBox();

    void setShowFog(bool show_fog) { show_fog_ = show_fog; }

    void draw(const glm::mat4 &view, const glm::mat4 &proj);
};