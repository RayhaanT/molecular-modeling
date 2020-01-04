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

extern std::vector<std::vector<glm::vec3>> configurations;

int bondingElectrons;

using namespace std;
map<string, Element> elements;
extern vector<BondedElement> VSEPRModel;
std::vector<glm::vec3> tetrahedron;

Element searchElements(string symbol) {
	int size  = elements.size();
	Element e = elements[symbol];
	if(size == elements.size()) {
		return e;
	}
	elements.erase(symbol);
	return Element(-1);
}

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

int getFormalCharge(BondedElement b) {
	return b.base.valenceNumber - (b.bondedElectrons/2) - b.loneElectrons;
}

int checkStability(BondedElement b) {
	if (b.base.periodNumber == 1) {
		return 2 - b.bondedElectrons - b.loneElectrons;
	}
	if (b.base.exception == true) {
		return 0;
	}
	return 8 - b.bondedElectrons - b.loneElectrons;
}


//Used to create double and triple bonds
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

int getTotalFormalCharge(vector<BondedElement> structure) {
	int totalCharge = 0;
	for (int i = 0; i < structure.size(); i++) {
		totalCharge += abs(getFormalCharge(structure[i]));
	}
	return totalCharge;
}

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

bool checkStringComponent(string main, string check) {
	size_t pos = main.find(check);
	return pos != std::string::npos;
}

bool checkForDigits(string main) {
	for(auto i : main) {
		if(isdigit(i)) {
			return true;
		}
	}
	return false;
}

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

void Bond(BondedElement &a, BondedElement &b) {
	a.neighbours.push_back(b);
	a.bondedElectrons+=2;
	a.loneElectrons--;
	b.neighbours.push_back(a);
	b.bondedElectrons+=2;
	b.loneElectrons--;
}

Substituent positionAtoms(Substituent structure) {
	if(structure.components.size() < 1) {
		return Substituent();
	}
	
	structure.components[0].position = glm::vec3(0.0f);
	for(int i = 1; i < structure.components.size(); i++) {
		structure.components[i].position = structure.components[i-1].position;
		structure.components[i].position += glm::vec3(tetrahedron[0].x, structure.components[i].id % 2 == 1 ? tetrahedron[0].y : 0, 0);
		if(structure.components[i].id % 2 == 0) {
			structure.components[i].rotation = glm::toMat4(glm::angleAxis(PI, glm::vec3(1.0f, 0.0f, 0.0f)));
		}
	}

	return structure;
}

vector<Substituent> interpretSubstituent(string name, string place) {
	std::vector<int> attachPoints;
	string num = "";

	for(int i = 0; i < place.length(); i++) {
		if(place[i]==',') {
			attachPoints.push_back(stoi(num));
		}
		else {
			num+=place[i];
		}
	}
	attachPoints.push_back(stoi(num));

	int carbonNum = findNumberTerm(name);
	cout << "C:::" << carbonNum << endl;

	Substituent newSub;
	Element carbon = elements["C"];
	for(int i = 0; i < carbonNum; i++) {
		BondedElement newCarbon = BondedElement(4, 0, carbon);
		newCarbon.id = i+1;
		if(i > 0) {
			Bond(newCarbon, newSub.components[i-1]);
		}
		newSub.components.push_back(newCarbon);
	}

	if(checkStringComponent(name, "cyclo")) {
		Bond(newSub.components[0], newSub.components[newSub.components.size()-1]);
	}

	string suffix = name.substr(name.length()-4, 3);
	if(suffix == "ane" || suffix == "ene" || suffix == "yne") {
		newSub.connectionPoint = -1;
		return {newSub};
	}
	else {
		vector<Substituent> allSubs;
		for(int i = 0; i < attachPoints.size(); i++) {
			newSub.connectionPoint = attachPoints[i];
			allSubs.push_back(newSub);
		}
		return allSubs;
	}
}

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
		for(auto s : newSubGroup) {
			for(auto b : s.components) {
				std::cout << b.base.name << " " << b.id << std::endl;
			}
		}
		returnVec.insert(returnVec.end(), newSubGroup.begin(), newSubGroup.end());
	}
	newSubGroup = interpretSubstituent(splitIn.back(), "-1");
	returnVec.insert(returnVec.end(), newSubGroup.begin(), newSubGroup.end());
	return returnVec;
}

