#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include "json.hpp"
#include "Public.h"

using namespace muduo::net;
using namespace muduo;
using json = nlohmann::json;

//消息处理回调函数类型
using MessageHandler = std::function<void(const TcpConnectionPtr& conn,json& js,Timestamp)>;

//服务器服务类
class ChatService{
public:
    //获取单例对象
    static ChatService& GetInstance();
    //登录业务
    void Login(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //注册业务
    void Register(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //获取不同消息对应的处理函数
    MessageHandler GetMsgHandler(int type);

    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) =delete;
private:
    ChatService();

    //存储消息ID和对应处理函数
    std::unordered_map<int,MessageHandler> MsgHandlerMap_m;

};

#endif