#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"

#define STB_IMAGE_IMPLEMENTATION //Needed for headers to compile
#include "OpenGLHeaders/Camera.h"
#include "OpenGLHeaders/Texture.h"
#include "OpenGLHeaders/Shader.h"
#include "VSEPR.h"
#include "Sphere.h"
#include "cylinder.h"

#include <vector>
#include <thread>
#include <iostream>
#include <string>
#include <cmath>

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))

///VBOs Vertex Buffer Objects contain vertex data that is sent to memory in the GPU, vertex attrib calls config bound VBO
///VAOs Vertex Array Objects when bound, any vertex attribute calls and attribute configs are stored in VAO
///Having multiple VAOs allow storage of multiple VBO configs, before drawing, binding VAO with right config applies to draw
///Vertex attributes are simply inputs of the vertex shader, configured so vertex shader knows how to interpret data
///EBOs Element Buffer Objects stores indices used for OpenGL to know which vertices to draw
///EBOs used when needing multiple triangles to render but triangles have overlapping points
///E.G. rectangle made with 2 triangles have 2 overlapping vertices
///Vertex shader changes 3D coords of vertices
///Fragment shader sets color of pixels
///OpenGL objects are used to reference pieces of OpenGL state machine
///E.G. when creating a shader program,  a program is created in the state machine and its ID
///is passed to the program object
///Similarly, with the VBO, the real VBO is stored in the background state machine, the
///object holds the ID of the real object and its value is bound to the real object
///With ALL (or most) OpenGL objects, they must be bound so that any function calls for that object type configures the
///ID you created
///Depth information stored in Z buffer, depth testing done automatically, must be enabled
///Depth buffer must also be cleared in the clear function

int representation = 0; //0 = electron, 1 = sphere, 2 = ball and stick
const float W = 800;
const float H = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool restrictY = true;
const float atomDistance = 4;
const float electronSpeed = 3;

std::vector<BondedElement> VSEPRModel;
std::vector<std::vector<glm::vec3>> configurations;
//Define offset variables
float lastX = W / 2;
float lastY = H / 2;
float yaw = -90; float pitch = 0;
bool firstMouse = true;
float fov = 45.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), yaw, pitch);
// const Sphere sphere(1.0f, 36, 18, false); //Blocky
const Sphere sphere(1.0f, 36, 18, true); //Smooth
const Cylinder cylinder(1.0f, 1.0f, 25);
unsigned int lightingShader = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
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

	camera.ProcessMouseMovement(xoffset, yoffset, true);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_R))
	{
		representation++;
		representation = representation % 3;

		if (representation != 0) {
			Shader("Shaders/VeShMap.vs", "Shaders/FrShDirectional.fs", lightingShader);
			glUseProgram(lightingShader);
			setVec3(lightingShader, "light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
			setVec3(lightingShader, "light.ambient", glm::vec3(0.2, 0.2, 0.2));
			setVec3(lightingShader, "light.diffuse", glm::vec3(0.3, 0.3, 0.3));
			setVec3(lightingShader, "light.specular", glm::vec3(0.7, 0.7, 0.7));
			setInt(lightingShader, "material.diffuse", 0);
			setInt(lightingShader, "material.specular", 1);
			setFloat(lightingShader, "material.shininess", 32.0f);
		}
		else {
			Shader("Shaders/VeShMap.vs", "Shaders/FrShMap.fs", lightingShader);
			glUseProgram(lightingShader);
			setInt(lightingShader, "material.diffuse", 0);
			setInt(lightingShader, "material.specular", 1);
			setFloat(lightingShader, "material.shininess", 32.0f);
		}
	}
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	camera.RotationSpeed += yoffset * smoothingConstant;
}

