#include "SerialLedCppLib.h"


int main() {

	SerialLedCppLib::LedController ledController;


	SerialLedCppLib::LedController::SerialError err;

	ledController.AutoConnect(10);


	std::cout << "Version: " << ledController.GetVersion() << std::endl;






	std::cout << ledController.GetLedCount() << std::endl;

	ledController.StartPulse(1000, 0, 1000, 0, 25, 0, 25);

	//std::cout << ledController.SetStaticColor(255, 255, 255) << std::endl;

	//for (int i = 0; i < 10; i++)
	//{
	//	for (int j = 0; j < 255; j++)
	//	{
	//		ledController.SetStaticColorAsync(j, (255 - j), 0);
	//	}
	//}

	//ledController.SetStaticColor(255, 123, 121);
	//std::cout << ledController.GetLedColor(1);

	//for (int i = 0; i < 10; i++)
	//{
	//	for (int j = 0; j < 255; j++)
	//	{
	//		ledController.SetStaticColor(j, (255 - j), 0);
	//	}
	//}



	return 0;
}