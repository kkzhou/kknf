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

#ifndef __TEST_PACKET_HPP__
#define __TEST_PACKET_HPP__

#pragma pack(1)

class ReqToBF {
public:
    int l_;
    int seq_;
    int a_;
    int b_;
};
class RspFromBF {
public:
    int l_;
    int seq_;
    int a_;
    int sum_;
};
class ReqToBB1 {
public:
    int l_;
    int seq_;
    int a_;
};
class RspFromBB1 {
public:
    int l_;
    int seq_;
    int another_a_;
};
class ReqToBB2 {
public:
    int l_;
    int seq_;
    int b_;
};
class RspFromBB2 {
public:
    int l_;
    int seq_;
    int another_b_;
};
#pragma pack(0)

#endif
