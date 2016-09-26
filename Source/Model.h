#ifndef MODEL_H
#define MODEL_H

#include "platform.h"

#include <string>
#include <vector>

namespace luna
{
	class BasicMesh;

	enum ePRIMITIVE_MESH
	{
		PRIMITIVE_QUAD = 0,
		PRIMITIVE_TRIANGLE = 1,
		MAX_PRIMITIVE_MESH
	};

	class Model
	{
	public:
		Model(ePRIMITIVE_MESH meshid);
		Model(std::string path);
		~Model();

		void Draw(const VkCommandBuffer& commandbuffer);
		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory);
		void setMainBuffer(const VkBuffer& buffer);

	private:
		static int getInteger_(const std::string& data, size_t& currentLine, const char& delim);
		static float getFloat_(const std::string& data, size_t& currentLine, const char& delim);
		std::vector<BasicMesh*> m_meshes;
	};
}

#endif


