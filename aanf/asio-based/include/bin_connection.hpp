#ifndef _BIN_CONNECTION_HPP_
#define _BIN_CONNECTION_HPP_

#include <boost/asio/detail/socket_ops.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include "basic_connection.hpp"


class BinConnection : public BasicConnection {
public:
    void Start() {
        boost::asio::async_read(
            socket(),
            boost::asio::buffer(&((recv_buffer()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &BinConnection::HandleLengthRead,
                    shared_from_this(),
                    boost::asio::placeholders::error_code,
                    boost::asio::placeholders::bytes_transferred)));
    };

    // 长度域被读出后的处理函数
    void HandleLengthRead(const boost::system::error_code &ec, std::size_t byte_num) {

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
                        boost::asio::placeholders::error_code,
                        boost::asio::placeholders::bytes_transferred)));

        } else {
            // 如果出现错误，不用处理，
            // 此函数退出时，该 connection就会被析构，连接就会关闭。
        }
    };

    // 一个完整的报文被读出后的处理函数
    void HandleCompleteRead(const boost::system::error_code &ec, std::size_t byte_num) {

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

            int ret = 1;
            while ((ret = ProcessMessage(new_message)) == 1) {
            }
            if( ret == -1) {

            } else if( ret == 0) {
            } else {
                BOOST_ASSERT(false);
            }
        }
    };

    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // return value:
    // 0: OK
    // -1: Fail
    // 1: Process again
    int ProcessMessage(boost::shared_ptr<MessageInfo> msg) = 0;
private:

};
#endif
