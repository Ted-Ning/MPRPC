# 将src目录下的所有cpp代码放入SRC_LIST
# aux_source_directory(. SRC_LIST) 该方法当创建新的代码文件时,会出现问题
set(
    SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    rpcheader.pb.cc 
    rpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc
    ) 

# 将SRC_LIST所包含的文件编译成一个动态库,命名为mprpc
# 因为muduo库默认编译为.a的静态库,这里自己的框架也编译为静态库否则会出现链接错误
add_library(mprpc ${SRC_LIST})

#依赖muduo网络库,网络库有两个base和net,muoduo使用了pthread库,也要链接
#muduo_net是muduo核心库,net写在base前面,因为base也依赖net
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)