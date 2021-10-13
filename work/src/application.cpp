
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

using namespace std;
using namespace cgra;
using namespace glm;

Application::Application(GLFWwindow *window) : m_window(window)
{
    terrain_renderer = make_shared<TerrainRenderer>();
    fog_renderer = make_shared<FogRenderer>();
    sky = make_shared<SkyBox>(200.f, fog_renderer);
    water_renderer = make_shared<WaterRenderer>(terrain_renderer, sky, fog_renderer);
    water_renderer->setShowTerrain(show_terrain);
}

void Application::render()
{
    // retrieve the window height
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    m_windowsize = vec2(width, height); // update window size
    glViewport(0, 0, width, height);    // set the viewport to draw to the entire window
    terrain_renderer->m_windowsize = m_windowsize;

    m_windowsize = vec2(width, height); // update window size
    glViewport(0, 0, width, height);    // set the viewport to draw to the entire window

    // clear the back-buffer
    glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // enable flags for normal/forward rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // projection matrix
    mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

    // view matrix
    mat4 view = translate(mat4(1), vec3(0, 0, -m_distance)) * rotate(mat4(1), m_pitch, vec3(1, 0, 0)) * rotate(mat4(1), m_yaw, vec3(0, 1, 0));

    // helpful draw options
    if (m_show_grid)
        drawGrid(view, proj);
    if (m_show_axis)
        drawAxis(view, proj);
    glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

    sky->draw(view, proj);

    // draw
    if (show_terrain)
        terrain_renderer->render(view, proj);
    if (show_water)
        water_renderer->render(view, proj);
    if (show_fog)
        fog_renderer->render(view, proj);
}

void Application::renderGUI()
{
    // setup window
    ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiSetCond_Once);
    ImGui::Begin("Options", 0);

    // display current camera parameters
    ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
    // ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
    // ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);
    ImGui::Separator();

    // helpful drawing options
    ImGui::Checkbox("Show axis", &m_show_axis);
    ImGui::SameLine();
    ImGui::Checkbox("Show grid", &m_show_grid);
    ImGui::Checkbox("Wireframe", &m_showWireframe);
    ImGui::SameLine();
    if (ImGui::Button("Screenshot"))
        rgba_image::screenshot(true);

    ImGui::Separator();
    if (ImGui::Checkbox("Terrain", &show_terrain))
        water_renderer->setShowTerrain(show_terrain);
    ImGui::SameLine();
    ImGui::Checkbox("Water", &show_water);
    ImGui::SameLine();
    if (ImGui::Checkbox("Fog", &show_fog)) {
        sky->setShowFog(show_fog);
        water_renderer->setShowFog(show_fog);
        WaterRenderer::setSceneUpdated();
    }
    ImGui::Separator();

    if (show_terrain) {
        if (ImGui::CollapsingHeader("Terrain##ID1"))
            terrain_renderer->renderGUI();
    }

    if (show_water) {
        if (ImGui::CollapsingHeader("Water##ID1"))
            water_renderer->renderGUI();
    }

    if (show_fog) {
        if (ImGui::CollapsingHeader("Fog##ID1"))
            fog_renderer->renderGUI();
    }

    // finish creating window
    ImGui::End();
}

void Application::cursorPosCallback(double xpos, double ypos)
{
    if (m_leftMouseDown)
    {
        vec2 whsize = m_windowsize / 2.0f;

        // clamp the pitch to [-pi/2, pi/2]
        m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f)) - acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
        m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

        // wrap the yaw to [-pi, pi]
        m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f)) - acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
        if (m_yaw > pi<float>())
            m_yaw -= float(2 * pi<float>());
        else if (m_yaw < -pi<float>())
            m_yaw += float(2 * pi<float>());
        WaterRenderer::setSceneUpdated();
    }

    // updated mouse position
    m_mousePosition = vec2(xpos, ypos);
}

void Application::mouseButtonCallback(int button, int action, int mods)
{
    (void)mods; // currently un-used

    // capture is left-mouse down
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}

void Application::scrollCallback(double xoffset, double yoffset)
{
    (void)xoffset; // currently un-used
    m_distance *= pow(1.1f, -yoffset);
    WaterRenderer::setSceneUpdated();
}

void Application::keyCallback(int key, int scancode, int action, int mods)
{
    (void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}

void Application::charCallback(unsigned int c)
{
    (void)c; // currently un-used
}

void Application::resize(int width, int height)
{
    water_renderer->resize(width, height);
    WaterRenderer::setSceneUpdated();
}
