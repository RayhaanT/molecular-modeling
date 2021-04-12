#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "VSEPR.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "data.h"
#include "render.h"

extern std::vector<std::vector<glm::vec3>> configurations;
uint32_t BondedElement::maxUID = 0;

int bondingElectrons;

using namespace std;
map<string, Element> elements;
extern vector<BondedElement> VSEPRModel;
std::vector<glm::vec3> tetrahedron;
std::map<std::string, int> numberTerms;

/**
 * Find an element by its abbreviated symbol
 * E.g. He = helium, Br = bromine
 * 
 * @param symbol the element's symbol
 * @return the appropriate element object if found, an empty one otherwise
 */
Element searchElements(string symbol) {
	int size  = elements.size();
	Element e = elements[symbol];
	if(size == elements.size()) {
		return e;
	}
	elements.erase(symbol);
	return Element(-1);
}

/**
 * Find the elements in a chemical formula
 * 
 * @param formulaFull the chemical formula as a string
 * @return a list of the contained elements
 */
vector<Element> readFormula(string formulaFull) {
	string formula;
	int charge = 0;
	char sign = ' ';
	bool space = false;
	for(int i = 0; i < formulaFull.length(); i++) {
		if(formulaFull[i] == ' ') {
			space = true;
		}
		else {
			if (!space) {
				formula += formulaFull[i];
			}
			if (space) {
				if (isdigit(formulaFull[i]) && formulaFull[i] != '-' && formulaFull[i] != '+') {
					charge = formulaFull[i] - '0';
				}
				else if(formulaFull[i] != ' ') {
					sign = formulaFull[i];
				}
			}
		} 
	}
	if(sign == '-') {
		charge = -charge;
	}

	if (formula.length() < 1)
		return vector<Element>();

	vector<Element> comp;
	string symbol = "";
	symbol += formula[0];

	for (int i = 1; i < formula.length(); i++) {
		if (isdigit(formula[i])) {
			Element e = searchElements(symbol);
			if (e.atomicNumber != -1) {
				for (int x = 0; x < formula[i] - '0'; x++) {
					comp.push_back(e);
				}
			}
			else {
				return vector<Element>();
			}
			symbol = "";
		}
		else if (islower(formula[i])) {
			symbol += formula[i];
		}
		else {
			Element e = searchElements(symbol);
			if (e.atomicNumber != -1) {
				comp.push_back(e);
			}
			else {
				return vector<Element>();
			}
			symbol = ""; symbol += formula[i];
		}
	}
	Element e = searchElements(symbol);
	if (e.atomicNumber != -1) {
		comp.push_back(e);
	}
	if(charge != 0) {
		comp.push_back(Element(-1, -charge, -1, ""));
	}

	if(comp.size() > 0) {
		return comp;
	}
	else {
		return vector<Element>();
	}
}

/**
 * Get the formal charge on a bonded atom
 * 
 * @param b the atom
 * @return the formal charge
 */
int getFormalCharge(BondedElement b) {
	return b.base.valenceNumber - (b.bondedElectrons/2) - b.loneElectrons;
}

/**
 * Check if an atom is stable, or how far it is from stability
 * 
 * @param b the atom to check
 * @return the number of electrons away from stability
 *         >0 means missing electrons, 0 = stable
 */
int checkStability(BondedElement b) {
	if (b.base.periodNumber == 1) {
		return 2 - b.bondedElectrons - b.loneElectrons;
	}
	if (b.base.exception == true) {
		return 0;
	}
	return 8 - b.bondedElectrons - b.loneElectrons;
}


/**
 * Create double/triple bonds until if necessary
 * 
 * @param structure the chemical structure
 * @return the updated structure with higher order bonds
 */
vector<BondedElement> rebond(vector<BondedElement> structure) {
	for (int i = 1; i < structure.size(); i++)
	{
		if (structure[i].loneElectrons < 1 || structure[0].loneElectrons < 1)
		{
			continue;
		}
		structure[i].loneElectrons-=2;
		structure[i].bondedElectrons+=2;
		structure[0].loneElectrons-=2;
		structure[0].bondedElectrons+=2;
		bondingElectrons+=2;
		if (bondingElectrons == 0)
		{
			break;
		}
	}
	return structure;
}

