#include "IOManager.h"
#include "DebugLog.h"
#include "Global.h"

namespace luna
{
	namespace IO
	{
		void LoadFile(const std::string & filename, std::vector<char>& buffer)
		{
#if VK_USE_PLATFORM_WIN32_KHR
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
			{
				DebugLog::throwEx("failed to open file!");
			}

			const size_t fileSize = static_cast<size_t>(file.tellg());

			if (buffer.size() > 0)
				buffer.clear();

			buffer.resize(fileSize);

			// seek back to the beggining of the file and read all of the bytes at once
			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

#elif defined(__ANDROID__)

			AAsset* asset = AAssetManager_open(global::androidApplication->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
			if (asset == nullptr)
				DebugLog::printF("failed to open file!");

			size_t size = static_cast<size_t>(AAsset_getLength(asset));
			if(size <= 0)
				DebugLog::printF("file size is less than zero");

			if (buffer.size() > 0)
				buffer.clear();

			buffer.resize(size);

			AAsset_read(asset, buffer.data(), size);
			AAsset_close(asset);
#endif



		}
	}

}
