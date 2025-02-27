#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "User.h"
#include "db.h"
//好友信息表接口
class FriendModel{
public:
    //添加好友
    bool Insert(int userid,int friendid);
    //返回用户好友列表
    std::vector<User> Query(int userid);
private:
};
#endif