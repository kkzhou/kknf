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

#ifndef _TEST_MESSAGES_HPP_
#define _TEST_MESSAGES_HPP_

#define CMD_GET_USER_INFO 0x00000001
#define CMD_GET_USER_GENDER 0x00000002
#define CMD_GET_USER_AGE 0x00000003

#define TYPE_BF_REQ 0x00000001
#define TYPE_BF_RSP 0x00000002
#define TYPE_BB1_REQ 0x00000003
#define TYPE_BB1_RSP 0x00000004
#define TYPE_BB2_REQ 0x00000005
#define TYPE_BB2_RSP 0x00000006

//报文格式，简单起见，就不序列化反序列化了，直接内存拷贝。
#pragma pack(1)
class Packet {
public:
    int type_;
};
class BFReq : public Packet {
public:
    int user_id_;
    int seq_;
    int cmd_;
};
class BFRsp : public Packet {
public:
    int user_id_;
    int seq_;
    int cmd_;
    int gender_;
    int age_;
};
class BB1Req : public Packet {
public:
    int user_id_;
    int req_index_;
    int seq_;
    int cmd_;
};
class BB1Rsp : public Packet {
public:
    int user_id_;
    int req_index_;
    int seq_;
    int cmd_;
    int gender_;
};
class BB2Req : public Packet {
public:
    int user_id_;
    int req_index_;
    int seq_;
    int cmd_;
};
class BB2Rsp : public Packet {
public:
    int user_id_;
    int req_index_;
    int seq_;
    int cmd_;
    int age_;
};
#pragma pack(0)

#endif
