#pragma once

#include "glad/glad.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
class Shader
{
public:
	// Null constructor for globals
	Shader() {}

	// Constructor
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	// Uniform interfaces
	void setVec3(const GLchar* name, glm::vec3 value);
	void setMat4(const GLchar* name, glm::mat4 value);
	void setFloat(const GLchar* name, float value);
	void setInt(const GLchar* name, int value);
	
	void use();

private:
	unsigned int ID;
};