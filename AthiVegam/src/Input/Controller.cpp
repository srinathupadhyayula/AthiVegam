#include "AthiVegam/Input/Controller.h"
#include "AthiVegam/Log.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_gamecontroller.h"

#include <algorithm>


namespace AthiVegam::Input
{
	std::unordered_map<int, std::unique_ptr<Controller::SDLController>> Controller::availableControllers;
	float Controller::deadzone = 0.1f; /*Should probalby be customizable*/

	constexpr const auto SDL_AXES_NORMALIER = 32767.f;


	void Controller::OnControllerConnected(SDL_ControllerDeviceEvent& e)
	{
		int deviceIndex = e.which;

		if (SDL_IsGameController(deviceIndex))
		{
			auto controller = std::make_unique<Controller::SDLController>();
			controller->gc = SDL_GameControllerOpen(deviceIndex);
			if (controller->gc)
			{
				controller->controllerIndex = deviceIndex;
				std::fill(controller->buttonStates.begin(), controller->buttonStates.end(), false);
				std::fill(controller->prevButtonStates.begin(), controller->prevButtonStates.end(), false);
				std::fill(controller->axesStates.begin(), controller->axesStates.end(), 0.0f);
				std::fill(controller->prevAxesStates.begin(), controller->prevAxesStates.end(), 0.0f);

				int mapIndex = GentNextFreeIndex();
				VEGAM_INFO("Controller connected: mapIndex({}), deviceIndex({})", mapIndex, deviceIndex);
				availableControllers[mapIndex] = std::move_if_noexcept(controller);
			}
			else
			{
				VEGAM_ERROR("SDL Error: Error opening game controller with Device Index {}: {}", deviceIndex, SDL_GetError());
			}
		}
	}

	void Controller::OnControllerDisconnected(SDL_ControllerDeviceEvent& e)
	{
		int deviceIndex = e.which;

		for (auto it = availableControllers.begin(); it != availableControllers.end(); ++it)
		{
			auto controller = it->second.get();
			if (controller->controllerIndex == deviceIndex)
			{
				VEGAM_WARN("Controller disconnected: {}", deviceIndex);
				SDL_GameControllerClose(controller->gc);
				availableControllers.erase(it);
				break;
			}
		}
	}

	void Controller::Shutdown()
	{
		for (auto it = availableControllers.begin(); it != availableControllers.end();)
		{
			auto controller = it->second.get();
			SDL_GameControllerClose(controller->gc);
			it = availableControllers.erase(it);
		}
	}

	void Controller::Update()
	{
		for (auto it = availableControllers.begin(); it != availableControllers.end(); ++it)
		{
			auto controller = it->second.get();
			VEGAM_ASSERT((controller && controller->gc), "Invalid Game Controller in availableControllers map");
			if (controller && controller->gc)
			{
				controller->UpdatePreviousStates();

				for (auto i = 0; i < static_cast<int>(ControllerButton::COUNT); ++i)
				{
					controller->buttonStates[i] = SDL_GameControllerGetButton(controller->gc, static_cast<SDL_GameControllerButton>(i));
				}

				for (auto i = 0; i < static_cast<int>(ControllerAxis::COUNT); ++i)
				{
					controller->axesStates[i] = std::clamp(SDL_GameControllerGetAxis(controller->gc, static_cast<SDL_GameControllerAxis>(i)) / SDL_AXES_NORMALIER, -1.f, 1.f);
				}
			}
		}
	}

	bool Controller::GetButton(int controllerId, ControllerButton button)
	{
		auto it = availableControllers.find(controllerId);
		if (it != availableControllers.end())
		{
			return it->second.get()->buttonStates[static_cast<int>(button)];
		}
		VEGAM_WARN("Controller Id: {} is not available", controllerId);
		return false;
	}

	bool Controller::GetButtonDown(int controllerId, ControllerButton button)
	{
		auto it = availableControllers.find(controllerId);
		if (it != availableControllers.end())
		{
			return it->second.get()->buttonStates[static_cast<int>(button)] && !it->second.get()->prevButtonStates[static_cast<int>(button)];
		}
		VEGAM_WARN("Controller Id: {} is not available", controllerId);
		return false;
	}

	bool Controller::GetButtonUp(int controllerId, ControllerButton button)
	{
		auto it = availableControllers.find(controllerId);
		if (it != availableControllers.end())
		{
			return !it->second.get()->buttonStates[static_cast<int>(button)] && it->second.get()->prevButtonStates[static_cast<int>(button)];
		}
		VEGAM_WARN("Controller Id: {} is not available", controllerId);
		return false;
	}

	float Controller::GetAxisRaw(int controllerId, ControllerAxis axis)
	{
		auto it = availableControllers.find(controllerId);
		if (it != availableControllers.end())
		{
			return it->second.get()->axesStates[static_cast<int>(axis)];
		}
		VEGAM_WARN("Controller Id: {} is not available", controllerId);
		return 0.0f;
	}

	float Controller::GetAxis(int controllerId, ControllerAxis axis)
	{
		auto val = GetAxisRaw(controllerId, axis);
		return abs(val) > deadzone ? val: 0.f;
	}

	int Controller::GentNextFreeIndex()
	{
		int ret = -1;
		for (int i = 0; i < SDL_NumJoysticks(); ++i)
		{
			if (availableControllers.count(i) == 0)
			{
				ret = i;
				break;
			}
		}

		return ret;
	}

}