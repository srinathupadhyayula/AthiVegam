#pragma once
#include "spdlog/spdlog.h"

#define ATHI_VEGAM_DEFAULT_LOGGER_NAME "VegamLogger"

#if defined(AV_PLATFORM_WINDOWS)
	#define VEGAM_BREAK __debugbreak();
#elif defined(AV_PLATFORM_MAC)
	#define VEGAM_BREAK __builtin_debugtrap();
#else
	#define VEGAM_BREAK __builtin_trap();
#endif

#ifndef AV_CONFIG_RELEASE
	#define VEGAM_TRACE(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);}
	#define VEGAM_DEBUG(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);}
	#define VEGAM_INFO(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);}
	#define VEGAM_WARN(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);}
	#define VEGAM_ERROR(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);}
	#define VEGAM_FATAL(...) if (spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(ATHI_VEGAM_DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);}
	#define VEGAM_ASSERT(x, msg) if ((x)) {} else {VEGAM_FATAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); VEGAM_BREAK}
#else
	#define VEGAM_TRACE(...) (void)0
	#define VEGAM_DEBUG(...) (void)0
	#define VEGAM_INFO(...) (void)0
	#define VEGAM_WARN(...) (void)0
	#define VEGAM_ERROR(...) (void)0
	#define VEGAM_FATAL(...) (void)0
	#define VEGAM_ASSERT(x, msg) if ((x)) {} else {VEGAM_FATAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__);}
#endif
