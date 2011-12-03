 /*
    Copyright (C) <2011>  <ZHOU Xiaobo(zhxb.ustc@gmail.com)>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

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
                    pair<ConnectionKey, ConnectionList> > >(key, new_list));
            if (!ret.second) {
            }
        }
    };

    void DeleteConnection(ConnectionKey &key, boost::shared_ptr<BasicConnection> exact_connection) {

        boost::mutex::scoped_lock locker(mutex_);

        ConnectionListMap::iterator it;
        it = connection_pool_.find(key);

        if (it != connection_map_.end()) {
            return;
        } else {
            ConnectionList::iterator list_it, list_endit;
            list_it = it->second.begin();
            list_endit = it->second.end();
            boost::shared_ptr<BasicConnection> ret = std::find(list_it, list_endit, exact_connection);
            if (ret) {
                it->second.erase(exact_connection);
            }
            exact_connection->socket().close();
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
