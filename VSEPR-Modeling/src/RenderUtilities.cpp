#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "VSEPR.h"
#include "OpenGLHeaders/shader.h"
#include "render.h"
#include "Sphere.h"
#include "cylinder.h"
#include "glm/gtc/matrix_transform.hpp"

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
 * Get a quaternion that would rotate the initial vector
 * to the target vector's direction
 * 
 * @param start the intial vector
 * @param dest the target vector
 * @return a quaternion describing the rotation
*/
glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
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