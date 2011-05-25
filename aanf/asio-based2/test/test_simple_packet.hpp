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

#pragma pack(4)
class TestPacketBase {
public:
    enum PacketType {
        T_REQ_TO_BF = 1,
        T_RSP_FROM_BF,
        T_REQ_TO_BB1,
        T_RSP_FROM_BB1,
        T_REQ_TO_BB2,
        T_RSP_FROM_BB2
    };
public:
    int len_;
    PacketType type_;
    int seq_;
};

class ReqToBF : public TestPacketBase {
public:
    int a_;
    int b_;
};
class RspFromBF : public TestPacketBase {
public:
    int sum_;
    int error_;
};
class ReqToBB1 : public TestPacketBase {
public:
    int a_;
};
class RspFromBB1 : public TestPacketBase {
public:
    int another_a_;
};
class ReqToBB2 : public TestPacketBase {
public:
    int b_;
};
class RspFromBB2 : public TestPacketBase {
public:
    int another_b_;
};
#pragma pack(0)
