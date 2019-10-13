#pragma once

#include <string>
#include <map>
#include <vector>

struct Element{
	int atomicNumber;
	int valenceNumber;
	int periodNumber;
	std::string name;
	bool exception = false;


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

std::vector<Element> VSEPRMain();