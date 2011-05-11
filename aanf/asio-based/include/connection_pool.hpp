#ifndef _CONNECTION_POOL_HPP_
#define _CONNECTION_POOL_HPP_

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <vector>

#include "connection_pool.hpp"

namespace AANF {

class BasicConnection;

class ConnectionKey {
public:
    BasicConnection::IP_Address addr_;
    uint16_t port_;
};

class ConnectionPool {
public:

    ConnectionPool* GetConnectionPool() {
        static ConnectionPool *inst = 0;
        if (!inst) {
            inst = new ConnectionPool;
        }
        return inst;
    };

    boost::shared_ptr<BasicConnection> GetIdleConnection(ConnectionKey &key) {

        boost::mutex::scoped_lock locker(mutex_);
        ConnectionListMap::iterator map_it;
        map_it = connection_pool_.find(key);
        if (map_it == connection_pool_.end()) {
            return boost::shared_ptr(0);
        }

        ConnectionList::iterator list_it = map_it->second.begin();
        for (; list_it != map_it->second.end(); list_it++) {
            if (!(*list_it)->in_use() && (*list_it)->connected()) {
                (*list_it)->Use();
                return *list_it;
            }
        }
    };

    void InsertConnection(ConnectionKey &key, boost::shared_ptr<BasicConnection> new_connection) {

        boost::mutex::scoped_lock locker(mutex_);

        ConnectionListMap::iterator it;
        it = connection_pool_.find(key);

        if (it != connection_map_.end()) {
            it->second.push_back(new_connection);
        } else {
            ConnectionList new_list;
            new_list.push_back(new_connection);
            pair<ConnectionListMap::iterator, bool> ret;
            ret = connection_map_.insert(
                    pair<ConnectionKey, ConnectionList> > >(key, new_list) );
            if (!ret.second) {
            }
        }
    };

private:
    boost::mutex mutex_;

    typedef std::map<ConnectionKey,
        std::list<boost::shared_ptr<BasicConnection> > > ConnectionListMap;
    typedef std::list<boost::shared_ptr<BasicConnection> > ConnectionList;

    ConnectionListMap connection_pool_;
private:
    ConnectionPool(){};
    ConnectionPool(ConnectionPool&);
    ConnectionPool& operator=(ConnectionPool&);
};

};
#endif
