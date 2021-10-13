
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
#include "water/WaterRenderer.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
//#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace terrain;
using namespace glm;


void basic_terrain_model::draw(const glm::mat4& view, const glm::mat4 proj, const vec4 & clip_plane) {
	mat4 modelview = view * modelTransform;

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	glUniform4fv(glGetUniformLocation(shader, "uClipPlane"), 1, value_ptr(clip_plane));
	glUniform1i(glGetUniformLocation(shader, "textureSampler0"), 3);
	glUniform1i(glGetUniformLocation(shader, "textureSampler1"), 4);
	glUniform1i(glGetUniformLocation(shader, "textureSampler2"), 5);
	glUniform1f(glGetUniformLocation(shader, "scale"), scale);
	//glUniform1fv(glGetUniformLocation(shader, "trasitionHeightOffsets"), 201*201, trasitionHeightOffsets);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, sandTexture);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	mesh.draw(); // draw
}


TerrainRenderer::TerrainRenderer() {

	cgra::shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//terrain//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.color = vec3(0, 1, 0);

	m_model.modelTransform = translate(mat4(1), vec3(-size / 2, 0, -size / 2));
	m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
	m_model.scale = scale;


	//bind texture

	unsigned int sandTexture;
	glGenTextures(1, &sandTexture);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, sandTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	textureImageSand = cgra::rgba_image(CGRA_SRCDIR + std::string("//res//textures//sand_texture.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureImageSand.size.x, textureImageSand.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImageSand.data.data());

	m_model.sandTexture = sandTexture;



	unsigned int grassTexture;
	glGenTextures(1, &grassTexture);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	textureImageGrass = cgra::rgba_image(CGRA_SRCDIR + std::string("//res//textures//grass_texture.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureImageGrass.size.x, textureImageGrass.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImageGrass.data.data());

	m_model.grassTexture = grassTexture;



	unsigned int stoneTexture;
	glGenTextures(1, &stoneTexture);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, stoneTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	textureImageStone = cgra::rgba_image(CGRA_SRCDIR + std::string("//res//textures//stone_texture.png"));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureImageStone.size.x, textureImageStone.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImageStone.data.data());

	m_model.stoneTexture = stoneTexture;
	
}



//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------


void TerrainRenderer::render(const glm::mat4& view, const glm::mat4& proj, const vec4& clip_plane) {

	// draw the model
	m_model.scale = scale;
	m_model.draw(view, proj, clip_plane);
}


void TerrainRenderer::renderGUI() {

	//generated a new seed and terrain
	if (ImGui::Button("New Seed")) {
		genPermutations();
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        // This tells the water renderer that it needs to update the 
        // reflection and refraction textures
        WaterRenderer::setSceneUpdated();
	}

	//chose terrain type
	if (ImGui::Combo("Terrain Type", &fractalType, "Normal Terrain\0Smooth Valleys\0Hybrid Multifractal\0", 3)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}

	//terrain options 
	if (ImGui::SliderFloat("Scale", &scale, 1, 100, "%.0f", 1.0f)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}

	if (ImGui::SliderFloat("Base Frequency", &baseFrequency, 0, 0.2, "%.3f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}

	if (ImGui::SliderFloat("Frequency Multiplier", &frequencyMultiplier, 1, 5, "%.1f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}

	if (ImGui::SliderFloat("Amptitude Multiplier", &amtitudeMultiplier, 0, 1, "%.2f")) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}

	if (ImGui::SliderInt("Num Octaves", &numOctaves, 0, 10)) {
		m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
        WaterRenderer::setSceneUpdated();
	}
	
	//extra options for hybrid multifractal terrain type
	if (fractalType == 2) {
		if (ImGui::SliderFloat("Offset", &offset, -1, 1, "%.2f")) {
			m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
            WaterRenderer::setSceneUpdated();
		}

		if (ImGui::SliderFloat("H", &H, 0, 1, "%.2f")) {
			m_model.mesh = generateTerrain(size, size / squareSize, numOctaves);
            WaterRenderer::setSceneUpdated();
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
	//generate extra points along all sides for calculating the normals at the edges
	vector<vector<float>> heightMap(numTrianglesAcross+3, vector<float>(numTrianglesAcross + 3, 0));
	float stepSize = size / numTrianglesAcross;
	for (int y = 0; y <= numTrianglesAcross + 2; y++) {
		for (int x = 0; x <= numTrianglesAcross + 2; x++) {
			if (fractalType == 0) {
				heightMap[y][x] = homogeneousfbm(x * stepSize, y * stepSize, numOctaves);
			}
			else if (fractalType == 1) {
				heightMap[y][x] = heterogeneousfbm(x * stepSize, y * stepSize, numOctaves) - 0.5f;
			}
			else {
				heightMap[y][x] = hybridMultifractal(x * stepSize, y * stepSize, numOctaves) - 0.5f;
			}
		}
	}

	//generate mesh
	mesh_builder plane_mb = generatePlane(size, numTrianglesAcross);

	for (int y = 1; y <= numTrianglesAcross + 1; y++) {
		for (int x = 1; x <= numTrianglesAcross + 1; x++) {
			int i = (y-1) * (numTrianglesAcross+1) + (x-1);

			plane_mb.vertices[i].pos.y = heightMap[y][x] * scale;

			//calc normal
			float normX = heightMap[y][x-1] - heightMap[y][x + 1]; //difference in height of previous vertex and next vertex along the x axis
			float normZ = heightMap[y-1][x] - heightMap[y+1][x]; //difference in height of previous vertex and next vertex along the z axis	
			plane_mb.vertices[i].norm = normalize(vec3(normX, 2, normZ));
		}
	}


	//generate texture transition offsets
	for (int y = 0; y <= numTrianglesAcross; y++) {
		for (int x = 0; x <= numTrianglesAcross; x++) {
			int i = y*(numTrianglesAcross + 1) + x;
			plane_mb.vertices[i].offset = homogeneousfbm(x * 1, y * 1, 5);
		}
	}

	return plane_mb.build();
}


mesh_builder TerrainRenderer::generatePlane(float size, int numTrianglesAcross) {

	std::vector<vec3> vertices;
	std::vector<vec2> positions;
	std::vector<int> indices;

	float stepSize = size / numTrianglesAcross;

	for (int y = 0; y <= numTrianglesAcross; y++) {
		for (int x = 0; x <= numTrianglesAcross; x++) {
			//make vertex
			vertices.push_back(vec3(x*stepSize, 0, y*stepSize));
			positions.push_back(vec2(x,y));

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
						positions[i],
						0.0f}); //uv
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














