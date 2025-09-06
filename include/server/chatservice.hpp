#ifndef CHATSERVICE_H
#define CHARSERVICE_H

#include <unordered_map>
#include <functional>
#include <mutex>
#include "muduo/net/TcpConnection.h"
#include "json.hpp"
#include "public.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

// 业务类
class ChatService
{
public:
    // 单例对象
    static ChatService *instance();

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注销登录业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组聊天
    void chatGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 获取消息处理器
    MsgHandler getHandler(int msgid);

    // 客户端异常关闭
    void clientCloseException(const TcpConnectionPtr &conn);

    // 服务器重置用户状态信息
    void resetState();

    //处理redis上报消息
    void handlerRedisSubscribeMessage(int id,string message);

private:
    ChatService();

    // 处理消息id和对应的事件业务映射表
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的连接  线程安全问题
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // 互斥锁 保证_userConnMap的线程安全
    mutex _mtx;

    // redis对象
    Redis _redis;
};

#endif