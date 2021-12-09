#define GL_GLEXT_PROTOTYPES

// Library headers
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"

// Custom headers
#define STB_IMAGE_IMPLEMENTATION // Needed for headers to compile
#include "OpenGLHeaders/camera.h"
#include "OpenGLHeaders/texture.h"
#include "OpenGLHeaders/shader.h"
#include "VSEPR.h"
#include "Sphere.h"
#include "cylinder.h"
#include "render.h"

// STD headers
#include <vector>
#include <thread>
#include <iostream>
#include <string>
#include <cmath>
#include <utility>

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))

int representation = 1; // 0 = electron, 1 = sphere, 2 = ball and stick

// Local VSEPRModel for the rendering thread
std::vector<BondedElement> VSEPRModel;

// State variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool restrictY = true;
bool black = true;

// Rendering constants
const float atomDistance = 3.5f;
const float electronSpeed = 3;
const float lineColor[] = {0.2f, 0.2f, 0.2f, 1};
const float stickSetWidth = 3.0f;

// Definitions of extern variables
unsigned int sphereVAO;
unsigned int sphereVBO;
unsigned int cylinderVAO;
unsigned int cylinderVBO;
unsigned int fastSphereVAO;
unsigned int fastSphereVBO;
unsigned int fastCylinderVAO;
unsigned int fastCylinderVBO;

// Define offset variables
float lastX = W / 2;
float lastY = H / 2;
float yaw = -90; float pitch = 0;
bool firstMouse = true;
float fov = 45.0f;

// Major class definitions
Camera camera(glm::vec3(0.0f, 0.0f, CAMERA_DISTANCE), glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch);
// const Sphere sphere(1.0f, 36, 18, false); //Blocky
const Sphere sphere(1.0f, 36, 18, true); //Smooth
const Sphere sphere_fast(1.0f, 18, 9, true);
const Cylinder cylinder(0.125f, atomDistance / 2, 64);
const Cylinder cylinder_fast(0.125f, atomDistance, 32);
Shader lightingShader;
Shader lampProgram;

/**
 * Sets the size of the framebuffer
 * 
 * @param window the GLFW window to get the framebuffer for
 * @param width the window width
 * @param height the window height
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

/**
 * Process mouse movements
 * 
 * @param window the GLFW window where the event occurred
 * @param xpos the cursor x coord
 * @param ypos the cursor y coord
*/
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	//camera.ProcessMouseMovement(xoffset, yoffset, true);
	if(clicked) {
		camera.ProcessArcBall(xpos, ypos);
	}
}

/**
 * Process keyboard input
 * 
 * @param window the GLFW window where the event occurred
 * @param key integer keycode (translated scancode)
 * @param scancode key scancode (raw input, system-specific)
 * @param action press/release
 * @param mods bitfield for modifier keys (e.g. shift)
*/
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_R))
	{
		representation++;
		representation = representation % 3;
		if (VSEPRModel.size() > 7 && representation == 0) {
			representation++;
		}

		if (representation != 0) {
			lightingShader = Shader(LIT_MODEL_VERT_PATH, LIT_MODEL_DIR_FRAG_PATH);
			lightingShader.use();
			lightingShader.setVec3(LIGHT_OBJ(DIRECTION), glm::vec3(-0.2f, -1.0f, -0.3f));
			lightingShader.setVec3(LIGHT_OBJ(AMBIENT), glm::vec3(0.4, 0.4, 0.4));
			lightingShader.setVec3(LIGHT_OBJ(DIFFUSE), glm::vec3(0.5, 0.5, 0.5));
			lightingShader.setVec3(LIGHT_OBJ(SPECULAR), glm::vec3(0.5, 0.5, 0.5));
			lightingShader.setInt(MATERIAL_OBJ(DIFFUSE), 0);
			lightingShader.setInt(MATERIAL_OBJ(SPECULAR), 1);
			lightingShader.setFloat(MATERIAL_OBJ(SHININESS), 32.0f);
		}
		else {
			lightingShader = Shader(LIT_MODEL_VERT_PATH, LIT_MODEL_POINT_FRAG_PATH);
			lightingShader.use();
			lightingShader.setInt(MATERIAL_OBJ(DIFFUSE), 0);
			lightingShader.setInt(MATERIAL_OBJ(SPECULAR), 1);
			lightingShader.setFloat(MATERIAL_OBJ(SHININESS), 32.0f);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_C)) {
		black = !black;
	}
}

/**
 * Process scroll wheel events
 * 
 * @param window the GLFW window where the event occurred
 * @param xoffset the horizontal scroll (most mice cannot do this)
 * @param yoffset the vertical scroll
*/
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(window, yoffset);
}

