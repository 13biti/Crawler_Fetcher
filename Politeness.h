#ifndef POLITENESS_H
#define POLITENESS_H

#include <map>
#include <string>

class Politeness {
private:


public:
    Politeness(int fixTimer);  // Constructor declaration

    std::map<std::string, int> politenessMap;
    int DefaultTimer;

    void updatePoliteList(std::string websiteName , int timeCounter);
    void Timer();
    void emptyDomainDeclaration(std::string key, int value);
};

#endif // POLITENESS_H