/**
 * Predict the lewis structure of a compound given
 * a list of atoms that compose it
 * 
 * @param formula the list of atoms
 * @return the final compound structure
 */
vector<BondedElement> constructLewisStructure(vector<Element> formula) {
	int eTotal = 0;
	for(int i = 0; i < formula.size(); i++) {
		eTotal += formula[i].valenceNumber;
	}
	bondingElectrons = eTotal;

	vector<BondedElement> lewisStructure;
	lewisStructure.push_back(BondedElement(0, 0, formula[0]));
	for (int i = 1; i < formula.size(); i++) {
		if(formula[i].name != "") {
			lewisStructure[0].bondedElectrons+=2;
			lewisStructure.push_back(BondedElement(0, 2, formula[i]));
			bondingElectrons-=2;
		}
	}

	for (int i = 1; i < lewisStructure.size(); i++) {
		int missingElectrons = checkStability(lewisStructure[i]);
		if (missingElectrons > 0) {
			for (int x = 0; x < missingElectrons / 2; x++) {
				lewisStructure[i].loneElectrons+=2;
				bondingElectrons-=2;
			}
		}
	}
	int missingElectrons = checkStability(lewisStructure[0]);
	if (missingElectrons > 0) {
		for (int x = 0; x < missingElectrons / 2; x++) {
			lewisStructure[0].loneElectrons+=2;
			bondingElectrons-=2;
		}
	}
	if (bondingElectrons < 0) {
		lewisStructure = rebond(lewisStructure);
		if(bondingElectrons < 0) {
			lewisStructure = rebond(lewisStructure);
		}
	}
	if(bondingElectrons > 0 && lewisStructure[0].base.periodNumber >= 3) {
		lewisStructure[0].loneElectrons+=bondingElectrons;
		bondingElectrons = 0;
	}

	if ((checkStability(lewisStructure[0]) != 0 && lewisStructure[0].base.periodNumber < 3) || bondingElectrons != 0) {
		return vector<BondedElement>();
	}
	return lewisStructure;
}

/**
 * Find the total formal charge on a compound
 * 
 * @param structure the compound's structure
 * @return the total formal charge
 */
int getTotalFormalCharge(vector<BondedElement> structure) {
	int totalCharge = 0;
	for (int i = 0; i < structure.size(); i++) {
		totalCharge += abs(getFormalCharge(structure[i]));
	}
	return totalCharge;
}

/**
 * Test different variations of bonding to minimize
 * the total formal charge on the structure
 * 
 * @param structure the original structure
 * @return the optimized structure
 */
vector<BondedElement> optimizeFormalCharge(vector<BondedElement> structure) {
	vector<BondedElement> returnVector = structure;
	int totalCharge = getTotalFormalCharge(structure);
	for(int x = 0; x < 1; x++) {
		for(int i = structure.size() - 1; i > 1; i--) {
			if(structure[0].base.periodNumber >= 3 && structure[i].loneElectrons > 0) {
				structure[0].bondedElectrons+=2;
				structure[i].bondedElectrons+=2;
				structure[i].loneElectrons-=2;
				int newTotal = getTotalFormalCharge(structure);
				if(newTotal < totalCharge) {
					totalCharge = newTotal;
					returnVector = structure;
				}
			}
		}
	}

	return returnVector;
}

/**
 * Check if a string contains a substring
 * 
 * @param main the base string
 * @param check the substring
 * @return true if the substring is found, false otherwise
 */
bool checkStringComponent(string main, string check) {
	size_t pos = main.find(check);
	return pos != std::string::npos;
}

/**
 * Check if a string contains any numbers
 * 
 * @param main the base string
 * @return true if theres a number, false otherwise
 */
bool checkForDigits(string main) {
	for(auto i : main) {
		if(isdigit(i)) {
			return true;
		}
	}
	return false;
}

/**
 * Find the last instance of a substring
 * 
 * @param main the base string
 * @param check the substring
 * @return the starting index of the last appearance of the substring
 */
