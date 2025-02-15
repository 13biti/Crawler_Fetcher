#include "UrlManager.h"
#include "FileOperations.h"
#include <iostream>
#include <fstream>
#include <regex>

// Define fileOperator as a static member of UrlManager

UrlManager::UrlManager(FileOperations& obj) : fileOperator(obj){}

void UrlManager::sortingUrls(const std::string &filePath) {
    if (!fileOperator.fileExist(filePath)) {
        std::cerr << "Error: File does not exist: " << filePath << std::endl;
        return;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::smatch result;
        std::regex pattern("(?:https?://)([^/]+)");
        if (std::regex_search(line, result, pattern)) {
            std::string target = result.str(1);
            std::string UrlFilePath = target + ".csv";
            if (!fileOperator.createNewFile(UrlFilePath)) {
                std::cerr << "Error: Failed to create file: " << UrlFilePath << std::endl;
                continue;
            }
            std::ofstream UrlFile(UrlFilePath, std::ios::app);
            if (!UrlFile.is_open()) {
                std::cerr << "Error: Failed to open file for writing: " << UrlFilePath << std::endl;
                continue;
            }
            UrlFile << line << std::endl;
            UrlFile.close();
        } else {
            std::cerr << "Warning: Website not found in the URL. It is not correct!" << std::endl;
        }
    }
}

std::string UrlManager::getUrl(const std::string &Url) {
    std::ifstream file(Url);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << Url << std::endl;
        return "";
    }
    std::string tmp;
    std::getline(file, tmp);
    if (file.eof()) {
        std::cerr << "Warning: File is empty: " << Url << std::endl;
        return "";
    }
    fileOperator.removeLine(Url);
    return tmp;
}
