#ifndef DOWNLOADER_H
#define DOWNLOADER_H
#include <chrono>
#include <cppcodec/base64_default_rfc4648.hpp>
#include <curl/curl.h>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>
using json = nlohmann::json;

struct DownloadResult {
  std::string url;
  std::string html_content_base64;
  long http_code = 0;
  CURLcode result;
  std::string error_message;
  std::string timestamp;

  json to_json() const {
    return {{"url", url},
            {"html_content_base64", html_content_base64},
            {"http_code", http_code},
            {"result", result},
            {"error_message", error_message},
            {"timestamp", timestamp}};
  }

  auto get_encoded_html(std::string html_content) const {
    return cppcodec::base64_rfc4648::encode(html_content.data(),
                                            html_content.size());
  }
  std::string get_decoded_html() const {
    auto decoded =
        cppcodec::base64_rfc4648::decode<std::string>(html_content_base64);
    return decoded;
  }

  static DownloadResult from_json(const json &j) {
    DownloadResult res;
    res.url = j.value("url", "");
    res.html_content_base64 = j.value("html_content_base64", "");
    res.http_code = j.value("http_code", 0L);
    res.result = static_cast<CURLcode>(j.value("result", 0));
    res.error_message = j.value("error_message", "");
    res.timestamp = j.value("timestamp", "");
    return res;
  }
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
