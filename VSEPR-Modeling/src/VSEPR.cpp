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

std::vector<std::vector<glm::vec3>> configurations;
uint32_t BondedElement::maxUID = 0;

using namespace std;
map<string, Element> elements;
vector<BondedElement> VSEPRModel;
std::vector<glm::vec3> tetrahedron;
std::map<std::string, int> numberTerms;

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
 * Create double/triple bonds until if necessary
 * 
 * @param structure the chemical structure
 * @return the updated structure with higher order bonds
 */
vector<BondedElement> rebond(vector<BondedElement> structure, int eTotal) {
	for (int i = 1; i < structure.size(); i++)
	{
		if (structure[i].loneElectrons < 1)
			continue;
		structure[0].loneElectrons--;
		structure[i].loneElectrons--;
		bondSafe(structure[i], structure[0]);
		if (countElectrons(structure) == eTotal || structure[0].loneElectrons < 1)
			break;
	}
	return structure;
}

/**
 * Predicts the positions of atoms in a simple
 * covalent compound using periodic table data
 * 
 * @param structure the compound's structure
 * @return the structure with updated position information
 */
vector<BondedElement> positionSimpleAtoms(vector<BondedElement> structure) {
	int configIndex = configIndex = structure.size() - 2;
	if (structure.size() > 2) {
		configIndex += structure[0].loneElectrons / 2;
	}

	structure[0].position = glm::vec3(0);
	structure[0].vanDerWaalsPosition = glm::vec3(0);
	BondedElement center = structure[0];

	for(int i = 1; i < structure.size(); i++) {
		glm::vec3 dir = glm::normalize(configurations[configIndex][i - 1]);

		int bondOrder = findInstances(structure[i].neighbours, structure[i].neighbours[0]);
		structure[i].vanDerWaalsPosition = dir * getSphereDistance(structure[i], center, bondOrder);
		structure[i].position = dir * getStickDistance();
		if(structure[i].base.name == "hydrogen") {
			structure[i].position *= 0.7;
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

	// Bond central to peripheral once
	vector<BondedElement> lewisStructure;
	lewisStructure.push_back(BondedElement(formula[0].valenceNumber, 0, formula[0]));
	for (int i = 1; i < formula.size(); i++) {
		if(formula[i].name != "") {
			lewisStructure.push_back(BondedElement(formula[i].valenceNumber, 0, formula[i]));
			bondSafe(lewisStructure[0], lewisStructure.back()); // One bond between central and outer is necessary
		}
	}

	// Fill in lone electrons until all atoms are stable
	for (int i = 0; i < lewisStructure.size(); i++) {
		int missingElectrons = checkStability(lewisStructure[i]);
		if (missingElectrons >= 0) {
			lewisStructure[i].loneElectrons += missingElectrons;
		}
		else if(formula[0].periodNumber < 3) {
			string errorMessage = "Bond error: ";
			errorMessage += lewisStructure[i].base.name + " cannot be bonded";
			throw errorMessage.c_str();
		}
	}

	if (countElectrons(lewisStructure) > eTotal) {
		lewisStructure = rebond(lewisStructure, eTotal);
		if(countElectrons(lewisStructure) > eTotal) {
			lewisStructure = rebond(lewisStructure, eTotal);
		}
	}
	int excess = countElectrons(lewisStructure) - eTotal;
	if(excess > 0 && lewisStructure[0].base.periodNumber >= 3) {
		lewisStructure[0].loneElectrons += excess;
	}

	if ((checkStability(lewisStructure[0]) != 0 && lewisStructure[0].base.periodNumber < 3) || countElectrons(lewisStructure) != eTotal) {
		return vector<BondedElement>();
	}

	for(int i = 0; i < lewisStructure.size(); i++) {
		if(getFormalCharge(lewisStructure[i]) != 0) {
			lewisStructure = optimizeFormalCharge(lewisStructure);
			break;
		} 
	}

	lewisStructure = positionSimpleAtoms(lewisStructure);
	lewisStructure = generateCylinders(lewisStructure);
	return lewisStructure;
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
	for(int x = 0; x < 2; x++) {
		for(int i = structure.size() - 1; i > 1; i--) {
			if(structure[0].base.periodNumber >= 3 && structure[i].loneElectrons > 0) {
				if(!shiftBond(structure[0], structure[i])) {
					structure = returnVector;
					continue;
				}

				int newTotal = getTotalFormalCharge(structure);
				if(newTotal < totalCharge) {
					totalCharge = newTotal;
					returnVector = structure;
				}
				else {
					structure = returnVector;
				}
			}
		}
	}

	return returnVector;
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

	// Position cyclo group in a circle
	if(cyclo) {
		// Generate positions along a unit circle
		vector<glm::vec3> positions;
		float angleInterval = (2*PI)/structure.components.size();
		for(int i = 0; i < structure.components.size(); i++) {
			float angle = angleInterval * i;
			positions.push_back(glm::vec3(cos(angle), 0.0f, sin(angle)));
		}

		// Adjust the circle's radius for number of atoms
		float adjMagnitude = glm::length(positions[0]-positions[1]);
		float multiplier = 1/adjMagnitude;
		int adjBondOrder = findInstances(structure.components[0].neighbours, structure.components[0].neighbours[0]);
		float multiplier_v = getSphereDistance(structure.components[0], structure.components[1], adjBondOrder)/adjMagnitude;

		// Scale and apply positions
		for(int i = 0; i < structure.components.size(); i++) {
			structure.components[i].position = positions[i]*multiplier * getStickDistance();
			structure.components[i].vanDerWaalsPosition = positions[i]*multiplier_v;
			glm::mat4 rotation = glm::rotate(glm::mat4(), (angleInterval*i)-(PI/2), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 secondaryAxis = glm::cross(positions[i], glm::vec3(0.0f, 1.0f, 0.0f));
			structure.components[i].rotation = glm::rotate(rotation, PI/2, secondaryAxis);
		}
		return structure;
	}
	
	// Position atoms in a straight hydrocarbon chain
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
		parseCSV(DATA_TABLE_PATH);
	}
	catch(const std::exception& err) {
		std::cerr << err.what() << '\n';
		exit(1);
	}
	cout << "Periodic table data loaded" << endl << endl;

	tetrahedron = {glm::vec3(1, 0, -1 / sqrt(2)), glm::vec3(-1, 0, -1 / sqrt(2)), glm::vec3(0, 1, 1 / sqrt(2)), glm::vec3(0, -1, 1 / sqrt(2))};
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
					printf("%s%s|%d%s|%d   |%d   |%d   |%c%d  |\n",
						structure[i].base.name.c_str(),
						string(rName, ' ').c_str(),
						e.atomicNumber,
						string(rAN, ' ').c_str(),
						e.valenceNumber,
						structure[i].bondedElectrons/2,
						structure[i].loneElectrons/2,
						sign,
						abs(formalCharge));
				}
				else {
					printf("%s%s|%d%s|%d   |%d   |%d   |%d   |\n",
						structure[i].base.name.c_str(),
						string(rName, ' ').c_str(),
						e.atomicNumber,
						string(rAN, ' ').c_str(),
						e.valenceNumber,
						structure[i].bondedElectrons/2,
						structure[i].loneElectrons/2,
						abs(formalCharge));
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

		try {
			structure = constructLewisStructure(comp);
		}
		catch(const char *errMsg) {
			cout << errMsg << endl;
			continue;
		}

		if (structure.size() < 1) {
			cout << "Lewis structure not possible" << endl;
			continue;
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