int findLastComponent(string main, string check) {
	if(check.length() > main.length()) {
		return -1;
	}
	int compIndex = -1;
	int index = 0;
	for(int i = 0; i < main.length(); i++) {
		if(main[i] == check[index]) {
			index++;
		}
		else {
			index = 0;
		}
		if(index == check.length()) {
			compIndex = i - check.length() + 1;
			index = 0;
		}
	}
	return compIndex;
}

/**
 * Find the length of a substituent
 * 
 * @param name the substituent name
 * @return the length according to the prefix
 */
int findNumberTerm(string name) {
	vector<string> keys;
	int diIndex;
	for(auto it = numberTerms.begin(); it != numberTerms.end(); it++) {
		keys.push_back(it->first);
	}

	int lastTerm = 0;
	int highestIndex = -1;
	for(int i = 0; i < keys.size(); i++) {
		int newIndex = findLastComponent(name, keys[i]);
		if(newIndex > highestIndex) {
			lastTerm = numberTerms[keys[i]];
		}
	}

	return lastTerm;
}

/**
 * Check if an atom has a valid number of electrons
 * 
 * @param e the atom
 * @return true if its valid, false otherwise
 */
bool checkBondedElementValidity(BondedElement e) {
	if(e.bondedElectrons < 0 || e.loneElectrons < 0) {return false;}
	if(e.base.periodNumber == 1 && e.bondedElectrons + e.loneElectrons > 2) {return false;}
	if(e.base.periodNumber == 2 && e.bondedElectrons + e.loneElectrons > 8) {return false;}
	return true;
}

/**
 * Bond two atoms together and check if the bond is valid
 * 
 * @param a the first atom
 * @param b the second atom
 * @return whether the two atoms still have a valid # of electrons
 */
bool bond(BondedElement &a, BondedElement &b) {
	a.neighbours.push_back(b.getUID());
	a.bondedElectrons+=2;
	a.loneElectrons--;
	b.neighbours.push_back(a.getUID());
	b.bondedElectrons+=2;
	b.loneElectrons--;

	return checkBondedElementValidity(a) && checkBondedElementValidity(b);
}

/**
 * Bond two atoms and throw an exception if it fails
 * 
 * @param a the first element
 * @param b the second element
 */
void bondSafe(BondedElement &a, BondedElement &b) {
	if(!bond(a, b)) {
		vector<string> errors;
		string errorMessage = "Bond error: ";
		if(a.loneElectrons < 0 || checkStability(a) < 0) {
			errorMessage += (a.base.name + " overbonded");
		}
		else if(b.loneElectrons < 0 || checkStability(b) < 0) {
			errorMessage += (b.base.name + " overbonded");
		}
		throw errorMessage.c_str();
	}
}

/**
 * Find how many times a key value occurs in a list
 * 
 * @param v the list
 * @param key the key value
 * @return the number of occurrences
 */
int findInstances(vector<uint32_t> v, uint32_t key) {
	int count = 0;
	for(uint32_t b : v) {
		if(b == key) {
			count++;
		}
	}
	return count;
}

/**
 * Shift all the atoms to center their positions on screen
 * This function's method involves centering the structure between
 * its most extreme points
 * 
 * @param structure the compound's structure
 * @return the newly centered structure
 */
