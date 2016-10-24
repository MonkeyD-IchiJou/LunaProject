#ifndef SCENE_H
#define SCENE_H

#include <string>

namespace luna
{
	class ComponentManager;

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

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

	protected:
		/* every scene has its own name */
		std::string m_scenename = "default";

		/* component manager contains all the components */
		ComponentManager* m_componentmanager = nullptr;
	};
}
#endif // !SCENE_H


