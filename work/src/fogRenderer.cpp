
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "fogRenderer.hpp"
#include "water/WaterRenderer.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;

static double framerate = 1.0 / 60.0;
double previousTime = glfwGetTime();
double deltaTime = 0;
double currentTime = 0;


void basic_fog_model::draw(const glm::mat4& view, const glm::mat4 proj) {

}


FogRenderer::FogRenderer() {

}


void FogRenderer::render(const glm::mat4& view, const glm::mat4& proj) {
	currentTime = glfwGetTime();
	deltaTime += (currentTime - previousTime) / framerate;
	previousTime = currentTime;
	if (deltaTime >= 1.0) {
		frameIndex = frameIndex + indexSpeed;
		frameIndex2 = frameIndex2 + 0.005;
		deltaTime = 0;
	}
}


void FogRenderer::renderGUI() {
	ImGui::SliderFloat("Near", &near, 0.0, 1, "");
	if (ImGui::SliderFloat("Far", &far, 0.0, 20, ""))
        WaterRenderer::setSceneUpdated();
	ImGui::SliderFloat("Speed", &indexSpeed, 0.01, 0.1, "");
	ImGui::SliderFloat("Amplitude", &amplitude, 0.01, 0.1, "");
	ImGui::SliderFloat("Period", &period, 4, 0.5, "");
}
