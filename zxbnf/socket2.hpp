#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

namespace ZXBNF {
    class AsyncSocket {
    public:
    private:
    };

    class AsyncTCPSocket : public AsyncSocket {
    };

    class AsyncTCPDataSocket : public AsyncTCPSocket {
    };

    class AsyncTCPServerSocket : public AsyncTCPDataSocket {
    };

    class AsyncTCPClientSocket : public AsyncTCPDataSocket {
    };
    
    class AsyncUDPSocket : public AsyncSocket {
    };

    
};

#endif
