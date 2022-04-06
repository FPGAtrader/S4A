#pragma once

#if defined _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4566 )
#pragma warning( disable : 4459 )
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

#if defined _MSC_VER
#pragma warning( pop )
#endif



#include <iostream>
#include <filesystem>

#include <string>
#include <map>
#include <unordered_map>


namespace S4{

#define LOG_SIZE_MB (1024 * 1024)

//singleton logger
class s4logger : std::enable_shared_from_this<s4logger>
{
public:
	struct cfg_t {
		bool enable_console = (bool)true;       //日志输出到控制台
		spdlog::level::level_enum console_level = spdlog::level::level_enum::info;  //控制台日志等级

		bool enable_file_all = (bool)false;     //通用日志输出到文件
		bool enable_file_all_pure = (bool)true; //不带时间戳的通用日志输出到文件
		spdlog::level::level_enum file_all_level = spdlog::level::level_enum::info; //通用日志输出等级

		bool enable_file_err = (bool)false;     //错误日志
		bool enable_file_err_pure = (bool)false;//不带时间戳的错误日志

		size_t max_file_size_MB = (size_t)9999;
		size_t max_files = (size_t)10;

		std::string save_path = (std::string)"./logs";
		std::string file_preamble = (std::string)"S4_";
	};
public:
    typedef std::shared_ptr<s4logger> Ptr;

	static Ptr pInstance() {
		static Ptr _g_pLogger;
		if (!_g_pLogger) {
            std::unique_lock<std::mutex> lock(_mutex);
            if (!_g_pLogger) {
                init_time_base();

                spdlog::init_thread_pool(65536, 1);  // queue with 64k items and 1 backing thread.

                _g_pLogger.reset(new s4logger);
                _g_pLogger->init();
            }
		}

		return _g_pLogger;
	}

	void init(const s4logger::cfg_t&) noexcept;

	template<typename... Args>
	inline void err(const char *fmt, Args &&... args)
	{
	    if (_asynclogger) _asynclogger->error(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void info(const char *fmt, Args &&... args)
	{
	    if (_asynclogger) _asynclogger->info(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void warn(const char *fmt, Args &&... args)
	{
	    if (_asynclogger) _asynclogger->warn(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void trace(const char *fmt, Args &&... args)
	{
	    if (_asynclogger) _asynclogger->trace(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void fatal(const std::string& file, int line, const char *fmt, Args &&... args)
	{
        if (_asynclogger) {
            _asynclogger->critical(fmt, std::forward<Args>(args)...);
            _asynclogger->critical(file + ":" + std::to_string(line));
        }
		flush();

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		exit(-1);
	}

	inline void flush() noexcept{
        if (_asynclogger) _asynclogger->flush();
	}

    inline const std::string& time_base_cref() const{
        return _time_base;
    }

    inline const std::filesystem::path& file_folder_cref() const{
        return _file_folder;
    }

private:
	s4logger();

	cfg_t _param;

	std::unordered_map<std::string, spdlog::sink_ptr> _file_sinks;

	spdlog::sink_ptr _console;

    std::shared_ptr<spdlog::logger> _asynclogger;

    static std::string _time_base;
    std::filesystem::path _file_folder = ".";

	static std::mutex _mutex;
private:
    static
	void init_time_base() {
		char upTime[64];
		time_t utc;
		time(&utc);
		utc += 3600 * 8;
		struct tm ltm = *gmtime(&utc);
		strftime(upTime, 64, "%Y_%m_%d__%H_%M_%S", &ltm);
		_time_base = std::string(upTime);
	}
    void init();

	void init_file_folder(void);

	void init_console();

	void init_file(const std::string& name_preamble, bool enable, bool pure, bool error);
	void init_files();
    
    void init_asyncloger();
};

#define	TRACE(fmt, ...) {S4::s4logger::pInstance()->trace("[global] " fmt, ## __VA_ARGS__);}
#define	INFO(fmt, ...) {S4::s4logger::pInstance()->info("[global] " fmt, ## __VA_ARGS__);}
#define	WARN(fmt, ...) {S4::s4logger::pInstance()->warn("[global] " fmt, ## __VA_ARGS__);S4::s4logger::pInstance()->flush();}
#define	ERR(fmt, ...) {S4::s4logger::pInstance()->err("[global] " fmt, ## __VA_ARGS__);S4::s4logger::pInstance()->flush();}
#define	FATAL(fmt, ...) {S4::s4logger::pInstance()->fatal(__FILE__, __LINE__, "[global] " fmt, ## __VA_ARGS__);}
#define FLUSH() {S4::s4logger::pInstance()->flush();}


#define LCL_TRAC(fmt, ...) {S4::s4logger::pInstance()->trace("[" __LCL_NAME__ "] " fmt, ## __VA_ARGS__);}
#define	LCL_INFO(fmt, ...) {S4::s4logger::pInstance()->info("[" __LCL_NAME__ "] " fmt, ## __VA_ARGS__);}
#define	LCL_WARN(fmt, ...) {S4::s4logger::pInstance()->warn("[" __LCL_NAME__ "] " fmt, ## __VA_ARGS__);S4::s4logger::pInstance()->flush();}
#define	LCL_ERR(fmt, ...) {S4::s4logger::pInstance()->err("[" __LCL_NAME__ "] " fmt, ## __VA_ARGS__);S4::s4logger::pInstance()->flush();}
#define	LCL_FATAL(fmt, ...) {S4::s4logger::pInstance()->fatal(__FILE__, __LINE__, "[" __LCL_NAME__ "] " fmt,## __VA_ARGS__);}
#define LCL_FLUSH() {S4::s4logger::pInstance()->flush();}

}//namespace S4