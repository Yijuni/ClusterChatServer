#include "ChatService.h"

ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

void ChatService::Login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO<<"do login service!!";
}

void ChatService::Register(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO<<"do register service!!";
}


MessageHandler ChatService::GetMsgHandler(int type)
{
    //没找到对应的消息处理函数,返回一个默认的处理函数
    if(MsgHandlerMap_m.find(type)==MsgHandlerMap_m.end()){
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_ERROR<<"msgid : ["<<type<<"] can't find handler!";
        }; 
    }
    return MsgHandlerMap_m[type];
}

ChatService::ChatService()
{
    //注册消息以及对应的回调函数
    MsgHandlerMap_m.insert({MsgType::LOGIN_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::REG_MSG,std::bind(&ChatService::Register,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
}
