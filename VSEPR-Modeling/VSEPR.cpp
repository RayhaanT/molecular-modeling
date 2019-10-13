#include <iostream>
#include <vector>
#include <algorithm>
#include "VSEPR.h"
#include <string>

extern Element perTable[] = {
Element(1, 1, 1, "Hydrogen"),
Element(2, 2, 1, "Helium"),
Element(3, 1, 2, "Lithium"),
Element(4, 2, 2, "Beryllium"),
Element(5, 3, 2, "Boron"),
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

std::string elements[] = { "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca"};

using namespace std;

int searchElements(string symbol) {
	for (int i = 0; i < sizeof(elements)/sizeof(*elements); i++) {
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

vector<Element> VSEPRMain() {
	string inFormula;

	while (1) {
		cin >> inFormula;
		vector<Element> comp = readFormula(inFormula);
		for each (Element e in comp) 
		{
			cout << e.name << endl;
		}
	}

	return vector<Element>();
}