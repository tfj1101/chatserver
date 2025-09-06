#include <muduo/base/Logging.h>
#include <string>
#include <vector>
#include <map>

#include "db.h"
#include "chatservice.hpp"

using namespace std;

// 单例对象
ChatService *ChatService::instance()
{
    static ChatService instance;
    return &instance;
}

// 注册消息以及对应的回调函数
ChatService::ChatService()
{
    // 注册登录回调
    _msgHandlerMap.insert({LOGIN_MEG, std::bind(&ChatService::login, this, _1, _2, _3)});

    // 注销业务回调
    _msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    // 注册用户回调
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});

    // 一对一聊天回调
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});

    // 添加好友回调
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 创建群组回调
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});

    // 加入群组回调
    _msgHandlerMap.insert({ADD_GROUP_MEG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});

    // 群组聊天
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2, _3)});

    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage, this, _1, _2));
    }
}

// 处理redis上报消息
void ChatService::handlerRedisSubscribeMessage(int id, string message)
{
    {
        lock_guard<mutex> lock(_mtx);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(message);
            return;
        }
    }

    //存储离线消息
    _offlineMsgModel.insert(id,message);
}

// 获取消息处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志,没有对应回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
        return _msgHandlerMap[msgid];
}

// 处理注销登录业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];

    {
        lock_guard<mutex> lock(_mtx);
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 从_userConnMap表中移除
            _userConnMap.erase(it);
        }
    }

    // 用户注销，在redis上取消订阅通道
    _redis.unsubscribe(id);

    // 修改用户状态
    User user;
    user.setId(id);
    user.setState("offline");
    _userModel.updateState(user);
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online") // 该用户已经登录，不允许重复登录
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请输入新的账号登录";
            conn->send(response.dump());
        }
        else // 登录成功
        {
            // 添加在线用户连接信息
            {
                lock_guard<mutex> lock(_mtx);
                _userConnMap.insert({id, conn});
            }

            // 在redis上订阅 通道 id
            _redis.subscribe(id);

            // 更新用户状态信息
            user.setState("online");
            if (_userModel.updateState(user))
            {
                LOG_INFO << "更新成功";
            }

            // 响应报文
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getId();
            response["errno"] = 0;
            response["name"] = user.getName();
            response["state"] = user.getState();

            // 检查是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {

                response["offlinemsg"] = vec;

                // 删除离线消息
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> friendVec = _friendModel.queryFriend(id);
            if (!friendVec.empty())
            {
                vector<string> friends;
                for (auto &user : friendVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    friends.push_back(js.dump());
                }

                response["friends"] = friends;
            }

            // 查询该用户的群组信息并返回  ["groups"]
            vector<Group> groupVec = _groupModel.queryGroups(id);
            if (!groupVec.empty())
            {
                vector<string> groups;
                for (auto &g : groupVec)
                {
                    json js;
                    js["id"] = g.getId();
                    js["groupname"] = g.getName();
                    js["groupdesc"] = g.getDesc();

                    vector<string> users;
                    for (auto &u : g.getGroupUser())
                    {
                        json jsgroupuser;
                        jsgroupuser["id"] = u.getId();
                        jsgroupuser["name"] = u.getName();
                        jsgroupuser["state"] = u.getState();
                        jsgroupuser["role"] = u.getRole();
                        users.push_back(jsgroupuser.dump());
                    }
                    js["users"] = users;
                    groups.push_back(js.dump());
                }
                response["groups"] = groups;
            }

            conn->send(response.dump());

            LOG_INFO << "登录成功！";
        }
    }
    else
    {
        // 登陆失败 用户不存在  或用户存在但密码错误
        LOG_INFO << "登录失败！";
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["id"] = user.getId();
        response["errno"] = 1;
        response["errmsg"] = "用户名不存在或密码错误";
        conn->send(response.dump());
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // 将json数据反序列化到user类中
    User user;
    string name = js["name"];
    string pwd = js["password"];

    user.setName(name);
    user.setPwd(pwd);

    auto state = _userModel.insert(user);
    if (state) // 注册成功
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else // 注册失败
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }

    // 数据库
}

// 客户端异常关闭
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_mtx);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);

                // 从_userConnMap表中移除
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户注销，取消在redis中的订阅通道
    _redis.unsubscribe(user.getId());

    // 修改用户状态
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 处理一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_mtx);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) // 在线 且在同一台服务器上
        {
            // toid在线 ，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 服务器重置用户状态信息
void ChatService::resetState()
{
    char sql[] = "update user set state = 'offline' where state = 'online' ";
    Mysql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息 到数据库
    _friendModel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    auto id = js["id"];
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 创建群组类
    Group group(-1, name, desc);

    // 添加到数据库
    if (_groupModel.createGroup(group))
    {
        // 创建群组创始人的信息
        _groupModel.addGroup(id, group.getId(), "creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天
void ChatService::chatGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    int groupid = js["groupid"];
    vector<int> useridVec = _groupModel.queryGroupUsers(id, groupid);
    lock_guard<mutex> lock(_mtx);
    for (int id : useridVec)
    {

        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end()) // 在线  转发群组消息 在同一台服务器
        {
            it->second->send(js.dump());
        }
        else // 不在线  存储离线消息
        {
            // 查询toid是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }

            _offlineMsgModel.insert(id, js.dump());
        }
    }
}