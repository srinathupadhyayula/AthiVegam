#pragma once

#include <array>

namespace AthiVegam::Input
{
	/** Adopted from SDL */
	enum class MouseButton
	{
		AV_FIRST_BUTTON =
		    1, /* SDL Mouse Buttons Start at Index 1 */
		AV_MOUSE_LEFT = AV_FIRST_BUTTON,
		AV_MOUSE_MIDDLE,
		AV_MOUSE_RIGHT,
		AV_MOUSE_X1,
		AV_MOUSE_X2,
	};
	constexpr static const int MouseButtonCount =
	    5; // SDL supports 5 mouse buttons
	using MouseButtonStates =
	    std::array<bool, MouseButtonCount>;

	class Mouse
	{
	  public:
		static void Initialize();
		static void Update();

		inline static int X() { return x; }
		inline static int Y() { return y; }
		inline static int DX() { return x - xPrev; }
		inline static int DY() { return y - yPrev; }

		static bool Button(MouseButton button);
		static bool ButtonDown(MouseButton button);
		static bool ButtonUp(MouseButton button);

	  private:
		static int x, xPrev;
		static int y, yPrev;

		static MouseButtonStates buttonStates;
		static MouseButtonStates prevButtonStates;
	};
} // namespace AthiVegam::Input
