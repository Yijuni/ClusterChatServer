#include "ChatServer.h"

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenaddr, const string &name):
    loop_m(loop),server_m(loop,listenaddr,name)
{
    //注册连接回调
    server_m.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));
    //注册消息回调
    server_m.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置线程数
    server_m.setThreadNum(4);

}

void ChatServer::Start()
{
    server_m.start();
}

void ChatServer::onConnection(const TcpConnectionPtr & conn)
{
    //用户断开连接
    if(!conn->connected()){
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr & conn, Buffer * buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //数据反序列化
    json jsobj = json::parse(buf);
    /*解耦网络模块和业务模块的代码*/
    //根据消息id，获取业务模块的消息处理函数
    MessageHandler handler = ChatService::GetInstance().GetMsgHandler(jsobj["msgid"].get<int>());
    //调用消息处理函数对象
    handler(conn,jsobj,time);
}
