
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

	//generated a new seed and terrain
	if (ImGui::Button("New Seed")) {
		genPermutations();
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	//chose terrain type
	if (ImGui::Combo("Terrain Type", &fractalType, "Normal Terrain\0Smooth Valleys\0Hybrid Multifractal\0", 3)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	//terrain options 
	if (ImGui::SliderFloat("Scale", &scale, 1, 100, "%.0f", 1.0f)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderFloat("Base Frequency", &baseFrequency, 0, 0.2, "%.3f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderFloat("Frequency Multiplier", &frequencyMultiplier, 1, 5, "%.1f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderFloat("Amptitude Multiplier", &amtitudeMultiplier, 0, 1, "%.2f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}

	if (ImGui::SliderInt("Num Octaves", &numOctaves, 0, 10)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	}
	
	//extra options for hybrid multifractal terrain type
	if (fractalType == 2) {
		if (ImGui::SliderFloat("Offset", &offset, -1, 1, "%.2f")) {
			m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
		}

		if (ImGui::SliderFloat("H", &H, 0, 1, "%.2f")) {
			m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
		}
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
	//shuffle the seed (permutation table)
	std::shuffle(permutations, permutations + 256, default_random_engine());
}



//--------------------------------------------------------------------------------
// Generate Tererain
//--------------------------------------------------------------------------------


gl_mesh TerrainRenderer::generateTerrain(float size, int numTrianglesAcross, int numOctaves) {
	
	//generate height map
	//generate extra points along all sides to be used for calculating the normals
	//vector<vector<float>> heightMap = vector<vector<float>>();
	//float stepSize = size / numTrianglesAcross;
	//for (int y = 0; y < numTrianglesAcross+1 + 2; y++) {
	//	for (int x = 0; x < numTrianglesAcross+1 + 2; x++) {
	//		if (fractalType == 0) {
	//			heightMap[y][x] = homogeneousfbm(x * stepSize, y * stepSize, numOctaves);
	//		}
	//		else if (fractalType == 1) {
	//			heightMap[y][x] = heterogeneousfbm(x * stepSize, y * stepSize, numOctaves) - 0.5f;
	//		}
	//		else {
	//			heightMap[y][x] = hybridMultifractal(x * stepSize, y * stepSize, numOctaves) - 0.5f;
	//		}
	//	}
	//}

	//generate mesh
	mesh_builder plane_mb = generatePlane(size, numTrianglesAcross);

	//for (int y = 1; y < numTrianglesAcross + 1; y++) {
	//	for (int x = 1; x < numTrianglesAcross + 1; x++) {
	//		int i = (y-1) * (x-1) + (x-1);

	//		plane_mb.vertices[i].pos.y = heightMap[y][x] * scale;

	//		//calc normal
	//		float normX = heightMap[y][x-1] - heightMap[y][x + 1]; //difference in height of previous vertex and next vertex along the x axis
	//		float normZ = heightMap[y-1][x] - heightMap[y+1][x]; //difference in height of previous vertex and next vertex along the z axis	
	//		plane_mb.vertices[i].norm = normalize(vec3(normX, 2, normZ));
	//	}
	//}

	for (int i = 0; i < plane_mb.vertices.size(); i++) {
		//set vertex height
		float noise = 0;
		if (fractalType == 0) {
			noise = homogeneousfbm(plane_mb.vertices[i].pos.x, plane_mb.vertices[i].pos.z, numOctaves);
		}else if(fractalType == 1){
			noise = heterogeneousfbm(plane_mb.vertices[i].pos.x, plane_mb.vertices[i].pos.z, numOctaves) - 0.5f;
		} else {
			noise = hybridMultifractal(plane_mb.vertices[i].pos.x, plane_mb.vertices[i].pos.z, numOctaves) - 0.5f;
		}
		plane_mb.vertices[i].pos.y = noise * scale;

		//calc normal
		int n = numTrianglesAcross + 1;
		int x = i % n;
		int y = (i-x) / n;
		float normX;
		float normZ;
		if (x > 0 && x < n && y > 0 && y < n) {			
			normX = plane_mb.vertices[i - 1].pos.y - plane_mb.vertices[i + 1].pos.y; //difference in height of previous vertex and next vertex along the z axis
			normZ = plane_mb.vertices[i - n].pos.y - plane_mb.vertices[i + n].pos.y; //difference in height of previous vertex and next vertex along the x axis			
		}

		plane_mb.vertices[i].norm = normalize(vec3(normX, 2 * scale, normZ));
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



float TerrainRenderer::homogeneousfbm(float x, float y, int numOctaves) {
	float CurrentHeight = 0;

	//add multiple octaves (frequencies that are double the last frequancy and half the amptitude) together to make rough terrain.
	for (int i = 0; i < numOctaves; i++) {
		float noise = perlinNoise(x * baseFrequency * pow(frequencyMultiplier, i), y * baseFrequency * pow(frequencyMultiplier, i)) * (float)pow(amtitudeMultiplier, i);

		//add noise to current height function value
		CurrentHeight += noise;

	}

	return CurrentHeight;
}

float TerrainRenderer::heterogeneousfbm(float x, float y, int numOctaves) {
	float CurrentHeight = 0;
	float weight = 1.0f;

	//add multiple octaves (frequencies that are double the last frequancy and half the amptitude) together to make rough terrain.
	//weight the amptitude of each frequqncy by the current height of the function to smooth out valleys.
	for (int i = 0; i < numOctaves; i++) {
		float noise = perlinNoise(x*baseFrequency*pow(frequencyMultiplier,i), y*baseFrequency*pow(frequencyMultiplier,i));

		//move range to [0..1] form [-1..1]
		//means that adding octaves increases the height of mountains instead of just insreasing the roughness (average added height is positive instead of 0).
		noise = (noise + 1) / 2.0f;
		
		//reduce amptitude of higher frequencies
		noise *= (float)pow(amtitudeMultiplier, i);

		//scale by current height (to smooth valleys)
		float scaledNoise = noise * weight;
		
		//add noise to current height function value
		CurrentHeight += scaledNoise;

		//get the new weight for the next octave
		weight = fmin(1.0f, CurrentHeight);
	}

	return CurrentHeight;
}

float TerrainRenderer::hybridMultifractal(float x, float y, int numOctaves) {
	float CurrentHeight = 0;
	float weight = 1.0f;
	float previousNoiseVal = 1.0f;

	//add multiple octaves (frequencies that are double the last frequancy and half the amptitude) together to make rough terrain.
	//weight the amptitude of each frequqncy by the current height of the function to smooth out valleys.
	for (int i = 0; i < numOctaves; i++) {
		float noise = (perlinNoise(x * baseFrequency * pow(frequencyMultiplier, i), y * baseFrequency * pow(frequencyMultiplier, i)) + offset) * pow(pow(amtitudeMultiplier, i), H);

		//scale by current height (to smooth valleys)
		float scaledNoise = noise * weight;

		//add noise to current height function value
		CurrentHeight += scaledNoise;

		//get the new weight for the next octave
		weight = fmin(1.0f, scaledNoise);
	}

	return CurrentHeight;
}














