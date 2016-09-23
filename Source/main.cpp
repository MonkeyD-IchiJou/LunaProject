#include "WinNative.h"
#include "DebugLog.h"
#include "Renderer.h"

/* all the game code is here */
static void luna_main()
{
	/* INIT EARLIER */
	// auto init vulkan when first getinstance
	luna::Renderer* renderer = luna::Renderer::getInstance();

	// auto create the window when first getinstance
	luna::WinNative* window = luna::WinNative::getInstance();

	// update the window and its input
	window->UpdateOSWin();

	/* Major Clean Up*/
	window->Destroy();
	renderer->Destroy();
}

#if _DEBUG
/* console window enable when i want to debug */
int main()
#else
/* release build for Window */
int CALLBACK WinMain( _In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
#endif

{
#if _DEBUG 

	/* if got debug .. then try and catch error in real time */
	try 
	{
		luna_main();
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
	luna_main();

#endif

	return 0;
}