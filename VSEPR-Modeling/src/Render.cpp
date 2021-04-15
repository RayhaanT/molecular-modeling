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
#include "OpenGLHeaders/shader.h"
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
void renderCylinder(glm::vec3 start, glm::vec3 end, glm::vec3 startColor, glm::vec3 endColor, Shader shader, bool fast) {
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
 * Renders cylinders to the screen for ball-and-stick models
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
 * Renders simple covalent compounds.
 * Compounds with single central atoms including
 * polyatomic ions are currently supported
 * 
 * @param structure the compound's structure
 * @param rotationModel the camera rotation model
 * @param shader the shader for atoms
 * @param rep which representation to render
 */
void renderSimpleCompound(std::vector<BondedElement> structure, glm::mat4 rotationModel, Shader shader, int rep) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	int configIndex;

	// Determine VSEPR config
	if(structure.size() > 2) {
		configIndex = structure.size() - 2 + (structure[0].loneElectrons/2);
	}
	else {
		configIndex = structure.size() - 2;
	}

    if(rep == 0) {
        for (int i = 0; i < structure.size() + (structure[0].loneElectrons/2); i++) {
            glm::mat4 model;
            // Check if drawing an atom or lone pair
            if(i < structure.size()) {
                BondedElement b = structure[i];
                glm::vec3 pos = b.position;
                if (b.base.name == "hydrogen") {
                    pos /= 0.7;
                }
                model *= rotationModel;
                model = glm::translate(model, pos);
                model = glm::scale(model, glm::vec3(b.base.atomicRadius > 0 ? structure[i].base.atomicRadius : 0.8f));
                shader.setVec3("color", b.base.color);
                shader.setMat4("model", model);
                sphere.draw();
            }
            else {
                glm::vec3 lonePairPos = configurations[configIndex][i - 1] * getStickDistance();
                model *= rotationModel;
                model = glm::translate(model, lonePairPos);
                model = glm::scale(model, glm::vec3(structure[0].base.atomicRadius > 0 ? structure[0].base.atomicRadius : 0.8f));
                shader.setVec3("color", structure[0].base.color);
                shader.setMat4("model", model);
                sphere.drawLines(lineColor);
            }
        }
    }
    if(rep == 1) {
        for(BondedElement b : structure) {
            glm::mat4 model;
            glm::vec3 pos = b.vanDerWaalsPosition;
            model *= rotationModel;
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(b.base.vanDerWaalsRadius));
            shader.setMat4("model", model);
            shader.setVec3("color", b.base.color);
            sphere.draw();
        }
    }
    if(rep == 2) {
        for(BondedElement b : structure) {
            for (glm::mat4 m : b.cylinderModels) {
                fastRenderCylinder(b.base.color, m, rotationModel, shader, false);
            }
            glBindVertexArray(sphereVAO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
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
            sphere.draw();
        }
    }
}

/**
 * Renders an organic compound (so far only saturated hydrocarbons)
 * 
 * @param structure the compound's structure as a vector of compounds
 * @param shader the shader program for atoms
 * @param rotationModel the camera rotation model
 * @param rep which representation to render
 */
void renderOrganic(std::vector<BondedElement> structure, Shader shader, glm::mat4 rotationModel, int rep) {
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

/**
 * Render all the electron point lights in the electron orbit model
 * 
 * @param program the electron shader program
 * @param atomProgram a reference to the atom shader program
 *                    needed to update point light positions
 * @param model the compound structure
 * @param rotationModel the camera rotation
 */
void renderElectrons(Shader program, Shader &atomProgram, std::vector<BondedElement> model, glm::mat4 rotationModel) {
	program.use();
	//glBindVertexArray(lightVAO);

	glm::mat4 lightModel;

	program.setMat4(MODEL, lightModel);	
	//glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));

	glm::vec3 lightVec3;
	glm::vec3 light2Vec3;
	if (VSEPRModel.size() > 0)
	{
		int numberOfBonds = VSEPRModel[0].bondedElectrons/2;
		int configIndex;
		if (VSEPRModel.size() > 2)
		{
			configIndex = VSEPRModel.size() - 2 + (VSEPRModel[0].loneElectrons/2);
		}
		else
		{
			configIndex = 0;
		}
		int lightIndex = 0;
		for (int i = 1; i < VSEPRModel.size(); i++)
		{
			for (int x = 0; x < VSEPRModel[i].bondedElectrons/2; x++)
			{
				lightModel = glm::mat4();
				glm::vec3 newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedElectrons/2, false);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				program.use();
				lightModel *= rotationModel;
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				program.setMat4(MODEL, lightModel);
				sphere.drawLines(lineColor);
				lightIndex++;

				//Draw complimentary
				lightModel = glm::mat4();
				newLightPos = calculateOrbitPosition(VSEPRModel[0], VSEPRModel[i], configIndex, i, x, VSEPRModel[i].bondedElectrons/2, true);
				setPointLightPosition(lightIndex, atomProgram, newLightPos);
				program.use();
				lightModel *= rotationModel;
				lightModel = glm::translate(lightModel, newLightPos);
				lightModel = glm::scale(lightModel, glm::vec3(0.1f));
				program.setMat4(MODEL, lightModel);
				sphere.drawLines(lineColor);
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
		program.use();
		lightModel = glm::translate(lightModel, lightVec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		program.setMat4(MODEL, lightModel);
		sphere.draw();
		lightModel = glm::mat4();
		lightModel = glm::translate(lightModel, light2Vec3);
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));
		program.setMat4(MODEL, lightModel);
		sphere.draw();
	}
}