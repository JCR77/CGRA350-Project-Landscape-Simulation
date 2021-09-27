#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../cgra/cgra_mesh.hpp"
#include "../cgra/cgra_shader.hpp"
#include "../cgra/cgra_wavefront.hpp"

/**
 * Temporary!
 */
struct TemporaryTerrain
{
    GLuint shader = 0;
    cgra::gl_mesh mesh;
    glm::vec3 color{0, 1, 0};
    glm::mat4 modelTransform{1.0};
    GLuint texture;

    TemporaryTerrain()
    {
        using namespace glm;
        using namespace cgra;

        shader_builder sb;
        sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_vert.glsl"));
        sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_frag.glsl"));
        shader = sb.build();
        mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//temp-terrain.obj")).build();
        modelTransform = scale(rotate(mat4(1), radians(-90.f), vec3(1, 0, 0)), vec3(0.3));
        color = vec3(0, 1, 0);
    }

    void draw(const glm::mat4 &view, const glm::mat4 proj)
    {
        using namespace glm;
        mat4 modelview = view * modelTransform;

        glUseProgram(shader); // load shader and variables
        glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
        glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

        mesh.draw(); // draw
    }
};