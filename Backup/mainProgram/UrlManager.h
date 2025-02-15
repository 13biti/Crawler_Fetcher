#ifndef URL_MANAGER_H
#define URL_MANAGER_H

#include <string>
#include "FileOperations.h"
class UrlManager {
public:
    UrlManager(FileOperations& obj);
    void sortingUrls(const std::string &filePath);
    std::string getUrl(const std::string &Url);

private:
     FileOperations& fileOperator;
};

#endif // URL_MANAGER_H
