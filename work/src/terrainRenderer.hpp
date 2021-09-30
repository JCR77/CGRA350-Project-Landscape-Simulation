
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
struct basic_terrain_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4& view, const glm::mat4 proj);
};


// Main terrain renerer class
//
class TerrainRenderer {
public:
	glm::vec2 m_windowsize;

private:

	// geometry
	basic_terrain_model m_model;


	//UI
	float scale = 20;
	float frequency = 0.04;
	//float resolution = 2;

public:
	// setup
	TerrainRenderer();

	// disable copy constructors (for safety)
	TerrainRenderer(const TerrainRenderer&) = delete;
	TerrainRenderer& operator=(const TerrainRenderer&) = delete;

	// rendering callbacks (every frame)
	void render(const glm::mat4& view, const glm::mat4& proj);
	void renderGUI();

private:
	//generate perlin noise
	float perlinNoise(float x, float y);
	glm::vec2 getCornerVector(int corner);
	float fade(float t);
	float lerp(float x, float p1, float p2);

	//generate terrain	
	cgra::gl_mesh generateTerrain(float size, int numTrianglesAcross, int numOctaves);
	cgra::mesh_builder generatePlane(float size, int numTrianglesAcross);
	float fbmNoise(float x, float y, int numOctaves);

};