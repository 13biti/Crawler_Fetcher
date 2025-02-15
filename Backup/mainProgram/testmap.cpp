#include <map>
#include<string>
#include <iostream>
std::map<std::string, int> politenessMap;
int main (){
for (int i = 0 ; i < 1000000000 ; i ++){
  std::pair<std::string , int> p1 (("websiteName" + std::to_string(i)) , (i*2));
    politenessMap.insert(p1); 
}
for (auto& pair : politenessMap) {
            pair.second -= 60;
    }
std::cout<<"done" << std::endl;
}
