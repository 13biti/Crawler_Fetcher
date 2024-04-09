#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H
bool fileExist(const std::string& fileName);
bool CreateNewFile(const std::string& filePath);
// i should change this in fouturer , i need to remove link in file not fist line
void removeLine(std::string UrlFile);
#endif // !FILE_OPERATION_H
