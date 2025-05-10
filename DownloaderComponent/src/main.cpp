#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../include/CurlDownloder.h"
#include <chrono>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <thread>
using json = nlohmann::json;
void downloader() {
  std::cout << "downloader is initiated " << std::endl;
  QueueManager *newLinksQueue = new QueueManager(Config::queueBaseUrl);
  Downloader downloader;

  newLinksQueue->getToken(Config::downloaderReadUsername, Config::queuePassword,
                          Config::apiLogin);
  auto readtoken = newLinksQueue->returnToken();

  newLinksQueue->getToken(Config::downloaderWriteUsername,
                          Config::queuePassword, Config::apiLogin);
  auto writetoken = newLinksQueue->returnToken();

  auto getLink = [newLinksQueue, &readtoken]() -> UrlPack {
    auto msg = newLinksQueue->receiveMessage(Config::downloadLinksQueueName,
                                             readtoken, Config::apiReceive);

    if (msg.status) {
      json jsonPayload = json::parse(msg.message);
      std::string downloadbleUrl = jsonPayload["Url"];
      std::string base_url = jsonPayload["base_url"];
      std::cout << "[main][downloader][getlink] unpacking URLPACK resutl : "
                << base_url + " \t " << downloadbleUrl << std::endl;
      return UrlPack{base_url, downloadbleUrl, true};
    } else
      return UrlPack();
  };
  auto sendResutl = [newLinksQueue,
                     writetoken](const DownloadResult &res) -> void {
    json payload = res.to_json();
    newLinksQueue->sendMessage(Config::downloadedQueueName,
                               payload.dump(), // Convert JSON to string
                               writetoken, Config::apiSend);
  };

  std::cout << "lord have mercy \n";
  while (true) {
    UrlPack message = getLink();
    std::cout << "message ready to downloaded " << message.status << message.Url
              << message.base_url << "\n";
    if (message.status) {
      DownloadResult result = downloader.DownloadSingle(message.Url);
      result.base_url = message.base_url;
      sendResutl(result);
    } else {
      std::this_thread::sleep_for(
          std::chrono::seconds(2)); // back off on failure
    }
  }
}
int main() {
  downloader();
  return 0;
}
