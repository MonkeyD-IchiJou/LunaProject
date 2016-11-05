#include "WinNative.h"
#include "DebugLog.h"

static void LunaMain()
{
	// window creation start here
	auto win = luna::WinNative::getInstance();
	win->Create();
	
	// destroy the windows
	win->Destroy();
}

#if _DEBUG

/* console window enable when i want to debug */
int main()
{
	/* if got debug .. then try and catch error in real time */
	try 
	{
		LunaMain();
	} 
	catch (const std::runtime_error& e) 
	{
		luna::DebugLog::printF("[Main Thread Catch]->");
		luna::DebugLog::printF(e.what());
		system("pause");

		// destroy the windows
		luna::WinNative::getInstance()->Destroy();

		return EXIT_FAILURE;
	}

	return 0;

#elif VK_USE_PLATFORM_WIN32_KHR

/* release build for Window */
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{

#elif VK_USE_PLATFORM_ANDROID_KHR

#include "Global.h"

/* release build for android */
void android_main(android_app* pApplication)
{
	// makes sure native glue is not stripped by the linker
	app_dummy();

	// pointer to the android application
	luna::global::androidApplication = pApplication;

#endif
	
	/* release build version */
	/* no debug or whatsoever, zero overhead */
	LunaMain();
}