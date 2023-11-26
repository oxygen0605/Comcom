#include "Comcom.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <mqueue.h>
#include <cerrno>
#include <pthread.h>
#include <memory>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>

struct SharedMemoryCommunicatior::SharedData {
    pthread_mutex_t       mutex;
    unsigned short int sequence;   // シーケンス番号
    unsigned short int   length;   // データ長
    char                data[1];   // データ領域
};

struct SharedMemoryCommunicatior::Config{
public:
    Config():
          _config_name()
        , _mq()
        , _attr()
        , _sharedMaxSize(1024)
        , _shmName(std::move("/shared_memory_"))
        , _mqName(std::move("/message_queue_"))
        , _sendFlag(0)
    {}
    std::string    _config_name;
    mqd_t          _mq;
    struct mq_attr _attr;
    unsigned short _sharedMaxSize;
    std::string    _shmName;
    std::string    _mqName;
    int            _isConnected;
    int            _sendFlag;

};
//コンストラクタ
SharedMemoryCommunicatior::SharedMemoryCommunicatior():
      _pConfig(std::make_unique<Config>())
    , _sharedData(nullptr)
{}
//デストラクタ
SharedMemoryCommunicatior::~SharedMemoryCommunicatior(){
    if (_pConfig->_isConnected) {
        Disconnect();
    }
}
//コピーコンストラクタ
SharedMemoryCommunicatior::SharedMemoryCommunicatior(const SharedMemoryCommunicatior&):
      _pConfig(std::make_unique<Config>())
    , _sharedData(nullptr)
{}
//コピー代入演算子
SharedMemoryCommunicatior& SharedMemoryCommunicatior::operator=(const SharedMemoryCommunicatior& rhs){
    *_pConfig = *rhs._pConfig;
    _sharedData = rhs._sharedData;
    return *this;
}
//ムーブコンストラクタ
SharedMemoryCommunicatior::SharedMemoryCommunicatior(SharedMemoryCommunicatior&&) = default;    
//ムーブ代入演算子
SharedMemoryCommunicatior& SharedMemoryCommunicatior::operator=(SharedMemoryCommunicatior&&) = default;

void SharedMemoryCommunicatior::Connect(EndpointInfo& ep) {
    
    _pConfig->_shmName += ep.endpointName;
    _pConfig->_mqName += ep.endpointName;
    std::cout << "shared memory file name:"<< _pConfig->_shmName << std::endl;
    std::cout << "message queue file name:"<<_pConfig->_mqName << std::endl;

    int shm_fd = shm_open(_pConfig->_shmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        ::perror("shm_open");
        exit(1);
    }
    
    if (ep.sendFlag) {
        std::cout << " initialize share memory space." << std::endl;
        _pConfig->_sendFlag = ep.sendFlag;
        // 共有メモリ領域のサイズを設定 一旦、1024バイトに設定
        ftruncate(shm_fd, sizeof(SharedData) + _pConfig->_sharedMaxSize);
        _sharedData = static_cast<SharedData*>(
            mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));

        // Mutex属性の初期化
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

        // 共有メモリ内のmutexの初期化
        pthread_mutex_init(&_sharedData->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        
        // メッセージキューをオープン write only
        //メッセージキューがOpenするとRecv可能になるため、最後にオープンする。
        struct mq_attr mq_attr;
        mq_attr.mq_flags   = 0;   // 通常は0
        mq_attr.mq_maxmsg  = 1;   // キューの最大長を1に設定
        mq_attr.mq_msgsize = 256; // 各メッセージの最大サイズを256バイトに設定
        mq_attr.mq_curmsgs = 0;   // 現在のメッセージ数（設定不要）
        _pConfig->_mq = mq_open(_pConfig->_mqName.c_str(), O_WRONLY | O_CREAT, 0644, &mq_attr);

    } else {
        // 受信側処理 shm領域が確保されたか確認するだけ。
        struct stat sb;
        while (true) {
            if (fstat(shm_fd, &sb) == -1) {
                ::perror("fstat");
                exit(1);
            }

            if (sb.st_size != 0) {
                break;
            }

            std::cout << "Waiting for the writer to initialize the shared memory" << std::endl;
            sleep(1);
        }
        //_sharedData = static_cast<SharedData*>(
        //    mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
        
    }

    close(shm_fd);
    _pConfig->_isConnected = 1;
}

