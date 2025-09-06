#include "usermodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>
#include <iostream>
#include <stdio.h>
using namespace std;

// user表的添加用户方法
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    Mysql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入的用户数据的主键 user id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

// 通过id 查找user表，并将对应用户信息保存到user对象中
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    Mysql mysql;
    if (mysql.connect())
    {
        auto res = mysql.query(sql);
        if (res != nullptr)
        {
            auto row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

// 更新用户登录状态操
bool UserModel::updateState(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(),user.getId());
    Mysql mysql;
    if (mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}