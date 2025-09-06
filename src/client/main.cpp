#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user.hpp"
#include "group.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;

// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;

// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 显示当前请登录用户的基本信息
void showCurrentUserData();

// 接受线程
void readTaskHandler(int clientfd);

// 获取系统时间  给聊天信息添加时间
string getCurrentTime();

// 主聊天页面
void mainMenu(int clientfd);

bool stop;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "commend invaild example: ./chatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "client socket error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(server)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用于接收用户的输入，负责发送数据
    while (1)
    {
        cout << "===============" << endl;
        cout << "1.login" << endl;
        cout << "2.register" << endl;
        cout << "3.quit" << endl;

        int choice;
        cin >> choice;
        cin.get();

        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MEG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();
            size_t total_len = request.length();
            size_t sent_len = 0;

            // 确保完整发送数据
            while (sent_len < total_len)
            {
                int len = send(clientfd, request.c_str() + sent_len, total_len - sent_len, 0);
                if (len == -1)
                {
                    if (errno == EINTR)
                    {
                        continue; // 如果是被信号中断，继续发送
                    }
                    cerr << "send login msg error: " << request << endl;
                    break;
                }
                sent_len += len;
            }

            if (sent_len == total_len)
            {
                char buffer[1024] = {0};
                string response_str;
                bool recv_complete = false;

                // 循环接收完整的响应数据
                while (!recv_complete)
                {
                    int len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1)
                    {
                        if (errno == EINTR)
                        {
                            continue; // 如果是被信号中断，继续接收
                        }
                        cerr << "recv login response error" << endl;
                        break;
                    }
                    else if (len == 0)
                    {
                        cerr << "服务器连接已关闭" << endl;
                        break;
                    }

                    response_str.append(buffer, len);

                    // 尝试解析JSON来验证是否接收完整
                    try
                    {
                        json::parse(response_str);
                        recv_complete = true;
                    }
                    catch (json::parse_error &)
                    {
                        // 如果解析失败，继续接收数据
                        continue;
                    }
                }

                if (recv_complete)
                {
                    try
                    {

                        json response = json::parse(buffer);
                        if (0 != response["errno"].get<int>()) // 登录失败
                        {
                            cerr << response["errmsg"] << endl;
                        }
                        else // 登录成功
                        {
                            g_currentUser.setId(response["id"].get<int>());
                            g_currentUser.setName(response["name"]);
                            g_currentUser.setState("online");

                            // 记录当前用户的好友列表信息
                            if (response.contains("friends"))
                            {
                                g_currentUserFriendList.clear();
                                vector<string> vec1 = response["friends"];
                                for (auto &str : vec1)
                                {
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }
                            }

                            // 记录当前用户的群组列表信息
                            if (response.contains("groups"))
                            {
                                g_currentUserGroupList.clear();
                                vector<string> vec1 = response["groups"];
                                for (auto &str : vec1)
                                {
                                    json js = json::parse(str);
                                    Group group;
                                    group.setId(js["id"].get<int>());
                                    group.setName(js["groupname"]);
                                    group.setDesc(js["groupdesc"]);

                                    vector<string> vec2 = js["users"];
                                    for (auto &str : vec2)
                                    {
                                        GroupUser user;
                                        json js = json::parse(str);
                                        user.setId(js["id"].get<int>());
                                        user.setName(js["name"]);
                                        user.setState(js["state"]);
                                        user.setRole(js["role"]);
                                        group.getGroupUser().push_back(user);
                                    }
                                    g_currentUserGroupList.push_back(group);
                                }
                            }

                            // 显示登录用户的基本信息
                            showCurrentUserData();

                            // 显示当前用户的离线消息 个人聊天信息或群组消息
                            if (response.contains("offlinemsg"))
                            {
                                vector<string> vec = response["offlinemsg"];
                                for (auto &str : vec)
                                {
                                    json js = json::parse(str);
                                    // time + [id] + name + " said: " + xxx
                                    switch (js["msgid"].get<int>())
                                    {
                                    case ONE_CHAT_MSG:
                                    {
                                        cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"].get<string>() << endl;
                                    }
                                    case GROUP_CHAT_MSG:
                                    {
                                        cout << "群消息[" << js["groupid"] << "]: " << js["time"] << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"].get<string>() << endl;
                                    }
                                    break;
                                    }
                                }
                            }

                            // 登录成功，启动接受线程负责接受数据 线程只启动一次
                            static int readthreadnumber = 0;
                            if (readthreadnumber == 0)
                            {
                                thread readtask(readTaskHandler, clientfd);
                                readtask.detach();
                                readthreadnumber++;
                            }

                            stop = false;
                            // 进入聊天主界面
                            mainMenu(clientfd);
                        }
                    }
                    catch (const json::parse_error &e)
                    {
                        // json解析错误
                        cout << "json parser:" << e.what();
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "password:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (-1 == len)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["errno"].get<int>()) // 注册失败
                    {
                        cerr << responsejs["errmsg"] << endl;
                    }
                    else
                    {
                        cout << name << "register success, userid is " << responsejs["id"] << ", do not forget it!" << endl;
                    }
                }
            }
        }
        break;
        case 3:
        {
            close(clientfd);
            exit(-1);
        }
        break;
        default:
        {
            cerr << "invaild input!" << endl;
            break;
        }
        }
    }
    return 0;
}

