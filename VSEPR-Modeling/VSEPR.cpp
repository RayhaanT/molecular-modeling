#include <iostream>
#include <vector>
#include <algorithm>
#include "VSEPR.h"
#include <string>

extern std::vector<std::vector<glm::vec3>> configurations;

Element perTable[] = {
Element(1, 1, 1, "Hydrogen"),
Element(2, 2, 1, "Helium"),
Element(3, 1, 2, "Lithium"),
Element(4, 2, 2, "Beryllium", true),
Element(5, 3, 2, "Boron", true),
Element(6, 4, 2, "Carbon"),
Element(7, 5, 2, "Nitrogen"),
Element(8, 6, 2, "Oxygen"),
Element(9, 7, 2, "Fluorine"),
Element(10, 8, 2, "Neon"),
Element(11, 1, 3, "Sodium"),
Element(12, 2, 3, "Magnesium"),
Element(13, 3, 3, "Aluminum"),
Element(14, 4, 3, "Silicon"),
Element(15, 5, 3, "Phosphorous"),
Element(16, 6, 3, "Sulfur"),
Element(17, 7, 3, "Chlorine"),
Element(18, 8, 3, "Argon"),
Element(19, 1, 4, "Potassium"),
Element(20, 2, 4, "Calcium")
};

std::string elements[] = { "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca" };
int bondingPairs;

using namespace std;

extern vector<BondedElement> VSEPRModel;

int searchElements(string symbol) {
	for (int i = 0; i < sizeof(elements) / sizeof(*elements); i++) {
		if (elements[i] == symbol) {
			return i;
		}
	}
	return -1;
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
				else {
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
			int index = searchElements(symbol);
			if (index != -1) {
				for (int x = 0; x < formula[i] - '0'; x++) {
					comp.push_back(perTable[index]);
				}
			}
			symbol = "";
		}
		else if (islower(formula[i])) {
			symbol += formula[i];
		}
		else {
			int index = searchElements(symbol);
			if (index != -1) {
				comp.push_back(perTable[index]);
			}
			symbol = ""; symbol += formula[i];
		}
	}
	int index = searchElements(symbol);
	if (index != -1) {
		comp.push_back(perTable[index]);
	}
	if(charge != 0) {
		comp.push_back(Element(-1, -charge, -1, ""));
	}

	return comp;
}

int getFormalCharge(BondedElement b) {
	return b.base.valenceNumber - b.bondedPairs - (b.lonePairs*2);
}

int checkStability(BondedElement b) {
	if (b.base.name == "Hydrogen" || b.base.name == "Helium") {
		return 2 - ((b.bondedPairs * 2) + (b.lonePairs * 2));
	}
	if (b.base.exception == true) {
		return 0;
	}
	return 8 - ((b.bondedPairs * 2) + (b.lonePairs * 2));
}

vector<BondedElement> rebond(vector<BondedElement> structure) {
	for (int i = 1; i < structure.size(); i++)
	{
		if (structure[i].lonePairs < 1 || structure[0].lonePairs < 1)
		{
			continue;
		}
		structure[i].lonePairs--;
		structure[i].bondedPairs++;
		structure[0].lonePairs--;
		structure[0].bondedPairs++;
		bondingPairs++;
		if (bondingPairs == 0)
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
	bondingPairs = eTotal / 2;

	vector<BondedElement> lewisStructure;
	lewisStructure.push_back(BondedElement(0, 0, formula[0]));
	for (int i = 1; i < formula.size(); i++) {
		if(formula[i].name != "") {
			lewisStructure[0].bondedPairs++;
			lewisStructure.push_back(BondedElement(0, 1, formula[i]));
			bondingPairs--;
		}
	}

	for (int i = 1; i < lewisStructure.size(); i++) {
		int missingElectrons = checkStability(lewisStructure[i]);
		if (missingElectrons > 0) {
			for (int x = 0; x < missingElectrons / 2; x++) {
				lewisStructure[i].lonePairs++;
				bondingPairs--;
			}
		}
	}
	int missingElectrons = checkStability(lewisStructure[0]);
	if (missingElectrons > 0) {
		for (int x = 0; x < missingElectrons / 2; x++) {
			lewisStructure[0].lonePairs++;
			bondingPairs--;
		}
	}
	if (bondingPairs < 0) {
		lewisStructure = rebond(lewisStructure);
		if(bondingPairs < 0) {
			lewisStructure = rebond(lewisStructure);
		}
	}

	if ((checkStability(lewisStructure[0]) != 0 && lewisStructure[0].base.periodNumber < 3) || bondingPairs != 0) {
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
			if(structure[0].base.periodNumber >= 3 && structure[i].lonePairs > 0) {
				structure[0].bondedPairs++;
				structure[i].bondedPairs++;
				structure[i].lonePairs--;
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

vector<BondedElement> VSEPRMain() {
	string inFormula;
	configurations = {
		std::vector<glm::vec3>{glm::vec3(1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(COS_30, -SIN_30, 0), glm::vec3(-COS_30, -SIN_30, 0), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(-COS_30, -SIN_30, SIN_30), glm::vec3(COS_30, -SIN_30, SIN_30), glm::vec3(0, -SIN_30, -COS_30), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1), glm::vec3(-COS_30, 0, SIN_30), glm::vec3(COS_30, 0, SIN_30)},
		std::vector<glm::vec3>{glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec3(SIN_45, 0, -SIN_45), glm::vec3(SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, -SIN_45)},
	};

	while (1) {
		getline(cin, inFormula);
		vector<Element> comp = readFormula(inFormula);
		if(inFormula == "H2O") {
			comp = readFormula("OH2");
		}
		for (int i = 0; i < comp.size(); i++)
		{
			cout << comp[i].name << " " << comp[i].valenceNumber << endl;
		}
		vector<BondedElement> structure = constructLewisStructure(comp);

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
		for (int i = 0; i < structure.size(); i++) {
			cout << structure[i].base.name << " " << structure[i].bondedPairs << " " << structure[i].lonePairs << " " << getFormalCharge(structure[i]) << endl;
		}
	}

	return vector<BondedElement>();
}