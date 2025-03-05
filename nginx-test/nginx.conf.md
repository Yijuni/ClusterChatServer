### nginx.conf需要添加和http块同级别
##基于tcp的负载均衡
stream{
    upstream MyServer{
        #服务   IP:PORT    消息转发权重  心跳超过三次失败说明服务器挂掉  心跳超时时间
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
    }
    server{
        proxy_connect_timeout 1s;
        #proxy_timeout 3s;
        #nginx监听的端口号
        listen 8000;
        #标记所有提供服务的服务器
        proxy_pass MyServer;

        tcp_nodelay on;
    }
}