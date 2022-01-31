#include "AthiVegam/Input/Keyboard.h"

#include "SDL2/SDL.h"

namespace AthiVegam::Input
{
	KeyStates Keyboard::keyStates;
	KeyStates Keyboard::prevKeyStates;

	void Keyboard::Initialize()
	{
		std::fill(keyStates.begin(), keyStates.end(),
		          false);
		std::fill(prevKeyStates.begin(),
		          prevKeyStates.end(), false);
	}

	void Keyboard::Update()
	{
		prevKeyStates = keyStates;
		const Uint8* state = SDL_GetKeyboardState(nullptr);
		for (int i = (int)KeyCode::AV_FIRST_KEY;
		     i < KeyCount; ++i)
		{
			keyStates[i] = state[i];
		}
	}

	bool Keyboard::Key(KeyCode key)
	{
		auto keyIndex = static_cast<int>(key);
		return keyStates[keyIndex];
	}

	bool Keyboard::KeyDown(KeyCode key)
	{
		auto keyIndex = static_cast<int>(key);
		return keyStates[keyIndex]
		       && !prevKeyStates[keyIndex];
	}

	bool Keyboard::KeyUp(KeyCode key)
	{
		auto keyIndex = static_cast<int>(key);
		return !keyStates[keyIndex]
		       && prevKeyStates[keyIndex];
	}
} // namespace AthiVegam::Input
