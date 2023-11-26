#pragma once

#include <memory>
#include <string>

class CommonCommunicatior;
using ComCom = CommonCommunicatior;

typedef struct EndpointInfo {
    std::string endpointName;
    int sendFlag;
} EndpointInfo;

/* 
 * CommonCommunicatior
 * this class provide user programs same interfaces such as socket, message queue, shared memory.
 */
class CommonCommunicatior {
private:

protected:
    CommonCommunicatior(){}

public:

    virtual ~CommonCommunicatior(){}
    CommonCommunicatior(const CommonCommunicatior&) = default;
    CommonCommunicatior& operator=(const CommonCommunicatior&) = default;
    CommonCommunicatior(CommonCommunicatior&&) = default;
    CommonCommunicatior& operator=(CommonCommunicatior&&) = default;

    virtual void Open(){}
    virtual void Close(){}
    virtual void Connect(EndpointInfo& addr_info) = 0;
    virtual void Disconnect() = 0;
    virtual void Send(const char* dataset, const int& length) = 0;
    virtual void Recv(char*& dataset, int& length, int& timeout_sec) = 0;
};

class SocketCommunicatior : public CommonCommunicatior {
public:
    // Implement the virtual functions here
    void Connect(EndpointInfo& addr_ifno) override{}
    void Send(const char* dataset, const int& length) override{}
    void Recv(char*& dataset, int& length, int& timeout_sec) override{}
    void Disconnect()override{}
};

class SharedMemoryCommunicatior : public CommonCommunicatior {
private:
    // pImpl idiomでコンパイル時にファイルの依存性を回避する。
    struct Config;
    std::unique_ptr<Config> _pConfig;
    //共有メモリ領域のデータ構造とポインタ
    struct SharedData; 
    SharedData* _sharedData;

public:
    SharedMemoryCommunicatior();
    ~SharedMemoryCommunicatior();

    SharedMemoryCommunicatior(const SharedMemoryCommunicatior&);
    SharedMemoryCommunicatior& operator=(const SharedMemoryCommunicatior&);
    SharedMemoryCommunicatior(SharedMemoryCommunicatior&&);
    SharedMemoryCommunicatior& operator=(SharedMemoryCommunicatior&&);
    
    void Connect(EndpointInfo& addr_ifno) override;
    void Send(const char* dataset, const int& length) override;
    void Recv(char*& dataset, int& length, int& timeout_sec) override;
    void Disconnect() override;
};

class CommunicationBuilder {
public:
    enum CommunicationType {
        SOCKET,
        MESSAGE_QUEUE,
        SHARED_MEMORY
    };

    static CommonCommunicatior* Build(CommunicationType type){
        switch(type){
            //case SOCKET:
            //   return new SocketCommunicatior();
            //case MESSAGE_QUEUE:
            //    return new MessageQueueCommunicatior();
            case SHARED_MEMORY:
                return new SharedMemoryCommunicatior();
            default:
                return NULL;
        }
    };
};