void setUpPointLights(int num, unsigned int &program) {
	glUseProgram(program);

	for(int i = 0; i < num; i++) {
		setVec3(program, ("pointLights[" + std::to_string(i) + "].ambient").c_str(), glm::vec3(0.05, 0.05, 0.05));
		setVec3(program, ("pointLights[" + std::to_string(i) + "].diffuse").c_str(), glm::vec3(0.3, 0.3, 0.3));
		setVec3(program, ("pointLights[" + std::to_string(i) + "].specular").c_str(), glm::vec3(0.7, 0.7, 0.7));
		setFloat(program, ("pointLights[" + std::to_string(i) + "].constant").c_str(), 1.0f);
		setFloat(program, ("pointLights[" + std::to_string(i) + "].linear").c_str(), 0.09f);
		setFloat(program, ("pointLights[" + std::to_string(i) + "].quadratic").c_str(), 0.032f);
	}
	for(int i = num; i < MAX_POINT_LIGHTS; i++) {
		setVec3(program, ("pointLights[" + std::to_string(i) + "].ambient").c_str(), glm::vec3(0.0f));
		setVec3(program, ("pointLights[" + std::to_string(i) + "].diffuse").c_str(), glm::vec3(0.0f));
		setVec3(program, ("pointLights[" + std::to_string(i) + "].specular").c_str(), glm::vec3(0.0f));
	}
}

void setPointLightPosition(int index, unsigned int &program, glm::vec3 pos) {
	glUseProgram(program);
	setVec3(program, ("pointLights[" + std::to_string(index) + "].position").c_str(), pos);
}

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

glm::vec3 calculateOrbitPosition(BondedElement central, BondedElement bonded, int configIndex, int modelIndex, int offset, int offsetTotal, bool pair, glm::mat4 rotationModel) {
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
	v = v * rotationModel;

	return glm::vec3(v.x, v.y, v.z);
}

glm::vec3 calculateOrbitPosition(glm::vec3 v, int configIndex, int modelIndex, bool pair) {
	if(pair) {
		glm::mat4 transform;
		glm::vec4 v4 = glm::vec4(v, 1.0f);
		transform = glm::rotate(transform, 180.0f * (PI / 180.0f), (configurations[configIndex][modelIndex]));
		glm::vec3 test = configurations[configIndex][modelIndex];
		printf("X: %f Y: %f Z: %f \n", test.x, test.y, test.z);
		v4 = v4 * transform;
		return glm::vec3(v4);
	}
	return v;
}