/**
 * Handle mouse clicks
 * 
 * @param window the GLFW window where the event occurred
 * @param button the code for the button that was pressed
 * @param action press/release
 * @param mods bitfield for modifier keys (e.g. shift, alt)
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS) {
			clicked = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			camera.SetRadius(xpos, ypos);
		}
		else
			clicked = false;
	}
}

int main()
{
	clicked = false;
	std::thread VSEPRthread(VSEPRMain);

	//Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a GLFW Window
	GLFWwindow* window = glfwCreateWindow(W, H, "VSEPR Models", NULL, NULL);
	glfwMakeContextCurrent(window);

	//glad init: intializes all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Set size of rendering window
	glViewport(0, 0, W, H);

	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Create a Vertex Array Object
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.getInterleavedVertexSize(), sphere.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)(sizeof(float)*3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)(sizeof(float)*6));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &cylinderVAO);
	glBindVertexArray(cylinderVAO);

	glGenBuffers(1, &cylinderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, cylinder.getInterleavedVertexSize(), cylinder.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	//VAO and VBOs for low-poly sphere and cylinder
	glGenVertexArrays(1, &fastSphereVAO);
	glBindVertexArray(fastSphereVAO);

	glGenBuffers(1, &fastSphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fastSphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere_fast.getInterleavedVertexSize(), sphere_fast.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sphere_fast.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sphere_fast.getInterleavedStride(), (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sphere_fast.getInterleavedStride(), (void *)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &fastCylinderVAO);
	glBindVertexArray(fastCylinderVAO);

	glGenBuffers(1, &fastCylinderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fastCylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, cylinder_fast.getInterleavedVertexSize(), cylinder_fast.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cylinder_fast.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cylinder_fast.getInterleavedStride(), (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, cylinder_fast.getInterleavedStride(), (void *)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	//Create light cube VAO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glm::vec3 objectColour = glm::vec3(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

	lightingShader = Shader(LIT_MODEL_VERT_PATH, LIT_MODEL_DIR_FRAG_PATH);
	lightingShader.use();
	lightingShader.setVec3(LIGHT_OBJ(DIRECTION), glm::vec3(-0.2f, -1.0f, -0.3f));
	lightingShader.setVec3(LIGHT_OBJ(AMBIENT), glm::vec3(0.4, 0.4, 0.4));
	lightingShader.setVec3(LIGHT_OBJ(DIFFUSE), glm::vec3(0.5, 0.5, 0.5));
	lightingShader.setVec3(LIGHT_OBJ(SPECULAR), glm::vec3(0.5, 0.5, 0.5));
	lightingShader.setInt(MATERIAL_OBJ(DIFFUSE), 0);
	lightingShader.setInt(MATERIAL_OBJ(SPECULAR), 1);
	lightingShader.setFloat(MATERIAL_OBJ(SHININESS), 32.0f);

	lampProgram = Shader(POINT_LIGHT_VERT_PATH, POINT_LIGHT_FRAG_PATH);

	unsigned int diffMap;
	glActiveTexture(GL_TEXTURE0);
	loadTexture(diffMap, FLAT_TEXTURE_PATH);

	unsigned int specMap;
	glActiveTexture(GL_TEXTURE1);
	loadTexture(specMap, FLAT_TEXTURE_PATH);

	glEnable(GL_DEPTH_TEST);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Set mouse input callback function
	void mouse_callback(GLFWwindow *window, double xpos, double ypos);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	float time = 0;

	//Render Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        VSEPRModel = mutateModel();

		camera.ProcessKeyboard(window, deltaTime, false);

		if(black) {
			glClearColor(0, 0, 0, 1);
		}
		else {
			glClearColor(1, 1, 1, 1);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

		if(!clicked) {
			time += deltaTime * camera.RotationSpeed;
		}
		glm::mat4 model;
		glm::mat4 rotationModel = glm::rotate(glm::mat4(), time, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationModel *= camera.GetArcMatrix();
		glm::mat4 reverseRotationModel = glm::rotate(model, -time, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), W / H, 0.1f, 100.0f);

		lightingShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specMap);

		if (organic || VSEPRModel.size() > 7) {
			lightingShader.setMat4(MODEL, model);
			lightingShader.setMat4(VIEW, view);
			lightingShader.setMat4(PROJECTION, projection);
			lightingShader.setVec3(VIEW_POS, camera.Position);

			renderOrganic(VSEPRModel, lightingShader, rotationModel, representation);

			//Swap buffer and poll IO events
			glfwSwapBuffers(window);
			glfwPollEvents();

			continue;
		}

		if(representation == 0) {
			lampProgram.use();
			lampProgram.setMat4(VIEW, view);
			lampProgram.setMat4(PROJECTION, projection);
			renderElectrons(lampProgram, lightingShader, VSEPRModel, rotationModel);
		}

		//Pass our matrices to the shader through a uniform
		lightingShader.setMat4(MODEL, model);
		lightingShader.setMat4(VIEW, view);
		lightingShader.setMat4(PROJECTION, projection);
		lightingShader.setVec3(VIEW_POS, camera.Position);

		// Draw spheres
		if (VSEPRModel.size() > 0) {
			renderSimpleCompound(VSEPRModel, rotationModel, lightingShader, representation);
		}
		else {
			model = glm::mat4();
			lightingShader.setMat4(MODEL, model);
			sphere.draw();
		}
        
        // Done drawing for this frame so tell the model it's safe to update
        readyFrameUpdate();

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
