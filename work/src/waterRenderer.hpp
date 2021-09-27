
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_water_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4& view, const glm::mat4 proj);
};


// Main terrain renerer class
//
class WaterRenderer {
private:

	// geometry
	basic_water_model m_model;

public:
	// setup
	WaterRenderer();

	// disable copy constructors (for safety)
	WaterRenderer(const WaterRenderer&) = delete;
	WaterRenderer& operator=(const WaterRenderer&) = delete;

	// rendering callbacks (every frame)
	void render(const glm::mat4& view, const glm::mat4& proj);
	void renderGUI();
};