#ifdef _WIN32
#include <windows.h> // Needed to avoid compile errors
#endif

// If we're compiling for mac for some reason
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// Headers
#include "OpenGLHeaders/ShaderClass.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include "render.h"
#include "Sphere.h"
#include "cylinder.h"

// Geometry classes
const Sphere sphere(1.0f, 36, 18, true); //Smooth
const Sphere sphere_fast(1.0f, 18, 9, true);
const Cylinder cylinder(0.125f, getStickDistance() / 2, 64);
const Cylinder cylinder_fast(0.125f, getStickDistance(), 32);
const Cylinder hydrogenCylinder(0.125f, getStickDistance() / 4, 64);

/**
 * Check for a unique id in a list
 * 
 * @param id the id to search for
 * @param list the list to parse
 * @return whether a match was found
 */
bool containsUID(uint32_t id, std::vector<uint32_t> list) {
    for(uint32_t i : list) {
        if(i == id) {
            return true;
        }
    }
    return false;
}

/**
 * Find an element by id in a group of neigbours
 * 
 * @param key the id of the neighbour element to find
 * @param group the group of neighbours to parse
 * @return the found element or a blank one if no match 
 */
BondedElement findNeighbour(uint32_t key, std::vector<BondedElement> group) {
    for(BondedElement b : group) {
        if(key == b.getUID()) {
            return b;
        }
    }
    return BondedElement();
}

/**
 * Render a cylinder to the screen for ball-and-stick models
 * Obsoleted by fastRenderCylinder()
 * Cylinder transformations are now calculated before-hand
 * instead of in real-time to help performance
 * 
 * @param start the cylinder's start position
 * @param end the end position
 * @param startColor the color of the first atom
 * @param endColor the color of the second atom
 * @param shader the shader program for cylinders
 * @param fast whether to use the low-poly models
 */
void RenderCylinder(glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, Shader shader, bool fast) {
    shader.use();

    //Rotation
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 dir = glm::normalize(end-start);
    float vDot = glm::dot(up, dir);
    float mag = 1.0f;
    float angle = acos(vDot / mag);
    glm::vec3 axis = glm::cross(up, dir);
    glm::mat4 rotation = glm::toMat4(glm::angleAxis(angle, axis));

    //Cylinder one
    if(fast) {
        glBindVertexArray(fastCylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fastCylinderVBO);
    } else {
        glBindVertexArray(cylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    }
    glm::mat4 model = glm::mat4();
    model = glm::translate(model, start);
    model = glm::rotate(model, angle, axis);
    shader.setMat4("model", model);
    shader.setVec3("color", startColor);
    if(fast) {
        cylinder_fast.draw();
        return;
    }
    cylinder.draw();

    //Cylinder two
    model = glm::mat4();
    model = glm::translate(model, start);
    model = glm::rotate(model, angle, axis);
    model = glm::translate(model, glm::vec3(0.0f, getStickDistance() / 2, 0.0f));
    shader.setMat4("model", model);
    shader.setVec3("color", endColor);
    cylinder.draw();
}

/**
 * Render a cylinder to the screen for ball-and-stick models
 * Replaces RenderCylinder() by using the pre-calculated
 * transformation matrices to improve performance
 * 
 * @param color the color of the atom attached
 * @param transform the pre-calculated transformation matrix for this cylinder
 * @param rotation the rotation of the entire system
 * @param shader the shader program for cylinders
 * @param fast whether to use low-poly models
 */
void fastRenderCylinder(glm::vec3 color, glm::mat4 transform, glm::mat4 rotation, Shader shader, bool fast) {
    if(fast) {
        glBindVertexArray(fastCylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fastCylinderVBO);
    } else {
        glBindVertexArray(cylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    }

    glm::mat4 model;
    model *= rotation;
    model *= transform;
    shader.setMat4("model", model);
    shader.setVec3("color", color);
    if (fast) {
        cylinder_fast.draw();
        return;
    }
    cylinder.draw();
}

/**
 * Renders an organic compound (so far only saturated hydrocarbons)
 * 
 * @param structure the compound's structure as a vector of compounds
 * @param shader the shader program for atoms
 * @param rotationModel the camera rotation model
 * @param rep which representation to render
 */
void RenderOrganic(std::vector<BondedElement> structure, Shader shader, glm::mat4 rotationModel, int rep) {
    glm::quat rotationQuat = glm::quat_cast(rotationModel);
    shader.use();
    bool fast = false;
    if(fast) {
        glBindVertexArray(fastSphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fastSphereVBO);
    } else {
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    }

    if(rep == 0) {
        return;
    }
    if(rep == 1) {
        for(BondedElement b : structure) {
            glm::mat4 model;
            glm::vec3 pos = b.vanDerWaalsPosition;
            pos = glm::vec3(glm::vec4(pos, 0.0f));
            model *= rotationModel;
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(b.base.vanDerWaalsRadius));
            shader.setMat4("model", model);
            shader.setVec3("color", b.base.color);
            if(fast) {
                sphere_fast.draw();
            } else {
                sphere.draw();
            }
        }
    }
    if(rep == 2) {
        std::vector<uint32_t> closedSet;
        for(BondedElement b : structure) {
            for (glm::mat4 m : b.cylinderModels) {
                fastRenderCylinder(b.base.color, m, rotationModel, shader, fast);
            }
            if(fast) {
                glBindVertexArray(fastSphereVAO);
                glBindBuffer(GL_ARRAY_BUFFER, fastSphereVBO);
            } else {
                glBindVertexArray(sphereVAO);
                glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            }
            glm::mat4 model;
            model *= rotationModel;
            if(b.base.name == "hydrogen") {
                model = glm::translate(model, b.position);
                model = glm::scale(model, glm::vec3(0.75f));
            }
            else {
                model = glm::translate(model, b.position);
                model = glm::scale(model, glm::vec3(1.0f)); 
            }
            shader.setMat4("model", model);
            shader.setVec3("color", b.base.color);
            if(fast) {
                sphere_fast.draw();
            } else {
                sphere.draw();
            }
        }
    }
}