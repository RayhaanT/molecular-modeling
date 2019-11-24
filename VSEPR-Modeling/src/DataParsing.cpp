#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "data.h"
#include "VSEPR.h"
#include <map>
#include <iterator>

using namespace std;

vector<string> csvStringToVector(string rawLine);
int stoiSafe(string s);
float stofSafe(string s);

extern std::map<std::string, Element> elements;

void parseCSV(std::string path)
{
    fstream inFile;
    inFile.open(path);
    if(!inFile) {
        cerr << "Unable to open " << path;
        exit(1);
    }

    while(inFile.good()) {
        string rawLine;
        getline(inFile, rawLine);
        if (!isdigit(rawLine[0])) {
            continue;
        }
        vector<string> line = csvStringToVector(rawLine);
        string name = line[5];
        string shortHand = line[1];
        int atomicNumber = stoiSafe(line[0]);
        int periodNumber = stoiSafe(line[4]);

        float electronegativity = stofSafe(line[12]);
        float atomicRadius = stofSafe(line[23]);
        //float covalentRadius = stofSafe(line[25]);
        float vanDerWaalsRadius = stofSafe(line[46])/100.0;

        float covalentRadiusSingle = (stofSafe(line[50]) + stof(line[51]))/2.0f;
        float covalentRadiusDouble = stofSafe(line[52]);
        float covalentRadiusTriple = stofSafe(line[53]);

        //CPK colors
        float colors[3];
        int colorIndex = 0;
        string value = "";
        string rgbCode = line[54];
        for(int i = 0; i < rgbCode.length(); i++) {
            if(isdigit(rgbCode[i]) && rgbCode[i] != '-') {
                value += rgbCode[i];
            }
            if(rgbCode[i] == '-') {
                colors[colorIndex] = stofSafe(value)/255.0f;
                value = "";
                colorIndex++;
            }
        }
        colors[colorIndex] = stofSafe(value)/255.0f;
        
        string electronConfig = line[22];

        int valenceNumber = stoiSafe(line[2]);
        if(valenceNumber > 12) {
            valenceNumber -= 10;
        }
        else if (valenceNumber > 2) {
            vector<string> config;
            string sect;
            stringstream configStream(electronConfig);
            int highestOrbital = 0;
            while(getline(configStream, sect, ' ')) {
                if(!isdigit(sect[0])) {
                    continue;
                }
                if(sect[0] - '0' > highestOrbital) {
                    highestOrbital = sect[0] - '0';
                }
                config.push_back(sect);
            }

            vector<string> filteredConfig;
            for(int i = 0; i < config.size(); i++) {
                if(config[i][0] - '0' == highestOrbital) {
                    filteredConfig.push_back(config[i]);
                }
            }

            valenceNumber = 0;
            for (int i = 0; i < filteredConfig.size(); i++) {
                valenceNumber += filteredConfig[i][2] - '0';
            }
        }

        Element newElement;

        newElement.name = name;
        newElement.atomicNumber = atomicNumber;
        newElement.atomicRadius = atomicRadius;
        newElement.covalentRadii[0] = covalentRadiusSingle;
        newElement.covalentRadii[1] = covalentRadiusDouble;
        newElement.covalentRadii[2] = covalentRadiusTriple;
        newElement.color.x = colors[0]; //Red
        newElement.color.y = colors[1]; //Blue
        newElement.color.z = colors[2]; //Green
        newElement.vanDerWaalsRadius = vanDerWaalsRadius;
        newElement.electronegativity = electronegativity;
        newElement.periodNumber = periodNumber;
        newElement.valenceNumber = valenceNumber;

        if(name == "beryllium" || name == "boron") {
            newElement.exception = true;
        }

        elements.insert(pair<string, Element>(shortHand, newElement));
    }
}

//stoi and stof cause errors if the string is empty
int stoiSafe(string s) {
    if(isdigit(s[0])) {
        return stoi(s);
    }
    return 0;
}

float stofSafe(string s) {
    if (isdigit(s[0])) {
        return stof(s);
    }
    return 0;
}

vector<string> csvStringToVector(string in) {
    vector<string> result;
    stringstream lineStream(in);
    string cell;

    while(getline(lineStream, cell, ',')) {
        result.push_back(cell);
    }
    //Check for trailing comma
    if (!lineStream && cell.empty()) {
        result.push_back("");
    }

    return result;
}