Substituent fillInHydrogens(Substituent structure) {
	Element rawHydrogen = elements["H"];
	int numberOfCarbons = structure.components.size();
	for(int c = 0; c < numberOfCarbons; c++) {
		BondedElement carbon = structure.components[c];
		for(int i = carbon.neighbours.size()-1; i < carbon.numberOfBonds; i++) {
			BondedElement hydrogen = BondedElement(1, 0, rawHydrogen);
			hydrogen.position = carbon.position;
			glm::vec3 offset = configurations[carbon.numberOfBonds-1][i+1];
			hydrogen.position += glm::vec3(glm::vec4(offset, 0.0f) * carbon.rotation);
			Bond(hydrogen, structure.components[c]);
			structure.components.push_back(hydrogen);
		}
	}
	return structure;
}

Substituent rotateSubstituent(Substituent structure, glm::vec3 dir) {
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	float vDot = glm::dot(right, dir);
	float nDot = glm::dot(glm::normalize(right), glm::normalize(dir));
	float angle = acos(vDot/nDot);
	glm::vec3 axis = glm::cross(right, dir);
	glm::mat4 rotation = glm::toMat4(glm::angleAxis(angle, axis));

	for(int i = 0; i < structure.components.size(); i++) {
		structure.components[i].position = glm::vec3(glm::vec4(structure.components[i].position, 0.0f) * rotation);
	}

	return structure;
}

vector<BondedElement> interpretOrganic(string in) {
	vector<Substituent> subs = findSubstituents(in);
	Substituent central = subs.back();
	subs.pop_back();
	if(subs.size() > 0) {
		for (int i = 0; i < subs.size(); i++)
		{
			Bond(subs[i].components[0], central.components[subs[i].connectionPoint - 1]);
			subs[i] = fillInHydrogens(subs[i]);
		}
	}
	central = fillInHydrogens(central);
	for(int i = 0; i < subs.size(); i++) {
		vector<BondedElement>::iterator pos = find(central.components[subs[i].connectionPoint - 1].neighbours.begin(), central.components[subs[i].connectionPoint - 1].neighbours.end(), subs[i].components[0]);
		int index = distance(central.components[subs[i].connectionPoint - 1].neighbours.begin(), pos);
		if (index < configurations[central.components[subs[i].connectionPoint - 1].numberOfBonds-1].size()) {
			glm::vec3 dir = configurations[central.components[subs[i].connectionPoint - 1].numberOfBonds-1][index];
			subs[i] = rotateSubstituent(subs[i], dir);
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

	return returnVec;
} 

void setUpMap() {
	numberTerms["meth"] = 1;
	numberTerms["eth"] = 2;
	numberTerms["prop"] = 3;
	numberTerms["but"] = 4;
	numberTerms["pent"] = 5;
	numberTerms["hex"] = 6;
	numberTerms["hept"] = 7;
	numberTerms["oct"] = 8;
	numberTerms["non"] = 9;
	numberTerms["dec"] = 10;
	numberTerms["di"] = 2;
	numberTerms["tri"] = 3;
	numberTerms["tetr"] = 4;
	numberTerms["bis"] = 2;
	numberTerms["tris"] = 3;
	numberTerms["tetrakis"] = 4;
	numberTerms["pentakis"] = 5;
}

vector<BondedElement> VSEPRMain() {
	setUpMap();
	parseCSV("periodicTableData.csv");

	tetrahedron = {glm::vec3(-1, 0, -1/sqrt(2)), glm::vec3(0, 1, 1/sqrt(2)), glm::vec3(0, -1, 1/sqrt(2)), glm::vec3(1, 0, -1/sqrt(2))};
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
			structure = interpretOrganic(inFormula);
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