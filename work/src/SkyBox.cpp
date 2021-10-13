#include "SkyBox.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"

using namespace std;
using namespace glm;
using namespace cgra;

SkyBox::SkyBox(float size, weak_ptr<FogRenderer> fog)
    : SkyBox(size, {"sky_right.png", "sky_left.png", "sky_top.png", "sky_bottom.png", "sky_front.png", "sky_back.png"})
{
    this->fog = fog;
}

SkyBox::SkyBox(float size, std::vector<std::string> file_names)
{
    createMesh();

    transform = scale(mat4(1), vec3(size));

    string location = CGRA_SRCDIR + string("/res/textures/");
    for (int i = 0; i < file_names.size(); i++)
    {
        file_names.at(i) = location + file_names.at(i);
    }

    texture = rgba_image::createCubeMapTexture(file_names);

    // set shader
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("/res/shaders/sky_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("/res/shaders/sky_frag.glsl"));
    shader = sb.build();
}

void SkyBox::draw(const mat4 &view, const mat4 &proj)
{
    glUseProgram(shader); // load shader and variables

    // extract only rotation and scale from view transformation, to prevent skybox from
    // moving as the camera moves
    vec3 v;
    vec4 v4;
    vec3 scale;
    quat rotation;
    glm::decompose(view, scale, rotation, v, v, v4);
    mat4 rot_view = mat4_cast(rotation) * glm::scale(mat4(1), scale);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, false, value_ptr(rot_view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, false, value_ptr(transform));
    glUniform1f(glGetUniformLocation(shader, "uFog"), show_fog ? fog.lock()->far : 0.f);
    mesh.draw();

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

    mesh = builder.build();
}

SkyBox::~SkyBox()
{
    glDeleteProgram(shader);
    glDeleteTextures(1, &texture);
    mesh.destroy();
}