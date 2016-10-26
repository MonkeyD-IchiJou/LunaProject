#ifndef ROTATE_SCRIPT_H
#define ROTATE_SCRIPT_H

#include "Component.h"

namespace luna
{
	class ScriptComponent :
		public Component
	{
	public:
		ScriptComponent();
		virtual ~ScriptComponent();

		void Update() override;
		void Reset() override;
	};
}

#endif

