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
class Politeness 
{
  private :
    std::map<std::string , int> politenessMap;
    int DefaultTimer = (3*60);
  public:
    void addToPolitnessList(string websiteName){
      pair<string , int>p1 (websiteName , DefaultTimer);
      politenessMap.insert(p1);
      cout<<"something add to list1 !!"<<endl;
    }
    void Timer(){
      for (auto itr = politenessMap.begin() ; itr != politenessMap.end() ; ++itr){
        itr->second -= 60;
        cout<<itr->second<<endl;
        if (itr->second == 0){
          cout<<"some one reached zero !" << endl;
          usleep(1000000);
          cout<< getReadyUrl(itr->first)<<endl;
          itr->second = DefaultTimer;
        }
      }
      cout<<"reduction happend"<<endl;
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
  fstream file (filePath , ios::in);
  string tmp ;
  if (file.peek() == std::ifstream::traits_type::eof())
    return "";
  getline(file , tmp);
  removeLine(ReadyUrl);
  return tmp;
}
void removeLine(string ReadyUrl){
  string command = " ex -sc '1d | x' " + ReadyUrl;
  cout<<command<<endl;
  std::system(command.c_str());
}
int main () {
  SortingAllUrls("./test.txt");
  while (1){
    usleep(3000000);
    list1.Timer();
  }
}
