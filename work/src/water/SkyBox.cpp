#include "SkyBox.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../cgra/cgra_image.hpp"
#include "../cgra/cgra_shader.hpp"

using namespace std;
using namespace glm;
using namespace cgra;

SkyBox::SkyBox(float size, std::vector<std::string> file_names)
{
    createMesh();

    transform_ = scale(mat4(1), vec3(size));

    string location = CGRA_SRCDIR + string("/res/textures/");
    for (int i = 0; i < file_names.size(); i++)
    {
        file_names.at(i) = location + file_names.at(i);
    }

    texture_ = rgba_image::createCubeMapTexture(file_names);

    // set shader
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//water//sky_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//water//sky_frag.glsl"));
    shader_ = sb.build();
}

void SkyBox::draw(const mat4 &view, const mat4 &proj)
{
    glUseProgram(shader_); // load shader and variables

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uViewMatrix"), 1, false, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uModelMatrix"), 1, false, value_ptr(transform_));
    mesh_.draw();

    glUseProgram(0); // load shader and variables
}

void SkyBox::createMesh()
{
    // create mesh
    mesh_builder builder;

    builder.push_vertex({.pos = vec3(-1, 1, -1)});
    builder.push_vertex({.pos = vec3(1, 1, -1)});
    builder.push_vertex({.pos = vec3(-1, 1, 1)});
    builder.push_vertex({.pos = vec3(1, 1, 1)});
    builder.push_vertex({.pos = vec3(-1, -1, -1)});
    builder.push_vertex({.pos = vec3(1, -1, -1)});
    builder.push_vertex({.pos = vec3(-1, -1, 1)});
    builder.push_vertex({.pos = vec3(1, -1, 1)});

    vector<int> indices = {
        0, 2, 1, 1, 2, 3, 1, 5, 0, 0, 5, 4, 3, 7, 1, 1, 7, 5,
        2, 6, 3, 3, 6, 7, 0, 4, 2, 2, 4, 6, 6, 4, 7, 7, 4, 5};

    for (int i : indices)
    {
        builder.push_index(i);
    }

    mesh_ = builder.build();
}