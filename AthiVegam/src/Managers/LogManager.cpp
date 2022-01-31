#include "AthiVegam/Managers/LogManager.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace AthiVegam::Managers
{
	std::shared_ptr<spdlog::logger> LogManager::logger =
	    nullptr;

	void LogManager::Initialize(const std::string& name)
	{
		// auto consoleSink =
		// std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		// consoleSink->set_pattern("%^[%Y-%m-%d
		// %H:%M:%S.%e] %v%$");
		//
		// std::vector<spdlog::sink_ptr> sinks{ consoleSink
		// };

		logger = spdlog::stdout_color_mt(name);
		logger->set_pattern(
		    "%^[%Y-%m-%d %H:%M:%S.%e] %v%$");
		logger->flush_on(spdlog::level::trace);
		// spdlog::register_logger(logger);
	}

	void LogManager::Shutdown()
	{
		// spdlog::shutdown();
	}

	std::shared_ptr<spdlog::logger> LogManager::Logger()
	{
		return LogManager::logger;
	}
} // namespace AthiVegam::Managers
