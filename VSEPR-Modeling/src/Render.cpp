#ifdef _WIN32
#include <windows.h> // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "OpenGLHeaders/Shader.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include "render.h"
#include "Sphere.h"
#include "cylinder.h"

const Sphere sphere(1.0f, 36, 18, true);

void RenderOrganic(std::vector<BondedElement> structure, unsigned int shader, glm::mat4 rotationModel, int rep) {
    glUseProgram(shader);

    if(rep == 0) {
        return;
    }
    if(rep == 1) {
        for(BondedElement b : structure) {
            glm::mat4 model;
            glm::vec3 pos = b.position;
            pos = glm::vec3(glm::vec4(pos, 0.0f)*rotationModel);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(b.base.vanDerWaalsRadius));
            setMat4(shader, "model", model);
            setVec3(shader, "color", b.base.color);
            sphere.draw();
        }
    }
    if(rep == 2) {
        
    }
}