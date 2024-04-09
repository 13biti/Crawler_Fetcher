//#ifndef FILE_OPERATION_H
//#define FILE_OPERATION_H
//bool fileExist(const std::string& fileName);
//bool CreateNewFile(const std::string& filePath);
//// i should change this in fouturer , i need to remove link in file not fist line
//void removeLine(std::string UrlFile);
//#endif // !FILE_OPERATION_H
#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include <unordered_set>

class FileOperations {
public:
    FileOperations();
    bool fileExist(const std::string& fileName);
    bool createNewFile(const std::string& filePath);
    void removeLine(const std::string& urlFile);
    int updateListForFirstTime();

    std::unordered_set<std::string> filesList;
private:

};

#endif // FILE_OPERATIONS_H
