
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "temporaryTerrain.hpp"
#include "waterSurface.hpp"

class WaterRenderer
{
private:
    TemporaryTerrain terrain_;
    WaterSurface water_;

public:
    // setup
    WaterRenderer();

    // disable copy constructors (for safety)
    WaterRenderer(const WaterRenderer &) = delete;
    WaterRenderer &operator=(const WaterRenderer &) = delete;

    // rendering callbacks (every frame)
    void render(const glm::mat4 &view, const glm::mat4 &proj);
    void renderGUI();
};