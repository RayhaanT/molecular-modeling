#include "VSEPR.h"
#include "render.h"
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

extern map<string, Element> elements;

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
 * Counts the number of electrons in a structure.
 * Used to ensure electrons aren't lost/created.
 * 
 * @param structure the atom structure
 * @return the total number of electrons
 */
int countElectrons(vector<BondedElement> structure) {
	int total = 0;
	for(BondedElement b : structure) {
		total += b.loneElectrons + (b.bondedElectrons / 2);
	}
	return total;
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
		std::sort(b.neighbours.begin(), b.neighbours.end());

		for(int i = 0; i < b.neighbours.size(); i++) {
			auto n = b.neighbours[i];
			int bondOrder = 1;

			try{
				// Max bond order is 3
				if(b.neighbours.at(i + 1) == n) {
					bondOrder++; i++;
				}
				if(b.neighbours.at(i + 2) == n) {
					bondOrder++; i++;
				}
			}
			catch (const std::exception &e) {
				// Do nothing, just to avoid if statements
			}

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

			//Cylinders
			for(int i = 0; i < bondOrder; i++) {
				float lateralOffset = stickSetWidth / 2;
				if(bondOrder == 2) {
					lateralOffset = stickSetWidth / 3 * (i + 1);
				}
				else if(bondOrder == 3) {
					lateralOffset = stickSetWidth / 2 * i;
				}
				lateralOffset -= stickSetWidth / 2;

				glm::mat4 model = glm::mat4();
				model = glm::translate(model, start);
				model = glm::rotate(model, angle, axis);
				model = glm::translate(model, glm::vec3(lateralOffset, 0, 0));
				if (updatedNeighbour.base.name == "hydrogen" || b.base.name == "hydrogen") {
					model = glm::scale(model, glm::vec3(1.0f, 0.7f, 1.0f));
				}
				b.cylinderModels.push_back(model);
			}
		}
		newStruc.push_back(b);
	}
	return newStruc;
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