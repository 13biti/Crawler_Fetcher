#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main (){
  fstream file ("./test.txt" , ios::in);
  string line , tmp ;
  if (file.is_open()){
    while (getline(file , tmp)){
      line = tmp;
      cout <  < line << endl ;
    }
  }
  return 0;
}
