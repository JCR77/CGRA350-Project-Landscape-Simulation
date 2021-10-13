
#pragma once

#include <memory>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "WaterSurface.hpp"
#include "Timer.hpp"
#include "../SkyBox.hpp"
#include "../terrainRenderer.hpp"

class WaterRenderer
{
private:
    /**
     * True if the scene has been updated since the previous call to render.
     * If true, the refraction and reflection textures should be re-rendered
     * in the next render call.
     * 
     * Possible updates:
     * - window resize
     * - terrain changed shape/position
     * - water height changed
     * - camera view changed
     */
    static bool scene_updated;

    bool show_terrain = true;

    enum class Type
    {
        Reflection,
        Refraction
    };

    Timer timer;

    std::unique_ptr<WaterSurface> water;

    GLuint refraction_fbo;
    GLuint reflection_fbo;
    GLuint refraction_texture;
    GLuint reflection_texture;
    GLuint depth_texture;

    std::weak_ptr<TerrainRenderer> terrain_renderer;
    std::weak_ptr<SkyBox> sky;

    glm::ivec2 window_size;

    void initFbos();
    int generateColourTexture(Type type);
    glm::vec4 getClipPlane(Type type);

    void renderRefraction(const glm::mat4 &view, const glm::mat4 &proj);
    void renderReflection(const glm::mat4 &view, const glm::mat4 &proj);
    void destroy();
    void unbindTextures();
    void bindTextures();

public:
    ~WaterRenderer();

    // setup
    WaterRenderer(std::weak_ptr<TerrainRenderer> terrain_renderer, std::weak_ptr<SkyBox> sky);

    // disable copy constructors (for safety)
    WaterRenderer(const WaterRenderer &) = delete;
    WaterRenderer &operator=(const WaterRenderer &) = delete;

    static void setSceneUpdated() { scene_updated = true; }
    void setShowTerrain(bool show_terrain);

    // rendering callbacks (every frame)
    void render(const glm::mat4 &view, const glm::mat4 &proj);
    void renderGUI();
    void resize(int width, int height);
};