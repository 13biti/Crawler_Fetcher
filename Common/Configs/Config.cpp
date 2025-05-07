#include "Config.h"
#include <cstdlib>

#define NEW_LINKS_QUEUE_BASE_URL "http://127.0.0.1:5000"
std::string Config::getenvOrDefault(const char *name, const char *defaultVal) {
  const char *val = std::getenv(name);
  return val ? val : defaultVal;
}
const std::string Config::queueBaseUrl =
    Config::getenvOrDefault("QUEUE_BASE_URL", "http://127.0.0.1:5000");
const std::string Config::rawLinksQueueName =
    Config::getenvOrDefault("RAW_LINKS_QUEUE_NAME", "rawlinks");
const std::string Config::downloadLinksQueueName =
    Config::getenvOrDefault("DOWNLOAD_LINKS_QUEUE_NAME", "downloadlinks");
const std::string Config::downloadedQueueName =
    Config::getenvOrDefault("DOWNLOADED_QUEUE_NAME", "downloaded");
const std::string Config::processorWriteUsername =
    Config::getenvOrDefault("PROC_WRITE_USERNAME", "u2");
const std::string Config::processorReadUsername =
    Config::getenvOrDefault("PROC_READ_USERNAME", "u1");
const std::string Config::downloaderWriteUsername =
    Config::getenvOrDefault("DOWN_WRITE_USERNAME", "u2");
const std::string Config::downloaderReadUsername =
    Config::getenvOrDefault("DOWN_READ_USERNAME", "u1");
const std::string Config::queuePassword =
    Config::getenvOrDefault("QUEUE_PASSWORD", "123");
const std::string Config::apiLogin =
    Config::getenvOrDefault("API_LOGIN", "login");
const std::string Config::apiSend =
    Config::getenvOrDefault("API_SEND", "/write");
const std::string Config::apiReceive =
    Config::getenvOrDefault("API_RECEIVE", "/read");
const std::string Config::mongoUrlsUri =
    Config::getenvOrDefault("MONGO_URLS_URI", "");
const std::string Config::mongoUrlsDb =
    Config::getenvOrDefault("MONGO_URLS_DB", "");
const std::string Config::mongoUrlsClient =
    Config::getenvOrDefault("MONGO_URLS_CLIENT", "");
