#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MEG = 1,  // 登录msg
    REG_MSG,        // 注册msg
    REG_MSG_ACK,    // 注册响应消息
    LOGIN_MSG_ACK,  // 登录响应消息
    ONE_CHAT_MSG,   // 一对一聊天
    ADD_FRIEND_MSG, // 添加好友

    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MEG,    // 加入群组
    GROUP_CHAT_MSG,   // 群聊天
    LOGOUT_MSG,       // 注销消息
};

#endif