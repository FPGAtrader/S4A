#include "common/s4logger.h"
#include <cstdlib>
#include <cmath>

//定义本地模块日志前缀
#ifdef __LCL_NAME__
#undef __LCL_NAME__
#endif
#define __LCL_NAME__ "test_logger"


int main(int argc, char** argv)
{
    INFO("main():");
    LCL_INFO("start");
    INFO("git SHA1={}", S4GIT_SHA1);//打印当前版本的git hash

    const std::string& logger_time = S4::s4logger::pInstance()->time_base_cref();
    INFO("默认日志仅输出到控制台和\"S4{}_pure.log\"文件中。", logger_time);
    
    WARN("开始测试动态日志切换：");
    S4::s4logger::cfg_t p;
    p.save_path = "./logs";
    p.enable_file_all_pure = false; //disable all file logs

    /////////// 逐个开启日志文件
    p.enable_file_all = true;
    S4::s4logger::pInstance()->init(p);
    LCL_INFO("这是一条本地模块普通消息，将输出到\"./logs/S4_{}.log\"文件中。", logger_time);

    p.enable_file_all_pure = true;
    S4::s4logger::pInstance()->init(p);
    LCL_WARN("这是一条本地模块警告消息，将输出到\"./logs/S4_{0}.log\"和\"S4_{0}_pure.log\"文件中。", logger_time);

    p.enable_file_err = true;
    S4::s4logger::pInstance()->init(p);
    LCL_ERR("这是一条本地模块错误消息，将输出到\"./logs/S4_{0}.log\"、\"S4_{0}_pure.log\"和\"S4_{0}_err.log\"文件中。", logger_time);

    p.enable_file_err_pure = true;
    S4::s4logger::pInstance()->init(p);
    ERR("这是一条通用模块错误消息，将输出到\"./logs/S4_{0}.log\"、\"S4_{0}_pure.log\"、\"S4_{0}_err.log\"和\"S4_{0}_err_pure.log\"文件中。", logger_time);

    INFO("----------------- 切换日志文件名前缀为 test_logger_{0}.log ------------------", logger_time);
    p.file_preamble = "test_logger_";
    S4::s4logger::pInstance()->init(p);
    for (int i=3; i>=0; --i){
        INFO("----------------- 准备多线程测试 倒数{} ------------------", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    auto f = [&](int seq){
        for (int i=0; i<20; ++i){
            int r = rand() % (std::abs(5-seq)+1);
            std::this_thread::sleep_for(std::chrono::milliseconds(r));
            LCL_INFO("线程#{}: 第#{}条日志", seq, i);
        }
        FLUSH();
    };

    std::vector<std::thread> ts;
    for (int i=0; i<5; ++i){
        ts.emplace_back(
            std::thread(f, i)
        );
    }

    for (int i=0; i<5; ++i){
        ts[i].join();
    }


    LCL_FATAL("这是一条本地模块致命消息，将终止程序，返回非0.");
}