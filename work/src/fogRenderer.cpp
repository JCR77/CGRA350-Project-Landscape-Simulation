
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "fogRenderer.hpp"
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
	mat4 modelview = view * modelTransform;

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	//drawCylinder();
	mesh.draw(); // draw
}


FogRenderer::FogRenderer() {

	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fog//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fog//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 0, 0);

	


	
}


void FogRenderer::render(const glm::mat4& view, const glm::mat4& proj) {
	currentTime = glfwGetTime();
	deltaTime += (currentTime - previousTime) / framerate;
	previousTime = currentTime;
	if (deltaTime >= 1.0) {
		frameIndex = frameIndex + indexSpeed;
		deltaTime = 0;
	}
	viewMatrix = view;
	projectionMatrix = proj;
	m_model.draw(view, proj);
	int w, h;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32F , w,  h, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	
}


void FogRenderer::renderGUI() {

	// example of how to use input boxes
	ImGui::InputFloat("Near", &near);
	ImGui::InputFloat("Far", &far);
	ImGui::InputFloat("Speed", &indexSpeed);
	ImGui::InputFloat("Amplitude", &amplitude);
	ImGui::InputFloat("Period", &period);
	if (ImGui::Button("Enable/Disable Fog")) 
	{
		if (state == 0.0f)
		{
			state = 1.0f;
		}
		else
		{
			state = 0.0f;
		}
	}
}

