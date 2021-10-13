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

    mesh = builder.build();

    // set shader
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("/res/shaders/water/water_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("/res/shaders/water/water_frag.glsl"));
    shader = sb.build();

    // normal map
    rgba_image normal_image = rgba_image(CGRA_SRCDIR + string("/res/textures/normal_map.png"));
    normal_image.wrap = vec2(GL_REPEAT, GL_REPEAT);
    normal_map = normal_image.uploadTexture();

    // dudv map
    rgba_image dudv_image = rgba_image(CGRA_SRCDIR + string("/res/textures/dudv_map.png"));
    dudv_image.wrap = vec2(GL_REPEAT, GL_REPEAT);
    dudv_map = dudv_image.uploadTexture();

    // bind to texture units
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "uRefraction"), TextureUnit::Refraction);
    glUniform1i(glGetUniformLocation(shader, "uReflection"), TextureUnit::Reflection);
    glUniform1i(glGetUniformLocation(shader, "uNormalMap"), TextureUnit::NormalMap);
    glUniform1i(glGetUniformLocation(shader, "uDudvMap"), TextureUnit::DudvMap);
    glUniform1i(glGetUniformLocation(shader, "uDepth"), TextureUnit::Depth);

    glUseProgram(0);
}

void WaterSurface::setTextures(int refraction, int reflection, int depth)
{
    refraction_texture = refraction;
    reflection_texture = reflection;
    depth_texture = depth;
}

void WaterSurface::draw(const glm::mat4 &view, const glm::mat4 proj, float delta_time)
{
    updateOffsets(delta_time);

    glUseProgram(shader); // load shader and variables
    glBindVertexArray(mesh.vao);

    // allow alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bindTextures();

    // loading uniform variables
    mat4 model_transform = translate(mat4(1), vec3(0, height, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uViewMatrix"), 1, false, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelMatrix"), 1, false, value_ptr(model_transform));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(colour));
    glUniform1f(glGetUniformLocation(shader, "uDistortionStrength"), distortion_strength);
    glUniform1f(glGetUniformLocation(shader, "uRippleSize"), ripple_size);
    glUniform2fv(glGetUniformLocation(shader, "uPrimaryOffset"), 1, value_ptr(primary_offset.current_offset));
    glUniform2fv(glGetUniformLocation(shader, "uSecondaryOffset"), 1, value_ptr(secondary_offset.current_offset));

    glDrawElements(mesh.mode, mesh.index_count, GL_UNSIGNED_INT, 0);

    unbindTextures();

    glDisable(GL_BLEND);
    glUseProgram(0);
}

void WaterSurface::updateOffsets(float delta_time)
{
    primary_offset.update(distortion_speed, delta_time);
    secondary_offset.update(distortion_speed, delta_time);
}

WaterSurface::~WaterSurface()
{
    // Destroy all the textures
    glDeleteTextures(1, &refraction_texture);
    glDeleteTextures(1, &reflection_texture);
    glDeleteTextures(1, &normal_map);
    glDeleteTextures(1, &dudv_map);
    glDeleteTextures(1, &depth_texture);
    glDeleteProgram(shader);
    mesh.destroy();
}

void WaterSurface::unbindTextures()
{
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Refraction);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Reflection);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::NormalMap);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::DudvMap);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Depth);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void WaterSurface::bindTextures()
{
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Refraction);
    glBindTexture(GL_TEXTURE_2D, refraction_texture);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Reflection);
    glBindTexture(GL_TEXTURE_2D, reflection_texture);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::NormalMap);
    glBindTexture(GL_TEXTURE_2D, normal_map);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::DudvMap);
    glBindTexture(GL_TEXTURE_2D, dudv_map);
    glActiveTexture(GL_TEXTURE0 + TextureUnit::Depth);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
}