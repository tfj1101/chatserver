#ifndef CHATSERVER_H
#define CHARSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenaddr, const string &nameArg);

    // 启动服务
    void start();

private:
    //连接和断开处理函数
    void onConnection(const TcpConnectionPtr &conn);
    //消息处理函数
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);

    TcpServer server_;
    EventLoop *loop_;
};

#endif