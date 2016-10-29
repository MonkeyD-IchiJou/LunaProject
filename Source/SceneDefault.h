#ifndef SCENE_DEFAULT_H
#define SCENE_DEFAULT_H

#include "Scene.h"
#include "SceneSmall.h"

namespace luna
{
	class SceneDefault :
		public Scene, SceneSmall
	{
	public:
		SceneDefault();
		virtual ~SceneDefault();

		/* load datas to renderer and prepare everything before update every frame */
		void EarlyUpdate(FramePacket& framepacket) override;

		/* scene update objects transformation, physics, AI, secondary command buffer, every frame */
		void Update(FramePacket& framepacket) override;

	private:
		/* scene necessary init */
		void Init_() override;

		/* Deinit everything */
		void DeInit_() override;
	};
}

#endif // !SCENE_DEFAULT_H


