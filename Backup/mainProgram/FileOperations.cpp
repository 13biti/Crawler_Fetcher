#include "FileOperations.h"
#include <fstream>
#include <iostream>

FileOperations::FileOperations() {}

bool FileOperations::fileExist(const std::string& fileName) {
    std::ifstream myfile(fileName);
    return myfile.good();
}

bool FileOperations::createNewFile(const std::string& filePath) {
    if (fileExist(filePath)) {
        return true;
    }

    try {
        std::ofstream myfile(filePath, std::ios::app);
        if (!myfile.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return false;
        }

        //myfile << filePath << std::endl;
        myfile.close();
        myfile.open("./WebsiteRecorder.csv", std::ios::app);
        std::cout << filePath << std::endl;
        myfile << filePath << std::endl;
        myfile.close();
        filesList.insert(filePath);
        return true;
    } catch (const std::ofstream::failure& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return false;
    }
}

void FileOperations::removeLine(const std::string& urlFile) {
    std::string command = "ex -sc '1d | x' " + urlFile;
    std::system(command.c_str());
    std::cout<<"remove one line in file " << urlFile << std::endl;
}

int FileOperations::updateListForFirstTime() {
    std::fstream fileCollection("./WebsiteRecorder.csv", std::ios::ate);
    if (!fileCollection.is_open()) {
        std::cerr << "Failed to open file collection or there is not" << std::endl;
        return 1;
    }

    std::string fileName;
    while (std::getline(fileCollection, fileName)) {
        if (filesList.find(fileName) == filesList.end()) {
            filesList.insert(fileName);
        }
    }
    return 0;
}

