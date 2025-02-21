#include "db.h"
#include <muduo/base/Logging.h>

//数据库信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "moyoj@123456";
static std::string dbname = "chat";

MySQL::MySQL()
{
    conn_m = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if(conn_m != nullptr){
        mysql_close(conn_m);
    }
}

bool MySQL::Connect()
{
    MYSQL *p = mysql_real_connect(conn_m,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
    if(p!=nullptr){
        //C和C++代码默认的编码字符ASCII,如果不设置，MYSQL获取的中文信息会乱码
        mysql_query(conn_m,"set name gbk");
    }
    return p;
}

bool MySQL::Update(std::string sql)
{
    if(mysql_query(conn_m,sql.c_str())){
        LOG_INFO << __FILE__ <<":"<<__LINE__<<":"<<sql<<"更新失败!";
        return false;
    }
    return true;
}

MYSQL_RES *MySQL::Query(std::string sql)
{
    if(mysql_query(conn_m,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败!";
        return nullptr;
    }
    return mysql_use_result(conn_m);
}
