#include "../include/QueueManager.h"
#include "../include/PerformHttpRequest.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <curl/curl.h>
#include <curl/header.h>
#include <nlohmann/json.hpp>
#include <string>
HTTPRequest httpRequest;
// i think one day it may needed !!
QueueManager::~QueueManager() { return; }
const std::string &QueueManager::getToken(const std::string &username,
                                          const std::string &password,
                                          const std::string &api) {
  std::string url =
      Queue_Manager_Server_Base_Url.empty()
          ? "http://127.0.0.1:8000/" + api
          : Queue_Manager_Server_Base_Url +
                (Queue_Manager_Server_Base_Url.back() == '/' ? "" : "/") + api;

  std::string jsonData = "{\"username\": \"" + username +
                         "\", \"password\": \"" + password + "\"}";
  std::vector<std::string> headers = {"Content-Type: application/json"};

  HTTPRequest::HttpResponse response =
      httpRequest.performHttpRequest(url, "POST", jsonData, headers);

  if (response.statusCode == 200) {
    try {
      nlohmann::json jsonResponse = nlohmann::json::parse(response.data);
      if (jsonResponse.contains("token")) {
        setActive_token(jsonResponse["token"]); // Set the active token
      } else {
        std::cerr << "Token not found in response!" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "HTTP request failed with status code: " << response.statusCode
              << std::endl;
  }
  return Active_Token;
}
bool QueueManager::createQueue(const std::string &queueName, std::string &api) {
  return true;
};
bool QueueManager::sendMessage(const std::string &queue_name,
                               const std::string &message,
                               const std::string &token, std::string api) {

  std::string url =
      Queue_Manager_Server_Base_Url.empty()
          ? "http://127.0.0.1:8000/" + api
          : Queue_Manager_Server_Base_Url +
                (Queue_Manager_Server_Base_Url.back() == '/' ? "" : "/") + api;

  std::string jsonData = "{\"queue_name\": \"" + queue_name +
                         "\", \"message\": \"" + message + "\"}";
  std::vector<std::string> headers = {"Content-Type: application/json",
                                      "Authorization: Bearer " + token};

  HTTPRequest::HttpResponse response =
      httpRequest.performHttpRequest(url, "POST", jsonData, headers);

  if (response.statusCode == 200)
    return true;
  else {
    std::cerr << "HTTP request failed with status code: " << response.statusCode
              << std::endl;
    return false;
  }
};
QueueManager::Message
QueueManager::receiveMessage(const std::string &queue_name,
                             const std::string &token, std::string api) {

  std::string url =
      Queue_Manager_Server_Base_Url.empty()
          ? "http://127.0.0.1:8000/" + api
          : Queue_Manager_Server_Base_Url +
                (Queue_Manager_Server_Base_Url.back() == '/' ? "" : "/") + api;
  std::string jsonData = "{\"queue_name\": \"" + queue_name + "\"}";

  std::vector<std::string> headers = {"Content-Type: application/json",
                                      "Authorization: Bearer " + token};

  HTTPRequest::HttpResponse response =
      httpRequest.performHttpRequest(url, "POST", jsonData, headers);

  if (response.statusCode == 200) {
    try {
      nlohmann::json jsonResponse = nlohmann::json::parse(response.data);

      if (jsonResponse.contains("message") &&
          !jsonResponse["message"].is_null())
        return QueueManager::Message{
            true, jsonResponse["message"].get<std::string>()};
      else
        std::cerr << "message field is missing or null in response!"
                  << std::endl;

    } catch (const std::exception &e) {
      std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "HTTP request failed with status code: " << response.statusCode
              << std::endl;
  }
  return QueueManager::Message();
};
