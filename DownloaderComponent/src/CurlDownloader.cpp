#include "../include/CurlDownloder.h"
#include <iostream>
#include <sstream>

Downloader::Downloader() { InitCurl(); }

Downloader::~Downloader() { CleanupCurl(); }

void Downloader::InitCurl() { curl_global_init(CURL_GLOBAL_ALL); }

void Downloader::CleanupCurl() { curl_global_cleanup(); }

size_t Downloader::WriteToString(void *contents, size_t size, size_t nmemb,
                                 void *userp) {
  size_t totalSize = size * nmemb;
  std::string *str = static_cast<std::string *>(userp);
  str->append(static_cast<char *>(contents), totalSize);
  return totalSize;
}

DownloadResult Downloader::DownloadSingle(const std::string &url) {
  CURL *curl = curl_easy_init();
  DownloadResult result;
  result.url = url;

  if (!curl) {
    result.result = CURLE_FAILED_INIT;
    result.error_message = "Failed to initialize curl.";
    return result;
  }

  std::string buffer;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "HTML-Downloader/1.0");

  result.result = curl_easy_perform(curl);
  if (result.result == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.http_code);
    result.html_content = buffer;
  } else {
    result.error_message = curl_easy_strerror(result.result);
  }
  result.timestamp = GetCurrentTimestamp();
  curl_easy_cleanup(curl);
  return result;
}

std::vector<DownloadResult>
Downloader::DownloadMultiple(const std::vector<std::string> &urls,
                             int max_parallel) {
  CURLM *multi_handle = curl_multi_init();
  std::vector<DownloadResult> results(urls.size());
  std::vector<CURL *> easy_handles(urls.size(), nullptr);
  std::vector<std::string> buffers(urls.size());

  int running_handles = 0;
  size_t transfers = 0;
  size_t completed = 0;

  auto setup_transfer = [&](size_t i) {
    CURL *easy = curl_easy_init();
    buffers[i].clear();

    curl_easy_setopt(easy, CURLOPT_URL, urls[i].c_str());
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &buffers[i]);
    curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, "HTML-Downloader/1.0");

    curl_easy_setopt(easy, CURLOPT_PRIVATE, reinterpret_cast<void *>(i));
    curl_multi_add_handle(multi_handle, easy);
    results[i].url = urls[i];
    easy_handles[i] = easy;
  };

  while (transfers < urls.size() &&
         transfers < static_cast<size_t>(max_parallel)) {
    setup_transfer(transfers++);
  }

  while (completed < urls.size()) {
    curl_multi_perform(multi_handle, &running_handles);

    CURLMsg *msg;
    int msgs_left;
    while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
      if (msg->msg == CURLMSG_DONE) {
        CURL *handle = msg->easy_handle;
        size_t index;
        curl_easy_getinfo(handle, CURLINFO_PRIVATE, &index);
        results[index].result = msg->data.result;
        if (msg->data.result == CURLE_OK) {
          results[index].html_content = buffers[index];
          curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE,
                            &results[index].http_code);
        } else {
          results[index].error_message = curl_easy_strerror(msg->data.result);
        }
        results[index].timestamp = GetCurrentTimestamp();
        curl_multi_remove_handle(multi_handle, handle);
        curl_easy_cleanup(handle);
        completed++;
        if (transfers < urls.size()) {
          setup_transfer(transfers++);
        }
      }
    }

    if (running_handles > 0) {
      curl_multi_wait(multi_handle, NULL, 0, 1000, NULL);
    }
  }

  curl_multi_cleanup(multi_handle);
  return results;
}
