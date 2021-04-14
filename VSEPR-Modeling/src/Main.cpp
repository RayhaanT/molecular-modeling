// Library headers
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"

// Custom headers
#define STB_IMAGE_IMPLEMENTATION // Needed for headers to compile
#include "OpenGLHeaders/Camera.h"
#include "OpenGLHeaders/Texture.h"
#include "OpenGLHeaders/ShaderClass.h"
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
 * Set up a given number of electron lights in the shader
 * 
 * @param num the number of electron lights
 * @param program a reference to the shader program
*/
void setUpPointLights(int num, Shader &program) {
	program.use();

	for(int i = 0; i < num; i++) {
		program.setVec3(POINT_LIGHT_LIST(i, AMBIENT), glm::vec3(0.05, 0.05, 0.05));
		program.setVec3(POINT_LIGHT_LIST(i, DIFFUSE), glm::vec3(0.3, 0.3, 0.3));
		program.setVec3(POINT_LIGHT_LIST(i, SPECULAR), glm::vec3(0.7, 0.7, 0.7));
		program.setFloat(POINT_LIGHT_LIST(i, CONSTANT), 1.0f);
		program.setFloat(POINT_LIGHT_LIST(i, LINEAR), 0.09f);
		program.setFloat(POINT_LIGHT_LIST(i, QUADRATIC), 0.032f);
	}
	for(int i = num; i < MAX_POINT_LIGHTS; i++) {
		program.setVec3(POINT_LIGHT_LIST(i, AMBIENT), glm::vec3(0.0f));
		program.setVec3(POINT_LIGHT_LIST(i, DIFFUSE), glm::vec3(0.0f));
		program.setVec3(POINT_LIGHT_LIST(i, SPECULAR), glm::vec3(0.0f));
	}
}

/**
 * Update the position of an electron point light
 * 
 * @param index the index of the electron to move
 * @param program a reference to the shader program
 * @param pos the new position for the light
*/
void setPointLightPosition(int index, Shader &program, glm::vec3 pos) {
	program.use();
	program.setVec3(POINT_LIGHT_LIST(index, POSITION), pos);
}

/**
 * Get a quaternion that would rotate the initial vector
 * to the target vector's direction
 * 
 * @param start the intial vector
 * @param dest the target vector
 * @return a quaternion describing the rotation
*/
glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f)
	{
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;
	
	return glm::quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs);
}

/**
 * Calculate the position of an electron point light traveling
 * its' figure-8 pattern
 * 
 * @param central the central atom of the compound
 * @param bonded the peripheral atom also in the bond
 * @param configIndex the index of the VSEPR configuration
 * @param modelIndex the index of the bond within the VSEPR config
 * @param offset which bond within the total order the elctron belongs to
 * @param offsetTotal the total bond order
 * @param pair whether its the first or second electron in a pair
 * @return the current position of the described electron
*/
glm::vec3 calculateOrbitPosition(BondedElement central, BondedElement bonded, int configIndex, int modelIndex, int offset, int offsetTotal, bool pair) {
	float largerAR = central.base.atomicRadius;
	float smallerAR = bonded.base.atomicRadius;
	if(largerAR < smallerAR) {
		float holdAR = largerAR;
		largerAR = smallerAR;
		smallerAR = largerAR;
	}
	
	float ePI = (2*PI)/electronSpeed;
	float xOffset = ((2.0f / offsetTotal) * offset * ePI) - (4 / offsetTotal * ePI);

	float x = 1.4 * largerAR * sin((float)(glfwGetTime()-xOffset) * electronSpeed);
	float distance = atomDistance;
	float y = distance * cos((float)(glfwGetTime() - xOffset) * (electronSpeed / 2)) + distance / 2;

	if(modelIndex-1 >= configurations[configIndex].size() || configIndex >= configurations.size()) {
		return glm::vec3(0.0f);
	}
	glm::vec3 direction = configurations[configIndex][modelIndex - 1];
	glm::mat4 transform;
	glm::vec4 v = glm::vec4(x, y, 0.0f, 0.0f);
	
	if(pair) {
		transform = glm::rotate(transform, 180.0f * (PI / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	transform = glm::rotate(transform, (PI)/offsetTotal * offset, glm::vec3(0.0f, 1.0f, 0.0f));
	v = v * transform;
	transform = glm::toMat4(RotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f) * distance, direction * distance));
	transform = glm::rotate(transform, 180.0f * (PI/ 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	v = v * transform;

	return glm::vec3(v.x, v.y, v.z);
}

/**
 * Get a matrix describing the required translation
 * to render a cylinder for ball-and-stick models
 * 
 * @param bondOrder a pair describing bond orders
 *                  [0] = total bond order
 *                  [1] = the bond this cylinder is for
 * @param rotationModel the rotation of the system at large
 * @param direction the direction the cylinder is facing
 * @param cylinderModel the existing model to apply the translation to
 * @return the translated transformation matrix
*/
glm::mat4 getCylinderOffset(std::pair<int, int> bondOrder, glm::mat4 rotationModel, glm::vec3 direction, glm::mat4 cylinderModel) {
	if(bondOrder.first == 1 || (bondOrder.first == 3 && bondOrder.second == 2)) {
		return cylinderModel;
	}
	
	glm::mat4 finalMat = glm::mat4();
	glm::vec3 up = glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * rotationModel);
	glm::vec3 left = glm::cross(direction, up);
	float length = 0;
	if(bondOrder.first == 2) {
		length = bondOrder.second == 1 ? 1.0f/2.7 : -1.0f/2.7;
	}
	else if(bondOrder.first == 3) {
		length = bondOrder.second == 1 ? 1.0f / 2.7 : -1.0f / 2.7;
	}

	left = glm::normalize(left) * length;
	finalMat = glm::translate(finalMat, left);
	finalMat *= cylinderModel;

	return finalMat;
}

/**
 * Get the matrix to properly position a cylinder
 * for ball-and-stick models
 * 
 * @param configIndex the VSEPR configuration index
 * @param modelIndex the bond index within the VSEPR config
 * @param bondOrder a pair describing bond orders
 *                  [0] = total bond order
 *                  [1] = the bond this cylinder is for
 * @param rotationModel the rotation of the system at large
 * @return the final transformation for the described cylinder
 */
glm::mat4 getCylinderRotation(int configIndex, int modelIndex, std::pair<int, int> bondOrder, glm::mat4 rotationModel) {
	glm::vec3 target = configurations[configIndex][modelIndex];
	glm::vec3 direction = glm::vec3(glm::vec4(target, 0.0f));
	
	glm::mat4 rotMatrix;
	rotMatrix = glm::toMat4(RotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f) * atomDistance, direction * atomDistance));
	rotMatrix = getCylinderOffset(bondOrder, rotationModel, direction, rotMatrix);

	return rotMatrix;
}

