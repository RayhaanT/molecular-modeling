#ifndef CAMERA_H
#define CAMERA_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "VSEPR.h"

#include <iostream>
#include <vector>

enum Camera_Movement {
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
bool clicked;

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
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	glm::mat4 GetArcMatrix() {
		return glm::lookAt(Position, origin, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(GLFWwindow *window, float deltaTime, bool restrictY)
	{
		float camspeed = 7 * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, true);
		}
		if(glfwGetKey(window, GLFW_KEY_E)) {
			RotationSpeed += 1 * smoothingConstant * camspeed;
		}
		if (glfwGetKey(window, GLFW_KEY_Q)) {
			RotationSpeed -= 1 * smoothingConstant * camspeed;
		}
		//Move without flying: along x and z only
		if (restrictY)
		{
			if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
				Position += camspeed*glm::vec3(Front.x, 0.0f, Front.z);
			if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
				Position -= camspeed*glm::vec3(Front.x, 0.0f, Front.z);
			if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
				Position -= camspeed*glm::normalize(glm::cross(glm::vec3(Front.x, 0.0f, Front.z), Up));
			if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
				Position += camspeed*glm::normalize(glm::cross(glm::vec3(Front.x, 0.0f, Front.z), Up));
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
				Position += camspeed*Front;
			if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
				Position -= camspeed*Front;
			if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT))
				Position -= camspeed*glm::normalize(glm::cross(Front, Up));
			if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT))
				Position += camspeed*glm::normalize(glm::cross(Front, Up));
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateCameraVectors();
	}

	void SetDivisor(double xPos, double yPos) {
		xPos -= W / 2;
		yPos -= H / 2;
		divisor = 2*sqrt((xPos * xPos) + (yPos * yPos));
	}

	void ProcessArcBall(float xoffset, float yoffset) {
		lon += (xoffset / divisor) * C_PI;
		lat += (yoffset / divisor) * C_PI;
		// std::cout << lon * 180 / C_PI << " " << lat * 180 / C_PI << std::endl;
		// std::cout << Up.x << " " << Up.y << " " << Up.z << std::endl;
		glm::vec3 spherePos;
		spherePos.y = -sin(lat) * CAMERA_DISTANCE;
		float rCrossSection = cos(lat) * CAMERA_DISTANCE;
		spherePos.x = rCrossSection * sin(lon);
		spherePos.z = -rCrossSection * cos(lon);
		Position = spherePos;
		updateArcVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	float lon = 0.0f;
	float lat = 0.0f;
	float divisor = 1.0f;

	float GetModulus(float base, float divisor) {
		float mod;
		// Handle negatives
		if (base < 0)
			mod = -base;
		else
			mod = base;
		if (divisor < 0)
			divisor = -divisor;

		// Finding mod by repeated subtraction

		while (mod >= divisor)
			mod = mod - divisor;

		// Sign of result typically depends
		// on sign of a.
		if (base < 0)
			return -mod;

		return mod;
	}

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void updateArcVectors() {
		Right = glm::normalize(glm::cross(glm::normalize(Position-origin), WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		float simpleAngle = abs(GetModulus(lat*180/C_PI, 360));
		//Check which half of the sphere the angle is in
		if(simpleAngle < 90 || simpleAngle > 270) {
			//Calculate up with global up of (0, 1, 0)
			Up = glm::normalize(glm::cross(Right, glm::normalize(Position - origin)));
		}
		else {
			//Calculate up with global up of (0, -1, 0) -- Reversing order of cross results in this
			Up = glm::normalize(glm::cross(glm::normalize(Position - origin), Right));
		}
	}
};
#endif