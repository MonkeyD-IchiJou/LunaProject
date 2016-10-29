#ifndef STORAGE_DATA_H
#define STORAGE_DATA_H

#include <glm\glm.hpp>
#include "enum_c.h"
#include <vector>

namespace luna
{
	struct UBOData
	{
		glm::mat4 view, proj;
	};

	struct InstanceData
	{
		glm::mat4 model{};
		glm::mat4 transpose_inverse_model{};
		glm::vec4 material{};
	};

	struct FontInstanceData
	{
		glm::mat4 transformation{};
		glm::mat4 fontMaterials{};
		glm::vec2 uv[4];
	};

	class BasicMeshComponent;
	struct RenderingInfo
	{
		eMODELS modelID = MAX_MODELS;
		MESH_TEX textureID = MAX_MESH_TEX;
		std::vector<BasicMeshComponent*> instancedatas{};

		static int totalcounter;
	};
	
	struct FramePacket
	{
		std::vector<RenderingInfo> renderinfos;
		std::vector<InstanceData> instancedatas;
		std::vector<FontInstanceData> fontinstancedatas;
		UBOData maincamdata{};
	};
}

#endif