#pragma once

#define GL_GLEXT_PROTOTYPES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "VSEPR.h"

#include <iostream>
#include <vector>

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

#define C_PI 3.14159265359

// Default camera values
const float W = 800;
const float H = 600;
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVTY = 0.08f;
const float ZOOM = 45.0f;
const float CAMERA_DISTANCE = 20.0f;
const float CIRCUMFERENCE = CAMERA_DISTANCE * 2 * PI;
extern bool clicked;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void arcballScroll(double yOffset);

//Processes input and calculates euler values and view matrix
class Camera
{
public:
	//Camera properties
	glm::vec3 Position;
	glm::vec3 ballPos;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//Euler angles
	float Yaw;
	float Pitch;
	//Options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	float RotationSpeed = 0;
	const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	glm::mat4 GetViewMatrix();
	glm::mat4 GetArcMatrix();
	glm::mat4 GetReverseArcMatrix();

	void ProcessKeyboard(GLFWwindow *window, float deltaTime, bool restrictY);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessArcBall(float xPos, float yPos);
	void SetRadius(double xPos, double yPos);
	void ProcessMouseScroll(GLFWwindow *window, float yoffset);

private:
	float sphereRadius = 1.0f;
	glm::mat4 arcMatrix;
	glm::mat4 reverseArcMatrix;
	glm::vec2 lastPos2D = glm::vec2(0.0f);
	const float zoomSense = 0.75f;

	glm::vec3 GetSurfacePoint(float x, float y);

	float GetMagnitude(glm::vec3 v);
	float GetModulus(float base, float divisor);

	void updateCameraVectors();
	void updateArcVectors();
};