#include "friendmodel.hpp"
#include "db.h"
// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    char sql[512] = {0};
    sprintf(sql, "insert into friend values(%d,%d)",
            userid, friendid);

    Mysql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表
vector<User> FriendModel::queryFriend(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select u.id, u.name, u.state from user u inner join friend f on f.friendid = u.id where f.userid = %d ",
            userid);

    Mysql mysql;
    vector<User> friendlist;
    if (mysql.connect())
    {

        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friendlist.push_back(user);
            }
            mysql_free_result(result);
            return friendlist;
        }
    }
    return friendlist;
}