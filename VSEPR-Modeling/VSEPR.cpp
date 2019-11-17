#include <iostream>
#include <vector>
#include <algorithm>
#include "VSEPR.h"
#include <string>
#include "data.h"

extern std::vector<std::vector<glm::vec3>> configurations;

int bondingPairs;

using namespace std;
map<string, Element> elements;
extern vector<BondedElement> VSEPRModel;

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
	return b.base.valenceNumber - b.bondedPairs - (b.lonePairs*2);
}

int checkStability(BondedElement b) {
	if (b.base.periodNumber == 1) {
		return 2 - ((b.bondedPairs * 2) + (b.lonePairs * 2));
	}
	if (b.base.exception == true) {
		return 0;
	}
	return 8 - ((b.bondedPairs * 2) + (b.lonePairs * 2));
}


//Used to create double and triple bonds
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
	if(bondingPairs > 0 && lewisStructure[0].base.periodNumber >= 3) {
		lewisStructure[0].lonePairs+=bondingPairs;
		bondingPairs = 0;
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

float getAtomicRadius(BondedElement b) {
	float periodMod = b.base.periodNumber * PERIOD_CONSTANT;
	float groupMod = b.base.valenceNumber * GROUP_CONSTANT;
	if (b.base.atomicNumber == 1) {
		groupMod = 7 * GROUP_CONSTANT;
	}
	return (periodMod/groupMod)*UPSCALING;
}

vector<BondedElement> VSEPRMain() {
	parseCSV("periodicTableData.csv");

	string inFormula;
	configurations = {
		std::vector<glm::vec3>{glm::vec3(1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0)},
		std::vector<glm::vec3>{glm::vec3(COS_30, -SIN_30, 0), glm::vec3(-COS_30, -SIN_30, 0), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(-COS_30, -SIN_30, SIN_30), glm::vec3(COS_30, -SIN_30, SIN_30), glm::vec3(0, -SIN_30, -COS_30), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(0, 0, -1), glm::vec3(-COS_30, 0, SIN_30), glm::vec3(0, -1, 0), glm::vec3(COS_30, 0, SIN_30), glm::vec3(0, 1, 0)},
		std::vector<glm::vec3>{glm::vec3(SIN_45, 0, -SIN_45), glm::vec3(SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, SIN_45), glm::vec3(-SIN_45, 0, -SIN_45), glm::vec3(0, 1, 0), glm::vec3(0, -1, 0)},
	};

	while (1) {
		getline(cin, inFormula);
		vector<Element> comp = readFormula(inFormula);
		if(comp.size() < 1) {
			continue;
		}
		if(inFormula == "H2O") {
			comp = readFormula("OH2");
		}
		for (int i = 0; i < comp.size(); i++)
		{
			//cout << comp[i].name << " " << comp[i].valenceNumber << endl;
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
				printf("%s%s|%d%s|%d   |%d   |%d   |%c%d  |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedPairs, structure[i].lonePairs, sign, abs(formalCharge));
			}
			else {
				printf("%s%s|%d%s|%d   |%d   |%d   |%d   |\n", structure[i].base.name.c_str(), string(rName, ' ').c_str(), e.atomicNumber, string(rAN, ' ').c_str(), e.valenceNumber, structure[i].bondedPairs, structure[i].lonePairs, abs(formalCharge));
			}
			//cout << structure[i].base.name << " BP: " << structure[i].bondedPairs << " LP: " << structure[i].lonePairs << " FC: " << getFormalCharge(structure[i]) << endl;
		}
	}

	return vector<BondedElement>();
}