/**
 * Render all the electron point lights in the electron orbit model
 * 
 * @param program the electron shader program
 * @param atomProgram a reference to the atom shader program
 *                    needed to update point light positions
 * @param model the compound structure
 * @param rotationModel the camera rotation
 */
void renderElectrons(Shader program, Shader &atomProgram, std::vector<BondedElement> model, glm::mat4 rotationModel) {
	program.use();
	//glBindVertexArray(lightVAO);

	glm::mat4 lightModel;

	program.setMat4(MODEL, lightModel);	
	//glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));

	glm::vec3 lightVec3;
	glm::vec3 light2Vec3;
	if (VSEPRModel.size() > 0)
	{
		int numberOfBonds = VSEPRModel[0].bondedElectrons/2;
		int configIndex;
		if (VSEPRModel.size() > 2)
		{
			configIndex = VSEPRModel.size() - 2 + (VSEPRModel[0].loneElectrons/2);
		}
		else
		{
			configIndex = 0;
		}
		int lightIndex = 0;
		for (int i = 1; i < VSEPRModel.size(); i++)
		{
			for (int x = 0; x < VSEPRModel[i].bondedElectrons/2; x++)
			{
				lightModel = glm::mat4();
				glm::vec3 newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedElectrons/2, false);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				program.use();
				lightModel *= rotationModel;
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				program.setMat4(MODEL, lightModel);
				sphere.drawLines(lineColor);
				lightIndex++;

				//Draw complimentary
				lightModel = glm::mat4();
				newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedElectrons/2, true);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				program.use();
				lightModel *= rotationModel;
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				program.setMat4(MODEL, lightModel);
				sphere.drawLines(lineColor);
				lightIndex++;
			}
		}
		setUpPointLights(lightIndex, atomProgram);
	}
	else
	{
		lightVec3 = glm::vec3(1.2 * sin((float)(glfwGetTime())), 1.2 * cos((float)(glfwGetTime())), 0.0f);
		light2Vec3 = glm::vec3(1.5 * sin((float)(glfwGetTime())), 1.5 * cos((float)(glfwGetTime())), 0.0f);
		setPointLightPosition(0, atomProgram, lightVec3);
		setPointLightPosition(1, atomProgram, light2Vec3);
		program.use();
		lightModel = glm::translate(lightModel, lightVec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		program.setMat4(MODEL, lightModel);
		sphere.draw();
		lightModel = glm::mat4();
		lightModel = glm::translate(lightModel, light2Vec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		program.setMat4(MODEL, lightModel);
		sphere.draw();
	}
}

/**
 * Get the bond distance between two atoms
 * 
 * @param model the compound structure
 * @param index the index of the peripheral atom
 * @param order the bond order
 * @return the bond distance
 */
float getSphereDistance(std::vector<BondedElement> model, int index, int order) {
	// Schomaker and Stevenson formula for bond length (not used with current data)
	// return model[0].base.covalentRadius + model[index].base.covalentRadius - 0.09 * abs(model[0].base.electronegativity - model[index].base.electronegativity);

	// Sum of covalent radii based on bond order
	return (model[0].base.covalentRadii[order-1] + model[index].base.covalentRadii[order-1])/100;
}

/**
 * Get the bond distance between two atoms
 * 
 * @param a the first element
 * @param b the second element
 * @param order the bond order
 * @return the bond distance
 */
float getSphereDistance(BondedElement a, BondedElement b, int order) {
	return (a.base.covalentRadii[order-1] + b.base.covalentRadii[order-1])/100;
}

/**
 * Get the atomic distance for ball-and-stick models
 * 
 * @return the distance
 */
float getStickDistance() {
	return atomDistance;
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

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}