void SharedMemoryCommunicatior::Send(const char* dataset, const int& length) {
    
    struct mq_attr attr;
    while (true)
    {
        if (mq_getattr(_pConfig->_mq, &attr) == -1) {
            std::cerr << "Error getting message queue attributes: " << strerror(errno) << std::endl;
            return; // 属性取得に失敗した場合もフル状態を確認できない
        }
        
        if(attr.mq_curmsgs != attr.mq_maxmsg){
            break;
        } else {
            std::cout << "Message queue is full." << std::endl;
            sleep(1);
        }
    }
    
    // データを共有メモリに書き込む
    std::cout << "Send: " << dataset << std::endl;
    pthread_mutex_lock(&_sharedData->mutex);
    _sharedData->length = length;
    memcpy(_sharedData->data, dataset, length);
    //std::cout << "Send: " << _sharedData->data<< " " << _sharedData->length << std::endl;
    pthread_mutex_unlock(&_sharedData->mutex); 
    mq_send(_pConfig->_mq, "W", strlen("W") + 1, 0);
}

void SharedMemoryCommunicatior::Recv(char*& dataset, int& length, int& timeout_sec) {
    
    // メッセージキューをオープン read only (送信側でリセットする可能性があるためRecvのたびにオープンする)
    // メッセージキューがオープン出来たら送信側の準備は完了していると判断する。
    mqd_t mq = mq_open(_pConfig->_mqName.c_str(), O_RDONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Error opening message queue: " << strerror(errno) << std::endl;
        //エラーに応じた処理
        switch (errno) {
            case EACCES:
                std::cerr << "The queue exists, but the user does not have permission to open it." << std::endl;
                break;
            case EEXIST:
                std::cerr << "Both O_CREAT and O_EXCL were specified to mq_open(), but a queue with this name already exists." << std::endl;
                break;
            case ENOENT:
                std::cerr << "The O_CREAT flag was not specified, and the named queue does not exist." << std::endl;
                break;
            default:
                std::cerr << "An unknown error occurred." << std::endl;
        }
        return;
    }

    // 共有メモリ領域のマッピング。送信側で新しく作り直している場合があるため一旦
    int shm_fd = shm_open(_pConfig->_shmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        ::perror("shm_open");
        return;
    }

    struct stat sb;
    if (fstat(shm_fd, &sb) == -1) {
        ::perror("fstat");
        std::cout << "Disconnected" << std::endl;
        return;
    }

    _sharedData = static_cast<SharedData*>(
        mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    
    close(shm_fd);  // shm_fdはもう必要ないため閉じる
    
    // selectのための準備
    struct mq_attr attr;
    mq_getattr(mq, &attr);
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mq, &readfds);
    
    // 10秒のタイムアウトを設定
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    if (select(mq + 1, &readfds, NULL, NULL, &timeout) > 0) {
        if (FD_ISSET(mq, &readfds)) {
            // メッセージを受け取る
            char buffer[attr.mq_msgsize];
            mq_receive(mq, buffer, attr.mq_msgsize, NULL);
            //std::cout << "Received message queue: " << buffer << std::endl;

            // 共有メモリにアクセスしてデータを読み取る
            pthread_mutex_lock(&_sharedData->mutex);
            //std::cout << "here" << std::endl;
            length = _sharedData->length;
            //std::cout << "Received message length: " << length << std::endl;
            dataset = new char[length];
            memcpy(dataset, _sharedData->data, length);
            pthread_mutex_unlock(&_sharedData->mutex);
        }
    } else{
        ::perror("select");
    }
}

void SharedMemoryCommunicatior::Disconnect() {
    if (_pConfig->_sendFlag) {
        mq_close(_pConfig->_mq);
        pthread_mutex_destroy(&_sharedData->mutex);
        munmap(_sharedData, sizeof(SharedData));
        shm_unlink(_pConfig->_shmName.c_str());
        mq_unlink(_pConfig->_mqName.c_str());
    }
    _pConfig->_isConnected = 0;
}
