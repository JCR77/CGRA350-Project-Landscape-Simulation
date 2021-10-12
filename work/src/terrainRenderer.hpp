
#pragma once

#include <string>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "terrain_mesh.hpp"
#include "cgra/cgra_image.hpp"


// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_terrain_model {
	GLuint shader = 0;
	terrain::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint grassTexture;
	GLuint sandTexture;
	GLuint stoneTexture;
	float scale = 20;
	GLuint offsetBuffer = 0;
	std::vector<float> offsets = std::vector<float>();
	std::vector<std::vector<float>> heightMap;

	float blendDist = 2.0f;
	float transitionHeight1 = 0.0f;
	float transitionHeight2 = 0.5f;

	void draw(const glm::mat4& view, const glm::mat4 proj, const glm::vec4 &clip_plane);
};


// Main terrain renerer class
//
class TerrainRenderer {
public:
	glm::vec2 m_windowsize;

private:

	// geometry
	basic_terrain_model m_model;
	float worldSize = 100;
	float squareSize = 0.5;
	float mapSize = worldSize / squareSize + 1;

	//noise
	int permutations[256] = {151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,
							103,30,69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,
							26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,
							87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
							77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,
							46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,
							187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,
							198,173,186, 3,64,52,217,226,250,124,123,5,202,38,147,118,126,
							255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,
							170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,
							172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,
							104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
							81,51,145,235,249,14,239,107,49,192,214, 31,181,199,106,157,184,
							84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,
							67,29,24,72,243,141,128,195,78,66,215,61,156,180};

	//base terrain
	float scale = 20;
	float baseFrequency = 0.04;
	int numOctaves = 4;
	float frequencyMultiplier = 2;
	float amtitudeMultiplier = 0.5;

	int fractalType = 1; //0 = normal terrain (homogeneous),		1 = smooth valleys (heterogeneous),		2 = Hybrid Multifractal

	float offset = 0.7;
	float H = 0.25;

	//errosion
	bool shouldErodeTerrain = false;
	int erodeIter = 0;
	float talusThreshold = 0.7f;
	float sedimentvolume = 0.3;

	float rainIter = 20;
	float evapIter = 40;

	float kr = 0.1;
	float ks = 0.5;
	float ke = 0.5;
	float kc = 0.1;

	std::vector<std::vector<float>> waterVolume = std::vector<std::vector<float>>();
	std::vector<std::vector<float>> sedimentVolume = std::vector<std::vector<float>>();

	//textures
	cgra::rgba_image textureImageGrass;
	cgra::rgba_image textureImageSand;
	cgra::rgba_image textureImageStone;


public:
	// setup
	TerrainRenderer();

	// disable copy constructors (for safety)
	TerrainRenderer(const TerrainRenderer&) = delete;
	TerrainRenderer& operator=(const TerrainRenderer&) = delete;

	// rendering callbacks (every frame)
	void render(const glm::mat4& view, const glm::mat4& proj, const glm::vec4& clip_plane=glm::vec4(0.0));
	void renderGUI();

private:
	//generate perlin noise
	float perlinNoise(float x, float y);
	glm::vec2 getCornerVector(int corner);
	float fade(float t);
	float lerp(float x, float p1, float p2);
	void genPermutations();

	//generate terrain	
	void generateTerrain(int numOctaves);
	terrain::mesh_builder generatePlane();
	//terrain::mesh_builder generateMeshFromHeightMap(std::vector<std::vector<float>> heightMap, int size, int numTriangles);

	float homogeneousfbm(float x, float y, int numOctaves);
	float heterogeneousfbm(float x, float y, int numOctaves);
	float hybridMultifractal(float x, float y, int numOctaves);

	std::vector<std::vector<float>> erodeTerrainTerraces(std::vector<std::vector<float>> heightMap, int size, int numIterations);

	std::vector<std::vector<float>> erodeTerrainRealistic(std::vector<std::vector<float>> heightMap, int size, int numIterations, bool rain);

};