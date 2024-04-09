#include "Politeness.h"
#include "UrlManagement.h" // Assuming getUrl function is declared here

#include <fstream>
#include <iostream>
#include <unistd.h>

Politeness::Politeness() : DefaultTimer(3 * 60) {}  // Constructor definition

void Politeness::addToPolitnessList(std::string websiteName) {
    politenessMap[websiteName] = DefaultTimer;
}

void Politeness::updatePoliteList() {
    std::fstream myfile("./WebsiteRecorder.csv", std::ios::in);
    std::string line;
    while (std::getline(myfile, line)) {
        if (politenessMap.find(line) == politenessMap.end()) {
            politenessMap[line] = DefaultTimer;
        }
    }
}

void Politeness::Timer() {
    for (auto& pair : politenessMap) {
        if (pair.second > 0) {
            pair.second -= 60;
            if (pair.second == 0) {
                usleep(1000000);
                pair.second = DefaultTimer;
                std::cout << getUrl(pair.first) << std::endl;
            }
        }
    }
}

void Politeness::setkeyValue(std::string key, int value) {
    politenessMap[key] = (value == -1) ? -1 : DefaultTimer;
}

