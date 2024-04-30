# 将callee目录下的代码文件以及上一级目录的文件放入SRC_LIST
#set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

# 编译成可执行文件 命名为provider
add_executable(consumer ${SRC_LIST})
# 链接mprpc和protobuf外部库
target_link_libraries(consumer mprpc protobuf)