#include<thread>
#include <iostream>
#include<vector>
#include<chrono>

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

}

//获取系统时间
std::string getCurrentTime(){
    return std::string();
}

//聊天页面程序
void mainMenu(){

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
        std::cout<<"choose please"<<std::endl;
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
                    std::cout<<js["time"]<<"["<<js["userid"]<<"] "<<js["name"]<<" send msg : "<<js["msg"]<<std::endl;
                }
            }
            //登陆成功
            std::thread readTask(readTaskHandler,clientfd);
            readTask.detach();

            //进入主界面聊天
            mainMenu();
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