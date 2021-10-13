#define GLM_ENABLE_EXPERIMENTAL

// std
#include <iostream>
#include <string>
#include <stdexcept>

// project
#include "application.hpp"
#include "opengl.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_image.hpp"
#include <glm/gtx/string_cast.hpp>


using namespace std;
using namespace cgra;

float cool = 7;

// forward decleration for cleanliness
namespace
{
    void cursorPosCallback(GLFWwindow *, double xpos, double ypos);
    void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods);
    void scrollCallback(GLFWwindow *win, double xoffset, double yoffset);
    void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods);
    void charCallback(GLFWwindow *win, unsigned int c);
    void framebufferResize(GLFWwindow *win, int width, int height);
    void APIENTRY debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, GLvoid *);

    // global static pointer to application once we create it
    // nessesary for interfacing with the GLFW callbacks
    Application *application_ptr = nullptr;
}

// main program
//
int main() {
	// initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // unrecoverable error
	}

	// force OpenGL to create a 3.3 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// disallow legacy functionality (helps OS X work)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// request a debug context so we get debug callbacks
	// remove this for possible GL performance increases
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// create a windowed mode window and its OpenGL context
	GLFWwindow *window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
	if (!window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // unrecoverable error
	}

	// make the window's context current.
	// if we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(window);

	// initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // unrecoverable error
	}

	// print out our OpenGL versions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;

	// enable GL_ARB_debug_output if available (not necessary, just helpful)
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// this allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		// setup up the callback
		glDebugMessageCallbackARB(debugCallback, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}

	// initialize ImGui
	if (!cgra::gui::init(window)) {
		cerr << "Error: Could not initialize ImGui" << endl;
		abort(); // unrecoverable error
	}

	// attach input callbacks to window
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCharCallback(window, charCallback);
    glfwSetFramebufferSizeCallback(window, framebufferResize);

	// create the application object (and a global pointer to it)
	Application application(window);
	application_ptr = &application;

	//More Dumb stuff
	float quadVertices[] = { //Mesh used to get the image on screen
	 // positions   // texCoords
	 -1.0f,  1.0f,  0.0f, 1.0f,
	 -1.0f, -1.0f,  0.0f, 0.0f,
	  1.0f, -1.0f,  1.0f, 0.0f,

	 -1.0f,  1.0f,  0.0f, 1.0f,
	  1.0f, -1.0f,  1.0f, 0.0f,
	  1.0f,  1.0f,  1.0f, 1.0f
	};

	//Get screen resolution
	int w, h;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	//Shader for same as above
	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fog//framebuffer_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//fog//framebuffer_frag.glsl"));
	GLuint shader = sb.build();

	//Even more dumb stuff
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorBuffer;
	glGenTextures(1, &textureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	unsigned int depthBuffer;
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//Texture of rendered image should now be created

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "its not complete lol" << "\n";
	}

	//Load fog texture
	GLuint fogTexture;
	//fogTexture = rgba_image("fogTexture.png").uploadTexture;
	rgba_image test = rgba_image(CGRA_SRCDIR + string("/res/textures/fogTexture.png"));
	fogTexture = test.uploadTexture();

	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "fogTexture"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fogTexture);


	// loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		//Fix to get correct window size
		glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, depthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		//The usual
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		application.render();
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind your FBO to set the default framebuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader); // shader program for rendering the quad

		glUniform1i(glGetUniformLocation(shader, "originalOutput"),0);
		glUniform1i(glGetUniformLocation(shader, "depthBuffer"), 1);
		glUniform1i(glGetUniformLocation(shader, "fogTexture"), 2);

		glUniform1f(glGetUniformLocation(shader, "waveOffset"), application.fog_renderer->frameIndex);
		glUniform1f(glGetUniformLocation(shader, "amplitude"), application.fog_renderer->amplitude);
		glUniform1f(glGetUniformLocation(shader, "period"), application.fog_renderer->period);


		glUniform1f(glGetUniformLocation(shader, "near"), application.fog_renderer->near);
		glUniform1f(glGetUniformLocation(shader, "far"), application.fog_renderer->far);

		glUniform1f(glGetUniformLocation(shader, "state"), application.show_fog);
		glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(application.fog_renderer->projectionMatrix));
		glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(application.fog_renderer->viewMatrix));

		//std::cout << glm::to_string(application.fog_renderer.viewMatrix) << std::endl;


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer); // color attachment texture

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthBuffer); // color attachment texture

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fogTexture);


		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, depthBuffer); // color attachment texture
		glBindVertexArray(quadVAO);
		// You can also use VAO or attribute pointers instead of only VBO...
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		cgra::gui::newFrame();
		application.renderGUI();
		cgra::gui::render();

		glfwSwapBuffers(window);

		// poll for and process events
		glfwPollEvents();
	}

	// clean up ImGui
	cgra::gui::shutdown();
	glfwTerminate();
}

namespace
{
    void framebufferResize(GLFWwindow *win, int width, int height)
    {
        application_ptr->resize(width, height);
    }

    void cursorPosCallback(GLFWwindow *, double xpos, double ypos)
    {
        // if not captured then foward to application
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;
        application_ptr->cursorPosCallback(xpos, ypos);
    }

    void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods)
    {
        // forward callback to ImGui
        cgra::gui::mouseButtonCallback(win, button, action, mods);

        // if not captured then foward to application
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;
        application_ptr->mouseButtonCallback(button, action, mods);
    }

    void scrollCallback(GLFWwindow *win, double xoffset, double yoffset)
    {
        // forward callback to ImGui
        cgra::gui::scrollCallback(win, xoffset, yoffset);

        // if not captured then foward to application
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;
        application_ptr->scrollCallback(xoffset, yoffset);
    }

    void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods)
    {
        // forward callback to ImGui
        cgra::gui::keyCallback(win, key, scancode, action, mods);

        // if not captured then foward to application
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureKeyboard)
            return;
        application_ptr->keyCallback(key, scancode, action, mods);
    }

    void charCallback(GLFWwindow *win, unsigned int c)
    {
        // forward callback to ImGui
        cgra::gui::charCallback(win, c);

        // if not captured then foward to application
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantTextInput)
            return;
        application_ptr->charCallback(c);
    }

    // function to translate source to string
    const char *getStringForSource(GLenum source)
    {
        switch (source)
        {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Application";
        case GL_DEBUG_SOURCE_OTHER:
            return "Other";
        default:
            return "n/a";
        }
    }

    // function to translate severity to string
    const char *getStringForSeverity(GLenum severity)
    {
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            return "High";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "Medium";
        case GL_DEBUG_SEVERITY_LOW:
            return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return "None";
        default:
            return "n/a";
        }
    }

    // function to translate type to string
    const char *getStringForType(GLenum type)
    {
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "Deprecated Behaviour";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "Undefined Behaviour";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "Performance";
        case GL_DEBUG_TYPE_OTHER:
            return "Other";
        default:
            return "n/a";
        }
    }

    // actually define the function
    void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar *message, GLvoid *)
    {
        // Don't report notification messages
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        // nvidia: avoid debug spam about attribute offsets
        if (id == 131076)
            return;

        cerr << "GL [" << getStringForSource(source) << "] " << getStringForType(type) << ' ' << id << " : ";
        cerr << message << " (Severity: " << getStringForSeverity(severity) << ')' << endl;

        if (type == GL_DEBUG_TYPE_ERROR_ARB)
            throw runtime_error("GL Error: "s + message);
    }
}
