#include "redis.h"

Redis::Redis():publish_context_m(nullptr),subscribe_context_m(nullptr)
{
}

Redis::~Redis()
{
    if(publish_context_m!=nullptr){
        redisFree(publish_context_m);
    }
    if(subscribe_context_m!=nullptr){
        redisFree(subscribe_context_m);
    }
}

bool Redis::Connect()
{
    publish_context_m = redisConnect("127.0.0.1",6379);
    if(nullptr==publish_context_m){
        std::cerr<<"connect redis failed"<<std::endl;
        return false;
    }
    subscribe_context_m = redisConnect("127.0.0.1",6379);
    if(nullptr==subscribe_context_m){
        std::cerr<<"connect redis failed"<<std::endl;
        return false;
    }
    //在单独线程中监听通道上的事件
    std::thread t([&](){
        Observer_channel_message();
    });
    t.detach();
    std::cout<<"connect success!"<<std::endl;
    return true;
}

bool Redis::Publish(int channel, std::string message)
{
    //reply是动态生成的结构体，用完之后free
    redisReply *reply = (redisReply*)redisCommand(publish_context_m,"PUBLISH %d %s",channel,message.c_str());
    if(nullptr==reply){
        std::cerr<<"publish command failed"<<std::endl;
        return false;
    }
    freeReplyObject(reply);
    std::cout<<"跨服务器发送消息成功 channel:"<< channel<<" "<<message<<std::endl;
    return true;
}

bool Redis::Subscribe(int channel)
{
    //subscribe会导致线程阻塞，这里只订阅通道，但是不接受消息，消息单独在其他线程接收，防止程序阻塞在这
    //redisAppendCommand只会把命令写在缓冲区
    //redisCommand:先写到缓冲区->缓冲区数据写到服务器->阻塞等待消息一气呵成
    if(REDIS_ERR==redisAppendCommand(subscribe_context_m,"SUBSCRIBE %d",channel)){
        std::cerr<<"SUBSCRIBE command failed"<<std::endl;
        return false;
    }
    //redisBufferWrite循环发送缓冲区数据，直到缓冲区数据发送完毕，done被置为1
    int done=0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(subscribe_context_m,&done)){
            std::cerr<<"subscribe command failed"<<std::endl;
            return false;
        }
    }
    std::cout<<"订阅成功"<<std::endl;
    return true;
}

bool Redis::Unsubscribe(int channel)
{

    if(REDIS_ERR==redisAppendCommand(subscribe_context_m,"UNSUBSCRIBE %d",channel)){
        std::cerr<<"UNSUBSCRIBE command failed"<<std::endl;
        return false;
    }
    //redisBufferWrite循环发送缓冲区数据，直到缓冲区数据发送完毕，done被置为1
    int done=0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(subscribe_context_m,&done)){
            std::cerr<<"UNsubscribe command failed"<<std::endl;
            return false;
        }
    }
    std::cout<<"取消订阅成功"<<std::endl;
    return true;
}

void Redis::Observer_channel_message()
{
    std::cout<<"channel回调"<<std::endl;
    redisReply* reply;
    while(REDIS_OK==redisGetReply(subscribe_context_m,(void **)&reply)){
        //收到的消息是带三个元素的数组
        if(reply !=nullptr && reply->element[2]!=nullptr && reply->element[2]->str !=nullptr){
            //给业务层上报消息 来自哪个通道的哪个消息
            std::cout<<"成功收到其他服务器信息:"<<reply->element[2]->str<<std::endl;
            notify_message_handler_m(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    std::cerr<<"Observer_channel_message quit"<<std::endl;
}

void Redis::Init_notify_message_handler(std::function<void(int, std::string)> fc)
{
    notify_message_handler_m = fc;
}
