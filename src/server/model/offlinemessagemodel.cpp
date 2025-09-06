#include "offlinemessagemodel.hpp"
#include "db.h"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlineMessage values(%d,'%s')",
            userid, msg.c_str());

    Mysql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlineMessage where userid = %d ",
            userid);

    Mysql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlineMessage where userid = %d ",
            userid);

    Mysql mysql;
    vector<string> vec;
    if (mysql.connect())
    {

        auto result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(result);
        }
    }
    return vec;
}