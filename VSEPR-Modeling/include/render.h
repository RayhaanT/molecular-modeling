#include "VSEPR.h"
#include "data.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Cylinder rendering
glm::mat4 getCylinderOffset(std::pair<int, int> bondOrder, glm::mat4 rotationModel, glm::vec3 direction, glm::mat4 cylinderModel);
glm::mat4 getCylinderRotation(int configIndex, int modelIndex, std::pair<int, int> bondOrder, glm::mat4 rotationModel);

// Get atom distances
float getSphereDistance(std::vector<BondedElement> model, int index, int order);
float getStickDistance(std::vector<BondedElement> model, int index);

// Master render functions
void RenderOrganic(std::vector<BondedElement> structure, unsigned int shader, glm::mat4 rotationModel, int rep);