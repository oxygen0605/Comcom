#include<iostream>
#include<unistd.h>
#include"Comcom.hpp"

int main(){
    
    EndpointInfo ep = {"sample", 1};
    char* dataset;
    int length;

    ComCom* shm_sender = CommunicationBuilder::Build(CommunicationBuilder::SHARED_MEMORY);
    shm_sender->Connect(ep);
    
    for(int i = 0; i < 5; ++i){
        std::string message = "Hello from writer!" + std::to_string(i);
        shm_sender->Send(message.c_str(), message.length());
        sleep(1);
    }
    delete shm_sender;
    return 0;
}