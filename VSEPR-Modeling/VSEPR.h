#pragma once

#ifndef VSEPR_H
#define VSEPR_H

#include <string>
#include <map>
#include <vector>
#include "include/glm/glm.hpp"

struct Element{
	int atomicNumber;
	int valenceNumber;
	int periodNumber;
	std::string name;
	bool exception = false;

	Element() {

	}

	Element(int _atomicNumber, int _valenceNumber, int _periodNumber, std::string _name) {
		atomicNumber = _atomicNumber;
		valenceNumber = _valenceNumber;
		periodNumber = _periodNumber;
		name = _name;
	}

	Element(int _atomicNumber, int _valenceNumber, int _periodNumber, std::string _name, bool _exception) {
		exception = _exception;
		valenceNumber = _valenceNumber;
		periodNumber = _periodNumber;
		name = _name;
	}
};

struct BondedElement {
	Element base;
	int lonePairs;
	int bondedPairs;

	BondedElement(int _lonePairs, int _bondedPairs, Element _base) {
		base = _base;
		lonePairs = _lonePairs;
		bondedPairs = _bondedPairs;
	}
};

extern std::vector<BondedElement> VSEPRModel;
std::vector<BondedElement> VSEPRMain();

#endif