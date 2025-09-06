#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include<string>
#include<vector>
using namespace std;

//处理离线消息接口
class OfflineMsgModel
{
public:
    //存储用户的离线消息
    void insert(int userid,string msg);

    //删除用户的离线消息
    void remove(int userid);

    //查询用户的离线消息
    vector<string> query(int userid);

    //存储离线消息
    vector<string> _Msg;

};

#endif