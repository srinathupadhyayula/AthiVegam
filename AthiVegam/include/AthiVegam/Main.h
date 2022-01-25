#pragma once

#include "AthiVegam/Engine.h"
#include "AthiVegam/App.h"

/* Client Application will implement this method*/
std::unique_ptr<AthiVegam::App> CreateApp();

int main()
{
	AthiVegam::Engine::Instance().Run(CreateApp());

	return 0;
}


