#定义SRC_LIST包含本目录所有文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
#生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST})
#指定可执行文件链接时需要的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread)