#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../include/CurlDownloder.h"
#include <iostream>
#include <set>
void downloader() {
  QueueManager *newLinksQueue = new QueueManager(Config::queueBaseUrl);
  newLinksQueue->getToken(Config::downloaderReadUsername, Config::queuePassword,
                          Config::apiLogin);

  auto getLink = [newLinksQueue]() -> QueueManager::Message {
    return newLinksQueue->receiveMessage(Config::downloadLinksQueueName);
  };

  newLinksQueue->getToken(Config::downloaderWriteUsername,
                          Config::queuePassword, Config::apiLogin);
  auto token = newLinksQueue->returnToken();
  Downloader downloader;
  auto sendLink = [newLinksQueue, token](const DownloadResult &res) -> void {
    std::string payload = "{ \"url\": \"" + res.url + "\", \"http_code\": " +
                          std::to_string(res.http_code) + "\"timestamp\": \"" +
                          res.timestamp + "\"html\": \"" + res.html_content +
                          "\" }";
    newLinksQueue->sendMessage(Config::downloadedQueueName, payload, token,
                               Config::apiSend);
  };

  QueueManager::Message message = getLink();
  DownloadResult result = downloader.DownloadSingle(message.message);
  sendLink(result);
}
int main() {}
