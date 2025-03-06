#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>
#include <signal.h>

//服务器ctrl+c结束后，重置user状态信息
void resetHandler(int){
    ChatService::GetInstance().Reset();
    exit(0);
}

int main(int argc,char** argv){
    signal(SIGINT,resetHandler);
    EventLoop loop;
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"chatserver1");
    server.Start();
    loop.loop();
    return 0;

}