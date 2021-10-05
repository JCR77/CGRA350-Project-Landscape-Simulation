#include "WaterSurface.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../cgra/cgra_shader.hpp"

using namespace cgra;
using namespace std;
using namespace glm;

WaterSurface::WaterSurface(float size, float height) : height_(height)
{
    // centered at x = 0, z = 0
    mesh_builder builder;

    float half_size = size / 2;

    // create square plane
    mesh_vertex v0 = {.pos = vec3(-half_size, 0, -half_size), .norm = vec3(0, 1, 0), .uv = vec2(0, 1)};
    mesh_vertex v1 = {.pos = vec3(half_size, 0, -half_size), .norm = vec3(0, 1, 0), .uv = vec2(1, 1)};
    mesh_vertex v2 = {.pos = vec3(-half_size, 0, half_size), .norm = vec3(0, 1, 0), .uv = vec2(0, 0)};
    mesh_vertex v3 = {.pos = vec3(half_size, 0, half_size), .norm = vec3(0, 1, 0), .uv = vec2(1, 0)};

    builder.push_vertex(v0);
    builder.push_vertex(v1);
    builder.push_vertex(v2);
    builder.push_vertex(v3);

    // triangulate
    builder.push_index(0);
    builder.push_index(2);
    builder.push_index(1);
    builder.push_index(1);
    builder.push_index(2);
    builder.push_index(3);

    mesh_ = builder.build();

    // set shader
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//water//water_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//water//water_frag.glsl"));
    shader_ = sb.build();
}

void WaterSurface::setHeight(float height)
{
    height_ = height;
}

void WaterSurface::setTextures(int refraction, int reflection)
{
    refraction_texture_ = refraction;
    reflection_texture_ = reflection;

    // bind to texture units
    glUseProgram(shader_);
    glUniform1i(glGetUniformLocation(shader_, "uRefraction"), TextureUnit::Refraction);
    glUniform1i(glGetUniformLocation(shader_, "uReflection"), TextureUnit::Reflection);
    // normal map
    // distortion map
    glUseProgram(0);
}

void WaterSurface::draw(const glm::mat4 &view, const glm::mat4 proj)
{
    glUseProgram(shader_); // load shader and variables
    glBindVertexArray(mesh_.vao);

    // allow alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // bind textures
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Refraction);
    glBindTexture(GL_TEXTURE_2D, refraction_texture_);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Reflection);
    glBindTexture(GL_TEXTURE_2D, reflection_texture_);

    // translate by height
    mat4 transform = translate(mat4(1), vec3(0, height_, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uViewMatrix"), 1, false, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uModelMatrix"), 1, false, value_ptr(transform));
    glUniform3fv(glGetUniformLocation(shader_, "uColor"), 1, value_ptr(colour_));

    glDrawElements(mesh_.mode, mesh_.index_count, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);
    glUseProgram(shader_); // load shader and variables
}