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
struct Element{
	int atomicNumber;
	int valenceNumber;
	int periodNumber;
	std::string name;
	float electronegativity;
	float atomicRadius;
	float covalentRadius;
	float vanDerWaalsRadius;
	bool exception = false;

	Element() {}

	Element(int _atomicNumber) {atomicNumber = _atomicNumber;}

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