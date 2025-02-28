#include "FriendModel.h"

bool FriendModel::Insert(int userid, int friendid)
{
    char sql[1024];
    sprintf(sql,"insert into Friend value(%d,%d)",userid,friendid);
    MySQL db;
    if(db.Connect()){
        if(db.Update(sql)){
            return true;
        }
    }
    return false;
}

std::vector<User> FriendModel::Query(int userid)
{
    char sql[1024];
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d;",userid);
    std::vector<User> userlist;
    MySQL db;
    if(db.Connect()){
        MYSQL_RES *res;
        if((res = db.Query(sql))!=nullptr){
            MYSQL_ROW row;
            while(row=mysql_fetch_row(res)){
                userlist.emplace_back(atoi(row[0]),row[1],"",row[2]);
            }
            mysql_free_result(res);
        }
    }
    return userlist;
}
