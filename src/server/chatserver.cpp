#include "chatserver.hpp"
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
#include "muduo/base/Logging.h"
#include <iostream>
using json = nlohmann::json;
using namespace muduo;
using namespace std;
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenaddr, const string &nameArg)
    : server_(loop, listenaddr, nameArg), loop_(loop)
{

    // 注册用户连接和断开回调函数
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调函数
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 线程数量
    server_.setThreadNum(2);
}

// 启动服务
void ChatServer::start()
{
    server_.start();
}

//
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}


void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();

    // 打印接收到的消息内容，方便调试
    cout << "收到消息: [" << buf << "]" << endl;

    // 检查消息是否为空
    if (buf.empty())
    {
        LOG_ERROR << "收到空消息";
        return;
    }

    // 数据反序列化
    try
    {
        json js = json::parse(buf);
        
        // 检查消息格式
        if (!js.contains("msgid"))
        {
            LOG_ERROR << "消息格式错误: 缺少msgid字段";
            return;
        }

        // 获取消息处理器
        auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
        
        // 检查处理器是否存在
        if (msgHandler)
        {
            // 处理消息
            msgHandler(conn, js, time);
        }
        else
        {
            LOG_ERROR << "未知的消息类型: msgid = " << js["msgid"].get<int>();
        }
    }
    catch (const json::parse_error &e)
    {
        // JSON解析错误
        LOG_ERROR << "JSON解析错误: " << e.what() << "\n收到的数据: [" << buf << "]";
        if (conn && conn->connected())
        {
            conn->send("消息格式错误，请检查JSON格式\n");
        }
    }
    catch (const json::exception &e)
    {
        // JSON其他错误
        LOG_ERROR << "JSON错误: " << e.what();
        if (conn && conn->connected())
        {
            conn->send("消息处理失败，请重试\n");
        }
    }
    catch (const std::exception &e)
    {
        // 其他未预期的错误
        LOG_ERROR << "处理消息时发生错误: " << e.what();
    }
}
