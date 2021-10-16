
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
struct basic_fog_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4& view, const glm::mat4 proj);
};


// Main terrain renerer class
//
class FogRenderer {
public:
	// setup
	FogRenderer();

	// disable copy constructors (for safety)
	FogRenderer(const FogRenderer&) = delete;
	FogRenderer& operator=(const FogRenderer&) = delete;

	// rendering callbacks (every frame)
	void render(const glm::mat4& view, const glm::mat4& proj);
	void renderGUI();

	//Variables
	float near = 0.006f;
	float far = 140.0f;
	float state = 0;

	float frameIndex = 0;
	float frameIndex2 = 0;
	float indexSpeed = 0.015f;
	float amplitude = 0.095f;
	float period = 2.0f;
};