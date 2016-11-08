#ifndef FRAME_PACKET_H
#define FRAME_PACKET_H

#include <vector>
#include <glm\glm.hpp>
#include <array>
#include "enum_c.h"

namespace luna
{
	struct UBOData
	{
		glm::mat4 view, proj;
	};

	struct UBOPointLightData
	{
		glm::vec3 position{};
		glm::vec3 color{};
	};

	struct InstanceData
	{
		glm::mat4 model{};
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
		std::vector<RenderingInfo>* renderinfos = nullptr;
		std::vector<InstanceData> instancedatas;
		std::vector<FontInstanceData> fontinstancedatas;
		std::array<UBOPointLightData, 10> pointlightsdatas{};
		UBOData maincamdata{};
	};
}

#endif