#include "Politeness.h"
#include "UrlManager.h" // Assuming getUrl function is declared here

#include <fstream>
#include <iostream>
#include <unistd.h>

Politeness::Politeness(int fixTimer) : DefaultTimer(120) {}  // Constructor definition


void Politeness::updatePoliteList (std::string websiteName , int timeCounter) {
    politenessMap[websiteName] = timeCounter;
}
//    std::fstream myfile("./WebsiteRecorder.csv", std::ios::in);
//    std::string line;
//    while (std::getline(myfile, line)) {
//        if (politenessMap.find(line) == politenessMap.end()) {
//            politenessMap[line] = DefaultTimer;
//        }
//    }

void Politeness::Timer() {
    for (auto& pair : politenessMap) {
        std::cout<<pair.second<<std::endl;
        if (pair.second > 0) {
            pair.second -= 60;
        }
    }
}

void Politeness::emptyDomainDeclaration(std::string key, int value) {
   int x = value < 0 ? -1 : DefaultTimer;
    politenessMap[key] = x;
}

