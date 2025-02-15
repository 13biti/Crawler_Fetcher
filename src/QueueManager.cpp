#include "../include/QueueManager.h"
#include "../include/PerformHttpRequest.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <curl/header.h>
#include <nlohmann/json.hpp>
#include <string>
HTTPRequest httpRequest;
const std::string &QueueManager::getToken(const std::string &username,
                                          const std::string &password,
                                          const std::string &api) {
  // Use the provided base URL or default to localhost
  std::string url = Queue_Manager_Server_Base_Url.empty()
                        ? "http://127.0.0.1:8000/login"
                        : Queue_Manager_Server_Base_Url + api;

  // Prepare JSON payload
  std::string jsonData = "{\"username\": \"" + username +
                         "\", \"password\": \"" + password + "\"}";
  std::vector<std::string> headers = {"Content-Type: application/json"};

  // Perform HTTP request
  HTTPRequest::HttpResponse response =
      httpRequest.performHttpRequest(url, "POST", jsonData, headers);

  if (response.statusCode == 200) {
    // Parse the JSON response
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
                               const std::string &token,
                               std::string api = "write") {

  std::string url = Queue_Manager_Server_Base_Url.empty()
                        ? "http://127.0.0.1:8000/write"
                        : Queue_Manager_Server_Base_Url + "/" + api;

  // Prepare JSON payload
  std::string jsonData = "{\"queue_name\": \"" + queue_name +
                         "\", \"message\": \"" + message + "\"}";
  std::vector<std::string> headers = {"Content-Type: application/json",
                                      "Authorization: Bearer " + token};

  // Perform HTTP request
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
std::string QueueManager::receiveMessage(const std::string &queue_name,
                                         const std::string &token,
                                         std::string api = "read") {

  std::string url = Queue_Manager_Server_Base_Url.empty()
                        ? "http://127.0.0.1:8000/read"
                        : Queue_Manager_Server_Base_Url + "/" + api;

  // Prepare JSON payload
  std::string jsonData = "{\"queue_name\": \"" + queue_name + "\"}";

  std::vector<std::string> headers = {"Content-Type: application/json",
                                      "Authorization: Bearer " + token};

  // Perform HTTP request
  HTTPRequest::HttpResponse response =
      httpRequest.performHttpRequest(url, "POST", jsonData, headers);

  if (response.statusCode == 200) {
    // Parse the JSON response
    try {
      nlohmann::json jsonResponse = nlohmann::json::parse(response.data);
      if (jsonResponse.contains("message")) {
        return (jsonResponse["message"]); // Set the active token
      } else {
        std::cerr << "message not found in response!" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "HTTP request failed with status code: " << response.statusCode
              << std::endl;
  }
  return "";
};
