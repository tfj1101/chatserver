#include "groupmodel.hpp"
#include "db.h"
#include <iostream>

using namespace std;

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[512] = {0};
    sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());

    Mysql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 添加群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[512] = {0};
    sprintf(sql, "insert into groupuser values(%d,%d,'%s')",
            groupid, userid, role.c_str());

    Mysql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[512] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a,groupuser b where b.groupid = a.id and b.userid = %d",
            userid);

    Mysql mysql;
    vector<Group> groupVec = {};
    if (mysql.connect())
    {
        auto result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(result);
        }

        // 查询群组的用户信息
        for (auto &group : groupVec)
        {
            sprintf(sql, "select a.id,a.name,a.state from user a inner join groupuser b on b.userid = a.id where b.groupid = %d",
                    group.getId());
            MYSQL_RES *result = mysql.query(sql);

            if (result != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result)) != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    group.getGroupUser().push_back(user);
                }
                mysql_free_result(result);
            }
        }
    }
    return groupVec;
}

// 根据指定groupid查询群组用户id列表，除了用户userid自己，主要是用户群聊业务，给群组其他成员发送群消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[512] = {0};
    sprintf(sql, "select b.userid from groupuser b where b.groupid = %d and b.userid != %d ", groupid, userid);

    Mysql mysql;
    vector<int> idVec = {};
    if (mysql.connect())
    {
        auto result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(result);
        }
    }
    return idVec;
}