#include "PerformHttpRequest.h"
#include <iostream>

int main() {
  HTTPRequest httpRequest;

  // Example: Perform a GET request
  std::string getResponse = httpRequest.performHttpRequest(
      "https://jsonplaceholder.typicode.com/posts/1", "GET");
  std::cout << "GET Response: " << getResponse << std::endl;

  // Example: Perform a POST request with headers and data
  std::vector<std::string> headers = {"Content-Type: application/json",
                                      "Authorization: Bearer YOUR_TOKEN_HERE"};
  std::string postData = R"({"key": "value"})";
  std::string postResponse = httpRequest.performHttpRequest(
      "https://jsonplaceholder.typicode.com/posts", "POST", postData, headers);
  std::cout << "POST Response: " << postResponse << std::endl;

  return 0;
}
