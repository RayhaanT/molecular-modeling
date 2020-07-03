#pragma once

#include "VSEPR.h"
#include "data.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OpenGLHeaders/ShaderClass.h"

extern unsigned int sphereVAO;
extern unsigned int sphereVBO;
extern unsigned int cylinderVAO;
extern unsigned int cylinderVBO;
extern unsigned int fastSphereVAO;
extern unsigned int fastSphereVBO;
extern unsigned int fastCylinderVAO;
extern unsigned int fastCylinderVBO;

// Cylinder rendering
glm::mat4 getCylinderOffset(std::pair<int, int> bondOrder, glm::mat4 rotationModel, glm::vec3 direction, glm::mat4 cylinderModel);
glm::mat4 getCylinderRotation(int configIndex, int modelIndex, std::pair<int, int> bondOrder, glm::mat4 rotationModel);

// Get atom distances
float getSphereDistance(std::vector<BondedElement> model, int index, int order);
float getSphereDistance(BondedElement a, BondedElement b, int order);
float getStickDistance();

// Master render functions
void RenderOrganic(std::vector<BondedElement> structure, Shader shader, glm::mat4 rotationModel, int rep);