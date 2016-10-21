#include "DebugLog.h"
#include "Renderer.h"
#include "WinNative.h"
#include "VulkanSwapchain.h"

#include <chrono>
// we use a fixed timestep of 1 / (60 fps) = 16 milliseconds
constexpr std::chrono::nanoseconds timestep(16666666);

/* all the game code is here */
static void luna_main()
{
	/* INIT EARLIER FROM HERE */
	// auto init vulkan when first getinstance
	luna::Renderer* renderer = luna::Renderer::getInstance();
	
	// auto create the window and its surface when first getinstance
	luna::WinNative* window = luna::WinNative::getInstance();

	// create necessary resources, models, ubo, textures, shaders, swapchain, fbos .. all must loaded here and once only 
	renderer->CreateResources();
	/* TILL HERE */

	// command buffer record how it is going to render
	renderer->Record();
	
	using clock = std::chrono::steady_clock;
	std::chrono::nanoseconds lag(0);
	clock::time_point time_start = clock::now();
	
	while (!window->isClose())
	{
		// calc how much time have elapsed from prev frame
		auto delta_time = clock::now() - time_start;
		time_start = clock::now(); // start of this frame

		// update the window and its input
		window->UpdateOSWin();

		// update game logic
		renderer->Update();

		// begin to render everything and present it on screen
		renderer->Render();

		luna::DebugLog::printL(std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count());
	}

	/* Major Clean Up*/
	renderer->CleanUpResources();
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