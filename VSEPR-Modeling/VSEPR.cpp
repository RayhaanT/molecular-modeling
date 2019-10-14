#include <iostream>
#include <vector>
#include <algorithm>
#include "VSEPR.h"
#include <string>

extern Element perTable[] = {
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

using namespace std;

int searchElements(string symbol) {
	for (int i = 0; i < sizeof(elements) / sizeof(*elements); i++) {
		if (elements[i] == symbol) {
			return i;
		}
	}
	return -1;
}

vector<Element> readFormula(string formula) {
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

	return comp;
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

vector<BondedElement> constructLewisStructure(vector<Element> formula) {
	int eTotal = 0;
	for each (Element e in formula) {
		eTotal += e.valenceNumber;
	}
	int bondingPairs = eTotal / 2;

	vector<BondedElement> lewisStructure;
	lewisStructure.push_back(BondedElement(0, 0, formula[0]));
	for (int i = 1; i < formula.size(); i++) {
		lewisStructure[0].bondedPairs++;
		lewisStructure.push_back(BondedElement(0, 1, formula[i]));
		bondingPairs--;
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
	if (bondingPairs != 0) {
		for (int i = 1; i < lewisStructure.size(); i++) {
			lewisStructure[i].lonePairs--;
			lewisStructure[i].bondedPairs++;
			lewisStructure[0].lonePairs--;
			lewisStructure[0].bondedPairs++;
			bondingPairs++;
			if (bondingPairs == 0) {
				break;
			}
		}
	}

	return lewisStructure;
}

vector<Element> VSEPRMain() {
	string inFormula;

	while (1) {
		cin >> inFormula;
		vector<Element> comp = readFormula(inFormula);
		vector<BondedElement> structure = constructLewisStructure(comp);
		for each (BondedElement e in structure) 
		{
			cout << e.base.name << " " << e.bondedPairs << " " << e.lonePairs << " " << checkStability(e) << endl;
		}
	}

	return vector<Element>();
}