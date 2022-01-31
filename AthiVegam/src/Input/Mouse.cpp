#include "AthiVegam/Input/Mouse.h"

#include "AthiVegam/Log.h"
#include "SDL2/SDl.h"

#include <algorithm>

namespace AthiVegam::Input
{
	int Mouse::x = 0;
	int Mouse::xPrev = 0;
	int Mouse::y = 0;
	int Mouse::yPrev = 0;

	MouseButtonStates Mouse::buttonStates;
	MouseButtonStates Mouse::prevButtonStates;

	void Mouse::Initialize()
	{
		std::fill(buttonStates.begin(), buttonStates.end(),
		          false);
		std::fill(prevButtonStates.begin(),
		          prevButtonStates.end(), false);
	}

	void Mouse::Update()
	{
		xPrev = x;
		yPrev = y;
		prevButtonStates = buttonStates;
		Uint32 state = SDL_GetMouseState(&x, &y);
		for (int i = 0; i < buttonStates.size(); ++i)
		{
			buttonStates[i] = state & SDL_BUTTON(i + 1);
		}
	}

	bool Mouse::Button(MouseButton button)
	{
		auto buttonIndex = static_cast<int>(button) - 1;
		return buttonStates[buttonIndex];
	}

	bool Mouse::ButtonDown(MouseButton button)
	{
		auto buttonIndex = static_cast<int>(button) - 1;
		return buttonStates[buttonIndex]
		       && !prevButtonStates[buttonIndex];
	}

	bool Mouse::ButtonUp(MouseButton button)
	{
		auto buttonIndex = static_cast<int>(button) - 1;
		return !buttonStates[buttonIndex]
		       && prevButtonStates[buttonIndex];
	}
} // namespace AthiVegam::Input
