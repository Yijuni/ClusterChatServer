#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
#include <string>
#include <iostream>
class Redis{
public:
    Redis();
    ~Redis();

    //连接redis服务器
    bool Connect();
    //向redis指定channel发布消息
    bool Publish(int channel,std::string message);
    //向指定channel订阅消息
    bool Subscribe(int channel);
    //向指定channel取消订阅
    bool Unsubscribe(int channel);
    //在其他线程接收channel上发布的消息
    void Observer_channel_message();
    //初始化 上报通道消息的回调函数
    void Init_notify_message_handler(std::function<void(int,std::string)> fc);
private:
    //hiredis同步上下文对象，处理publish
    redisContext *publish_context_m;
    //处理subscribe
    redisContext *subscribe_context_m;
    //回到函数，接收到订阅消息时用来给service层上报
    std::function<void(int,std::string)> notify_message_handler_m;
};
#endif
