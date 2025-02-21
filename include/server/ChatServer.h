#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include "json.hpp"
#include "ChatService.h"
#include "Public.h"

using namespace muduo::net;
using namespace muduo;
using json = nlohmann::json;

class ChatServer{
public:
    //聊天服务器初始化
    ChatServer(EventLoop* loop,const InetAddress& listenaddr,const string& name);
    //启动服务
    void Start();

private:    
    //连接相关的回调函数
    void onConnection(const TcpConnectionPtr&);
    //消息相关的回调函数
    void onMessage(const TcpConnectionPtr&,Buffer*,Timestamp);
    TcpServer server_m;//muduo库服务器类对象
    EventLoop* loop_m;//事件循环对象指针
};

#endif