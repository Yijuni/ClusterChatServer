#include "ChatService.h"
#include "map"
ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

void ChatService::Login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["userid"].get<int>();
    std::string pwd = js["password"];

    User user = usermodel_m.Query(id);
    json response;
    if(user.GetId()==id && user.GetPassword()==pwd){
        if(user.GetState()=="online"){
            //登陆成功
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this number is used，please input again！";
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
        response["userid"] = user.GetId();
        response["name"] = user.GetName();
        //用户登陆成功，需要订阅与自己编号相同的通道，这样其他服务器用户可以向该用户发消息
        redis_m.Subscribe(user.GetId());
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
                jstmp["userid"] = user1.GetId();
                jstmp["name"] = user1.GetName();
                jstmp["state"] = user1.GetState();
                vec1.push_back(jstmp.dump());
            } 
            response["friends"] = vec1;
        }

        //查询登录用户群组信息
        std::vector<Group> groups = groupmodel_m.QueryGroup(user.GetId());
        if(!groups.empty()){
            //"group"
            std::vector<std::string> groupVec;
            for(Group &group:groups){
                json groupjs;
                groupjs["groupid"] = group.GetId();
                groupjs["groupname"] = group.GetName();
                groupjs["groupdesc"] = group.GetDesc();
                std::vector<std::string> uservec;
                for(GroupUser &groupuser:group.GetUsers()){
                    json userjs;
                    userjs["userid"] = groupuser.GetId();
                    userjs["username"] = groupuser.GetName();
                    userjs["state"] = groupuser.GetState();
                    userjs["role"] = groupuser.GetRole();
                    uservec.push_back(userjs.dump());
                }
                groupjs["groupusers"] = uservec;
                groupVec.push_back(groupjs.dump());
            }
            response["groups"] = groupVec;
        }
        conn->send(response.dump());
        return;
    }
    //登录失败
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "login failed!id or password error！";
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
        response["userid"] = user.GetId();
        response["errno"] = 0;
        conn->send(response.dump());
        return;
    }
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "register failed";
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
        //异常退出取消订阅通道，别人没法向其发消息
        redis_m.Unsubscribe(user.GetId());
    }
    
}
void ChatService::LoginOut(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_m);
        auto iter = UserConnMap_m.find(userid);
        if(iter!=UserConnMap_m.end()){
            UserConnMap_m.erase(iter);
        }
    }
    User user(userid);
    user.SetState("offline");
    usermodel_m.UpdateState(user);
    //取消一下通道，没法向它发消息
    redis_m.Unsubscribe(userid);
}
void ChatService::OneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    //查本机器是否有该用户
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
    //查询其他主机是否有该用户
    User user = usermodel_m.Query(toid);
    if(user.GetState()=="online"){
        std::cout<<"不在本服务器"<<std::endl;
        redis_m.Publish(toid,js.dump());
        return;
    }
    std::cout<<"不在线插入数据库"<<std::endl;
    //不在线存储离线消息
    offlinemsgmodel_m.Insert(toid,js.dump());
}

void ChatService::AddFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    friendmode_m.Insert(userid,friendid);
    friendmode_m.Insert(friendid,userid);
}

void ChatService::Reset()
{
    //把所有用户的状态online设置为offline
    usermodel_m.ResetState();
}

void ChatService::CreateGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if(groupmodel_m.CreateGroup(group)){
        //数据库存储创建人的信息,相当于创建人加了群
        groupmodel_m.AddGroup(userid,group.GetId(),"creator");
    }
}

void ChatService::GroupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["fromid"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::string chatmsg = js.dump();
    std::vector<int> groupuser = groupmodel_m.QueryGroupUsers(userid,groupid);
    for(int& id:groupuser){
        std::lock_guard<std::mutex> lock(connMutex_m);
        LOG_INFO<<chatmsg;
        auto iter = UserConnMap_m.find(id);
        if(iter==UserConnMap_m.end()){
            //查看是否在其他主机
            User user = usermodel_m.Query(id);
            if(user.GetState()=="online"){
                redis_m.Publish(id,chatmsg);
                continue;
            }
            offlinemsgmodel_m.Insert(id,chatmsg);
        }else{
            iter->second->send(chatmsg);
        }
        
    }
}

void ChatService::AddGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupmodel_m.AddGroup(userid,groupid,"normal");
}

void ChatService::HandleRedisSubscribeMessage(int channle, std::string message)
{
    std::lock_guard<std::mutex> lock(connMutex_m);
    auto iter = UserConnMap_m.find(channle);
    if(iter!=UserConnMap_m.end()){
        iter->second->send(message);
        std::cout<<"成功跨服务器发送消息"<<std::endl;
        return;
    }
    //如果不在线(不过应该很难存在这种情况),插入离线消息
    offlinemsgmodel_m.Insert(channle,message);
}

ChatService::ChatService()
{
    //注册消息以及对应的回调函数
    MsgHandlerMap_m.insert({MsgType::LOGIN_MSG,std::bind(&ChatService::Login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::REG_MSG,std::bind(&ChatService::Register,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::ONT_CHAT_MSG,std::bind(&ChatService::OneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::ADD_FRIEND_MSG,std::bind(&ChatService::AddFriend,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::CREATE_GROUP_MSG,std::bind(&ChatService::CreateGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::GROUP_CHAT_MAG,std::bind(&ChatService::GroupChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::ADD_GROUP_MSG,std::bind(&ChatService::AddGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    MsgHandlerMap_m.insert({MsgType::LOGIN_OUT_MSG,std::bind(&ChatService::LoginOut,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

    if(redis_m.Connect()){
        std::cout<<"redis连接成功"<<std::endl;
        redis_m.Init_notify_message_handler(std::bind(&ChatService::HandleRedisSubscribeMessage,this,std::placeholders::_1,std::placeholders::_2));
    }
}
