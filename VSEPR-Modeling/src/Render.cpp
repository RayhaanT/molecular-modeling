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
const Cylinder cylinder(0.125f, getStickDistance() / 2, 64);

bool containsUID(uint32_t id, std::vector<uint32_t> list) {
    for(uint32_t i : list) {
        if(i == id) {
            return true;
        }
    }
    return false;
}

BondedElement findNeighbour(BondedElement key, std::vector<BondedElement> group) {
    for(BondedElement b : group) {
        if(key == b) {
            return b;
        }
    }
    return BondedElement();
}

void RenderCylinder(glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, unsigned int shader) {
    //Rotation
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 dir = glm::normalize(end-start);
    float vDot = glm::dot(up, dir);
    float mag = 1.0f;
    float angle = acos(vDot / mag);
    glm::vec3 axis = glm::cross(dir, up);
    glm::mat4 rotation = glm::toMat4(glm::angleAxis(angle, axis));

    //Cylinder one
    glBindVertexArray(cylinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glm::mat4 model = glm::translate(model, start);
    model *= rotation;
    setMat4(shader, "model", model);
    setVec3(shader, "color", startColor);
    cylinder.draw();

    //Cylinder two
    model = glm::mat4();
    model = glm::translate(model, end);
    model *= rotation;
    model = glm::translate(model, glm::vec3(0.0f, getStickDistance() / 2, 0.0f));
    setMat4(shader, "model", model);
    setVec3(shader, "color", startColor);
    cylinder.draw();
}

void RenderOrganic(std::vector<BondedElement> structure, unsigned int shader, glm::mat4 rotationModel, int rep) {
    glUseProgram(shader);

    if(rep == 0) {
        return;
    }
    if(rep == 1) {
        for(BondedElement b : structure) {
            glm::mat4 model;
            glm::vec3 pos = b.vanDerWaalsPosition;
            pos = glm::vec3(glm::vec4(pos, 0.0f)*rotationModel);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(b.base.vanDerWaalsRadius));
            setMat4(shader, "model", model);
            setVec3(shader, "color", b.base.color);
            sphere.draw();
        }
    }
    if(rep == 2) {
        std::vector<uint32_t> closedSet;
        for(BondedElement b : structure) {
            for(BondedElement n : b.neighbours) {
                if(!containsUID(n.getUID(), closedSet)) {
                    BondedElement updatedNeighbour = findNeighbour(n, structure);
                    glm::vec3 start = glm::vec3(glm::vec4(b.position, 0.0f) * rotationModel);
                    glm::vec3 end = glm::vec3(glm::vec4(updatedNeighbour.position, 0.0f) * rotationModel);
                    RenderCylinder(start, end, b.base.color, updatedNeighbour.base.color, shader);
                }
            }
            glBindVertexArray(sphereVAO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            glm::mat4 model;
            glm::vec3 pos = b.position*getStickDistance();
            pos = glm::vec3(glm::vec4(pos, 0.0f) * rotationModel);
            model = glm::translate(model, pos);
            setMat4(shader, "model", model);
            setVec3(shader, "color", b.base.color);
            sphere.draw();
            closedSet.push_back(b.getUID());
        }
    }
}