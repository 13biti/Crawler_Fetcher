#include <regex>
#include <fstream>
#include <string>
#include <iostream>
#include "FileOperations.h"
#include "UrlManagement.h"

using namespace std;
void SortingUrls(string filePath){
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
       //       list1.setkeyValue(UrlFilePath , 0);
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

string getUrl(string Url){
  string filePath = Url;
  ifstream file (filePath , ios::in);
  string tmp ;
  getline(file , tmp);
  if (file.eof()){
    //list1.setkeyValue(Url , -1);
    return "";
  }
  removeLine(Url);
  return tmp;
}
