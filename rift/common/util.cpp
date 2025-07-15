#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "rift/common/util.h"

namespace rift{
    static int g_pid = 0; // 整个程序共用一个进程号，所以设置成全局
    static thread_local int t_thread_id = 0; // 线程号是每个线程私有的，所以设置thread_local
    
    pid_t getPid(){
        if (g_pid != 0){
            return g_pid;
        }
        return getpid(); // 注意大小写
    }

    pid_t getThreadId(){
        if (t_thread_id != 0){
            return t_thread_id;
        }
        return syscall(SYS_gettid);
    }
}