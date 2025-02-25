#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
//离线消息表操作接口
class OfflineMessageMode{
public:
    //存储用户离线消息
    bool Insert(int userid,std::string msg);
    //删除用户离线消息
    bool Remove(int userid);
    //查询用户离线消息
    std::vector<std::string> Query(int userid);
};
#endif