#ifndef RIFT_INCLUDE_ZOOKEEPERUTIL_H
#define RIFT_INCLUDE_ZOOKEEPERUTIL_H

#include<semaphore.h>
#include<zookeeper/zookeeper.h>
#include<string>

//封装的zk客户端
class ZClient
{
    public:
        ZClient();
        ~ZClient();
        //ZClient启动连接zkserver
        void start();
        //在zkserver中创建一个节点，根据指定的path
        void create(const char* path,const char* data,int datalen,int state=0);
        //根据参数指定的znode节点路径，或者znode节点值
        std::string getData(const char* path);
        
    private:
        //Zk的客户端句柄
        zhandle_t* m_zhandle;
};

#endif
