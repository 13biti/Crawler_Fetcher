#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../include/CurlDownloder.h"
#include <iostream>
#include <set>
#include <string>
void downloader() {
  QueueManager *newLinksQueue = new QueueManager(Config::queueBaseUrl);
  Downloader downloader;

  newLinksQueue->getToken(Config::downloaderReadUsername, Config::queuePassword,
                          Config::apiLogin);
  auto readtoken = newLinksQueue->returnToken();

  newLinksQueue->getToken(Config::downloaderWriteUsername,
                          Config::queuePassword, Config::apiLogin);
  auto writetoken = newLinksQueue->returnToken();

  auto getLink = [newLinksQueue, &readtoken]() -> QueueManager::Message {
    return newLinksQueue->receiveMessage(Config::downloadLinksQueueName,
                                         readtoken, Config::apiReceive);
  };
  auto sendResutl = [newLinksQueue,
                     writetoken](const DownloadResult &res) -> void {
    json payload = res.to_json();
    newLinksQueue->sendMessage(Config::downloadedQueueName,
                               payload.dump(), // Convert JSON to string
                               writetoken, Config::apiSend);
  };
  while (true) {

    QueueManager::Message message = getLink();
    DownloadResult result = downloader.DownloadSingle(message.message);
    sendResutl(result);
  }
}
int main() {}
