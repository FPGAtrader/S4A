#include "common/s4logger.h"

namespace S4 {

std::mutex s4logger::_mutex;
std::string s4logger::_time_base = "null";

static time_t last_err_time_ = 0;
inline void err_handler(const std::string &msg)
{
    auto now = time(nullptr);
    if (now - last_err_time_ < 60)
    {
        return;
    }
    last_err_time_ = now;
    fmt::print(stderr, "[*** LOGGER ERROR ***]  {}\n", msg);
	throw std::exception(std::logic_error("logger error!"));	//logger fail is critical error!

}

s4logger::s4logger()
{
	// _param = new glb_conf_ctx_t::logger_t;
}

void s4logger::init()
{
	flush();

	init_console();

	init_file_folder();
	init_files();

    init_asyncloger();
}

void s4logger::init(const s4logger::cfg_t& p) noexcept {
    std::unique_lock<std::mutex> lock(_mutex);
	_param = p;
    init();
}


void s4logger::init_file_folder(void) {
	_file_folder = _param.save_path;
	if (std::filesystem::exists(_file_folder)) {
		if (!std::filesystem::is_directory(_file_folder)) {
			if (!std::filesystem::create_directory(_file_folder)) {
				std::cerr << "create_directory(" << _file_folder << ") fail!" << std::endl;
				throw std::runtime_error("logger init fail!");
			}
		}
	}
	else
	{
		if (!std::filesystem::create_directories(_file_folder)) {
			std::cerr << "create_directory(" << _file_folder << ") fail!" << std::endl;
			throw std::runtime_error("logger init fail!");
		}
	}
}

void s4logger::init_console() {
	if (_param.enable_console) {
		if (!_console) {
			_console = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
			_console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %^%v%$");
		}
		_console->set_level(_param.console_level);
	}
	else if (_console) {
		_console = nullptr;
	}
}

void s4logger::init_file(const std::string& name_preamble, bool enable, bool pure, bool error) {
	spdlog::sink_ptr pFile_sink;
	std::string name = name_preamble + _time_base;
	if (error) {
		name += "_err";
	}
	if (pure) {
		name += "_pure";
	}

	std::filesystem::path file_path = _file_folder / (name + ".log");

    std::string sink_name = std::string(pure?"p":"t") + std::string(error?"e":"a"); //only 4 types of error file now.
	if (!enable) {
		if (_file_sinks.count(sink_name) != 0) {
			_file_sinks.erase(sink_name);
		}
		return;
	}

	//enable
	if (_file_sinks.count(sink_name) != 0) {
		pFile_sink = _file_sinks.at(sink_name);
		if (((spdlog::sinks::rotating_file_sink_st*)pFile_sink.get())->filename() != file_path.string()) {	//redirect file path
			_file_sinks.erase(sink_name);
			pFile_sink = nullptr;
		}
	}

	if (!pFile_sink) {
		pFile_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(file_path.string(), _param.max_file_size_MB * LOG_SIZE_MB, _param.max_files);
		_file_sinks[sink_name] = pFile_sink;
		//spdlog::sinks::rotating_file_sink_mt* raw = (spdlog::sinks::rotating_file_sink_mt*)pFile_sink->sinks()[0].get();
		//std::cout << raw->filename() << std::endl;
	}
	if (pure) {
		pFile_sink->set_pattern("[%l] %^%v%$");
	}
	else {
		pFile_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %^%v%$");
	}

	if (error) {
		pFile_sink->set_level(spdlog::level::err);
	}
	else {
		pFile_sink->set_level(_param.file_all_level);
	}
}

void s4logger::init_files() {
	init_file(_param.file_preamble, _param.enable_file_all, false, false);
	init_file(_param.file_preamble, _param.enable_file_all_pure, true, false);
	init_file(_param.file_preamble, _param.enable_file_err, false, true);
	init_file(_param.file_preamble, _param.enable_file_err_pure, true, true);
}

    
void s4logger::init_asyncloger()
{
    std::vector<spdlog::sink_ptr> sinks;
    if (_console)
        sinks.push_back(_console);

    for (auto& f : _file_sinks){
        sinks.push_back(f.second);
    }
        
    spdlog::drop("s4logger_async_instance");
    _asynclogger = std::make_shared<spdlog::async_logger>("s4logger_async_instance", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	_asynclogger->set_error_handler(err_handler);
    spdlog::register_logger(_asynclogger);

}

}//namespace S4
