#include <iostream>
#include <fstream>
#include <string>
using namespace std;
inline bool fileExist (const string& fileName){
  ifstream fileStatus;
  fileStatus.open(fileName);

  return fileStatus.good();
}
int main () {
  std::string fileName = "";
  int counter = 1;
  int Nfile;
  std::cout<< "how many file ?" ;
  std::cin >> Nfile ;
  for (int i = 0 ; i < Nfile ; i++){
    std::cout << "enter file name :" << std::endl;
    std::cin >> fileName;
    string compliteName = to_string(counter)+"_"+fileName+".txt";
    if (fileExist(compliteName)){
      cout<<"file alrady exist !"<<endl;
    }else{
      std::ofstream myfile(compliteName);
    }
  }

}
