#include "OpenGLHeaders/Camera.h"

bool clicked;

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}
// Constructor with scalar values
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
{
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

/**
 * Get the view matrix calculated using Euler Angles and the LookAt Matrix
 * 
 * @returns view matrix
*/
glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(Position, Position + Front, Up);
}

/**
 * Get the matrix calculated using the arc ball view method
 * 
 * @return matrix
 */
glm::mat4 Camera::GetArcMatrix() {
	return arcMatrix;
}

/**
 * Get a reverse of the arc ball matrix
 * 
 * @return reversed matrix
 */
glm::mat4 Camera::GetReverseArcMatrix() {
	return reverseArcMatrix;
}

/**
 * Processes input received from any keyboard-like input system
 * Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
 * 
 * @param window the GLFW window where the event occurred
 * @param deltaTime the change in time bewteen frames
 * @param restrictY whether to limit movement in the y axis
 */
void Camera::ProcessKeyboard(GLFWwindow *window, float deltaTime, bool restrictY) {
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
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP))
			Position += camspeed*Front;
		if (glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_DOWN))
			Position -= camspeed*Front;
	}
}

/**
 * Processes input received from a mouse input system
 * Expects the offset value in both the x and y direction
 * 
 * @param xoffset the change in x pos of the mouse
 * @param yoffset the change in y pos of the mouse
 * @param constrainPitch whether to constrain rotation within a 90 degree zone
 */
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
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

/**
 * Set the radius of the arc ball sphere depending
 * on where the user clicked the screen
 * 
 * @param xPos the x coord of the click
 * @param yPos the y coord of the click
 */
void Camera::SetRadius(double xPos, double yPos) {
	// Adjust for origin at top-left corner
	xPos -= W / 2;
	yPos -= H / 2;
	// Convert to screen coords
	xPos /= W; xPos *= -2;
	yPos /= H; yPos *= -2;
	sphereRadius = sqrt((xPos * xPos) + (yPos * yPos));
	lastPos2D = glm::vec2(xPos, yPos);
}
/**
 * Rotate the view based on mouse movement using the arc ball method
 * 
 * @param xPos the x position of the mouse
 * @param yPos the y position of the mouse
 */
void Camera::ProcessArcBall(float xPos, float yPos) {
	// Adjust for origin at top-left corner
	xPos -= W / 2;
	yPos -= H / 2;
	// Convert to screen coords
	xPos /= W; xPos *= -2;
	yPos /= H; yPos *= -2;
	float xoffset = xPos - lastPos2D.x;
	float yoffset = yPos - lastPos2D.y;
	glm::quat xRotation = glm::angleAxis((float)(-xoffset * C_PI)/(sphereRadius*2), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat yRotation = glm::angleAxis((float)(-yoffset * C_PI)/(sphereRadius*2), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 xMatrix = glm::toMat4(xRotation);
	glm::mat4 yMatrix = glm::toMat4(yRotation);
	glm::quat xRotationReverse = glm::angleAxis((float)(xoffset * C_PI) / (sphereRadius * 2), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat yRotationReverse = glm::angleAxis((float)(yoffset * C_PI) / (sphereRadius * 2), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 xMatrixReverse = glm::toMat4(xRotationReverse);
	glm::mat4 yMatrixReverse = glm::toMat4(yRotationReverse);
	yMatrixReverse *= xMatrixReverse;
	yMatrixReverse *= reverseArcMatrix;
	reverseArcMatrix = yMatrixReverse;
	yMatrix *= xMatrix;
	yMatrix *= arcMatrix;
	arcMatrix = yMatrix;
	lastPos2D = glm::vec2(xPos, yPos);
}

/**
 * Process mouse scroll events to zoom in/out
 * Only uses vertical scrolls
 * 
 * @param window the GLFW window where the event covered
 * @param yoffset the change in vertical scroll
 */
void Camera::ProcessMouseScroll(GLFWwindow *window, float yoffset) {
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
	Position += zoomSense * Front * yoffset;
	//ProcessArcBall(lastPos2D.x, lastPos2D.y);
}

/**
 * Finds point on a sphere from a given point on screen (pythagoras method)
 * 
 * @param x the x screen coordinate
 * @param y the y screen coordinate
 * @return the point on the 3D sphere
 */
glm::vec3 Camera::GetSurfacePoint(float x, float y) {
	glm::vec3 newPos;
	float xySquare = x * x + y * y;
	glm::vec2 tempPoint(x, y);
	float tempMagnitude = GetMagnitude(glm::vec3(tempPoint, 0.0f));
	int zone = floor(tempMagnitude / sphereRadius);
	if(zone % 4 == 1 || zone % 4 == 2) {
		float desiredLength = GetModulus(tempMagnitude, sphereRadius) + sphereRadius * (zone % 4 == 1 ? 1 : 2);
		if(tempMagnitude != 0) {
			tempPoint = (desiredLength / tempMagnitude) * tempPoint;
		}
		newPos = glm::vec3(-tempPoint + (2.0f * sphereRadius * glm::normalize(tempPoint)), 0.0f);
		xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
		newPos.z = -sqrt((sphereRadius * sphereRadius) - xySquare);
	} 
	// Code for 3rd quadrant if computed on its own:
	/*else if(zone % 4 == 2) {
		float desiredLength = GetModulus(tempMagnitude, sphereRadius);
		if(tempMagnitude != 0) {
			tempPoint = (desiredLength / tempMagnitude) * tempPoint;
		}
		newPos = -glm::vec3(tempPoint, 0.0f);
		xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
		newPos.z = -sqrt((sphereRadius * sphereRadius) - xySquare);
	}*/ else if(zone % 4 == 3) {
		float desiredLength = GetModulus(tempMagnitude, sphereRadius) + sphereRadius;
		if(tempMagnitude != 0) {
			tempPoint = (desiredLength / tempMagnitude) * tempPoint;
		}
		newPos = glm::vec3(tempPoint - (2.0f*sphereRadius*glm::normalize(tempPoint)), 0.0f);
		xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
		newPos.z = sqrt((sphereRadius * sphereRadius) - xySquare);
	} else {
		float desiredLength = GetModulus(tempMagnitude, sphereRadius);
		if(tempMagnitude != 0) {
			tempPoint = (desiredLength / tempMagnitude) * tempPoint;
		}
		newPos = glm::vec3(tempPoint, 0.0f);
		xySquare = newPos.x * newPos.x + newPos.y * newPos.y;
		newPos.z = sqrt((sphereRadius * sphereRadius) - xySquare);
	}
	return newPos;
}

/**
 * Get the magnitude of a vector
 * 
 * @param v the vector
 * @return the magnitude
 */
float Camera::GetMagnitude(glm::vec3 v) {
	return sqrt((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}

/**
 * Get the modulus of 2 floats
 * 
 * @param base the base number
 * @param divisor the divisor
 * @return base % divisor
 */
float Camera::GetModulus(float base, float divisor) {
	float mod;
	// Handle negatives
	if (base < 0)
		mod = -base;
	else
		mod = base;
	if (divisor < 0)
		divisor = -divisor;
	while (mod >= divisor)
		mod = mod - divisor;
	if (base < 0)
		return -mod;
	return mod;
}

/**
 * Calculates the front vector from the Camera's (updated) Eular Angles
 */
void Camera::updateCameraVectors() {
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

/**
 * Update the direction vectors for the arc ball camera model
 */
void Camera::updateArcVectors() {
	Right = glm::normalize(glm::cross(glm::normalize(Position-origin), WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	//float simpleAngle = abs(GetModulus(lat*180/C_PI, 360));
	float simpleAngle = 0;
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