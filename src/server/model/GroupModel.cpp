#include "GroupModel.h"

bool GroupModel::CreateGroup(Group &group)
{
    char sql[1024];
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",group.GetName().c_str(),group.GetDesc().c_str());
    MySQL db;
    if(db.Connect()){
        if(db.Update(sql)){
            group.SetId(mysql_insert_id(db.GetConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::AddGroup(int userid, int groupid, std::string role)
{
    char sql[1024];
    sprintf(sql,"insert into GroupUser values(%d,%d,'%s')",groupid,userid,role.c_str());
    MySQL db;
    if(db.Connect()){
        if(db.Update(sql)){
            return true;
        }
    }
    return false;
}

std::vector<Group> GroupModel::QueryGroup(int userid)
{
    char sql[1024];
    sprintf(sql,"select a.ID,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.ID=b.groupid where b.userid=%d",userid);
    MySQL db;
    std::vector<Group> vec;
    if(db.Connect()){
        MYSQL_RES *res = db.Query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                Group group(atoi(row[0]),row[1],row[2]);
                vec.emplace_back(group);
            }
            mysql_free_result(res);
        }
    }
    //查询群组用户信息
    for(Group& group : vec){
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser on a.id=b.userid where b.groupid=%d",group.GetId());
        MYSQL_RES* res = db.Query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                GroupUser groupuser;
                groupuser.SetId(atoi(row[0]));
                groupuser.SetName(row[1]);
                groupuser.SetState(row[2]);
                groupuser.SetRole(row[3]);
                group.GetUsers().emplace_back(groupuser);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}

std::vector<int> GroupModel::QueryGroupUsers(int userid, int groupid)
{
    char sql[1024];
    sprintf(sql,"select userid from GroupUser where groupid=%d and userid!=%d",groupid,userid);
    MySQL db;
    std::vector<int> vec;
    if(db.Connect()){
        MYSQL_RES *res = db.Query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
