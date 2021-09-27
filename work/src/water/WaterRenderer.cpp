
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "WaterRenderer.hpp"
#include "../cgra/cgra_geometry.hpp"
#include "../cgra/cgra_gui.hpp"
#include "../cgra/cgra_image.hpp"
#include "../cgra/cgra_shader.hpp"
#include "../cgra/cgra_wavefront.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

WaterRenderer::WaterRenderer()
{
    water_ = WaterSurface(300, 10);
}

void WaterRenderer::render(const glm::mat4 &view, const glm::mat4 &proj)
{
    terrain_.draw(view, proj);
    water_.draw(view, proj);
}

void WaterRenderer::renderGUI()
{
    // example of how to use input boxes
    static float exampleInput;
    if (ImGui::InputFloat("example input", &exampleInput))
    {
        cout << "example input changed to " << exampleInput << endl;
    }
}