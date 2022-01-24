#pragma once

#include <array>
#include <unordered_map>
#include <memory>

using SDL_GameController = struct _SDL_GameController;
struct SDL_ControllerDeviceEvent;

namespace AthiVegam::Input
{
	/** Adopted from SDL */
	enum class ControllerAxis
	{
		LEFT_STICK_HORIZONTAL = 0,
		LEFT_STICK_VERTICAL,
		RIGHT_STICK_HORIZONTAL,
		RIGHT_STICK_VERTICAL,
		LEFT_TRIGGER,
		RIGHT_TRIGGER,
		COUNT
	};

	/** Adopted from SDL */
	enum class ControllerButton
	{
		A,
		B,
		X,
		Y,
		BACK,
		GUIDE,
		START,
		LS,
		RS,
		LB,
		RB,
		DPAD_UP,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		/*BUTTON_MISC1,    */ /*Disabled for now! Enable these when required */
		/*BUTTON_PADDLE1,  */ /*Disabled for now! Enable these when required */
		/*BUTTON_PADDLE2,  */ /*Disabled for now! Enable these when required */
		/*BUTTON_PADDLE3,  */ /*Disabled for now! Enable these when required */
		/*BUTTON_PADDLE4,  */ /*Disabled for now! Enable these when required */
		/*BUTTON_TOUCHPAD, */ /*Disabled for now! Enable these when required */
		COUNT
	};

	class Controller
	{
	public:
		static void OnControllerConnected(SDL_ControllerDeviceEvent& e);
		static void OnControllerDisconnected(SDL_ControllerDeviceEvent& e);

		static void Shutdown();
		static void Update();

		inline static bool IsControllerAvailabe(int controllerId) { return availableControllers.count(controllerId) > 0; }

		static bool GetButton(int controllerId, ControllerButton button);
		static bool GetButtonDown(int controllerId, ControllerButton button);
		static bool GetButtonUp(int controllerId, ControllerButton button);
		static float GetAxisRaw(int controllerId, ControllerAxis axis);
		static float GetAxis(int controllerId, ControllerAxis axis);
		

	private:
		static int GentNextFreeIndex();

	private:
		struct SDLController
		{
			int controllerIndex = -1;
			SDL_GameController * gc = nullptr;
			
			std::array<bool, (int)ControllerButton::COUNT> buttonStates;
			std::array<bool, (int)ControllerButton::COUNT> prevButtonStates;
			
			std::array<float, (int)ControllerAxis::COUNT> axesStates;		/* -1f to 1f */
			std::array<float, (int)ControllerAxis::COUNT> prevAxesStates;	/* -1f to 1f */

			void UpdatePreviousStates()
			{
				prevButtonStates = buttonStates;
				prevAxesStates = axesStates;
			}
		};

		static std::unordered_map<int, std::unique_ptr<SDLController>> availableControllers;
		static float deadzone;

	};
}