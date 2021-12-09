#pragma once

#ifndef VSEPR_H
#define VSEPR_H

#include <string>
#include <map>
#include <vector>
#include "glm/glm.hpp"

#define SIN_45 0.70710678118654752440084436210485
#define COS_30 0.86602540378443864676372317075294
#define SIN_30 0.5
#define PERIOD_CONSTANT 0.4
#define GROUP_CONSTANT 0.8
#define UPSCALING 4
#define PI 3.14159265358979323846264338f
#define MAX_POINT_LIGHTS 36
#define smoothingConstant 0.2f

extern std::map <std::string, int> numberTerms;
static bool organic;

struct BondedElement;
struct Element;
struct Substituent;
struct BondingOrbital;
// struct FunctionalGroup;
// struct RGroupConnection;

/**
 * Data structure to hold information about elements.
 * These should be constant after startup as the data
 * comes from recorded research and are not subject to change.
 * Element data is loaded in in DataParsing.cpp at startup
 * and kept in an extern map for later access
 */
struct Element {
	int atomicNumber;
	int valenceNumber;
	int periodNumber;
	std::string name;
	float electronegativity;
	float atomicRadius;
	float covalentRadii[3];
	float vanDerWaalsRadius;
	bool exception = false;
	glm::vec3 color;

	Element() {}

	Element(int _atomicNumber) {atomicNumber = _atomicNumber;}

	/**
	 * Construct a new Element object
	 * 
	 * @param _atomicNumber the atomic number
	 * @param _valenceNumber the valence number
	 * @param _periodNumber the period number
	 * @param _name the element's full name
	 */
	Element(int _atomicNumber, int _valenceNumber, int _periodNumber, std::string _name) {
		atomicNumber = _atomicNumber;
		valenceNumber = _valenceNumber;
		periodNumber = _periodNumber;
		name = _name;
	}

	/**
	 * Construct a new Element object
	 * 
	 * @param _atomicNumber the atomic number
	 * @param _valenceNumber the valence number
	 * @param _periodNumber the period number
	 * @param _name the element's full name
	 * @param _exception whether its valence shells are an exception to the octet stability rule
	 */
	Element(int _atomicNumber, int _valenceNumber, int _periodNumber, std::string _name, bool _exception) {
		exception = _exception;
		valenceNumber = _valenceNumber;
		periodNumber = _periodNumber;
		name = _name;
	}
};

/**
 * Data structure used to represent an atom in a compound
 * Includes information about the element, how its bonded
 * to other atoms, position, and rendering data 
 */
struct BondedElement {
public:
	Element base;
	int loneElectrons;
	int bondedElectrons;
	int id = 0;
	std::vector<uint32_t> neighbours;
	glm::vec3 position;
	glm::vec3 vanDerWaalsPosition;
	glm::mat4 rotation = glm::mat4();
	int numberOfBonds;
	std::vector<glm::mat4> cylinderModels;

	/**
	 * Construct a new BondedElement (represents an atom)
	 * 
	 * @param _loneElectrons the number of lone electrons
	 * @param _bondedElectrons the number of bonded electrons
	 * @param _base the element of the atom
	 */
	BondedElement(int _loneElectrons, int _bondedElectrons, Element _base) {
		uid = generateUID();
		base = _base;
		loneElectrons = _loneElectrons;
		bondedElectrons = _bondedElectrons;
		if(base.periodNumber == 1) {
			numberOfBonds = 2 - base.valenceNumber;
		}
		else {
			numberOfBonds = 8 - base.valenceNumber;
		}
		if(numberOfBonds < 1) {
			numberOfBonds = 1;
		}
	}

	BondedElement() {}

	uint32_t getUID() {
		return uid;
	}

	/**
	 * Generate a new unique id for cloning purposes
	 */
	void refreshUID() {
		uid = generateUID();
	}

	friend bool operator==(const BondedElement &lhs, const BondedElement &rhs) {
		return lhs.uid == rhs.uid;
	}

private:
	static uint32_t maxUID;
	uint32_t uid;

