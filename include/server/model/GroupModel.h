#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "Group.h"
#include "db.h"
class GroupModel{
public:
    //创建群组
    bool CreateGroup(Group& group);
    //加入群组
    bool AddGroup(int userid,int groupid,std::string role);
    //查询用户所在群组信息
    std::vector<Group> QueryGroup(int userid);
    //根据groupid查询群组用户id列表，用于群发消息，获得userid对应的TcpConnectionPtr发送消息
    std::vector<int> QueryGroupUsers(int userid,int groupid); 
private:
    
};
#endif