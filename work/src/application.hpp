
#pragma once

#include <memory>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "terrainRenderer.hpp"
#include "water/WaterRenderer.hpp"
#include "fogRenderer.hpp"

// Main application class
//
class Application
{
private:
    // window
    glm::vec2 m_windowsize;
    GLFWwindow *m_window;

    // oribital camera
    float m_pitch = .35;
    float m_yaw = 0;
    float m_distance = 85.46;

    // last input
    bool m_leftMouseDown = false;
    glm::vec2 m_mousePosition;

    // drawing flags
    bool m_show_axis = false;
    bool m_show_grid = false;
    bool m_showWireframe = false;

    // renderers
    std::shared_ptr<TerrainRenderer> terrain_renderer;
    std::shared_ptr<WaterRenderer> water_renderer;
    FogRenderer fog_renderer;

    std::shared_ptr<SkyBox> sky;

    bool show_terrain = true;
    bool show_water = false;
    bool show_fog = false;

public:
    // setup
    Application(GLFWwindow *);

    // disable copy constructors (for safety)
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    // rendering callbacks (every frame)
    void render();
    void renderGUI();

    // input callbacks
    void cursorPosCallback(double xpos, double ypos);
    void mouseButtonCallback(int button, int action, int mods);
    void scrollCallback(double xoffset, double yoffset);
    void keyCallback(int key, int scancode, int action, int mods);
    void charCallback(unsigned int c);
    void resize(int width, int height); // resizing the window
};