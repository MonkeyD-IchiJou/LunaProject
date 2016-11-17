#ifndef SCENE_DEFAULT_H
#define SCENE_DEFAULT_H

#include "Scene.h"
#include "SceneMedium.h"

namespace luna
{
	class SceneDefault :
		public Scene, SceneMedium
	{
	public:
		SceneDefault();
		virtual ~SceneDefault();

		/* scene update objects transformation, physics, AI, secondary command buffer, every frame */
		void Update(FramePacket& framepacket, std::array<Worker*, 2>& workers) override;

	private:
		/* scene necessary init */
		void Init_() override;

		/* Deinit everything */
		void DeInit_() override;
	};
}

#endif // !SCENE_DEFAULT_H


