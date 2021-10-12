
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
private:

	// geometry
	basic_fog_model m_model;
	basic_fog_model m_model2;

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
	float state = 0.0f;
	float near = 0.1f;
	float far = 300.0f;

	//Matricies
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	float frameIndex = 0;
	float indexSpeed = 0.05;
	float amplitude = 0.05;
	float period = 0.5;
};