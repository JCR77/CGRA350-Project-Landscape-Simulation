
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "terrainRenderer.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_terrain_model::draw(const glm::mat4& view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	//drawCylinder();
	mesh.draw(); // draw
}


TerrainRenderer::TerrainRenderer() {

	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(0, 1, 0);
	//	m_model.mesh.mode


	//test perlin noise algorithm
	for (int x = 1; x <= 30; x++) {
		for (int y = 1; y < 30; y++) {

			float noiseVal = perlinNoise(fmod(x * 0.05f, 256.0f), fmod(y * 0.05f, 256.0f));
			int luminance = (int)((noiseVal / 2.0f + 1) * 255);

			printf("%d, ", luminance);

		}
		printf("\n");
	}
}



//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------


void TerrainRenderer::render(const glm::mat4& view, const glm::mat4& proj) {

	/*myTerrain_dataVector.clear();

	for (int x = 0; x < m_windowsize.x; ++x) {
		for (int y = 0; y < m_windowsize.y; ++y) {

			float  noiseVal = perlinNoise(fmod(x * 0.01f, 256.0f), fmod(y * 0.01f, 256.0f));
			float  luminance = ((noiseVal / 2.0f + 1.0f));

			myTerrain_dataVector.push_back(luminance*255.0f);
		}
	}*/

	//glRasterPos2i(0, 0);
	//glDrawPixels(m_windowsize.x, m_windowsize.y, GL_LUMINANCE, GL_FLOAT, myTerrain_dataVector.data());

	m_model.mesh = generatePlane(20, 10).build();
	m_model.modelTransform = translate(mat4(1), vec3(-10, 0, -10));

	// draw the model
	m_model.draw(view, proj);
}


void TerrainRenderer::renderGUI() {

	// example of how to use input boxes
	static float exampleInput;
	if (ImGui::InputFloat("example input", &exampleInput)) {
		cout << "example input changed to " << exampleInput << endl;
	}

}



//--------------------------------------------------------------------------------
// Perlin Noise
//--------------------------------------------------------------------------------


float TerrainRenderer::perlinNoise(float x, float y) {
	assert(x <= 255);
	assert(y <= 255);

	//generate permutation table
	int p[] = { 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,
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
				67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

	//get squate corner and point in square coords	
	int X = (int)x; //square coords
	int Y = (int)y;
	float xf = x - X; //point in square coords
	float yf = y - Y;

	//get hash for each corner
	int TR = p[(p[(X + 1) % 256] + Y + 1) % 256]; //top right
	int TL = p[(p[X       % 256] + Y + 1) % 256]; //top left
	int BR = p[(p[(X + 1) % 256] + Y)     % 256]; //bottom right
	int BL = p[(p[X       % 256] + Y)     % 256]; //bottom left

	//get const vector for each corner 
	vec2 TR_ConstVec = geCornerVector(TR);
	vec2 TL_ConstVec = geCornerVector(TL);
	vec2 BR_ConstVec = geCornerVector(BR);
	vec2 BL_ConstVec = geCornerVector(BL);

	//get vertor to point for each corner
	vec2 TR_VectorToPoint = vec2(xf - 1.0f, yf - 1.0f);
	vec2 TL_VectorToPoint = vec2(xf,        yf - 1.0f);
	vec2 BR_VectorToPoint = vec2(xf - 1.0f, yf);
	vec2 BL_VectorToPoint = vec2(xf,		yf);

	//get dot product for each corner
	float TR_Val = dot(TR_ConstVec, TR_VectorToPoint);
	float TL_Val = dot(TL_ConstVec, TL_VectorToPoint);
	float BR_Val = dot(BR_ConstVec, BR_VectorToPoint);
	float BL_Val = dot(BL_ConstVec, BL_VectorToPoint);

	//get fade func of point in square
	float u = fade(xf);
	float v = fade(yf);

	//interp to get result
	float interpTop = lerp(u, TR_Val, TL_Val);
	float interpBottom = lerp(u, BR_Val, BL_Val);
	float result = lerp(v, interpTop, interpBottom); //between -1 and 1

	return result;
}

//Gets the constant vector at a corner given its hash from the permutations table which can then be used to get the
//dot product with the vector towards the point in the square. Does this by returning a different vector based on the
//first 4 bits of the hash of the conter.
//probably not as efficient as Perlin's grad() function but much easier to understand than a bunch of bit flipping.
vec2 TerrainRenderer::geCornerVector(int corner) {
	int hash = corner % 4;
	switch (hash) {
	case 0: return vec2(1.0f, 1.0f); break;
	case 1: return vec2(-1.0f, 1.0f); break;
	case 2: return vec2(-1.0f, -1.0f); break;
	case 3: return vec2(1.0f, -1.0f); break;
	default: return vec2(1); break; //should never happen
	}
}

float TerrainRenderer::fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float TerrainRenderer::lerp(float x, float p1, float p2) {
	assert(x >= 0 && x <= 1);

	return p1 + x * (p2 - p1);
}



//--------------------------------------------------------------------------------
// Generate Tererain
//--------------------------------------------------------------------------------


gl_mesh TerrainRenderer::generateTerrain(float width, float length, int numOctaves) {
	return m_model.mesh;
}


mesh_builder TerrainRenderer::generatePlane(float size, int numTrianglesAcross) {

	std::vector<vec3> vertices;
	std::vector<int> indices;

	float stepSize = size / numTrianglesAcross;

	for (int x = 0; x <= numTrianglesAcross; x++) {
		for (int y = 0; y <= numTrianglesAcross; y++) {
			//make vertex
			vertices.push_back(vec3(x*stepSize, 0, y*stepSize));

			//make triangles (populate index buffer)
			if (x < numTrianglesAcross && y < numTrianglesAcross) {
				int currentIndex = x * (numTrianglesAcross+1) + y;

				indices.push_back(currentIndex);
				indices.push_back(currentIndex + 1);
				indices.push_back(currentIndex + numTrianglesAcross+1);

				indices.push_back(currentIndex + 1);
				indices.push_back(currentIndex + numTrianglesAcross+1 + 1);
				indices.push_back(currentIndex + numTrianglesAcross+1);
			}
		}
	}

	//make mesh
	mesh_builder mb;

	for (int i = 0; i < vertices.size(); i++) {
		mb.push_vertex(mesh_vertex{
						vertices[i], //point coordinates
						vec3(0,1,0), //normal
						vec2(0,0) }); //uv
	}

	for (int i = 0; i < indices.size(); i++) {
		mb.push_index(indices[i]);
	}

	return mb;
}


float TerrainRenderer::fbmNoise(float x, float y, int numOctaves) {
	return 0;
}











