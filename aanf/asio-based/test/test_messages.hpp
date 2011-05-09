#ifndef _TEST_MESSAGES_HPP_
#define _TEST_MESSAGES_HPP_

#define CMD_GET_USER_INFO 0x00000001
#define CMD_GET_USER_GENDER 0x00000002
#define CMD_GET_USER_AGE 0x00000003

#pragma pack(1)
class BFReq {
public:
    int user_id_;
    int seq_;
    int cmd_;
};
class BFRsp {
public:
    int user_id_;
    int seq_;
    int cmd_;
    int gender_;
    int age_;
};
class BB1Req {
public:
    int user_id_;
    int seq_;
    int cmd_;
};
class BB1Rsp {
public:
    int user_id_;
    int seq_;
    int cmd_;
    int gender_;
};
class BB2Req {
public:
    int user_id_;
    int seq_;
    int cmd_;
};
class BB2Rsp {
public:
    int user_id_;
    int seq_;
    int cmd_;
    int age_;
};
#pragma pack(0)
#endif
