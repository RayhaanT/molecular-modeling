#include "VSEPR.h"

using namespace std;

extern std::map<std::string, int> numberTerms;

void setUpMap()
{
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

    // // Alkyl halides
    // Substituent bromo;
    // bromo.components.push_back(BondedElement(6, 2, elements["bromine"]));
    // Substituent chloro;
    // chloro.components.push_back(BondedElement(6, 2, ))
}
