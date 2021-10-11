#include "WaterSurface.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../cgra/cgra_shader.hpp"
#include "../cgra/cgra_image.hpp"

using namespace cgra;
using namespace std;
using namespace glm;

WaterSurface::WaterSurface(float size, float height) : height(height)
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
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("/res/shaders/water/water_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("/res/shaders/water/water_frag.glsl"));
    shader_ = sb.build();

    // normal map
    rgba_image normal_image = rgba_image(CGRA_SRCDIR + string("/res/textures/normal_map.png"));
    normal_image.wrap = vec2(GL_REPEAT, GL_REPEAT);
    normal_map_ = normal_image.uploadTexture();

    // dudv map
    rgba_image dudv_image = rgba_image(CGRA_SRCDIR + string("/res/textures/dudv_map.png"));
    dudv_image.wrap = vec2(GL_REPEAT, GL_REPEAT);
    dudv_map_ = dudv_image.uploadTexture();

    // bind to texture units
    glUseProgram(shader_);
    glUniform1i(glGetUniformLocation(shader_, "uRefraction"), TextureUnit::Refraction);
    glUniform1i(glGetUniformLocation(shader_, "uReflection"), TextureUnit::Reflection);
    glUniform1i(glGetUniformLocation(shader_, "uNormalMap"), TextureUnit::NormalMap);
    glUniform1i(glGetUniformLocation(shader_, "uDudvMap"), TextureUnit::DudvMap);

    glUseProgram(0);
}

void WaterSurface::setTextures(int refraction, int reflection)
{
    refraction_texture_ = refraction;
    reflection_texture_ = reflection;
}

void WaterSurface::draw(const glm::mat4 &view, const glm::mat4 proj, float delta_time)
{
    updateOffsets(delta_time);

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
    glActiveTexture(GL_TEXTURE0 + TextureUnit::NormalMap);
    glBindTexture(GL_TEXTURE_2D, normal_map_);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::DudvMap);
    glBindTexture(GL_TEXTURE_2D, dudv_map_);

    // loading uniform variables
    mat4 model_transform = translate(mat4(1), vec3(0, height, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uViewMatrix"), 1, false, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uModelMatrix"), 1, false, value_ptr(model_transform));
    glUniform3fv(glGetUniformLocation(shader_, "uColor"), 1, value_ptr(colour_));
    glUniform1f(glGetUniformLocation(shader_, "uDistortionStrength"), distortion_strength);
    glUniform1f(glGetUniformLocation(shader_, "uRippleSize"), ripple_size);
    glUniform2fv(glGetUniformLocation(shader_, "uPrimaryOffset"), 1, value_ptr(primary_offset_.current_offset));
    glUniform2fv(glGetUniformLocation(shader_, "uSecondaryOffset"), 1, value_ptr(secondary_offset_.current_offset));

    glDrawElements(mesh_.mode, mesh_.index_count, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);
    glUseProgram(0);
}

void WaterSurface::updateOffsets(float delta_time)
{
    primary_offset_.update(distortion_speed, delta_time);
    secondary_offset_.update(distortion_speed, delta_time);
}

WaterSurface::~WaterSurface()
{
    // Destroy all the textures
    glDeleteTextures(1, &refraction_texture_);
    glDeleteTextures(1, &reflection_texture_);
    glDeleteTextures(1, &normal_map_);
    glDeleteTextures(1, &dudv_map_);
    glDeleteProgram(shader_);
    mesh_.destroy();
}