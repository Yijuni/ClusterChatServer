#ifndef DB_H
#define DB_H

#include <mysql/mysql.h> 
#include <string>


//数据库操作类
class MySQL{
public:
    //初始化数据库
    MySQL();

    //释放数据库连接资源
    ~MySQL();

    //连接数据库
    bool Connect();
    //更新操作
    bool Update(std::string sql);
    //查询操作
    MYSQL_RES *Query(std::string sql);
private:
    MYSQL* conn_m;
};

#endif