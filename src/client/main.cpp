#include<thread>
#include <iostream>
#include<vector>
#include<chrono>
#include<ctime>
#include <unordered_map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "json.hpp"
#include "Public.h"
#include "User.h"
#include "Group.h"
using json = nlohmann::json;

//当前登录用户
User currentUser;
//当前用户好友
std::vector<User> curUserFriendList;
//当前用户群组信息
std::vector<Group> curUserGroupList;
//支持的命令列表
std::unordered_map<std::string,std::string> command={
    {"help","显示所有支持的命令[help]"},
    {"chat","一对一聊天[chat:friendid:message]"},
    {"addfriend","添加好友[addfriend:friendid]"},
    {"creategroup","创建群组[creategroup:groupname:groupdesc]"},
    {"addgroup","加入群组[addgroup:groupid]"},
    {"groupchat","群聊[groupchat:groupid:message]"},
    {"loginout","退出[loginout]"}
};
//获取系统时间
std::string getCurrentTime(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *local = std::localtime(&now_time);
    char buffer[128];
    sprintf(buffer,"[%d-%d-%d %d:%d:%d]->",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
    return std::string(buffer);
}

void help(int fd,std::string msg){
    std::cout<<"<<<<<<command list>>>>>"<<std::endl;
    for(auto& iter:command){
        std::cout<<iter.first<<":"<<iter.second<<std::endl;
    }
    std::cout<<"<<<<<<<<<>>>>>>>>>"<<std::endl;
}

void chat(int fd,std::string msg){
    int index = msg.find(":");
    if(index==-1){
        std::cerr<<"命令格式错误[chat:friendid:message]"<<std::endl;
        return;
    }
    int friendid = atoi(msg.substr(0,index).c_str());
    //应该要检测是否有这个好友
    //todo....
    json request;
    request["msgid"] = ONT_CHAT_MSG;
    request["fromid"] = currentUser.GetId();
    request["name"] = currentUser.GetName();
    request["toid"] = friendid;
    request["msg"] = msg.substr(index+1);
    request["time"] = getCurrentTime();
    std::string requeststr = request.dump();
    int len = send(fd,requeststr.c_str(),strlen(requeststr.c_str()),0);
    if(len==-1){
        std::cerr<<"send request error,last request msg: "<<requeststr<<std::endl;
    }
}

void addfriend(int fd,std::string msg){
    int friendid = atoi(msg.c_str());
    json request;
    request["msgid"]=ADD_FRIEND_MSG;
    request["userid"]=currentUser.GetId();
    request["friendid"]=friendid;
    std::string requeststr = request.dump();
    int len = send(fd,requeststr.c_str(),strlen(requeststr.c_str()),0);
    if(len==-1){
        std::cerr<<"send request error,last request msg: "<<requeststr<<std::endl;
    }
}

void creategroup(int fd,std::string msg){

}

void addgroup(int fd,std::string msg){

}

void groupchat(int fd,std::string msg){

}

void loginout(int fd,std::string msg){

}
//系统客户端命令处理函数
std::unordered_map<std::string,std::function<void(int,std::string)>> commandHandler={
    {"help",help},
    {"addfriend",addfriend},
    {"chat",chat},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};
//显示当前用户登录信息
void showCurUserData(){
    std::cout<<"==========login user=========="<<std::endl;
    std::cout<<"current login user id: "<<currentUser.GetId()<<" name: "<<currentUser.GetName()<<std::endl;
    std::cout<<"----------friend list----------"<<std::endl;
    if(!curUserFriendList.empty()){
        for(User &user:curUserFriendList){
            std::cout<<user.GetId()<<" "<<user.GetName()<<" "<<user.GetState()<<std::endl;
        }
    }
    std::cout<<"----------group list----------"<<std::endl;
    if(!curUserGroupList.empty()){
        for(Group &group:curUserGroupList){
            std::cout<<"----------"<<std::endl;
            std::cout<<group.GetId()<<"-"<<group.GetName()<<"-"<<group.GetDesc()<<std::endl;
            std::cout<<"----------"<<std::endl;
            for(GroupUser& groupUser:group.GetUsers()){
                std::cout<<groupUser.GetId()<<groupUser.GetRole()<<" "<<groupUser.GetName()<<" "<<groupUser.GetState()<<std::endl;
            }
        }
    }
    std::cout<<"=============================="<<std::endl;
}

//接受线程
void readTaskHandler(int clientfd){
    for(;;){
        char buffer[1024];
        int len = recv(clientfd,buffer,1024,0);
        if(len==0 || len==-1){
            close(clientfd);
            exit(-1);
        }
        json msg = json::parse(buffer);
        if(msg["msgid"].get<int>()==ONT_CHAT_MSG){
            std::cout<<msg["time"].get<std::string>()<<"["<<msg["fromid"]<<"]"<<msg["name"].get<std::string>()
            <<" send : "<<msg["msg"].get<std::string>()<<std::endl;
        }
    }
}


//聊天页面程序
void mainMenu(int clientfd){
    help(0,"");
    char buffer[1024]={0};
    for(;;){
        std::cin.getline(buffer,1024);
        std::string origincmd(buffer);
        std::string cmd;
        int index = origincmd.find(":");
        if(index==std::string::npos){
            cmd = origincmd;//说明是不含":"的命令
        }else{
            cmd = origincmd.substr(0,index);
        }
        auto iter = commandHandler.find(cmd);
        if(iter==commandHandler.end()){
            std::cout<<"invalid input"<<std::endl;
            continue;
        }
        //获取命令处理函数进行处理
        iter->second(clientfd,origincmd.substr(index+1));

    }
}

//主线程作为发送线程
int main(int argc,char** argv){
    if(argc<3){
        std::cerr<<"command invalid! example ./ChatClient 127.0.0.1 6000"<<std::endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    
    //创建clientsocket
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
        std::cerr<<"socket create failed!"<<std::endl;
        exit(-1);
    }

    //填写服务器信息
    sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr =inet_addr(ip); 
    int ret = connect(clientfd,(sockaddr*)&serveraddr,sizeof(sockaddr_in));
    if(-1 == ret)
    {
        if (errno == EINTR)
        {
            //connect 动作被信号中断，重试connect
            std::cout << "connecting interruptted by signal, try again." << std::endl;
            exit(0);
        } else if (errno == EINPROGRESS)
        {
            std::cout<<"正在连接"<<std::endl;
        } else {
            //真的出错了，
            close(clientfd);
            exit(-1);
        }
    }

    //开始接受用户输入信息，发送消息
    while(1){
        //显示界面菜单
        std::cout<<"==========请选择数字，数字代表的含义如下=========="<<std::endl;
        std::cout<<"1. login"<<std::endl;
        std::cout<<"2. register"<<std::endl;
        std::cout<<"3. quit"<<std::endl;
        std::cout<<"================================================"<<std::endl;
        std::cout<<"choose please:"<<std::endl;
        std::cout<<">>";
        int choice;
        std::cin>>choice;
        std::cin.get();//获取缓冲区残余的回车键
        switch(choice)
        {
        case 1:
        {
            int id=0;
            char password[50]={0};
            std::cout<<"userid :";
            std::cin>>id;
            std::cin.get();//读取残留回车
            std::cout<<"password:";
            std::cin.getline(password,50);

            json request;
            request["msgid"]=LOGIN_MSG;
            request["userid"]=id;
            request["password"]=password;
            std::string requeststr = request.dump();
            int len = send(clientfd,requeststr.c_str(),requeststr.length()+1,0);
            if(len==-1){
                std::cerr<<"login message request send failed!"<<std::endl;
                continue;
            }

            char buff[1014]={0};
            len = recv(clientfd,buff,1024,0);
            if(len==-1){
                std::cerr<<"recv reponse message failed"<<std::endl;
                continue;
            }

            json response = json::parse(buff);
            std::cout<<buff<<std::endl;
            if(response["errno"].get<int>()!=0){
                std::cerr<<response["errmsg"]<<std::endl;
                continue;
            }

            //获取当前用户信息
            currentUser.SetId(response["userid"].get<int>());
            currentUser.SetName(response["name"]);
            //记录当前好友信息
            if(response.contains("friends")){
                std::vector<std::string> friends = response["friends"];
                for(std::string &jsonstr:friends){
                    json js = json::parse(jsonstr);
                    //添加好友信息到本地好友列表
                    curUserFriendList.emplace_back(js["userid"].get<int>(),js["name"],"",js["state"]);
                }
            } 

            if(response.contains("group")){
                std::vector<std::string> groups = response["groups"];
                for(std::string &groupstr:groups){
                    Group group;
                    json groupjs = json::parse(groupstr);
                    std::vector<std::string> groupusers = groupjs["groupusers"];
                    for(std::string& groupuserstr: groupusers){
                        json userjs = json::parse(groupuserstr);
                        GroupUser groupuser;
                        groupuser.SetId(userjs["userid"]);
                        groupuser.SetName(userjs["username"]);
                        groupuser.SetRole(userjs["role"]);
                        groupuser.SetState(userjs["state"]);
                        group.GetUsers().push_back(groupuser);
                    }
                    group.SetId(groupjs["groupid"]);
                    group.SetName(groupjs["groupname"]);
                    group.SetDesc(groupjs["groupdesc"]);
                    curUserGroupList.push_back(group);
                }
            }
            showCurUserData();

            //显示用户离线信息
            if(response.contains("offlinemsg")){
                std::vector<std::string> vec = response["offlinemsg"];
                for(std::string &str:vec){
                    json js = json::parse(str);
                    std::cout<<js["time"].get<std::string>()<<"["<<js["fromid"].get<int>()
                    <<"] "<<js["name"].get<std::string>()<<" send msg : "<<js["msg"].get<std::string>()<<std::endl;
                }
            }
            //登陆成功
            std::thread readTask(readTaskHandler,clientfd);
            readTask.detach();

            //进入主界面聊天
            mainMenu(clientfd);
            break;
        }
        case 2:
        {
            char name[50];
            char password[50];
            std::cout<<"user name:";
            std::cin.getline(name,50);
            std::cout<<"password:";
            std::cin.getline(password,50);
            std::cout<<name<<" "<<password;
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = password;
            std::string request = js.dump();
            int len = send(clientfd,request.c_str(),request.length()+1,0);
            if(len==-1){
                std::cerr<<"send register msg error: "<<request<<std::endl;
                continue;
            }
            char buffer[1024];
            len = recv(clientfd,buffer,1024,0);
            if(len==-1){
                std::cerr<<"recv msg error!"<<std::endl;
                continue;
            }
            json response = json::parse(buffer);
            if(response["errno"].get<int>() != 0){
                std::cerr<<response["errmsg"]<<std::endl;
                continue;
            }
            std::cout<<"register success!,userid is: "<<response["userid"]<<" ,remember it!"<<std::endl;
            break;
        }
        case 3:
            close(clientfd);
            exit(0);
            break;
        default:
            {
                std::cerr<<"input invalid!"<<std::endl;
                exit(-1);
            }
        }
    }
    return 0;
}