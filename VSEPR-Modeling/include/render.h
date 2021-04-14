#pragma once

#include "VSEPR.h"
#include "data.h"
#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OpenGLHeaders/shader.h"

// Centrally define file paths
#define POINT_LIGHT_VERT_PATH "shaders/VeShColors.vs"
#define POINT_LIGHT_FRAG_PATH "shaders/FrShLight.fs"
#define LIT_MODEL_VERT_PATH "shaders/VeShMap.vs"
#define LIT_MODEL_POINT_FRAG_PATH "shaders/FrShMap.fs"
#define LIT_MODEL_DIR_FRAG_PATH "shaders/FrShDirectional.fs"
#define FLAT_TEXTURE_PATH "RedTexture.png"

// Centrally define uniform names to avoid input errors
// Incorrect uniform names don't raise any exceptions or compile errors
// See shaders/*.fs or shaders/*.vs for uniform names
#define MODEL "model"
#define VIEW "view"
#define PROJECTION "projection"
#define VIEW_POS "viewPos"
#define COLOR "color"

// Objects/Lists of objects
#define LIGHT_OBJ(property) "light." property
#define MATERIAL_OBJ(property) "material." property
#define POINT_LIGHT_LIST(index, property) ("pointLights[" + std::to_string(index) + "]." property).c_str()

// Object property names
#define DIRECTION "direction"
#define AMBIENT "ambient"
#define DIFFUSE "diffuse"
#define SPECULAR "specular"
#define SHININESS "shininess"
#define CONSTANT "constant"
#define LINEAR "linear"
#define QUADRATIC "quadratic"
#define POSITION "position"

// Data containers for rendering
extern unsigned int sphereVAO;
extern unsigned int sphereVBO;
extern unsigned int cylinderVAO;
extern unsigned int cylinderVBO;
extern unsigned int fastSphereVAO;
extern unsigned int fastSphereVBO;
extern unsigned int fastCylinderVAO;
extern unsigned int fastCylinderVBO;

// Rendering state and constants
extern int representation; // 0 = electron, 1 = sphere, 2 = ball and stick
extern const float atomDistance;
extern const float electronSpeed;
extern const float lineColor[];
extern const float stickSetWidth;

// Cylinder rendering
glm::mat4 getCylinderOffset(std::pair<int, int> bondOrder, glm::mat4 rotationModel, glm::vec3 direction, glm::mat4 cylinderModel);
glm::mat4 getCylinderRotation(int configIndex, int modelIndex, std::pair<int, int> bondOrder, glm::mat4 rotationModel);

// Get atom distances
float getSphereDistance(std::vector<BondedElement> model, int index, int order);
float getSphereDistance(BondedElement a, BondedElement b, int order);
float getStickDistance();

// Master render functions
void renderOrganic(std::vector<BondedElement> structure, Shader shader, glm::mat4 rotationModel, int rep);
void renderSimpleCompound(std::vector<BondedElement> structure, glm::mat4 rotationModel, Shader shader, int representation);