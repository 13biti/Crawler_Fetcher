#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno> 
using namespace std;

inline bool fileExist (const string& fileName){
  string compliteFileName = fileName+".csv";
  ifstream myfile(compliteFileName);
  return myfile.good();
}

bool CreateNewFile(const string& filename){
  string compliteFileName = filename+".csv";
  
  if (fileExist(compliteFileName))
    return true;
  
  try {
    ofstream myfile;
    myfile.open(compliteFileName , ios::ate);
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

int main () {

}
