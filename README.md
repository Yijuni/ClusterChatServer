# ClusterChatServer
## 简介
基于ngnix实现tcp连接负载均衡和muduo网络库的聊天服务器集群和客户端的源代码
## 使用
cd build  
rm -rf *  
cmake ..  
make  
## 配置
### nginx
根据linux版本下载nginx源码进行编译，确保nginx服务能启动，并根据nginx-test文件夹下的文件对本地nginx.conf进行配置  
并启动nginx，如果执行失败可尝试管理员权限执行，如果配置没有加载尝试执行 sudo ./nginx -s reload
### redis
下载redis到主机并启动即可
