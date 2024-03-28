#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <regex>
#include <map>
#include <unistd.h>
#include <cstdlib>
using namespace std;
string getReadyUrl(string ReadyUrl);
void removeLine(string ReadyUrl);
bool haveUrl(string websiteName);
class Politeness 
{
  private :
    std::map<std::string , int> politenessMap;
    int DefaultTimer = (3*60);
  public:
    void addToPolitnessList(string websiteName){
      pair<string , int>p1 (websiteName , DefaultTimer);
      politenessMap.insert(p1);
    }
    void updatePoliteList(){
      fstream myfile("./WebsiteRecorder.csv" , ios::in);
      string line;
      while (getline(myfile , line)){
        if (politenessMap.find(line) != politenessMap.end())
          continue;
        pair<string , int> p1 (line , DefaultTimer);
        politenessMap.insert(p1);
        //for (auto itr = politenessMap.begin() ; itr != politenessMap.end() ; ++itr){
      }
    }
    void Timer(){
      for (auto itr = politenessMap.begin() ; itr != politenessMap.end() ; ++itr){
        if(itr->second > 0){
          itr->second -= 60;
          if (itr->second == 0){
            usleep(1000000);
            itr->second = DefaultTimer;
            cout<< getReadyUrl(itr->first)<<endl;
          }
        }
      }
    }
    void setkeyValue(string key , int value){
      switch (value){
        case -1:
          politenessMap[key] = -1;
          break;
        default:
          politenessMap[key] = DefaultTimer;
          break;
      }
    }
};
Politeness list1;

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
    else{
      myfile.close();
      myfile.open("./WebsiteRecorder.csv" , ios::app);
      cout<<filePath<<endl;
      myfile << filePath << endl;
      myfile.close();
      list1.addToPolitnessList(filePath);
      return true;
    }
  }
  catch(const std::ofstream::failure& e){
    cout<< "unable to create new file . something go wrong"<<endl
      <<"Exception: "<< e.what() <<endl;
    exit(0);
  }
}
//should include regex 
void SortingAllUrls(string filePath){
  if (fileExist(filePath)){
    fstream file (filePath , ios::in);
    string tmp , line;
    while (getline(file , tmp)){
      string target = tmp ;
      smatch result;
      regex pattern ("(?:https?://)([^/]+)");
       if (std::regex_search(target, result, pattern)) {
          //now we have website name so lets find out if its file exist or not !!
          string UrlFilePath = result.str(1)+".csv";
          CreateNewFile(UrlFilePath);
          try{
            fstream UrlFile (UrlFilePath , ios::app);
            // here add url to file 
            if (UrlFile.is_open()){
              UrlFile << target << endl;
              UrlFile.close();
              list1.setkeyValue(UrlFilePath , 0);
            }else {
              throw std::ios_base::failure("faild to open file in inserting url !");
            }
          }catch(...){
            std::cout<<"cannot open file to insert url !"<<endl;
          }
        } else {
          std::cout << "Website not found in the URL. its not correct!!" << std::endl;
        }
    }
  }
}


string getReadyUrl(string ReadyUrl){
  string filePath = ReadyUrl;
  ifstream file (filePath , ios::in);
  string tmp ;
  getline(file , tmp);
  if (file.eof()){
    list1.setkeyValue(ReadyUrl , -1);
    return "";
  }
  removeLine(ReadyUrl);
  return tmp;
}
void removeLine(string ReadyUrl){
  string command = " ex -sc '1d | x' " + ReadyUrl;
  std::system(command.c_str());
}
int main () {
  list1.updatePoliteList();
  SortingAllUrls("./test.txt");
  while (1){
    usleep(1000000);
    list1.Timer();
  }
}
