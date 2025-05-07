#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../include/CurlDownloder.h"
#include <chrono>
#include <string>
#include <thread>
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

  auto getLink = [newLinksQueue, &readtoken]() -> QueueManager::Message {
    auto msg = newLinksQueue->receiveMessage(Config::downloadLinksQueueName,
                                             readtoken, Config::apiReceive);
    if (msg.status)
      return msg;
    return QueueManager::Message{false, ""};
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
    if (message.status) {
      std::cout << "here is the message " << message.message;
      DownloadResult result = downloader.DownloadSingle(message.message);
      std::cout << "here is the result code  " << result.error_message
                << result.http_code;
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
