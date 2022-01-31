#pragma once

#include "spdlog/spdlog.h"

#include <memory>

namespace AthiVegam::Managers
{
	constexpr auto AV_DEFAULT_LOGGER_NAME = "VegamLogger";

	class LogManager
	{
	  public:
		static void Initialize(const std::string& name =
		                           AV_DEFAULT_LOGGER_NAME);
		static void Shutdown();

		static std::shared_ptr<spdlog::logger> Logger();

	  private:
		static std::shared_ptr<spdlog::logger> logger;
	};
} // namespace AthiVegam::Managers
