#pragma once

#include "AthiVegam/App.h"
#include "AthiVegam/Engine.h"

/* Client Application will implement this method*/
std::unique_ptr<AthiVegam::App> CreateApp();

int main()
{
	AthiVegam::Engine::Instance().Run(CreateApp());

	return 0;
}
