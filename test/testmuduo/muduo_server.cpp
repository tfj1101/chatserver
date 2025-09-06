#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <string>
#include <functional>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器先注册用户连接回调函数和断开回调函数
        _server.setConnectionCallback(std::bind(&ChatServer::onconnection, this, _1));

        // 给服务器先注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 线程数量
        _server.setThreadNum(4);
    }

    void start()
    {
        _server.start();
    }

private:
    // 处理连接和断开
    void onconnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << " --> " << conn->localAddress().toIpPort() << "start online " << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << " --> " << conn->localAddress().toIpPort() << "start offline " << endl;
            conn->shutdown();
        }
    }

    // 处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data: " << buf << " ,time: " << time.toString() << endl;
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr("192.168.155.140", 6000);

    ChatServer chat(&loop, listenAddr, "chatserver");
    chat.start();
    loop.loop();

    return 0;
}
