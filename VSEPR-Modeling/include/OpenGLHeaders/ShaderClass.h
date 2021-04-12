#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
class Shader
{
public:
	// Null constructor for globals
	Shader() {}

	/**
	 * Construct a new shader program class
	 * 
	 * @param vertexPath the path to the vertex shader
	 * @param fragmentPath the path to the fragment shader
	 */
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
		// 1.Retrieve vertex/frag source code from file path
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		//Make ifstreams able to through exceptions
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			//Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			//Read buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			//Close file handlers
			vShaderFile.close();
			fShaderFile.close();
			//Convert to string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		//2. Compile
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		//Vertex
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		//Compile-time error check
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		//Fragment
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		//Compile-time error check
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		//Shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		//Linking error check
		glGetShaderiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		//Delete shaders
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	/**
	 * Set a vec3 uniform on the shader program
	 * 
	 * @param name the name of the uniform
	 * @param value the value to pass
	 */
	void setVec3(const GLchar* name, glm::vec3 value) {
		unsigned int loc = glGetUniformLocation(ID, name);
		glUniform3f(loc, value.x, value.y, value.z);
	}

	/**
	 * Set a mat4 uniform on the shader program
	 * 
	 * @param name the name of the uniform
	 * @param value the value to pass
	 */
	void setMat4(const GLchar* name, glm::mat4 value) {
		unsigned int loc = glGetUniformLocation(ID, name);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
	}

	/**
	 * Set a float uniform on the shader program
	 * 
	 * @param name the name of the uniform
	 * @param value the value to pass
	 */
	void setFloat(const GLchar* name, float value) {
		unsigned int loc = glGetUniformLocation(ID, name);
		glUniform1f(loc, value);
	}

	/**
	 * Set an int uniform on the shader program
	 * 
	 * @param name the name of the uniform
	 * @param value the value to pass
	 */
	void setInt(const GLchar* name, int value) {
		unsigned int loc = glGetUniformLocation(ID, name);
		glUniform1i(loc, value);
	}

	/**
	 * Use this shader program
	 */
	void use() {
		glUseProgram(ID);
	}

private:
	unsigned int ID;
};