void renderElectrons(unsigned int program, unsigned int &atomProgram, std::vector<BondedElement> model, glm::mat4 rotationModel) {
	glUseProgram(program);
	//glBindVertexArray(lightVAO);

	glm::mat4 lightModel;

	setMat4(program, "model", lightModel);
	//glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));

	glm::vec3 lightVec3;
	glm::vec3 light2Vec3;
	if (VSEPRModel.size() > 0)
	{
		int numberOfBonds = VSEPRModel[0].bondedPairs;
		int configIndex;
		if (VSEPRModel.size() > 2)
		{
			configIndex = VSEPRModel.size() - 2 + VSEPRModel[0].lonePairs;
		}
		else
		{
			configIndex = 0;
		}
		int lightIndex = 0;
		for (int i = 1; i < VSEPRModel.size(); i++)
		{
			for (int x = 0; x < VSEPRModel[i].bondedPairs; x++)
			{
				lightModel = glm::mat4();
				glm::vec3 newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedPairs, false, rotationModel);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				glUseProgram(program);
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				setMat4(program, "model", lightModel);
				sphere.draw();
				lightIndex++;

				//Draw complimentary
				lightModel = glm::mat4();
				newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedPairs, true, rotationModel);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				glUseProgram(program);
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				setMat4(program, "model", lightModel);
				sphere.draw();
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
		glUseProgram(program);
		lightModel = glm::translate(lightModel, lightVec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		setMat4(program, "model", lightModel);
		sphere.draw();
		lightModel = glm::mat4();
		lightModel = glm::translate(lightModel, light2Vec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		setMat4(program, "model", lightModel);
		sphere.draw();
	}
}

float getSphereDistance(std::vector<BondedElement> model, int index) {
	// return model[0].base.covalentRadius + model[index].base.covalentRadius - 0.09 * abs(model[0].base.electronegativity - model[index].base.electronegativity);
	return model[0].base.covalentRadius + model[index].base.covalentRadius;
}

float getStickDistance(std::vector<BondedElement> model, int index) {
	return atomDistance;
}

int main()
{
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
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int sphereVBO;
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.getInterleavedVertexSize(), sphere.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)(sizeof(float)*3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sphere.getInterleavedStride(), (void *)(sizeof(float)*6));
	glEnableVertexAttribArray(2);

	unsigned int cylinderVAO;
	glGenVertexArrays(1, &cylinderVAO);
	glBindVertexArray(cylinderVAO);

	unsigned int cylinderVBO;
	glGenBuffers(1, &cylinderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, cylinder.getInterleavedVertexSize(), cylinder.getInterleavedVertices(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, cylinder.getInterleavedStride(), (void *)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	//Create light cube VAO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glm::vec3 objectColour = glm::vec3(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

	Shader("Shaders/VeShMap.vs", "Shaders/FrShMap.fs", lightingShader);
	glUseProgram(lightingShader);
	setInt(lightingShader, "material.diffuse", 0);
	setInt(lightingShader, "material.specular", 1);
	setFloat(lightingShader, "material.shininess", 32.0f);
	setUpPointLights(2, lightingShader);

	unsigned int lampProgram = 0;
	Shader("Shaders/VeShColors.vs", "Shaders/FrShLight.fs", lampProgram);

	unsigned int diffMap;
	glActiveTexture(GL_TEXTURE0);
	loadTexture(diffMap, "RedTexture.png");

	unsigned int specMap;
	glActiveTexture(GL_TEXTURE1);
	loadTexture(specMap, "RedTexture.png");

	glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Set mouse input callback function
	void mouse_callback(GLFWwindow *window, double xpos, double ypos);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	float time = 0;

	//Render Loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		camera.ProcessKeyboard(window, deltaTime, false);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

		float lineColor[] = {0.2f, 0.2f, 0.2f, 1};

		time += deltaTime*camera.RotationSpeed;
		glm::mat4 model;
		glm::mat4 rotationModel = glm::rotate(model, time, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), W / H, 0.1f, 100.0f);

		glUseProgram(lightingShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specMap);

		if(representation == 0) {
			glUseProgram(lampProgram);
			setMat4(lampProgram, "view", view);
			setMat4(lampProgram, "projection", projection);
			renderElectrons(lampProgram, lightingShader, VSEPRModel, rotationModel);
		}

		//Pass our matrices to the shader through a uniform
		setMat4(lightingShader, "model", model);
		setMat4(lightingShader, "view", view);
		setMat4(lightingShader, "projection", projection);
		setVec3(lightingShader, "viewPos", camera.Position);

		//Draw spheres
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if(VSEPRModel.size() > 0) {
			int configIndex;
			float bondDistance;
			if(VSEPRModel.size() > 2) {
				configIndex = VSEPRModel.size() - 2 + VSEPRModel[0].lonePairs;
			}
			else {
				configIndex = VSEPRModel.size() - 2;
			}
			for (int i = 1; i < VSEPRModel.size() + VSEPRModel[0].lonePairs; i++) {
				model = glm::mat4();
				if(i < VSEPRModel.size()) {
					bondDistance = representation == 1 ? getSphereDistance(VSEPRModel, i) : getStickDistance(VSEPRModel, i);
				}
				else if(representation == 0) {
					bondDistance = getStickDistance(VSEPRModel, 0);
				}
				else  {continue;}
				glm::vec4 v = glm::vec4(configurations[configIndex][i - 1] * bondDistance, 1.0f);
				glm::vec3 v3 = glm::vec3(v * rotationModel);
				model = glm::translate(model, v3);
				model = glm::rotate(model, -time, glm::vec3(0.0f, 1.0f, 0.0f));
				if (i < VSEPRModel.size() && representation == 1) {
					model = glm::scale(model, glm::vec3(VSEPRModel[i].base.vanDerWaalsRadius));
				}
				else if(representation != 1) {
					model = glm::scale(model, i < VSEPRModel.size() ? glm::vec3(VSEPRModel[i].base.atomicRadius) : glm::vec3(VSEPRModel[0].base.atomicRadius * 0.8));
				}
				setMat4(lightingShader, "model", model);
				if(i < VSEPRModel.size()) {
					glBindVertexArray(cylinderVAO);
					glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
					cylinder.draw();
					glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
					//sphere.draw();
				}
				else if(VSEPRModel.size() > 2) {
					sphere.drawLines(lineColor);
				}
			}
			model = glm::mat4();
			model = glm::scale(model, glm::vec3(representation == 1 ? VSEPRModel[0].base.vanDerWaalsRadius : VSEPRModel[0].base.atomicRadius));
			setMat4(lightingShader, "model", model);
			sphere.draw();
		}
		else {
			model = glm::mat4();
			setMat4(lightingShader, "model", model);
			sphere.draw();
		}

		//Swap buffer and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}