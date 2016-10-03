#ifndef MODEL_H
#define MODEL_H

#include "platform.h"

#include <string>
#include <vector>

namespace luna
{
	class BasicMesh;
	class ModelResources;

	enum ePRIMITIVE_MESH
	{
		PRIMITIVE_QUAD = 0,
		PRIMITIVE_TRIANGLE = 1,
		PRIMITIVE_CUBE = 2,
		MAX_PRIMITIVE_MESH
	};

	class Model
	{
	friend class ModelResources;

	public:
		// load models with native mesh id
		Model(ePRIMITIVE_MESH meshid);

		// load models with files
		Model(std::string path);

		// destructor
		~Model();

		void Draw(const VkCommandBuffer& commandbuffer);
		void DrawInstanced(const VkCommandBuffer& commandbuffer, const uint32_t& instancecount);
		inline uint32_t getTotalMeshes() { return m_totalmeshes; }

	private:
		static int getInteger_(const std::string& data, size_t& currentLine, const char& delim);
		static float getFloat_(const std::string& data, size_t& currentLine, const char& delim);

		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory);
		void setMainBuffer(const VkBuffer& buffer);

		uint32_t m_totalmeshes = 0;
		std::vector<BasicMesh*> m_meshes;
	};
}

#endif


