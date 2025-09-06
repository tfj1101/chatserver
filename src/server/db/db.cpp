#include "db.h"
#include <muduo/base/Logging.h>

const string server = "localhost";
const int port = 3306;
const string user = "root";
const string passward = "";
const string dbname = "chat";

Mysql::Mysql()
{
    _conn = mysql_init(nullptr);
}

Mysql::~Mysql()
{
    if (_conn)
        mysql_close(_conn);
}

// 获取MySQL连接
MYSQL *Mysql::getConnection()
{
    return _conn;
}

bool Mysql::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), passward.c_str(),dbname.c_str(), 3306, nullptr, 0);

    if (p!=nullptr)
    {
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
        return true;
    }
    else
    {
        LOG_INFO << "connect mysql failed!";

    }
    return false;
}

// 更新操作
bool Mysql::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败！";
        return false;
    }
    return true;
}

// 查询操作
MYSQL_RES *Mysql::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败！";
        return nullptr;
    }

    return mysql_use_result(_conn);
}
