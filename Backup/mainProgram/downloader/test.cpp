#include <iostream>
#include <string>
#include "QueueManager.h"
using namespace std;
int main (){
    QueueManager queueManager("localhost" , 5672);
    std::string receivedUrl = queueManager.receiveMessage("test");
    cout<<receivedUrl<<std::endl;


}
//
// Created by kk_gorbee on 4/15/24.
//

