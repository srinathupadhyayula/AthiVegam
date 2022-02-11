#pragma once

#include "AthiVegam/Core/VegamWindow.h"

namespace AthiVegam
{
	class App
	{
	  public:
		virtual ~App() = default;

		virtual Core::WindowProperties GetWindowProperties()
		{
			return Core::WindowProperties();
		}
		virtual void Initialize(){};
		virtual void Shutdown(){};
		virtual void Update(){};
		virtual void Render(){};
		virtual void ImGuiRender(){};
	};
} // namespace AthiVegam
