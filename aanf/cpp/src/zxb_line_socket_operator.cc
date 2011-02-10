
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

#include "zxb_line_socket_operator.h"

namespace ZXB{

LineSocketOperator::LineSocketOperator() : socket_(0) {
}

LineSocketOperator::~LineSocketOperator() {
}

int LineSocketOperator::ReadHandler(Packet *&in_pack) {

}
int LineSocketOperator::WriteHandler() {

}

}; // namespace ZXB