vector<BondedElement> centerPositions(vector<BondedElement> structure) {
	glm::vec3 lowestExtreme = glm::vec3(0);
	glm::vec3 greatestExtreme = glm::vec3(0);
	glm::vec3 lowestExtreme_v = glm::vec3(0);
	glm::vec3 greatestExtreme_v = glm::vec3(0);

	for(BondedElement e : structure) {
		// Lowest extreme regular positions
		if (e.position.x < lowestExtreme.x) { lowestExtreme.x = e.position.x; }
		if (e.position.y < lowestExtreme.y) { lowestExtreme.y = e.position.y; }
		if (e.position.z < lowestExtreme.z) { lowestExtreme.z = e.position.z; }

		// Greatest extreme regular positions
		if (e.position.x > greatestExtreme.x) { greatestExtreme.x = e.position.x; }
		if (e.position.y > greatestExtreme.y) { greatestExtreme.y = e.position.y; }
		if (e.position.z > greatestExtreme.z) { greatestExtreme.z = e.position.z; }

		// Lowest extreme van der waals positions
		if (e.vanDerWaalsPosition.x < lowestExtreme_v.x) { lowestExtreme_v.x = e.vanDerWaalsPosition.x; }
		if (e.vanDerWaalsPosition.y < lowestExtreme_v.y) { lowestExtreme_v.y = e.vanDerWaalsPosition.y; }
		if (e.vanDerWaalsPosition.z < lowestExtreme_v.z) { lowestExtreme_v.z = e.vanDerWaalsPosition.z; }

		// Highest extreme van der waals positions
		if (e.vanDerWaalsPosition.x > greatestExtreme_v.x) { greatestExtreme_v.x = e.vanDerWaalsPosition.x; }
		if (e.vanDerWaalsPosition.y > greatestExtreme_v.y) { greatestExtreme_v.y = e.vanDerWaalsPosition.y; }
		if (e.vanDerWaalsPosition.z > greatestExtreme_v.z) { greatestExtreme_v.z = e.vanDerWaalsPosition.z; }
	}

	glm::vec3 offset = (lowestExtreme + greatestExtreme) * 0.5f;
	glm::vec3 offset_v = (lowestExtreme_v + greatestExtreme_v) * 0.5f;

	for(int i = 0; i < structure.size(); i++) {
		structure[i].position -= offset;
		structure[i].vanDerWaalsPosition -= offset_v;
	}

	return structure;
}

/**
 * Shift all the atoms to center their positions on screen
 * This function's method involves shifting the structure
 * by the average of all the atoms' positions
 * 
 * @param structure the compound's structure
 * @return the newly centered structure
 */
vector<BondedElement> averageCenterPositions(vector<BondedElement> structure) {
	glm::vec3 totalOffset;
	glm::vec3 totalOffset_v;
	for(BondedElement e : structure) {
		totalOffset+=e.position;
		totalOffset_v+=e.vanDerWaalsPosition;
	}
	float divisor = 1.0f / (float)(structure.size());
	glm::vec3 offset = totalOffset * divisor;
	glm::vec3 offset_v = totalOffset_v * divisor;
	for(int i = 0; i < structure.size(); i++) {
		structure[i].position -= offset;
		structure[i].vanDerWaalsPosition -= offset_v;
	}
	return structure;
}

/**
 * Predict the positions of each atom in a substituent
 * of a compound that has already been bonded
 * 
 * @param structure the structure of the substituent
 * @param cyclo whether the substituent is a cyclo group
 * @return the substituent with predicted positions
 */
