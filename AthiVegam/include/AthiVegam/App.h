#pragma once

namespace AthiVegam
{
	class App
	{
	  public:
		virtual ~App() = default;

		virtual void Initialize(){};
		virtual void Shutdown(){};
		virtual void Update(){};
		virtual void Render(){};
	};
} // namespace AthiVegam
