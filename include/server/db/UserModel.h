#ifndef USERMODEL_H
#define USERMODEL_H

#include "User.h"

//USER表数据操作类
class UserModel{
public:
    //插入新用户
    bool Insert(User& user);
    //根据id查询用户信息
    User Query(int id);
    //更新用户状态信息
    bool UpdateState(User& user);
    //重置用户状态信息
    void ResetState();
private:
};
#endif