#ifndef PUBLIC_H
#define PUBLIC_H
//Server和Client的公用文件
enum MsgType{
    LOGIN_MSG = 1,//登录消息
    LOGIN_MSG_ACK, //登录响应消息
    LOGIN_OUT_MSG,//退出消息
    REG_MSG,//注册消息
    REG_MSG_ACK, //注册响应消息
    ONT_CHAT_MSG, //一对一聊天消息
    ADD_FRIEND_MSG, //添加好友

    CREATE_GROUP_MSG,//创建群组
    ADD_GROUP_MSG,//加入群聊
    GROUP_CHAT_MAG,//群聊信息

    
};
#endif