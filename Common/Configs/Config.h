#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
  static std::string getenvOrDefault(const char *name, const char *defaultVal);

  static const std::string queueBaseUrl;
  static const std::string rawLinksQueueName;
  static const std::string downloadLinksQueueName;
  static const std::string downloadedQueueName;
  static const std::string processorWriteUsername;
  static const std::string processorReadUsername;
  static const std::string downloaderWriteUsername;
  static const std::string downloaderReadUsername;
  static const std::string queuePassword;
  static const std::string apiLogin;
  static const std::string apiSend;
  static const std::string apiReceive;
  static const std::string mongoUrlsUri;
  static const std::string mongoUrlsDb;
  static const std::string mongoUrlsClient;
  static const std::string mongoHandlerClient;
};

#endif // CONFIG_H
