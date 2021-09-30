
// std
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>

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
	m_model.color = vec3(0, 1, 0);

	m_model.modelTransform = translate(mat4(1), vec3(-size / 2, 0, -size / 2));
	m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	
}



//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------


void TerrainRenderer::render(const glm::mat4& view, const glm::mat4& proj) {

	// draw the model
	m_model.draw(view, proj);
}


void TerrainRenderer::renderGUI() {

	if (ImGui::SliderFloat("Scale", &scale, 1, 100, "%.0f", 1.0f)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderFloat("Frequency", &frequency, 0, 0.5, "%.2f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderInt("Num Octaves", &numOctaves, 1, 8)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::Button("New Seed")) {
		genPermutations();
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

}



//--------------------------------------------------------------------------------
// Perlin Noise
//--------------------------------------------------------------------------------


float TerrainRenderer::perlinNoise(float x, float y) {

	//get square corner and point in square coords	
	int X = (int)fmod(x, 256); //square coords
	int Y = (int)fmod(y, 256);
	float xf = fmod(x, 1.0f); //point in square coords
	float yf = fmod(y, 1.0f);

	//get hash for each corner
	//TODO double to table to don't need %256
	int TR = permutations[(permutations[(X + 1) % 256] + Y + 1) % 256]; //top right
	int TL = permutations[(permutations[X       % 256] + Y + 1) % 256]; //top left
	int BR = permutations[(permutations[(X + 1) % 256] + Y)     % 256]; //bottom right
	int BL = permutations[(permutations[X       % 256] + Y)     % 256]; //bottom left

	//get const vector for each corner 
	vec2 TR_ConstVec = getCornerVector(TR);
	vec2 TL_ConstVec = getCornerVector(TL);
	vec2 BR_ConstVec = getCornerVector(BR);
	vec2 BL_ConstVec = getCornerVector(BL);

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
	float interpTop = lerp(u, TL_Val, TR_Val);
	float interpBottom = lerp(u, BL_Val, BR_Val);
	float result = lerp(v, interpBottom, interpTop); //between -1 and 1

	return result;
}

//Gets the constant vector at a corner given its hash from the permutations table which can then be used to get the
//dot product with the vector towards the point in the square. Does this by returning a different vector based on the
//first 4 bits of the hash of the conter.
//probably not as efficient as Perlin's grad() function but much easier to understand than a bunch of bit flipping.
vec2 TerrainRenderer::getCornerVector(int corner) {
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

void TerrainRenderer::genPermutations() {
	std::shuffle(permutations, permutations + 256, default_random_engine());
}



//--------------------------------------------------------------------------------
// Generate Tererain
//--------------------------------------------------------------------------------


gl_mesh TerrainRenderer::generateTerrain(float size, int numTrianglesAcross, int numOctaves) {
	
	mesh_builder plane_mb = generatePlane(size, numTrianglesAcross);

	//float scale = 20;
	//float frequency = 0.04;

	for (int i = 0; i < plane_mb.vertices.size(); i++) {
		//float noise = perlinNoise(fmod(plane_mb.vertices[i].pos.x * frequency, 256.0f), fmod(plane_mb.vertices[i].pos.z * frequency, 256.0f));
		float noise = fbmNoise(plane_mb.vertices[i].pos.x, plane_mb.vertices[i].pos.z, numOctaves);
		plane_mb.vertices[i].pos.y = noise * scale;

		//calc normals
		int n = numTrianglesAcross + 1;
		int x = i % n;
		int y = (i-x) / n;
		if (x > 0 && x < n && y > 0 && y < n) {			
			float normX = plane_mb.vertices[i - 1].pos.y - plane_mb.vertices[i + 1].pos.y; //difference in height of previous vertex and next vertex along the z axis
			float normZ = plane_mb.vertices[i - n].pos.y - plane_mb.vertices[i + n].pos.y; //difference in height of previous vertex and next vertex along the x axis

			plane_mb.vertices[i].norm = normalize(vec3(normX, 2*scale, normZ));
		}
	}

	return plane_mb.build();
}


mesh_builder TerrainRenderer::generatePlane(float size, int numTrianglesAcross) {

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<int> indices;

	float stepSize = size / numTrianglesAcross;

	for (int y = 0; y <= numTrianglesAcross; y++) {
		for (int x = 0; x <= numTrianglesAcross; x++) {
			//make vertex
			vertices.push_back(vec3(x*stepSize, 0, y*stepSize));

			//make triangles (populate index buffer)
			if (x < numTrianglesAcross && y < numTrianglesAcross) {
				int currentIndex = y * (numTrianglesAcross+1) + x;

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
	float Totalnoise = 0;

	//fmod(plane_mb.vertices[i].pos.x * frequency, 256.0f)
	for (int i = 0; i < numOctaves; i++) {
		float noise = perlinNoise(x*frequency*pow(2,i), y*frequency*pow(2,i)) / (float)pow(2, i);
		Totalnoise += noise;
	}

	return Totalnoise;
}











