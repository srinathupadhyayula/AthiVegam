#include <iostream>
#include "AthiVegam/Engine.h"



int main() 
{
	std::cout << "Welcome to AthiVegam!" << std::endl;
	
	AthiVegam::Engine::Instance().Run();


	std::cout << 25u - 50;


	std::cout << "Press enter to exit!" << std::endl;
	std::cin.ignore();

	return 0;
}

