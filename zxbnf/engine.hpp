#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

namespace ZXBNF {
    class Engine {
    public:
	int Run();
	int AddListener(AsyncTCPListenSocket*);
    private:
    };
};

#endif
