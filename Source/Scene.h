#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <array>

#include "FramePacket.h"
#include "Worker.h"

namespace luna
{
	class ComponentManager;
	class Renderer;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		/* scene update objects transformation, physics, AI, secondary command buffer, every frame */
		virtual void Update(FramePacket& framepacket, std::array<Worker, 4>& workers) = 0;

	protected:
		/* scene necessary init */
		virtual void Init_() = 0;

		/* Deinit everything */
		virtual void DeInit_() = 0;

		/* fill up the instance datas */
		static void GetInstanceData_(std::vector<InstanceData>& instancedatas, const std::vector<RenderingInfo>& renderinfos);

	protected:
		/* every scene has its own name */
		std::string m_scenename = "default";

		/* component manager contains all the components */
		ComponentManager* m_componentmanager = nullptr;

		/* all the mesh render info */
		std::vector<RenderingInfo> m_renderinfos;
	};
}
#endif // !SCENE_H


