#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

class UserModel
{
public:
    // user表的添加用户方法
    bool insert(User &user);

    // 通过id 查找user表，并将对应用户信息保存到user对象中
    User query(int id);

    // 更新用户登录状态操
    bool updateState(User& user);
};

#endif