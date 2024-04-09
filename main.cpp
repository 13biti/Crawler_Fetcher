#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <regex>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include "FileOperations.h"
#include "UrlManager.h"
#include "Politeness.h"

using namespace std;
Politeness politeness(2*60);
FileOperations fileOperator;
UrlManager urlManager(fileOperator);

int main () {
    fileOperator.updateListForFirstTime();
    urlManager.sortingUrls("/home/kk_gorbee/Documents/project/Fetcher/mainProgram/test.txt");
    for(const auto& element : fileOperator.filesList) {
        if (politeness.politenessMap.find(element) == politeness.politenessMap.end())
            politeness.updatePoliteList(element, 180);
    }

    while (true) {
        usleep(1000000);
        politeness.Timer();
        for (auto itr =politeness.politenessMap.begin() ; itr !=politeness.politenessMap.end() ; ++itr) {
            if (itr->second == 0) {
                string Url = urlManager.getUrl(itr->first);
                if (Url == "") {
                    politeness.emptyDomainDeclaration(itr->first, -1);
                    continue;
                }
                std::cout << Url << std::endl;
                politeness.emptyDomainDeclaration(itr->first, 0);
            }
        }
    }

    //  list1.updatePoliteList();
//  SortingUrls("./test.txt");
//  std::cout << "done sorting "<<endl;
//  while (1){
//    usleep(1000000);
//    cout << "done ss "<<endl;
//    list1.Timer();
//    cout<< "out of timer  "<<endl;
}
