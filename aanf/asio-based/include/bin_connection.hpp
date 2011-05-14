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

#ifndef _BIN_CONNECTION_HPP_
#define _BIN_CONNECTION_HPP_

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include "basic_connection.hpp"


class BinConnection : public BasicConnection {
public:
    virtual void StartRead(const boost::system::error_code &ec) {
        boost::asio::async_read(
            socket(),
            boost::asio::buffer(&((recv_buffer()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleLengthRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };
    virtual void StartWrite(const boost::system::error_code &ec, boost::shared_ptr<MessageInfo> msg) {

        boost::asio::async_write(
            socket(),
            boost::asio::buffer(msg->data()),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };

    virtual void StartConnect(const boost::system::error_code &ec, std::string &to_ip, uint16_t to_port, boost::shared_ptr<MessageInfo> msg) {

        IP_Address to_addr = boost::asio::ip::address::from_string(to_ip);
        TCP_Endpoint to_endpoint(to_addr, to_port);
        socket()->async_connect(to_addr,
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleConnect,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    msg)));
    };

private:
    virtual void HandleConnectThenWrite(const boost::system::error_code &ec, boost::shared_ptr<MessageInfo> msg) {
        set_connected(true);
        boost::asio::async_write(
            socket(),
            boost::asio::buffer(msg->data()),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };

    virtual void HandleConnectThenRead(const boost::system::error_code &ec) {

        boost::asio::async_read(
            socket(),
            boost::asio::buffer(&((recv_buffer()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleLengthRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };

    virtual void HandleWriteThenRead(const boost::system::error_code &ec, std::size_t byte_num) {

        boost::asio::async_read(
            socket(),
            boost::asio::buffer(&((recv_buffer()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleLengthRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };

    // 发送完后放入空闲队列
    virtual void HandleWriteThenIdle(const boost::system::error_code &ec, std::size_t byte_num) {

        UnUse();
    };

    // 发送完后关闭连接
    virtual void HandleWriteThenClose(const boost::system::error_code &ec, std::size_t byte_num) {

        boost::system::error e;
        socket().close(e);
        ConnectionPool::GetConnectionPool()->
    }

    virtual void HandleReadThenRead(const boost::system::error_code &ec, std::size_t byte_num) {

        if (!ec) {
            int len = *(int*)(&((recv_buffer()->at(0))));
            len = boost::asio::detail::socket_ops::network_to_host_long(len);
            if (len > max_recv_buffer_size()) {
                return;
            }

            if (len > recv_buffer().size()) {
                std::vector<char> new_buffer;
                new_buffer.reserve(len);
                copy(recv_buffer()->begin(), recv_buffer()->end(), new_buffer->begin());
                recv_buffer()->swap(new_buffer);
            }
            // 开始读剩余的内容
            boost::asio::async_read(
                socket(),
                boost::asio::buffer(&((recv_buffer()->at(4)), recv_buffer()->size() - 4)),
                boost::asio::transfer_all(),
                strand().wrap(
                    boost::bind(
                        &BinConnection::HandleCompleteRead,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));

        } else {
            // 如果出现错误，不用处理，
            // 此函数退出时，该 connection就会被析构，连接就会关闭。
        }
    };


    // 一个完整的报文被读出后的处理函数
    virtual void HandleReadThenProcess(const boost::system::error_code &ec, std::size_t byte_num) {

        if (!ec) {
            int len = *(int*)(&((recv_buffer()->at(0))));
            len = boost::asio::detail::socket_ops::network_to_host_long(len);
            BOOST_ASSERT(byte_num == len - 4);
            boost::shared_ptr<MessageInfo> new_message(new MessageInfo);

            new_message->set_from_ip(socket()->remote_endpoint().address().to_string());
            new_message->set_from_port(socket()->remote_endpoint().port());
            new_message->set_to_ip(socket()->local_endpoint().address().to_string());
            new_message->set_to_port(socket()->local_endpoint().port());

            new_message->set_arrive_time(boost::posix_time::second_time::localtime());
            new_message->set_len(len);
            new_message->set_data(recv_buffer());

            ProcessResult ret = ProcessMessage(new_message);
            if (ret.size() == 0) {
                // 关闭连接
                boost::system::system_error e;
                socket().close(e);
                return;
            }

            ProcessResult::iterator it, endit;
            it = ret.begin();
            endit = ret.end();

            while (it) {

                boost::shared_ptr<MessageInfo> msg = it->first;
                // 先判断是不是按连接返回的应答
                if (msg->to_ip() == new_message->from_ip() && msg->to_port() == new_message->to_port()) {
                    boost::asio::async_write(
                        socket(),
                        boost::asio::buffer(msg->data()),
                        boost::asio::transfer_all(),
                        strand().wrap(
                        boost::bind(
                            &BinConnection::HandleWrite,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
                } else {
                    // 如果不是，则查找空闲连接，或者创建新连接，并发送

                }
            }
        }
    };


private:

};
#endif
