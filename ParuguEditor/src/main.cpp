#include <iostream>
#include "AthiVegam/Engine.h"


int main() 
{
	std::cout << "Welcome to AthiVegam!" << std::endl;
	AthiVegam::GetInfo();
	AthiVegam::Initialize();
	AthiVegam::Shutdown();

	std::cout << "Press enter to exit!" << std::endl;
	std::cin.ignore();

	return 0;
}
