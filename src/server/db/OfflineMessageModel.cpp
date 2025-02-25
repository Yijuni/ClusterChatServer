#include "OfflineMessageModel.h"
#include "db.h"
bool OfflineMessageMode::Insert(int userid, std::string msg)
{
    char sql[1024];
    sprintf(sql,"insert into OfflineMessage value('%d','%s')",userid,msg.c_str());
    MySQL db;
    if(db.Connect()){
        if(db.Update(sql)){
            return true;
        }
    }   
    return false;
}

bool OfflineMessageMode::Remove(int userid)
{
    char sql[1024];
    sprintf(sql,"delete from OfflineMessage where userid = %d",userid);
    MySQL db;
    if(db.Connect()){
        if(db.Update(sql)){
            return true;
        }
    }   
    return false;
}

std::vector<std::string> OfflineMessageMode::Query(int userid)
{
    char sql[1024];
    sprintf(sql,"select message from OfflineMessage where userid = %d",userid);
    MySQL db;
    std::vector<std::string> vec;
    if(db.Connect()){
        MYSQL_RES *res = db.Query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr){
                vec.push_back(row[0]);
            } 
            mysql_free_result(res);
        }
    }   
    return vec;
}
