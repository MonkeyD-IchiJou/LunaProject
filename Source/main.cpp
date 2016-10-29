#include "DebugLog.h"
#include "LunaManager.h"

#if _DEBUG
/* console window enable when i want to debug */
int main()
#else
/* release build for Window */
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
#endif

{
	auto lunaManager = luna::LunaManager::getInstance();
	
#if _DEBUG 

	/* if got debug .. then try and catch error in real time */
	try 
	{
		lunaManager->Run();
		lunaManager->Destroy();
	} 
	catch (const std::runtime_error& e) 
	{
		luna::DebugLog::print("[Main Thread Catch]->");
		luna::DebugLog::printL(e.what());
		system("pause");
		return EXIT_FAILURE;
	}

#else //BUILD_ENABLE_DEBUG

	/* release build version */
	/* no debug or whatsoever, zero overhead */
	lunaManager->Run();
	lunaManager->Destroy();

#endif

	return 0;
}