// 显示当前请登录用户的基本信息
void showCurrentUserData()
{
    cout << " ===================login user====================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;

    cout << "====================friend list======================" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (auto user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }

    cout << "=====================group list======================" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (auto group : g_currentUserGroupList)
        {
            cout << "groupid: " << group.getId() << " 群名:" << group.getName() << " 群描述:" << group.getDesc() << endl;
            for (auto user : group.getGroupUser())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    else
    {
        cout << "群组信息为空" << endl;
    }
    cout << "============================================" << endl;
}

// 获取系统时间  给聊天信息添加时间
string getCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char data[60] = {0};
    sprintf(data, "%d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return string(data);
}

// 接受线程
void readTaskHandler(int clientfd)
{
    while (1)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len <= 0)
        {
            close(clientfd);
            exit(-1);
        }

        // 接受chatserver转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        switch (js["msgid"].get<int>())
        {
        case ONE_CHAT_MSG:
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        case GROUP_CHAT_MSG:
        {
            cout << "群消息[" << js["groupid"] << "]: " << js["time"] << " [" << js["id"] << "]" << js["name"] << " said: " << js["msg"].get<string>() << endl;
        }
        break;
        }
    }
}

// 帮助
void help(int fd = -1, string cmd = "");
// 一对一聊天
void chat(int, string content);
// 添加好友
void addfriend(int, string cmd);
// 创建群组
void creategroup(int, string cmd);
// 加入群聊
void addgroup(int, string cmd);
// 群聊
void groupchat(int, string cmd);
// 注销登录
void quit(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令格式"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群聊，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"quit", "注销登录，格式quit"},
};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"quit", quit},
};

// 主聊天页面
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (!stop)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令字段
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用对应命令的事件处理回调
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

void help(int, string)
{
    cout << "show command list" << endl;
    for (auto m : commandMap)
    {
        cout << m.first << " : " << m.second << endl;
    }
    cout << endl;
}

// 一对一聊天 friendid:message
void chat(int clientfd, string content)
{
    int idx = content.find(":"); // friendid:content
    if (-1 == idx)
    {
        cerr << "chat command invalid" << endl;
        return;
    }
    int friendid = atoi(content.substr(0, idx).c_str());
    string message = content.substr(idx + 1, content.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    // 后续可以完善   确保数据完整发送
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error ->" << message << endl;
    }
}
// 添加好友 addfriend:friendid
void addfriend(int clientfd, string cmd)
{
    int friendid = atoi(cmd.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["friendid"] = friendid;
    js["id"] = g_currentUser.getId();

    string msg = js.dump();

    int len = send(clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);
    if (len < 0)
    {
        cerr << "send addfriend msg error ->" << msg << endl;
    }
}
// 创建群组  groupname:groupdesc
void creategroup(int clientfd, string cmd)
{
    //  cmd  =  groupname:groupdesc
    int idx = cmd.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid" << endl;
        return;
    }
    string groupname = cmd.substr(0, idx);

    string groupdesc = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string msg = js.dump();

    int len = send(clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);
    if (len < 0)
    {
        cerr << "send create group msg error ->" << msg << endl;
    }
}
// 加入群聊  addgroup:groupid
void addgroup(int clientfd, string cmd)
{
    int groupid = atoi(cmd.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MEG;
    js["groupid"] = groupid;
    js["id"] = g_currentUser.getId();

    string msg = js.dump();

    int len = send(clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);
    if (len < 0)
    {
        cerr << "send addfriend msg error ->" << msg << endl;
    }
}
// 群聊  groupchat:groupid:message
void groupchat(int clientfd, string cmd)
{
    int idx = cmd.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid" << endl;
        return;
    }
    int groupid = atoi(cmd.substr(0, idx).c_str());
    string message = cmd.substr(idx + 1, cmd.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["groupname"] =
        js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send groupchat msg error ->" << buffer << endl;
    }
}
// 注销登录 quit
void quit(int clientfd, string)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();

    string msg = js.dump();

    int len = send(clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);
    if (len < 0)
    {
        cerr << "send logout msg error ->" << msg << endl;
    }
    else
    {
        stop = true;
    }
}
