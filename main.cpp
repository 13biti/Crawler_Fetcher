#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <regex>
using namespace std;
inline bool fileExist (const string& fileName){
  //string compliteFileName = fileName+".csv";
  ifstream myfile(fileName);
  return myfile.good();
}
//filePath should be something like that : /something/.../filename.filetype
bool CreateNewFile(const string& filePath){
  if (fileExist(filePath))
    return true;
  
  try {
    ofstream myfile;
    myfile.open(filePath , ios::ate);
    myfile.exceptions(ofstream::failbit | ofstream::badbit);
    
    if (!myfile.is_open()){
      ios_base::iostate state = myfile.rdstate();
            switch (state) {
                case std::ios_base::failbit:
                    std::cerr << "Failed to open file. check permissions or ... ";
                    break;
                case std::ios_base::badbit:
                    std::cerr << "Stream is in a bad state. check the memory or ... ";
                    break;
                case std::ios_base::eofbit:
                    std::cerr << "End of file reached unexpectedly. ";
                    break;
                default:
                    // Handle other states if needead
                    std::cerr << "Unknown error occurred. ";
                    break;
            }		
      std::cerr << "Error code: " << errno << 
        ", Error message: " << strerror(errno) << std::endl;
      throw std::ios_base::failure("Failed to open file.");  
    }
    else return true;
  }
  catch(const std::ofstream::failure& e){
    cout<< "unable to create new file . something go wrong"<<endl
      <<"Exception: "<< e.what() <<endl;
    exit(0);
  }
}
//should include regex 
void Readingfile(string filePath){
  if (fileExist(filePath)){
    fstream file (filePath , ios::in);
    string tmp , line;
    while (getline(file , tmp)){
      string target = tmp ;
      smatch result;
      regex pattern ("(?:https?://)([^/]+)");
       if (std::regex_search(target, result, pattern)) {
          std::cout << "Website: " << result.str(1) << std::endl;
        } else {
          std::cout << "Website not found in the URL." << std::endl;
        }
    }
  }
}
int main () {
  Readingfile("./test.txt");
}
