#ifndef FRAME_PACKET_H
#define FRAME_PACKET_H

#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "enum_c.h"

namespace luna
{
	struct UBOData
	{
		glm::mat4 view, transposeinverseview, proj, prevview;
	};

	struct PointLightData
	{
		glm::vec4 position{};
		glm::vec4 color{};
	};

	struct InstanceData
	{
		glm::mat4 model{};
		glm::mat4 transposeinversemodel{};
		glm::mat4 prevmodel{};
		glm::vec4 material{};
	};

	struct FontInstanceData
	{
		glm::mat4 transformation{};
		glm::mat4 fontMaterials{};
		glm::vec2 uv[4];
	};

	struct MainDirLightData
	{
		glm::vec4 diffusespec{};
		glm::vec4 ambientlight{};
		glm::vec4 dirlightdir{};
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
		std::vector<PointLightData> pointlightsdatas{};
		MainDirLightData dirlightdata{};
		UBOData maincamdata{};
		glm::vec3 maincampos{};
	};
}

#endif