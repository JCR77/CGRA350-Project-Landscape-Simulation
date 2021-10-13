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
    GLuint shader;
    GLuint texture;
    cgra::gl_mesh mesh;

    glm::mat4 transform;

    std::weak_ptr<FogRenderer> fog;
    bool show_fog = false;
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

    void setShowFog(bool show_fog) { this->show_fog = show_fog; }

    void draw(const glm::mat4 &view, const glm::mat4 &proj);
};