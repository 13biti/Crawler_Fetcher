#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
#include <string>
#include <vector>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

struct DownloadResult {
  std::string url;
  std::string html_content;
  long http_code = 0;
  CURLcode result;
  std::string error_message;
  std::string timestamp;
};
class Downloader {
public:
  Downloader();
  ~Downloader();
  DownloadResult DownloadSingle(const std::string &url);
  std::vector<DownloadResult>
  DownloadMultiple(const std::vector<std::string> &urls, int max_parallel = 5);

private:
  static size_t WriteToString(void *contents, size_t size, size_t nmemb,
                              void *userp);
  void InitCurl();
  void CleanupCurl();
  std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
  }
};

#endif // DOWNLOADER_H
