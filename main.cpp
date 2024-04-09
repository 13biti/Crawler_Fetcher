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
#include "UrlManagement.h"
#include "Politeness.h"

using namespace std;
Politeness list1;

int main () {
  list1.updatePoliteList();
  SortingUrls("./test.txt");
  std::cout << "done sorting "<<endl;
  while (1){
    usleep(1000000);
    cout << "done ss "<<endl;
    list1.Timer();
    cout<< "out of timer  "<<endl;
  }
}
