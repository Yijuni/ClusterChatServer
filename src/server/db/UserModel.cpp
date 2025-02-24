#include "UserModel.h"
#include "db.h"
#include <iostream>
bool UserModel::Insert(User &user)
{
    //组装sql语句
    char sql[1014];
    sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s')",
        user.GetName().c_str(),user.GetPassword().c_str(),user.GetState().c_str()); 
    MySQL mysql;
    if(mysql.Connect()){
        if(mysql.Update(sql)){
            //获取给插入成功的用户生成的id，给到用户作为用户号
            user.SetId(mysql_insert_id(mysql.GetConnection()));
            return true;
        }
    }
    
    return false;
}

User UserModel::Query(int id)
{
    //组装sql语句
    char sql[1014];
    //根据用户号查询用户信息
    sprintf(sql,"select * from User where id = %d",id);
    MySQL mysql;
    if(mysql.Connect()){
        MYSQL_RES *res = mysql.Query(sql);
        if(res!=nullptr){
            MYSQL_ROW row =  mysql_fetch_row(res);
            if(row !=nullptr){
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                user.SetPassword(row[3]);
                
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}

bool UserModel::UpdateState(User &user)
{
    char sql[1024];
    sprintf(sql,"update User set state = '%s' where id = %d",user.GetState().c_str(),user.GetId());
    MySQL mysql;
    if(mysql.Connect()){
        if(mysql.Update(sql)){
            return true;
        }
    }
    return false;
}
