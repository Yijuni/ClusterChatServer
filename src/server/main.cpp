#include "ChatServer.h"
#include <iostream>

int main(){
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"chatserver1");
    server.Start();
    loop.loop();
    return 0;

}