Substituent positionAtoms(Substituent structure, bool cyclo) {
	if(structure.components.size() < 1 || (cyclo && structure.components.size() < 3)) {
		return Substituent();
	}

	if(cyclo) {
		vector<glm::vec3> positions;
		float angleInterval = (2*PI)/structure.components.size();
		for(int i = 0; i < structure.components.size(); i++) {
			float angle = angleInterval * i;
			positions.push_back(glm::vec3(cos(angle), 0.0f, sin(angle)));
		}
		float adjMagnitude = glm::length(positions[0]-positions[1]);
		float multiplier = 1/adjMagnitude;
		int adjBondOrder = findInstances(structure.components[0].neighbours, structure.components[0].neighbours[0]);
		float multiplier_v = getSphereDistance(structure.components[0], structure.components[1], adjBondOrder)/adjMagnitude;
		for(int i = 0; i < structure.components.size(); i++) {
			structure.components[i].position = positions[i]*multiplier * getStickDistance();
			structure.components[i].vanDerWaalsPosition = positions[i]*multiplier_v;
			glm::mat4 rotation = glm::rotate(glm::mat4(), (angleInterval*i)-(PI/2), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 secondaryAxis = glm::cross(positions[i], glm::vec3(0.0f, 1.0f, 0.0f));
			structure.components[i].rotation = glm::rotate(rotation, PI/2, secondaryAxis);
		}
		return structure;
	}
	
	structure.components[0].position = glm::vec3(0.0f);
	structure.components[0].vanDerWaalsPosition = glm::vec3(0.0f);
	structure.components[0].rotation = glm::toMat4(glm::angleAxis(PI, glm::vec3(1.0f, 0.0f, 0.0f)));
	for(int i = 1; i < structure.components.size(); i++) {
		glm::vec3 offset = glm::vec3(tetrahedron[0].x, structure.components[i].id % 2 == 1 ? tetrahedron[0].y : -tetrahedron[0].y, 0);
		//Regular position
		structure.components[i].position = structure.components[i-1].position;
		structure.components[i].position += offset*getStickDistance();

		//VanDerWaals position
		structure.components[i].vanDerWaalsPosition = structure.components[i-1].vanDerWaalsPosition;
		int bondOrder = findInstances(structure.components[i].neighbours, structure.components[i].neighbours[0]);
		BondedElement neighbour = findNeighbour(structure.components[i].neighbours[0], structure.components);
		structure.components[i].vanDerWaalsPosition += offset * getSphereDistance(structure.components[i], neighbour, bondOrder);

		if(i == structure.components.size()-1) {
			structure.components[i].rotation *= glm::toMat4(glm::angleAxis(PI, glm::vec3(0.0f, 1.0f, 0.0f)));
		}
		if (structure.components[i].id % 2 == 1) {
			structure.components[i].rotation *= glm::toMat4(glm::angleAxis(PI, glm::vec3(1.0f, 0.0f, 0.0f)));
		}
	}

	return structure;
}

/**
 * Generate the transformation matrices required to
 * render cylinders for ball-and-stick models
 * 
 * @param structure the compound's structure
 * @return the structure updated with cylinder models
 */
vector<BondedElement> generateCylinders(vector<BondedElement> structure) {
	vector<BondedElement> newStruc;
	for(BondedElement b : structure) {
		glm::vec3 start = b.position;
		for(uint32_t n : b.neighbours) {
			BondedElement updatedNeighbour = findNeighbour(n, structure);
			glm::vec3 end = updatedNeighbour.position;

			//Rotation
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 dir = glm::normalize(end - start);
			float vDot = glm::dot(up, dir);
			float mag = 1.0f;
			float angle = acos(vDot / mag);
			glm::vec3 axis = glm::cross(up, dir);
			glm::mat4 rotation = glm::toMat4(glm::angleAxis(angle, axis));

			//Cylinder one
			glm::mat4 model = glm::mat4();
			model = glm::translate(model, start);
			model = glm::rotate(model, angle, axis);
			if (updatedNeighbour.base.name == "hydrogen" || b.base.name == "hydrogen") {
				model = glm::scale(model, glm::vec3(1.0f, 0.7f, 1.0f));
			}
			b.cylinderModels.push_back(model);
		}
		newStruc.push_back(b);
	}
	return newStruc;
}

/**
 * Construct a group of substituents of an organic compound
 * given its name and position on the base chain
 * All substituents on a given base atom will be calculated here
 * 
 * @param name the name of substituent
 * @param place the position on the base chain
 * @return a list of the substituents
 */
vector<Substituent> interpretSubstituent(string name, string place) {
	std::vector<int> attachPoints;
	string num = "";

	for(int i = 0; i < place.length(); i++) {
		if(place[i]==',') {
			attachPoints.push_back(stoi(num));
			num = "";
		}
		else {
			num+=place[i];
		}
	}
	attachPoints.push_back(stoi(num));

	int carbonNum = findNumberTerm(name);

	Substituent newSub;
	Element carbon = elements["C"];
	for(int i = 0; i < carbonNum; i++) {
		BondedElement newCarbon = BondedElement(4, 0, carbon);
		newCarbon.id = i+1;
		if(i > 0) {
			bondSafe(newCarbon, newSub.components[i-1]);
		}
		newSub.components.push_back(newCarbon);
	}

	if(checkStringComponent(name, "cyclo")) {
		bondSafe(newSub.components[0], newSub.components[newSub.components.size()-1]);
		newSub = positionAtoms(newSub, true);
	}
	else {
		newSub = positionAtoms(newSub, false);
	}

	string suffix = name.substr(name.length()-4, 3);
	if(suffix == "ane" || suffix == "ene" || suffix == "yne") {
		newSub.connectionPoint = -1;
		return {newSub};
	}
	else {
		vector<Substituent> allSubs;
		newSub.connectionPoint = attachPoints[0];
		allSubs.push_back(newSub);
		for(int i = 1; i < attachPoints.size(); i++) {
			newSub.connectionPoint = attachPoints[i];
			allSubs.push_back(newSub.duplicate());
		}
		return allSubs;
	}
}

/**
 * Construct all the constituents in an organic compound
 * 
 * @param in the name of the compound
 * @return a list of substituents
 */
vector<Substituent> findSubstituents(string in) {
	transform(in.begin(), in.end(), in.begin(), ::tolower); //Make string lowercase
	
	std::vector<string> splitIn;
	string current = "";
	for(int i = 0; i < in.length(); i++) {
		if(i < in.length()-2) { //Check for groups without dash separation
			if(in[i] == 'y' && in[i+1] == 'l' && in[i+2] != '-') {
				current += 'y';
				current += 'l';
				splitIn.push_back(current);
				current = "";
				i += 2;
			}
		}
		if(in[i]=='-') {
			splitIn.push_back(current);
			current = "";
		}
		else {
			current+=in[i];
		}
	}
	splitIn.push_back(current);

	vector<Substituent> newSubGroup;
	vector<Substituent> returnVec;
	for(int i = splitIn.size()-2; i > 0; i--) {
		if(checkForDigits(splitIn[i])) {
			continue;
		}
		newSubGroup = interpretSubstituent(splitIn[i], splitIn[i-1]);
		returnVec.insert(returnVec.end(), newSubGroup.begin(), newSubGroup.end());
	}
	newSubGroup = interpretSubstituent(splitIn.back(), "-1");
	returnVec.insert(returnVec.end(), newSubGroup.begin(), newSubGroup.end());
	return returnVec;
}

/**
 * Add hydrogens to a substituent to satisfy all carbons
 * 
 * @param structure the substituent to fill
 * @return the filled hydrogen
 */
Substituent fillInHydrogens(Substituent structure) {
	Element rawHydrogen = elements["H"];
	int numberOfCarbons = structure.components.size();
	for(int c = 0; c < numberOfCarbons; c++) {
		BondedElement carbon = structure.components[c];
		for(int i = carbon.neighbours.size(); i < carbon.numberOfBonds; i++) {
			BondedElement hydrogen = BondedElement(1, 0, rawHydrogen);
			//Regular position
			hydrogen.position = carbon.position;
			glm::vec3 offset = configurations[carbon.numberOfBonds-1][i];
			offset = glm::vec3(glm::vec4(offset, 0.0f) * carbon.rotation);
			hydrogen.position += offset*getStickDistance() * 0.7f;
			if(c == 0 && structure.connectionPoint > 0) {
				hydrogen.position.x = -hydrogen.position.x;
			}

			//VanDerWaals Position
			hydrogen.vanDerWaalsPosition = carbon.vanDerWaalsPosition;
			hydrogen.vanDerWaalsPosition += offset*getSphereDistance(hydrogen, carbon, 1);
			if(c == 0 && structure.connectionPoint > 0) {
				hydrogen.vanDerWaalsPosition.x = -hydrogen.vanDerWaalsPosition.x;
			}

			bondSafe(hydrogen, structure.components[c]);
			structure.components.push_back(hydrogen);
		}
	}
	return structure;
}

/**
 * Rotate all atoms in a substituent to align
 * with the atom in the base structure
 * 
 * @param structure the substituent to rotate
 * @param dir the direction to rotate towards
 * @param parent the base atom
 * @return the rotated substituent 
 */
Substituent rotateSubstituent(Substituent structure, glm::vec3 dir, BondedElement parent) {
	if(structure.components.size() < 1) {
		return Substituent();
	}

	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 simpleDir = glm::normalize(glm::vec3(dir.x, 0.0f, dir.z));
	float vDot = glm::dot(right, simpleDir);
	float mag = 1.0f;
	float angle = acos(vDot/mag);
	glm::vec3 axis = glm::cross(simpleDir, right);
	glm::mat4 rotation = glm::mat4();
	rotation = glm::rotate(rotation, PI, glm::vec3(right));
	rotation = glm::rotate(rotation, angle, axis);

	float vanDerWaalsOffset = getSphereDistance(structure.components[0], parent, findInstances(structure.components[0].neighbours, parent.getUID()));

	for(int i = 0; i < structure.components.size(); i++) {
		structure.components[i].position = glm::vec3(glm::vec4(structure.components[i].position, 0.0f) * rotation);
		structure.components[i].position += parent.position;
		structure.components[i].position += dir * getStickDistance();

		structure.components[i].vanDerWaalsPosition = glm::vec3(glm::vec4(structure.components[i].vanDerWaalsPosition, 0.0f) * rotation);
		structure.components[i].vanDerWaalsPosition += parent.vanDerWaalsPosition;findInstances(structure.components[i].neighbours, parent.getUID());
		structure.components[i].vanDerWaalsPosition += dir * vanDerWaalsOffset;
	}

	return structure;
}

/**
 * Predict the structure of an organic compound
 * 
 * @param in the name of the compound
 * @return a list of atoms representing the structure
 */
vector<BondedElement> interpretOrganic(string in) {
	vector<Substituent> subs = findSubstituents(in);
	Substituent central = subs.back();
	subs.pop_back();
	if(subs.size() > 0) {
		for (int i = 0; i < subs.size(); i++)
		{
			bondSafe(subs[i].components[0], central.components[subs[i].connectionPoint - 1]);
			subs[i] = fillInHydrogens(subs[i]);
		}
	}
	central = fillInHydrogens(central);
	for(int i = 0; i < subs.size(); i++) {
		vector<uint32_t>::iterator pos = find(central.components[subs[i].connectionPoint - 1].neighbours.begin(), central.components[subs[i].connectionPoint - 1].neighbours.end(), subs[i].components[0].getUID());
		int index = distance(central.components[subs[i].connectionPoint - 1].neighbours.begin(), pos);
		if (index < configurations[central.components[subs[i].connectionPoint - 1].numberOfBonds-1].size()) {
			glm::vec3 dir = glm::vec3(glm::vec4(configurations[central.components[subs[i].connectionPoint - 1].numberOfBonds-1][index], 0.0f)*central.components[subs[i].connectionPoint - 1].rotation);
			subs[i] = rotateSubstituent(subs[i], dir, central.components[subs[i].connectionPoint - 1]);
		}
	}

	vector<BondedElement> returnVec;
	int reserveNum = 0;
	for(int i = 0; i < subs.size(); i++) {
		reserveNum+=subs[i].components.size();
	}
	returnVec.reserve(reserveNum + central.components.size());
	for(int i = 0; i < subs.size(); i++) {
		returnVec.insert(returnVec.end(), subs[i].components.begin(), subs[i].components.end());
	}
	returnVec.insert(returnVec.end(), central.components.begin(), central.components.end());
	// returnVec = centerPositions(returnVec);
	returnVec = averageCenterPositions(returnVec);
	returnVec = generateCylinders(returnVec);

	return returnVec;
} 

/**
 * The main function for predicting structures
 * Runs in a seperate thread from main
 * 
 * @return nothing because of while(1)
 */
vector<BondedElement> VSEPRMain() {
	setUpMap();

	try {
		parseCSV("periodicTableData.csv");
	}
	catch(const std::exception& err) {
		std::cerr << err.what() << '\n';
		exit(1);
	}
	cout << "Periodic table data loaded" << endl << endl;

	tetrahedron = {glm::vec3(1, 0, -1 / sqrt(2)), glm::vec3(-1, 0, -1 / sqrt(2)), glm::vec3(0, 1, 1 / sqrt(2)), glm::vec3(0, -1, 1 / sqrt(2))};
	//glm::quat shift = glm::angleAxis((float)((PI/2)-atan(sqrt(2))), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat shift = glm::angleAxis((float)(PI/2), glm::vec3(1.0f, 0.0f, 0.0f));
	for(int i = 0; i < tetrahedron.size(); i++) {
		glm::vec4 temp = glm::vec4(tetrahedron[i], 1.0f);
		tetrahedron[i] = glm::normalize(glm::vec3(temp * shift));
	} 

	string inFormula;
	configurations = {
		std::vector<glm::vec3>{glm::vec3(1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(COS_30, -SIN_30, 0), glm::vec3(-COS_30, -SIN_30, 0), glm::vec3(0, 1, 0)},
		tetrahedron,
		std::vector<glm::vec3>{glm::vec3(0, 0, -1), glm::vec3(-COS_30, 0, SIN_30), glm::vec3(0, -1, 0), glm::vec3(COS_30, 0, SIN_30), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(SIN_45, 0, -SIN_45), glm::vec3(SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, -SIN_45), glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)},
	};

	while (1) {
		getline(cin, inFormula);
		vector<BondedElement> structure;
		if (checkStringComponent(inFormula, "ane") || checkStringComponent(inFormula, "ene") || checkStringComponent(inFormula, "yne")) {
			organic = true;
			try	{
				structure = interpretOrganic(inFormula);
			}
			catch(const char* errMsg) {
				cout << errMsg << endl;
				continue;
			}
			int longestName = 0;
			for(int i = 0; i < structure.size(); i++) {
				if(structure[i].base.name.length() > longestName) {
					longestName = structure[i].base.name.length();
				}
			}
			printf(string(longestName + 2, ' ').c_str());
			printf("| AN | VN | BP | LP | FC |\n");
			for (int i = 0; i < structure.size(); i++) {
				Element e = structure[i].base;
				int formalCharge = getFormalCharge(structure[i]);
				int rName = longestName + 2 - structure[i].base.name.length();
				int rAN = e.atomicNumber < 10 ? 3 : e.atomicNumber < 100 ? 2 : 1;
				char sign = formalCharge < 0 ? '-' : '+';
				if(formalCharge != 0) {
					printf("%s%s|%d%s|%d   |%d   |%d   |%c%d  |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedElectrons/2, structure[i].loneElectrons/2, sign, abs(formalCharge));
				}
				else {
					printf("%s%s|%d%s|%d   |%d   |%d   |%d   |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedElectrons/2, structure[i].loneElectrons/2, abs(formalCharge));
				}
			}
			VSEPRModel = structure;
			continue;
		}

		vector<Element> comp;
		if(inFormula == "H2O")
			comp = readFormula("OH2");
		else
			comp = readFormula(inFormula);
		if(comp.size() < 1) {
			continue;
		}
		structure = constructLewisStructure(comp);

		if (structure.size() < 1) {
			cout << "Lewis structure not possible" << endl;
			continue;
		}

		for(int i = 0; i < structure.size(); i++) {
			if(getFormalCharge(structure[i]) != 0) {
				structure = optimizeFormalCharge(structure);
				break;
			} 
		}
		VSEPRModel = structure;
		int longestName = 0;
		for(int i = 0; i < structure.size(); i++) {
			if(structure[i].base.name.length() > longestName) {
				longestName = structure[i].base.name.length();
			}
		}
		printf(string(longestName + 2, ' ').c_str());
		printf("| AN | VN | BP | LP | FC |\n");
		for (int i = 0; i < structure.size(); i++) {
			Element e = structure[i].base;
			int formalCharge = getFormalCharge(structure[i]);
			int rName = longestName + 2 - structure[i].base.name.length();
			int rAN = e.atomicNumber < 10 ? 3 : e.atomicNumber < 100 ? 2 : 1;
			char sign = formalCharge < 0 ? '-' : '+';
			if(formalCharge != 0) {
				printf("%s%s|%d%s|%d   |%d   |%d   |%c%d  |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedElectrons/2, structure[i].loneElectrons/2, sign, abs(formalCharge));
			}
			else {
				printf("%s%s|%d%s|%d   |%d   |%d   |%d   |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedElectrons/2, structure[i].loneElectrons/2, abs(formalCharge));
			}
		}
	}

	return vector<BondedElement>();
}