	/**
	 * Generate a new unique id for atoms
	 * 
	 * @return the new id
	 */
	uint32_t generateUID() {
		maxUID++;
		return maxUID;
	}
};

/**
 * Structure representing substituent chains of an
 * organic molecule
 * E.g. 1,2-dimethylcyclopropane has 3 substituents:
 *   1: cyclopropane base
 *   2: methyl group bonded to first cyclopropane atom
 *   3: methyl group bonded to second cyclopropane atom
 * 
 * Contains data about the component atoms, parent substituent
 * if it exists, and where it connects to its parent 
 */
struct Substituent {
	std::vector<BondedElement> components;
	Substituent *parent;
	int connectionPoint;

	/**
	 * Copy this substituent while generating new unique ids
	 * for the atoms
	 * 
	 * @return the cloned substituent 
	 */
	Substituent duplicate() {
		Substituent newSub;
		std::vector<uint32_t> oldComponentIds;
		for(BondedElement b : components) {
			oldComponentIds.push_back(b.getUID());
		}
		std::vector<BondedElement> refreshComponents = this->components;
		newSub.parent = this->parent;
		newSub.connectionPoint = this->connectionPoint;

		for(int i = 0; i < this->components.size(); i++) {
			refreshComponents[i].refreshUID();
			newSub.components.push_back(refreshComponents[i]);
		}

		for(int i = 0; i < newSub.components.size(); i++) {
			for(int n = 0; n < newSub.components[i].neighbours.size(); n++) {
				for(int c = 0; c < oldComponentIds.size(); c++) {
					if(oldComponentIds[c]==newSub.components[i].neighbours[n]) {
						newSub.components[i].neighbours[n] = newSub.components[c].getUID();
					}
				}
			}
		}

		return newSub;
	}
};

// /**
//  * WIP for organic compound functional groups
//  */
// struct FunctionalGroup {
// 	std::vector<BondedElement> components;
// 	std::vector<RGroupConnection> rPorts;

// 	struct RGroupConnection {
// 		bool primary;
// 		std::vector<uint32_t> bondingAtoms;
// 	};

	
// };

extern std::vector<BondedElement> VSEPRModel;
extern std::vector<std::vector<glm::vec3>> configurations;
std::vector<BondedElement> VSEPRMain();
bool containsUID(uint32_t id, std::vector<uint32_t> list);
BondedElement findNeighbour(uint32_t key, std::vector<BondedElement> group);
void setUpMap();
std::vector<BondedElement> mutateModel(std::vector<BondedElement> model = std::vector<BondedElement>());

// ------------------------------ Chemistry utilities ------------------------------ //

// Atom state
int getFormalCharge(BondedElement b);
int checkStability(BondedElement b);
bool checkBondedElementValidity(BondedElement e);
bool bond(BondedElement &a, BondedElement &b);
void bondSafe(BondedElement &a, BondedElement &b);

// Structure state
int countElectrons(std::vector<BondedElement> structure);
int getTotalFormalCharge(std::vector<BondedElement> structure);

// General
Element searchElements(std::string symbol);
int findInstances(std::vector<uint32_t> v, uint32_t key);
bool checkStringComponent(std::string main, std::string check);
bool checkForDigits(std::string main);
int findLastComponent(std::string main, std::string check);
int findNumberTerm(std::string name);

// Positioning
std::vector<BondedElement> generateCylinders(std::vector<BondedElement> structure);
std::vector<BondedElement> averageCenterPositions(std::vector<BondedElement> structure);
std::vector<BondedElement> centerPositions(std::vector<BondedElement> structure);

// ------------------------------ Main structure predicting functions ------------------------------ //
std::vector<Element> readFormula(std::string formulaFull);
std::vector<BondedElement> constructLewisStructure(std::vector<Element> formula);
std::vector<BondedElement> interpretOrganic(std::string in);

static std::map<std::string, Substituent> functionalGroups;

#endif
