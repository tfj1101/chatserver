#include"chatserver.hpp"
#include"chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;

//处理服务器ctrl+c结束后的，重置用户状态信息
void resetHandler(int)
{
    ChatService::instance()->resetState();
    exit(0);
}


int main(int argc,char* argv[])
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 9999" << endl;
        exit(-1);
    }
    signal(SIGINT,resetHandler);
    EventLoop loop;
    int port = atoi(argv[2]);
    string address = argv[1];
    
    InetAddress addr(address.c_str(),port);

    ChatServer chat(&loop,addr,"chatserver");

    chat.start();
    loop.loop();

    return 0;
}