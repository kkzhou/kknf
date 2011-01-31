
#include "zxb_netframe.h"

class Server {
public:
    // Server类是一个单体
    static Server* GetServerInstance();

    // 加载/重新加载配置文件，因为不是所有配置项都是可重新加载的，因此要区分。
    int LoadConfig(bool first_time, std::string &config_file);
    // 通过发送信号来触发配置文件的重新加载
    static void LoadConfigSignalHandler(int signo, short events, CallBackArg *arg);

    // 初始化能写到远程日志服务器的logger，这个logger可写到远端，也可写在本地
    int InitLogger(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port);

    // 初始化能把数据上报到远端服务器的的reporter
    int InitReporter(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port);
private:
    // 我用于侦听的ip和端口
    std::vector<std::string> my_listen_ipstr_;
    std::vector<uint16_t> my_listen_port_;

    // 当我主动去连接其他服务器时，使用的ip和端口，一般来说用any就行了
    std::vector<std::string> my_connect_ipstr_;
    std::vector<uint16_t> my_connect_port_;
};
