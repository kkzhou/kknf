#ifndef __EXAMPLE_MESSAGE_HPP__
#define __EXAMPLE_MESSAGE_HPP__

#pragma pack(1)
struct BFReq {
    int sid;
    int user_id;
};
struct BFRsp {
    int sid;
    int user_id;
    int user_salary;
    int user_age;
};

struct BBReq1 {
    int user_id;
};
struct BBRsp1 {
    int user_id;
    int user_salary;
};

struct BBReq2 {
    int user_id;
};
struct BBRsp2 {
    int user_id;
    int user_age;
};

#pragma pack(0)

#endif
