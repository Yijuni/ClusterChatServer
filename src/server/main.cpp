#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>
#include <signal.h>

//服务器ctrl+c结束后，重置user状态信息
void resetHandler(int){
    ChatService::GetInstance().Reset();
    exit(0);
}

int main(){
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"chatserver1");
    server.Start();
    loop.loop();
    return 0;

}