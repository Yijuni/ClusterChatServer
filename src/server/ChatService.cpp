#include "ChatService.h"
#include "map"
ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

void ChatService::Login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = usermodel_m.Query(id);
    json response;
    if(user.GetId()==id && user.GetPassword()==pwd){
        if(user.GetState()=="online"){
            //登陆成功
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入新的账号！";
            conn->send(response.dump());
            return;
        }
        //登陆成功
        {
            //记录该登录用户
            std::lock_guard<std::mutex> lock(connMutex_m);
            UserConnMap_m.insert({id,conn});
        }
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.GetId();
        response["name"] = user.GetName();

        user.SetState("online");
        //更新数据库用户在线状态
        usermodel_m.UpdateState(user);

        //查询用户是否有离线消息
        std::vector<std::string> vec = offlinemsgmodel_m.Query(user.GetId());
        if(!vec.empty()){
            //获取离线消息
            response["offlinemsg"] = vec;
            //删除离线消息
            offlinemsgmodel_m.Remove(user.GetId());
        }

        //查询登录用户的好友信息
        std::vector<User> uservec = friendmode_m.Query(user.GetId());
        if(!uservec.empty()){
            std::vector<std::string> vec1;
            for(auto& user1:uservec){
                json jstmp;
                jstmp["id"] = user1.GetId();
                jstmp["name"] = user1.GetName();
                jstmp["state"] = user1.GetState();
                vec1.push_back(jstmp.dump());
            } 
            response["friends"] = vec1;
        }

        conn->send(response.dump());
        return;
    }
    //登录失败
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "登录失败!用户名或者密码错误！";
    conn->send(response.dump());
}

void ChatService::Register(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.SetName(name);
    user.SetPassword(pwd);
    bool state = usermodel_m.Insert(user);
    json response;
    if(state){
        //注册成功
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.GetId();
        response["errno"] = 0;
        conn->send(response.dump());
        return;
    }
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "注册失败！";
    conn->send(response.dump());
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

void ChatService::ClientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex_m);
        for(auto it=UserConnMap_m.begin();it!=UserConnMap_m.end();it++){
            if(it->second == conn){
                //从map表删除用户连接信息
                user.SetId(it->first);
                UserConnMap_m.erase(it);
                break;
            }
        }
    }
    if(user.GetId()!=-1){
        user.SetState("offline");
        usermodel_m.UpdateState(user); 
    }
}

void ChatService::OneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_m);
        auto iter = UserConnMap_m.find(toid);
        if(iter!=UserConnMap_m.end()){
            //to在线
            iter->second->send(js.dump());
            LOG_INFO<<"发送成功";
            return;
        }

    }
    //不在线存储离线消息
    offlinemsgmodel_m.Insert(toid,js.dump());
}

void ChatService::AddFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    friendmode_m.Insert(userid,friendid);
    
}

void ChatService::Reset()
{
    //把所有用户的状态online设置为offline
    usermodel_m.ResetState();
}

ChatService::ChatService()
{
    //注册消息以及对应的回调函数
    MsgHandlerMap_m.insert({MsgType::LOGIN_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::REG_MSG,std::bind(&ChatService::Register,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::ONT_CHAT_MSG,std::bind(&ChatService::OneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::ADD_FRIEND_MSG,std::bind(&ChatService::AddFriend,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
}
