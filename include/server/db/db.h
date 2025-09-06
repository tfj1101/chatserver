#ifndef DB_H
#define DB_H

#include<mysql/mysql.h>
#include<string>
using namespace std;



class Mysql
{
public:
    Mysql();

    ~Mysql();

    //连接数据库
    bool connect();

    //查询操作
    MYSQL_RES* query(string sql);

    //更新操作
    bool update(string sql);

    //获取MySQL连接
    MYSQL* getConnection();

private:

    MYSQL* _conn;
};


#endif