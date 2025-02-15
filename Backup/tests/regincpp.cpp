#include <iostream>
#include <fstream>
#include <regex>

int main() {
    const std::string pattern = ".*10000000.*";  // Your regular expression pattern
    const std::string filename = "DataBaseLinkFile.txt";  // Your file name

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::stringstream fileContent;
    fileContent << file.rdbuf();
    file.close();

    std::regex regexPattern(pattern);
    if (std::regex_search(fileContent.str(), regexPattern)) {
        std::cout << "Pattern found!" << std::endl;
    } else {
        std::cout << "Pattern not found." << std::endl;
    }

    return 0;
}

