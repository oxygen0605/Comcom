#include<iostream>
#include<unistd.h>
#include"Comcom.hpp"

int main(){
    
    EndpointInfo ep = {"sample", 0};
    char* data = nullptr;
    int length;
    int timeout_sec = 5;

    ComCom* shm_recver = CommunicationBuilder::Build(CommunicationBuilder::SHARED_MEMORY);
    shm_recver->Connect(ep);
    
    while(true){
        shm_recver->Recv(data, length, timeout_sec);
        if(data != nullptr) {
            std::cout << "Received: " << data << std::endl;
            data = nullptr;
        }
        sleep(1);
    }
    return 0;
}