#ifndef POLITENESS_H
#define POLITENESS_H

#include <map>
#include <string>

class Politeness {
private:
    std::map<std::string, int> politenessMap;
    int DefaultTimer;

public:
    Politeness();  // Constructor declaration
    void addToPolitnessList(std::string websiteName);
    void updatePoliteList();
    void Timer();
    void setkeyValue(std::string key, int value);
};

#endif // POLITENESS_H

