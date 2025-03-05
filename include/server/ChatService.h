#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <mutex>
#include "json.hpp"
#include "Public.h"
#include "UserModel.h"
#include "OfflineMessageModel.h"
#include "FriendModel.h"
#include "GroupModel.h"
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
    //处理客户端异常退出
    void ClientCloseException(const TcpConnectionPtr &conn);
    //一对一聊天服务
    void OneChat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //添加好友业务
    void AddFriend(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //服务器异常关闭，业务重置
    void Reset();
    //创建群组
    void CreateGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //群聊服务
    void GroupChat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //加群
    void AddGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) =delete;
private:
    ChatService();

    //存储消息ID和对应处理函数
    std::unordered_map<int,MessageHandler> MsgHandlerMap_m;

    //存储在线用户通信连接
    std::unordered_map<int,TcpConnectionPtr> UserConnMap_m;
    
    //数据操作类
    UserModel usermodel_m;
    OfflineMessageMode offlinemsgmodel_m;
    FriendModel friendmode_m;
    GroupModel groupmodel_m;
    //互斥锁保证UserConnMap的线程安全
    std::mutex connMutex_m;
};

#endif