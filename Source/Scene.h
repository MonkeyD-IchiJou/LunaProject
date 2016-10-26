#ifndef SCENE_H
#define SCENE_H

#include <string>
#include "StorageData.h"

namespace luna
{
	class ComponentManager;
	class Renderer;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		/* load datas to renderer and prepare everything before update every frame */
		virtual void EarlyUpdate() = 0;

		/* scene update objects transformation, physics, AI, secondary command buffer, every frame */
		virtual void Update() = 0;

		/* scene render every frame */
		virtual void Render() = 0;

		/* scene update after rendering on the screen */
		virtual void LateUpdate() = 0;

	protected:
		/* scene necessary init */
		virtual void Init_() = 0;

		/* Deinit everything */
		virtual void DeInit_() = 0;

		/* fill up the instance datas */
		void GetInstanceData_(std::vector<InstanceData>& instancedatas);

	protected:
		/* every scene has its own name */
		std::string m_scenename = "default";

		/* component manager contains all the components */
		ComponentManager* m_componentmanager = nullptr;

		/* render datas grouping for renderer later */
		std::vector<RenderingInfo> m_renderinfos;

		/* every scenes will have a renderer */
		Renderer* m_renderer = nullptr;
	};
}
#endif // !SCENE_H


