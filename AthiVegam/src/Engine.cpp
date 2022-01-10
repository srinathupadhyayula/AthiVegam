#include "Engine.h"
#include <iostream>

#include "sdl2/SDL.h"

void AthiVegam::GetInfo()
{
#ifdef  AV_CONFIG_DEBUG
	std::cout << "Configuration : DEBUG" << std::endl;
#endif //  AV_CONFIG_DEBUG
#ifdef AV_CONFIG_RELEASE
	std::cout << "Configuration : RELEASE" << std::endl;
#endif // AV_CONFIG_RELEASE
#ifdef AV_PLATFORM_WINDOWS
	std::cout << "Platform : WINDOWS" << std::endl;
#endif // AV_PLATFORM_WINDOWS
#ifdef AV_PLATFORM_LINUX
	std::cout << "Platform : LINUX" << std::endl;
#endif // AV_PLATFORM_LINUX
#ifdef AV_PLATFORM_MAC
	std::cout << "Platform : MAC" << std::endl;
#endif // AV_PLATFORM_MAC
	
}

bool AthiVegam::Initialize()
{
	bool ret = true;

	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		std::cout << "Error initializing SDL2: " << SDL_GetError() << std::endl;
		ret = false;
	}
	else
	{
		SDL_version version;
		SDL_VERSION(&version);
		std::cout << "SDL " << (int32_t)version.major << "." << (int32_t)version.minor << "." << (int32_t)version.patch << std::endl;
	}
	return ret;
}

void AthiVegam::Shutdown()
{
	SDL_Quit();
}
