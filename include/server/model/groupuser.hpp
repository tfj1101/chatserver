#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"


//群组用户， 复用用户类 User ，新增role信息
class GroupUser : public User
{
public:

    void setRole(string role) { this->role = role; }

    string getRole() { return this->role; }

protected:
    string role;
};

#endif