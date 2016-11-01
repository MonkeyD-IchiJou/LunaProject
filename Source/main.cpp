#include "DebugLog.h"
#include "LunaManager.h"
#include "Global.h"

#if _DEBUG
/* console window enable when i want to debug */
int main()
{

#elif VK_USE_PLATFORM_WIN32_KHR
/* release build for Window */
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{

#elif VK_USE_PLATFORM_ANDROID_KHR
/* release build for android */
void android_main(android_app* pApplication)
{

#endif

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